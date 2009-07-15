;
; os2bird.asm
; Copyleft (c) Knut St. Osmundsen
; modified for osFree project
; by Valery V. Sedletski
; (Entry table parsing for NE
; format modules added)
;

.386p

extrn FlatR0CS     :word
extrn FlatR0DS     :word
extrn DevHlp       :dword
extrn _kprintf     :near

public GetFlatSelectors
public GetProcAddr
public GetModule

;include ldr.inc

SAS_selector          equ   70h
SAS_CBSIG             equ   4

SAS struc
SAS_signature   db      SAS_CBSIG dup (?)
SAS_tables_data dw      ?
SAS_flat_sel    dw      ?
SAS_config_data dw      ?
SAS_dd_data     dw      ?
SAS_vm_data     dw      ?
SAS_task_data   dw      ?
SAS_RAS_data    dw      ?
SAS_file_data   dw      ?
SAS_info_data   dw      ?
SAS ends

SAS_vm_section struc
SAS_vm_arena    dd      ?
SAS_vm_object   dd      ?
SAS_vm_context  dd      ?
SAS_vm_krnl_mte dd      ?
SAS_vm_glbl_mte dd      ?
SAS_vm_pft      dd      ?
SAS_vm_prt      dd      ?
SAS_vm_swap     dd      ?
SAS_vm_idle_head dd      ?
SAS_vm_free_head dd      ?
SAS_vm_heap_info dd      ?
SAS_vm_all_mte  dd      ?
SAS_vm_section ends

MTE struc
mte_flags2      dw      ?
mte_handle      dw      ?
mte_swapmte     dd      ?
mte_link        dd      ?
mte_flags1      dd      ?
mte_impmodcnt   dd      ?
mte_sfn         dw      ?
mte_usecnt      dw      ?
mte_modname     db      8 dup (?)
MTE ends

OTE struc
ote_size        dd      ?
ote_base        dd      ?
ote_flags       dd      ?
ote_pagemap     dd      ?
ote_mapsize     dd      ?
ote_sel DW      ?
ote_hob DW      ?
OTE ends

SMTE struc
smte_mpages     dd      ?
smte_startobj   dd      ?
smte_eip        dd      ?
smte_stackobj   dd      ?
smte_esp        dd      ?
smte_pageshift  dd      ?
smte_fixupsize  dd      ?
smte_objtab     dd      ?
smte_objcnt     dd      ?
smte_objmap     dd      ?
smte_itermap    dd      ?
smte_rsrctab    dd      ?
smte_rsrccnt    dd      ?
smte_restab     dd      ?
smte_enttab     dd      ?
smte_fpagetab   dd      ?
smte_frectab    dd      ?
smte_impmod     dd      ?
smte_impproc    dd      ?
smte_datapage   dd      ?
smte_nrestab    dd      ?
smte_cbnrestab  dd      ?
smte_autods     dd      ?
smte_debuginfo  dd      ?
smte_debuglen   dd      ?
smte_heapsize   dd      ?
smte_path       dd      ?
smte_semcount   dw      ?
smte_semowner   dw      ?
smte_pfilecache dd      ?
smte_stacksize  dd      ?
smte_alignshift dw      ?
smte_NEexpver   dw      ?
smte_pathlen    dw      ?
smte_NEexetype  dw      ?
smte_csegpack   dw      ?
SMTE ends

OBJEXEC               equ   00000004h
MTEFORMATLX           equ   0002h

DevHlp_GetDOSVar      equ   36

ERROR_PROC_NOT_FOUND  equ   127
ERROR_BAD_EXE_FORMAT  equ   193

_TEXT    segment byte public 'CODE'  use16

