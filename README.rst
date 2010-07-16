SILLY SCHEME: COMPLETELY IMPRACTICAL EXERCISE IN REINVENTING  THE WHEEL
=======================================================================

Status
======

Latest achievement:
    * We can run factorial using y-combinator (see tests/fact.scm).

What works:
    * Read-Print-Eval Loop works.
    * Some builtin arithmetics (fixnum and double), list functions.
    * Lambda works, non-builtin function calls work.
    * Lexical bindings.

What doesn't:
    * No memory management yet.
    * Syntax/special forms are almost non-existent.
    * No proper tail-calls.
    * No continuations.
    * Error handling is just not there.

Design
======

Interpreter
-----------

As of now, it's a more or less vanilla SECD machine, modified for
varargs and special forms.

Internal Representation
-----------------------
Tagged values. scm_val is C 'long'. Lower 3 bits are tags for primitive
types, higher bits are used for pointers in non-primitive types. We rely
on allocation policy granting 8-byte alignment. When memory management is
implemented, this policy will be enforced by allocator.

Primitive types are: CHAR, BOOL, FIXNUM, SYMBOL.

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

Braindump
=========

1. Tagging. Apparently, in a hindsight, 8-byte alignment requirement sucks.
Especially since sizeof(struct cell) is 12 on 32-bit and 20 on 64 bit. So,
maybe we should change the tagging scheme. Something like this:

+-------------+----------------------------+
|Lower 2 bits | Type                       |
+-------------+----------------------------+
|     00      | Cell ptr                   |
+-------------+----------------------------+
|     10      | Fixnum (even)              |
+-------------+----------------------------+
|     11      | Fixnum (odd)               |
+-------------+----------------------------+
|     01      | Extended tag (next 6 bits) |
+-------------+----------------------------+

2. Tail calls: instead of storing APPLY in control register, we should store
(cons APPLY stack-position). Then we can get rid of dump register pushes.

3. Memory management: we can try to force every non-cell blob object (like
string data) to be always pointed at exactly one cell. Then we get a pool for
cells and another pool for blobs. Objects in cell pool can be garbage-collected
trivially (walking C-stack may be necessary, though), blobs are freed when the
refrencing cell is GC-ed. I don't think I care enough to do real compaction --
freelist should be enough.

TODO
=====

* [x] read
* [x] parse
* [x] eval
   * [x] symbols
   * [x] env
      * [x] lexical
   * [-] proc
      * [x] native
      * [x] compiled
      * [-] syntax
* [x] print
* [ ] tail calls
* [ ] continuations
* [ ] GC

Next up:
--------
Special forms / macros need some love badly. Got a plan for tail-calls.
