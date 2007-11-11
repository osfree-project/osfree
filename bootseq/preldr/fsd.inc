;
; fsd.inc
;


BOOT_MAGIC   equ 0x2BADB002   ; extended boot info magic

MFS_BASE     equ 0x7c0        ; minifsd base

BOOTSEC_BASE equ 0x85E00      ; bootsector relocation base
STAGE0_BASE  equ 0x86000      ; Stage0 load base
STAGE1_BASE  equ 0x8A200      ; Stage1 load base
UFSD_BASE    equ 0x8D200      ; uFSD   load base
UXFD_BASE    equ 0x90000      ; uXFD   load base
LDR_BASE     equ 0x10000      ; os2ldr base

STAGE0_SEG   equ 0x8600       ; preldr segment
MFS_SEG      equ 0x7c         ; minifsd segment
LDR_SEG      equ 0x1000       ; os2ldr segment

STAGE0_LEN   equ (STAGE1_BASE - STAGE0_BASE)
