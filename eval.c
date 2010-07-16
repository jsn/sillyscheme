#include "scheme.h"

struct evaluator    *scm_create_evaluator(void) {
    struct evaluator *scm = malloc(sizeof(*scm)) ;
    ASSERT(scm) ;
    scm->s = NIL ;
    scm->e = env_create(NIL) ;
    scm->c = NIL ;
    scm->d = NIL ;
    define_toplevels(scm->e) ;
    return scm ;
}

#define PREV_S(scm) CAAR(scm->d)
#define PREV_E(scm) CADAR(scm->d)
#define PREV_C(scm) CDDAR(scm->d)
#define PREV_D(scm) CDR(scm->d)

/* unique special values garanteed not to be coming from elsewhere */
#define S_APPLY     MKTAG(13, SPECIAL)
#define S_EVAL      MKTAG(14, SPECIAL)

#define PUSH(x)     scm->s = cons(x, scm->s)

void                scm_push(struct evaluator *scm,
        scm_val s, scm_val e, scm_val c) {

    if (!NULL_P(scm->c))
        scm->d = cons(cons(scm->s, cons(scm->e, scm->c)), scm->d) ;

    scm->c = c ;
    scm->e = e ;
    scm->s = s ;
}

void                scm_pop(struct evaluator *scm) {
    scm->s = cons(CAR(scm->s), PREV_S(scm)) ;
    scm->e = PREV_E(scm) ;
    scm->c = PREV_C(scm) ;
    scm->d = PREV_D(scm) ;
}

void        scm_invoke(struct evaluator *scm, scm_val c) {
    scm_val code = CAR(c), args = CDR(c) ;

    scm_val apply = cons(S_APPLY, NIL) ;
    scm_val stack = NIL ;

    if (type_of(code) == SYMBOL) {
        code = env_get(scm->e, code) ;
        if (SYNTAX_P(code)) {
            if (code.c->flags & FL_EVAL) CDR(apply) = cons(S_EVAL, NIL) ;
            stack = cons(code, args) ;
            c = NIL ;
        }
    }

    if (!NULL_P(c)) apply = reverse_append(c, apply) ;

    scm_push(scm, stack, scm->e, apply) ;
}

scm_val         scm_apply(struct evaluator *scm) {
    scm_val proc = CAR(scm->s), args = CDR(scm->s) ;

    scm->s = NIL ;
    ASSERT(type_of(proc) == PROCEDURE) ;
    if (proc.c->flags & FL_BUILTIN) {
        scm_val (*f)() = CAR(proc).p ;
        return f(args, scm->e, CDR(proc)) ;
    } else {
        scm_push(scm, NIL, env_bind_formals(CDR(proc), CAAR(proc), args),
                CDAR(proc)) ;
        return S_EVAL ;
    }
}

scm_val             scm_eval(struct evaluator *scm, scm_val code) {
    scm->s = NIL ;
    scm->c = cons(code, scm->c) ;

    while (PAIR_P(scm->c) || PAIR_P(scm->d)) {
        scm_val c ;

        if (NULL_P(scm->c)) {
            scm_pop(scm) ;
            continue ;
        }

        c = CAR(scm->c) ;
        scm->c = CDR(scm->c) ;

        switch (type_of(c)) {
            case FIXNUM: case CHAR: case BOOL: case FLOAT: case STRING:
                break ;
            case SYMBOL: c = env_get(scm->e, c) ; break ;
            case CONS:   scm_invoke(scm, c) ;     continue ;

            case SPECIAL:
                if (EQ_P(c, S_APPLY)) {
                    c = scm_apply(scm) ;
                    if (EQ_P(c, S_EVAL)) continue ;
                } else if (EQ_P(c, S_EVAL)) {
                    scm->c = cons(CAR(scm->s), scm->c) ;
                    scm->s = CDR(scm->s) ;
                    continue ;
                } else
                    die("unknown special %d\n", UNTAG(c)) ;
                break ;
            default:
                c = cons(intern("error"), c) ;
                break ;
        }
        PUSH(c) ;
    }

    return CAR(scm->s) ;
}

void        eval_tests(void) {
    FILE    *fp ;
    struct scm_scanner *scan ;
    struct evaluator *scm ;

    printf("\n;; --- EVAL TESTS --- ;;\n") ;

    ASSERT(fp = fopen("tests/fact.scm", "r")) ;
    scan = scm_create_scanner(fp) ;
    scm = scm_create_evaluator() ;
    SCM_DEBUG(scm_eval(scm, scm_read(scan, NIL)),
            "factorial of 5 using y-combinator") ;
    scm_destroy_scanner(scan) ;
    fflush(stdout) ;
}
