#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <stdlib.h>
#include <stdio.h>

enum sc_types {
    NONE, FIXNUM, FLOAT, CHAR, STRING, SYMBOL, SPECIAL
} ;

struct sc_atom {
} ;

void        scm_read(const char *fname) ;
void        die(const char *fmt, ...) ;

#define ASSERT(x) if (!(x)) die("failed: %s, %d\n", __FILE__, __LINE__)
#define ENSURE(x, msg) if (!(x)) die(msg)

#endif
