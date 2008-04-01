;
; mb_header.inc
; multiboot header
;


multiboot_header struc

  ;  Must be MULTIBOOT_MAGIC - see below.
  magic         dd ?
  
  ;  Feature flags - see below. 
  flags         dd ?
  
  ;
  ;  Checksum
  ; 
  ;  The above fields plus this one must equal 0 mod 2^32.
  ;  
  checksum      dd ?
  
  ;  These are only valid if MULTIBOOT_AOUT_KLUDGE is set.
  header_addr   dd ?
  load_addr     dd ?
  load_end_addr dd ?
  bss_end_addr  dd ?
  entry_addr    dd ?

  ;  These are only valid if MULTIBOOT_VIDEO_MODE is set.
  vmode_type     dd ?
  vwidth         dd ?
  vheight        dd ?
  vdepth         dd ?

multiboot_header ends

  ; 
  ;  The entire multiboot_header must be contained
  ;  within the first MULTIBOOT_SEARCH bytes of the kernel image.
  ;  
MULTIBOOT_SEARCH	equ	8192

  ;  Magic value identifying the multiboot_header.
MULTIBOOT_MAGIC		equ	0x1BADB002

  ;
  ;  Features flags for 'flags'.
  ;  If a boot loader sees a flag in MULTIBOOT_MUSTKNOW set
  ;  and it doesn't understand it, it must fail.
  ; 
MULTIBOOT_MUSTKNOW	equ	0x0000FFFF

  ;  currently unsupported flags...  this is a kind of version number.
MULTIBOOT_UNSUPPORTED	equ	0x0000FFF8

  ;  Align all boot modules on i386 page (4KB) boundaries.
MULTIBOOT_PAGE_ALIGN	equ	0x00000001

  ;  Must pass memory information to OS.
MULTIBOOT_MEMORY_INFO	equ	0x00000002

  ;  Must pass video information to OS.
MULTIBOOT_VIDEO_MODE	equ	0x00000004

  ;  This flag indicates the use of the address fields in the header.
MULTIBOOT_AOUT_KLUDGE	equ	0x00010000
