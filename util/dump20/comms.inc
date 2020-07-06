;----------------------------------------------------------------------------
; comms.inc --
;----------------------------------------------------------------------------

; Run dispatch function if there is incoming data
comms_check_dispatch:
                        tst.b   (RXF).w
                        bpl.s   comms_dispatch
                        rts

; Host command dispatch
comms_dispatch:
                        moveq   #0, d0
                        jsr     usb_getb
                        andi.b  #$07, d0
                        add.b   d0, d0
                        add.b   d0, d0
                        lea     @dispatch_table(pc, d0.w), a0
                        move.l  (a0), a0
                        jsr     (a0)
                        rts

;----------------------------------------------------------------------------
; Data
;----------------------------------------------------------------------------

                        align   4
@dispatch_table:
                        dc.l    cmd_nop
                        dc.l    cmd_echo
                        dc.l    cmd_download
                        dc.l    cmd_upload
                        dc.l    cmd_exec
                        dc.l    cmd_reset
                        dc.l    cmd_nop
                        dc.l    cmd_fault

;----------------------------------------------------------------------------
; Dispatch command table
;----------------------------------------------------------------------------

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
                        rts

;----------------------------------------------------------------------------
; End
;----------------------------------------------------------------------------

