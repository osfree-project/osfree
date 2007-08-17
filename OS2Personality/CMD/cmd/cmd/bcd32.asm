          title     BCD32.ASM - 32-bit BCD utilities for 4OS32

          page      ,80

          comment   }

          (C) Copyright 1992  Rex C. Conn
          All Rights Reserved

          Author:    Rex C. Conn

          These 4DOS support routines provide BCD addition, subtraction,
          multiplication, division, and modulo functions for 4DOS & 4OS2

          } end description

        .386P

DGROUP  GROUP   DATA32
        ASSUME  CS: FLAT, DS: FLAT, SS: FLAT

        PUBLIC    add_bcd
        PUBLIC    multiply_bcd
        PUBLIC    divide_bcd

DATA32  SEGMENT DWORD USE32 PUBLIC 'DATA'

SIZE_INTEGER    equ   16
SIZE_DECIMAL    equ   8
SIZE_MANTISSA   equ   24
SIZE_DIVIDEND   equ   SIZE_MANTISSA + SIZE_DECIMAL + 1
SIZE_WORKSPACE  equ   SIZE_MANTISSA * 2

multiplicand   dd      0FFh
divisor        dd      0FFh
workspace      db      ((2 * SIZE_MANTISSA) - 1) DUP (0FFh)
workspace_end  db      0FFh
dividend_of    db      0FFh             ; Dividend overflow.
dividend       db      SIZE_DIVIDEND DUP (0FFh)
save_sign      db      0FFh

DATA32    ENDS

CODE32    SEGMENT DWORD USE32 PUBLIC 'CODE'


          ;
          ; ROUND_BCD - Round up the target
          ;
          ; On entry:
          ;         DF set
          ;         EDI = end of mantissa + 1
          ;         ECX = number of bytes to round
          ;
          ; On exit:
          ;         Carry set if overflow
          ;         EAX, ECX, EDI destroyed
          ;         All other registers unchanged, interrupts on
          ;

round_bcd PROC      NEAR

          cmp       byte ptr [edi],5    ; if not 5 or greater, return
          jb        short no_round
          dec       edi                 ; get previous digit
          stc

round_loop:
          mov       al,[edi]
          adc       al,0                ; add carry to digit
          aaa
          mov       [edi],al
          dec       edi
          jnc       short round_exit            ; if CF = 0, return
          loop      round_loop
          jmp       short round_exit

no_round:
          clc                           ; if CF = 0, no overflow
round_exit:
          ret

round_bcd ENDP


          ;
          ; ADD_BCD - Add or subtract two BCD numbers
          ;
          ; On entry:
          ;         x = first BCD structure
          ;         y = second BCD structure
          ;
          ; On exit:
          ;         x = total
          ;         AX, CX, DX, ES destroyed
          ;         All other registers unchanged, interrupts on
          ;

add_bcd  PROC      NEAR

          push      ebp
          mov       ebp,esp

          push      esi
          push      edi
          push      ebx

          mov       esi,[ebp+08h]       ; x
          mov       edi,[ebp+0Ch]       ; y

          mov       ah,[esi]            ; save the signs
          mov       al,[edi]

          mov       ecx,SIZE_MANTISSA
          add       edi,ecx             ; set to end of BCD structure
          add       esi,ecx             ;   (for backwards read)

          cmp       al,ah               ; if signs both plus or minus, then
          je        short addition      ;   add, else subtract

          mov       edx,esi             ; save target pointer
          clc
sub_loop:
          mov       al,[esi]
          sbb       al,[edi]            ; subtract & carry
          aas
          mov       [esi],al            ; put it back to the result
          dec       esi
          dec       edi
          loop      sub_loop
          mov       al,'+'              ; if no carry, result > 0
          jnc       short write_sign

          mov       esi,edx             ; negative - subtract result from 0
          clc
          mov       ecx,SIZE_MANTISSA

negate_loop:
          mov       al,0
          sbb       al,[esi]            ; subtract & ASCII adjust
          aas
          mov       [esi],al
          dec       esi
          loop      negate_loop
          mov       al,'-'              ; store the negative sign
          jmp       short write_sign

addition:
          mov       save_sign,al
          clc                           ; clear carry for start
add_loop:
          mov       al,[esi]
          adc       al,[edi]            ; add & carry
          aaa
          mov       [esi],al
          dec       esi
          dec       edi
          loop      add_loop
          jc        short add_error     ; carry out of max size, return overflow
          mov       al,save_sign

write_sign:
          mov       [esi],al
          xor       eax,eax
          jmp       short add_bye

