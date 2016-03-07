/*  =========================================================================
    glar_panel - No title

    =========================================================================
*/

#ifndef GLAR_PANEL_H_INCLUDED
#define GLAR_PANEL_H_INCLUDED

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _glar_panel_t glar_panel_t;

//  @interface
//  Create a new glar_panel, return the reference if successful, or NULL
//  if construction failed due to lack of available memory.
glar_panel_t *
    glar_panel_new (void);

//  Destroy the glar_panel and free all memory used by the object.
void
    glar_panel_destroy (glar_panel_t **self_p);

//  Enable verbose tracing
void
    glar_panel_verbose (zs_parser_t *self, bool verbose);

//  Self test of this class
void
    glar_panel_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
