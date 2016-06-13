#lang racket
(require rsound)
(require rsound/draw)
(require ffi/vector)

(define (play-sound s n)
  (cond [(> n 0)
         (play s)
         (sleep 0.1)
         ;;(play (resample 2 sound))
         (play-sound s (- n 1))]))

(define (analyze-sound s)
  (let ([v (rsound-data s)])
    v))
         

(let ([sound (rs-read "/home/weiju/Desktop/Laser_Shoot7.wav")])
  ;;(rsound-fft-draw sound)
  (play-sound sound 3)
  (play-sound sound 3)
  ;;(s16vector->list (analyze-sound sound))
  )
