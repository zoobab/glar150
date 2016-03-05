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
    zlist_t *players;           //  Players in our catch team
} self_t;

static int
s_parse_args (self_t *self, int argc, char *argv [])
{
    //  Defaults
    self->iface = "wlan0";

    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("blink_shell [options] ...");
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


//  We flash our light once, then pass the ball on
static void
s_play_catch (self_t *self)
{
    //  Flash LED 1 three times rapidly, and blink light
    if (system ("blink 1 0.5") == 0)
        zstr_send (self->panel, "1X,-,X,-,X,-,");
    else {
        sleep (1);
        puts ("BLINK!");
    }
    //  Find first peer with UUID greater than ours
    char *player = (char *) zlist_first (self->players);
    while (player) {
        if (strcmp (player, zyre_uuid (self->zyre)) > 0)
            break;
        player = (char *) zlist_next (self->players);
    }
    //  Wrap around to first if needed
    if (!player)
        player = (char *) zlist_first (self->players);

    //  Send if we have another player, else drop the ball
    if (player)
        zyre_whispers (self->zyre, player, "%s", "catch");
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
            if (streq (zyre_event_group (event), "BLINK")) {
                zsys_info ("[%s] peer joined %s",
                           zyre_event_peer_name (event),
                           zyre_event_group (event));
                self->peers++;
                //  Flash LED 1 three times rapidly
                zstr_send (self->panel, "1X,-,X,-,X,-,");
            }
            else
            if (streq (zyre_event_group (event), "CATCH")) {
                //  Add to list of players
                zlist_append (self->players, (void *) zyre_event_peer_uuid (event));
                zlist_sort (self->players, NULL);
            }
        }
        else
        if (streq (zyre_event_type (event), "LEAVE")) {
            if (streq (zyre_event_group (event), "BLINK")) {
                zsys_info ("[%s] peer left", zyre_event_peer_name (event));
                self->peers--;
                //  Flash LED 2 three times rapidly
                zstr_send (self->panel, "2X,-,X,-,X,-,");
            }
            else
            if (streq (zyre_event_group (event), "CATCH"))
                zlist_remove (self->players, (void *) zyre_event_peer_uuid (event));
        }
        else
        if (streq (zyre_event_type (event), "SHOUT")) {
            zsys_info ("[%s](%s) received SHOUT",
                       zyre_event_peer_name (event), zyre_event_group (event));
            zmsg_t *msg = zyre_event_msg (event);
            char *command = zmsg_popstr (msg);
            zsys_info ("Run command '%s'", command);
            if (streq (command, "catch"))
                s_play_catch (self);
            else
            if (system (command) == 0) {
                zsys_info ("System '%s' OK", command);
                //  Flash LED 1 once slowly
                zstr_send (self->panel, "1X;-;");
                zyre_whispers (self->zyre, zyre_event_peer_uuid (event), "%s", "OK");
            }
            else {
                zsys_info ("System '%s' FAIL", command);
                //  Flash LED 2 once slowly
                zstr_send (self->panel, "2X;-;");
                zyre_whispers (self->zyre, zyre_event_peer_uuid (event), "%s", "FAILED");
            }
            free (command);
        }
        else
        if (streq (zyre_event_type (event), "WHISPER")) {
            zsys_info ("[%s] received WHISPER", zyre_event_peer_name (event));
            zmsg_t *msg = zyre_event_msg (event);
            char *command = zmsg_popstr (msg);
            if (streq (command, "catch"))
                s_play_catch (self);
            free (command);
        }
        else
        if (streq (zyre_event_type (event), "EXIT"))
            zlist_remove (self->players, (void *) zyre_event_peer_uuid (event));

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
        if (streq (command, "catch")) {
            //  Send to any peer, it doesn't matter which
            zlist_t *peers = zyre_peers (self->zyre);
            if (zlist_size (peers)) {
                zyre_whispers (self->zyre, (char *) zlist_first (peers), "%s", "catch");
                zlist_destroy (&peers);
            }
            else
                zsys_error ("there's no-one to play catch...");
        }
        else
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
    zyre_set_interface (self->zyre, self->iface);
    zyre_start (self->zyre);
    zyre_join (self->zyre, "BLINK");

    if (!self->console) {
        self->players = zlist_new ();
        zlist_autofree (self->players);
        zyre_join (self->zyre, "CATCH");
    }
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
    if (!self->console)
        zyre_leave (self->zyre, "CATCH");

    zyre_leave (self->zyre, "BLINK");
    zyre_stop (self->zyre);
    zyre_destroy (&self->zyre);
    zactor_destroy (&self->panel);
    zlist_destroy (&self->players);

    free (self);
    return 0;
}
