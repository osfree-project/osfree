; $Id: xms_brk.asm 364 2002-04-02 18:13:06Z skaus $
;	Criter and ^Break handler for external programs for XMS Swap
;	variant

TEXT segment word public 'CODE' use16

	public lowlevel_cbreak_handler_
lowlevel_cbreak_handler_:

include dmy_cbrk.asm

TEXT ends

      end
