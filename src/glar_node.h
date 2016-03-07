/*  =========================================================================
    glar_node - No title

    =========================================================================
*/

#ifndef GLAR_NODE_H_INCLUDED
#define GLAR_NODE_H_INCLUDED

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _glar_node_t glar_node_t;

//  @interface
//  Create a new glar_node, return the reference if successful, or NULL
//  if construction failed due to lack of available memory.
glar_node_t *
    glar_node_new (void);

//  Destroy the glar_node and free all memory used by the object.
void
    glar_node_destroy (glar_node_t **self_p);

//  Enable verbose tracing
void
    glar_node_verbose (zs_parser_t *self, bool verbose);

//  Self test of this class
void
    glar_node_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
