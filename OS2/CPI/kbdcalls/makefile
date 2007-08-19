#
# A Makefile for kbdcalls.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = kbdcalls
DESC     = Keyboard API
OBJS     = ..\shared\stub.obj
ADD_COPT = -i=..\include
LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk import
 @%append $^&.lnk   KBDSETCUSTXT      DOSCALLS.745,
 @%append $^&.lnk   KBDPROCESSINIT    DOSCALLS.746,
 @%append $^&.lnk   KBDGETCP          DOSCALLS.747,
 @%append $^&.lnk   KBDCHARIN         DOSCALLS.748,
 @%append $^&.lnk   KBDSETCP          DOSCALLS.749,
 @%append $^&.lnk   KBDLOADINSTANCE   DOSCALLS.750,
 @%append $^&.lnk   KBDSYNCH          DOSCALLS.751,
 @%append $^&.lnk   KBDREGISTER       DOSCALLS.752,
 @%append $^&.lnk   KBDSTRINGIN       DOSCALLS.753,
 @%append $^&.lnk   KBDGETSTATUS      DOSCALLS.754,
 @%append $^&.lnk   KBDSETSTATUS      DOSCALLS.755,
 @%append $^&.lnk   KBDGETFOCUS       DOSCALLS.756,
 @%append $^&.lnk   KBDFLUSHBUFFER    DOSCALLS.757,
 @%append $^&.lnk   KBDXLATE          DOSCALLS.758,
 @%append $^&.lnk   KBDSWITCHFGND     DOSCALLS.759,
 @%append $^&.lnk   KBDSHELLINIT      DOSCALLS.760,
 @%append $^&.lnk   KBDCLOSE          DOSCALLS.761,
 @%append $^&.lnk   KBDFREEFOCUS      DOSCALLS.762,
 @%append $^&.lnk   KBDFREE           DOSCALLS.763,
 @%append $^&.lnk   KBDDEREGISTER     DOSCALLS.764,
 @%append $^&.lnk   KBDSETFGND        DOSCALLS.765,
 @%append $^&.lnk   KBDPEEK           DOSCALLS.766,
 @%append $^&.lnk   KBDOPEN           DOSCALLS.767,
 @%append $^&.lnk   KBDGETHWID        DOSCALLS.768,
 @%append $^&.lnk   KBDSETHWID        DOSCALLS.769
 @%append $^&.lnk export
 @%append $^&.lnk   KBDSETCUSTXT.1,
 @%append $^&.lnk   KBDPROCESSINIT.2,
 @%append $^&.lnk   KBDGETCP.3,
 @%append $^&.lnk   KBDCHARIN.4,
 @%append $^&.lnk   KBDSETCP.5,
 @%append $^&.lnk   KBDLOADINSTANCE.6,
 @%append $^&.lnk   KBDSYNCH.7,
 @%append $^&.lnk   KBDREGISTER.8,
 @%append $^&.lnk   KBDSTRINGIN.9,
 @%append $^&.lnk   KBDGETSTATUS.10,
 @%append $^&.lnk   KBDSETSTATUS.11,
 @%append $^&.lnk   KBDGETFOCUS.12,
 @%append $^&.lnk   KBDFLUSHBUFFER.13,
 @%append $^&.lnk   KBDXLATE.14,
 @%append $^&.lnk   KBDSWITCHFGND.15,
 @%append $^&.lnk   KBDSHELLINIT.16,
 @%append $^&.lnk   KBDCLOSE.17,
 @%append $^&.lnk   KBDFREEFOCUS.18,
 @%append $^&.lnk   KBDFREE.19,
 @%append $^&.lnk   KBDDEREGISTER.20,
 @%append $^&.lnk   KBDSETFGND.21,
 @%append $^&.lnk   KBDPEEK.22,
 @%append $^&.lnk   KBDOPEN.23,
 @%append $^&.lnk   KBDGETHWID.24,
 @%append $^&.lnk   KBDSETHWID.25
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

install: .SYMBOLIC
 $(CP) $(PROJ).exe ..$(SEP)bin

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
