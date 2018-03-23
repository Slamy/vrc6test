; startup code for cc65/ca65

.import _main
.export __STARTUP__:absolute=1
.import _NSF_PLAY
.import _NSF_INIT

; linker-generated symbols

.import __STACK_START__, __STACK_SIZE__
.include "zeropage.inc"

; definitions
PPU_CTRL      = $2000
PPU_MASK      = $2001
PPU_STATUS    = $2002
OAM_ADDRESS   = $2003
OAM_DMA       = $4014
APU_DMC       = $4010
APU_STATUS    = $4015
APU_FRAME_CTR = $4017

.segment "ZEROPAGE"

; no variables

.segment "HEADER"

; iNES header
; see http://wiki.nesdev.com/w/index.php/INES

.byte $4e, $45, $53, $1a ; "NES" followed by MS-DOS EOF
.byte $10                ; size of PRG ROM in 16 KiB units
.byte $01                ; size of CHR ROM in 8 KiB units
.byte $80                ; horizontal mirroring, mapper 024 (NROM)
.byte $10                ; mapper 024 (NROM)
.byte $00                ; size of PRG RAM in 8 KiB units
.byte $00                ; NTSC
.byte $00                ; unused
.res 5, $00              ; zero-filled

.segment "STARTUP"

; initialize RAM and jump to C main()
start:
    sei ; ignore IRQs
    cld ; disable decimal mode

    ; disable APU frame IRQs
    ldx #$40
    stx APU_FRAME_CTR

    ; setup stack
    ldx #$ff
    txs

    inx ; x = $00
    stx PPU_CTRL ; disable NMI
    stx PPU_MASK ; disable rendering
    stx APU_DMC  ; disable DMC IRQs

    ; If the user presses reset during vblank, the PPU may reset
    ; with the vblank flag still true. This has about a 1 in 13
    ; chance of happening on NTSC or 2 in 9 on PAL. Clear the
    ; flag now so the @vblankwait1 loop sees an actual vblank.
    bit PPU_STATUS

    ; First of two waits for vertical blank to make sure that the
    ; PPU has stabilized
@vblank_wait_1:
    bit PPU_STATUS
    bpl @vblank_wait_1

    ; We now have about 30,000 cycles to burn before the PPU stabilizes.

    stx APU_STATUS ; disable music channels

    ; We'll fill RAM with $00.
    txa
@clear_ram:
    sta $00,   x
    sta $0100, x
    sta $0300, x
    sta $0400, x
    sta $0500, x
    sta $0600, x
    sta $0700, x ; Remove this if you're storing reset-persistent data

    ; We skipped $0200, x on purpose. Usually, RAM page 2 is used for the
    ; display list to be copied to OAM. OAM needs to be initialized to
    ; $ef-$ff, not 0, or we'll get a bunch of garbage sprites at (0, 0).

    inx
    bne @clear_ram

	; Initialize OAM data in $0200 to have all y coordinates off-screen
	; (e.g. set every fourth byte starting at $0200 to $ef)
	lda #$ef
@clear_oam:
	sta $0200, x

	inx
    inx
	inx
	inx
	bne @clear_oam

    ; Second of two waits for vertical blank to make sure that the
    ; PPU has stabilized
@vblank_wait_2:
    bit PPU_STATUS
    bpl @vblank_wait_2

	; initialize PPU OAM
    stx OAM_ADDRESS ; $00
    lda #$02 ; use page $0200-$02ff
    sta OAM_DMA

    ; set the C stack pointer
    lda #<(__STACK_START__ + __STACK_SIZE__)
    sta sp
    lda #>(__STACK_START__+__STACK_SIZE__)
    sta sp+1

	lda PPU_STATUS ; reset the PPU latch

	;jsr _NSF_INIT
	
	jsr init_apu

	;cli
	
    jmp _main ; call into our C main()

init_apu:
        ; Init $4000-4013
        ldy #$13
@loop:  lda @regs,y
        sta $4000,y
        dey
        bpl @loop
 
        ; We have to skip over $4014 (OAMDMA)
        lda #$0f
        sta $4015
        lda #$40
        sta $4017
        
        rts
@regs:
        .byte $30,$08,$00,$00
        .byte $30,$08,$00,$00
        .byte $80,$00,$00,$00
        .byte $30,$00,$00,$00
        .byte $00,$00,$00,$00
        
        
; do nothing for interrupts
nmi:
	rti
	
irq:
	rti

	pha
	
	txa
	pha
	
	tya
	pha
	
	php
	
	;jsr _NSF_PLAY
	
	plp
	
	pla
	tay
	
	pla
	tax
	
	pla
	
    rti

.segment "RODATA"


.segment "CODE0"
.segment "CODE1"
.segment "CODE2"
.segment "CODE3"
.segment "CODE4"
.segment "CODE5"
.segment "CODE6"
.segment "CODE7"
.segment "CODE8"
.segment "CODE9"
.segment "CODEa"
.segment "CODEb"
.segment "CODEc"
.segment "CODEd"
.segment "CODEe"
.segment "CODEf"

; nothing yet

.segment "VECTORS"

; set interrupt vectors to point to handlers
.word nmi   ;$fffa NMI
.word start ;$fffc Reset
.word irq   ;$fffe IRQ

.segment "CHARS"

; include CHR ROM data
.incbin "sprites.chr"

