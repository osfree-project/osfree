;          title     OS32UTIL - Assembler utilities for 4OS2 & TCMD/OS2
;
;          ;
;          page      ,132                ;make wide listing
;          ;
;          comment   }
;
;          (C) Copyright 1988 - 1996  Rex Conn & J.P. Software
;          All Rights Reserved
;
;          Original Author:    Rex Conn
;
;          } end description

        .586
        .387
CODE32  SEGMENT DWORD USE32 PUBLIC 'CODE'
CODE32  ENDS

        ASSUME  CS:FLAT, DS:FLAT, SS:FLAT, ES:FLAT

CODE32    SEGMENT

PUBLIC    _get_cpu
PUBLIC    _get_ndp

          ;
          ; GET_CPU - get cpu type
          ;
          ; On exit:
          ;    AX = 286 if an 80286
          ;         386 if an 80386 or 80486
          ;
          ;    CX destroyed, all other registers unchanged, interrupts on
          ;

EF_ID   equ     0200000h                ; ID bit in EFLAGS register

          ALIGN     04H

_get_cpu  PROC      NEAR

          push      ebp
          mov       ebp,esp

          push      ebx                 ;save registers
          push      esi
          push      edi

          pushfd                        ;save EFLAGS for exit
          pushfd                        ;push EFLAGS register
          pop       eax                 ;EAX = EFLAGS

          mov       edx, eax            ;EDX = EFLAGS
          xor       eax, 40000h         ;toggle AC bit(bit 18) in EFLAGS
          push      eax                 ;push new value
          popfd                         ;put it in EFLAGS
          pushfd                        ;push EFLAGS
          pop       eax                 ;EAX = EFLAGS
          and       eax, 40000h         ;isolate bit 18 in EAX
          and       edx, 40000h         ;isolate bit 18 in ECX
          cmp       eax, edx            ;are EAX and EDX equal?
          je        short got_386       ;yes - it's a 386

          ;it's a 486 or Pentium
          pushfd                        ;get EFLAGS in EAX
          pop       eax
          mov       edx, eax
          xor       eax, EF_ID          ;try to toggle ID bit
          push      eax
          popfd
          pushfd                        ;get new EFLAGS
          pop       eax

          and       eax, EF_ID          ;if we can't toggle ID, it's a 486
          and       edx, EF_ID
          cmp       eax, edx
          je        short got_486

          mov   eax, 1
          cpuid

          and       ax, 0F00h           ;mask everything except family field
          cmp       ax, 0500h           ;get Family field (5=Pentium)
          jb        got_486

          cmp       eax,0500h           ; get Family field (5=Pentium)
          jb        got_486

          cmp       eax, 500h
          jbe       got_586

          mov       eax, 686            ; Pentium Pro
          jmp       short restore_486
got_586:
          mov       eax,586             ; Pentium Classic
          jmp       short restore_486
got_486:
          mov       eax, 486
          jmp       short restore_486
got_386:
          mov       eax, 386            ;it's an 80386

restore_486:
          popfd                         ;restore EFLAGS
          pop       edi                 ;restore registers
          pop       esi
          pop       ebx

          mov       esp,ebp
          pop       ebp

          ret

_get_cpu  ENDP


          ;
          ; GET_NDP - get ndp type
          ;
          ; On exit:
          ;    AX = 0 if no ndp
          ;        387 if an 80387
          ;
          ;  All other registers unchanged, interrupts on
          ;

          ALIGN     04H

_get_ndp  PROC      NEAR

control   EQU       [esp-2]

          fninit                                ; try to initialize NDP
          mov       byte ptr control+1,0        ; clear memory byte
          fnstcw    control                     ; put control word in mem
          mov       eax,387
          mov       ch,byte ptr control+1
          cmp       ch,03h                      ; if AH is 3, NDP is present
          je        short bye_ndp               ;   found NDP
          xor       eax,eax                     ; clear AX to show no NDP
bye_ndp:
          ret

_get_ndp  ENDP

CODE32    ENDS

          END
