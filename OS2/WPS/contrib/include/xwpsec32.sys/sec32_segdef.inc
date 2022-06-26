
;
; segment definitions for drv32.sys
;

DATA16 segment word public use16 'DATA16'
DATA16 ends

CODE16 segment word public use16 'CODE'
CODE16 ends

CODE32 segment dword public use32 'CODE'
CODE32 ends

BSS32   segment dword use32 public 'BSS'
BSS32   ends

; if we do not have c_common, we end up with linker errors
; V1.0.1 (2003-01-10) [umoeller]
c_common        segment dword use32 public 'DATA'
c_common        ends

CONST32_RO      segment dword use32 public 'CONST'
CONST32_RO      ends

DATA32  segment dword use32 public 'DATA'
DATA32  ends

DGROUP   group BSS32, c_common, DATA32

