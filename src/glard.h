/*  =========================================================================
    glard - No title

    =========================================================================
*/

#ifndef GLARD_H_INCLUDED
#define GLARD_H_INCLUDED

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _glard_t glard_t;

//  @interface
//  Create a new glard, return the reference if successful, or NULL
//  if construction failed due to lack of available memory.
glard_t *
    glard_new (void);

//  Destroy the glard and free all memory used by the object.
void
    glard_destroy (glard_t **self_p);

//  Enable verbose tracing
void
    glard_verbose (zs_parser_t *self, bool verbose);

//  Self test of this class
void
    glard_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
