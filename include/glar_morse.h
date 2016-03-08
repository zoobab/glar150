/*  =========================================================================
    glar_morse - Morse lamp controller

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Glar150 Project.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef GLAR_MORSE_H_INCLUDED
#define GLAR_MORSE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


//  @interface
//  Create new glar_morse actor instance.
//  @TODO: Describe the purpose of this actor!
//
//      zactor_t *glar_morse = zactor_new (glar_morse, NULL);
//
//  Destroy glar_morse instance.
//
//      zactor_destroy (&glar_morse);
//
//  Enable verbose logging of commands and activity:
//
//      zstr_send (glar_morse, "VERBOSE");
//      zsock_wait (glar_morse);
//
//  This is the glar_morse constructor as a zactor_fn;
GLAR_EXPORT void
    glar_morse_actor (zsock_t *pipe, void *args);

//  Self test of this actor
GLAR_EXPORT void
    glar_morse_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
