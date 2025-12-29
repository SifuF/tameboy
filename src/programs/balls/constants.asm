; Palettes
PALETTE_1  EQU %11100100   ;00 = white, 01 = light grey, 10 = dark grey, 11 = black (read RTL)
PALETTE_2  EQU %11000100   ;white, light grey, white, black

; Initializing
SP_INIT  EQU $cfff ; Initial location of Stack Pointer

; Screen constants
SCREEN_HEIGHT EQU 144  ; Visible Pixels before VBlank ($90)
SCREEN_WIDTH  EQU 160  ; Visible Pixels before HBlank ($A0)
LCDC_ON       EQU $80  ; Turn LCDC on
LCDC_STANDARD EQU $d3  ; LCDC, BG, Sprites on, Window Tile Map starts at $9c00, 
                       ; BG & Window Tile Data starts at $8000,
                       ; BG Tile Map Display starts at $9800,
                       ; OBJ (Sprite) Size is set to 8x8 pixels

; Joypad constants
BTN_RIGHT  EQU 4  ; Directional Right
BTN_LEFT   EQU 5  ; Directional Left
BTN_UP     EQU 6  ; Directional Up
BTN_DOWN   EQU 7  ; Directional Down
BTN_A      EQU 0  ; Button A
BTN_B      EQU 1  ; Button B
BTN_SELECT EQU 2  ; Button Select
BTN_START  EQU 3  ; Button Start

; Sound constants
SOUND_ON   EQU $80
USE_ALL_CHANNELS   EQU $FF ; Set all audio channels to both output terminals (stereo)
MASTER_VOLUME_MAX  EQU $77 ; Set both output terminals to highest volume
ENVELOPE_NO_SOUND  EQU $08 ; Sets an envelope to no sound and direction to "increase"

; Hardware registers
rMBC        EQU $2000 ; MBC Controller - Select ROM bank 0 (not needed in Tetris)
rJOYP       EQU $ff00 ; Joypad (R/W)
rSB         EQU $ff01 ; Serial transfer data (R/W)
rSC         EQU $ff02 ; Serial Transfer Control (R/W)
rSC_ON    EQU 7
rSC_CGB   EQU 1
rSC_CLOCK EQU 0
rDIV        EQU $ff04 ; Divider Register (R/W)
rTIMA       EQU $ff05 ; Timer counter (R/W)
rTMA        EQU $ff06 ; Timer Modulo (R/W)
rTAC        EQU $ff07 ; Timer Control (R/W)
rTAC_ON        EQU 2
rTAC_4096_HZ   EQU 0
rTAC_262144_HZ EQU 1
rTAC_65536_HZ  EQU 2
rTAC_16384_HZ  EQU 3
rIF         EQU $ff0f ; Interrupt Flag (R/W)
rNR10       EQU $ff10 ; Channel 1 Sweep register (R/W)
rNR11       EQU $ff11 ; Channel 1 Sound length/Wave pattern duty (R/W)
rNR12       EQU $ff12 ; Channel 1 Volume Envelope (R/W)
rNR13       EQU $ff13 ; Channel 1 Frequency lo (Write Only)
rNR14       EQU $ff14 ; Channel 1 Frequency hi (R/W)
rNR21       EQU $ff16 ; Channel 2 Sound Length/Wave Pattern Duty (R/W)
rNR22       EQU $ff17 ; Channel 2 Volume Envelope (R/W)
rNR23       EQU $ff18 ; Channel 2 Frequency lo data (W)
rNR24       EQU $ff19 ; Channel 2 Frequency hi data (R/W)
rNR30       EQU $ff1a ; Channel 3 Sound on/off (R/W)
rNR31       EQU $ff1b ; Channel 3 Sound Length
rNR32       EQU $ff1c ; Channel 3 Select output level (R/W)
rNR33       EQU $ff1d ; Channel 3 Frequency's lower data (W)
rNR34       EQU $ff1e ; Channel 3 Frequency's higher data (R/W)
rNR41       EQU $ff20 ; Channel 4 Sound Length (R/W)
rNR42       EQU $ff21 ; Channel 4 Volume Envelope (R/W)
rNR43       EQU $ff22 ; Channel 4 Polynomial Counter (R/W)
rNR44       EQU $ff23 ; Channel 4 Counter/consecutive; Initial (R/W)
rNR50       EQU $ff24 ; Channel control / ON-OFF / Volume (R/W)
rNR51       EQU $ff25 ; Selection of Sound output terminal (R/W)
rNR52       EQU $ff26 ; Sound on/off
rLCDC       EQU $ff40 ; LCD Control (R/W)

rLCDC_STAT  EQU $ff41 ; LCDC Status (R/W)
rSCY        EQU $ff42 ; Scroll Y (R/W)
rSCX        EQU $ff43 ; Scroll X (R/W)
rLY         EQU $ff44 ; LCDC Y-Coordinate (R)
rLYC        EQU $ff45 ; LY Compare (R/W)
rDMA        EQU $ff46 ; DMA Transfer and Start Address (W)
rBGP        EQU $ff47 ; BG Palette Data (R/W)
rOBP0       EQU $ff48 ; Object Palette 0 Data (R/W)
rOBP1       EQU $ff49 ; Object Palette 1 Data (R/W)
rWY         EQU $ff4a ; Window Y Position (R/W)
rWX         EQU $ff4b ; Window X Position minus 7 (R/W)
rIE         EQU $ffff ; Interrupt Enable (R/W)
