(define upto
  (lambda (x f)
    (let loop ((i 0))
      (if (>= i x) #t
        (begin (f i) (loop (+ i 1)))))))

(display '(upto 10))
(upto 10 (lambda (x) (display x) (display '-)))
(newline)

