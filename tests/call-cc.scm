(define *z* 'notset)

(print "z is " *z*)

(define f
  (lambda (i)
    (let loop ((i i))
      (if (= 0 i) #f
        (begin
          (if (= i 5)
            (call-with-current-continuation
              (lambda (k)
                (print "setting continuation")
                (set! *z* k)))
            #t)
          (print "i is " i)
          (loop (- i 1)))))))

(f 13)
(*z* #f)
