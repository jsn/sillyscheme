#include "scheme.h"

#define FLOAT_OF(x) (type_of(x) == FLOAT ? x.c->data.f : (double)UNTAG(x))
#define DEFINE_FUNC(name)   \
    scm_val name(scm_val args, Silly scm, scm_val hint)

#define CALL(sym, cdr)  cons(intern(#sym), cdr)

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

    if (got_double) {
        scm_val CELL(v, FLOAT, NIL, NIL) ;
        v.c->data.f = rv ;
        return v ;
    }
    return MKTAG((long)rv, FIXNUM) ;
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

scm_val     append_bang(scm_val x, scm_val y) {
    scm_val v ;
    FOREACH(v, x)
        if (NULL_P(CDR(v))) {
            CDR(v) = y ;
            return x ;
        };
    return y ;
}

DEFINE_FUNC(fn_append_bang) {
    scm_val x = NIL, y ;
    FOREACH(y, args) x = append_bang(x, CAR(y)) ;
    return x ;
}

DEFINE_FUNC(fn_append) {
    scm_val head = NIL, tail = NIL, x, v ;

    FOREACH(x, args)
        if (NULL_P(CDR(x)))
            if (NULL_P(head))
                head = tail = CAR(x) ;
            else
                CDR(tail) = CAR(x) ;
        else
            FOREACH(v, CAR(x)) {
                if (NULL_P(head))
                    head = tail = cons(CAR(v), NIL) ;
                else
                    tail = CDR(tail) = cons(CAR(v), NIL) ;
            }
    return head ;
}

