;; ok, let's see if we can have turtles all the way down

(_set! 'if
  (syntax-lambda (cond_ then_ else_)
    `((_if ,cond_ (lambda () ,then_) (lambda () ,else_)))))

(_set! 'defmacro
  (syntax-lambda (sym . rest)
    (if (pair? sym)
      `(_define ',(car sym) (syntax-lambda ,(cdr sym) ,@rest))
      `(_define ',sym ,@rest))))

(defmacro (define sym . rest)
    (if (pair? sym)
      `(_define ',(car sym) (lambda ,(cdr sym) ,@rest))
      `(_define ',sym ,@rest)))

(define (map1 f list)
  (define (loop list done)
    (if (null? list) (reverse! done)
      (loop (cdr list) (cons (f (car list)) done))))
  (loop list '()))

(define (map f . lists)
  (define (loop lists ac)
    (if (null? (car lists)) (reverse! ac)
      (loop (map1 cdr lists) (cons (apply f (map1 car lists)) ac))))
  (loop lists '()))

(define (for-each f . lists)
  (if (null? (car lists)) #t
    (begin
      (apply f (map car lists))
      (apply for-each f (map cdr lists)))))

(define (not bool) (_if bool #f #t))

(defmacro (and . rest)
  (if (null? rest) #t
    `(if ,(car rest) (and ,@(cdr rest)) #f)))

(defmacro (or . rest)
  (if (null? rest) #f
    `(if ,(car rest) #t (or ,@(cdr rest)))))

(defmacro (begin . body) `((lambda () ,@body)))

(defmacro (let head . rest)
  (if (list? head)
    `((lambda (,@(map1 car head)) ,@rest) ,@(map1 cadr head))
    `((lambda ()
        (define ,head (lambda (,@(map1 car (car rest))) ,(cadr rest)))
        (,head ,@(map1 cadr (car rest)))))))

(define (fold-left f s lst)
  (if (null? lst) s
    (fold-left f (f s (car lst)) (cdr lst))))

(define (min first . rest)
  (fold-left (lambda (x y) (_if (> x y) y x)) first rest))

(define (max first . rest)
  (fold-left (lambda (x y) (_if (< x y) y x)) first rest))

(define (abs x) (_if (>= x 0) x (- 0 x)))
(define (exact? x) (and (number? x) (not (inexact? x))))
(define integer? exact?)
(define real? number?)
(define (complex? x) #f)
(define (zero? x) (= x 0))
(define (positive? x) (> x 0))
(define (negative? x) (< x 0))
(define (odd? x) (and (integer? x) (= (% x 2) 1)))
(define (even? x) (not (odd? x)))
(define (boolean? x) (or (eq? x #t) (eq? x #f)))

(define (list-tail list k) (if (> k 0) (list-tail (cdr list) (- k 1)) list))
(define (list-ref list k) (car (list-tail list k)))

(define (any f x list)
  (if (null? list) #f (if (f x (car list)) list (any f x (cdr list)))))

(define (memq x list) (any eq? x list))
(define (memv x list) (any eqv? x list))
(define (member x list) (any equal? x list))

(define quotient /)
(define remainder %)
(define eqv? eq?)

(define (equal? x y)
  (or (eqv? x y)
      (and (pair? x) (pair? y)
           (equal? (car x) (car y))
           (equal? (cdr x) (cdr y)))))

(define (call/cc f)
  (capture/cc (lambda (c) (f (lambda (val) (apply/cc c val))))))

(define call-with-current-continuation call/cc)

(define (list . args)
  (if (null? args) '() (cons (car args) (apply list (cdr args)))))

(define (length list)
  (let loop ((list list) (l 0))
    (if (null? list) l (loop (cdr list) (+ l 1)))))

(define %%% #f)

(defmacro (set! sym val) `(_set! ',sym ,val))

(define (repl)
  (print* ">> ")
  (let ((term (read)))
    (if (eof-object? term) term
      (let ((value (eval term)))
        (print* "==> " )
        (set! %%% value)
        (display value)
        (newline)
        (repl)))))
