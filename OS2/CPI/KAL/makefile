#
# A Makefile for kal.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = kal
DESC     = Kernel Abstracion Layer API
OBJS     = ..\shared\stub.$(O) file.$(O)
ADD_COPT = -i=..\include
LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk SYSTEM os2v2 dll initglobal
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk export
 @%append $^&.lnk   DosClose.1,
 @%append $^&.lnk   DosOpen.2,
 @%append $^&.lnk   DosRead.3,
 @%append $^&.lnk   DosWrite.4,
 @%append $^&.lnk   DosFindClose.5,
 @%append $^&.lnk   DosFindFirst.6,
 @%append $^&.lnk   DosFindNext.7
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

install: .SYMBOLIC
 $(CP) $(PROJ).exe ..$(SEP)bin

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
