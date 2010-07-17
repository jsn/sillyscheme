#include "scheme.h"
#include <string.h>
#include <stdarg.h>

#define MAX_SYMBOL  (1 << 10)

size_t  nsyms ;
char    *syms[MAX_SYMBOL] ;

void        die(const char *fmt, ...) { /* i don't know where else to put it */
    va_list vl ;
    va_start(vl, fmt) ;
    vfprintf(stderr, fmt, vl) ;
    va_end(vl) ;
    exit(-2) ;
}

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

scm_val     assq(scm_val alist, scm_val key) {
    scm_val v ;

    FOREACH(v, alist) {
        ENSURE(LIST_P(v), "assq: not a list\n") ;
        ENSURE(PAIR_P(CAR(v)), "assq: not a pair\n") ;
        if (EQ_P(CAAR(v), key)) return CAR(v) ;
    }
    return FALSE ;
}

int         type_of(scm_val v) {
    return TAG(v) == 0 ?
        (NULL_P(v) ? NONE : v.c->type) : TAG(v) ;
}

scm_val     cons(scm_val car, scm_val cdr) {
    scm_val v ;
    ENSURE(v.c = malloc(sizeof(struct cell)), "malloc()") ;
    v.c->type = CONS ;
    CAR(v) = car ;
    CDR(v) = cdr ;
    return v ;
}

