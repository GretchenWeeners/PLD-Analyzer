;----------------------------------------------------------------------------
; Test program
;----------------------------------------------------------------------------

                        include "global.inc"
                        include "macros.inc"

                        rsset   VAR_BASE
count                   rs.l    1

;----------------------------------------------------------------------------
; Vector table
;----------------------------------------------------------------------------

                        include "init.inc"

;----------------------------------------------------------------------------
; Main program
;----------------------------------------------------------------------------
main:
                        move.w  #$2700, sr

                        ; Wait for enumeration
        @wait:          btst    #3, (BXH).w
                        bne.s   @wait

                        ; Enumerate OK
                        move.b  #$40, (RXH).w

                        ;--------------------------------------------------
                        ; Main loop
                        ;--------------------------------------------------
    @main_loop:
                        jsr     comms_dispatch
                        bra.s   @main_loop

;----------------------------------------------------------------------------
; Comms functions
;----------------------------------------------------------------------------

comms_check_dispatch:
                        tst.b   (RXF).w
                        bpl.s   comms_dispatch
                        rts
comms_dispatch:
                        moveq   #0, d0
                        jsr     usb_getb
                        andi.b  #$0F, d0
                        add.b   d0, d0
                        add.b   d0, d0
                        lea     @dispatch_table(pc, d0.w), a0
                        move.l  (a0), a0
                        jsr     (a0)
                        rts

                        align   2
@dispatch_table:
                        dc.l    cmd_nop
                        dc.l    cmd_echo
                        dc.l    cmd_download
                        dc.l    cmd_upload
                        dc.l    cmd_exec
                        dc.l    cmd_reset
                        dc.l    cmd_nop
                        dc.l    cmd_fault

                        dc.l    cmd_read_20
                        dc.l    cmd_nop
                        dc.l    cmd_nop
                        dc.l    cmd_nop
                        dc.l    cmd_nop
                        dc.l    cmd_nop
                        dc.l    cmd_nop
                        dc.l    cmd_nop

                        align   2
; Do nothing
cmd_nop:
                        nop
                        rts
; Echo test
cmd_echo:
                        jsr     usb_getb
                        jsr     usb_sendb
                        cmp.b   #27, d0
                        bne.s   cmd_echo
                        rts
; Send data to host
; A0 = address
; D0 = length
cmd_download:
                        jsr     usb_getl
                        move.l  d0, a0
                        jsr     usb_getl
                        jsr     usb_send_checksum
                        rts
; Get data from host
; A0 = address
; D0 = length
cmd_upload:
                        jsr     usb_getl
                        move.l  d0, a0
                        jsr     usb_getl
                        jsr     usb_get_checksum
                        rts
; Jump to address
; A0 = address
cmd_exec:
                        jsr     usb_getl
                        move.l  d0, a0
                        jmp     (a0)

; Software reset
cmd_reset:
                        lea     (ROM_BASE).w, a0
                        move.l  (a0)+, a7
                        move    a7, usp
                        move.l  (a0)+, a0
                        nop
                        jmp     (a0)

; Trigger double bus fault
cmd_fault:
                        moveq   #-1, d0
                        move.l  d0, a7
                        move    a7, usp
                        tst.l   (a7)
        @loop:          stop    #$2700
                        bra.s   @loop

;----------------------------------------------------------------------------
; USB functions
;----------------------------------------------------------------------------

usb_sendb:
                        tst.b   (TXE).w
                        bmi.s   usb_sendb
                        move.b  d0, (USB_DATA).w
                        rts
usb_sendw:
                        bsr.s   usb_sendb
                        lsr.w   #8, d0
                        bsr.s   usb_sendb
                        rts
usb_sendl:
                        bsr.s   usb_sendb
                        lsr.l   #8, d0
                        bsr.s   usb_sendb
                        lsr.l   #8, d0
                        bsr.s   usb_sendb
                        lsr.l   #8, d0
                        bsr.s   usb_sendb
                        rts
