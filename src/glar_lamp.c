/*  =========================================================================
    glar_lamp - Lamp controller

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Glar150 Project.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    glar_lamp - LED lamp controller
@discuss
    Commands:
        + -         Switch lamp on or off
        nnn         Pause for nnn msec
        ?nnn        Pause for nnn msec if inactive
        *           Repeat previous sequence
@end
*/

#include "glar_classes.h"
#include "glar_lamp_fsm.h"     //  Generated state machine engine

//  Structure of our actor

struct _glar_lamp_t {
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
//  Create a new glar_lamp instance

static glar_lamp_t *
glar_lamp_new (zsock_t *pipe, void *args)
{
    glar_lamp_t *self = (glar_lamp_t *) zmalloc (sizeof (glar_lamp_t));
    assert (self);
    self->pipe = pipe;
    self->poller = zpoller_new (self->pipe, NULL);
    self->fsm = fsm_new (self);
    self->timeout = -1;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the glar_lamp instance

static void
glar_lamp_destroy (glar_lamp_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        glar_lamp_t *self = *self_p;
        fsm_destroy (&self->fsm);
        zpoller_destroy (&self->poller);
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  This is the actor which runs in its own thread.

void
glar_lamp_actor (zsock_t *pipe, void *args)
{
    glar_lamp_t *self = glar_lamp_new (pipe, args);
    assert (self);
    zsock_signal (self->pipe, 0);       //  Tell caller we're ready

    //  Main loop, we wait on commands coming via our pipe
    while (!self->terminated) {
        if (self->verbose)
            zsys_info ("glar_lamp: wait timeout=%d", self->timeout);
        zsock_t *which = (zsock_t *) zpoller_wait (self->poller, self->timeout);
        if (which == self->pipe) {
            zmsg_t *request = zmsg_recv (self->pipe);
            if (!request)
                return;        //  Interrupted

            char *command = zmsg_popstr (request);
            if (self->verbose)
                zsys_info ("glar_lamp: pipe command=%s", command);
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
    glar_lamp_destroy (&self);
}


//  ---------------------------------------------------------------------------
//  get_next_command
//

static void
get_next_command (glar_lamp_t *self)
{
    self->command = *self->seq_ptr;
    if (self->command == '\0')
        fsm_set_next_event (self->fsm, finished_event);
    else {
        self->seq_ptr++;
        if (self->command == '+')
            fsm_set_next_event (self->fsm, lamp_on_event);
        else
        if (self->command == '-')
            fsm_set_next_event (self->fsm, lamp_off_event);
        else
        if (self->command == '?') {
            fsm_set_next_event (self->fsm, maybe_event);
            self->command = *self->seq_ptr++;
            assert (isdigit (self->command));
        }
        else
        if (isdigit (self->command))
            fsm_set_next_event (self->fsm, sleep_event);
        else
        if (self->command == '*')
            fsm_set_next_event (self->fsm, repeat_event);
        else
            zsys_error ("Unexpected command '%c', ignored", self->command);
    }
}


//  ---------------------------------------------------------------------------
//  set_lamp_on
//

static void
s_set_lamp (const char *value)
{
    int handle = open ("/sys/class/gpio/gpio1/value", O_WRONLY);
    if (handle == -1)
        //  We're probably not on a GL-AR150
        zsys_info ("set lamp=%s", value);
    else {
        if (write (handle, value, strlen (value)) == -1)
            zsys_error ("can't write to GPIO 1");
        close (handle);
    }
}

static void
set_lamp_on (glar_lamp_t *self)
{
    s_set_lamp ("1");
}


//  ---------------------------------------------------------------------------
//  set_lamp_off
//

static void
set_lamp_off (glar_lamp_t *self)
{
    s_set_lamp ("0");
}


//  ---------------------------------------------------------------------------
//  collect_timeout_value
//

static void
collect_timeout_value (glar_lamp_t *self)
{
    self->timeout = 0;
    while (true) {
        self->timeout = (self->timeout * 10) + (self->command - '0');
        if (isdigit (*self->seq_ptr))
            self->command = *self->seq_ptr++;
        else
            break;
    }
}


//  ---------------------------------------------------------------------------
//  sleep_as_specified
//

static void
sleep_as_specified (glar_lamp_t *self)
{
    zclock_sleep (self->timeout);
}


//  ---------------------------------------------------------------------------
//  prepare_blocking_poll
//

static void
prepare_blocking_poll (glar_lamp_t *self)
{
    self->timeout = -1;
}


//  ---------------------------------------------------------------------------
//  start_sequence_again
//

static void
start_sequence_again (glar_lamp_t *self)
{
    assert (self->sequence);
    self->seq_ptr = self->sequence;
}


//  --------------------------------------------------------------------------
//  Self test of this actor.

void
glar_lamp_test (bool verbose)
{
    printf (" * glar_lamp: ");

    //  @selftest
    zactor_t *glar_lamp = zactor_new (glar_lamp_actor, NULL);
    if (verbose)
        zstr_send (glar_lamp, "VERBOSE");
    zstr_send (glar_lamp, "+300-300+300-300");
    zstr_send (glar_lamp, "+?300-?300+?300-?300");
    zclock_sleep (500);
    zstr_send (glar_lamp, "+300-300");
    zactor_destroy (&glar_lamp);
    //  @end

    printf ("OK\n");
}
