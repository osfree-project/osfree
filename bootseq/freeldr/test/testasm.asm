public loadhigh
public entry_msg, exit_msg, args_msg
public current_seg

extern printk:NEAR

_TEXT segment word public 'CODE' use16
assume cs:_TEXT, ds:_TEXT, es:nothing

;
;  unsigned short __cdecl loadhigh(unsigned long buf,
;                                  unsigned short chunk,
;                                  unsigned char far *readbuf);
;
;  Move chunk bytes from readbuf 16:16 address to
;  buf 0:32 address, which can be above 1 Megabyte
;
;  Stack state:
;
;  word  ptr [bp + 2]  - return address
;  dword ptr [bp + 4]  - buf
;  word  ptr [bp + 8]  - chunk
;  word  ptr [bp + 10] - readbuf offset
;  word  ptr [bp + 12] - readbuf seg
;
;  Note: chunk should be <= 32 Kb (a restriction of
;  function 87h of int 15h)
;

loadhigh proc near

        push bp
        mov  bp, sp
.386
        mov  current_seg, _TEXT

        ; Signal about function entry
        lea  ax, entry_msg
        push ax
        call printk
        add  sp, 2

        ; Print function arguments
        mov  ax,   word ptr [bp + 10]  ; readbuf offset
        push ax
        mov  ax,   word ptr [bp + 12]  ; readbuf segment
        push ax
        mov  ax,   word ptr [bp + 8]   ; chunk
        push ax
        mov  ax,   word ptr [bp + 6]   ; buf high word
        push ax
        mov  ax,   word ptr [bp + 4]   ; buf low  word
        push ax
        lea  ax, args_msg
        push ax
        call printk
        add  sp, 12

        ; Choose two variants:
        ; below or above 1 Mb.
        cmp  dword ptr [bp + 4], 100000h
        jae  above_1mb

        ; buf is below 1 Mb
        ; We're just copying contents of readbuf into buf
        ; as string
        push si
        push di
        push ds
        push es
        push cx

        ; Copy strings
        lds  si,  dword ptr [bp + 10]

        mov  eax, dword ptr [bp + 4]
        shr  eax, 4       ; Get segment from phys address
        mov  es,  ax

        shl  eax, 4
        sub  eax, dword ptr [bp + 4]
        neg  eax
        mov  di,  ax

        mov  cx,  word ptr [bp + 8]

        cld
        rep  movsb

        pop  cx
        pop  es
        pop  ds
        pop  di
        pop  si

        jmp  exit1
.286
above_1mb:
        ;
        ; Above 1 Mb, empty for now
        ;

        lea  ax, above_1mb_msg
        push ax
        call printk
        add  sp, 2

exit1:
        ; Signal about function exit
        lea  ax, exit_msg
        push ax
        call printk
        add  sp, 2

        pop  bp

        ; successful return
        mov  ax, 0

        ret

loadhigh endp

entry_msg          db  'loadhigh() is entered',0
exit_msg           db  'loadhigh() is exited',0

args_msg           db  'function arguments:',13,10
                   db  'buf     = 0x%08lX,',13,10
                   db  'chunk   = %d,',13,10
                   db  'readbuf = 0x%04X:0x%04X',0

regs_msg           db  'ds:si = 0x%04X:0x%04X,',13,10
                   db  'es:di = 0x%04X:0x%04X,',13,10
                   db  'cx    = 0x%04X',0

above_1mb_msg      db  'above 1 Mb',0

current_seg  dw  ?

_TEXT ends

      end
