/*  =========================================================================
    blink - description

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

int main (int argc, char *argv [])
{
    bool verbose = false;
    char *iface = NULL;
    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("monitor [options] ...");
            puts ("  --help / -h            this help");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --interface / -i       use this interface");
            return 0;
        }
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            verbose = true;
        else
        if (streq (argv [argn], "--interface")
        ||  streq (argv [argn], "-i"))
            iface = argv [++argn];
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return 1;
        }
    }
    //  Start LED cycling until Zyre is ready
    blink_led_t *led [3] = { blink_led_new (1), blink_led_new (2), blink_led_new (3) };
    blink_led_on (led [0]);

    zyre_t *zyre = zyre_new (NULL);
    if (verbose)
        zyre_set_verbose (zyre);
    if (iface)
        zyre_set_interface (zyre, iface);
    zyre_start (zyre);
    zyre_join (zyre, "BLINK");

    zpoller_t *poller = zpoller_new (zyre_socket (zyre), NULL);
    bool waiting = true;
    int current_led = 0;
    while (waiting) {
        zsock_t *which = (zsock_t *) zpoller_wait (poller, 333);
        if (!which)
            break;              //  Interrupted

        printf ("."); fflush (stdout);
        blink_led_off (led [current_led]);
        current_led = (current_led + 1) % 3;
        blink_led_on (led [current_led]);

        zmsg_t *msg = zyre_recv (zyre);
        char *command = zmsg_popstr (msg);
        if (streq (command, "JOIN"))
            waiting = false;
        zstr_free (&command);
        zmsg_destroy (&msg);
    }
    blink_led_off (led [current_led]);

    while (true) {
        zmsg_t *msg = zyre_recv (zyre);
        if (!msg)
            break;
        zmsg_destroy (&msg);
    }
    zyre_destroy (&zyre);
    blink_led_destroy (&led [0]);
    blink_led_destroy (&led [1]);
    blink_led_destroy (&led [2]);
    return 0;
}
