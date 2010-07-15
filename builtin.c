#include "scheme.h"

#define FLOAT_OF(x) (type_of(x) == FLOAT ? x.c->data.f : (double)UNTAG(x))

scm_val fn_foldl_arith(scm_val args, scm_val env, scm_val hint) {
    scm_val v ;
    int     got_double = 0 ;
    double  rv = 0 ;

    switch (UNTAG(hint)) {
        case '/':
        case '%':
        case '-':
            rv = FLOAT_OF(CAR(args)) ;
            args = CDR(args) ;
            break ;
        case '*':
            rv = 1 ;
            break ;
    }

    FOREACH(v, args) {
        double x = FLOAT_OF(CAR(v)) ;
        switch (UNTAG(hint)) {
            case '+': rv += x ; break ;
            case '-': rv -= x ; break ;
            case '*': rv *= x ; break ;
            case '/': rv /= x ; break ;
            case '%': rv = (long)rv % (long)x ; break ;
            default:
                die("unknown op '%c'\n", UNTAG(hint)) ;
        }
        got_double |= (type_of(CAR(v)) == FLOAT) ;
    }
    return got_double ? make_float(rv) : MKTAG((long)rv, FIXNUM) ;
}

scm_val     fn_reverse_bang(scm_val args, scm_val env, scm_val hint) {
    scm_val v = NIL, tmp ;

    while (!NULL_P(args)) {
        tmp = args ;
        args = CDR(args) ;
        CDR(tmp) = v ;
        v = tmp ;
    }
    return v ;
}

#define DEF_PROC(name, func, hint)  \
    env_define(env, intern(name), make_builtin(0, func, MKTAG(hint, CHAR)))

void        define_toplevels(scm_val env) {
    DEF_PROC("+", fn_foldl_arith, '+') ;
    DEF_PROC("-", fn_foldl_arith, '-') ;
    DEF_PROC("*", fn_foldl_arith, '*') ;
    DEF_PROC("/", fn_foldl_arith, '/') ;
    DEF_PROC("reverse!", fn_reverse_bang, 0) ;
}

void        builtin_tests(void) {
    scm_val e = env_create(NIL) ;
    scm_val v ;
    printf("\n;; --- BUILTIN TESTS --- ;;\n") ;

    v = cons(MKTAG(3, FIXNUM), NIL) ;
    SCM_DEBUG(fn_foldl_arith(v, e, MKTAG('+', CHAR)), "(+ 6)") ;
    v = cons(MKTAG(6, FIXNUM), v) ;
    SCM_DEBUG(fn_foldl_arith(v, e, MKTAG('%', CHAR)), "(% 6 3)") ;
    SCM_DEBUG(fn_foldl_arith(v, e, MKTAG('-', CHAR)), "(- 6 3)") ;
    SCM_DEBUG(fn_foldl_arith(v, e, MKTAG('/', CHAR)), "(/ 6 3)") ;
    v = cons(MKTAG(9, FIXNUM), v) ;
    SCM_DEBUG(fn_foldl_arith(v, e, MKTAG('*', CHAR)), "(* 9 6 3)") ;
    v = fn_reverse_bang(v, e, NIL) ;
    SCM_DEBUG(v, "(reverse! '(9 6 3))") ;
    SCM_DEBUG(v = fn_reverse_bang(CDR(v), e, NIL), "(reverse! '(6 9))") ;
    SCM_DEBUG(v = fn_reverse_bang(CDR(v), e, NIL), "(reverse! '(6))") ;
    SCM_DEBUG(fn_reverse_bang(NIL, e, NIL), "(reverse! '())") ;
}
