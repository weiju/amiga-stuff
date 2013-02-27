;; This is a small test for compiling to 68000 assembly
;; note that there is no space allowed in the operand field
start:
        move.l  #5,d0
        add.l   #1,d0
        sub.l   #2,d0
        jsr     label1
        rts

label1:
	moveq  #1,d0
        rts

