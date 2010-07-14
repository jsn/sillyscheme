#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <stdlib.h>
#include <stdio.h>

enum scm_types {                        /* i-something */
    NONE, FIXNUM, CHAR, BOOL, SYMBOL,   /* immediate */ 
    CONS, FLOAT, STRING,                /* indirection */
    SPECIAL,                            /* immaterial */
} ;

#define TAGBITS     3
#define TAG(x)      ((x).l & 7)
#define MKTAG(l, t) ((scm_val)(((long)(l) << TAGBITS) | t))
#define UNTAG(x)    ((x).l >> TAGBITS)

union _scm_val {
    long     l ;
    void    *p ;
} ;

#define NIL ((scm_val)0L)
#define NULL_P(x) (x.l == 0)

#define EQ_P(x, y) (x.l == y.l)

#define FALSE   MKTAG(0, BOOL)
#define TRUE    MKTAG(1, BOOL)
#define SCM_EOF MKTAG(-1, CHAR)

typedef union _scm_val scm_val ;

struct cell {
    short type, flags ;
    union {
        struct { scm_val car, cdr ; } cons ;
        double f ;
    } data ;
};

struct scm_scanner ;

struct      scm_scanner *scm_create_scanner(FILE *fp) ;
void        scm_destroy_scanner(struct scm_scanner *sc) ;

scm_val     scm_read(struct scm_scanner *sc, scm_val list) ;
void        scm_print(scm_val v, FILE *fp) ;

scm_val     intern(const char *s) ;
const char  *sym_to_string(scm_val v) ;
struct cell *mkcell(int type) ;
int         type_of(scm_val v) ;

scm_val     list_p(scm_val v) ;
scm_val     pair_p(scm_val v) ;
scm_val     cons(scm_val car, scm_val cdr) ;
scm_val     assq(scm_val alist, scm_val key) ;

#define     PAIR_P(v)   (type_of(v) == CONS)
#define     LIST_P(v)   (type_of(v) == NONE || PAIR_P(v))

#define     CAR(v)  ((struct cell *)(v).p)->data.cons.car
#define     CDR(v)  ((struct cell *)(v).p)->data.cons.cdr
#define     CAAR(v) CAR(CAR(v))
#define     CADR(v) CDR(CAR(v))

scm_val     env_create(scm_val parent) ;
scm_val     env_get_pair(scm_val env, scm_val key, int force, int up) ;
scm_val     env_get(scm_val env, scm_val key) ;
#define env_define(env, key, val) CDR(env_get_pair(env, key, 1, 0)) = val
#define env_set(env, key, val) CDR(env_get_pair(env, key, 1, 1)) = val

void        die(const char *fmt, ...) ;

#define ASSERT(x) if (!(x)) die("failed: %s, %d\n", __FILE__, __LINE__)
#define ENSURE(x, msg) if (!(x)) die(msg)

#define SCM_DEBUG(x, msg) { \
    printf("%s: ", msg) ;   \
    scm_print(x, stdout) ;  \
    printf("\n") ;          \
}

void        env_tests(void) ;
void        assoc_tests(void) ;
void        parse_tests(void) ;

#endif
