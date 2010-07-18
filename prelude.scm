;; ok, let's see if we can have turtles all the way down

(_set! 'if
  (syntax-lambda (cond_ then_ else_)
    `((_if ,cond_ (lambda () ,then_) (lambda () ,else_)))))

(_set! 'defmacro
  (syntax-lambda (sym first . rest)
    (if (pair? sym)
      `(_define ',(car sym) (syntax-lambda ,(cdr sym) ,first ,@rest))
      `(_define ',sym ,first ,@rest))))

(defmacro (define sym first . rest)
    (if (pair? sym)
      `(_define ',(car sym) (lambda ,(cdr sym) ,first ,@rest))
      `(_define ',sym ,first ,@rest)))

(define (map f list . done)
  (if (null? list) (reverse! done)
    (apply map f (cdr list) (cons (f (car list)) done))))

(defmacro (begin first . rest) `((lambda () ,first ,@rest)))

(defmacro (let head . rest)
  (if (list? head)
    `((lambda (,@(map car head)) ,@rest) ,@(map cadr head))
    `((lambda ()
        (define ,head
          (lambda (,@(map car (car rest))) ,(cadr rest)))
        (,head ,@(map cadr (car rest)))))))

(define (call/cc f)
  (capture/cc (lambda (c) (f (lambda (val) (apply/cc c val))))))

(define call-with-current-continuation call/cc)

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
