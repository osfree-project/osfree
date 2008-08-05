#
# A Makefile for pmdrag.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = pmmle
DESC     = Multi-Line Editor Control
OBJS     = ..\shared\stub.obj
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initinstance terminstance
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk import
 @%append $^&.lnk   MLE_INITIALIZE                   PMCTLS.31,
 @%append $^&.lnk   MULTILINEEDITPROC                PMCTLS.32,
 @%append $^&.lnk   BDMULTILINEEDITPROC              PMCTLS.33
 @%append $^&.lnk export
 @%append $^&.lnk   MLE_INITIALIZE.                  00001,
 @%append $^&.lnk   MULTILINEEDITPROC.               00002,
 @%append $^&.lnk   BDMULTILINEEDITPROC.             00003
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 @$(MAKE) $(MAKEOPT) install

install: .SYMBOLIC
 $(CP) $(PROJ).dll ..$(SEP)bin

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
