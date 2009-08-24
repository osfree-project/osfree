;
; filesys.asm:
; filesystem access realmode (mu_*-) functions
;

name filesys

public mu_Open
public mu_Read
public mu_Close
public mu_Terminate

public preldr_ds
public preldr_ss_sp
public preldr_es

extrn filemax           :dword
extrn call_pm           :near
extrn ufs_open          :near
extrn ufs_read          :near
extrn ufs_seek          :near
extrn ufs_close         :near
extrn com_outchar       :near

.386p

switch_to_preldr macro
        push ax
        mov  ax,  ds
        mov  cs:ldr_ds, ax
        mov  ax,  es
        mov  cs:ldr_es, ax
        mov  ax, sp
        inc  ax                                  ; skip ax in stack
        inc  ax                                  ;
        mov  word ptr cs:ldr_ss_sp, ax
        mov  ax, ss
        mov  word ptr cs:ldr_ss_sp + 2, ax

        mov  ax,  cs:preldr_ds
        mov  ds,  ax
        mov  ax,  cs:preldr_es
        mov  es,  ax
        pop  ax
        lss  sp,  cs:preldr_ss_sp
endm

switch_to_ldr macro
        push ax
        mov  ax, ds
        mov  preldr_ds, ax
        mov  ax, es
        mov  preldr_es, ax
        mov  ax, sp
        inc  ax                                  ; skip ax in stack
        inc  ax                                  ;
        mov  word ptr preldr_ss_sp, ax
        mov  ax, ss
        mov  word ptr preldr_ss_sp + 2, ax

        mov  ax, ldr_ds
        mov  ds, ax
        mov  ax, ldr_es
        mov  es, ax
        pop  ax
        lss  sp, ldr_ss_sp
endm


_TEXT16 segment dword public 'CODE' use16

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

        push ebx
        push ecx

        ; char far *pName
        mov  ebx, dword ptr [bp + 06h]

        switch_to_preldr

        ; convert far ptr in ebx to
        ; FLAT ptr.
        mov  ecx, ebx
        shr  ecx, 16
        shl  ecx, 4
        and  ebx, 0ffffh
        ;and  ecx, 0fffffh
        add  ebx, ecx

        mov  al, 'o'
        call com_outchar

        ; switch to PM and call muOpen
        mov  eax, offset _TEXT:muOpen
        push eax
        call call_pm
        add  sp, 4

        mov  al, 'e'
        call com_outchar

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
        mov  es:[di], edx ; size
        xor  dx, dx
        pop  di
        pop  es

        pop  ecx
        pop  ebx

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

        push ebx
        push ecx

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

        mov  al, 'r'
        call com_outchar

        mov  eax, offset _TEXT:muRead
        push eax
        call call_pm
        add  sp, 4

        mov  al, 'e'
        call com_outchar

        ; ebx (count of bytes read) -> dx:ax
        mov  edx, ebx
        shr  edx, 16
        mov  eax, ebx
        and  eax, 0ffffh
        ;and  edx, 0ffffh

        switch_to_ldr

        pop  ecx
        pop  ebx

        pop  bp

        retf
mu_Read endp

;
; void __cdecl
; mu_Close(void);
;

mu_Close proc far
        switch_to_preldr

        mov  al, 'c'
        call com_outchar

        mov  eax, offset _TEXT:muClose
        push eax
        call call_pm
        add  sp, 4

        mov  al, 'e'
        call com_outchar

        switch_to_ldr

        retf
mu_Close endp

;
; void __cdecl
; mu_Terminate(void);
;

mu_Terminate proc far
        mov  al, 't'
        call com_outchar

        mov  al, 'e'
        call com_outchar

        retf
mu_Terminate endp

ldr_ds       dw 0
ldr_ss_sp    dd 0
ldr_es       dw 0

preldr_ds    dw 0
preldr_ss_sp dd 0
preldr_es    dw 0


_TEXT16 ends

_TEXT   segment dword public 'CODE' use32


muOpen proc near
       mov  eax, ebx
       call ufs_open

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
       call ufs_seek

       cmp  ecx, 0
       jnz  later1
       mov  ecx, 0ffffffffh
later1:
       mov  eax, edx
       mov  edx, ecx
       call ufs_read

       mov  ebx, eax

       ret
muRead endp

muClose proc near
       call ufs_close

       ret
muClose endp


_TEXT   ends

        end
