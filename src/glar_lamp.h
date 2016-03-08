/*  =========================================================================
    glar_lamp - No title

    =========================================================================
*/

#ifndef GLAR_LAMP_H_INCLUDED
#define GLAR_LAMP_H_INCLUDED

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _glar_lamp_t glar_lamp_t;

//  @interface
//  Create a new glar_lamp, return the reference if successful, or NULL
//  if construction failed due to lack of available memory.
glar_lamp_t *
    glar_lamp_new (void);

//  Destroy the glar_lamp and free all memory used by the object.
void
    glar_lamp_destroy (glar_lamp_t **self_p);

//  Enable verbose tracing
void
    glar_lamp_verbose (zs_parser_t *self, bool verbose);

//  Self test of this class
void
    glar_lamp_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
