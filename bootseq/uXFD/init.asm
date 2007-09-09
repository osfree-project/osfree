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

       mov  ax, ds
       mov  word ptr cs:BPBPtr + 2, ax
       mov  word ptr cs:BPBPtr, si

       mov  ax, es
       mov  word ptr cs:LIPPtr + 2, ax
       mov  word ptr cs:LIPPtr, di

       mov  ax, cs
       mov  ds, ax
       mov  es, ax

       call fmt_load

       ;lea  ax, msg
       ;push ax
       ;les  di, LIPPtr
       ;call dword ptr es:[di + 64]  ; printk()
       ;add  sp, 2

       retf

_start endp

_TEXT ends


_DATA segment para public 'DATA' use16

BPBPtr    dd   ?
LIPPtr    dd   ?

msg       db   "Hello!!!",0

_DATA ends

      end _start
