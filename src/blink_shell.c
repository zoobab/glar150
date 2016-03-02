/*  =========================================================================
    blink_shell - Blink shell

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Blink Project.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    blink - 
@discuss
@end
*/

#include "blink.h"

typedef struct {
    bool verbose;
    bool console;
    char *iface;
    blink_led_t *led [3];
    zyre_t *zyre;
    size_t peers;               //  Number of peers
} self_t;

static int
s_parse_args (self_t *self, int argc, char *argv [])
{
    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("monitor [options] ...");
            puts ("  --help / -h            this help");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --interface / -i       use this interface");
            puts ("  --console / -c         remote control console");
            return -1;
        }
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            self->verbose = true;
        else
        if (streq (argv [argn], "--console")
        ||  streq (argv [argn], "-c"))
            self->console = true;
        else
        if (streq (argv [argn], "--interface")
        ||  streq (argv [argn], "-i"))
            self->iface = argv [++argn];
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return -1;
        }
    }
    return 0;
}


static bool
s_join_cluster (self_t *self)
{
    //  Start LED cycling until Zyre is ready
    zsys_info ("blink_shell: waiting...");

    //  Wait for at least one peer to join the group
    //  While we're waiting we blink the LEDs in a cycle
    bool waiting = true;
    int current_led = 0;
    printf (" ");
    char *tick = ".oO";

    zpoller_t *poller = zpoller_new (zyre_socket (self->zyre), NULL);
    while (waiting) {
        zsock_t *which = (zsock_t *) zpoller_wait (poller, 333);
        if (zsys_interrupted)
            break;
        printf ("\b%c", tick [current_led]);
        fflush (stdout);

        blink_led_off (self->led [current_led]);
        current_led = (current_led + 1) % 3;
        blink_led_on (self->led [current_led]);

        if (which == zyre_socket (self->zyre)) {
            zmsg_t *msg = zyre_recv (self->zyre);
            char *command = zmsg_popstr (msg);
            if (streq (command, "JOIN")) {
                self->peers++;
                waiting = false;
            }
            zstr_free (&command);
            zmsg_destroy (&msg);
        }
    }
    blink_led_off (self->led [current_led]);
    if (zsys_interrupted)
        return false;

    //  Set first LED on permanently to show we're active
    blink_led_on (self->led [0]);
    zsys_info ("blink_shell: attached to cluster");
    return true;
}


static void
s_capture_commands (self_t *self)
{
    //  Wait for commands, and execute via /bin/sh
    while (true) {
        zyre_event_t *event = zyre_event_new (self->zyre);
        if (!event)
            break;              //  Interrupted
        if (self->verbose)
            zyre_event_print (event);

        if (streq (zyre_event_type (event), "JOIN")) {
            zsys_info ("[%s] peer joined", zyre_event_peer_name (event));
            self->peers++;
            //  Flash 2nd LED three times rapidly
            for (int repeat = 0; repeat < 3; repeat++) {
                blink_led_on (self->led [1]);
                zclock_sleep (100);
                blink_led_off (self->led [1]);
                zclock_sleep (100);
            }
        }
        else
        if (streq (zyre_event_type (event), "LEAVE")) {
            zsys_info ("[%s] peer left", zyre_event_peer_name (event));
            self->peers--;
            //  Flash 3rd LED three times rapidly
            for (int repeat = 0; repeat < 3; repeat++) {
                blink_led_on (self->led [2]);
                zclock_sleep (100);
                blink_led_off (self->led [2]);
                zclock_sleep (100);
            }
        }
        else
        if (streq (zyre_event_type (event), "SHOUT")) {
            zsys_info ("[%s](%s) received ping (SHOUT)",
                       zyre_event_peer_name (event), zyre_event_group (event));
            zmsg_t *msg = zyre_event_msg (event);
            char *command = zmsg_popstr (msg);

            if (system (command) == 0) {
                zsys_info ("System '%s' OK", command);
                //  Flash 2nd LED once slowly
                blink_led_on (self->led [1]);
                zclock_sleep (500);
                blink_led_off (self->led [1]);
                zyre_whispers (self->zyre, zyre_event_peer_uuid (event), "%s", "OK");
            }
            else {
                zsys_info ("System '%s' FAIL", command);
                //  Flash 3rd LED once slowly
                blink_led_on (self->led [1]);
                zclock_sleep (500);
                blink_led_off (self->led [1]);
                zyre_whispers (self->zyre, zyre_event_peer_uuid (event), "%s", "FAILED");
            }
            free (command);
        }
        zyre_event_destroy (&event);
    }
}


static void
s_broadcast_commands (self_t *self)
{
    while (!zsys_interrupted) {
        char command [1024];
        if (!fgets (command, 1024, stdin))
            break;
        //  Line ends in \n, which we need to discard
        command [strlen (command) - 1] = 0;
        zyre_shouts (self->zyre, "BLINK", "%s", command);

        //  Wait for answers from peers
        //  TODO: do this asynchronously
        size_t answers = 0;
        while (answers < self->peers) {
            zyre_event_t *event = zyre_event_new (self->zyre);
            if (!event)
                break;              //  Interrupted
            if (self->verbose)
                zyre_event_print (event);

            if (streq (zyre_event_type (event), "JOIN"))
                self->peers++;
            else
            if (streq (zyre_event_type (event), "LEAVE"))
                self->peers--;
            else
            if (streq (zyre_event_type (event), "WHISPER")) {
                answers++;
                zmsg_t *msg = zyre_event_msg (event);
                char *answer = zmsg_popstr (msg);
                printf ("%s: %s\n", zyre_event_peer_name (event), answer);
                free (answer);
            }
            zyre_event_destroy (&event);
        }
    }
}


int main (int argc, char *argv [])
{
    self_t *self = (self_t *) zmalloc (sizeof (self_t));
    if (s_parse_args (self, argc, argv))
        return 0;

    //  Startup
    self->zyre = zyre_new (NULL);
    for (int index = 0; index < 3; index++)
        self->led [index] = blink_led_new (index);
    if (self->verbose)
        zyre_set_verbose (self->zyre);
    if (self->iface)
        zyre_set_interface (self->zyre, self->iface);
    zyre_start (self->zyre);
    zyre_join (self->zyre, "BLINK");

    bool ready = s_join_cluster (self);
    if (ready) {
        if (self->console)
            s_broadcast_commands (self);
        else
            s_capture_commands (self);
    }
    //  Shutdown
    zyre_leave (self->zyre, "BLINK");
    zyre_stop (self->zyre);
    zyre_destroy (&self->zyre);
    for (int index = 0; index < 3; index++)
        blink_led_destroy (&self->led [index]);

    free (self);
    return 0;
}
