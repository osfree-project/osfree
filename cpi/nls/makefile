# $Id: makefile,v 1.3 2004/08/16 04:51:21 prokushev Exp $

ROOT=..\..\..

DIR = $(ROOT)\bin\os2\dll
PROJ = nls
DESC = National Language Support API
OBJS = ..\shared\stub.obj
ADD_COPT = -i=..\include

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).dll $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(DIR)\$(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    map
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk LIBRARY $(LINKLIB)
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
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)

# Clean all

clean: .symbolic
 -@del *.obj $(LOG)
 -@del *.sbr $(LOG)
 -@del *.map $(LOG)
 -@del *.err $(LOG)
 -@del *.wmp $(LOG)
 -@del *.lnk $(LOG)
 -@del *.sym $(LOG)
