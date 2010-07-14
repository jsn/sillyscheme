#include "scheme.h"
#include <string.h>

#define MAX_SYMBOL  (1 << 10)

size_t  nsyms ;
char    *syms[MAX_SYMBOL] ;

char    *intern(const char *s) {
    size_t  i ;

    for (i = 0; i < nsyms; i ++) if (strcmp(syms[i], s) == 0) return syms[i] ;
    ENSURE(syms[nsyms] = strdup(s), "strdup()") ;
    ASSERT(nsyms < MAX_SYMBOL) ;
    return syms[nsyms ++] ;
}

struct cell *mkcell(int type) {
    struct cell *c = malloc(sizeof(*c)) ;
    ENSURE(c, "malloc()") ;
    c->type = type ;
    return c ;
}

scm_val     list_p(scm_val v) {
    if (EQ_P(v, NIL)) return TRUE ;
    if (v.l & 3) return FALSE ;
    return ((struct cell *)v.p)->type == CONS ? TRUE : FALSE ;
}

scm_val     cons(scm_val car, scm_val cdr) {
    struct cell *c = mkcell(CONS) ;
    c->data.cons.car = car ;
    c->data.cons.cdr = cdr ;
    return (scm_val)((void *)c) ;
}

