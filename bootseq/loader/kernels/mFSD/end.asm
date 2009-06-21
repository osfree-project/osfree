;
;
;

name start

public mbi
public pad

_TEXT    segment dword public 'CODE'  use16
_TEXT    ends
_DATA    segment dword public 'DATA'  use16
_DATA    ends

DGROUP   group _TEXT,_DATA

_DATA    segment dword public 'DATA'  use16

pad  label byte
include aaa.inc

align 4

mbi  dd  11111111h

_DATA    ends

         end
