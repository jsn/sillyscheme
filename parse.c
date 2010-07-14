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

static scm_val parse_char(int c) {
    scm_val v ;
    v.l = (c << 2) | CHAR ;
    return v ;
}

scm_val     scm_read(struct scm_scanner *sc) {
    int  token ;

    for (;;) {
        token = scml_lex(sc->scanner) ;
        switch(token) {
            case 0:
                return parse_char(-1) ;
            case FIXNUM:
                return parse_fixnum(sc->extra) ;
            case FLOAT:
                printf("float %s\n", sc->extra) ; break ;
            case STRING:
                printf("string %s\n", sc->extra) ; break ;
            case SYMBOL:
                printf("symbol %s\n", sc->extra) ; break ;
            case CHAR:
                return parse_char(sc->extra[2]) ;
            case SPECIAL:
                printf("special %s\n", sc->extra) ; break ;
            default:
                die("lexer says hi: %d (%s)\n", token, sc->extra) ;
        }
    }
}

void        scm_print(scm_val v, FILE *fp) {
    switch(v.l & 3) {
        case FIXNUM: fprintf(fp, "%ld", v.l >> 2) ; break ;
        case CHAR: {
                       int c = v.l >> 2 ;
                       if (c < 0) fprintf(fp, "#\\eof") ;
                       else fprintf(fp, "#\\%c", c) ;
                       break ;
                   }
        default:
            die("print says hi: %d\n", v.l) ;
    }
}
