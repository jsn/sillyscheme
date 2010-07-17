;; ok, let's see if we can have turtles all the way down

(_set! 'define
       (syntax-lambda (sym val) `(_define ',sym ,val)))

(define if
  (syntax-lambda (cond_ then_ else_)
                 `((_if ,cond_ (lambda () ,then_) (lambda () ,else_)))))

(define set!
  (syntax-lambda (sym val) `(_set! ',sym ,val)))

(define map
  (lambda (f list)
    (if (null? list) '()
      (cons (f (car list)) (map f (cdr list))))))

(define begin
  (syntax-lambda (first . rest)
                 `((lambda () ,first ,@rest))))

(define unnamed-let
  (syntax-lambda (defs . body)
                 `((lambda (,@(map car defs)) ,@body) ,@(map cadr defs))))

(define named-let
  (syntax-lambda (name defs . body)
                 `((lambda ()
                     (define ,name
                       (lambda (,@(map car defs)) ,@body))
                     (,name ,@(map cadr defs))))))

(define let
  (syntax-lambda (head . rest)
                 (if (list? head)
                   `(unnamed-let ,head ,@rest)
                   `(named-let ,head ,@rest))))

