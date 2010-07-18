#include "scheme.h"

#define MAX_CELLS   1000000

static struct cell  cells[MAX_CELLS] ;
static size_t       ncells ;

scm_val     scm_alloc_cell(int type) {
    scm_val v ;
    v.c = cells + ncells ++ ;
    v.c->type = type ;
    return v ;
}
