;
; segord.asm
;

.386p

_TEXT16    segment dword public 'CODE'  use16
_TEXT16    ends
DCONST     segment dword public 'FAR_DATA'  use16
DCONST     ends
DCONST2    segment dword public 'FAR_DATA'  use16
DCONST2    ends
D_DATA     segment dword public 'FAR_DATA'  use16
D_DATA     ends

D_GROUP    group   _TEXT16,DCONST,DCONST2,D_DATA

BEGTEXT    segment dword public 'CODE'  use32
BEGTEXT    ends
_TEXT      segment dword public 'CODE'  use32
_TEXT      ends
_MSGSEG32  segment dword public 'CODE'  use32
_MSGSEG32  ends
_NULL      segment dword public 'DATA'  use32
_NULL      ends
_AFTERNULL segment dword public 'DATA'  use32
_AFTERNULL ends
CONST      segment dword public 'DATA'  use32
CONST      ends
CONST2     segment dword public 'DATA'  use32
CONST2     ends
_DATA      segment dword public 'DATA'  use32
_DATA      ends
XIB        segment dword public 'DATA'  use32
XIB        ends
XI         segment dword public 'DATA'  use32
XI         ends
XIE        segment dword public 'DATA'  use32
XIE        ends
YIB        segment dword public 'DATA'  use32
YIB        ends
YI         segment dword public 'DATA'  use32
YI         ends
YIE        segment dword public 'DATA'  use32
YIE        ends
TIB        segment dword public 'DATA'  use32
TIB        ends
TI         segment dword public 'DATA'  use32
TI         ends
TIE        segment dword public 'DATA'  use32
TIE        ends
DATA       segment dword public 'DATA'  use32
DATA       ends
_BSS       segment dword public 'BSS'   use32
_BSS       ends
stack      segment dword public 'STACK' use32
stack      ends

DGROUP group _NULL,_AFTERNULL,CONST,CONST2,_DATA,XIB,XI,XIE,YIB,YI,YIE,TIB,TI,TIE,DATA,_BSS,stack
;BEGTEXT,_TEXT,_MSGSEG32,

           end
