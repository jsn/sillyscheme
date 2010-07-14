#include "scheme.h"
#include "scanner.h"

int main (int ac, char const* av[]) {
    struct scm_scanner *scanner = scm_create_scanner(stdin) ;

    for (;;) {
        scm_val v = scm_read(scanner) ;
        scm_print(v, stdout) ;
        if (v.l == ((-1 << 2) | CHAR)) break ;
    }
    return 0;
}
