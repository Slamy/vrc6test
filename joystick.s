.include        "zeropage.inc"

.include        "nes.inc"

.export _joystick_read
.export _joystick_state 

.segment "ZEROPAGE"

_joystick_state: .res 1

.segment "STARTUP"
; ------------------------------------------------------------------------
; READ: Read a particular joystick passed in A.
;

_joystick_read:
        and     #$01            ; Fix joystick number
        tay                     ; Joystick number (0,1) into Y

        lda     #1
        sta     APU_PAD1
        lda     #0
        sta     APU_PAD1

; Read joystick

        ldx     #8
@Loop:  lda     APU_PAD1,y
        ror     a
        ror     _joystick_state
        dex
        bne     @Loop

        rts

        
