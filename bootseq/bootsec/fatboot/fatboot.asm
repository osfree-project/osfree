;
; FAT bootsector
; (c) Copyleft osFree project
;
; inspired by os2ldr.sek by Veit Kannegieser
;
; valerius, May 2007
; (Valery V. Sedletski, _valerius (at) mail (dot) ru)
;
; (+) Can handle both FAT12 and FAT16 volumes
; (+) Supports loading files from any subdirectory of any depth
; (+) Uses a binary config file which contains file paths
;     and their load addresses
; (+) Config file by default resides in /boot/bootsec.cfg
; (+) Config path is customizable. You can change it with any
;     filename and subdirectory name. A subdir must reside
;     in the root directory
; (+) Can load any number of files
; (+) After loading a number of files it jumps to the last file
; (+) Configures registers according to the standard protocol
;     between a micro IFS and OS/2 loader
; (+) Both LBA and CHS addressing is supported
; (-) No support for non-standard format floppies
; (*) Presently hangs when booting from a flash disk on some
;     BIOSes. (?)
; (+) Like os2ldr.sek, can load OS2LDR directly, without
;     a need in OS2BOOT
; (-) Loads one sector at one read operation
; (-) Error messages are a one-lettered ('R' -- read error;
;     'N' -- a name is not found).
;
; (+) A feature
; (-) An absent feature
; (*) A bug
;

name boot

.386

boot segment para public 'CODE' use16

include fatboot.inc

                org 07c00h
start proc far
                jmp short real_start
lba_flag        db  0                                       ; Here is force_lba flag
                                                            ; instead of nop instruction
oem_id          db                    '[osFree]'            ; System Identifier
BPBlock         bios_parameters_block <>

real_start:
                mov   bp, bt_addr - EXT_PARAMS_SIZE

                xor   si, si

                cli
                mov   ss, si
                mov   sp, bp
                sti

                mov   ds, si

                push  si

                push  si
                push  the_start
                retf
the_start:
                ;xor   dh, dh                                ; CHS/LBA flag

                test  dl, 80h
                jz    short use_chs                         ; floppy drive -- use CHS
no_floppy:
                ;mov   si, bootsig                           ; 0xaa55
                ;ror   si, 8

                mov   ah, 41h                               ; try 41h of int 13h
                mov   bx, 55aah                             ; (probe if LBA is available)
                ;mov   bx, si
                int   13h

                jnc   short use_lba
                cmp   bx, 0aa55h
                ;cmp   bx, bootsig
                jz    short use_lba

                and   cx, 1                                 ; check bit 0 of cx register,
                jnz   short use_lba                         ; if set then int13 ext disk read
use_chs:                                                            ; functions are supported
                inc   si

                ;mov   al, 'C'
                ;call  err
                ;jmp   short $
use_lba:
                ;mov   al, 'L'
                ;call  err
                ;jmp   short $

                mov   al, 'T'
                call  err
                jmp   short $

if 0

                mov   word ptr [bp].force_chs, si
                mov   byte ptr [bp].drive, dl
read_fat:
                ; load FAT at fat_seg
                mov   cx, [bp].bpb.fat_size

                mov   dx, fat_seg
                mov   es, dx
                ;push  fat_seg
                ;pop   es

                xor   bx, bx

                movzx eax, [bp].bpb.res_sectors             ; 1st FAT offset (reserved sectors)

                push  ax

                call  read_run                              ; read FAT
read_root:
                ;mov   dx, fat_seg

                ; load root directory at root_dir_seg
                ;movzx eax, cx                               ;
                mov   ax, cx                                ;  size of FAT max 128k takes 1 byte in sectors
                ;mov   cl, [bp].bpb.n_fats                   ;
                ;mul   cl                                    ; result in ax
                mul   byte ptr [bp].bpb.n_fats

                ;xor   eax, eax
                ;xor   ebx, ebx

                pop   bx                                    ; reserved sectors

                ;add   eax, ebx

                add   ax, bx                                ; root dir offset == res_sectors + fat_size * n_fats
                ;adc   dx, 0
                movzx  eax, ax


                ;shl   edx, 16
                ;add   eax, edx

                ;push  dx                                    ; 1st rootdir sector number in dx:ax
                ;push  ax

                movzx ecx, [bp].bpb.root_dir_size
                shr   ecx, 4
                ;mov   ax, [bp].bpb.root_dir_size
                ;mov   bx, 32
                ;mul   bx
                ;mov   bx, [bp].bpb.sect_size
                ;dec   bx
                ;add   ax, bx
                ;adc   dx, 1
                ;inc   bx
                ;div   bx
                ;mov   cx, ax

                ;pop   eax                                   ; 1st rootdir sector in eax

                mov   ebx, eax
                add   ebx, ecx
                mov   [bp].cluster_base, ebx

                push  root_dir_seg
                pop   es

                xor   bx, bx

                call  read_run                              ; read root directory

                mov   di, bx                                ; es:di --> rootdir

                lea   si, cfg_path
                shl   cx, 9                                 ; convert from sectors to bytes
                mov   bx, scr_seg

                ; load config file at scr_seg:0
                call  load_file_by_path

                mov   ds, bx                                ; ds:si -->
                xor   si, si                                ; scr_seg:0
