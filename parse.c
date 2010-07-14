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

void                scm_destroy_scanner(struct scm_scanner *sc) { free(sc) ; }

static scm_val parse_fixnum(const char *s) {
    long l ;
    char *ep ;

    l = strtol(s, &ep, 0) ;
    ASSERT(!*ep) ;
    return MKTAG(l, FIXNUM) ;
}

static scm_val parse_float(const char *s) {
    scm_val v ;
    char *ep ;

    v.p = mkcell(FLOAT) ;
    ((struct cell *)v.p)->data.f = strtod(s, &ep) ;
    ASSERT(!*ep) ;
    return v ;
}

static scm_val parse_string(const char *s) {
    scm_val v ;
    v.p = mkcell(STRING) ;
    ENSURE(CAR(v).p = strdup(s), "strdup()") ;
    CDR(v).l = strlen(s) ;
    return v ;
}

static scm_val parse_quoted(struct scm_scanner *sc, const char *sym) {
    return cons(intern(sym), cons(scm_read(sc, NIL), NIL)) ;
}

scm_val     scm_read(struct scm_scanner *sc, scm_val list) {
    int     token = scml_lex(sc->scanner) ;
    scm_val v ;
    switch(token) {
        case 0:         v = SCM_EOF ; break ;
        case BOOL:      v = MKTAG((sc->extra ? 1 : 0), BOOL) ; break ;
        case FIXNUM:    v = parse_fixnum(sc->extra) ; break ;
        case FLOAT:     v = parse_float(sc->extra) ; break ;
        case CHAR:      v = MKTAG(sc->extra[2], CHAR) ; break ;
        case STRING:    v = parse_string(sc->extra) ; break ;
        case SYMBOL:    v = intern(sc->extra) ; break ;
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
                    if (NULL_P(list)) die("unexpected '%c'\n", *sc->extra) ;
                    if (EQ_P(list, TRUE)) return NIL ;
                    ASSERT(PAIR_P(list)) ;
                    CDR(list) = NIL ;
                    return list ;
                case '.':
                    if (NULL_P(list)) die("unexpected '%c'\n", *sc->extra) ;
                    ASSERT(PAIR_P(list)) ;
                    CDR(list) = scm_read(sc, NIL) ;
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

    if (NULL_P(list)) return v ;
    if (EQ_P(list, TRUE)) return scm_read(sc, cons(v, NIL)) ;
    ASSERT(PAIR_P(list)) ;
    CDR(list) = scm_read(sc, cons(v, NIL)) ;
    return list ;
}

static void print_list(scm_val v, FILE *fp, int no_parens) {
    if (!no_parens) fprintf(fp, "(") ;
    if (!NULL_P(v)) {
        if (no_parens) fprintf(fp, " ") ;
        scm_print(CAR(v), fp) ;
        if (LIST_P(CDR(v))) {
            print_list(CDR(v), fp, 1) ;
        } else {
            fprintf(fp, " . ") ;
            scm_print(CDR(v), fp) ;
        }
    }
    if (!no_parens) fprintf(fp, ")") ;
}

void        scm_print(scm_val v, FILE *fp) {

    if (LIST_P(v)) return print_list(v, fp, 0) ;

    switch(TAG(v)) {
        case BOOL:   fprintf(fp, "#%c", UNTAG(v) ? 't' : 'f') ; break ;
        case FIXNUM: fprintf(fp, "%ld", UNTAG(v)) ; break ;
        case SYMBOL: fprintf(fp, "%s", sym_to_string(v)) ; break ;
        case CHAR: {
            int c = UNTAG(v) ;
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
                default:
                    die("unknown cell type %d\n", c->type) ;
            }
        }
    }
}

void        parse_tests(void) {
    FILE    *fp ;
    struct scm_scanner *scanner ;

    printf("\n;; --- PARSE TESTS --- ;;\n") ;

    ASSERT(fp = fopen("tests/parse.scm", "r")) ;
    scanner = scm_create_scanner(fp) ;

    for (;;) {
        scm_val v = scm_read(scanner, NIL) ;
        scm_print(v, stdout) ;
        printf("\n;;;;\n") ;
        if (EQ_P(v, SCM_EOF)) break ;
    }
    scm_destroy_scanner(scanner) ;
    fflush(stdout) ;
}