DEFINE_FUNC(fn_apply) {
    scm_val head = NIL, tail = NIL, x ;

    FOREACH(x, args) {
        scm_val v = NULL_P(CDR(x)) ? CAR(x) : cons(CAR(x), NIL) ;
        if (NULL_P(head))
            head = tail = v ;
        else
            tail = CDR(tail) = v ;
    }
    return scm_apply(head, scm, hint) ;
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
            case '>': rv = rv && (x >  y) ; break ;
            case 'g': rv = rv && (x >= y) ; break ;
            case '<': rv = rv && (x <  y) ; break ;
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
DEFINE_FUNC(fn_eq_p) { return EQ_P(CAR(args), CADR(args)) ? TRUE : FALSE ; }
DEFINE_FUNC(fn_number_p) {
    int i = type_of(CAR(args)) ;
    return (i == FIXNUM || i == FLOAT) ? TRUE : FALSE ;
}
DEFINE_FUNC(fn_inexact_p) { return type_of(CAR(args)) == FLOAT ? TRUE : FALSE ;}
DEFINE_FUNC(fn_symbol_p) { return TAG(CAR(args)) == SYMBOL ? TRUE : FALSE ;}
DEFINE_FUNC(fn_string_p) { return type_of(CAR(args)) == STRING ? TRUE : FALSE ;}
DEFINE_FUNC(fn_char_p) { return TAG(CAR(args)) == CHAR ? TRUE : FALSE ;}
DEFINE_FUNC(fn_string_to_symbol) { return intern(CAAR(args).p) ; }

DEFINE_FUNC(fn_car) { return CAR(CAR(args)) ; }
DEFINE_FUNC(fn_cdr) { return CDR(CAR(args)) ; }
DEFINE_FUNC(fn_caar) { return CAAR(CAR(args)) ; }
DEFINE_FUNC(fn_cadr) { return CADR(CAR(args)) ; }
DEFINE_FUNC(fn_cdar) { return CDAR(CAR(args)) ; }
DEFINE_FUNC(fn_cddr) { return CDDR(CAR(args)) ; }

DEFINE_FUNC(fn_cons) { return cons(CAR(args), CADR(args)) ; }

DEFINE_FUNC(fn_display) { scm_print(CAR(args), scm->fp_o) ; return FALSE ; }
DEFINE_FUNC(fn_newline) { fprintf(scm->fp_o, "\n") ; return FALSE ; }
DEFINE_FUNC(fn_read) { return scm_read(scm, NIL) ; }
DEFINE_FUNC(fn_eof_object_p) { return EQ_P(CAR(args), SCM_EOF) ? TRUE : FALSE ;}
DEFINE_FUNC(fn_print) {
    scm_val v ;
    FOREACH(v, args) {
        scm_val w = CAR(v) ;
        switch (type_of(w)) {
            case STRING: fwrite(CAR(w).p, UNTAG(CDR(w)), 1, scm->fp_o) ; break ;
            case CHAR: fputc(UNTAG(w), scm->fp_o) ; break ;
            default: scm_print(w, scm->fp_o) ; break ;
        }
    }
    if (EQ_P(hint, intern("print")))
        fputc('\n', scm->fp_o) ;
    else
        fflush(stdout) ;
    return FALSE ;
}

DEFINE_FUNC(syn_quote) { return CAR(args) ; }

DEFINE_FUNC(syn_quasiquote) {
    scm_val v = CAR(args), cdr, car ;

#define Q(x)        CALL(quote, cons(x, NIL))
#define QQ(x)       CALL(quasiquote, cons(x, NIL))
#define QQ_LIST(l)  (NULL_P(l) ? Q(l) : QQ(l))

    switch (type_of(v)) {
        case FIXNUM: case CHAR: case BOOL: case FLOAT: case STRING:
            return v ;
        case SYMBOL: case NONE:
            return Q(v) ;
        case CONS:
            break ;
        default:
            die("oops\n") ;
    }

    /* CONS */
    car = CAR(v) ;
    cdr = CDR(v) ;

    if (EQ_P(car, intern("unquote"))) return CAR(cdr) ;
    if (EQ_P(car, intern("quasiquote"))) return Q(QQ(CAR(cdr))) ;
    if (type_of(car) == CONS && EQ_P(CAR(car), intern("unquote-splicing")))
        return CALL(append,
                cons(CADR(car),
                    cons(QQ_LIST(cdr),
                        NIL))) ;
    return CALL(cons,
                cons(QQ(car),
                    cons(QQ_LIST(cdr),
                        NIL))) ;
}

DEFINE_FUNC(fn_define) {
    env_define(scm->e, CAR(args), CADR(args)) ;
    return CADR(args) ;
}

DEFINE_FUNC(fn_set_bang) {
    env_set(scm->e, CAR(args), CADR(args)) ;
    return CADR(args) ;
}

DEFINE_FUNC(fn_if) {
    scm_val cond = CAR(args), then_ = CADR(args), else_ = CADDR(args) ;
    return EQ_P(cond, FALSE) ? else_ : then_ ;
}

DEFINE_FUNC(syn_lambda) {
    scm_val CELL(proc, PROCEDURE, args, scm->e) ;
    return proc ;
}

DEFINE_FUNC(syn_syntax_lambda) {
    scm_val CELL(proc, PROCEDURE, args, scm->e) ;
    proc.c->flags |= FL_SYNTAX | FL_EVAL ;
    return proc ;
}

DEFINE_FUNC(syn_transform_lambda) {
    scm_val CELL(proc, PROCEDURE, args, scm->e) ;
    proc.c->flags |= FL_SYNTAX ;
    return proc ;
}

DEFINE_FUNC(fn_pair_p) { return PAIR_P(CAR(args)) ? TRUE : FALSE ; }
DEFINE_FUNC(fn_list_p) { return LIST_P(CAR(args)) ? TRUE : FALSE ; }
DEFINE_FUNC(fn_null_p) { return NULL_P(CAR(args)) ? TRUE : FALSE ; }

scm_val     make_builtin(int flags, native_proc f, scm_val hint) {
    scm_val CELL(proc, PROCEDURE, NIL, hint) ;
    proc.c->flags = FL_BUILTIN | flags ;
    CAR(proc).p = f ;
    return proc ;
}

#define DEF_PROC_CHAR(name, func, hint)  \
    env_define(env, intern(name), make_builtin(0, func, MKTAG(hint, CHAR)))

#define DEF_PROC(name, func) \
    env_define(env, intern(name), make_builtin(0, func, intern(name)))

#define DEF_SYNTAX(name, flags, func) \
    env_define(env, intern(name), make_builtin(FL_SYNTAX | flags, func, intern(name)))

void        define_toplevels(scm_val env) {
    DEF_PROC_CHAR("+", fn_foldl_arith, '+') ;
    DEF_PROC_CHAR("-", fn_foldl_arith, '-') ;
    DEF_PROC_CHAR("*", fn_foldl_arith, '*') ;
    DEF_PROC_CHAR("/", fn_foldl_arith, '/') ;
    DEF_PROC_CHAR("%", fn_foldl_arith, '%') ;
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
    DEF_PROC("read", fn_read) ;
    DEF_PROC("print", fn_print) ;
    DEF_PROC("print*", fn_print) ;
    DEF_PROC("eof-object?", fn_eof_object_p) ;
    DEF_PROC("_if", fn_if) ;
    DEF_PROC("_define", fn_define) ;
    DEF_PROC("_set!", fn_set_bang) ;
    DEF_PROC("append!", fn_append_bang) ;
    DEF_PROC("append", fn_append) ;
    DEF_PROC("list?", fn_list_p) ;
    DEF_PROC("pair?", fn_pair_p) ;
    DEF_PROC("null?", fn_null_p) ;
    DEF_PROC("apply", fn_apply) ;
    DEF_PROC("eval", fn_eval) ;
    DEF_PROC("capture/cc", fn_capture_cc) ;
    DEF_PROC("apply/cc", fn_apply_cc) ;
    DEF_PROC("eq?", fn_eq_p) ;
    DEF_PROC("number?", fn_number_p) ;
    DEF_PROC("inexact?", fn_inexact_p) ;
    DEF_PROC("symbol?", fn_symbol_p) ;
    DEF_PROC("string?", fn_string_p) ;
    DEF_PROC("char?", fn_char_p) ;
    DEF_PROC("string->symbol", fn_string_to_symbol) ;

    DEF_SYNTAX("quote", 0, syn_quote) ;
    DEF_SYNTAX("quasiquote", FL_EVAL, syn_quasiquote) ;
    DEF_SYNTAX("lambda", 0, syn_lambda) ;
    DEF_SYNTAX("syntax-lambda", 0, syn_syntax_lambda) ;
    DEF_SYNTAX("transform-lambda", 0, syn_transform_lambda) ;
}
