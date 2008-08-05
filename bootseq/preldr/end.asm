;
; end.asm:
; ending labels definition
;

.386p

public   exe_end
public   bss_end

CONST2   segment dword public 'DATA'  use32
CONST2   ends
_end1    segment dword public 'DATA'  use32
align 4
exe_end:
_end1    ends
_end2    segment dword public 'BSS'   use32
_end2    ends
_BSS     segment dword public 'BSS'   use32
_BSS     ends
_end3    segment dword public 'BSS'   use32
align 4
bss_end:
_end3    ends
_end4    segment dword public 'BSS'   use32
_end4    ends

         end