file_loop:
                ; load a set of files
                ; according to the config file
                mov   bx, word ptr [si]                     ; file load segment address
                inc   si                                    ;
                inc   si                                    ; move to the path beginning

                ; load a file at bx:0
                call  load_file_by_path

                inc   si                                    ; skip ';' symbol
                cmp   byte ptr [si], '='                    ; End of config file marker --
                jne   short file_loop                       ; '=' sign


final_setup:    ; set up registers according to
                ; the standard interface between
                ; bootsector and OS/2 loader
                pop   ds                                    ; ds:si -->
                lea   si, BPBlock                           ; BPB

                                                            ; es:di -->
                                                            ; root directory

                push  bx                                    ; loader segment
                push  di                                    ; 0

                mov   bx, word ptr [bp].cluster_base        ; bx = cluster base
                mov   dx, di                                ; dh = 0
                mov   dl, [bp].drive                        ; dl = boot drive number

                retf                                        ; "return" to the loader
                                                            ; according to the address on stack

endif  ; if 0 -- hide the end of code

start endp

;
; Loads a file by its pathname
; Input:
; es:di --> rootdir
; cx    --  size of root directory
; ds:si --> path to the file
; bx    --  a segment to load at
; Result: loads a file at bx:0
;
load_file_by_path proc near
                push  es
                ;push  cx
path_loop:
                call  find_name

                push  ds

                ;push  fat_seg
                ;pop   ds
                ;mov   dx, fat_seg

                push  bx
                call  load_file
                mov   cx, bx                                ; length in bytes
                pop   bx

                pop   ds

                add   si, 8 + 3                             ; next path component
                xor   di, di                                ; es:di --> subdirectory

                cmp   byte ptr [si], ';'                    ; end of path? (';' -- path end marker)
                jne   short path_loop

                ;pop   cx
                pop   es

                ret
load_file_by_path endp

;
; Loads a file by directory entry
; Input:
; es:di --  file directory entry
; dx    --> fat_seg
; bx    --> segment to load at
; Result:
; File loaded
; its size is returned in bx
;

load_file proc near
                movzx eax, es:[di].dir_fst_clus_lo          ; 1st cluster number

                mov   es, bx
                xor   bx, bx

load_new_cluster:
                push  eax

                ; calculate sector from cluster
                dec   eax
                dec   eax

                movzx di, [bp].bpb.clus_size
                bsf   cx, di
                shl   eax, cl

                mov   cx, di

                add   eax, [bp].cluster_base                ; eax = sector number

                call  read_run                              ; load cluster

                shl   cx, 9
                add   bx, cx

                pop   eax                                   ; cluster number in eax

                call  find_next_cluster

                jb    short load_new_cluster                ; find next cluster
eof:
                ret
load_file endp


;
; Find next cluster number
; Input:
; ax = cluster number
; dx    -- fat_seg
; Output:
; ax = cluster value
;
find_next_cluster proc near
                mov   cx, 0fff1h                            ; EOF

                push  si
                ;push  ds
                push  dx

                mov   si, ax
                add   si, ax                                ; si = 2*ax
                jnc   short no_dx_adjust
                add   dh, 10h                               ; dx = dx + 4096
no_dx_adjust:
                mov   ds, dx
                cmp   [bp].bpb.fat_size, 12                 ; FAT12 or FAT16?

                jae   short fat_16
