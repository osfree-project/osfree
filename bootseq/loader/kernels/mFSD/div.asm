;*****************************************************************************
;*
;*                            Open Watcom Project
;*
;*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
;*
;*  ========================================================================
;*
;*    This file contains Original Code and/or Modifications of Original
;*    Code as defined in and that are subject to the Sybase Open Watcom
;*    Public License version 1.0 (the 'License'). You may not use this file
;*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
;*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
;*    provided with the Original Code and Modifications, and is also
;*    available at www.sybase.com/developer/opensource.
;*
;*    The Original Code and all software distributed under the License are
;*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
;*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
;*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
;*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
;*    NON-INFRINGEMENT. Please see the License for the specific language
;*    governing rights and limitations under the License.
;*
;*  ========================================================================
;*
;* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
;*               DESCRIBE IT HERE!
;*
;*****************************************************************************


;========================================================================
;==     Name:           I4M,U4M                                        ==
;==     Operation:      integer four byte multiply                     ==
;==     Inputs:         DX;AX   integer M1                             ==
;==                     CX;BX   integer M2                             ==
;==     Outputs:        DX;AX   product                                ==
;==     Volatile:       CX, BX destroyed                               ==
;========================================================================

include struct.inc

_TEXT   segment byte public 'CODE'  use16

public __I4M
public __U4M
public __U4D

__I4M:
__U4M:
        assume cs:_TEXT

        xchg    ax,bx           ; swap low(M1) and low(M2)
        push    ax              ; save low(M2)
        xchg    ax,dx           ; exchange low(M2) and high(M1)
        or      ax,ax           ; if high(M1) non-zero
        _if     ne              ; then
          mul   dx              ; - low(M2) * high(M1)
        _endif                  ; endif
        xchg    ax,cx           ; save that in cx, get high(M2)
        or      ax,ax           ; if high(M2) non-zero
        _if     ne              ; then
          mul   bx              ; - high(M2) * low(M1)
          add   cx,ax           ; - add to total
        _endif                  ; endif
        pop     ax              ; restore low(M2)
        mul     bx              ; low(M2) * low(M1)
        add     dx,cx           ; add previously computed high part

        ret                     ; and return!!!

;========================================================================
;==     Name:           U4D                                            ==
;==     Operation:      Unsigned 4 byte divide                         ==
;==     Inputs:         DX;AX   Dividend                               ==
;==                     CX;BX   Divisor                                ==
;==     Outputs:        DX;AX   Quotient                               ==
;==                     CX;BX   Remainder                              ==
;==     Volatile:       none                                           ==
;========================================================================

__U4D:
        or      cx,cx           ; check for easy case
        jne     noteasy         ; easy if divisor is 16 bit
        dec     bx              ; decrement divisor
        _if     ne              ; if not dividing by 1
          inc   bx              ; - put divisor back
          cmp   bx,dx           ; - if quotient will be >= 64K
          _if   be              ; - then
;
;       12-aug-88, added thanks to Eric Christensen from Fox Software
;       divisor < 64K, dividend >= 64K, quotient will be >= 64K
;
;       *note* this sequence is used in ltoa's #pragmas; any bug fixes
;              should be reflected in ltoa's code bursts
;
            mov   cx,ax         ; - - save low word of dividend
            mov   ax,dx         ; - - get high word of dividend
            sub   dx,dx         ; - - zero high part
            div   bx            ; - - divide bx into high part of dividend
            xchg  ax,cx         ; - - swap high part of quot,low word of dvdnd
          _endif                ; - endif
          div   bx              ; - calculate low part
          mov   bx,dx           ; - get remainder
          mov   dx,cx           ; - get high part of quotient
          sub   cx,cx           ; - zero high part of remainder
        _endif                  ; endif
        ret                     ; return


noteasy:                        ; have to work to do division
;
;       check for divisor > dividend
;
        _guess                  ; guess: divisor > dividend
          cmp   cx,dx           ; - quit if divisor <= dividend
          _quif b               ; - . . .
          _if   e               ; - if high parts are the same
            cmp   bx,ax         ; - - compare the lower order words
            _if   be            ; - - if divisor <= dividend
              sub   ax,bx       ; - - - calulate remainder
              mov   bx,ax       ; - - - ...
              sub   cx,cx       ; - - - ...
              sub   dx,dx       ; - - - quotient = 1
              mov   ax,1        ; - - - ...
              ret               ; - - - return
            _endif              ; - - endif
          _endif                ; - endif
          sub   cx,cx           ; - set divisor = 0 (this will be quotient)
          sub   bx,bx           ; - ...
          xchg  ax,bx           ; - return remainder = dividend
          xchg  dx,cx           ; - and quotient = 0
          ret                   ; - return
        _endguess               ; endguess
