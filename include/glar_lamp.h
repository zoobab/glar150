/*  =========================================================================
    glar_lamp - LED lamp controller

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Glar150 Project.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef GLAR_LAMP_H_INCLUDED
#define GLAR_LAMP_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


//  @interface
//  Create new glar_lamp actor instance.
//  @TODO: Describe the purpose of this actor!
//
//      zactor_t *glar_lamp = zactor_new (glar_lamp, NULL);
//
//  Destroy glar_lamp instance.
//
//      zactor_destroy (&glar_lamp);
//
//  Enable verbose logging of commands and activity:
//
//      zstr_send (glar_lamp, "VERBOSE");
//      zsock_wait (glar_lamp);
//
//  This is the glar_lamp constructor as a zactor_fn;
GLAR_EXPORT void
    glar_lamp_actor (zsock_t *pipe, void *args);

//  Self test of this actor
GLAR_EXPORT void
    glar_lamp_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
