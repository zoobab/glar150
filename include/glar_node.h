/*  =========================================================================
    glar_node - Glar150 service

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Glar150 Project.                          
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef GLAR_NODE_H_INCLUDED
#define GLAR_NODE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Create a new glar_node
GLAR_EXPORT glar_node_t *
    glar_node_new (const char *iface, bool console, const char *node_name);

//  Destroy the glar_node
GLAR_EXPORT void
    glar_node_destroy (glar_node_t **self_p);

//  Set verbose on/off
GLAR_EXPORT void
    glar_node_set_verbose (glar_node_t *self, bool verbose);

//  Execute state machine until finished
GLAR_EXPORT void
    glar_node_execute (glar_node_t *self);

//  Self test of this class
GLAR_EXPORT void
    glar_node_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
