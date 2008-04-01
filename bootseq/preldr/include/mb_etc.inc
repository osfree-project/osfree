;
; mb_etc.inc
; Additional multiboot definitions
;

; multiboot kernel load base
KERN_BASE       equ 0x100000

; real mode portion relocation base
REAL_BASE       equ 0x30000

_TEXT16_SIZE    equ 200h
STARTUP_SIZE    equ 100h
MODESW_SIZE     equ _TEXT16_SIZE - STARTUP_SIZE
