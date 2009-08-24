;
; loader.inc
; Interface between different parts of loader
;


; boot flags
BOOTFLAG_NOVOLIO   equ  0100h
BOOTFLAG_RIPL      equ  0200h
BOOTFLAG_MINIFSD   equ  0400h
BOOTFLAG_RESERVED3 equ  0800h
BOOTFLAG_MICROFSD  equ  1000h
BOOTFLAG_RESERVED5 equ  2000h
BOOTFLAG_RESERVED6 equ  4000h
BOOTFLAG_RESERVED7 equ  8000h

FileTable struc

    ; FileTable struc
    ft_cfiles              dw   ?
    ft_ldrseg              dw   ?
    ft_ldrlen              dd   ?
    ft_museg               dw   ?
    ft_mulen               dd   ?
    ft_mfsseg              dw   ?
    ft_mfslen              dd   ?
    ft_ripseg              dw   ?
    ft_riplen              dd   ?

    ; microfsd functions
    ft_muOpen              dd   ?
    ft_muRead              dd   ?
    ft_muClose             dd   ?
    ft_muTerminate         dd   ?

FileTable ends


FileTableExt struc

    ; FileTable struc
    ft_cfiles              dw   ?
    ft_ldrseg              dw   ?
    ft_ldrlen              dd   ?
    ft_museg               dw   ?
    ft_mulen               dd   ?
    ft_mfsseg              dw   ?
    ft_mfslen              dd   ?
    ft_ripseg              dw   ?
    ft_riplen              dd   ?

    ; microfsd functions
    ft_muOpen              dd   ?
    ft_muRead              dd   ?
    ft_muClose             dd   ?
    ft_muTerminate         dd   ?

    ; Extended fields

    ft_muMount             dd   ?

    ; stage0 functions
    ft_pbpb                dd   ?
    ft_stage0_seg          dw   ?
    ft_stage0_stack_top    dw   ?
    ft_stage0_stack        dw   ?
    ft_bios_device         db   ?
    ft_printk              dd   ?
    ft_microfsd_panic      dd   ?
    ft_DecoupePath         dd   ?
    ft_bios_read_hd        dd   ?

    ; microxfd functions

FileTableExt ends