fat_12:
                add   si, ax                                ; si = 3*ax
                shr   si, 1                                 ; si = 1.5*ax
                mov   ax, [si]                              ; oooo oooo oooo ....|.... oooo oooo oooo
                mov   ch, 0fh                               ; EOF = 0ff1h
                jc    short even_entry
odd_entry:
                shl   ax, 4                                 ; oooo oooo oooo ....|oooo oooo oooo 0000
even_entry:
                shr   ax, 4                                 ; 0000 oooo oooo oooo|0000 oooo oooo oooo
                jmp   short cmp_eof
fat_16:
                mov   ax, [si]                              ; now ax contains FAT entry
cmp_eof:
                cmp   ax, cx                                ; = EOF?

                pop   dx
                ;pop   ds
                pop   si

                ret
find_next_cluster endp


;
; cx == max word length
; compare while nr of symbols <= cx
; and while symbol <> '.'
;
;compare:
;cmploop:
;                cmp    byte ptr [si], '.'
;                je     short ext
;                cmpsb
;                loope  short cmploop
;                jmp    short qt
;ext:
;                cmp    byte ptr [di], ' '
;qt:
;                ret

;
; Find a directory entry by name
; Input:
; es:di -- directory
; cx    -- its size
; ds:si -- name to find
; Returns:
; es:di -- found directory entry
;

find_name proc near
                ;mov    al, '<'
                ;call   err

                ;mov   ax, es
                ;shl   eax, 16
                ;mov   ax, di
                ;call  printhex8
                ;jmp   short $

                shr   cx, 5
cmp_loop:
                pusha
                mov   cx, 8 + 3
                repe  cmpsb
                popa

;                pusha
;                mov   cx, 32
;lll:
;                mov   al, ds:[si]
;                inc   si
;                call  printhex2
;                loop  lll
;                popa

                ;pusha
                ;mov    cx, 8
                ;call   compare
                ;jne    short neql

                ;inc    si
                ;mov    cx, 3
                ;call   compare
                ;popa

                je    short name_found
neql:
                ;mov   ax, 20h
                ;add   di, ax
                ;sub   cx, ax
                ;jcxz  err1
                add    di, 20h
                loop  short cmp_loop
err1:
                mov   al, 'N'
                jmp   short err_name
name_found:
                ;mov   al, ';'
                ;call  err

                ;mov    al, '>'
                ;call   err

                ret
find_name endp


if 0

;
; printhex[248]: Write a hex number in (AL, AX, EAX) to the console
;

printhex2:
        pusha
        rol     eax, 24
        mov     cx, 2
        jmp     pp1
printhex4:
        pusha
        rol     eax, 16
        mov     cx, 4
        jmp     pp1
printhex8:
        pusha
        mov     cx, 8
pp1:
        rol     eax, 4
        push    eax
        and     al, 0Fh
        cmp     al, 10
        jae     high1
low1:
        add     al, '0'
        jmp     pp2
high1:
        add     al, 'A' - 10
pp2:
        mov     bx, 0001h
        mov     ah, 0Eh
        int     10h              ; display a char
        pop     eax
        loop    pp1
        popa

        ret

endif

;
;          Displays an error message (one letter)
;          al: symbol to output
err_read:
                mov    al, 'R'
err_name:
err:
                ; int 10h/ah=0eh: write char on screen in teletype mode
                xor    bx, bx
                mov    ah, 0eh ; ah=0eh -- function

                int    10h
                jmp    short $

;err   proc near
;err_read:
;                lea si, error_dsk_read_error
;                call outstr
;                jmp  short loop3
;err_loader_not_found:
;                lea si, error_loader_not_found
;                call outstr
;loop3:
;                ;jmp  short $
;err   endp


;outstr proc near
;                cld
;disp1:
;                lodsb
;                test   al, al
;                jz     short disp2
;                mov    ah, 0eh  ; int 10h/ah=0eh: write char on screen in teletype mode
;                xor    bx, bx
;                push   si
;                int    10h
;int10_exit:
;                pop    si
;                jmp    short disp1
;disp2:
;                ret
;outstr endp


;
; read_run:
; Reads a contiguous run of sectors.
;
;          Input:
;          eax    -- logical sector number to read from
;          cx     -- sector count to read 1 <= cx <= 128
;          es:bx  -- address to read to
;          dl     -- drive number
;

read_run proc near
                pusha
                push  es
                ;push  ds
                add   eax, [bp].bpb.hidden_secs             ; Add hidden sectors value
