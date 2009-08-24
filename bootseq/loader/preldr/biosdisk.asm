;
; biosdisk.asm
;

name biosdisk

.386p

public _text16_end

public biosdisk_int13_extensions
public biosdisk_standard
public check_int13_extensions
public get_diskinfo_standard

extrn  call_rm       :near
extrn  force_lba     :byte

include fsd.inc

_TEXT16 segment dword public 'CODE' use16

;
; int biosdisk_int13_extensions (int ax, int drive, void *dap);
;

biosdisk_int13_extensions_rm proc far
      mov     ax, bx
      mov     ds, cx
      int     13h             ; do the operation
      mov     dl, ah          ; save return value
      ; clear the data segment
      xor     ax, ax
      mov     ds, ax
      ; back to protected mode
      retf
biosdisk_int13_extensions_rm endp

;
; int biosdisk_standard (int ah, int drive,
;                       int coff, int hoff, int soff,
;                       int nsec, int segment);
;

biosdisk_standard_rm proc far
      mov     es, bx
      xor     bx, bx
      mov     si, 3           ; attempt at least three times */

u1:
      mov     ax, di
      int     13h             ; do the operation
      jnc     u2              ; check if successful

      mov     bl, ah          ; save return value
      ; if fail, reset the disk system
      xor     ax, ax
      int     13h

      dec     si
      cmp     si, 0
      je      u2
      xor     bl, bl
      jmp     u1              ; retry
u2:
      ; back to protected mode
      retf
biosdisk_standard_rm endp

;
; int check_int13_extensions (int drive);
;


check_int13_extensions_rm proc far
      mov     ah, 41h
      mov     bx, 55aah
      int     13h             ; do the operation

      ; check the result
      jc      uu1
      cmp     bx, 0aa55h
      jne     uu1

      mov     bl, ah          ; save the major version into bl

      ; check if AH=0x42 is supported if FORCE_LBA is zero
      mov     al, force_lba
      test    al, al
      jnz     uu2
      and     cx, 1
      jnz     uu2

uu1:
      xor     bl, bl
uu2:
      ; back to protected mode
      retf
check_int13_extensions_rm endp

;
; int get_diskinfo_standard (int drive,
;                            unsigned long *cylinders,
;                            unsigned long *heads,
;                            unsigned long *sectors);
;

get_diskinfo_standard_rm proc far
      mov    bh, dl
      and    bh, 7fh          ; clear HDD bit
      mov    ah, 8
      int    13h              ; do the operation
      ; check if successful
      test   ah, ah
      jnz    f1
      ; bogus BIOSes may not return an error number
      test   cl, 3fh          ; 0 sectors means no disk
      jnz    f1               ; if non-zero, then succeed

      pusha
      mov    al, dl
      add    al, 30h
      xor    bx, bx
      mov    ah, 0eh ; ah=0eh -- function
      int    10h
      popa

      cmp    dl, bh           ; dl == number of drives
      jae    f1               ; if non-zero, then succeed
      ; XXX 0x60 is one of the unused error numbers
      mov    ah, 60h
f1:
      mov    bl, ah           ; save return value in bl
      ; back to protected mode
      retf
get_diskinfo_standard_rm endp

_text16_end:

_TEXT16 ends


_TEXT segment dword public 'CODE'  use32

;
;   int biosdisk_int13_extensions (int ax, int drive, void *dap)
;
;   Call IBM/MS INT13 Extensions (int 13 %ax=AX) for DRIVE. DAP
;   is passed for disk address packet. If an error occurs, return
;   non-zero, otherwise zero.
;

biosdisk_int13_extensions proc near
        push    ebp
        mov     ebp, esp

        push    esi
        push    ebx

        ; compute the address of disk_address_packet
        mov     eax, [ebp + 10h]
        mov     si, ax
        xor     ax, ax
        shr     eax, 4
        mov     cx, ax          ; save the segment to cx

        ; drive
        mov     dl, [ebp + 0ch]
        ; ax
        mov     bx, [ebp + 8h]
        ; enter real mode
ifdef REAL_BASE
        mov     eax, REAL_BASE
else
ifdef MB_KERN
        mov     eax, REL1_BASE
else
        mov     eax, STAGE0_BASE