;
; int _cdecl _loadds GetFlatSelectors(void);
;
GetFlatSelectors proc near
    ASSUME  ds:_DATA, ss:NOTHING, es:NOTHING
    ; prolog.
    push    bp
    mov     bp, sp

    ;
    ; Save registers. (paranoia)
    ;
    push    es
    push    ds
    push    bx
    push    cx
    push    dx


    ;
    ; Get the DosTables
    ;
    xor     cx, cx
    mov     al, 9                       ; Undocumented by IBM, gets the dostables.
    mov     dl, DevHlp_GetDOSVar
    call dword ptr DevHlp
    jnc     ok1
    mov     ax, 0ffffh                  ; just set some value which isn't 0.
    jmp     return1
ok1:

    ;
    ; Now we have to skip the first table cause the good stuff is
    ; in the second one.
    ; The tables have an byte field at the start giving it's lenght in
    ; number of dwords.
    ;
    mov     es, ax                      ; now es:bx points to the first dos table.
    xor     ax, ax
    mov     al, es:[bx]                 ; Read the number of DWORDs in the table
    shl     al, 2                       ; Calculate the end of the table.
    inc     al                          ; (+1 for the char at start of the table)
    add     bx, ax                      ; now es:bx points to the second dos table.

    inc     bx                          ; skip the leading dword count.
    mov     ax, es:[bx+(9*4)]           ; R0FlatCS
    mov     FlatR0CS, ax
    mov     ax, es:[bx+(10*4)]          ; R0FlatDS
    mov     FlatR0DS, ax

    ;
    ; Success!
    ;
    xor     ax, ax

    ;
    ; Restore registers and return.
    ;
return1:
    pop     dx
    pop     cx
    pop     bx
    pop     ds
    pop     es
    leave
    ret
GetFlatSelectors endp

;
; int _cdecl _loadds GetModule(char far *fpszModName,
;                              unsigned short cchModName,
;                              struct p48 far *fpMte48);
;
GetModule proc near
    ASSUME  ds:_DATA, ss:NOTHING, es:NOTHING
    ; prolog.
    push    bp
    mov     bp, sp

    ;
    ; Save registers. (paranoia)
    ;
    push    es
    push    ds
    push    ebx
    push    ecx
    push    edx
    push    edi
    push    esi
    push    eax                         ; need to preserve high part of eax.


    ;
    ; Make a SAS selector.
    ; (This is a special kernel selector (70h readonly / 78h readwrite?))
    ;
    mov     ax, SAS_selector
    mov     es, ax                      ; es:0 is now pointing to 'struct SAS'.


    ;
    ; We now wanna find the vm section.
    ; And then we read the module chain.
    ;
    xor     bx, bx                      ; stupid me / alp
    ; ASSUME bx:ptr SAS
    mov     bx, es:[bx].SAS_vm_data     ; es:ax is now pointing to 'struct SAS_vm_section'
    ; ASSUME bx:ptr SAS_vm_section
    mov     eax, es:[bx].SAS_vm_all_mte ; This is a flat 32-bit pointer.

    ;
    ; Now we need to do flat stuff. ie. load flat ds into es.
    ;
    mov     bx, FlatR0DS
    mov     es, bx
    ASSUME es:nothing
    mov     eax,es:[eax]                ; This was a flat pointer to mte_h.
                                        ; We need mte_h which is the head pointer.
    ; ASSUME eax:far ptr MTE              ; (hope this is ok)


    ;
    ; Before walking the list we should prepare the modulename compare
    ; We load the module name into ecx and edx and padds it with zeros.
    ; ASSUMES (correctly) cchModName <= 8.
    ;
    mov     bx, ss:[bp + 8]             ; length of the module name.
    lds     si, ss:[bp + 4]             ; load far pointer to module name. (ds:si)
ifndef Drova
    xor     edx, edx
    xor     ecx, ecx
highdword:
    dec     bx
    cmp     bx, 3
    jle     lowdword
    shl     edx, 8
    mov     dl, byte ptr ds:[si+bx]
    jmp     highdword

lowdword:
    shl     ecx, 8
    mov     cl, byte ptr ds:[si+bx]
    dec     bx
    jns     lowdword

    ASSUME ds:nothing


    ;
    ; Walk thru the loaded modules looking for one with a
    ; matching internal name.
    ;
