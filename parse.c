#include <string.h>
#include "scheme.h"
#include "scanner.h"

struct scm_scanner {
    yyscan_t scanner ;
    char     *extra ;
};

struct scm_scanner  *scm_create_scanner(FILE *fp) {
    struct scm_scanner *sc = malloc(sizeof(*sc)) ;
    ASSERT(sc) ;
    scml_lex_init(&sc->scanner) ;
    if (fp) scml_set_in(fp, sc->scanner) ;
    scml_set_extra(&sc->extra, sc->scanner) ;

    return sc ;
}

static scm_val parse_fixnum(const char *s) {
    scm_val v ;
    long l ;
    char *ep ;

    l = strtol(s, &ep, 0) ;
    ASSERT(!*ep) ;
    v.l = (l << 2) | FIXNUM ;
    return v ;
}

static struct cell *mkcell(int type) {
    struct cell *c = malloc(sizeof(*c)) ;
    ENSURE(c, "malloc()") ;
    c->type = type ;
    return c ;
}

static scm_val parse_float(const char *s) {
    scm_val v ;
    char *ep ;

    v.p = mkcell(FLOAT) ;
    ((struct cell *)v.p)->data.f = strtod(s, &ep) ;
    ASSERT(!*ep) ;
    return v ;
}

static scm_val parse_string(const char *s, int type) {
    scm_val v ;
    struct cell *c = mkcell(type) ;
    ENSURE(c->data.cons.car.p = strdup(s), "strdup()") ;
    c->data.cons.cdr.l = strlen(s) ;
    v.p = c ;
    return v ;
}

scm_val     list_p(scm_val v) {
    if (EQ_P(v, NIL)) return TRUE ;
    if (v.l & 3) return FALSE ;
    return ((struct cell *)v.p)->type == CONS ? TRUE : FALSE ;
}

scm_val     cons(scm_val car, scm_val cdr) {
    struct cell *c = mkcell(CONS) ;
    c->data.cons.car = car ;
    c->data.cons.cdr = cdr ;
    return (scm_val)((void *)c) ;
}

static scm_val parse_quoted(struct scm_scanner *sc, const char *sym) {
    return cons(parse_string(sym, SYMBOL), cons(scm_read(sc, NIL), NIL)) ;
}

scm_val     scm_read(struct scm_scanner *sc, scm_val list) {
    int     token = scml_lex(sc->scanner) ;
    scm_val v ;
    switch(token) {
        case 0:         v = SCM_EOF ; break ;
        case BOOL:      v.l = ((sc->extra ? 1 : 0) << 2) | BOOL ; break ;
        case FIXNUM:    v = parse_fixnum(sc->extra) ; break ;
        case FLOAT:     v = parse_float(sc->extra) ; break ;
        case CHAR:      v.l = (((long)sc->extra[2]) << 2) | CHAR ; break ;
        case STRING:    v = parse_string(sc->extra, STRING) ; break ;
        case SYMBOL:    v = parse_string(sc->extra, SYMBOL) ; break ;
        case SPECIAL:
            switch(*sc->extra) {
                case '(':  v = scm_read(sc, TRUE) ; break ;
                case '\'': v = parse_quoted(sc, "quote") ; break ;
                case '`':  v = parse_quoted(sc, "quasiquote") ; break ;
                case ',':
                    v = parse_quoted(sc,
                            sc->extra[1] ? "unquote-splicing" : "unquote") ;
                    break ;
                case ')':
                    if (EQ_P(list, NIL)) die("unexpected '%c'\n", *sc->extra) ;
                    if (EQ_P(list, TRUE)) return NIL ;
                    ASSERT(EQ_P(list_p(list), TRUE)) ;
                    ((struct cell *)list.p)->data.cons.cdr = NIL ;
                    return list ;
                case '.':
                    if (EQ_P(list, NIL)) die("unexpected '%c'\n", *sc->extra) ;
                    ASSERT(EQ_P(list_p(list), TRUE)) ;
                    ((struct cell *)list.p)->data.cons.cdr = scm_read(sc, NIL) ;
                    if (!EQ_P(scm_read(sc, TRUE), NIL))
                        die("bad dotted pair\n") ;
                    return list ;
                default:
                    die("unknown special %c\n", *sc->extra) ;
            }
            break ;

        default:
            die("lexer says hi: %d (%s)\n", token, sc->extra) ;
    }

    if (EQ_P(list, NIL)) return v ;
    if (EQ_P(list, TRUE)) return scm_read(sc, cons(v, NIL)) ;
    ASSERT(EQ_P(list_p(list), TRUE)) ; 
    ((struct cell *)list.p)->data.cons.cdr = scm_read(sc, cons(v, NIL)) ;
    return list ;
}

static void print_cons(scm_val v, FILE *fp, int no_parens) {
    if (!no_parens) fprintf(fp, "(") ;
    if (!EQ_P(v, NIL)) {
        if (no_parens) fprintf(fp, " ") ;
        struct cell *c = v.p ;
        scm_print(c->data.cons.car, fp) ;
        if (EQ_P(list_p(c->data.cons.cdr), TRUE)) {
            print_cons(c->data.cons.cdr, fp, 1) ;
        } else {
            fprintf(fp, " . ") ;
            scm_print(c->data.cons.cdr, fp) ;
        }
    }
    if (!no_parens) fprintf(fp, ")") ;
}

void        scm_print(scm_val v, FILE *fp) {
    if (EQ_P(list_p(v), TRUE)) {
        print_cons(v, fp, 0) ;
        return ;
    }

    switch(v.l & 3) {
        case BOOL:   fprintf(fp, "#%c", (v.l >> 2) ? 't' : 'f') ; break ;
        case FIXNUM: fprintf(fp, "%ld", v.l >> 2) ; break ;
        case CHAR: {
            int c = v.l >> 2 ;
            if (c < 0) fprintf(fp, "#\\eof") ;
            else fprintf(fp, "#\\%c", c) ;
            break ;
        }
        default: {
            struct cell *c = v.p ;
            switch (c->type) {
                case FLOAT: fprintf(fp, "%f", c->data.f) ; break ;
                case STRING:
                    fprintf(fp, "\"%s\"", (char *)c->data.cons.car.p) ;
                    break ;
                case SYMBOL:
                    fprintf(fp, "%s", (char *)c->data.cons.car.p) ;
                    break ;
                default:
                    die("unknown cell type %d\n", c->type) ;
            }
        }
    }
}
