;----------------------------------------------------------------------------
; device.inc --
;----------------------------------------------------------------------------

                        align   2
device_init:
                        move.w  #$0000, (IX).w
                        move.w  #$0000, (RX).w
                        jsr     power_off
                        rts

;----------------------------------------------------------------------------
; Turn power off
;----------------------------------------------------------------------------
                        align   2
power_off:
                        push    d0

                        ; Turn off power
                        move.b  #PWR_OFF, (OUT).w
                       
                        ; Wait for OUT2=0, OUT1=1 (Vref < 1.40V)
        @wait_out1_hi:  tst.b   (OUT1).w
                        bpl.s   @wait_out1_hi
        @wait_out2_lo:  tst.b   (OUT2).w
                        bmi.s   @wait_out2_lo

                        ; Input MCP active low RESET# output
                        ; For power off, wait until LOW
        @wait_rst_lo:   tst.b   (RST).w
                        bmi.s   @wait_rst_lo

                        ; Input +5V rail directly
                        ; For power off, wait until LOW
        @wait_tvr_lo:   btst    #2, (BXH).w
                        bne.s   @wait_tvr_lo

                        ; Short delay
                        moveq   #$7F, d0
        @delay:         nop
                        dbra    d0, @delay

                        pop     d0
                        rts

;----------------------------------------------------------------------------
; Turn power off
;----------------------------------------------------------------------------
                        align   2
power_wait_off:
                        ; Wait for OUT2=0, OUT1=1 (Vref < 1.40V)
        @wait_out1_hi:  tst.b   (OUT1).w
                        bpl.s   @wait_out1_hi
        @wait_out2_lo:  tst.b   (OUT2).w
                        bmi.s   @wait_out2_lo

                        ; Input MCP active low RESET# output
                        ; For power off, wait until LOW
        @wait_rst_lo:   tst.b   (RST).w
                        bmi.s   @wait_rst_lo

                        ; Input +5V rail directly
                        ; For power off, wait until LOW
        @wait_tvr_lo:   btst    #2, (BXH).w
                        bne.s   @wait_tvr_lo

                        rts

;----------------------------------------------------------------------------
; Turn power on
;----------------------------------------------------------------------------
                        align   2
power_on:
                        push    d0

                        ; Turn on power
                        move.b  #PWR_ON, (OUT).w
                       
                        ; Wait for OUT2=1, OUT1=0 (Vref > 4.75V)
        @wait_out1_lo:  tst.b   (OUT1).w
                        bmi.s   @wait_out1_lo
        @wait_out2_hi:  tst.b   (OUT2).w
                        bpl.s   @wait_out2_hi

                        ; Input MCP active low RESET# output (350ms)
                        ; For power on, wait until HIGH
        @wait_rst_hi:   tst.b   (RST).w
                        bpl.s   @wait_rst_hi

                        ; Input +5V rail directly
                        ; For power on, wait until HIGH
        @wait_tvr_hi:   btst    #2, (BXH).w
                        beq.s   @wait_tvr_hi

                        ; Short delay
                        moveq   #$7F, d0
        @delay:         nop
                        dbra    d0, @delay

                        pop     d0
                        rts

;----------------------------------------------------------------------------
; Turn power off
;----------------------------------------------------------------------------
                        align   2
power_off_fast:
                        ; Turn off power
                        move.b  #PWR_OFF, (OUT).w
                       
                        ; Wait for OUT2=0, OUT1=1 (Vref < 1.40V)
        @wait_out1_hi:  tst.b   (OUT1).w
                        bpl.s   @wait_out1_hi
        @wait_out2_lo:  tst.b   (OUT2).w
                        bmi.s   @wait_out2_lo

                        ; Input MCP active low RESET# output
                        ; For power off, wait until LOW
        @wait_rst_lo:   tst.b   (RST).w
                        bmi.s   @wait_rst_lo

                        ; Input +5V rail directly
                        ; For power off, wait until LOW
        @wait_tvr_lo:   btst    #2, (BXH).w
                        bne.s   @wait_tvr_lo

                        rts

;----------------------------------------------------------------------------
; Turn power on
;----------------------------------------------------------------------------
                        align   2
power_on_fast:
                        ; Turn on power
                        move.b  #PWR_ON, (OUT).w
                       
                        ; Wait for OUT2=1, OUT1=0 (Vref > 4.75V)
        @wait_out1_lo:  tst.b   (OUT1).w
                        bmi.s   @wait_out1_lo
        @wait_out2_hi:  tst.b   (OUT2).w
                        bpl.s   @wait_out2_hi

                        ; Input +5V rail directly
                        ; For power on, wait until HIGH
        @wait_tvr_hi:   btst    #2, (BXH).w
                        beq.s   @wait_tvr_hi

                        rts

;----------------------------------------------------------------------------
; End
;----------------------------------------------------------------------------
