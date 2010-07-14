#include "scheme.h"

scm_val     env_create(scm_val parent) {
    ASSERT(LIST_P(parent)) ;
    return cons(NIL, parent) ;
}

scm_val      env_get_pair(scm_val env, scm_val key, int force, int up) {
    scm_val pair ;

    ASSERT(LIST_P(env)) ;
    pair = assq(CAR(env), key) ;

    if (EQ_P(pair, FALSE)) {
        if (up && PAIR_P(CDR(env)))
            return env_get_pair(CDR(env), key, force, up) ;

        if (force) {
            pair = cons(key, NIL) ;
            CAR(env) = cons(pair, CAR(env)) ;
        }
    }
    return pair ;
}

scm_val     env_get(scm_val env, scm_val key) {
    scm_val pair = env_get_pair(env, key, 0, 1) ;
    if (EQ_P(pair, FALSE)) {
        scm_print(key, stderr) ;
        die(": unbound variable\n") ;
    }
    return CDR(pair) ;
}

void        env_tests(void) {
    scm_val e1 = env_create(NIL), e2 = env_create(e1), e3 = env_create(e2) ;
    printf("\n;; --- ENV TESTS --- ;;\n") ;
    SCM_DEBUG(e3, "init") ;
    env_define(e1, intern("one"), TRUE) ;
    env_define(e2, intern("two"), MKTAG(2, FIXNUM)) ;
    env_define(e3, intern("three"), MKTAG(333, FIXNUM)) ;
    env_define(e2, intern("two2"), MKTAG(22, FIXNUM)) ;
    SCM_DEBUG(e3, "e1: one->#t; e2: two->2, two2->22; e3: three->333\n") ;
    env_set(e3, intern("one"), MKTAG(1111, FIXNUM)) ;
    env_set(e3, intern("three"), MKTAG(4444, FIXNUM)) ;
    SCM_DEBUG(e3, "e1: one->1111, three->4444\n") ;
    SCM_DEBUG(env_get(e3, intern("one")), "get one") ;
}
