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


_TEXT16 segment dword public 'CODE' use16

;
; int biosdisk_int13_extensions (int ax, int drive, void *dap);
;

biosdisk_int13_extensions_rm proc far
    retf
biosdisk_int13_extensions_rm endp

;
; int biosdisk_standard (int ah, int drive,
;                       int coff, int hoff, int soff,
;                       int nsec, int segment);
;

biosdisk_standard_rm proc far
    retf
biosdisk_standard_rm endp

;
; int check_int13_extensions (int drive);
;


check_int13_extensions_rm proc far
    retf
check_int13_extensions_rm endp

;
; int get_diskinfo_standard (int drive,
;                            unsigned long *cylinders,
;                            unsigned long *heads,
;                            unsigned long *sectors);
;

get_diskinfo_standard_rm proc far
    retf
get_diskinfo_standard_rm endp


_text16_end:

_TEXT16 ends


_TEXT segment dword public 'CODE'  use32

;
; int biosdisk_int13_extensions (int ax, int drive, void *dap);
;

biosdisk_int13_extensions proc near
    ret
biosdisk_int13_extensions endp

;
; int biosdisk_standard (int ah, int drive,
;                       int coff, int hoff, int soff,
;                       int nsec, int segment);
;

biosdisk_standard proc near
    ret
biosdisk_standard endp

;
; int check_int13_extensions (int drive);
;


check_int13_extensions proc near
    ret
check_int13_extensions endp

;
; int get_diskinfo_standard (int drive,
;                            unsigned long *cylinders,
;                            unsigned long *heads,
;                            unsigned long *sectors);
;

get_diskinfo_standard proc near
    ret
get_diskinfo_standard endp

_TEXT ends

        end
