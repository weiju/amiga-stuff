#lang racket
;; A simple top-level program to 
(require "fdtool.rkt")
(process-fd (command-line #:program "fdtool.rkt"
                     #:args (filename) filename))
