#include "scheme.h"

struct evaluator    *scm_create_evaluator(void) {
    struct evaluator *scm = malloc(sizeof(*scm)) ;
    ASSERT(scm) ;
    scm->s = cons(NIL, NIL) ;
    scm->e = env_create(NIL) ;
    scm->c = NIL ;
    scm->d = NIL ;
    define_toplevels(scm->e) ;
    scm_load_file(scm, "prelude.scm") ;
    return scm ;
}

scm_val             scm_load_file(struct evaluator *scm, const char *fname) {
    struct scm_scanner *scan ;
    FILE *fp ;
    scm_val v = FALSE ;

    ENSURE(fp = fopen(fname, "r"), "scm_load_file: fopen()\n") ;
    scan = scm_create_scanner(fp) ;

    for (;;) {
        v = scm_read(scan, NIL) ;
        if (EQ_P(v, SCM_EOF)) break ;
        v = scm_eval(scm, v) ;
    }
    scm_destroy_scanner(scan) ;
    return v ;
}

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

scm_val scm_apply(scm_val args, struct evaluator *scm, scm_val hint) {
    scm_val proc = CAR(args), as = CDR(args) ;

    if (type_of(proc) != PROCEDURE) {
        SCM_DEBUG(proc, "not a procedure") ;
        die("") ;
    }

    if (proc.c->flags & FL_BUILTIN) {
        scm_val (*f)() = CAR(proc).p ;
        return f(as, scm, CDR(proc)) ;
    } else {
        scm_val e = env_bind_formals(CDR(proc), CAAR(proc), as) ;

        if (NULL_P(scm->c) && !NULL_P(CDR(scm->e))) {   /* tail call */
            scm->e = e ;
            scm->c = CDAR(proc) ;
        } else
            scm_push(scm, NIL, e, CDAR(proc)) ;

        return S_EVAL ;
    }
}

scm_val fn_eval(scm_val args, struct evaluator *scm, scm_val hint) {
    PUSH(CAR(args)) ;
    scm->c = cons(S_EVAL, scm->c) ;
    return S_EVAL ;
}

scm_val             scm_eval(struct evaluator *scm, scm_val code) {
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
                    scm_val args = CAR(scm->s) ;
                    scm->s = CDR(scm->s) ;

                    c = scm_apply(args, scm, NIL) ;

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

    code = CAAR(scm->s) ;
    scm->s = cons(CDAR(scm->s), CDR(scm->s)) ;
    return code ;
}

static scm_val run_file(const char *fname) {
    FILE    *fp ;
    struct scm_scanner *scan ;
    struct evaluator *scm ;
    scm_val v = NIL ;

    ASSERT(fp = fopen(fname, "r")) ;
    scan = scm_create_scanner(fp) ;
    scm = scm_create_evaluator() ;
    while (!EQ_P(v, SCM_EOF))
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
    SCM_DEBUG(run_file("tests/fact-named.scm"), "named factorial of 10") ;
    SCM_DEBUG(run_file("tests/misc.scm"), "misc tests")
    fflush(stdout) ;
}
