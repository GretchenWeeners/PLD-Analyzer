;----------------------------------------------------------------------------
; 20-pin combinatorial PAL dumping routine
;----------------------------------------------------------------------------
; D3 : TPS22960 SR level (0= 75uS rise time, 1= 660-uS rise time)
; D2 : TPS22960 power output (0= off, 1= on)
; D1 : TPS20x1B power output (0= off, 1= on)
; D0 : ST890C power output (0= off, 1= on)
;----------------------------------------------------------------------------
; Rev.4 fmt
; d0-d7 -> ix0-ix7 -> pins 1,2,3,4,5,6,7,8
; d0-d1 -> ix8-ix9 -> pins 9,11
; d0-d7 -> bx0-bx7 -> pins 19,18,17,16,15,14,13,12
; Rev.3 fmt
; d0-d7 -> ix0-ix7 -> pins 23,22,21,20,19,18,17,16
; d0-d1 -> ix8,ix9 -> pins 15,14
; 8 bit bx, 10-bit ix
; assuming only bx was swapped;
; write linear ix, then swap bx
; also means swap rx
; Rev.1 fmt
; d0-d7

PWR_ON                  =       $04
PWR_OFF                 =       $00

                        include "global.inc"

;----------------------------------------------------------------------------
; Variables
;----------------------------------------------------------------------------

                        rsset   $480000
device_buffer           rs.b    $040000 ; 256K

                        include "init.inc"
                        include "device.inc"
                        include "usb.inc"
                        include "comms.inc"

;----------------------------------------------------------------------------
; Main program
;----------------------------------------------------------------------------
main:
                        move.b  #$00, (RXH).w
                        jsr     read_20pin
                        move.b  #$C0, (RXH).w
                        rts

;----------------------------------------------------------------------------
; Device routines
;----------------------------------------------------------------------------

                        align   2
read_20pin:
                        jsr     device_init
                        jsr     power_off
                        jsr     power_on

                        ; Delay for power-on settling
                        move.w  #$7FFF, d0
    @waitloop:          nop
                        dbra    d0, @waitloop

                        ; RXH (write)                   BXH (read)
                        ; D0 : B8 resistor level        B8 pin level
                        ; D1 : B9 resistor level        B9 pin level
                        ; D2 : (N.C.)                   TVR
                        ; D3 : (N.C.)                   SLP# (ENUM)
                        ; D4 : PIC WDM                  SCLK (ISP)
                        ; D5 : PIC WDE                  MODE (ISP)
                        ; D6 : GRN LED                  SDI  (ISP)
                        ; D7 : RED LED                  SDO  (ISP)


                        lea     device_buffer, a0
                        lea     revlut(pc), a1

                        moveq   #0, d2                  ; clear index

                        move.w  #$0000, d1              ; initial RX
        @rxloop:
                        ; Assign bitswapped RX
                        move.b  (a1, d1.w), (RXL).w

                        move.w  #$0000, d0              ; initial IX
        @ixloop:
                        ; Set IX inputs
                        move.w  d0, (IX).w              ; set IX

                        ; Wait for settling time
                        delay   16

                        ; Read IX inputs
                        move.b  (BXL).w, d2

                        moveq   #$0F, d7
    @recheck:           cmp.b   (BXL).w, d2
                      ; bne     fault
                      ; dbra    d7, @recheck
                        
                        ; Bitswap and store
                        move.b  (a1, d2.w), (a0)+       ; bitswap & store

                        ; Next IX
                        addq.w  #1, d0
                        cmpi.w  #$0400, d0
                        bne.s   @ixloop

                        addq.w  #1, d1
                        cmpi.w  #$0100, d1
                        bne.s   @rxloop

                        jsr     power_off
                        rts

;----------------------------------------------------------------------------
; Subroutines
;----------------------------------------------------------------------------

wait:                   push    d0
        @loop:          nop
                        dbra    d0, @loop
                        pop     d0
                        rts

; RAM test
; a0 = base
; a1 = end
ramtest:                move.l  a0-a1, -(a7)
                        move.l  #$DEADBEEF, d0
                        move.l  d0, d1
        @ramfill:       rept    16
                        move.l  d0, (a0)+
                        add.l   d1, d0
                        endr
                        cmpa.l  a0, a1
                        bne.s   @ramfill
                        move.l  (a7)+, a0-a1
                        move.l  #$DEADBEEF, d0
                        move.l  d0, d1
        @ramchk:        rept    16
                        cmp.l   (a0)+, d0
                        bne.s   @error
                        add.l   d1, d0
                        endr
                        cmpa.l  a0, a1
                        bne.s   @ramchk
                        rts
        @error:         jmp     fault

; ROM test
romtest:
                        moveq   #0, d0
                        lea     $000000, a0
                        lea     $000400, a1
    @checklo:           rept    16
                        add.l   (a0)+, d0
                        endr
                        cmpa.l  a0, a1
                        bne.s   @checklo
                        lea     $100(a0), a0
                        move.l  ($400).w, a1 ; header: rom size
    @checkhi:           rept    16
                        add.l   (a0)+, d0
                        endr
                        cmpa.l  a0, a1
                        bne.s   @checkhi
                        cmp.l   ($0404).w, d0 ; header: checksum
                        beq.s   @good
                        jmp     fault
        @good:          rts


; Force double bus fault
fault:
                        moveq   #-1, d0
                        move.l  d0, a7
                        move    a7, usp
                        tst.l   (a7)
                        stop    #$2700

;----------------------------------------------------------------------------
; End of program
;----------------------------------------------------------------------------

                        align   256
revlut:                 incbin  "data\revlut.bin"

                        cnop    0, $2000

;----------------------------------------------------------------------------
; End
;----------------------------------------------------------------------------
