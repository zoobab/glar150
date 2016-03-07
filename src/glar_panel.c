/*  =========================================================================
    glar_panel - LED panel controller

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Glar150 Project.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    glar_panel - LED panel controller
@discuss
    Commands:

    nnn         3-LED bitmap, n is 0 or 1 left to right
    ,           Pause for 100msec
    ;           Pause for 500msec
    .           Pause for 100msec if inactive
    :           Pause for 500msec if inactive
    *           Repeat previous sequence
@end
*/

#include "glar_classes.h"
#include "glar_panel_fsm.h"     //  Generated state machine engine

//  Structure of our actor

struct _glar_panel_t {
    zsock_t *pipe;              //  Actor command pipe
    zpoller_t *poller;          //  Socket poller
    bool terminated;            //  Did caller ask us to quit?
    bool verbose;               //  Verbose logging enabled?
    fsm_t *fsm;                 //  Our finite state machine
    char *sequence;             //  Current display sequence
    char *seq_ptr;              //  Pointer into current sequence
    char command;               //  Current command in sequence
    int selection;              //  LED bitmap, b000...b111
    int timeout;                //  Milliseconds for next poll
};


//  --------------------------------------------------------------------------
//  Create a new glar_panel instance

static glar_panel_t *
glar_panel_new (zsock_t *pipe, void *args)
{
    glar_panel_t *self = (glar_panel_t *) zmalloc (sizeof (glar_panel_t));
    assert (self);
    self->pipe = pipe;
    self->poller = zpoller_new (self->pipe, NULL);
    self->fsm = fsm_new (self);
    self->timeout = -1;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the glar_panel instance

static void
glar_panel_destroy (glar_panel_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        glar_panel_t *self = *self_p;
        fsm_destroy (&self->fsm);
        zpoller_destroy (&self->poller);
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  This is the actor which runs in its own thread.

void
glar_panel_actor (zsock_t *pipe, void *args)
{
    glar_panel_t * self = glar_panel_new (pipe, args);
    assert (self);
    zsock_signal (self->pipe, 0);       //  Tell caller we're ready

    //  Main loop, we wait on commands coming via our pipe
    while (!self->terminated) {
        if (self->verbose)
            zsys_info ("glar_panel: wait timeout=%d", self->timeout);
        zsock_t *which = (zsock_t *) zpoller_wait (self->poller, self->timeout);
        if (which == self->pipe) {
            zmsg_t *request = zmsg_recv (self->pipe);
            if (!request)
                return;        //  Interrupted

            char *command = zmsg_popstr (request);
            if (self->verbose)
                zsys_info ("glar_panel: pipe command=%s", command);
            if (streq (command, "VERBOSE")) {
                self->verbose = true;
                fsm_set_animate (self->fsm, true);
            }
            else
            if (streq (command, "$TERM"))
                self->terminated = true;
            else {
                //  Whatever came on the pipe is a new command sequence
                free (self->sequence);
                self->sequence = command;
                self->seq_ptr = self->sequence;
                command = NULL;
                get_next_command (self);
            }
            zstr_free (&command);
            zmsg_destroy (&request);
        }
        else
            get_next_command (self);

        //  Execute state machine until it needs an event
        fsm_execute (self->fsm);
    }
    glar_panel_destroy (&self);
}


//  ---------------------------------------------------------------------------
//  get_next_command
//

static void
get_next_command (glar_panel_t *self)
{
    self->command = *self->seq_ptr;
    if (self->command == '\0')
        fsm_set_next_event (self->fsm, finished_event);
    else {
        self->seq_ptr++;
        if (self->command == '0' || self->command == '1')
            fsm_set_next_event (self->fsm, bitmap_event);
        else
        if (self->command == ',')
            fsm_set_next_event (self->fsm, short_pause_event);
        else
        if (self->command == ';')
            fsm_set_next_event (self->fsm, long_pause_event);
        else
        if (self->command == '.')
            fsm_set_next_event (self->fsm, short_poll_event);
        else
        if (self->command == ':')
            fsm_set_next_event (self->fsm, long_poll_event);
        else
        if (self->command == '*')
            fsm_set_next_event (self->fsm, repeat_event);
        else
            zsys_error ("Unexpected command '%c', ignored", self->command);
    }
}


//  ---------------------------------------------------------------------------
//  collect_full_bitmap
//

static void
collect_full_bitmap (glar_panel_t *self)
{
    self->selection = 0;
    while (true) {
        self->selection = (self->selection << 1) + (self->command - '0');
        if (*self->seq_ptr == '0' || *self->seq_ptr == '1')
            self->command = *self->seq_ptr++;
        else
            break;
    }
}


//  ---------------------------------------------------------------------------
//  set_leds_as_specified
//

static void
s_set_led_status (int selection, int bitvalue, const char *name)
{
    const char *path = "/sys/devices/platform/leds-gpio/leds/gl_ar150:%s/brightness";
    const char *led_value = selection & bitvalue? "1": "0";
    char *device = zsys_sprintf (path, name);
    int handle = open (device, O_WRONLY);

    if (handle == -1)
        //  We're probably not on a GL-AR150
        zsys_info ("set led=%s value=%s", name, led_value);
    else {
        if (write (handle, led_value, 1) == -1)
            zsys_error ("can't write to device=%s", device);
        close (handle);
    }
    free (device);
}


static void
set_leds_as_specified (glar_panel_t *self)
{
    //  Set LED status from left to right
    s_set_led_status (self->selection, 4, "wan");
    s_set_led_status (self->selection, 2, "lan");
    s_set_led_status (self->selection, 1, "wlan");
}


//  ---------------------------------------------------------------------------
//  do_short_sleep
//

static void
do_short_sleep (glar_panel_t *self)
{
    zclock_sleep (100);
}


//  ---------------------------------------------------------------------------
//  do_long_sleep
//

static void
do_long_sleep (glar_panel_t *self)
{
    zclock_sleep (500);
}


//  ---------------------------------------------------------------------------
//  prepare_short_poll
//

static void
prepare_short_poll (glar_panel_t *self)
{
    self->timeout = 100;
}


//  ---------------------------------------------------------------------------
//  prepare_long_poll
//

static void
prepare_long_poll (glar_panel_t *self)
{
    self->timeout = 500;
}


//  ---------------------------------------------------------------------------
//  prepare_blocking_poll
//

static void
prepare_blocking_poll (glar_panel_t *self)
{
    self->timeout = -1;
}


//  ---------------------------------------------------------------------------
//  start_sequence_again
//

static void
start_sequence_again (glar_panel_t *self)
{
    assert (self->sequence);
    self->seq_ptr = self->sequence;
}


//  --------------------------------------------------------------------------
//  Self test of this actor.

void
glar_panel_test (bool verbose)
{
    printf (" * glar_panel: ");

    //  @selftest
    zactor_t *glar_panel = zactor_new (glar_panel_actor, NULL);
    if (verbose)
        zstr_send (glar_panel, "VERBOSE");
    zstr_send (glar_panel, "100,010,001,");
    zstr_send (glar_panel, "100.010.001.*");
    zclock_sleep (800);
    zstr_send (glar_panel, "111,000,");
    zactor_destroy (&glar_panel);
    //  @end

    printf ("OK\n");
}
