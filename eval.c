#include "scheme.h"
struct evaluator    *scm_create_evaluator(scm_val code) {
    struct evaluator *ev = malloc(sizeof(*ev)) ;
    ASSERT(ev) ;
    ev->s = NIL ;
    ev->c = code ;
    ev->d = FALSE ;
    ev->e = env_create(NIL) ;
    return ev ;
}

void                scm_destroy_evaluator(struct evaluator *scm) { free(scm) ; }

scm_val             scm_eval(struct evaluator *scm) {
    for (;;) {
        switch (type_of(scm->c)) {
            case FIXNUM: case CHAR: case BOOL: case FLOAT: case STRING:
                scm->d = scm->c ;
                break ;
            case SYMBOL:
                scm->d = env_get(scm->e, scm->c) ;
                break ;
            case CONS:
                /* push (rest-of-callseq . complete-list */
                scm->s = cons(cons(CDR(scm->c), NIL), scm->s) ;
                /* queue up first-of-callseq */
                scm->c = CAR(scm->c) ;
                continue ;

            default:
                scm->d = cons(intern("error-not-implemented"), scm->c) ;
                return FALSE ;
        }

        if (NULL_P(scm->s)) return TRUE ;
        
        /* there was a cons */
        if (NULL_P(CDAR(scm->s)) && SPECIAL_P(scm->d)) {
            /* syntax detected */
            SCM_DEBUG(cons(scm->d, CAAR(scm->s)), "special") ;
            die("syntax NI\n") ;
        }

        CDAR(scm->s) = cons(scm->d, CDAR(scm->s)) ;

        if (NULL_P(CAAR(scm->s))) {
            /* callseq evaluated */
            SCM_DEBUG(CDAR(scm->s), "callseq") ;
            die("apply NI\n") ;
        }

        scm->c = CAAAR(scm->s) ;
        CAAR(scm->s) = CDAAR(scm->s) ;
    }
}
