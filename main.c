#include "scheme.h"

int main (int ac, char const* av[]) {
    struct scm_scanner *scanner = scm_create_scanner(stdin) ;

    for (;;) {
        printf("\n> ") ;
        fflush(stdout) ;
        scm_val v = scm_read(scanner, NIL) ;
        scm_print(v, stdout) ;
        if (EQ_P(v, SCM_EOF)) break ;
        // scm_print(assq(v, MKTAG(4, FIXNUM)), stdout) ;
    }
    fflush(stdout) ;
    return 0;
}