usb_getb:
                        tst.b   (RXF).w
                        bmi.s   usb_getb
                        move.b  (USB_DATA).w, d0
                        rts
usb_getw:
                        bsr.s   usb_getb
                        lsl.w   #8, d0
                        bsr.s   usb_getb
                        rts
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
; a0 = destination
; d0 = byte length
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
; a0 = source data
; d0 = byte length
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
; a0 = destination
; d0 = byte length
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
; a0 = source data
; d0 = byte length
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
; Test
;----------------------------------------------------------------------------

RESULT_BUF              =       $480000



cmd_read_20:
                        ; Turn power off
                        move.b  #$00, (OUT).w

                        ; Wait for power off
        @wtvrl:         btst    #2, (BXH).w
                        bne.s   @wtvrl

                        ; Turn power on
                        move.b  #$01, (OUT).w

                        ; Wait for power on
        @wtvrh:         btst    #2, (BXH).w
                        beq.s   @wtvrh

                        ; Short delay
                        move.l  #$3FFF, d0
        @wait:          nop
                        dbra    d0, @wait

                        ; Read device

                        lea     RESULT_BUF, a0

                        move.w  #$0000, d1      ; rx (00ff)
        @rxloop:
                        move.b  d1, (RXL).w     ; set rxl
                        move.w  #$0000, d0      ; ix (03ff)
        @ixloop:
                        move.w  d0, (IX).w      ; set ix latches
                        nop
                        move.b  (BXL).w, (a0)+   ; read bx outputs
                        addq.w  #1, d0
                        cmpi.w  #$0400, d0
                        bne.s   @ixloop

                        addq.w  #1, d1
                        cmpi.w  #$0100, d1
                        bne.s   @rxloop

                        ; Send data to host
                        move.l  #($400*$100)-1, d0
                        lea     RESULT_BUF, a0
                        jsr     usb_send

                        rts

;=============================================================================

; bxh upper (b8,b9,tvr,slp#,sclk,mode,sdi,sdo)
; rxh upper (b8,b9,x,x,pic,pic,led,led)
; ixh upper (sclk,mode,sdo,nc)

cmd_read_24:
                        ; Turn power off
                        move.b  #$00, (OUT).w

                        ; Wait for power off
        @wtvrl:         btst    #2, (BXH).w
                        bne.s   @wtvrl

                        ; Turn power on
                        move.b  #$01, (OUT).w

                        ; Wait for power on
        @wtvrh:         btst    #2, (BXH).w
                        beq.s   @wtvrh

                        ; Short delay
                        move.l  #$3FFF, d0
        @wait:          nop
                        dbra    d0, @wait

                        ; Read device (send in 4k packets)

                        move.w  #$8000, d1 ; rx (03ff)
        @rxloop:
                        move.w  d1, (RX).w

                        lea     RESULT_BUF, a0
                        move.w  #$0000, d0 ; ix (0fff)
        @ixloop:
                        move.w  d0, (IX).w      ; set ix latches
                        nop
                        move.w  (BX).w, (a0)+   ; read bx outputs
                        addq.w  #1, d0
                        cmpi.w  #$1000, d0
                        bne.s   @ixloop

                        ; Send data to host (8k packet)
                        move.l  #$2000-1, d0
                        lea     RESULT_BUF, a0
                        jsr     usb_send

                        addq.w  #1, d1
                        cmpi.w  #$8400, d1
                        bne.s   @rxloop

                        rts


;----------------------------------------------------------------------------
; Libraries
;----------------------------------------------------------------------------

                        include "ex.inc"

;----------------------------------------------------------------------------
; Data
;----------------------------------------------------------------------------

                        if      1

                        ; Data pattern for integrity checks
                        cnop    0, $2000
i                       =       $deadbeef
                        rept    ((ROM_SIZE-@)/4)
                        dc.l    i
i                       =       i+$deadbeef
                        endr

                        ; Pad to full device size
                        cnop    0, ROM_SIZE

                        endc

                        cnop    0, $2000

;----------------------------------------------------------------------------
; End
;----------------------------------------------------------------------------

