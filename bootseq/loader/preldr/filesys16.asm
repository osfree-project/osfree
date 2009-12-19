;
; filesys16.asm:
; wrappers for 16-bit uFSD functions
;

name filesys16

.386p

include loader.inc
include fsd.inc

public  mu_Open_wr
public  mu_Read_wr
public  mu_Close_wr
public  mu_Terminate_wr

extrn   filetab_ptr16      :dword
extrn   call_rm            :near
extrn   stage0base        :dword

_TEXT16 segment dword public 'CODE' use16

muOpen_wr proc far
        push edx
        push ebx

        mov  eax, filetab_ptr16
        call dword ptr [eax].ft_muOpen

        ; copy return code to ebx
        xor  ebx, ebx
        mov  bx, ax

        retf
muOpen_wr endp

muRead_wr proc far
        push edx
        push ecx
        push ebx

        mov  eax, filetab_ptr16
        call dword ptr [eax].ft_muRead

        ; copy return code to ebx
        mov  bx, dx
        shl  ebx, 16
        mov  bx, ax

        retf
muRead_wr endp

muClose_wr proc far
        mov  eax, filetab_ptr16
        call dword ptr [eax].ft_muClose

        retf
muClose_wr endp

muTerminate_wr proc far

muTerminate_wr endp
        mov  eax, filetab_ptr16
        call dword ptr [eax].ft_muTerminate

        retf
_TEXT16 ends

_TEXT   segment dword public 'CODE' use32

;
; Open file using MicroFSD
;
; unsigned short __cdecl
; mu_Open_wr(char *pName,
;         unsigned long *pulFileSize);
;
mu_Open_wr proc near
        push ebp
        mov  ebp, esp

        ; char *pName
        mov  ebx, dword ptr [ebp + 08h]

        ; convert FLAT ptr in EBX to
        ; FAR ptr.
        mov  ecx, ebx
        shl  ecx, 12
        and  ebx, 0fh
        mov  cx, bx
        mov  ebx, ecx

        ; unsigned long *pulFileSize
        mov  edx, dword ptr [ebp +0ch]

        ; convert FLAT ptr in EDX to
        ; FAR ptr
        mov  ecx, edx
        shl  ecx, 12
        and  edx, 0fh
        mov  cx, dx
        mov  edx, ecx

        ; switch to PM and call muOpen
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
        shl  eax, 12
        mov  ax, offset _TEXT16:muOpen_wr
        push eax
        call call_rm
        add  esp, 4

        mov  eax, ebx

        pop  ebp

        ret
mu_Open_wr endp

;
; unsigned long  __cdecl
; mu_Read_wr(long loffseek,
;         char *pBuf,
;         unsigned long cbBuf);
;
mu_Read_wr proc near
        push ebp
        mov  ebp, esp

        ; long loffseek
        mov  ebx, dword ptr [ebp + 08h]

        ; char *pBuf
        mov  ecx, dword ptr [ebp + 0ch]

        ; convert FLAT ptr in ECX to
        ; FAR ptr
        mov  edx, ecx
        shl  edx, 12
        and  ecx, 0fh
        mov  dx, cx
        mov  ecx, edx

        ; unsigned long cbBuf
        mov  edx, dword ptr [ebp + 10h]

        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
        shl  eax, 12
        mov  ax, offset _TEXT16:muRead_wr
        push eax
        call call_rm
        add  esp, 4

        mov  eax, ebx

        pop  ebp

        ret
mu_Read_wr endp

;
; void __cdecl
; mu_Close_wr(void);
;
mu_Close_wr proc near
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
        shl  eax, 12
        mov  ax, offset _TEXT16:muClose_wr
        push eax
        call call_rm
        add  esp, 4

        ret
mu_Close_wr endp

;
; void __cdecl
; mu_Terminate_wr(void);
;
mu_Terminate_wr proc near
        mov     eax, offset _TEXT:stage0base
        mov     eax, [eax]
        shl  eax, 12
        mov  ax, offset _TEXT16:muTerminate_wr
        push eax
        call call_rm
        add  esp, 4

        ret
mu_Terminate_wr endp

_TEXT   ends

        end
