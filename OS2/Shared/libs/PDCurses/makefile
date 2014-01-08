# Makefile for common shared libs

PROJ = pdcurses
ADD_COPT = -i=..$(SEP)include
srcfiles = $(p)addch$(e) $(p)addchstr$(e) $(p)addstr$(e) $(p)attr$(e) $(p)beep$(e) $(p)bkgd$(e) &
           $(p)border$(e) $(p)clear$(e) $(p)color$(e) $(p)delch$(e) $(p)deleteln$(e) $(p)deprec$(e) &
           $(p)getch$(e) $(p)getstr$(e) $(p)getyx$(e) $(p)inch$(e) $(p)inchstr$(e) $(p)initscr$(e) $(p)inopts$(e) &
           $(p)insch$(e) $(p)insstr$(e) $(p)instr$(e) $(p)kernel$(e) $(p)keyname$(e) $(p)mouse$(e) $(p)move$(e) &
           $(p)outopts$(e) $(p)overlay$(e) $(p)pad$(e) $(p)panel$(e) $(p)printw$(e) $(p)refresh$(e) &
           $(p)scanw$(e) $(p)scr_dump$(e) $(p)scroll$(e) $(p)slk$(e) $(p)termattr$(e) $(p)terminfo$(e) &
           $(p)touch$(e) $(p)util$(e) $(p)window$(e) $(p)debug$(e) &
           $(p)pdcclip$(e) $(p)pdcdisp$(e) $(p)pdcgetsc$(e) $(p)pdckbd$(e) $(p)pdcscrn$(e) &
           $(p)pdcsetsc$(e) $(p)pdcutil$(e)
UNI2H = 1

!include $(%ROOT)/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
