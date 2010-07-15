#include "scheme.h"

int main (int ac, char const* av[]) {
    struct scm_scanner *scan = scm_create_scanner(stdin) ;
    struct evaluator *scm = scm_create_evaluator(NIL) ;

    scm_val last = intern("\%\%\%") ;

    env_define(scm->e, last, FALSE) ;

    for (;;) {
        printf("\n> ") ;
        fflush(stdout) ;
        scm->c = scm_read(scan, NIL) ;
        if (EQ_P(scm->c, SCM_EOF)) break ;
        if (EQ_P(scm_eval(scm), FALSE))
            printf("error: ") ;
        else
            env_set(scm->e, last, scm->d) ;
        scm_print(scm->d, stdout) ;
    }
    scm_destroy_scanner(scan) ;
    scm_destroy_evaluator(scm) ;
    fflush(stdout) ;
    return 0;
}
