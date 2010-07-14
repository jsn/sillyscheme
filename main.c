#include "scheme.h"
#include "scanner.h"

int main (int ac, char const* av[]) {
    struct scm_scanner *scanner = scm_create_scanner(stdin) ;

    for (;;) {
        printf("\n> ") ;
        fflush(stdout) ;
        scm_val v = scm_read(scanner, NIL) ;
        scm_print(v, stdout) ;
        if (v.l == ((-1 << 2) | CHAR)) break ;
    }
    fflush(stdout) ;
    return 0;
}
