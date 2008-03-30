#
# A Makefile for utlapi.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
PROJ     = utlapi
DESC     = osFree Utilities API
OBJS     = librectangles.$(O)
ADD_COPT = -i=..$(SEP)..$(SEP)include$(SEP)os3
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk SYSTEM os2v2 dll initglobal
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk export
 @%append $^&.lnk   RectCopy.1,
 @%append $^&.lnk   RectSet.2,
 @%append $^&.lnk   RectIsEmpty.3,
 @%append $^&.lnk   RectEqual.4,
 @%append $^&.lnk   RectSetEmpty.5,
 @%append $^&.lnk   RectOffset.6,
 @%append $^&.lnk   RectInflate.7,
 @%append $^&.lnk   RectPtIn.8,
 @%append $^&.lnk   RectIntersect.9,
 @%append $^&.lnk   RectUnion.10,
 @%append $^&.lnk   RectSubtract.11,
 @%append $^&.lnk   RectMakeRect.12,
 @%append $^&.lnk   RectMakePoints.13
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
