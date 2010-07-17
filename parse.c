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

static scm_val parse_float(Silly scm, char *s) {
    scm_val CELL(v, FLOAT, NIL, NIL) ;
    v.c->data.f = strtod(s, &s) ;
    ASSERT(!*s) ;
    return v ;
}

static scm_val parse_string(Silly scm, const char *s) {
    scm_val CELL(v, STRING, NIL, MKTAG(strlen(s), FIXNUM)) ;
    ENSURE(CAR(v).p = strdup(s), "strdup()") ;
    return v ;
}

static scm_val parse_quoted(Silly scm, const char *sym) {
    return cons(intern(sym), cons(scm_read(scm, NIL), NIL)) ;
}

scm_val     scm_read(Silly scm, scm_val list) {
    int     token ;

    if (!scm->sc) scm->sc = scm_create_scanner(scm->fp_i) ;
    token = scml_lex(scm->sc->scanner) ;

#define EXTRA   (scm->sc->extra)

    scm_val v ;
    switch(token) {
        case 0:         v = SCM_EOF ;                           break ;
        case BOOL:      v = MKTAG((EXTRA ? 1 : 0), BOOL) ;  break ;
        case FIXNUM:    v = parse_fixnum(EXTRA) ;           break ;
        case FLOAT:     v = parse_float(scm, EXTRA) ;       break ;
        case CHAR:      v = MKTAG(EXTRA[2], CHAR) ;         break ;
        case STRING:    v = parse_string(scm, EXTRA) ;      break ;
        case SYMBOL:    v = intern(EXTRA) ;                 break ;
        case SPECIAL:
            switch(*EXTRA) {
                case '(':  v = scm_read(scm, TRUE) ;             break ;
                case '\'': v = parse_quoted(scm, "quote") ;      break ;
                case '`':  v = parse_quoted(scm, "quasiquote") ; break ;
                case ',':
                    v = parse_quoted(scm,
                            EXTRA[1] ? "unquote-splicing" : "unquote") ;
                    break ;
                case ')':
                    if (NULL_P(list)) die("unexpected '%c'\n", *EXTRA) ;
                    if (EQ_P(list, TRUE)) return NIL ;
                    ASSERT(PAIR_P(list)) ;
                    CDR(list) = NIL ;
                    return list ;
                case '.':
                    if (NULL_P(list)) die("unexpected '%c'\n", *EXTRA) ;
                    ASSERT(PAIR_P(list)) ;
                    CDR(list) = scm_read(scm, NIL) ;
                    if (!NULL_P(scm_read(scm, TRUE))) die("bad dotted pair\n") ;
                    return list ;
                default: die("unknown special %c\n", *EXTRA) ;
            }
            break ;

        default: die("lexer says hi: %d (%s)\n", token, EXTRA) ;
    }

    if (NULL_P(list)) return v ;
    if (EQ_P(list, TRUE)) return scm_read(scm, cons(v, NIL)) ;
    ASSERT(PAIR_P(list)) ;
    CDR(list) = scm_read(scm, cons(v, NIL)) ;
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
