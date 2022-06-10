###############################################################################
# Makefile for ACPI toolkit sample programs
###############################################################################

!ifndef %TKROOT
!error TKROOT must be defined to be the root of the toolkit
!endif

!ifndef %WATCOM
!error WATCOM must be defined to be the root of the Watcom Compiler
!endif

TKROOT=$(%TKROOT)
WATCOM=$(%WATCOM)
LIBPATH=$(%LIB)

TARGET=ButtonJin
OBJS+= $(TARGET).obj
LIBS+= acpi32.lib

###############################################################################
# Common stuff to build a EXE
###############################################################################

CFLAGS = -bm -bt=OS2 -5 -fpi -5s -zq -zp4
!ifdef DEBUG
CFLAGS+= -d2
!endif

.before
  @set INCLUDE=.;$(%INCLUDE)$(WATCOM)\H;$(WATCOM)\H\OS2;$(TKROOT)\h;$(TKROOT)\h\platform;

.extensions:
.extensions: .exe .obj .c .lnk

all: $(TARGET).exe

.c.obj : .AUTODEPEND
    wcc386 $[* $(CFLAGS)

$(TARGET).exe: $(OBJS) .AUTODEPEND
	@%create $^*.lrf
	@%append $^*.lrf system os2v2_pm
	@%append $^*.lrf option quiet
	@%append $^*.lrf option map
    @%append $^*.lrf option DESCRIPTION 'ACPI Toolkit code'
	@%append $^*.lrf option OSNAME='OS/2'
	@%append $^*.lrf option ELIMINATE
	@%append $^*.lrf option MANYAUTODATA
	@%append $^*.lrf option SHOWDEAD
	@%append $^*.lrf libpath $(LIBPATH);
	@%append $^*.lrf name $^.
	@for %f in ($(OBJS)) do @%append $^*.lrf file %f
	@for %f in ($(LIBS)) do @%append $^*.lrf library %f
	wlink @$^*.lrf
    rc -n $^*.rc $^@
	@%erase $^*.lrf

.obj.exe : .AUTODEPEND
    wlink @$(linkfilename).lnk

clean : .SYMBOLIC
    @if exist $(TARGET).exe del $(TARGET).exe
    @if exist *.obj del *.obj
    @if exist *.map del *.map
    @if exist *.res del *.res
    @if exist *.lst del *.lst

