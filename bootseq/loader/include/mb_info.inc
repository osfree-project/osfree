;
; mb_info.inc
; multiboot info
;


  ;
  ;  The following value must be present in the EAX register.
  ;

MULTIBOOT_VALID         equ     0x2BADB002

mod_list struc
  ; the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive
  mod_start  dd ?
  mod_end    dd ?

  ; Module command line
  cmdline    dd ?

  ; padding to take it to 16 bytes (must be zero)
  pad        dd ?
mod_list ends

multiboot_info struc
  ; MultiBoot info version number
  flags       dd  ?

  ; Available memory from BIOS
  mem_lower   dd  ?
  mem_upper   dd  ?

  ; "root" partition
  boot_device dd  ?

  ; Kernel command line
  cmdline     dd  ?

  ; Boot-Module list
  mods_count  dd  ?
  mods_addr   dd  ?

  ; (ELF) Kernel section header table
  num         dd  ?
  size        dd  ?
  _addr       dd  ?
  shndx       dd  ?

  ; Memory Mapping buffer
  mmap_length dd  ?
  mmap_addr   dd  ?

  ; Drive Info buffer
  drives_length dd  ?
  drives_addr dd  ?

  ; ROM configuration table
  config_table dd  ?

  ; Boot Loader Name
  boot_loader_name dd  ?

  ; APM table
  apm_table   dd  ?

  ; Video
  vbe_control_info   dd  ?
  vbe_mode_info      dd  ?
  vbe_mode           dw  ?
  vbe_interface_seg  dw  ?
  vbe_interface_off  dw  ?
  vbe_interface_len  dw  ?
multiboot_info ends
