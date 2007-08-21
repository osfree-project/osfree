# For Watcom C/C++ 10.6 and higher
# Use wmake DEBUG=1 to get debug info

# debug all: link debug info in
# remaining options set in helpmgr.def
!ifdef DEBUG
MODE_LINKFLAGS=debug all
!endif

all: renmodul.exe renmodul.dll

renmodul.exe:  renmodul.obj main.obj
  wlink $(MODE_LINKFLAGS) file renmodul.obj file main.obj system os2v2 name renmodul.exe

renmodul.dll:  renmodul.obj renmodul.def dllmain.obj
  wlink $(MODE_LINKFLAGS) file renmodul.obj file dllmain.obj @renmodul.def

# Compile step: .c -> .obj

# /b2=os2: compile for os/2
# /s:  remove stack checks (crashes)
# /wx: all warnings
# /zq: quiet

!ifdef DEBUG
# /od: Disable optimisation
# /d2: full debug info
# /hc: codeview debug format
MODE_CFLAGS=/od /d2 /hc /dDEBUG

!else
# /os:  Optimise for size
MODE_CFLAGS=/os

!endif

CFLAGS=/bt=os2 /s /wx /zq $(MODE_CFLAGS)

# Makefile rule for compiling C files
.c.obj:
  wcc386 $(CFLAGS) $*.c

renmodul.obj: renmodul.c renmodul.h

main.obj: main.c renmodul.h

dllmain.obj: dllmain.c