else
    push        eax
    xor     edx, edx
    lodsd
    mov     ecx,eax
    cmp     bx,4
    jle     @@moduleloop1
    cmp     bx,7
    jl       @@low7
    lodsd
    jg      @@moduleloop1
    mov     edx,eax
    jmp      @@moduleloop1
@@low7:
    lodsw
    mov     dx,ax
    shl     edx,16
    xor     dx,dx
    ASSUME ds:nothing
    ;
    ;
    ;
@@moduleloop1:
    pop    eax
endif

moduleloop:
    ; check if end of list. (NULL pointer)
    or      eax, eax                    ; NULL?
    jnz      ok2
    pop     eax                         ; preserve high word.
    mov     ax, 2                       ; error file not found.
    jmp     return2
ok2:
    ; Compare filename
    cmp     dword ptr es:[eax].mte_modname, ecx
    jne     next
    cmp     dword ptr es:[eax + 4].mte_modname, edx
    jne     next
    jmp     found

    ; next module
next:
    mov     eax, es:[eax].mte_link      ; mte_link points to the next mte in the list.
    jmp     moduleloop


    ;
    ; We found the module we looked for.
    ;
found:
    lds     bx, ss:[bp + 0ah]           ; pointer to where to store the 16:32
                                        ; of the module MTE.
    mov     dword ptr ds:[bx], eax      ; Store the flat address
    mov     ax, es
    mov     word ptr ds:[bx+4], ax      ; Store the flat selector
    pop     eax
    xor     ax, ax                      ; return success.


    ;
    ; Restore registers and return.
    ;
return2:
    pop     esi
    pop     edi
    pop     edx
    pop     ecx
    pop     ebx
    pop     ds
    pop     es

    leave

    ret
GetModule endp

;
; int _cdecl _loadds GetProcAddr(char far *fpszModName,
;                                unsigned short cchModName,
;                                char far *fpszProcName,
;                                unsigned char cchProcName,
;                                struct p48 far *fp48EP);
;

fpMte48         equ word  ptr [bp - 10h]
fpszModName     equ dword ptr [bp + 4]
cchModName      equ word  ptr [bp + 8]
fpszProcName    equ dword ptr [bp + 0ah]
cchProcName     equ word  ptr [bp + 0eh]
fp48EP          equ [bp + 10h]

GetProcAddr proc near
    ASSUME  ds:_DATA, ss:NOTHING, es:NOTHING
    ; prolog.
    push    bp
    mov     bp, sp
    sub     sp, 10h                         ; reserve space for variables

    mov     ax, ds
    mov     fs, ax

    ;
    ; Save registers. (paranoia)
    ;
    push    fs
    push    es
    push    ds
    push    ebx
    push    ecx
    push    edx
    push    edi
    push    esi


    ;
    ; Get the Module MTE.
    ;
    ;
    push    ss
    lea     ax, fpMte48
    push    ax
    push    cchModName
    push    fpszModName
    call    GetModule
    add     sp, 0ah
    or      ax, ax
    jz      module_ok
    jmp     return3


    ;
    ; Get swapmte
    ;
module_ok:
    push    eax
    mov     ax,   word ptr fpMte48+4
    mov     ds, ax
    mov     eax, dword ptr fpMte48

    push    ds
    pushad

    push    eax
    push    ds
    mov     ax, fs
    mov     ds, ax
    push    ds
    lea     ax, DGROUP:mte_msg
    push    ax
    call    _kprintf
    add     sp, 10

    popad
    pop     ds

    ASSUME ds:nothing ; , eax:far ptr MTE
    ; !!!!!
    push    ax

    mov     ax, [eax].mte_flags2        ; check 'FORMAT LX'
    test    ax, MTEFORMATLX             ; flag in MTE
    jnz     skip_ne                     ; if not,
    mov     byte ptr fs:ne_flag, 1      ; set 'NE' flag
