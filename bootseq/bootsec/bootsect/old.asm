;
; ReadCHS:
; Reads a contiguous run of sectors
; by CHS.
;
;          Input:
;          dx:ax  -- logical sector number to read from
;          cl     -- sector count to read
;          es:di  -- address to read to
;          bl     -- drive number

ReadCHS proc near
                push ax

                mov  ax, headsCnt                          ; headsCnt * trackSize
                mul  byte ptr trackSize                    ; --> ax (cyl size)
                mov  si, ax                                ; si = cylinder size

                pop  ax
beginRead:
                push bx
                push dx
                push ax
                push cx
                push bx
                                                           ; divide LSN in dx:ax by cyl size in si
                div  si                                    ; now dx holds remainder
                                                           ; and ax holds quotient
                mov  bx, ax                                ; bx = Cylinder no
                mov  ax, dx                                ;
                div  byte ptr trackSize                    ; now al holds Head no
                                                           ; and ah holds sector no
                pop  dx                                    ; now dl = drive number

                inc  ah                                    ; sectors in CHS are numbered from 1, not 0 !!!
                mov  dh, al                                ; head no

                mov  cx, bx                                ;
                xchg cl, ch                                ;

                or   cl, ah                                ;

                mov  bx, trackSize                         ; bx --> max no of sectors we can
                sub  bl, ah                                ; read in a single call to int 13h, ah=2
                inc  bl                                    ;

                pop  ax                                    ; remaining amount of sectors to read

                cmp  ax, bx
                jb   _nosub
                sub  ax, bx
                push ax                                    ; remaining amount of sectors to read
                mov  al, bl                                ;
                jmp  _sub
_nosub:
                push 0                                     ; remaining amount of sectors to read
_sub:
                mov  bx, di                                ; es:bx --> buffer to read to

                mov  ah, 02h                               ; read disk function
                int  13h

                jnc  on
                jmp Err$Read
on:
                cmp  ah, 0
                jz   on0
                jmp Err$Read
on0:
                mov  bl, al                                ; count of sectors read

                xor  ah, ah                                ;
                mul  word ptr sectSize                     ; Now dx:ax = count of bytes read

                add  di, ax
on1:
                pop  cx
                pop  ax
                pop  dx

                add  al, bl                                ;
                adc  ah, 0                                 ; Now dx:ax = the next 1st byte to read
                adc  dx, 0                                 ;

                pop  bx

                jcxz endRead

                jmp  beginRead
endRead:
                ret

ReadCHS endp
