#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = bdcalls
DESC     = bi-di API
OBJS     = ..\shared\stub.obj
ADD_COPT = -bd

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
 @%append $^&.lnk LIBPATH $(%WATCOM)$(SEP)lib386;$(%WATCOM)$(SEP)lib386$(SEP)os2
 @%append $^&.lnk export
 @%append $^&.lnk   NLSCONVERTBIDINUMERICS.0001=NLS16CONVERTBIDINUMERICS,
 @%append $^&.lnk   NLSCONVERTBIDISTRING.0002=NLS16CONVERTBIDISTRING,
 @%append $^&.lnk   NLSINTSETBIDIATT.0003=NLS16INTSETBIDIATT,
 @%append $^&.lnk   NLSINTQUERYBIDIATT.0004=NLS16INTQUERYBIDIATT,
 @%append $^&.lnk   NLSSETBIDIATT.0005=NLS16SETBIDIATT,
 @%append $^&.lnk   NLSQUERYBIDIATT.0006=NLS16QUERYBIDIATT,
 @%append $^&.lnk   NLSINVERSESTRING.0007=NLS16INVERSESTRING,
 @%append $^&.lnk   NLSSETBIDIPRINT.0008=NLS16SETBIDIPRINT,
 @%append $^&.lnk   NLSEDITSHAPE.0009=NLS16EDITSHAPE,
 @%append $^&.lnk   NLSSHAPEBIDISTRING.00010=NLS16SHAPEBIDISTRING,
 @%append $^&.lnk   NLPOPUP.00011=NL16POPUP
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
