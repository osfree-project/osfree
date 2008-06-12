#
# A Makefile for MiniCMD
# (c) osFree project,
# prokushev, 2008
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

PROJ = makeini
DIR = $(ROOT)$(SEP)bin$(SEP)os2
DESC = INI file compiler
#defines object file names in format objname.$(O)
OBJS = xmakeini.$(O) linklist.$(O) stringh.$(O) xprf.$(O) xstring.$(O) &
       nls.$(O) dosh.$(O) prfh.$(O) dosh2.$(O) level.$(O)

!include $(%ROOT)/mk/all.mk


all: $(PROJ).exe .SYMBOLIC

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk import Dos16GetInfoSeg DOSCALLS.8
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 @$(MAKE) $(MAKEOPT) install

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)os2
