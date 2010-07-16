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

/* unique special values garanteed not to be coming from elsewhere */
#define S_APPLY     MKTAG(13, SPECIAL)
#define S_EVAL      MKTAG(14, SPECIAL)

#define PUSH(x)     scm->s = cons(cons(x, CAR(scm->s)), CDR(scm->s))

void                scm_push(struct evaluator *scm,
        scm_val s, scm_val e, scm_val c) {
    scm->d = cons(cons(scm->s, cons(scm->e, scm->c)), scm->d) ;
    scm->c = c ;
    scm->e = e ;
    scm->s = cons(s, NIL) ;
}

void                scm_pop(struct evaluator *scm) {
    scm_val     os = CAAR(scm->s) ;
    scm->s = CAAR(scm->d) ;
    PUSH(os) ;
    scm->e = CADAR(scm->d) ;
    scm->c = CDDAR(scm->d) ;
    scm->d = CDR(scm->d) ;
}

void        scm_invoke(struct evaluator *scm, scm_val c) {
    scm_val code = CAR(c), args = CDR(c) ;

    scm_val apply = cons(S_APPLY, scm->c) ;
    scm_val stack = NIL ;

    if (type_of(code) == SYMBOL) {
        code = env_get(scm->e, code) ;
        if (SYNTAX_P(code)) {
            if (code.c->flags & FL_EVAL)
                CDR(apply) = cons(S_EVAL, scm->c) ;
            stack = cons(code, args) ;
            c = NIL ;
        }
    }

    if (!NULL_P(c)) apply = reverse_append(c, apply) ;

    scm->s = cons(stack, scm->s) ;
    scm->c = apply ;
}

scm_val         scm_apply(struct evaluator *scm) {
    scm_val proc = CAAR(scm->s), args = CDAR(scm->s) ;
    scm->s = CDR(scm->s) ;

    ASSERT(type_of(proc) == PROCEDURE) ;
    if (proc.c->flags & FL_BUILTIN) {
        scm_val (*f)() = CAR(proc).p ;
        return f(args, scm->e, CDR(proc)) ;
    } else {
        scm_val e = env_bind_formals(CDR(proc), CAAR(proc), args) ;

        if (NULL_P(scm->c) && !NULL_P(CDR(scm->e))) {   /* tail call */
            scm->e = e ;
            scm->c = CDAR(proc) ;
        } else
            scm_push(scm, NIL, e, CDAR(proc)) ;

        return S_EVAL ;
    }
}

scm_val             scm_eval(struct evaluator *scm, scm_val code) {
    scm->s = cons(NIL, NIL) ;
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
                    scm->c = cons(CAAR(scm->s), scm->c) ;
                    scm->s = cons(CDAR(scm->s), CDR(scm->s)) ;
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

    return CAAR(scm->s) ;
}

static scm_val run_file(const char *fname) {
    FILE    *fp ;
    struct scm_scanner *scan ;
    struct evaluator *scm ;
    scm_val v ;

    ASSERT(fp = fopen(fname, "r")) ;
    scan = scm_create_scanner(fp) ;
    scm = scm_create_evaluator() ;
    v = scm_eval(scm, scm_read(scan, NIL)) ;
    scm_destroy_scanner(scan) ;
    return v ;
}

void        eval_tests(void) {

    printf("\n;; --- EVAL TESTS --- ;;\n") ;

    SCM_DEBUG(run_file("tests/fact.scm"),
            "factorial of 10 using y-combinator") ;
    SCM_DEBUG(run_file("tests/fact-tail.scm"),
            "tail-recursive factorial of 10 using y-combinator") ;
    fflush(stdout) ;
}
