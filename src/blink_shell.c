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
    zyre_t *zyre;
    zactor_t *panel;            //  LED control panel
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

static void
led_panel (zsock_t *pipe, void *args)
{
    blink_led_t *led [3];
    for (int index = 0; index < 3; index++)
        led [index] = blink_led_new (index);

    zsock_signal (pipe, 0);     //  Signal "ready" to caller
    while (!zsys_interrupted) {
        char *command = zstr_recv (pipe);
        if (streq (command, "$TERM"))
            break;
        char *opcode = command;
        blink_led_t *choice = NULL;
        while (*opcode) {
            if (isdigit (*opcode)) {
                uint index = *opcode - '0';
                assert (index < 3);
                choice = led [index];
            }
            else
            if (*opcode == 'X')
                blink_led_on (choice);
            else
            if (*opcode == '-')
                blink_led_off (choice);
            else
            if (*opcode == ',')
                zclock_sleep (100);
            else
            if (*opcode == ';')
                zclock_sleep (500);
            opcode++;
        }
        free (command);
    }
    for (int index = 0; index < 3; index++)
        blink_led_destroy (&led [index]);
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

        zstr_sendf (self->panel, "%c-", current_led + '0');
        current_led = (current_led + 1) % 3;
        zstr_sendf (self->panel, "%cX", current_led + '0');

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
    zstr_sendf (self->panel, "%c-", current_led + '0');
    if (zsys_interrupted)
        return false;

    //  Set first LED on permanently to show we're active
    zstr_send (self->panel, "0X");
    zsys_info ("blink_shell: attached to cluster");
    return true;
}


static void
s_capture_commands (self_t *self)
{
    //  Wait for commands, and execute via /bin/sh
    while (!zsys_interrupted) {
        zyre_event_t *event = zyre_event_new (self->zyre);
        if (!event)
            break;              //  Interrupted
        if (self->verbose)
            zyre_event_print (event);

        if (streq (zyre_event_type (event), "JOIN")) {
            zsys_info ("[%s] peer joined", zyre_event_peer_name (event));
            self->peers++;
            //  Flash LED 1 three times rapidly
            zstr_send (self->panel, "1X,-,X,-,X,-");
        }
        else
        if (streq (zyre_event_type (event), "LEAVE")) {
            zsys_info ("[%s] peer left", zyre_event_peer_name (event));
            self->peers--;
            //  Flash LED 2 three times rapidly
            zstr_send (self->panel, "2X,-,X,-,X,-");
        }
        else
        if (streq (zyre_event_type (event), "SHOUT")) {
            zsys_info ("[%s](%s) received ping (SHOUT)",
                       zyre_event_peer_name (event), zyre_event_group (event));
            zmsg_t *msg = zyre_event_msg (event);
            char *command = zmsg_popstr (msg);

            if (system (command) == 0) {
                zsys_info ("System '%s' OK", command);
                //  Flash LED 1 once slowly
                zstr_send (self->panel, "1X;-");
                zyre_whispers (self->zyre, zyre_event_peer_uuid (event), "%s", "OK");
            }
            else {
                zsys_info ("System '%s' FAIL", command);
                //  Flash LED 2 once slowly
                zstr_send (self->panel, "2X;-");
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

int
main (int argc, char *argv [])
{
    self_t *self = (self_t *) zmalloc (sizeof (self_t));
    if (s_parse_args (self, argc, argv))
        return 0;

    //  Startup
    self->zyre = zyre_new (NULL);
    if (self->verbose)
        zyre_set_verbose (self->zyre);
    if (self->iface)
        zyre_set_interface (self->zyre, self->iface);
    zyre_start (self->zyre);
    zyre_join (self->zyre, "BLINK");

    self->panel = zactor_new (led_panel, NULL);
    assert (self->panel);

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
    zactor_destroy (&self->panel);

    free (self);
    return 0;
}
