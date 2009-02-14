#
# A makefile for rxmath.dll
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS = 1
DEBUG = 0

!ifeq DEBUG 1
OPT   = -d3 -of -od
!else
OPT   = -d0 -oteanx
!endif

CLEANMASK = *.obj *.map *.err *.wmp *.lnk *.sym
DIR = $(FILESDIR)$(SEP)os2$(SEP)dll
PROJ1 = rexxmath
PROJ2 = rxmath
DESC = REXX math API library
ADD_COPT = -i=. -bd -bm -wx -s -mf $(OPT) -6s -fp6 -bd

OBJS1 = rexxmath.$(O)
OBJS2 = rxmath.$(O)

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ1).dll &
     $(DIR)$(SEP)$(PROJ2).dll &
     install # $(DIR)$(SEP)$(PROJ2).sym $(DIR)$(SEP)$(PROJ1).sym

$(DIR)$(SEP)$(PROJ1).dll: $(OBJS1)
 @$(MAKE) $(MAKEOPT) T=$^& OBJS=$(OBJS1) PROJ=$(PROJ1) link

$(DIR)$(SEP)$(PROJ2).dll: $(OBJS2)
 @$(MAKE) $(MAKEOPT) T=$^& OBJS=$(OBJS2) PROJ=$(PROJ2) link

link: .SYMBOLIC .PROCEDURE
 @%create $(T).lnk
 @%append $(T).lnk name      $(DIR)$(SEP)$(PROJ).dll
 @%append $(T).lnk system    os2v2 dll initinstance terminstance
 @%append $(T).lnk option    ManyAutoData
 @%append $(T).lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $(T).lnk OPTION MAP=$(T).wmp
 @%append $(T).lnk LIBRARY rexx.lib
 @%append $(T).lnk export
 @%append $(T).lnk    MATHLOADFUNCS = mathloadfuncs
 @for %i in ($(OBJS)) do @%append $(T).lnk FILE %i
 $(SAY) Linking $(T).dll
 @$(LINKER) $(LINKOPT) @$(T).lnk $(LOG)

$(DIR)$(SEP)$(PROJ1).sym: $(PROJ1).sym
 $(CP) $(PROJ1).sym $(DIR)

$(DIR)$(SEP)$(PROJ2).sym: $(PROJ2).sym
 $(CP) $(PROJ2).sym $(DIR)

$(PROJ1).sym: $(PROJ1).map

$(PROJ2).sym: $(PROJ2).map

$(PROJ1).map: $(PROJ1).wmp

$(PROJ2).map: $(PROJ2).wmp

# Clean all
clean: .symbolic
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) rxxmath.cmd $(FILESDIR)$(SEP)os2
 $(CP) rxxmathi.cmd $(FILESDIR)$(SEP)os2
