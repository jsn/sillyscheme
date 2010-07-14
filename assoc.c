#include "scheme.h"

scm_val     assq(scm_val alist, scm_val key) {
    scm_val v ;

    for (v = alist; !NULL_P(v); v = CDR(v)) {
        ENSURE(EQ_P(list_p(v), TRUE), "assq: not a list\n") ;
        ENSURE(PAIR_P(CAR(v)), "assq: not a pair\n") ;
        if (EQ_P(CAAR(v), key)) return CAR(v) ;
    }
    return FALSE ;
}

void        assoc_tests(void) {
    FILE    *fp ;
    struct scm_scanner *scanner ;
    scm_val alist, k ;

    printf("\n;; --- ASSOC TESTS --- ;;\n") ;

    ASSERT(fp = fopen("tests/assoc.scm", "r")) ;
    scanner = scm_create_scanner(fp) ;

    alist = scm_read(scanner, NIL) ;
    SCM_DEBUG(alist, "alist") ;

    k = scm_read(scanner, NIL) ;
    SCM_DEBUG(k, "k1") ;
    SCM_DEBUG(assq(alist, k), "v1") ;

    k = scm_read(scanner, NIL) ;
    SCM_DEBUG(k, "k2") ;
    SCM_DEBUG(assq(alist, k), "v2") ;
}
