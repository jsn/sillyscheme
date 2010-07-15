#include "scheme.h"

#define FLOAT_OF(x) (type_of(x) == FLOAT ? x.c->data.f : (double)UNTAG(x))
#define DEFINE_FUNC(name)   \
    scm_val name(scm_val args, scm_val env, scm_val hint)

DEFINE_FUNC(fn_foldl_arith) {
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

scm_val     reverse_bang(scm_val args) {
    scm_val v = NIL, tmp ;

    while (!NULL_P(args)) {
        tmp = args ;
        args = CDR(args) ;
        CDR(tmp) = v ;
        v = tmp ;
    }
    return v ;
}

DEFINE_FUNC(fn_reverse_bang) { return reverse_bang(CAR(args)) ; }

DEFINE_FUNC(fn_car) { return CAR(CAR(args)) ; }
DEFINE_FUNC(fn_cdr) { return CDR(CAR(args)) ; }
DEFINE_FUNC(fn_caar) { return CAAR(CAR(args)) ; }
DEFINE_FUNC(fn_cadr) { return CADR(CAR(args)) ; }
DEFINE_FUNC(fn_cdar) { return CDAR(CAR(args)) ; }
DEFINE_FUNC(fn_cddr) { return CDDR(CAR(args)) ; }

DEFINE_FUNC(fn_cons) { return cons(CAR(args), CADR(args)) ; }
DEFINE_FUNC(fn_display) { scm_print(CAR(args), stdout) ; return FALSE ; }
DEFINE_FUNC(fn_newline) { printf("\n") ; return FALSE ; }

DEFINE_FUNC(syn_quote) { return CAR(args) ; }
DEFINE_FUNC(syn_define) {
    env_define(env, CAR(args), CADR(args)) ;
    return CADR(args) ;
}

DEFINE_FUNC(syn_lambda) {
    scm_val proc = mkcell(PROCEDURE) ;
    CAR(proc) = args ;
    CDR(proc) = env ;
    return proc ;
}

#define DEF_PROC_CHAR(name, func, hint)  \
    env_define(env, intern(name), make_builtin(0, func, MKTAG(hint, CHAR)))

#define DEF_PROC(name, func) \
    env_define(env, intern(name), make_builtin(0, func, NIL))

#define DEF_SYNTAX_CHAR(name, func, hint)  \
    env_define(env, intern(name), make_builtin(1, func, MKTAG(hint, CHAR)))

#define DEF_SYNTAX(name, func) \
    env_define(env, intern(name), make_builtin(1, func, NIL))

void        define_toplevels(scm_val env) {
    DEF_PROC_CHAR("+", fn_foldl_arith, '+') ;
    DEF_PROC_CHAR("-", fn_foldl_arith, '-') ;
    DEF_PROC_CHAR("*", fn_foldl_arith, '*') ;
    DEF_PROC_CHAR("/", fn_foldl_arith, '/') ;
    DEF_PROC("reverse!", fn_reverse_bang) ;
    DEF_PROC("car", fn_car) ;
    DEF_PROC("cdr", fn_cdr) ;
    DEF_PROC("caar", fn_caar) ;
    DEF_PROC("cadr", fn_cadr) ;
    DEF_PROC("cdar", fn_cdar) ;
    DEF_PROC("cddr", fn_cddr) ;
    DEF_PROC("cons", fn_cons) ;
    DEF_PROC("display", fn_display) ;
    DEF_PROC("newline", fn_newline) ;

    DEF_SYNTAX_CHAR("quote", syn_quote, '\'') ;
    DEF_SYNTAX_CHAR("pseudoquote", syn_quote, '`') ;
    DEF_SYNTAX("define", syn_define) ;
    DEF_SYNTAX("lambda", syn_lambda) ;
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
    v = fn_reverse_bang(cons(v, NIL), e, NIL) ;
    SCM_DEBUG(v, "(reverse! '(9 6 3))") ;
    v = fn_reverse_bang(cons(CDR(v), NIL), e, NIL) ;
    SCM_DEBUG(v, "(reverse! '(6 9))") ;
    v = fn_reverse_bang(cons(CDR(v), NIL), e, NIL) ;
    SCM_DEBUG(v, "(reverse! '(6))") ;
    SCM_DEBUG(fn_reverse_bang(cons(NIL, NIL), e, NIL), "(reverse! '())") ;
}
