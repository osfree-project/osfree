                .286
                .model  Medium,Syscall
 
extrn           DosPutMessage           :far
extrn           DosExit                 :far
extrn           DosGetProcAddr          :far
extrn           DosLoadModule           :far
extrn           DosFreeModule           :far
extrn           DosGetEnv               :far

Stack16	SEGMENT WORD PUBLIC USE16 'Stack'
                db 0ffffh dup(?)
Stack16         ENDS

_DATA		SEGMENT WORD PUBLIC USE16 'DATA'
 OutCnt          dw 0                             ; ARGC
 OutCmd          dw 256 dup(?)                    ; ARGV[]
 Untfs           db 'UJFS',0
 Function        db 'CHKDSK',0
 TmpBuf          db 512 dup(?)
 Handle          dd 0
 CallAddr        dd ?
 CmdSel          dw ?
 EnvOffset       dw ?
 CmdOffset       dw ?
 SSsave          dw ?
 SPsave          dw ?
_DATA         ENDS

_TEXT		SEGMENT WORD PUBLIC USE16 'CODE'
		ASSUME	CS:_TEXT
		ASSUME	DS:_DATA
		ASSUME	ES:_DATA
              ASSUME SS:Stack16
                .386
start: 
        cld
        push    si
;  ax - envirovpment
;  bx - command line
        mov     CmdSel,ax              ; Save selector of Env(Cmd)
        mov     EnvOffset,bx           ; Save offset of envirovpment
; Setup stack
        mov     ax,ss
        mov     SSsave,ax
        mov     ax,sp
        mov     SPsave,ax
        mov     ax, seg Stack16
        mov     ss,ax
        xor	  sp,sp
        dec     sp
        pusha
;
        mov     ax,CmdSel
        push    ds
        pop     es
; Find begin command string
        mov     cx,0ffffh              ; count to check symbols
        push    si                     ; prepare registers for found
        push    ds
        mov     ds,ax                  ; selector to command line
        mov     si,bx                  ; offset to command line
        mov     di,offset OutCmd       ; pointer to ARGV
        xor     bx,bx                  ; ZERO ARGC
        mov     ax,si                  ; first parameter ARGV
        stosw                          ; save offset
        mov     ax,ds                  ; save sel
        stosw                          ;
        inc     bx                     ; ARGC 1 is min
        lodsb                          ; test first symbol path
        or      al,al                  ; Empty?
        jnz     short @@B2             ; No, go to check number of parameter
        jmp     short @@F1             ; without check command line
;
@@B2:                                  ; scan ZERO
        lodsb                          ; get symbol from path
        or      al,al                  ; End path line?
        jz      short @@FF             ; yes, go away
        loop    short @@B2             ; next symbol
@@FF:
        mov     cx,0ffffh              ; count to check symbols
        mov     ax,si                  ; offset to command line
        stosw                          ;
        mov     ax,ds                  ; selector to command line
        stosw
;
        lodsb
        or      al,al                  ; Command line present line?
        jz      short @@F1             ; no, go away
        inc     bx                     ; inc ARGC
        dec     si                     ; pointer to first symbol ARGV[1]
;
@@B1:                                  ; scan SPACE
        lodsb                          ; get symbol from command line
        cmp     al,' '                 ; end parameter?
        jne     short @@Cont           ; No, check for end command line
        mov     Byte Ptr Ds:[si-1],0   ; one parameter with end ZERO
        inc     bx                     ; increment ARGC
        mov     ax,si                  ; Save ARGV[ARGC]
        stosw                          ; 
        mov     ax,ds                  ; 
        stosw                          ;
@@Cont:
        or      al,al                  ; End command line?
        jz      short @@F2             ; yes, go away
        loop    short @@B1             ; next symbol
@@F2:
;;        inc     bx                     ; inc last ARGC
@@F1:
        pop     ds
        pop     si
        mov     OutCnt,bx              ; Save ARGC
ifdef Debug
;  Say to Screen
        not     cx
        push    1
        push    cx
        mov     ax,CmdSel
        mov     bx,CmdOffset
        push    ax
        push    bx
        call    DosPutMessage
;
endif
        xor     bx,bx
        push    ds                      ; Tmp name for failure
        push    offset TmpBuf
        push    4                       ; Name length
        push    ds                      ; DLL Name
        push    offset Untfs
        push    ds                      ; Pointer to HMODULE
        push    offset Handle        
        call    DosLoadModule
        or      ax,ax                   ; OK?
        jz      short ToAddr            ; Yes, go next
        mov     bx,11                   ; No DLL or no file 
        jmp     ToExit
ToAddr:                                 ; Now get address CHKDSK function from DLL
        mov     ax,Word Ptr Handle      ; Module handle
        push    ax
        push    ds                      ; Function's name
        push    offset Function
        push    ds                      ; Function address for call
        push    offset CallAddr
        call    DosGetProcAddr
        or      ax,ax                   ; OK?
        jz      short ToCall            ; Yes, go next
        mov     bx,1
        jmp     ToFree
ToCall:
        mov     bx,OutCnt
        push    bx                      ; argc
        push    ds                      ; argv pointer
        mov     bx,offset OutCmd
        push    bx
        push    ds                      ; env pointer
        mov     bx,offset EnvOffset
        push    bx
        mov     bp,sp                   ; save stack pointer
        call    Dword Ptr CallAddr      ; call Chkdsk
        mov     sp,bp                   ; restore stack pointer
ToFree:
        mov     ax,Word Ptr Handle      ; Handle for free
        push    ax
        call    DosFreeModule
        or      ax,ax                   ; OK?
        jz      short ToExit            ; Yes, go next
        mov     bx,12
;
ToExit:
; Restore stack
        popa
        mov     ax,SSsave
        mov     ss,ax
        mov     ax,SPsave
        mov     sp,ax
;
        push    1
        push    bx               ; Return code
        call    DosExit 
        pop     si
        ret
_TEXT   ENDS
                end     start 