skip_ne:
    pop     ax

    ; !!!!!
    mov     eax, [eax].mte_swapmte
    ASSUME ds:nothing ; , eax:far ptr SMTE
    or      eax, eax
    jnz     swapmte_ok
    pop     eax
    mov     ax, ERROR_BAD_EXE_FORMAT
    jmp     return3

    ;
    ; Get resident name table
    ;
swapmte_ok:
    push    ds
    pushad

    push    eax
    push    ds
    mov     ax, fs
    mov     ds, ax
    push    ds
    lea     ax, DGROUP:swapmte_msg
    push    ax
    call    _kprintf
    add     sp, 10

    popad
    pop     ds

    mov     ecx, [eax].smte_restab
    or      ecx, ecx
    jnz     restab_ok
    mov     ax, ERROR_PROC_NOT_FOUND
    jmp     return3


    ;
    ; Now we're gonna search this table.
    ; The layout is repetitions of this structure:
    ;   <db length><string><dw ordinal>
    ; The table is terminated by a string of length zero.
    ;
restab_ok:
    ; skip the first name as that is the module name.
    xor     edx, edx
    mov     dl, [ecx]
    cmp     word ptr [ecx + edx + 1], 0 ; check that's it's ordinal 0.
    je      nameloop
    add     ecx, edx
    add     ecx, 3

nameloop:
    xor     edx, edx
    mov     dl, [ecx]
    or      dl, dl
    jnz     namenotlast
    jmp     namenotfound
namenotlast:
    cmp     dx, cchProcName
    jne     namenext

    ;
    ; Compare the names.
    ;
    les     di, fpszProcName          ; load ds:di with proc name pointer.
    mov     esi, ecx
    inc     esi                         ; skip size byte. esi points to name now.
    dec     dx
namecomp:
    mov     bl, ds:[esi]
    cmp     bl, es:[di]
    jnz     namenext
    inc     esi
    inc     di
    dec     dx
    jns     namecomp
    ; match!
    mov     dx, [esi]                   ; read the ordinal for this name.
    jmp     namefound

namenext:
    ; Advance the pointer to the next name
    xor     edx, edx
    mov     dl, [ecx]
    add     ecx, edx                    ; + length of name
    add     ecx, 3                      ; + size byte and ordinal word
    jmp     nameloop

    ;
    ; Didn't find the name.
    ;
namenotfound:
    pop     eax
    mov     ax, ERROR_PROC_NOT_FOUND
    jmp     return3


    ;
    ; Found name. Now we got to search the entry table for this.
    ;
namefound:
    push    ds
    pushad

    push    dx
    mov     ax, fs
    mov     ds, ax
    push    ds
    lea     ax, DGROUP:ord_msg
    push    ax
    call    _kprintf
    add     sp, 6

    popad
    pop     ds

    mov     ecx, [eax].smte_enttab
    or      ecx, ecx
    jnz     enttab_ok
    pop     eax
    mov     ax, ERROR_BAD_EXE_FORMAT
    jmp     return3

    ;
    ; The entry table is organized in bundles.
    ; (DX is the ordinal we should find.)
    ;
enttab_ok:
    push    ds
    pushad

    push    ecx
    push    ds
    mov     ax, fs
    mov     ds, ax
    push    ds
    lea     ax, DGROUP:enttab_msg
    push    ax
    call    _kprintf
    add     sp, 10

    popad
    pop     ds
    ;
    ; Now ecx points to the entry table
    ; dx == ordinal
    ;
entloop:
    mov     bx, [ecx]                   ; cnt & type
    or      bl, bl
    jnz     entnotlast
    jmp     entnotfound
entnotlast:
    push    ds
    pushad

    push    dx
    push    bx
    mov     ax, fs
    mov     ds, ax
    push    ds
    lea     ax, DGROUP:bundle_msg
    push    ax
    call    _kprintf
    add     sp, 8

    popad
    pop     ds

    cmp     byte ptr fs:ne_flag, 0
    je      lx_logic

