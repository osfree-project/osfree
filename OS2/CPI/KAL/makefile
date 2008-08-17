#
# A Makefile for kal.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)$(SEP)bin$(SEP)os2$(SEP)dll
PROJ     = kal
DESC     = Kernel Abstracion Layer API
OBJS     = ..$(SEP)shared$(SEP)stub.$(O) file.$(O)
ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk SYSTEM os2v2 dll initglobal
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk export
 @%append $^&.lnk   DosFSCtl.1,  # all FS related API works via this API
 @%append $^&.lnk   DosPutMessage2.8,
 @%append $^&.lnk   DosQuerySysInfo.9,
 @%append $^&.lnk   DosExit.10,
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
 $(CP) $(PROJ).dll $(FILESDIR)$(SEP)os2$(SEP)dll
