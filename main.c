#include "scheme.h"

int main (int ac, char const* av[]) {
    struct scm_scanner *scanner = scm_create_scanner(stdin) ;
    scm_val toplevel = cons(NIL, NIL) ;
    env_define(toplevel, intern("a"), cons(MKTAG(1, FIXNUM), MKTAG(2, FIXNUM))) ;

    for (;;) {
        printf("\n> ") ;
        fflush(stdout) ;
        scm_val v = scm_read(scanner, NIL) ;
        if (EQ_P(v, SCM_EOF)) break ;
        scm_print(v, stdout) ;
    }
    scm_destroy_scanner(scanner) ;
    fflush(stdout) ;
    return 0;
}
