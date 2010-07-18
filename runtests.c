#include "scheme.h"

int main (int ac, char const* av[]) {
    gc_init(&ac) ;
    env_tests() ;
    eval_tests() ;
    return 0 ;
}