ne_logic:
    push    ds
    pushad

    mov     ax, fs
    mov     ds, ax
    push    ds
    lea     ax, DGROUP:ne_msg
    push    ax
    call    _kprintf
    add     sp, 4

    popad
    pop     ds

    mov     si, bx
    and     si, 0ffh
    or      bh, bh
    jz      entnextskip
    cmp     si, dx
    jnge    entnextused1
    jmp     entfound

entnextused1:
    mov     bh, 1
    mov     bl, 3
    jmp     entnext

lx_logic:
    mov     si, bx
    and     si, 0ffh                    ; 16-bit count.
    or      bh, bh                      ; type
    jz      entnextskip                 ; skip unused entries
    cmp     si, dx
    jnge    entnextused
    jmp     entfound

entnextused:
    and     bh, 0fh
    dec     bh
    jnz     next_2
    mov     bl, 3                       ; type 1
    jmp     entnext
next_2:
    dec     bh
    jnz     next_3
    mov     bl, 5                       ; type 2
    jmp     entnext
next_3:
    dec     bh
    jnz     next_4
    mov     bl, 5                       ; type 3
    jmp     entnext
next_4:
    dec     bh
    jnz     next_5
    mov     bl, 7                       ; type 4
    jmp     entnext
next_5:
    jmp     entnotfound

entnextskip:                            ; skip unused entries
    sub     dx, si
    jl      entnotfound                 ; if underflow.
    add     ecx, 2                      ; cnt & type
    jmp     entloop

entnext:      ; this sucks!!!!!!!!!
    push    eax
    push    edx
    xor     eax, eax
    mov     al, bl
    mul     si
    add     ecx, eax                    ; entries size
    cmp     byte ptr fs:ne_flag, 1      ; !!!!!
    jz      skip0
    add     ecx, 2                      ; the object index
skip0:                                  ; !!!!!
    pop     edx
    pop     eax
    jmp     entnextskip

entnotfound:
    pop     eax
    mov     ax, ERROR_BAD_EXE_FORMAT
    jmp     return3


    ;
    ; Found the right bundle.
    ;   ecx points to the bundle
    ;   dl is the 1 based index of the entry we want in the bundle.
    ;   bl is the count.
    ;   bh is the flags.
    ;
entfound:
    push    ds
    pushad

    xor     ax, ax
    mov     al, bh
    push    ax
    mov     ax, fs
    mov     ds, ax
    push    ds
    lea     ax, DGROUP:ent_msg
    push    ax
    call    _kprintf
    add     sp, 6

    popad
    pop     ds

    and     bh, 0fh
    cmp     bh, 1
    je      ent_1
    cmp     bh, 3
    jz      ent_3
    jmp     entnotfound                 ; sorry, 4 and 2 isn't supported!

    ;
    ; Load the offset into edx.
    ;
ent_1:
    mov     bl, 3
    jmp     entcalc_1_3

ent_3:
    mov     bl, 5
    jmp     entcalc_1_3

entcalc_1_3:
    push    eax
    xor     eax, eax
    dec     dx
    mov     al, bl
    mul     dx
    add     eax, ecx                    ; Add bundle base.
    cmp     byte ptr fs:ne_flag, 1
    jnz     lx1
ne1:                                    ; !!!!!
    add     eax, 2                      ;
    jmp     skip1
lx1:
    add     eax, 4                      ; now points to the entry in question.
skip1:                                  ; !!!!!
    mov     edx, dword ptr [eax+1]
    pop     eax
    cmp     bl, 5                       ; 16-bit offset?
    je      entobject
    and     edx, 0ffffh

    ;
    ; We now need to get the base address and selector of the object.
    ; (edx is now a 32-bit offset into the object)
    ;
