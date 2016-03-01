/*  =========================================================================
    blink_led - Control server LEDs

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Blink Project.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    blink_led - Control server LEDs
@discuss
@end
*/

#include "blink_classes.h"

#define DEVICE_PATH  "/sys/devices/platform/leds-gpio/leds/gl_ar150:%s/brightness"

//  Structure of our class

struct _blink_led_t {
    char *device;               //  Path to LED device
};


//  --------------------------------------------------------------------------
//  Create a new LED instance. The index matches the LEDs from left to right.

blink_led_t *
blink_led_new (int index)
{
    assert (index > 0 && index <= 3);
    blink_led_t *self = (blink_led_t *) zmalloc (sizeof (blink_led_t));
    assert (self);
    char *name [] = { "", "wlan", "lan", "wan" };
    self->device = zsys_sprintf (DEVICE_PATH, name [index]);
    //  Start with LED off
    blink_led_off (self);
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the blink_led

void
blink_led_destroy (blink_led_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        blink_led_t *self = *self_p;
        free (self->device);
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Switch the LED on; returns 0 if successful, else -1.

int
blink_led_on (blink_led_t *self)
{
    assert (self);
    assert (self->device);

    int handle = open (self->device, O_WRONLY);
    if (handle == -1 || write (handle, "1", 1) != 1) {
        zsys_error ("could not write to %s", self->device);
        return -1;
    }
    close (handle);
    return 0;
}


//  --------------------------------------------------------------------------
//  Switch the LED off; returns 0 if successful, else -1.

int
blink_led_off (blink_led_t *self)
{
    assert (self);
    assert (self->device);

    int handle = open (self->device, O_WRONLY);
    if (handle == -1 || write (handle, "0", 1) != 1) {
        zsys_error ("could not write to %s", self->device);
        return -1;
    }
    close (handle);
    return 0;
}


//  --------------------------------------------------------------------------
//  Self test of this class

void
blink_led_test (bool verbose)
{
    printf (" * blink_led: ");

    //  @selftest
    blink_led_t *self = blink_led_new (1);
    assert (self);
    int cycles = 10;
    while (cycles) {
        blink_led_on (self);
        zclock_sleep (100);
        blink_led_off (self);
        zclock_sleep (100);
    }
    blink_led_destroy (&self);
    //  @end
    printf ("OK\n");
}
