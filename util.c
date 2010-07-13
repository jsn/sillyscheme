#include "scheme.h"
#include <stdarg.h>

void        die(const char *fmt, ...) {
    va_list vl ;
    va_start(vl, fmt) ;
    vfprintf(stderr, fmt, vl) ;
    va_end(vl) ;
    exit(-2) ;
}

