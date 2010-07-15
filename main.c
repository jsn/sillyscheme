#include "scheme.h"

int main (int ac, char const* av[]) {
    struct scm_scanner *scan = scm_create_scanner(stdin) ;
    struct evaluator *scm = scm_create_evaluator() ;

    scm_val last = intern("\%\%\%") ;

    env_define(scm->e, last, FALSE) ;

    for (;;) {
        scm_val v ;
        printf("\n> ") ;
        fflush(stdout) ;
        v = scm_read(scan, NIL) ;
        if (EQ_P(v, SCM_EOF)) break ;
        v = scm_eval(scm, v) ;
        env_set(scm->e, last, v) ;
        scm_print(v, stdout) ;
    }
    scm_destroy_scanner(scan) ;
    fflush(stdout) ;
    return 0;
}
