#
# A Makefile for pmmerge.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
PROJ     = pmmerge
DESC     = OS/2 PM Merged Library
OBJS     = rectangles.$(O)
ADD_COPT = -i=..\include
LINKOPT  = lib os2386.lib lib clib3r.lib path rectangles
DIRS     = rectangles

!include $(%ROOT)/mk/all.mk

all: subs $(PROJ).dll .SYMBOLIC

subs: .SYMBOLIC
  $(MAKE) $(MAKEOPT) TARGET=all subdirs

$(PROJ).dll: .symbolic
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk SYSTEM os2v2 dll initglobal
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk import
 @%append $^&.lnk   RectCopy utlapi.1,
 @%append $^&.lnk   RectSet utlapi.2,
 @%append $^&.lnk   RectIsEmpty utlapi.3,
 @%append $^&.lnk   RectEqual utlapi.4,
 @%append $^&.lnk   RectSetEmpty utlapi.5,
 @%append $^&.lnk   RectOffset utlapi.6,
 @%append $^&.lnk   RectInflate utlapi.7,
 @%append $^&.lnk   RectPtIn utlapi.8,
 @%append $^&.lnk   RectIntersect utlapi.9,
 @%append $^&.lnk   RectUnion utlapi.10,
 @%append $^&.lnk   RectSubtract utlapi.11,
 @%append $^&.lnk   RectMakeRect utlapi.12,
 @%append $^&.lnk   RectMakePoints utlapi.13
 @%append $^&.lnk export
 @%append $^&.lnk   WinCopyRect.05127,
 @%append $^&.lnk   WinEqualRect.05173,
 @%append $^&.lnk   WinInflateRect.05200,
 @%append $^&.lnk   WinIntersectRect.05203,
 @%append $^&.lnk   WinIsRectEmpty.05209,
 @%append $^&.lnk   WinMakePoints.05229,
 @%append $^&.lnk   WinMakeRect.05230,
 @%append $^&.lnk   WinOffsetRect.05239,
 @%append $^&.lnk   WinPtInRect.05248,
 @%append $^&.lnk   WinSetRect.05364,
 @%append $^&.lnk   WinSetRectEmpty.05365,
 @%append $^&.lnk   WinSubtractRect.05391,
 @%append $^&.lnk   WinUnionRect.05402
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

install: .SYMBOLIC
 $(CP) $(PROJ).exe ..$(SEP)bin

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
