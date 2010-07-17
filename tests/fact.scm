(display
  (((lambda (X)
      ((lambda (procedure)
         (X (lambda (arg) ((procedure procedure) arg))))
       (lambda (procedure)
         (X (lambda (arg) ((procedure procedure) arg))))))
    (lambda (func-arg)
      (lambda (n)
        (if (= 0 n)
          1
          (* n (func-arg (- n 1)))))))
   10))
(newline)

