/*  =========================================================================
    glar_morse - Morse lamp controller

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

struct _glar_morse_t {
    zsock_t *pipe;              //  Actor command pipe
    zpoller_t *poller;          //  Socket poller
    bool terminated;            //  Did caller ask us to quit?
    bool verbose;               //  Verbose logging enabled?
    char sequence [1024];       //  Current display sequence
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
//  Create a new glar_morse instance

static glar_morse_t *
glar_morse_new (zsock_t *pipe, void *args)
{
    glar_morse_t *self = (glar_morse_t *) zmalloc (sizeof (glar_morse_t));
    assert (self);
    self->pipe = pipe;
    self->poller = zpoller_new (self->pipe, NULL);
    self->timeout = -1;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the glar_morse instance

static void
glar_morse_destroy (glar_morse_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        glar_morse_t *self = *self_p;
        zpoller_destroy (&self->poller);
        free (self);
        *self_p = NULL;
    }
}


static void
s_set_morse (bool on)
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

//  Convert text string into command sequence
//  Where:
//      +   switch on
//      -   switch off
//      .   pause one Morse cycle
//      *   repeat sequence from start

static void
s_build_sequence (char *sequence, char *string)
{
    sequence [0] = 0;
    while (*string) {
        char letter = toupper (*string++);
        if (isalnum (letter)) {
            for (int index = 0; index < sizeof (morse_alphabet) / sizeof (char *); index++) {
                if (letter == morse_alphabet [index][0]) {
                    char *bip_ptr = morse_alphabet [index] + 1;
                    while (*bip_ptr) {
                        if (*bip_ptr == '.')
                            strcat (sequence, "+.-.");
                        else
                            strcat (sequence, "+...-.");
                        bip_ptr++;
                    }
                    strcat (sequence, "..");
                    break;
                }
            }
        }
        else
        if (letter == ' ')
            strcat (sequence, ".....");
        else
        if (letter == '*')
            strcat (sequence, "*");
        else
            zsys_error ("glar_morse: unknown command '%c'", letter);
    }
    zsys_info ("Sequence=%s", sequence);
    s_set_morse (false);
}

static void
s_show_sequence (glar_morse_t *self)
{
    if (!self->seq_ptr)
        return;

    while (*self->seq_ptr) {
        char command = *self->seq_ptr++;
        if (command == '+')
            s_set_morse (true);
        else
        if (command == '-')
            s_set_morse (false);
        else
        if (command == '*')
            self->seq_ptr = self->sequence;
        else
        if (command == '.')
            break;
    }
}


//  --------------------------------------------------------------------------
//  This is the actor which runs in its own thread.

void
glar_morse_actor (zsock_t *pipe, void *args)
{
    glar_morse_t *self = glar_morse_new (pipe, args);
    assert (self);
    zsock_signal (self->pipe, 0);       //  Tell caller we're ready

    //  Main loop, we wait on commands coming via our pipe
    while (!self->terminated) {
        zsock_t *which = (zsock_t *) zpoller_wait (self->poller, MORSE_PERIOD);
        if (which == self->pipe) {
            zmsg_t *request = zmsg_recv (self->pipe);
            if (!request)
                return;        //  Interrupted

            char *command = zmsg_popstr (request);
            if (self->verbose)
                zsys_info ("glar_morse: pipe command=%s", command);
            if (streq (command, "VERBOSE"))
                self->verbose = true;
            else
            if (streq (command, "$TERM"))
                self->terminated = true;
            else {
                //  Whatever came on the pipe is a new sequence
                s_build_sequence (self->sequence, command);
                self->seq_ptr = self->sequence;
            }
            zstr_free (&command);
            zmsg_destroy (&request);
        }
        s_show_sequence (self);
    }
    glar_morse_destroy (&self);
}


//  --------------------------------------------------------------------------
//  Self test of this actor.

void
glar_morse_test (bool verbose)
{
    printf (" * glar_morse: ");

    //  @selftest
    zactor_t *glar_morse = zactor_new (glar_morse_actor, NULL);
    if (verbose)
        zstr_send (glar_morse, "VERBOSE");
    zstr_send (glar_morse, "SOS*");
    zclock_sleep (2000);
    zstr_send (glar_morse, "K");
    zactor_destroy (&glar_morse);
    //  @end

    printf ("OK\n");
}
