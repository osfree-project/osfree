;
; init.asm:
; uXFD startup
;

public BPBPtr
public LIPPtr

extrn  fmt_load:NEAR

.286

_TEXT segment para public 'CODE' use16
_TEXT ends
_DATA segment para public 'DATA' use16
_DATA ends

DGROUP  group _TEXT,_DATA
assume  DS:DGROUP,CS:DGROUP,SS:nothing

_TEXT segment para public 'CODE' use16
       org 0h

_start proc far

       push bp
       mov  bp, sp

       mov  ax, ds
       mov  word ptr cs:BPBPtr + 2, ax
       mov  word ptr cs:BPBPtr, si

       mov  ax, es
       mov  word ptr cs:LIPPtr + 2, ax
       mov  word ptr cs:LIPPtr, di

       push ds
       push es

       mov  ax, cs
       mov  ds, ax
       mov  es, ax

       push dword ptr [bp + 16]
       push dword ptr [bp + 12]
       push dword ptr [bp + 8]
       push dword ptr [bp + 4]

       call fmt_load

       add  sp, 16

       pop  es
       pop  ds

       pop  bp

       retf

_start endp

_TEXT ends


_DATA segment para public 'DATA' use16

BPBPtr    dd   ?
LIPPtr    dd   ?

_DATA ends

      end _start