entobject:
    push    ds
    pushad

    push    edx
    mov     ax, fs
    mov     ds, ax
    push    ds
    lea     ax, DGROUP:off_msg
    push    ax
    call    _kprintf
    add     sp, 8

    popad
    pop     ds

    cmp     byte ptr fs:ne_flag, 1
    jnz     lx2
ne2:
    mov     cl, [ecx + 1]
    xor     ch, ch
    jmp     skip2
lx2:
    mov     cx, [ecx + 2]               ; the object table index.
skip2:
    push    ds
    pushad

    push    cx
    mov     ax, fs
    mov     ds, ax
    push    ds
    lea     ax, DGROUP:cx_msg
    push    ax
    call    _kprintf
    add     sp, 6

    popad
    pop     ds

    or      cx, cx
    jz      storeptr
    cmp     cx, word ptr [eax].smte_objcnt
    jng     objcnt_ok
    pop     eax
    mov     ax, ERROR_BAD_EXE_FORMAT
    jmp     return3

objcnt_ok:
    mov     eax, [eax].smte_objtab
    push    eax
    push    edx
    xor     eax, eax
    mov     ax, size OTE
    dec     cx                          ; 1-based!
    mul     cx
    mov     ecx, eax
    pop     edx
    pop     eax
    add     eax, ecx                    ; points to OTE entry.
    ; ASSUME  eax: far ptr OTE

    ;
    ; Get the right selector and evt. add object base to offset.
    ; (bh is the bundle type.)
    ;
    cmp     bh, 1
    je      obj_1
    cmp     bh, 3
    je      obj_3

    int     3                           ; someone have messed with bh!!!

obj_1:
    mov     cx, [eax].ote_sel
    jmp storeptr

obj_3:
    add     edx, [eax].ote_base
    mov     cx, seg _DATA
    mov     es, cx
    ASSUME es:_DATA
    test    [eax].ote_flags, OBJEXEC
    jz      obj_3_DS
    mov     cx, es:FlatR0CS
    jmp storeptr

obj_3_DS:
    mov     cx, es:FlatR0DS
    jmp storeptr


    ;
    ; Store the pointer, cx:edx.
    ;
storeptr:
    push    ds
    pushad

    push    cx
    push    bx
    mov     ax, fs
    mov     ds, ax
    push    ds
    lea     ax, DGROUP:sto_msg
    push    ax
    call    _kprintf
    add     sp, 8

    popad
    pop     ds

    les     di, fp48EP
    mov     es:[di], edx
    mov     es:[di+4], cx
    pop     eax
    xor     ax, ax

    ;
    ; Restore registers and return.
    ;
return3:
    pop     esi
    pop     edi
    pop     edx
    pop     ecx
    pop     ebx
    pop     ds
    pop     es
    pop     fs

    leave

    ret
GetProcAddr endp

_TEXT    ends

CONST    segment byte  public 'DATA'  use16
CONST    ends
CONST2   segment byte  public 'DATA'  use16
CONST2   ends
_DATA    segment dword public 'DATA'  use16
_DATA    ends

DGROUP   group CONST,CONST2,_DATA

_DATA    segment dword public 'DATA'  use16

mte_msg       db  "IFS MTE entry @ 0x%04x:0x%08lx",13,10,0
swapmte_msg   db  "SwapMTE address @ 0x%04x:0x%08lx",13,10,0
enttab_msg    db  "Entry table @ 0x%04x:0x%08lx",13,10,0
bundle_msg    db  "bundle cnt & type: 0x%04x, ord: #%u",13,10,0
ord_msg       db  "Ordinal #%u",13,10,0
ent_msg       db  "ent: %u",13,10,0
off_msg       db  "entry offset: 0x%08lx",13,10,0
sto_msg       db  "cnt&type: 0x%04x, sel: 0x%04x",13,10,0
ne_msg        db  "this is an NE executable",13,10,0
cx_msg        db  "CX = %u",13,10,0

ne_flag       db  0

_DATA    ends

         end
