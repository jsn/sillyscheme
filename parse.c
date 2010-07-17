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

static scm_val parse_fixnum(char *s) {
    scm_val v = MKTAG(strtol(s, &s, 0), FIXNUM) ;
    ASSERT(!*s) ;
    return v ;
}

static scm_val parse_float(char *s) {
    scm_val v = make_float(strtod(s, &s)) ;
    ASSERT(!*s) ;
    return v ;
}

static scm_val parse_string(const char *s) {
    scm_val v = mkcell(STRING) ;
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
        case 0:         v = SCM_EOF ;                           break ;
        case BOOL:      v = MKTAG((sc->extra ? 1 : 0), BOOL) ;  break ;
        case FIXNUM:    v = parse_fixnum(sc->extra) ;           break ;
        case FLOAT:     v = parse_float(sc->extra) ;            break ;
        case CHAR:      v = MKTAG(sc->extra[2], CHAR) ;         break ;
        case STRING:    v = parse_string(sc->extra) ;           break ;
        case SYMBOL:    v = intern(sc->extra) ;                 break ;
        case SPECIAL:
            switch(*sc->extra) {
                case '(':  v = scm_read(sc, TRUE) ;             break ;
                case '\'': v = parse_quoted(sc, "quote") ;      break ;
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
                    if (!NULL_P(scm_read(sc, TRUE))) die("bad dotted pair\n") ;
                    return list ;
                default: die("unknown special %c\n", *sc->extra) ;
            }
            break ;

        default: die("lexer says hi: %d (%s)\n", token, sc->extra) ;
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
        case BOOL:    fprintf(fp, "#%c", UNTAG(v) ? 't' : 'f') ; break ;
        case FIXNUM:  fprintf(fp, "%ld", UNTAG(v)) ;             break ;
        case SYMBOL:  fprintf(fp, "%s", sym_to_string(v)) ;      break ;
        case SPECIAL: fprintf(fp, "#<special %ld>", UNTAG(v)) ;  break ;
        case CHAR: {
            int c = UNTAG(v) ;
            if (c < 0) fprintf(fp, "#!eof") ;
            else fprintf(fp, "#\\%c", c) ;
            break ;
        }
        default: {
            switch (v.c->type) {
                case FLOAT: fprintf(fp, "%f", v.c->data.f) ; break ;
                case STRING: fprintf(fp, "\"%s\"", (char *)CAR(v).p) ; break ;
                case CONTINUATION:
                    fprintf(fp, "#<continuation [%p]>", v.p) ; break ;

                case PROCEDURE:
                    fprintf(fp, "#<%s%s ",
                            ((v.c->flags & FL_BUILTIN) ? "builtin " : ""),
                            ((v.c->flags & FL_SYNTAX) ? "syntax" :"procedure"));
                    if (v.c->flags & FL_BUILTIN) {
                        if (NULL_P(CDR(v)))
                            fprintf(fp, "[%p]", CAR(v).p) ;
                        else {
                            fprintf(fp, "(") ;
                            scm_print(CDR(v), fp) ;
                            fprintf(fp, ")") ;
                        }
                    }
                    else
                        scm_print(CAR(v), fp) ;
                    fprintf(fp, ">") ;
                    break ;

                default: die("unknown cell type %d\n", v.c->type) ;
            }
        }
    }
}
