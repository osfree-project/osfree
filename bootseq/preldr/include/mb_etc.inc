;
; mb_etc.inc
; Additional multiboot definitions
;

;; FreeLdr magic, must be in EDX register
;FREELDR_MAGIC   equ 0x3badb002

; multiboot kernel load base
KERN_BASE       equ 0x800000

; real mode portion relocation base
REAL_BASE       equ 0x8000
REALSTACKINIT   equ 0x4000

_TEXT16_SIZE    equ 200h
STARTUP_SIZE    equ 100h
MODESW_SIZE     equ _TEXT16_SIZE - STARTUP_SIZE
