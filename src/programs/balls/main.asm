INCLUDE "constants.asm"

SECTION "rst 00", ROM0 [$00]
	jp Init
	DS $5

SECTION "rst 08", ROM0 [$08]
	jp Init
	DS $5

SECTION "rst 10", ROM0 [$10]
	rst $38
	DS $7

SECTION "rst 18", ROM0 [$18]
	rst $38
	DS $7

SECTION "rst 20", ROM0 [$20]
	rst $38
	DS $7

SECTION "rst 28", ROM0 [$28]
	rst $38
	DS $7

SECTION "rst 38", ROM0 [$38]
	jp Crash
	DS $5

SECTION "vblank", ROM0 [$40]
	jp VBlank
	DS $5

SECTION "hblank", ROM0 [$48]
	jp HBlank
	DS $5

SECTION "timer",  ROM0 [$50]
	jp Timer
	DS $5

SECTION "serial", ROM0 [$58]
	jp Serial
	DS $5

SECTION "Entry", ROM0 [$100]
	nop
	jp Start

SECTION "Header", ROM0 [$104]
	db $ce, $ed, $66, $66, $cc, $0d, $00, $0b, $03, $73, $00, $83, $00, $0c, $00, $0d,
	db $00, $08, $11, $1f, $88, $89, $00, $0e, $dc, $cc, $6e, $e6, $dd, $dd, $d9, $99,
	db $bb, $bb, $67, $63, $6e, $0e, $ec, $cc, $dd, $dc, $99, $9f, $bb, $b9, $33, $3e, 
	db "BALLS!", $00, $00, $00, $00, $00, $00, $00, $00, $00
	db $00		; dmg - classic gameboy
	db $00, $00	; new license
	db $00		; sgb flag: not sgb compatible
	db $00		; cart type: rom
	db $00		; rom size: 32 kb
	db $00		; ram size: 0 b
	db $00		; destination code: japanese
	db $01		; old license: not sgb compatible
	db $01		; mask rom version number
	db $0a		; header check [ok]
	db $16, $bf	; global check [ok]

SECTION "Main", ROM0 [$150]
Start::
	call Init
	jp Main_Loop

Serial::
	reti

VBlank::
	; clear IF bit if needed
	reti

HBlank::	
	reti

Timer::	
	reti

Crash::
	jr Crash

Init::
	di
	; Disable LCD
	; Clear VRAM / WRAM
	
	; Enable interrupts
	; Enable LCD
	ld a, $00
	ld d, $10
	call scroll_up
	call set_palettes
	call load_tiles
	call load_tilemap
	ld a, $54
	call scroll_down
	call wait_pause
	call load_tilemap_main
	ld hl, $c000
	ld a, 50
	ldi [hl], a
	ld [hl], a
	call draw_ball_sprite
	ret

TILESET_START:
	incbin "graphics/tileset.bin"
TILESET_END:

TILEMAP_START:
	incbin "graphics/tilemap.bin"
TILEMAP_END:

TILEMAP_MAIN_START:
	incbin "graphics/tilemap_main.bin"
TILEMAP_MAIN_END:

set_palettes::
	push af
	ld a, $E4
	ld [$FF47], a
	pop af
	ret

wait_pause::
	ld c, $0C
waiting_c::
	ld b, $FF
waiting_b::
	ld a, $FF
waiting_a::
	dec a
	jr nz, waiting_a
	dec b
	jr nz, waiting_b
	dec c
	jr nz, waiting_c
	ret

scroll_up::
	cp $54
	jr z, done_up
	ld c, $0F
wait_c::
	ld b, $FF
wait_b::
	dec b
	jr nz, wait_b
	dec c
	jr nz, wait_c
	inc a
	ld [rSCY], a
	jp nz, scroll_up
done_up::
	ret

scroll_down::
	cp $00
	jr z, done_up
	ld c, $0F
wait_down_c::
	ld b, $FF
wait_down_b::
	dec b
	jr nz, wait_down_b
	dec c
	jr nz, wait_down_c
	dec a
	ld [rSCY], a
	jp nz, scroll_down
done_down::
	ret

load_tiles::
	ld hl, TILESET_START ; address of tileset in ROM
	ld bc, TILESET_END - TILESET_START ; length of data set
	ld de, $8000 ; Starting address of tile data in VRAM
.loop_tile:
	ldi a, [hl]
	ld [de], a
	inc de
	dec bc
	ld a, b
	or c
	jr nz, .loop_tile
	ret

load_tilemap::
	ld hl, TILEMAP_START
	ld bc, TILEMAP_END - TILEMAP_START
	ld de, $9800
.loop_tilemap:
	ldi a, [hl]
	ld [de], a
	inc de
	dec bc
	ld a, b
	or c
	jr nz, .loop_tilemap
	ret

load_tilemap_main::
	ld hl, TILEMAP_MAIN_START
	ld bc, TILEMAP_MAIN_END - TILEMAP_MAIN_START
	ld de, $9800
.loop_tilemap_main:
	ldi a, [hl]
	ld [de], a
	inc de
	dec bc
	ld a, b
	or c
	jr nz, .loop_tilemap_main
	ret

draw_ball_sprite::
    ; Load position
    ld hl, $C000
    ldi a, [hl]     ; A = Y
    ld d, a         ; D = base Y
    ld a, [hl]      ; A = X
    ld e, a         ; E = base X

    ld hl, $FE00    ; OAM start

    ; ----------------
    ; Sprite 0 (top-left)
    ; ----------------
    ld a, d
    ldi [hl], a     ; Y
    ld a, e
    ldi [hl], a     ; X
    ld a, $09
    ldi [hl], a     ; tile
    xor a
    ldi [hl], a     ; attrs

    ; ----------------
    ; Sprite 1 (top-right)
    ; ----------------
    ld a, d
    ldi [hl], a     ; Y
    ld a, e
    add 8
    ldi [hl], a     ; X + 8
    ld a, $0B
    ldi [hl], a
    xor a
    ldi [hl], a

    ; ----------------
    ; Sprite 2 (bottom-left)
    ; ----------------
    ld a, d
    add 8
    ldi [hl], a     ; Y + 8
    ld a, e
    ldi [hl], a     ; X
    ld a, $0A
    ldi [hl], a
    xor a
    ldi [hl], a

    ; ----------------
    ; Sprite 3 (bottom-right)
    ; ----------------
    ld a, d
    add 8
    ldi [hl], a     ; Y + 8
    ld a, e
    add 8
    ldi [hl], a     ; X + 8
    ld a, $0C
    ldi [hl], a
    xor a
    ldi [hl], a

    ret

Animate:
	ld hl, $c001
	ld a, [hl]
	inc a
	ld [hl], a
	ret

Main_Loop::
	ld b, $F
waiting_main_b::
	ld c, $FF
waiting_main_c::
	dec c
	jr nz, waiting_main_c
	dec b
	jr nz, waiting_main_b
	call Animate
	call draw_ball_sprite
	jr Main_Loop