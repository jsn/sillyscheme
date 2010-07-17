SILLY SCHEME: COMPLETELY IMPRACTICAL EXERCISE IN REINVENTING  THE WHEEL
=======================================================================

Status
======

Latest achievements:
    * **NBU Software proudly presents: *call-with-current-continuation*!**
    * some definitions (*set!*, *define* and *if*) rewritten in scheme in 
      prelude.scm, which is loaded automatically during startup.

What works:
    * Read-Print-Eval Loop works.
    * Some builtin arithmetics (fixnum and double), list functions.
    * Lambda works, non-builtin function calls work.
    * We can run factorial using y-combinator (see tests/fact.scm).
    * Lexical bindings.
    * Tail-call elimination works.
    * *quasiquote* and user defined macros.

What doesn't:
    * No memory management yet.
    * Error handling is just not there.

Design
======

Interpreter
-----------

As of now, it's a more or less vanilla SECD machine, modified for
varargs, special forms and tail calls elimination. Modifications are as 
follows:

  * Stack is not a simple list, but a list of lists. apply() removes the 
    top list of the stack (so we can support varargs).
  * Some PROCEDUREs are marked with FL_SYNTAX. When SECD machine detects
    a syntax call (takes some look-ahead), arguments are not evaluated.
    Also, if FL_EVAL flag is set for syntax, the return value of the 
    procedure is queued up for re-evaluation.
  * When a tail call is detected, new dump frames allocation is skipped in 
    apply().

Internal Representation
-----------------------
Tagged values. scm_val is C 'long'. Lower 3 bits are tags for primitive
types, higher bits are used for pointers in non-primitive types. We rely
on allocation policy granting 8-byte alignment. When memory management is
implemented, this policy will be enforced by allocator.

Primitive types are: CHAR, BOOL, FIXNUM, SYMBOL, SPECIAL.

LAMBDA [the ultimate failure] (tm)
----------------------------------
PROCEDURE is a toplevel type.
flags used are SYNTAX, BUILTIN.

PROCEDURE
  is a cons(DEFINITION, env)
DEFINITION for non-BUILTIN
  is a cons(formals, body)
DEFINITION for BUILTIN
  is a cons(CFUNC, hint)
CFUNC
  is an scm_val (\*cfunc)(scm_val params, scm_val env, scm_val hint)

Special forms:
--------------

I admittedly don't understand macros well. For now, *quasiquote* is 
implemented, and hooked up as the mechanism for user-defined macros.

Braindump
=========

1. Tagging. Apparently, in a hindsight, 8-byte alignment requirement sucks. 
   Especially since sizeof(struct cell) is 12 on 32-bit and 20 on 64 bit.  
   So, maybe we should change the tagging scheme. Something like this:

   +-------------+----------------------------+
   | Lower 2bits | Type                       |
   +=============+============================+
   |     00      | Cell ptr                   |
   +-------------+----------------------------+
   |     10      | Fixnum (even)              |
   +-------------+----------------------------+
   |     11      | Fixnum (odd)               |
   +-------------+----------------------------+
   |     01      | Extended tag (next 6 bits) |
   +-------------+----------------------------+

2. Memory management: we can try to force every non-cell blob object (like
   string data) to be always pointed at by exactly one cell. Then we get a 
   pool for cells and another pool for blobs. Objects in cell pool can be 
   garbage-collected trivially (walking C-stack may be necessary, though), 
   blobs are freed when the refrencing cell is GC-ed. I don't think I care 
   enough to do real compaction -- freelist should be enough.

TODO
=====

* [x] read
* [x] parse
* [x] eval
   * [x] symbols
   * [x] env
      * [x] lexical
   * [x] proc
      * [x] native
      * [x] compiled
      * [x] syntax
* [x] print
* [x] tail calls
* [x] continuations
* [ ] GC

Next up:
--------
No idea yet, some code cleanup is due, I guess.
After that, memory management and scheme bootstrapping.
