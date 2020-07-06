;----------------------------------------------------------------------------
; global.inc
;----------------------------------------------------------------------------

ROM_BASE                =       $000000
ROM_SIZE                =       $080000 ; 512K

RAM_BASE                =       $400000
RAM_SIZE                =       $100000 ; 1MB

EXT_BASE                =       $800000
EXT_SIZE                =       $080000 ; 512K

REGBASE                 =       $FFFF8000

ENTRY_POINT             =       $500000
VAR_BASE                =       $480000
STACK_TOP               =       $507800
ISR_BASE                =       $507800
ISR_SIZE                =       $800
EX_GLOBAL_JMP           =       $507F00


; Register offsets from base
R_IX                    =       $00
R_IXH                   =       $00
R_IXL                   =       $01
R_RX                    =       $02
R_RXH                   =       $02
R_RXL                   =       $03
R_BX                    =       $02
R_BXH                   =       $02
R_BXL                   =       $03
R_DATA                  =       $04
R_FLUSH_TX              =       $05

R_ROMWS0                =       $08
R_ROMWS1                =       $09
R_RAMWS0                =       $0A
R_RAMWS1                =       $0B
R_EXTWS0                =       $0C
R_EXTWS1                =       $0D
R_PIOWS0                =       $0E
R_PIOWS1                =       $0F

R_OUT                   =       $10

R_RXF                   =       $18
R_FLT                   =       $19
R_OC                    =       $1A
R_OUT1                  =       $1B
R_SDO                   =       $1C
R_OUT2                  =       $1D
R_RST                   =       $1E
R_TXE                   =       $1F

; BXH bits
B_SDO                   =       7
B_SDI                   =       6
B_MODE                  =       5
B_SCLK                  =       4
B_ENUM                  =       3
B_TVR                   =       2

; RXH bits
B_FAULT_LED             =       7
B_ENUM_LED              =       6

; BXH/RXH bits
B_B9                    =       1
B_B8                    =       0

; Direct register addresses
RX                      =       (REGBASE+R_RX)
RXH                     =       (REGBASE+R_RXH)
RXL                     =       (REGBASE+R_RXL)

BX                      =       (REGBASE+R_BX)
BXL                     =       (REGBASE+R_BXL)
BXH                     =       (REGBASE+R_BXH)

IX                      =       (REGBASE+R_IX)
IXL                     =       (REGBASE+R_IXL)
IXH                     =       (REGBASE+R_IXH)

USB_DATA                =       (REGBASE+R_DATA)
USB_FLUSH_TX            =       (REGBASE+R_FLUSH_TX)

ROMWS0                  =       (REGBASE+R_ROMWS0)
ROMWS1                  =       (REGBASE+R_ROMWS1)
RAMWS0                  =       (REGBASE+R_RAMWS0)
RAMWS1                  =       (REGBASE+R_RAMWS1)
EXTWS0                  =       (REGBASE+R_EXTWS0)
EXTWS1                  =       (REGBASE+R_EXTWS1)
PIOWS0                  =       (REGBASE+R_PIOWS0)
PIOWS1                  =       (REGBASE+R_PIOWS1)

OUT                     =       (REGBASE+R_OUT)

RXF                     =       (REGBASE+R_RXF)
SDO                     =       (REGBASE+R_SDO)
OC                      =       (REGBASE+R_OC)
RST                     =       (REGBASE+R_RST)
FLT                     =       (REGBASE+R_FLT)
OUT2                    =       (REGBASE+R_OUT2)
OUT1                    =       (REGBASE+R_OUT1)
TXE                     =       (REGBASE+R_TXE)

;----------------------------------------------------------------------------
; Macros
;----------------------------------------------------------------------------

delay                   macro   1
                        rept    \1
                        nop
                        endr
                        endm

push                    macro   1
                        move.l  \1, -(a7)
                        endm

pop                     macro   1
                        move.l  (a7)+, \1
                        endm

;----------------------------------------------------------------------------
; END
;----------------------------------------------------------------------------
