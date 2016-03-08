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
    Converts a character string to Morse code on the lamp. Spaces are turned
    into word spaces. If a new command arrives, it will always interrupt the
    existing command. Ending the string with "*" causes it to repeat until
    interrupted.
@discuss
@end
*/

#include "glar_classes.h"

//  Structure of our actor

struct _glar_lamp_t {
    zsock_t *pipe;              //  Actor command pipe
    zpoller_t *poller;          //  Socket poller
    bool terminated;            //  Did caller ask us to quit?
    bool verbose;               //  Verbose logging enabled?
    char *sequence;             //  Current display sequence
    char *seq_ptr;              //  Pointer into current sequence
    int timeout;                //  Milliseconds for next poll
};

#define MORSE_PERIOD    150     //  Single time unit

char *morse_alphabet [] = {
    "A.-", "B-...", "C-.-.", "D-..", "E.", "F..-.", "G--.",
    "H....", "I..", "J.---", "K-.-", "L.-..", "M--", "N-.",
    "O---", "P.--.", "Q--.-", "R.-.", "S...", "T-", "U..-",
    "V...-", "W.--", "X-..-", "Y-.--", "Z--..",
    "0-----", "1.----", "2..---", "3...--", "4....-",
    "5.....", "6-....", "7--...", "8---..", "9----."
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
        zpoller_destroy (&self->poller);
        free (self);
        *self_p = NULL;
    }
}


static void
s_set_lamp (bool on)
{
    char *value = on? "1": "0";
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


//  Show a dot/dash sequence

static void
s_display_letter (char *sequence)
{
    while (*sequence) {
        if (*sequence == '.') {
            s_set_lamp (true);
            zclock_sleep (MORSE_PERIOD);
            s_set_lamp (false);
        }
        else
        if (*sequence == '-') {
            s_set_lamp (true);
            zclock_sleep (3 * MORSE_PERIOD);
            s_set_lamp (false);
        }
        zclock_sleep (MORSE_PERIOD);
        sequence++;
    }
}


static void
s_process_next (glar_lamp_t *self)
{
    if (*self->seq_ptr == '\0')
        return;             //  At end of string, do nothing

    char letter = *self->seq_ptr++;
    if (isalnum (letter)) {
        letter = toupper (letter);
        for (int index = 0; index < sizeof (morse_alphabet) / sizeof (char *); index++) {
            if (morse_alphabet [index][0] == letter) {
                s_display_letter (morse_alphabet [index] + 1);
                self->timeout = 2 * MORSE_PERIOD;
                break;
            }
        }
    }
    else
    if (letter == ' ') {
        s_set_lamp (false);
        self->timeout = 7 * MORSE_PERIOD;
    }
    else
    if (letter == '*') {
        //  Restart sequence
        self->timeout = 0;
        self->seq_ptr = self->sequence;
    }
    else {
        zsys_error ("glar_lamp: unknown command '%c'", letter);
        self->timeout = -1;
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
            if (streq (command, "VERBOSE"))
                self->verbose = true;
            else
            if (streq (command, "$TERM"))
                self->terminated = true;
            else {
                //  Whatever came on the pipe is a new string
                free (self->sequence);
                self->sequence = command;
                self->seq_ptr = self->sequence;
                command = NULL;
                s_process_next (self);
            }
            zstr_free (&command);
            zmsg_destroy (&request);
        }
        else
            s_process_next (self);
    }
    glar_lamp_destroy (&self);
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
    zstr_send (glar_lamp, "SOS*");
    zclock_sleep (2000);
    zstr_send (glar_lamp, "K");
    zactor_destroy (&glar_lamp);
    //  @end

    printf ("OK\n");
}
