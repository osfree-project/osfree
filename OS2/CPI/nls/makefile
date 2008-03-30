#
# A Makefile for nls.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = nls
DESC     = National Language Support API
OBJS     = ..\shared\stub.obj
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk import
 @%append $^&.lnk   DOSCASEMAP DOSCALLS.391,
 @%append $^&.lnk   DOSGETCOLLATE DOSCALLS.392,
 @%append $^&.lnk   DOSGETCTRYINFO DOSCALLS.393,
 @%append $^&.lnk   DOSGETDBCSEV DOSCALLS.394,
 @%append $^&.lnk   DosQueryCtryInfo DOSCALLS.395,
 @%append $^&.lnk   DosQueryDBCSEnv DOSCALLS.396,
 @%append $^&.lnk   DosMapCase DOSCALLS.397,
 @%append $^&.lnk   DosQueryCollate DOSCALLS.398
 @%append $^&.lnk export
 @%append $^&.lnk   DOSCASEMAP.1,
 @%append $^&.lnk   DOSGETCOLLATE.2,
 @%append $^&.lnk   DOSGETCTRYINFO.3,
 @%append $^&.lnk   DOSGETDBCSEV.4,
 @%append $^&.lnk   DosQueryCtryInfo.5,
 @%append $^&.lnk   DosQueryDBCSEnv.6,
 @%append $^&.lnk   DosMapCase.7,
 @%append $^&.lnk   DosQueryCollate.8
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).dll $(FILESDIR)$(SEP)os2$(SEP)dll