endif
endif
        shl     eax, 12
        mov     ax,  offset _TEXT16:biosdisk_int13_extensions_rm
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4

        xor     eax, eax
        mov     al, dl          ; return value in eax

        pop     ebx
        pop     esi

        pop     ebp

        ret
biosdisk_int13_extensions endp

;
;   int biosdisk_standard (int ah, int drive, int coff, int hoff, int soff,
;                          int nsec, int segment)
;
;   Call standard and old INT13 (int 13 %ah=AH) for DRIVE. Read/write
;   NSEC sectors from COFF/HOFF/SOFF into SEGMENT. If an error occurs,
;   return non-zero, otherwise zero.
;

biosdisk_standard proc near
        push    ebp
        mov     ebp, esp

        push    edx
        push    ecx

        push    ebx
        push    edi
        push    esi
        ; set up CHS information
        mov     eax, [ebp + 10h]
        mov     ch, al
        mov     al, [ebp + 18h]
        shl     al, 2
        shr     ax, 2
        mov     cl, al
        mov     dh, [ebp + 14h]
        ; drive
        mov     dl, [ebp + 0ch]
        ; segment
        mov     bx, [ebp + 20h]
        ; save nsec and ah to di
        mov     ah, [ebp + 8h]
        mov     al, [ebp + 1ch]
        mov     di, ax
        ; enter real mode
ifdef REAL_BASE
        mov     eax, REAL_BASE
else
ifdef MB_KERN
        mov     eax, REL1_BASE
else
        mov     eax, STAGE0_BASE
endif
endif
        shl     eax, 12
        mov     ax,  offset _TEXT16:biosdisk_standard_rm
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4

        ;xor     eax, eax
        ;mov     al, bl             ; return value in eax
        movzx    eax, bl

        pop     esi
        pop     edi
        pop     ebx

        pop     ecx
        pop     edx

        pop     ebp

        ret
biosdisk_standard endp

;
;   int check_int13_extensions (int drive)
;
;   Check if LBA is supported for DRIVE. If it is supported, then return
;   the major version of extensions, otherwise zero.
;

check_int13_extensions proc near
        push    ebp
        mov     ebp, esp

        push    ebx

        ; drive
        mov     dl, [ebp + 8]
        ; enter real mode
ifdef REAL_BASE
        mov     eax, REAL_BASE
else
ifdef MB_KERN
        mov     eax, REL1_BASE
else
        mov     eax, STAGE0_BASE
endif
endif
        shl     eax, 12
        mov     ax,  offset _TEXT16:check_int13_extensions_rm
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4

        xor     eax, eax
        mov     al, bl             ; return value in eax

        pop     ebx
        pop     ebp

        ret
check_int13_extensions endp

;
;   int get_diskinfo_standard (int drive, unsigned long *cylinders,
;                              unsigned long *heads, unsigned long *sectors)
;
;   Return the geometry of DRIVE in CYLINDERS, HEADS and SECTORS. If an
;   error occurs, then return non-zero, otherwise zero.
;

get_diskinfo_standard proc near
        push    ebp
        mov     ebp, esp

        push    ebx
        push    edi

        ; drive
        mov     dl, [ebp + 8]
        ; enter real mode
ifdef REAL_BASE
        mov     eax, REAL_BASE
else
ifdef MB_KERN
        mov     eax, REL1_BASE
else
        mov     eax, STAGE0_BASE
endif
endif
        shl     eax, 12
        mov     ax,  offset _TEXT16:get_diskinfo_standard_rm
        push    eax
        xor     eax, eax
        call    call_rm
        add     esp, 4

        xor     eax, eax

        ; restore ebp
        lea     ebp, [esp + 8]

        ; heads
        mov     al, dh
        inc     eax             ; the number of heads is counted from zero
        mov     edi, [ebp + 10h]
        mov     [edi], eax

        ; sectors
        ;xor     eax, eax
        mov     al, cl
        and     al, 3fh
        mov     edi, [ebp + 14h]
        mov     [edi], eax

        ; cylinders
        shr     cl, 6
        mov     ah, cl
        mov     al, ch
        inc     eax             ; the number of cylinders is
                                ; counted from zero
        mov     edi, [ebp + 0ch]
        mov     [edi], eax

        xor     eax, eax
        mov     al, bl          ; return value in eax

        pop     edi
        pop     ebx

        pop     ebp

        ret
get_diskinfo_standard endp

_TEXT ends

      end
