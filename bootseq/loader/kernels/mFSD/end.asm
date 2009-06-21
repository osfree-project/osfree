;
;
;

name start

public mbi
public pad

public _cstart_
public __STK
public _small_code_

_TEXT    segment dword public 'CODE'  use16
_TEXT    ends
CONST    segment dword public 'DATA'  use16
CONST    ends
CONST2   segment dword public 'DATA'  use16
CONST2   ends
_DATA    segment dword public 'DATA'  use16
_DATA    ends

DGROUP   group CONST,CONST2,_DATA

_TEXT    segment dword public 'CODE'  use16

__STK:
_cstart_:
         ret

_TEXT    ends


_DATA    segment dword public 'DATA'  use16

pad  label byte
;include aaa.inc

align 4

_small_code_  dd 0

mbi           dd  11111111h

_DATA    ends

         end
