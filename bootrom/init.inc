;----------------------------------------------------------------------------
; init.inc --
;----------------------------------------------------------------------------

                        org     ROM_BASE

                        ; Vector table
                        dc.l    STACK_TOP       ; ISP
                        dc.l    __reset         ; IPC
i                       =       2
                        rept    254
                        dc.l    ISR_BASE+(i*6)+(i<<24)
i                       =       i+1
                        endr

                        ; Parameter block
                        cnop    0, $400
checksum                dc.l    0

                        ; Startup code
                        cnop    0, $500
__reset:
                        ; Startup delay
                        rept    256
                        nop
                        endr

                        ; Disable interrupts
                        move.w  #$2700, sr

                        ; Set USP and ISP
                        lea     STACK_TOP, a7
                        move    a7, usp

                        ; Clear registers
                        moveq   #$00, d0
                        moveq   #$00, d1
                        moveq   #$00, d2
                        moveq   #$00, d3
                        moveq   #$00, d4
                        moveq   #$00, d5
                        moveq   #$00, d6
                        moveq   #$00, d7
                        move.l  d0, a0
                        move.l  d0, a1
                        move.l  d0, a2
                        move.l  d0, a3
                        move.l  d0, a4
                        move.l  d0, a5
                        move.l  d0, a6

                        ; Disable target device interface
                        moveq   #0, d0
                        move.w  d0, (IX).w
                        move.w  d0, (RX).w
                                         
                        ; Clear work RAM
                        lea     RAM_BASE, a0
                        move.l  #(RAM_SIZE/(4*8))-1, d0
                        moveq   #0, d1
        @clrwram:       rept    8
                        move.l  d1, (a0)+
                        endr
                        dbra    d0, @clrwram

                        ; Set up exception handling
                        jsr     ex_init

                        ; Run main program
                        jmp     main

;----------------------------------------------------------------------------
; End
;----------------------------------------------------------------------------
