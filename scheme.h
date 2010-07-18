#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <stdlib.h>
#include <stdio.h>

enum scm_types {                                /* i-something */
    NONE, FIXNUM, CHAR, BOOL, SYMBOL, SPECIAL,  /* immediate */ 
    FLOAT, STRING, CONS,                        /* indirection */
    PROCEDURE, CONTINUATION, GC_NODE            /* immaterial */
} ;

#define L_TAG(x) \
    ((x & 3) == 0 ? 0L : (x & 1) == 1 ? 1L : ((0xff & x) >> 2))
#define MKTAG(v, t) \
    ((scm_val)(long)(t == 1 ? ((v << 1) | 1) : ((v << 8) | (t << 2) | 2)))
#define L_UNTAG(x) \
    ((x & 1) == 1 ? (x >> 1) : (x >> 8))

#define TAGBITS     2
#define TAG(v)      L_TAG((v).l)
#define UNTAG(v)    L_UNTAG((v).l)

union _scm_val {
    long        l ;
    void        *p ;
    struct cell *c ;
} ;

#define NIL ((scm_val)0L)
#define NULL_P(x) (x.l == 0)

#define EQ_P(x, y) (x.l == y.l)

#define FALSE   MKTAG(0, BOOL)
#define TRUE    MKTAG(1, BOOL)
#define SCM_EOF MKTAG(-1, CHAR)

/* unique special values garanteed not to be coming from user code */
#define S_APPLY     MKTAG(13, SPECIAL)
#define S_EVAL      MKTAG(14, SPECIAL)

typedef union _scm_val scm_val ;

struct cell {
    short type, flags ;
    union {
        struct { scm_val car, cdr ; } cons ;
        double f ;
    } data ;
};

#define FL_BUILTIN  (1 << 0)
#define FL_SYNTAX   (1 << 1)
#define FL_EVAL     (1 << 2)
#define FL_GC_BLACK (1 << 3)
#define FL_GC_GRAY  (1 << 4)

struct scm_scanner ;

typedef struct evaluator {
    scm_val s, e, c, d ;
    scm_val top_e ;
    FILE *fp_i, *fp_o, *fp_e ;
    struct scm_scanner *sc ;
} * Silly ;

typedef scm_val (*native_proc)(scm_val args, Silly scm, scm_val hint) ;

struct      scm_scanner *scm_create_scanner(FILE *fp) ;
#define     scm_destroy_scanner free

scm_val     scm_read(Silly scm, scm_val list) ;
void        scm_print(scm_val v, FILE *fp) ;

scm_val     intern(const char *s) ;
const char  *sym_to_string(scm_val v) ;
int         type_of(scm_val v) ;

#define     CELL(v, t, car_, cdr_) v = cons(car_, cdr_) ; v.c->type = t

scm_val     cons(scm_val car, scm_val cdr) ;
scm_val     assq(scm_val alist, scm_val key) ;

#define     PAIR_P(v)   (type_of(v) == CONS)
#define     LIST_P(v)   (type_of(v) == NONE || PAIR_P(v))
#define     CONTINUATION_P(v)   (type_of(v) == CONTINUATION)

#define     CAR(v)      ((v).c)->data.cons.car
#define     CDR(v)      ((v).c)->data.cons.cdr
#define     CAAR(v)     CAR(CAR(v))
#define     CDAR(v)     CDR(CAR(v))
#define     CADR(v)     CAR(CDR(v))
#define     CDDR(v)     CDR(CDR(v))
#define     CAAAR(v)    CAR(CAAR(v))
#define     CDAAR(v)    CDR(CAAR(v))
#define     CADAR(v)    CAR(CDAR(v))
#define     CDDAR(v)    CDR(CDAR(v))
#define     CADDR(v)    CAR(CDDR(v))

#define     FOREACH(v, list)    for (v = list; !NULL_P(v); v = CDR(v))

#define     SYNTAX_P(x) (type_of(x) == PROCEDURE && ((x).c->flags & FL_SYNTAX))

scm_val     env_create(scm_val parent) ;
scm_val     env_get(scm_val env, scm_val key) ;
scm_val     env_get_pair(scm_val env, scm_val key, int force, int up) ;
#define env_define(env, key, val) CDR(env_get_pair(env, key, 1, 0)) = val
#define env_set(env, key, val) CDR(env_get_pair(env, key, 1, 1)) = val
scm_val     env_bind_formals(scm_val parent, scm_val formals, scm_val values) ;

Silly       scm_create_evaluator(void) ;
void        define_toplevels(scm_val env) ;
scm_val     scm_eval(Silly scm, scm_val code) ;
scm_val     scm_load_file(Silly scm, const char *fname) ;
void        scm_push(Silly scm, scm_val s, scm_val e, scm_val c) ;
scm_val     scm_apply(scm_val args, Silly scm, scm_val hint) ;
scm_val     reverse_append(scm_val args, scm_val head) ;

scm_val     fn_eval(scm_val args, Silly scm, scm_val hint) ;
scm_val     fn_capture_cc(scm_val args, Silly scm, scm_val hint) ;
scm_val     fn_apply_cc(scm_val args, Silly scm, scm_val hint) ;

scm_val     scm_alloc_cell(int type) ;

void        die(const char *fmt, ...) ;

void        gc_init(void *p) ;
void        gc_register(scm_val *v) ;

#define ASSERT(x) if (!(x)) die("failed: %s, %d\n", __FILE__, __LINE__)
#define ENSURE(x, msg) if (!(x)) die(msg)

#define SCM_DEBUG(x, msg) { \
    printf("%s: ", msg) ;   \
    scm_print(x, stdout) ;  \
    printf("\n") ;          \
}

void        env_tests(void) ;
void        parse_tests(void) ;
void        eval_tests(void) ;

#endif
