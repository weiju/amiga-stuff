init:
        move    #$ac,d7
	
mainloop:
wframe:
        cmp.b   #$ff,$dff006
        bne     wframe
        
        add     #1,d7
waitras1:
        cmp.b   $dff006,d7
        bne     waitras1
        move.w  #$fff,$dff180

waitras2:
        cmp.b   $dff006,d7
        beq     waitras2
        move.w  #$116,$dff180
        
        btst	#6,$bfe001
        bne	mainloop
        rts
	
