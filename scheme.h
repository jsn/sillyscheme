#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <stdlib.h>
#include <stdio.h>

enum scm_types { NONE, FIXNUM, CHAR, SYMBOL, FLOAT, STRING, SPECIAL } ;

union _scm_val {
    long     l ;
    void    *p ;
} ;

typedef union _scm_val scm_val ;

struct scm_scanner ;

scm_val     scm_read(struct scm_scanner *sc) ;
struct scm_scanner *scm_create_scanner(FILE *fp) ;
void        scm_print(scm_val v, FILE *fp) ;
void        die(const char *fmt, ...) ;

#define ASSERT(x) if (!(x)) die("failed: %s, %d\n", __FILE__, __LINE__)
#define ENSURE(x, msg) if (!(x)) die(msg)

#endif