add_error:
          mov       eax,-1

add_bye:
          pop       ebx
          pop       edi
          pop       esi

          pop       ebp
          ret
add_bcd  ENDP


          ;
          ; MULTIPLY_BCD - multiply two BCD numbers
          ;
          ; On entry:
          ;         x = first BCD structure
          ;         y = second BCD structure
          ;
          ; On exit:
          ;         x = total
          ;         AX, BX, CX, DX, ES destroyed
          ;         All other registers unchanged, interrupts on
          ;

multiply_bcd   PROC

          push      ebp
          mov       ebp,esp

          push      esi
          push      edi
          push      ebx

          mov       ebx,[ebp+08h]       ; x
          mov       edi,[ebp+0Ch]       ; y

          mov       ah,[ebx]            ; get the signs
          mov       al,[edi]

          mov       byte ptr save_sign,'+'      ; Assume plus result.
          cmp       ah,al               ; Signs the same?
          je        short init_multiply ; If so, return + result
          mov       byte ptr save_sign,'-'

init_multiply:
          add       edi,SIZE_MANTISSA   ; go to end of BCD structures
          mov       multiplicand,edi    ; save the target
          add       ebx,SIZE_MANTISSA

          cld
          lea       edi,workspace
          mov       eax,0               ; init workspace to 0's
          mov       ecx,SIZE_WORKSPACE
          rep       stosb

          lea       edi,workspace_end
          mov       ecx,SIZE_MANTISSA   ; # of digits to be multiplied

multiply_loop:
          push      edi                 ; Save current product place.

          cmp       byte ptr [ebx],0    ; if current multiplier == 0,
          je        short next_digit    ;   don't bother with it
          mov       edx,0                ; clear the carry
          mov       dl,[ebx]            ; get the multiplier

          mov       esi,multiplicand    ; restore multiplicand pointer
          push      ecx                 ; save multiplier counter
          mov       ecx,SIZE_MANTISSA

inner_multiply:
          mov       al,[esi]
          mul       dl                  ; multiply & adjust for BCD
          aam
          add       al,[edi]            ; accumulate & add carry
          aaa
          add       al,dh
          aaa
          mov       [edi],al

          mov       dh,ah               ; save carry
          dec       esi
          dec       edi
          loop      inner_multiply

          mov       [edi],ah            ; write carry
          pop       ecx

next_digit:
          pop       edi
          dec       edi                 ; Next product.
          dec       ebx                 ; Next multiplier.
          loop      multiply_loop

          mov       ecx,SIZE_MANTISSA
          lea       edi,workspace_end
          sub       edi,(SIZE_DECIMAL + 1)
          call      round_bcd           ; round product up

          mov       ecx,SIZE_MANTISSA
          lea       esi,workspace
          add       esi,SIZE_INTEGER
          mov       edi,[ebp+08h]       ; store the result in structure "x"
          mov       al,save_sign        ; store the sign
          stosb
          rep       movsb

          mov       eax,0               ; return 0 if no overflow
          mov       ecx,SIZE_INTEGER    ; check workspace for overflow
          lea       edi,workspace
          repe      scasb
          je        short multiply_return

multiply_error:
          mov       eax,-1              ; set AX to -1 for error

multiply_return:

          pop       ebx
          pop       edi
          pop       esi

          pop       ebp
          ret

multiply_bcd ENDP


          ;
          ; DIVIDE_BCD - Divide two BCD numbers
          ;
          ; On entry:
          ;         x = first BCD structure
          ;         y = second BCD structure
          ;         modulo_flag = flag to only do modulo division
          ;
          ; On exit:
          ;         x = total, y = dividend (modulo result)
          ;         AX, CX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

divide_bcd   PROC   NEAR

          push      ebp
          mov       ebp,esp

          push      esi
          push      edi
          push      ebx

          mov       ebx,[ebp+08H]       ; x
          mov       edi,[ebp+0CH]       ; y

          mov       ah,[ebx]            ; get the signs
          mov       al,[edi]

          mov       byte ptr save_sign,'+'      ; default positive result
          mov       ecx,0
          cmp       [ebp+010H],ecx      ; check modulo flag
          je        short div_signs

          cmp       ah,'-'              ; if modulo & source is negative,
          je        short neg_result    ;   then result is negative
          jmp       short init_division
div_signs:
          cmp       ah,al               ; signs the same?
          je        short init_division
neg_result:
          mov       byte ptr save_sign,'-'      ; switch to negative result

