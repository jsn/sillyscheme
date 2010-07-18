#include "scheme.h"

int main (int ac, char const* av[]) {
    gc_init(&ac) ;
    scm_eval(scm_create_evaluator(), cons(intern("repl"), NIL)) ;
    return 0;
}
