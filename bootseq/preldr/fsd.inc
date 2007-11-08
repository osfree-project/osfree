;
; fsd.inc
;


BOOT_MAGIC   equ 0x2BADB002   ; extended boot info magic

BOOTSEC_BASE equ 0x8C000      ; bootsector relocation base
STAGE0_BASE  equ 0x8C200      ; Stage0 load base
STAGE1_BASE  equ 0x90200      ; Stage1 load base
UFSD_BASE    equ 0x93200      ; uFSD   load base
UXFD_BASE    equ 0x96200      ; uXFD   load base

LDR_SEG      equ 0x1000       ; os2ldr segment

STAGE0_LEN   equ (STAGE1_BASE - STAGE0_BASE)
