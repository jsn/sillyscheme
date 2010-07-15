#include "scheme.h"

struct evaluator    *scm_create_evaluator(scm_val code) {
    struct evaluator *ev = malloc(sizeof(*ev)) ;
    ASSERT(ev) ;
    ev->s = NIL ;
    ev->c = code ;
    ev->d = FALSE ;
    ev->e = env_create(NIL) ;
    define_toplevels(ev->e) ;
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


        for (;;) { /* unwind */
            if (NULL_P(scm->s)) return TRUE ;

            /* there was a cons */

            if (NULL_P(CDAR(scm->s)) && SYNTAX_P(scm->d)) {
                /* syntax detected */
                scm_val code = scm->d, args = CAAR(scm->s) ;
                scm->s = CDR(scm->s) ;

                if (code.c->flags & FL_BUILTIN) {
                    scm_val (*f)() = CAR(code).p ;
                    scm->d = f(args, scm->e, CDR(code)) ;
                    continue ;
                } else {
                    SCM_DEBUG(cons(code, args), "syntax") ;
                    die("syntax non-builtin NI\n") ;
                }
                continue ;
            }

            CDAR(scm->s) = cons(scm->d, CDAR(scm->s)) ;

            if (NULL_P(CAAR(scm->s))) {
                /* callseq evaluated */
                scm_val callseq = reverse_bang(CDAR(scm->s)) ;
                scm_val code = CAR(callseq), args = CDR(callseq) ;
                scm->s = CDR(scm->s) ;

                ASSERT(type_of(code) == PROCEDURE) ;
                if (code.c->flags & FL_BUILTIN) {
                    scm_val (*f)() = CAR(code).p ;
                    scm->d = f(args, scm->e, CDR(code)) ;
                } else {
                    die("apply non-builtin NI\n") ;
                }
                continue ;
            }
            break ;
        }

        scm->c = CAAAR(scm->s) ;
        CAAR(scm->s) = CDAAR(scm->s) ;
    }
}
