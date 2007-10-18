;
; fsd.inc
;

STAGE0_BASE equ 32000h   ; Stage0 load base
STAGE1_BASE equ 36000h   ; Stage1 load base
UFSD_BASE   equ 39000h   ; uFSD   load base
UXFD_BASE   equ 3C000h   ; uXFD   load base

STAGE0_LEN  equ (STAGE1_BASE - STAGE0_BASE)
