read_allocmap:
                ; Read uFSD allocation map
                ; sector at 0x800:0x0

                pop  es                                    ; mapLoadSeg

                push dx                                    ; phys disk number
                push ds                                    ; save ds == 0x7c0

                call read_alloc_sector

                push es                                    ;
                pop  ds                                    ; ds --> current uFSD alloc sector

                call adjust_es

                push es
begin_read:
                xor  si, si                                ;

read_loop:
                mov  ax, word ptr [4*si]                   ; ds:si --> current uFSD sector address
                mov  dx, word ptr [4*si + 2]               ; dword ptr [4*si] ==> dx:ax

                mov  cl, byte ptr [bx + si]                ; Sector count
                ;
                ; Read extent
                ;

                push bx

                mov  bx, ds                                ;
                pop  ds                                    ; Exchange stack top <--> ds
                push bx                                    ;

                call ReadRun                               ; Read next uFSD extent

                mov  bx, ds                                ;
                pop  ds                                    ; Exchange stack top <--> ds
                push bx                                    ;

                pop  bx

                inc  si                                    ; Next sector / extent

                call adjust_es                             ; Adjust es by number of paragraphs read

                cmp  si, 80h                               ; If a sector number
                jae  end_read                              ; is greater than 128?
.386
                cmp  dword ptr [si], 0
.286
                jnz  read_loop
end_read:

.386
                cmp dword ptr mapAddr, 0
.286
                jz   the_end
                call read_alloc_sector
                jmp  begin_read
the_end:
                pop  es                                    ; restore es --> 0x820
                pop  ds                                    ; restore ds --> 0x7c0
                pop  dx                                    ; Physical disk number in dl
call_muFSD:
                lea  si, BPB                               ; address in ds:si

if 1
;;;;;;
                mov  di, si

                cli                                        ;

                push muFSDStackSS                          ;
                pop  ss                                    ; Set muFSD stack
                mov  sp, muFSDStackSP                      ;

                sti                                        ;

                push es                                    ; muFSD segment

                push si                                    ;

                push bootSecReloc                          ;
                pop  es                                    ;

                mov  cx, 200h - 11                         ; Relocate the
                rep  movsb                                 ; bootsector

                pop  si                                    ;
;;;;;;
else
                push es                                    ; muFSD segment
endif
                push muFSDEntry                            ; offset

                retf                                       ; "return" to micro FSD code

                ;
                ; On exit:
                ;
                ;            ds:si --> BPB at 0x7c0:0xb
                ;            dl    --  int 13h boot drive number
                ;            cs:ip --> muFSD at address 0x820:0x0

boot endp


;
; Loads next map sector into
; the scratch buffer and loads
; lengths array offset into bx
;

read_alloc_sector proc near
;                xor  bh, bh                                ;
;                mov  bl, lenOffset                         ; bx --> offset in bytes into the map sector
;                shl  bx, 2                                 ; (lengths table base)

                mov  bx, 101

                xor  di, di                                ; zero
                                                           ; load muFSD allocation map address
                mov  ax, word ptr mapAddr                  ; muFSD alloc. map address, low word
                mov  dx, word ptr mapAddr + 2              ; high word

                mov  cx, 1                                 ; Number of sectors to read

                call ReadRun                               ; read alloc map sector

                ret
read_alloc_sector endp

;
; Increment es by number of paragraphs read in cx
;

adjust_es proc near
                push es
                pop  ax

                shl  cx, 5                                 ; cx == number of paragraphs read
                add  ax, cx

                push ax
                pop  es

                ret
adjust_es endp
