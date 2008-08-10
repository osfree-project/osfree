#
# A Makefile for renmodul
# (c) osFree project,
# valerius, 2006/10/30
#

PROJ = renmodul
DESC = Rename module

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

srcfiles = $(p)renmodul$(e) $(p)main$(e)

#OBJS1 = renmodul.$(O) main.$(O)
#OBJS2 = renmodul.$(O) dllmain.$(O)
#OBJS  = renmodul.$(O) main.$(O) dllmain.$(O)

!include $(%ROOT)/mk/tools.mk

TARGETS = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).dll
ADD_COPT = -bt=os2 -s -wx -zq $(MODE_COPT)
