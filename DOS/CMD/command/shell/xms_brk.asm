; $Id: xms_brk.asm 364 2002-04-02 18:13:06Z skaus $
;	Criter and ^Break handler for external programs for XMS Swap
;	variant

segment _TEXT class=CODE

	global _lowlevel_cbreak_handler
_lowlevel_cbreak_handler:
%include "../criter/dmy_cbrk.asm"
