#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <stdlib.h>
#include <stdio.h>

enum scm_types {
    NONE, FIXNUM, CHAR, BOOL,       /* immediate */
    FLOAT, STRING, SYMBOL, CONS,    /* indirection */
    SPECIAL                         /* immaterial */
} ;

union _scm_val {
    long     l ;
    void    *p ;
} ;

#define NIL ((scm_val)0L)
#define NULL_P(x) (x.l == 0)

#define EQ_P(x, y) (x.l == y.l)

#define FALSE   ((scm_val)(long)BOOL)
#define TRUE    ((scm_val)((1L << 2) | BOOL))
#define SCM_EOF ((scm_val)((-1L << 2) | CHAR))

typedef union _scm_val scm_val ;

struct cell {
    int type ;
    union {
        struct { scm_val car, cdr ; } cons ;
        double f ;
    } data ;
};

struct scm_scanner ;

scm_val     scm_read(struct scm_scanner *sc, scm_val list) ;
struct      scm_scanner *scm_create_scanner(FILE *fp) ;
void        scm_print(scm_val v, FILE *fp) ;
void        die(const char *fmt, ...) ;

#define ASSERT(x) if (!(x)) die("failed: %s, %d\n", __FILE__, __LINE__)
#define ENSURE(x, msg) if (!(x)) die(msg)

#endif
