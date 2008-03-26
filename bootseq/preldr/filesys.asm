;
; filesys.asm:
; filesystem access realmode (mu_*-) functions
;

name filesys

ifndef STAGE1_5

public mu_Open
public mu_Read
public mu_Close
public mu_Terminate

public preldr_ds
public preldr_ss_sp
public preldr_es

endif

extrn filemax           :dword
extrn call_pm           :near
extrn freeldr_open      :near
extrn freeldr_read      :near
extrn freeldr_seek      :near
extrn freeldr_close     :near

.386p

switch_to_preldr macro
        mov  ax,  ds
        mov  cs:ldr_ds, ax
        mov  ax,  es
        mov  cs:ldr_es, ax
        mov  ax, sp
        mov  word ptr cs:ldr_ss_sp, ax
        mov  ax, ss
        mov  word ptr cs:ldr_ss_sp + 2, ax

        mov  ax,  cs:preldr_ds
        mov  ds,  ax
        mov  ax,  cs:preldr_es
        mov  es,  ax
        lss  sp,  cs:preldr_ss_sp
endm

switch_to_ldr macro
        mov  bx, ds
        mov  preldr_ds, bx
        mov  bx, es
        mov  preldr_es, bx
        mov  bx, sp
        mov  word ptr preldr_ss_sp, bx
        mov  bx, ss
        mov  word ptr preldr_ss_sp + 2, bx

        mov  bx, ldr_ds
        mov  ds, bx
        mov  bx, ldr_es
        mov  es, bx
        lss  sp, ldr_ss_sp
endm

_TEXT16 segment byte public 'CODE' use16


ifndef STAGE1_5


;
; Open file using MicroFSD
;
; unsigned short __cdecl
; mu_Open(char far *pName,
;         unsigned long far *pulFileSize);
;

mu_Open proc far
        push bp
        mov  bp, sp

        ; char far *pName
        mov  ebx, dword ptr [bp + 06h]

        switch_to_preldr

        ; convert far ptr in ebx to
        ; FLAT ptr.
        mov  ecx, ebx
        shr  ecx, 16
        shl  ecx, 4
        and  ebx, 0ffffh
        add  ebx, ecx

        ; switch to PM and call muOpen
        mov  eax, offset _TEXT:muOpen
        push eax
        call call_pm
        add  sp, 4

        cmp  ebx, 0
        jz   noerr1
err1:
        xor  edx, edx
        mov  ax, 1
        jmp  nok1
noerr1:
        xor  ax, ax
nok1:
        switch_to_ldr

        push es
        push di
        les  di, dword ptr [bp + 0ah]
        mov  [di], edx ; size
        xor  dx, dx
        pop  di
        pop  es

        pop  bp

        retf
mu_Open endp

;
; unsigned long  __cdecl
; mu_Read(long loffseek,
;         char far *pBuf,
;         unsigned long cbBuf);
;

mu_Read proc far
        push bp
        mov  bp, sp

        ; long loffseek
        mov  ebx, dword ptr [bp + 06h]
        ; char far *pBuf
        mov  edx, dword ptr [bp + 0ah]
        ; unsigned long cbBuf
        mov  ecx, dword ptr [bp + 0eh]

        switch_to_preldr

        ; convert a far ptr in edx
        ; to a FLAT ptr
        mov  eax, edx
        shr  eax, 16
        shl  eax, 4
        and  edx, 0ffffh
        add  edx, eax

        mov  eax, offset _TEXT:muRead
        push eax
        call call_pm
        add  sp, 4

        ; ebx (count of bytes read) -> dx:ax
        mov  edx, ebx
        shr  edx, 16
        mov  eax, ebx
        and  eax, 0ffffh

        switch_to_ldr

        pop  bp

        retf
mu_Read endp

;
; void __cdecl
; mu_Close(void);
;

mu_Close proc far
        switch_to_preldr

        mov  eax, offset _TEXT:muClose
        push eax
        call call_pm
        add  sp, 4

        switch_to_ldr

        retf
mu_Close endp

;
; void __cdecl
; mu_Terminate(void);
;

mu_Terminate proc far
        retf
mu_Terminate endp

ldr_ds       dw 0
ldr_ss_sp    dd 0
ldr_es       dw 0

preldr_ds    dw 0
preldr_ss_sp dd 0
preldr_es    dw 0


endif


_TEXT16 ends

_TEXT   segment byte public 'CODE' use32


ifndef STAGE1_5


muOpen proc near
       mov  eax, ebx
       call freeldr_open

       cmp  eax, 0
       jnz  noerr2
err2:
       mov  ebx, 1
       jmp  quit2
noerr2:
       xor  ebx, ebx
       mov  edx, offset _TEXT:filemax
       mov  edx, [edx]
quit2:
       ret
muOpen endp

muRead proc near
       mov  eax, ebx
       call freeldr_seek

       cmp  ecx, 0
       jnz  later1
       mov  ecx, 0ffffffffh
later1:
       mov  eax, edx
       mov  edx, ecx
       call freeldr_read

       mov  ebx, eax

       ret
muRead endp

muClose proc near
       call freeldr_close

       ret
muClose endp


endif


_TEXT   ends

        end
