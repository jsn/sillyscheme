(define if
  (syntax-lambda (cond_ then_ else_)
                 `((_if ,cond_
                        (lambda () ,then_)
                        (lambda () ,else_)))))

(define set!
  (syntax-lambda (sym val)
                 `(_set! ',sym ,val)))
