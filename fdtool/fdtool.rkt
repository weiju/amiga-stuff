#lang racket
;; This is the Racket version of fdtool.py
;; Writing the tool in a Lisp makes is easier to run it on
;; a classic Amiga

;; FD file comments start with a '*'
(define (fd-comment? line)
  (char=? #\* (string-ref line 0)))

(define (fd-command? line)
  (string=? "##" (substring line 0 2)))

;; takes a command line and modifies the params hash accordingly
(define (process-command params line)
  (print line)
  (cond [(string=? line "##private") (hash-set! params "is-private" #T)]
        [(string=? line "##public") (hash-set! params "is-private" #F)]
        [(and (> (string-length line) 7) (string=? (substring line 0 6) "##base"))
         (hash-set! params "base" (substring line 7))]
        [(and (> (string-length line) 7) (string=? (substring line 0 6) "##bias"))
         (hash-set! params "bias" (string->number (substring line 7)))])
  params)

(define (process-fundef params line)
  ;; TODO
  (printf "~s\n" line))

;; Process the input file line-by-line
(define (process filename)
  ;; params is a hash, could be a struct as well
  (let* ([params (make-hash '(("base" . "") ("is-private" . #T) ("bias" . 0)))]
         [lines (filter (lambda (x) (not (fd-comment? x))) (file->lines filename))])
    (map (lambda (line)
           (if (fd-command? line) (set! params (process-command params line))
               (process-fundef params line))) lines)
    params))
