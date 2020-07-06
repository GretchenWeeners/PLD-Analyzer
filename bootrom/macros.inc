;----------------------------------------------------------------------------
; macros.inc --
;----------------------------------------------------------------------------

delay                   macro
                        rept    \1
                        nop
                        endr
                        endm

sec                     macro
                        ori.b   #1, ccr
                        endm

clc                     macro
                        andi.b   #~1, ccr
                        endm

push                    macro   1
                        movem.l \1, -(a7)
                        endm

pop                     macro   1
                        movem.l (a7)+, \1
                        endm


printf                  macro

                        ; Save registers
                        push    d0/a0

                        ; Send string length
                        moveq   #(istre\@-istrs\@), d0
                        jsr     usb_sendb

                        ; Send string text
                        lea     istrs\@(pc), a0
                        jsr     usb_sends

                        ; Send parameter block length
                        moveq   #4*(narg-1), d0
                        jsr     usb_sendb

                        ; Send parameter block
                        lea     iparam\@(pc), a0
                        jsr     usb_sends

                        ; Skip following inline data
                        bra.s   iend\@

                        ; Define parameter #1 (string)
        istrs\@:        dc.b    \1
        istre\@:
                        align   2
        iparam\@:
                        ; Define parameter block
                        rept    narg-1
                        shift
                        dc.l    \1
                        endr
        iend\@:
                        pop     d0/a0
                        endm

;----------------------------------------------------------------------------
; End
;----------------------------------------------------------------------------
