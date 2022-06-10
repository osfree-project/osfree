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

###############################################################################
# Common stuff to build a EXE
###############################################################################

CFLAGS = -bm -bt=OS2 -6s -fpi -zq -zp4 -DACPI_APPLICATION
!ifdef DEBUG
CFLAGS+= -d2
!endif

.before
  @set INCLUDE=.;$(%INCLUDE)$(WATCOM)\H;$(WATCOM)\H\OS2;$(TKROOT)\h;$(TKROOT)\h\platform;

.extensions:
.extensions: .exe .obj .c .lnk

all: AcpiIRQ.exe EmbeddedRW.exe GetNotify.exe GetTable.exe PowerOff.exe SetGetState.exe &
    SetGetThrtl.exe acpibattery.exe acpitree.exe evaluate.exe irqstat.exe thermal.exe &
    GetValue.exe UpTime.exe GPEDevice.exe WaitButton.exe

.c.obj : .AUTODEPEND
  wcc386 $[* $(CFLAGS)

.obj.exe: .AUTODEPEND
	@%create $^*.lrf
	@%append $^*.lrf system os2v2
	@%append $^*.lrf option quiet
!ifdef DEBUG
	@%append $^*.lrf debug all
	@%append $^*.lrf option map
!endif	
  @%append $^*.lrf option DESCRIPTION 'ACPI Toolkit sample'
	@%append $^*.lrf option OSNAME='OS/2'
	@%append $^*.lrf option ELIMINATE
	@%append $^*.lrf option MANYAUTODATA
	@%append $^*.lrf option SHOWDEAD
	@%append $^*.lrf libpath $(LIBPATH);
	@%append $^*.lrf name $^.
	@%append $^*.lrf file $[.
	@%append $^*.lrf library acpi32.lib
	wlink @$^*.lrf
	@%erase $^*.lrf

clean : .SYMBOLIC
    @if exist *.exe del *.exe
    @if exist *.obj del *.obj
    @if exist *.map del *.map
    @if exist *.res del *.res
    @if exist *.lst del *.lst