begin_read:
                mov   dl,  [bp].drive
                ;push  eax

                pushad
;;;;!!!
                cmp   [bp].force_chs, 0                     ; LBA or CHS?
                jnz   short chs

lba:
                call  readsec_lba                           ; Read by LBA
                jmp   short chk

chs:
                call  readsec_chs                           ; Read by CHS
chk:
                popad

                jc   short err_read                         ; Signal a read error
                ;jnc   go_on
                ;mov   ax, es
                ;call  printhex4

                ;mov   ax, bx
                ;call  printhex4

                ;pop   eax
                ;call  printhex8

                ;cli
                ;hlt
go_on:
                ;pop   eax

                inc   eax
                ;add   bx, 200h

                mov  di, es
                add  di, 20h
                mov  es, di

                loop  short begin_read
end_read:
                ;pop  ds
                pop  es
                popa

                ret
read_run endp

;
; readsec_lba:
;
;               Reads a sector
;               using LBA,
;               to es:bx
;               from LBA address in eax
;               from disk device at dl
;


readsec_lba proc near
                ;
                ; int 13h 42h function
                ; Input:
                ;
                ;            ah = 42h
                ;            dl = drive number
                ;            ds:si = pointer to disk address packet
                ;
                ; Returns:
                ;
                ;            al = 0 if success,
                ;            error code otherwise
                ;push ds

                xor  ecx, ecx
                mov  ds, cx
                ;push  ss
                ;pop   ds

                lea  si, [bp].disk_addr_pkt
                mov  dword ptr [si].starting_block, eax     ; LBA of the 1st
                mov  dword ptr [si].starting_block + 4, ecx ; clear high dword
                mov  dword ptr [si], 00010010h              ; 0x10 - packet size, 0x00 -- reserved, 0x0001 -- sector count
                ;mov  word  ptr [si].pkt_size, 10h           ; size of packet absolute starting address
                ;mov  word ptr  [si].num_blocks, 1           ; number of blocks to transfer

                mov  word  ptr [si].buffer, bx              ; offset of read buffer
                mov  word  ptr [si].buffer + 2, es          ; segment
                ;mov  dword ptr [si].buffer + 4, ecx         ; clear high dword

                mov  ah, 42h
                int  13h

                ret

readsec_lba endp


;
; readsec_chs:
;
;               Reads a sector
;               using CHS,
;               to es:bx
;               from LBA address in eax
;               from disk device in dl
;



readsec_chs proc near
                push dx

                push eax

                mov  ax, [bp].bpb.heads_cnt                 ; headsCnt * trackSize
                mul  byte ptr [bp].bpb.track_size           ; --> ax (cyl size)
                mov  di, ax                                 ; di = cylinder size

                pop  ax                                     ;
                pop  dx                                     ;
                                                            ; divide LSN in dx:ax by cyl size in di
                div  di                                     ; now dx holds remainder
                                                            ; and ax holds quotient
                mov  cx, ax                                 ; cx = Cylinder no
                mov  ax, dx                                 ;
                div  byte ptr [bp].bpb.track_size           ; now al holds Head no
                                                            ; and ah holds sector no
                inc  ah                                     ; sectors in CHS are numbered from 1, not 0 !!!

                pop  dx

                mov  dh, al                                 ; head no

                xchg cl, ch                                 ;

                or   cl, ah                                 ;
                mov  ax, 0201h                              ; ah = 2 -- function and al = 1 -- count of sectors
                int  13h

                ret
readsec_chs endp

padsize      equ    512 - ($ - start) - 2 - (bootsig - vars)

if padsize gt 0
padding      db     padsize  dup (0)
endif

;if padsize lt 0
;  %out             'Bootsector does not fit in 512 bytes!'
;  %out             'Please clean up code.'
;  .err
;endif

; Various segments
bt_seg         equ   07c0h                                  ; bootsector
root_dir_seg   equ   1000h                                  ; root directory
scr_seg        equ   1FE0h                                  ; scratch area for pathnames
ldr_seg        equ   2000h                                  ; loader
fat_seg        equ   4000h                                  ; FAT

bt_addr        equ   bt_seg * 16

vars           label byte

cfg_path       db    'BOOT    ','   '                       ; path to the
               db    'BOOTSEC ','CFG'                       ; bootsector config file
               db    ';'                                    ; path end marker
bootsig        dw    0aa55h                                 ; boot signature

boot ends

     end start
