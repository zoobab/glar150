/*  =========================================================================
    glar_panel - LED panel controller

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Glar150 Project.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef GLAR_PANEL_H_INCLUDED
#define GLAR_PANEL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


//  @interface
//  Create new glar_panel actor instance.
//  @TODO: Describe the purpose of this actor!
//
//      zactor_t *glar_panel = zactor_new (glar_panel, NULL);
//
//  Destroy glar_panel instance.
//
//      zactor_destroy (&glar_panel);
//
//  Enable verbose logging of commands and activity:
//
//      zstr_send (glar_panel, "VERBOSE");
//      zsock_wait (glar_panel);
//
//  This is the glar_panel constructor as a zactor_fn;
GLAR_EXPORT void
    glar_panel_actor (zsock_t *pipe, void *args);

//  Self test of this actor
GLAR_EXPORT void
    glar_panel_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
