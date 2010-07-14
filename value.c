#include "scheme.h"
#include <string.h>

#define MAX_SYMBOL  (1 << 10)

size_t  nsyms ;
char    *syms[MAX_SYMBOL] ;

scm_val intern(const char *s) {
    size_t  i ;

    for (i = 0; i < nsyms; i ++)
        if (strcmp(syms[i], s) == 0) return MKTAG(i, SYMBOL) ;
    ENSURE(syms[nsyms] = strdup(s), "strdup()") ;
    ASSERT(nsyms < MAX_SYMBOL) ;
    nsyms ++ ;
    return MKTAG(nsyms - 1, SYMBOL) ;
}

const char  *sym_to_string(scm_val v) {
    int     i = UNTAG(v) ;
    ASSERT(TAG(v) == SYMBOL) ;
    ASSERT(i >= 0 && i < nsyms) ;
    return syms[i] ;
}

struct cell *mkcell(int type) {
    struct cell *c = malloc(sizeof(*c)) ;
    ENSURE(c, "malloc()") ;
    c->type = type ;
    return c ;
}

int         type_of(scm_val v) {
    return TAG(v) == 0 ?
        (NULL_P(v) ? NONE : ((struct cell *)v.p)->type) : TAG(v) ;
}

scm_val     pair_p(scm_val v) {
    return NULL_P(v) ? FALSE : list_p(v) ;
}

scm_val     list_p(scm_val v) {
    if (EQ_P(v, NIL)) return TRUE ;
    if (TAG(v)) return FALSE ;
    return ((struct cell *)v.p)->type == CONS ? TRUE : FALSE ;
}

scm_val     cons(scm_val car, scm_val cdr) {
    struct cell *c = mkcell(CONS) ;
    c->data.cons.car = car ;
    c->data.cons.cdr = cdr ;
    return (scm_val)((void *)c) ;
}

