;
; end.asm:
; ending labels definition
;

.386p

public   preldr0_end
public   bss_end

CONST2   segment byte public 'DATA'  use32
CONST2   ends
_end1    segment byte public 'DATA'  use32
preldr0_end:
_end1    ends
_BSS     segment byte public 'BSS'   use32
_BSS     ends
_end2    segment byte public 'BSS'   use32
bss_end:
_end2    ends

         end
