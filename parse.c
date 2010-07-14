#include "scheme.h"
#include "scanner.h"

void    scm_read(const char *fname) {
    FILE *fp = NULL ;
    int  token ;
    char *s ;
    yyscan_t    scanner ;

    if (fname) {
        fp = fopen(fname, "r") ;
        ENSURE(fp, "fopen()") ;
    }

    scml_lex_init(&scanner) ;
    if (fp) scml_set_in(fp, scanner) ;
    scml_set_extra(&s, scanner) ;

    while(0 != (token = scml_lex(scanner))) {
        switch(token) {
            case 0:
                break ;
            case FIXNUM:
                printf("int %s\n", s) ; break ;
            case FLOAT:
                printf("float %s\n", s) ; break ;
            case STRING:
                printf("string %s\n", s) ; break ;
            case SYMBOL:
                printf("symbol %s\n", s) ; break ;
            case CHAR:
                printf("char %s\n", s) ; break ;
            case SPECIAL:
                printf("special %s\n", s) ; break ;
            default:
                die("lexer says hi: %d (%s)\n", token, s) ;
        }
    }
}