;;;        push    bp              ; save work registers
;;;        push    si              ; ...
;;;        push    di              ; ...
;;;        sub     si,si           ; zero quotient
;;;        mov     di,si           ; ...
;;;        mov     bp,si           ; and shift count
;;;moveup:                         ; loop until divisor > dividend
;;;          _shl    bx,1          ; - divisor *= 2
;;;          _rcl    cx,1          ; - ...
;;;          jc      backup        ; - know its bigger if carry out
;;;          inc     bp            ; - increment shift count
;;;          cmp     cx,dx         ; - check if its bigger yet
;;;          jb      moveup        ; - no, keep going
;;;          ja      divlup        ; - if below, know we're done
;;;          cmp     bx,ax         ; - check low parts (high parts equal)
;;;          jbe     moveup        ; until divisor > dividend
;;;divlup:                         ; division loop
;;;        clc                     ; clear carry for rotate below
;;;        _loop                   ; loop
;;;          _loop                 ; - loop
;;;            _rcl  si,1          ; - - shift bit into quotient
;;;            _rcl  di,1          ; - - . . .
;;;            dec   bp            ; - - quif( -- shift < 0 ) NB carry not changed
;;;            js    donediv       ; - - ...
;;;backup:                         ; - - entry to remove last shift
;;;            rcr   cx,1          ; - - divisor /= 2 (NB also used by 'backup')
;;;            rcr   bx,1          ; - - ...
;;;            sub   ax,bx         ; - - dividend -= divisor
;;;            sbb   dx,cx         ; - - c=1 iff it won't go
;;;            cmc                 ; - - c=1 iff it will go
;;;          _until  nc            ; - until it won't go
;;;          _loop                 ; - loop
;;;            _shl  si,1          ; - - shift 0 into quotient
;;;            _rcl  di,1          ; - - . . .
;;;            dec   bp            ; - - going to add, check if done
;;;            js    toomuch       ; - - if done, we subtracted to much
;;;            shr   cx,1          ; - - divisor /= 2
;;;            rcr   bx,1          ; - - ...
;;;            add   ax,bx         ; - - dividend += divisor
;;;            adc   dx,cx         ; - - c = 1 iff bit of quotient should be 1
;;;          _until  c             ; - until divisor will go into dividend
;;;        _endloop                ; endloop
;;;toomuch:                        ; we subtracted too much
;;;        add     ax,bx           ; dividend += divisor
;;;        adc     dx,cx           ; ...
;;;donediv:                        ; now quotient in di;si, remainder in dx;ax
;;;        mov     bx,ax           ; move remainder to cx;bx
;;;        mov     cx,dx           ; ...
;;;        mov     ax,si           ; move quotient to dx;ax
;;;        mov     dx,di           ; ...
;;;        pop     di              ; restore registers
;;;        pop     si              ; ...
;;;        pop     bp              ; ...
;;;        ret                     ; and return
; SJHowe 24-01-2000
;
; At this point here what is known is that cx > 0 and dx > cx. At the very
; least cx is 1 and dx 2.
;
; Consider the quotient
;
; The maximum it can be is when division is
;
; FFFF:FFFF / 0001:0000
;
; The minimum it can be is when division is
;
; 0002:0000 / 0001:FFFF
;
; Doing the division reveals the quotient lies 1 between FFFF. It cannot
; exceed FFFF. Therefore there is no need to keep track of the quotient's
; high word, it is always 0.
;
; Accordingly register DI has been eliminated below
;
; Should make algoritm a little faster.
;
; SJHowe 24-01-2000

        push    bp              ; save work registers
        push    si              ; ...
        sub     si,si           ; zero quotient
        mov     bp,si           ; and shift count
moveup:                         ; loop until divisor > dividend
          _shl    bx,1          ; - divisor *= 2
          _rcl    cx,1          ; - ...
          jc      backup        ; - know its bigger if carry out
          inc     bp            ; - increment shift count
          cmp     cx,dx         ; - check if its bigger yet
          jb      moveup        ; - no, keep going
          ja      divlup        ; - if below, know we're done
          cmp     bx,ax         ; - check low parts (high parts equal)
          jbe     moveup        ; until divisor > dividend
divlup:                         ; division loop
        clc                     ; clear carry for rotate below
        _loop                   ; loop
          _loop                 ; - loop
            _rcl  si,1          ; - - shift bit into quotient
            dec   bp            ; - - quif( -- shift < 0 ) NB carry not changed
            js    donediv       ; - - ...
backup:                         ; - - entry to remove last shift
            rcr   cx,1          ; - - divisor /= 2 (NB also used by 'backup')
            rcr   bx,1          ; - - ...
            sub   ax,bx         ; - - dividend -= divisor
            sbb   dx,cx         ; - - c=1 iff it won't go
            cmc                 ; - - c=1 iff it will go
          _until  nc            ; - until it won't go
          _loop                 ; - loop
            _shl  si,1          ; - - shift 0 into quotient
            dec   bp            ; - - going to add, check if done
            js    toomuch       ; - - if done, we subtracted to much
            shr   cx,1          ; - - divisor /= 2
            rcr   bx,1          ; - - ...
            add   ax,bx         ; - - dividend += divisor
            adc   dx,cx         ; - - c = 1 iff bit of quotient should be 1
          _until  c             ; - until divisor will go into dividend
        _endloop                ; endloop
toomuch:                        ; we subtracted too much
        add     ax,bx           ; dividend += divisor
        adc     dx,cx           ; ...
donediv:                        ; now quotient in si, remainder in dx;ax
        mov     bx,ax           ; move remainder to cx;bx
        mov     cx,dx           ; ...
        mov     ax,si           ; move quotient to dx;ax
        xor     dx,dx           ; ...
        pop     si              ; restore registers
        pop     bp              ; ...

        ret                     ; and return

_TEXT   ends

        end
