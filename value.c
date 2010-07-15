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
    return MKTAG(nsyms ++, SYMBOL) ;
}

const char  *sym_to_string(scm_val v) {
    int     i = UNTAG(v) ;
    ASSERT(TAG(v) == SYMBOL) ;
    ASSERT(i >= 0 && i < nsyms) ;
    return syms[i] ;
}

scm_val mkcell(int type) {
    scm_val v ;
    ENSURE(v.c = malloc(sizeof(struct cell)), "malloc()") ;
    v.c->type = type ;
    return v ;
}

int         type_of(scm_val v) {
    return TAG(v) == 0 ?
        (NULL_P(v) ? NONE : v.c->type) : TAG(v) ;
}

scm_val     pair_p(scm_val v) {
    return NULL_P(v) ? FALSE : list_p(v) ;
}

scm_val     list_p(scm_val v) {
    if (EQ_P(v, NIL)) return TRUE ;
    if (TAG(v)) return FALSE ;
    return v.c->type == CONS ? TRUE : FALSE ;
}

scm_val     cons(scm_val car, scm_val cdr) {
    scm_val v = mkcell(CONS) ;
    CAR(v) = car ;
    CDR(v) = cdr ;
    return v ;
}

scm_val     make_builtin(
        int syntax, scm_val (*f)(scm_val args, scm_val env, scm_val hint),
        scm_val hint) {
    scm_val v = mkcell(PROCEDURE) ;
    v.c->flags |= FL_BUILTIN ;
    CAR(v).p = f ;
    CDR(v) = hint ;
    return v ;
}

scm_val     make_float(double x) {
    scm_val v = mkcell(FLOAT) ;
    v.c->data.f = x ;
    return v ;
}
