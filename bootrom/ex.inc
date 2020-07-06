;----------------------------------------------------------------------------
; ex.inc --
;----------------------------------------------------------------------------

; Put list of JMPs to handlers in the ISR table
ex_init:
                        lea     ISR_BASE, a0
                        move.l  #256-1, d0
                        move.w  #$4EF9, d1
                        move.l  #EX_GLOBAL_JMP, d2
                        move.l  #$01000000, d3
        @fill_isr:      move.w  d1, (a0)+
                        move.l  d2, (a0)+
                        add.l   d3, d2
                        dbra    d0, @fill_isr

                        move.w  #$4EF9, EX_GLOBAL_JMP+0
                        move.l  #_ex_handler, EX_GLOBAL_JMP+2

                        rts

_ex_handler:
                        ; Disable interrupts
                        move.w  #$2700, sr
        @loop:
                        ; Turn fault LED on
                        move.b  #$80, (RXH).w

                        ; Delay
                        move.w  #$3FFF, d0
        @delay1:        nop
                        dbra    d0, @delay1

                        ; Turn fault LED off
                        move.b  #$00, (RXH).w

                        ; Delay
                        move.w  #$3FFF, d0
        @delay2:        nop
                        dbra    d0, @delay2

                        bra.s   @loop

;----------------------------------------------------------------------------
; End
;----------------------------------------------------------------------------
