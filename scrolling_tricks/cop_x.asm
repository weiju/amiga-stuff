	
	SECTION copperlist,DATA,CHIP

	XDEF	_CopperList
	XDEF	_CopFETCHMODE
	XDEF	_CopBPLCON0
	XDEF	_CopBPLCON1
	XDEF	_CopBPLCON3
	XDEF	_CopBPLMODA
	XDEF	_CopBPLMODB
	XDEF	_CopDIWSTART
	XDEF	_CopDIWSTOP
	XDEF	_CopDDFSTART
	XDEF	_CopDDFSTOP
	XDEF	_CopPLANE1H
	XDEF	_CopPLANE1L
	XDEF	_CopPLANE2H
	XDEF	_CopPLANE2L
	XDEF	_CopPLANE3H
	XDEF	_CopPLANE3L
	XDEF	_CopPLANE4H
	XDEF	_CopPLANE4L
	XDEF	_CopPLANE5H
	XDEF	_CopPLANE5L
	XDEF	_CopPLANE6H
	XDEF	_CopPLANE6L
	XDEF	_CopPLANE7H
	XDEF	_CopPLANE7L
	XDEF	_CopPLANE8H
	XDEF	_CopPLANE8L
	XDEF	_CopSKY

_CopperList:
	dc.w	$180,0

_CopFETCHMODE:
	dc.w	$1FC,0

_CopBPLCON0:
	dc.w	$100,0

_CopBPLCON1:
	dc.w	$102,0

_CopBPLCON3:
	dc.w	$106,0

_CopBPLMODA:
	dc.w	$108,0
	
_CopBPLMODB:
	dc.w	$10A,0
	
_CopDIWSTART:
	dc.w	$8e,0
	
_CopDIWSTOP:
	dc.w	$90,0
	
_CopDDFSTART:
	dc.w	$92,0
	
_CopDDFSTOP:
	dc.w	$94,0

_CopPLANE1H:
	dc.w	$e0,0

_CopPLANE1L:
	dc.w	$e2,0

_CopPLANE2H:
	dc.w	$e4,0

_CopPLANE2L:
	dc.w	$e6,0

_CopPLANE3H:
	dc.w	$e8,0

_CopPLANE3L:
	dc.w	$ea,0

_CopPLANE4H:
	dc.w	$ec,0

_CopPLANE4L:
	dc.w	$ee,0

_CopPLANE5H:
	dc.w	$f0,0

_CopPLANE5L:
	dc.w	$f2,0

_CopPLANE6H:
	dc.w	$f4,0

_CopPLANE6L:
	dc.w	$f6,0

_CopPLANE7H:
	dc.w	$f8,0

_CopPLANE7L:
	dc.w	$fa,0

_CopPLANE8H:
	dc.w	$fc,0

_CopPLANE8L:
	dc.w	$fe,0

_CopSKY:
	dc.w	-1,-2
	dc.w	$180,$FFF

	dc.w	$390f,-2
	dc.w	$180,$EEF
	dc.w	$3A0f,-2
	dc.w	$180,$FFF
	dc.w	$3B0f,-2
	dc.w	$180,$EEF

	dc.w	$490f,-2
	dc.w	$180,$DDF
	dc.w	$4a0f,-2
	dc.w	$180,$EEF
	dc.w	$4b0f,-2
	dc.w	$180,$DDF

	dc.w	$590f,-2
	dc.w	$180,$CCF
	dc.w	$5A0f,-2
	dc.w	$180,$DDF
	dc.w	$5B0f,-2
	dc.w	$180,$CCF

	dc.w	$690f,-2
	dc.w	$180,$BBF
	dc.w	$6A0f,-2
	dc.w	$180,$CCF
	dc.w	$6B0f,-2
	dc.w	$180,$BBF

	dc.w	$790f,-2
	dc.w	$180,$AAF
	dc.w	$7A0f,-2
	dc.w	$180,$BBF
	dc.w	$7B0f,-2
	dc.w	$180,$AAF

	dc.w	$890f,-2
	dc.w	$180,$99E
	dc.w	$8A0f,-2
	dc.w	$180,$AAF
	dc.w	$8B0f,-2
	dc.w	$180,$99E

	dc.w	$990f,-2
	dc.w	$180,$88E
	dc.w	$9A0f,-2
	dc.w	$180,$99E
	dc.w	$9B0f,-2
	dc.w	$180,$88E

	dc.w	$A90f,-2
	dc.w	$180,$77E
	dc.w	$AA0f,-2
	dc.w	$180,$88E
	dc.w	$AB0f,-2
	dc.w	$180,$77E

	dc.w	$B90f,-2
	dc.w	$180,$66E
	dc.w	$BA0f,-2
	dc.w	$180,$77E
	dc.w	$BB0f,-2
	dc.w	$180,$66E

	dc.w	$C90f,-2
	dc.w	$180,$55D
	dc.w	$CA0f,-2
	dc.w	$180,$66E
	dc.w	$CB0f,-2
	dc.w	$180,$55D

	dc.w	$D90f,-2
	dc.w	$180,$44D
	dc.w	$DA0f,-2
	dc.w	$180,$55D
	dc.w	$DB0f,-2
	dc.w	$180,$44D

	dc.w	$E90f,-2
	dc.w	$180,$33D
	dc.w	$EA0f,-2
	dc.w	$180,$44D
	dc.w	$EB0f,-2
	dc.w	$180,$33D

	dc.w	$F90f,-2
	dc.w	$180,$22C
	dc.w	$FA0f,-2
	dc.w	$180,$33D
	dc.w	$FB0f,-2
	dc.w	$180,$22C

	dc.w	$FFDF,-2
	dc.w	$090F,-2
	dc.w	$180,$11C
	dc.w	$0A0F,-2
	dc.w	$180,$22C
	dc.w	$0B0F,-2
	dc.w	$180,$11C

	dc.w	$190f,-2
	dc.w	$180,$00B
	dc.w	$1A0f,-2
	dc.w	$180,$11C
	dc.w	$1B0f,-2
	dc.w	$180,$00B

	dc.w	-1,-2
	END

