;; Exec
ABSEXECBASE		EQU	4
_LVOOpenLibrary		EQU	-552
_LVOCloseLibrary	EQU	-414
;; DOS
_LVOOutput		EQU	-60
_LVOWrite		EQU	-48

start:
	move.l	4.w,a6
	lea	dosname(pc),a1
	moveq	#0,d0
	jsr	_LVOOpenLibrary(a6)
	tst.l	d0
	beq.s	nodos
	move.l	d0,a6
	lea	msg(pc),a0
	moveq	#-1,d3
	move.l	a0,d2
strlen:
	addq.l	#1,d3
	tst.b	(a0)+
	bne.s	strlen
	jsr	_LVOOutput(a6)
	move.l	d0,d1
	jsr	_LVOWrite(a6)
	move.l	a6,a1
	move.l	4.w,a6
	jsr	_LVOCloseLibrary(a6)
nodos:
	moveq	#0,d0
	rts
dosname:
	dc.b	'dos.library',0
msg:
	dc.b	'Hello, world!',10,0
