mainloop:
waitras1:
        cmp.b   #$ac,$dff006
        bne     waitras1
        move.w  #$fff,$dff180

waitras2:
        cmp.b   #$ac,$dff006
        beq     waitras2
        move.w  #$116,$dff180
        
        btst	  #6,$bfe001
	      bne	    mainloop
	      rts
	
