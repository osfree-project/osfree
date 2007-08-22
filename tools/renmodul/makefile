#
# A Makefile for qemu-img
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

# For Watcom C/C++ 10.6 and higher
# Use wmake DEBUG=1 to get debug info

# debug all: link debug info in
# remaining options set in helpmgr.def

!ifdef DEBUG

MODE_LINKOPT=debug all

# -b2=os2: compile for os/2
# -s:  remove stack checks (crashes)
# -wx: all warnings
# -zq: quiet

# -od: Disable optimisation
# -d2: full debug info
# -hc: codeview debug format
MODE_COPT=-od -d2 -hc -dDEBUG

!else

# -os:  Optimise for size
MODE_COPT=-os

!endif

ADD_COPT=-bt=os2 -s -wx -zq $(MODE_COPT)

32_BITS  = 1
ADD_COPT = -dQEMU_TOOL -d__WATCOM__ -d__OS2__ -i=. -i=..$(SEP)include

DIR = $(%ROOT)$(SEP)tools$(SEP)bin
PROJ = renmodul

OBJS1 = renmodul.$(O) main.$(O)
OBJS2 = renmodul.$(O) dllmain.$(O)
OBJS  = renmodul.$(O) main.$(O) dllmain.$(O)

!include $(%ROOT)/mk/all.mk

CLEANMASK = *.lnk *.wmp *.obj *.err *.log *.bak *.sym *.map *.exe *.dll

all: $(PROJ).exe $(PROJ).dll $(PROJ).exe.sym $(PROJ).dll.sym .SYMBOLIC
 $(CP) $< $(DIR)

$(PROJ).exe: $(OBJS1)
 @%create $^@.lnk
 @%append $^@.lnk SYSTEM os2v2
 @%append $^@.lnk OPTION MAP=$^@.wmp
 @%append $^@.lnk NAME $^@
 @for %%i in ($(OBJS1)) do @%append $^@.lnk FILE %%i
 @%append $^@.lnk $(MODE_LINKOPT)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^@.lnk $(LOG)

$(PROJ).dll: $(OBJS2)
 @%create $^@.lnk
 @%append $^@.lnk SYSTEM os2v2 dll initinstance terminstance
 @%append $^@.lnk EXPORT RenameModule = RenameModule
 @%append $^@.lnk OPTION MAP=$^@.wmp
 @%append $^@.lnk NAME $^@
 @for %%i in ($(OBJS2)) do @%append $^@.lnk FILE %%i
 @%append $^@.lnk $(MODE_LINKOPT)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^@.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
