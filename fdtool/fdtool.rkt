has#lang racket
;; This is the Racket version of fdtool.py
;; Writing the tool in a Lisp makes is easier to run it on
;; a classic Amiga

;; This file is intended to be used as a Racket module,
;; so here are the exports
(provide process-fd)

;; FD file comments start with a '*'
(define (fd-comment? line)
  (char=? #\* (string-ref line 0)))

(define (fd-command? line)
  (string=? "##" (substring line 0 2)))

;; takes a command line and modifies the params hash accordingly
(define (process-command params line)
  ;;(print line)
  (cond [(string=? line "##private") (hash-set! params "is-private" #T)]
        [(string=? line "##public") (hash-set! params "is-private" #F)]
        [(and (> (string-length line) 7) (string=? (substring line 0 6) "##base"))
         (hash-set! params "base" (substring line 7))]
        [(and (> (string-length line) 7) (string=? (substring line 0 6) "##bias"))
         (hash-set! params "offset" (string->number (substring line 7)))])
  params)

;; process a function definition, extracting the function name,
;; the parameter names and the assigned registers
(define (process-fundef params line)
  (let ([fun-match (regexp-match
                    #rx"^([^()]+)[(]([^()]*)[)][(]([^()]*)[)].*$" line)])
    (cond [fun-match
           ;; only process public functions
           (cond [(not (hash-ref params "is-private"))
                  (printf "-~s -> ~a\n"
                          (hash-ref params "offset") (car (cdr fun-match)))])
           ;; we need to advance the offset for both private and public
           (hash-set! params "offset" (+ 6 (hash-ref params "offset")))])))

;; Process the input file line-by-line
(define (process-fd filename)
  ;; params is a hash, could be a struct as well
  (let ([params (make-hash '(("base" . "")
                             ("is-private" . #T)
                             ("offset" . 0)))]
        [lines (filter (lambda (x) (not (fd-comment? x))) (file->lines filename))])
    (map (lambda (line)
           (if (fd-command? line) (set! params (process-command params line))
               (process-fundef params line))) lines)
    ;; make sure we do not print a potential return value
    (void)))
