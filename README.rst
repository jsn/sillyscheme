=============
SILLY SCHEME:
=============

---------------------------------------------------------
Completely impractical exercise in reinventing  the wheel
---------------------------------------------------------

**July 18, 2010: Oops, dates on my computer were screwed up for one day, 
hence the commits timestamps were all wrong. I fixed it, and force-pushed 
the changes to github. Unfortunately, if you cloned / fetched the 
repository recently, you'll have to re-clone it again now -- merges / 
rebases will not work otherwise.**

Rationale
=========

This project is inspired by `this series of blog posts 
<http://avva.livejournal.com/2244437.html>`_ by Anatoly Vorobey (the link 
is in Russian).

Scheme implementations are a dime a dozen these days. In fact, the best 
ones are even free. (My personal favorite is `Chicken Scheme 
<http://callcc.org/>`_). Nobody needs another scheme interpreter, not even 
me. So why write it? To have fun, obviously. Besides, sometimes I think 
every programmer should implement some kind of Lisp at least once -- and 
no, suffering the effects of `10th Greenspun Rule 
<http://en.wikipedia.org/wiki/Greenspun's_Tenth_Rule>`_ doesn't give you a 
free pass.

So. The plan is to implement a relatively straightforward Scheme 
interpreter in C. I plan to keep C codebase as small as possible -- maybe 
under 2000 lines tops, maybe under 1500.

Minimum viable feature set: a Lisp-1, with lexical scoping, Fixnum and 
floating point arithmetics, unhygienic macros, tail-call elimination, 
reentrant continuations, and automatic memory management (garbage 
collection).

Status
======

**Update: July 18, 2010: I think I've got the minimum viable feature set 
implemented. It's rough around the edges, but all the big things are in 
place and seem to work. Took me 5 days to get here, by git log.**

Latest achievements:
    * NBU Software proudly presents: Mark-and-sweep stop-the world 
      **Garbage Collection** !
    * ``call-with-current-continuation`` works.
    * ``read`` / ``print`` implemented, Read-Eval-Print Loop reimplemented 
      in scheme.

What works:
    * Some builtin arithmetics (fixnum and double), list functions.
    * ``lambda`` works, non-builtin function calls work.
    * We can run factorial using y-combinator (see ``tests/fact.scm``).
    * Lexical bindings.
    * Tail-call elimination works.
    * ``quasiquote`` and user defined macros.
    * Automatic memory management / Garbage Collection.

What doesn't:
    * Error handling is just not there.

Design
======

Interpreter
-----------

As of now, it's a more or less vanilla SECD [#SECD]_ machine, modified for
varargs, special forms and tail calls elimination. Modifications are as 
follows:

* Stack is not a simple list, but a list of lists. ``apply`` removes the 
  top list of the stack (so we can support varargs).
* Some PROCEDUREs are marked with FL_SYNTAX. When SECD machine detects
  a syntax call (takes some look-ahead), arguments are not evaluated.
  Also, if FL_EVAL flag is set for syntax, the return value of the 
  procedure is queued up for re-evaluation.
* When a tail call is detected, new dump frames allocation is skipped 
  in ``apply``.

Internal Representation
-----------------------

Tagged values. scm_val is C ``long``. Lower 3 bits define the semantics of 
the upper 29 bits as follows. We rely on the cell allocator to always align 
cells on 4-byte boundary. Since we have our own allocator, it's easy to 
enforce.

   +------------------------------------------+-----------------------------+
   |  Machine word bit values                 |        scm_val type         |
   +==========================================+=============================+
   |    <29 or 61 bits of ptr>00              | Cell ptr, type info in cell |
   +------------------------------------------+-----------------------------+
   |  <31 or 63 bits of fixnum>1              | Fixnum                      |
   +------------------------------------------+-----------------------------+
   | <24 or 56 bits of data><6 bits of tag>10 | Extended tag (next 6 bits)  |
   +------------------------------------------+-----------------------------+

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
  is an ``scm_val (\*cfunc)(scm_val params, scm_val env, scm_val hint)``

Continuations
-------------

In SECD machine [#SECD]_, continuation is the content of dump register. So, 
basically, we capture the state of SECD machine, and we can restore it 
later.

Special forms:
--------------

I admittedly don't understand macros well. For now, ``quasiquote`` is 
implemented, and hooked up as the mechanism for user-defined macros. It 
cons()-es like there's no tomorrow, of course.

Garbage Collection:
-------------------

Pretty naive tri-color mark-and-sweep `Garbage Collection 
<http://en.wikipedia.org/wiki/Garbage_collection_(computer_science)>`_ 
[#GC]_. We do our own C stack walking to collect pointers referencing 
something inside of our memory pool.

Braindump
=========

1. Memory management: we can try to force every non-cell blob object (like
   string data) to be always pointed at by exactly one cell. Then we get a 
   pool for cells and another pool for blobs. Objects in cell pool can be 
   garbage-collected trivially (walking C-stack may be necessary, though), 
   blobs are freed when the referencing cell is GC-ed. I don't think I care 
   enough to do real compaction -- freelist should be enough.

TODO
=====

* Error handling (probably via error continuation?)
* More builtin primitives
* Bootstrap prelude.scm further

Next up:
--------
No idea yet, some code cleanup is due, I guess.
After that, memory management improvements, error handling and scheme 
bootstrapping.

References
==========
.. [#SECD] `A Rational Deconstruction of Landin's SECD Machine 
   <www.brics.dk/~danvy/DSc/27_BRICS-RS-03-33.pdf>`_
.. [#GC] `Wikipedia: Garbage collection (computer science) # Tri-color 
   marking <http://en.wikipedia.org/wiki/Garbage_collection_(computer_science)#Tri-colour_marking>`_
