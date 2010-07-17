#include "scheme.h"

int main (int ac, char const* av[]) {
    scm_eval(scm_create_evaluator(), cons(intern("repl"), NIL)) ;
    return 0;
}
