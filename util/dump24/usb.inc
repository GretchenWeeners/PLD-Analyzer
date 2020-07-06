;----------------------------------------------------------------------------
; usb.inc --
; DLP-245M USB functions
;----------------------------------------------------------------------------

; Send byte to host
; d0 = data
usb_sendb:
                        tst.b   (TXE).w
                        bmi.s   usb_sendb
                        move.b  d0, (USB_DATA).w
                        rts
; Send word to host
; d0 = data
usb_sendw:
                        move.w  d0, -(a7)
                        bsr.s   usb_sendb
                        lsr.w   #8, d0
                        bsr.s   usb_sendb
                        move.w  (a7)+, d0
                        rts
; Send long to host
; d0 = data
usb_sendl:
                        move.l  d0, -(a7)
                        bsr.s   usb_sendb
                        lsr.l   #8, d0
                        bsr.s   usb_sendb
                        lsr.l   #8, d0
                        bsr.s   usb_sendb
                        lsr.l   #8, d0
                        bsr.s   usb_sendb
                        move.l  (a7)+, d0
                        rts
; Get byte from host
; d0 = data
usb_getb:
                        tst.b   (RXF).w
                        bmi.s   usb_getb
                        move.b  (USB_DATA).w, d0
                        rts
; Get word from host
; d0 = data
usb_getw:
                        bsr.s   usb_getb
                        lsl.w   #8, d0
                        bsr.s   usb_getb
                        rts
; Get long from host
; d0 = data
usb_getl:
                        bsr.s   usb_getb
                        lsl.l   #8, d0
                        bsr.s   usb_getb
                        lsl.l   #8, d0
                        bsr.s   usb_getb
                        lsl.l   #8, d0
                        bsr.s   usb_getb
                        rts

;----------------------------------------------------------------------------
; Get data from host
;----------------------------------------------------------------------------
; a0 = destination data buffer
; d0 = length of data in bytes
;----------------------------------------------------------------------------
usb_get:
                        push    a1-a2/d1
                        move.l  d0, d1
                        lea     (RXF).w, a1
                        lea     (USB_DATA).w, a2
        @poll:          tst.b   (a1)
                        bmi.s   @poll
                        move.b  (a2), (a0)+
                        subq.l  #1, d1
                        bne.s   @poll
                        pop     a1-a2/d1
                        rts

;----------------------------------------------------------------------------
; Send data to host
;----------------------------------------------------------------------------
; a0 = source data buffer
; d0 = length of data in bytes
;----------------------------------------------------------------------------
usb_send:
                        push    a1-a2/d1
                        move.l  d0, d1
                        lea     (TXE).w, a1
                        lea     (USB_DATA).w, a2
        @poll:          tst.b   (a1)
                        bmi.s   @poll
                        move.b  (a0)+, (a2)
                        subq.l  #1, d1
                        bne.s   @poll
                        pop     a1-a2/d1
                        rts

;----------------------------------------------------------------------------
; Get data from host (with checksum)
;----------------------------------------------------------------------------
; a0 = destination data buffer
; d0 = length of data in bytes
; Carry flag is set on a checksum mismatch
;----------------------------------------------------------------------------
usb_get_checksum:
                        push    a1-a2/d1-d3
                        move.l  d0, d1
                        lea     (RXF).w, a1
                        lea     (USB_DATA).w, a2
                        moveq   #0, d3

                        ; Get data and generate checksum
        @poll:          tst.b   (a1)
                        bmi.s   @poll
                        move.b  (a2), d2
                        add.b   d2, d3
                        move.b  d2, (a0)+
                        subq.l  #1, d1
                        bne.s   @poll

                        ; Send our checksum
                        move.b  d3, d0
                        jsr     usb_sendb

                        ; Get client checksum
                        jsr     usb_getb

                        ; Check if they match
                        cmp.b   d3, d0
                        beq.s   @match
                        ori.b   #1, ccr
                        bra.s   @done
        @match:
                        andi.b  #~1, ccr
        @done:
                        pop     a1-a2/d1-d3
                        rts

;----------------------------------------------------------------------------
; Send data to host (with checksum)
;----------------------------------------------------------------------------
; a0 = source data buffer
; d0 = length of data in bytes
; Carry flag is set on a checksum mismatch
;----------------------------------------------------------------------------
usb_send_checksum:
                        push    a1-a2/d1-d3
                        move.l  d0, d1
                        lea     (TXE).w, a1
                        lea     (USB_DATA).w, a2
                        moveq   #0, d3

                        ; Send data
        @poll:          tst.b   (a1)
                        bmi.s   @poll
                        move.b  (a0)+, d2
                        nop                     ; bus value
                        add.b   d2, d3
                        move.b  d2, (a2)
                        subq.l  #1, d1
                        bne.s   @poll

                        ; Send our checksum
                        move.b  d3, d0
                        jsr     usb_sendb

                        ; Get host checksum
                        jsr     usb_getb

                        ; Check if they match
                        cmp.b   d3, d0
                        beq.s   @match
                        ori.b   #1, ccr
                        bra.s   @done
        @match:
                        andi.b  #~1, ccr
        @done:
                        pop     a1-a2/d1-d3
                        rts

;----------------------------------------------------------------------------
; End
;----------------------------------------------------------------------------
