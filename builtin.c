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

DEFINE_FUNC(fn_foldl_cmp) {
    scm_val v ;
    int  rv = 1 ;
    double x ;

    x = FLOAT_OF(CAR(args)) ;
    args = CDR(args) ;

    FOREACH(v, args) {
        double y = FLOAT_OF(CAR(v)) ;
        switch (UNTAG(hint)) {
            case '=': rv = rv && (x == y) ; break ;
            case '>': rv = rv && (x > y) ; break ;
            case 'g': rv = rv && (x >= y) ; break ;
            case '<': rv = rv && (x < y) ; break ;
            case 'l': rv = rv && (x <= y) ; break ;
            case '!': rv = rv && (x != y) ; break ;
            default:
                die("unknown op '%c'\n", UNTAG(hint)) ;
        }
        x = y ;
    }
    return rv ? TRUE : FALSE ;
}

DEFINE_FUNC(fn_reverse_bang) { return reverse_bang(CAR(args)) ; }

scm_val     reverse_append(scm_val args, scm_val head) {
    while (!NULL_P(args)) {
        head = cons(CAR(args), head) ;
        args = CDR(args) ;
    }
    return head ;
}

DEFINE_FUNC(fn_reverse) { return reverse_append(CAR(args), NIL) ; }

DEFINE_FUNC(fn_car) { return CAR(CAR(args)) ; }
DEFINE_FUNC(fn_cdr) { return CDR(CAR(args)) ; }
DEFINE_FUNC(fn_caar) { return CAAR(CAR(args)) ; }
DEFINE_FUNC(fn_cadr) { return CADR(CAR(args)) ; }
DEFINE_FUNC(fn_cdar) { return CDAR(CAR(args)) ; }
DEFINE_FUNC(fn_cddr) { return CDDR(CAR(args)) ; }

DEFINE_FUNC(fn_cons) { return cons(CAR(args), CADR(args)) ; }
DEFINE_FUNC(fn_display) { scm_print(CAR(args), stdout) ; return FALSE ; }
DEFINE_FUNC(fn_newline) { printf("\n") ; return FALSE ; }

DEFINE_FUNC(fn_if) {
    scm_val cond = CAR(args), then_ = CADR(args), else_ = CADDR(args) ;
    return EQ_P(cond, FALSE) ? else_ : then_ ;
}

DEFINE_FUNC(syn_quote) { return CAR(args) ; }
DEFINE_FUNC(syn_define) {
    env_define(env, CAR(args), CADR(args)) ;
    return CADR(args) ;
}

DEFINE_FUNC(syn_if) {
    scm_val cond = CAR(args), then_ = CADR(args), else_ = CADDR(args) ;
    /* ((_if cond (lambda () then_) (lambda () else_))) */
    then_ = cons(intern("lambda"), cons(NIL, cons(then_, NIL))) ;
    else_ = cons(intern("lambda"), cons(NIL, cons(else_, NIL))) ;
    cond  = cons(cons(intern("_if"), cons(cond, cons(then_, cons(else_, NIL)))), NIL) ;
    return cond ;
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
    env_define(env, intern(name), make_builtin(0, func, intern(name)))

#define DEF_SYNTAX_CHAR(name, flags, func, hint)  \
    env_define(env, intern(name), \
            make_builtin(FL_SYNTAX | flags, func, MKTAG(hint, CHAR)))

#define DEF_SYNTAX(name, flags, func) \
    env_define(env, intern(name), make_builtin(FL_SYNTAX | flags, func, intern(name)))

void        define_toplevels(scm_val env) {
    DEF_PROC_CHAR("+", fn_foldl_arith, '+') ;
    DEF_PROC_CHAR("-", fn_foldl_arith, '-') ;
    DEF_PROC_CHAR("*", fn_foldl_arith, '*') ;
    DEF_PROC_CHAR("/", fn_foldl_arith, '/') ;
    DEF_PROC_CHAR("=", fn_foldl_cmp, '=') ;
    DEF_PROC_CHAR(">", fn_foldl_cmp, '>') ;
    DEF_PROC_CHAR(">=", fn_foldl_cmp, 'g') ;
    DEF_PROC_CHAR("<", fn_foldl_cmp, '<') ;
    DEF_PROC_CHAR("<=", fn_foldl_cmp, 'l') ;
    DEF_PROC_CHAR("!=", fn_foldl_cmp, '!') ;
    DEF_PROC("reverse!", fn_reverse_bang) ;
    DEF_PROC("reverse", fn_reverse) ;
    DEF_PROC("car", fn_car) ;
    DEF_PROC("cdr", fn_cdr) ;
    DEF_PROC("caar", fn_caar) ;
    DEF_PROC("cadr", fn_cadr) ;
    DEF_PROC("cdar", fn_cdar) ;
    DEF_PROC("cddr", fn_cddr) ;
    DEF_PROC("cons", fn_cons) ;
    DEF_PROC("display", fn_display) ;
    DEF_PROC("newline", fn_newline) ;
    DEF_PROC("_if", fn_if) ;

    DEF_SYNTAX_CHAR("quote", 0, syn_quote, '\'') ;
    DEF_SYNTAX_CHAR("pseudoquote", 0, syn_quote, '`') ;
    DEF_SYNTAX("lambda", 0, syn_lambda) ;
    DEF_SYNTAX("define", FL_EVAL, syn_define) ;
    DEF_SYNTAX("if", FL_EVAL, syn_if) ;
}

void        builtin_tests(void) {
    scm_val e = env_create(NIL) ;
    scm_val v ;
    printf("\n;; --- BUILTIN TESTS --- ;;\n") ;

    v = cons(MKTAG(3, FIXNUM), NIL) ;
    SCM_DEBUG(fn_foldl_arith(v, e, MKTAG('+', CHAR)), "(+ 3)") ;
    v = cons(MKTAG(6, FIXNUM), v) ;
    SCM_DEBUG(fn_foldl_arith(v, e, MKTAG('%', CHAR)), "(% 6 3)") ;
    SCM_DEBUG(fn_foldl_arith(v, e, MKTAG('-', CHAR)), "(- 6 3)") ;
    SCM_DEBUG(fn_foldl_arith(v, e, MKTAG('/', CHAR)), "(/ 6 3)") ;
    v = cons(MKTAG(9, FIXNUM), v) ;
    SCM_DEBUG(fn_foldl_arith(v, e, MKTAG('*', CHAR)), "(* 9 6 3)") ;
    SCM_DEBUG(fn_reverse(cons(v, NIL), e, NIL),  "(reverse '(9 6 3))") ;
    SCM_DEBUG(v, "orig") ;
    v = fn_reverse_bang(cons(v, NIL), e, NIL) ;
    SCM_DEBUG(v, "(reverse! '(9 6 3))") ;
    v = fn_reverse_bang(cons(CDR(v), NIL), e, NIL) ;
    SCM_DEBUG(v, "(reverse! '(6 9))") ;
    v = fn_reverse_bang(cons(CDR(v), NIL), e, NIL) ;
    SCM_DEBUG(v, "(reverse! '(6))") ;
    SCM_DEBUG(fn_reverse_bang(cons(NIL, NIL), e, NIL), "(reverse! '())") ;
}
