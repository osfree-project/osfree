# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..$(SEP)include

!include $(%ROOT)/mk/all.mk


all: $(ROOT)$(SEP)lib$(SEP)pdcurses.lib .SYMBOLIC

OBJS = addch.$(O) addchstr.$(O) addstr.$(O) attr.$(O) beep.$(O) bkgd.$(O) &
border.$(O) clear.$(O) color.$(O) delch.$(O) deleteln.$(O) deprec.$(O) &
getch.$(O) getstr.$(O) getyx.$(O) inch.$(O) inchstr.$(O) initscr.$(O) inopts.$(O) &
insch.$(O) insstr.$(O) instr.$(O) kernel.$(O) keyname.$(O) mouse.$(O) move.$(O) &
outopts.$(O) overlay.$(O) pad.$(O) panel.$(O) printw.$(O) refresh.$(O) &
scanw.$(O) scr_dump.$(O) scroll.$(O) slk.$(O) termattr.$(O) terminfo.$(O) &
touch.$(O) util.$(O) window.$(O) debug.$(O) &
pdcclip.$(O) pdcdisp.$(O) pdcgetsc.$(O) pdckbd.$(O) pdcscrn.$(O) &
pdcsetsc.$(O) pdcutil.$(O)


$(ROOT)$(SEP)lib$(SEP)pdcurses.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 $(LIB) $(LIBOPT) $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
