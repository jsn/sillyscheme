#include "scheme.h"

#define MAX_CELLS   10000   /* should be low enough to trigger GC often */

static struct cell  cells[MAX_CELLS] ;
static size_t       ncells ;
static scm_val      *stack_start ;
static int          stack_dir ;
static scm_val      roots ;
static scm_val      free_list ;

#define STACK_SIZE  1024

static struct {
    scm_val s[STACK_SIZE] ;
    size_t  n ;
    int     spill ;
} stack ;

#define STACK_INIT() stack.n = stack.spill = 0
#define PUSH(v) if (stack.n == STACK_SIZE) stack.spill = 1 ; \
                else stack.s[stack.n ++] = v
#define POP()   stack.s[-- stack.n]
#define GRAY(v) { (v).c->flags |= FL_GC_GRAY ; PUSH(v) ; }

void        gc_init(void *p) {
    stack_start = (scm_val *)p ;
    stack_dir = ((scm_val *)&p - stack_start) > 0 ? 1 : -1 ;
    // printf("stack at %p, stack dir is %d\n", stack_start, stack_dir) ;
}

void        gc_register(scm_val *root) {
    scm_val v = scm_alloc_cell(GC_NODE) ;
    CAR(v).p = root ;
    CDR(v) = roots ;
    roots = v ;
}

#define IN_RANGE(p)  \
    ((p).c >= cells && \
     (p).c <  (cells + ncells) && \
     (((p).l - (long)(cells)) % sizeof(*cells) == 0))

#define PTR_AND_NO_FLAG(v, flag) \
    (TAG(v) == 0 && IN_RANGE(v) && ((v).c->flags & (flag)) == 0)

static void gc_start(void) {
    scm_val v = NIL, *p ;

    STACK_INIT() ;

    for (p = stack_start; p != (scm_val *)&p; p += stack_dir)
        if (IN_RANGE(*p)) GRAY(*p) ;

    FOREACH(v, roots) {
        scm_val r = *(scm_val *)(CAR(v).p) ;
        v.c->flags = FL_GC_BLACK ;
        if (PTR_AND_NO_FLAG(r, FL_GC_GRAY)) GRAY(r) ;
    }
    // fprintf(stderr, "gc_start(): %d roots\n", stack.n) ;
}

static size_t   gc_blacken(scm_val v) {
    size_t n_grayed = 1 ;
    v.c->flags = (v.c->flags | FL_GC_BLACK) & ~FL_GC_GRAY ;
    if (v.c->type >= CONS) {
        if (PTR_AND_NO_FLAG(CAR(v), FL_GC_BLACK | FL_GC_GRAY)) {
            GRAY(CAR(v)) ;
            n_grayed ++ ;
        }
        if (PTR_AND_NO_FLAG(CDR(v), FL_GC_BLACK | FL_GC_GRAY)) {
            GRAY(CDR(v)) ;
            n_grayed ++ ;
        }
    }
    return n_grayed ;
}

static size_t   gc_mark(void) {
    size_t n_grayed = 0, total = 0 ;
    int i ;

    do {
        total += n_grayed ;
        n_grayed = 0 ;

        while (stack.n > 0) total += gc_blacken(POP()) ;
        if (!stack.spill) break ;
        stack.spill = 0 ;

        for (i = ncells - 1; i >= 0; i --) {
            scm_val v ;
            v.c = cells + i ;
            if (v.c->flags & FL_GC_GRAY) n_grayed += gc_blacken(v) ;
        }
    } while (n_grayed) ;

    return total ;
}

static void gc_sweep(void) {
    size_t i, n = 0 ;

    ENSURE(NULL_P(free_list), "free_list is not nil?\n") ;

    for (i = 0; i < ncells; i ++) {
        scm_val v ;
        v.c = cells + i ;

        if (v.c->flags & FL_GC_BLACK)
            v.c->flags &= ~(FL_GC_BLACK | FL_GC_GRAY) ;
        else {
            n ++ ;
            v.c->flags = 0 ;
            v.c->type = GC_NODE ;
            CDR(v) = free_list ;
            CAR(v) = NIL ;
            free_list = v ;
        }
    }
    // printf("swept %d\n", n) ;
}

static void gc_make_memory(void) {
    gc_start() ;
    gc_mark() ;
    gc_sweep() ;
}

scm_val     scm_alloc_cell(int type) {
    scm_val v = NIL ;

    if (ncells < MAX_CELLS)
        v.c = cells + ncells ++ ;
    else {
        if (NULL_P(free_list)) gc_make_memory() ;
        ENSURE(!NULL_P(free_list), "out of memory\n") ;
        v = free_list ;
        free_list = CDR(free_list) ;
    }

    v.c->type = type ;
    return v ;
}