init_division:
          cld
          mov       divisor,edi         ; save the divisor
          inc       dword ptr divisor   ; skip sign
          lea       edi,workspace
          mov       al,0
          mov       ecx,SIZE_DIVIDEND
          rep       stosb               ; Zero out quotient to start.

          mov       ecx,SIZE_MANTISSA
          lea       edi,dividend_of
          stosb                         ; zero out the overflow byte
          mov       esi,ebx
          inc       esi                 ; skip the sign
          rep       movsb               ; store the dividend

          mov       ecx,SIZE_DIVIDEND - SIZE_MANTISSA
          rep       stosb               ; Zero out dividend underflow.

          mov       ecx,SIZE_MANTISSA
          mov       edi,divisor         ; Look for zero divisor.
          repz      scasb
          jnz       short set_div_size
          jmp       divide_error        ; can't divide by 0

set_div_size:                           ; set the divisor size
          lea       edi,workspace       ; point to quotient
          add       edi,ecx
          add       dword ptr divisor,SIZE_MANTISSA - 1
          sub       divisor,ecx         ; first non-zero divisor digit
          inc       ecx                 ; DH = subtraction counter
          mov       edx,ecx             ; DL = significant divisor digits
          lea       esi,dividend
          mov       ecx,SIZE_DIVIDEND + 1
          sub       ecx,edx             ; number of times thru divide loop

divide_loop:
          cmp       dword ptr [ebp+010H],0      ; are we doing a modulo (x % y)?
          je        short not_modulo

          cmp       edi,offset FLAT:workspace + SIZE_MANTISSA
          jae       divide_end          ; only do integer divide for modulo

not_modulo:
          push      ecx                 ; Division count.
          push      edi                 ; pointer to quotient
          mov       ebx,esi             ; save pointer to dividend
          mov       dh,0                ; set sub counter to zero

next_divisor:
          cmp       byte ptr [esi-1],0  ; Overflow in remainder?
          jne       short subtract_divisor      ;   if yes, divisor fits
          mov       ecx,0
          mov       cl,dl               ; length of divisor
          mov       edi,divisor         ; pointer to divisor

compare_fit:
          cmpsb                         ; Does it fit in remainder?
          ja        short subtract_divisor      ; If yes, subtract.
          jb        short over_size             ; If no, save subtraction count.
          loop      compare_fit

subtract_divisor:
          inc       dh                  ; Sub counter
          mov       esi,divisor         ; restore divisor pointer
          mov       edi,ebx             ; restore dividend pointer
          mov       ecx,0
          mov       cl,dl               ; significant divisor digits
          dec       ecx                 ; Adjust for a moment.
          add       esi,ecx             ; Point to current position.
          add       edi,ecx
          inc       ecx

          clc
subtract_remainder:
          mov       ah,[esi]
          mov       al,[edi]            ; remainder digit
          sbb       al,ah               ; subtract & adjust
          aas
          mov       [edi],al
          dec       edi
          dec       esi
          loop      subtract_remainder

          sbb       byte ptr [edi],0    ; Subtract the carry, if any.

          mov       esi,ebx
          jmp       next_divisor        ; Next divisor.

over_size:
          mov       esi,ebx             ; get next dividend digit
          inc       esi
          pop       edi
          pop       ecx
          mov       al,dh               ; save quotient
          stosb
          loop      divide_loop

          mov       ecx,SIZE_MANTISSA
          lea       edi,workspace
          add       edi,SIZE_DIVIDEND - 1

          call      round_bcd           ; round up the quotient
          jc        short divide_error

          mov       ecx,SIZE_MANTISSA
          lea       esi,workspace
          add       esi,SIZE_DECIMAL
          mov       edi,[ebp+08H]       ; store the result in structure "x"
          mov       al,save_sign        ; store the sign
          stosb
          rep       movsb

          xor       eax,eax
          mov       ecx,SIZE_DECIMAL    ; check workspace for divide overflow
          lea       edi,workspace

          repe      scasb
          jne       short divide_error

divide_end:
          xor       eax,eax
          cmp       [ebp+010H],eax
          je        short divide_return

          mov       edi,[ebp+0CH]       ; stuff modulo into structure "y"

          mov       al,save_sign        ; store the sign
          stosb

          lea       esi,dividend        ; Dividend has remainder
          mov       ecx,SIZE_MANTISSA
          rep       movsb

          xor       eax,eax
          jmp       short divide_return

divide_error:
          mov       eax,-1

divide_return:
          pop       ebx
          pop       edi
          pop       esi

          leave
          ret       0

divide_bcd ENDP

CODE32    ends

END

