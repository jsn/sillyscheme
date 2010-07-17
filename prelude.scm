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

