.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj

#
# Need /Ge- to build DLL
#
OBJS = error.obj testerr.obj

all: testerr.exe

.cpp.obj:
        icc /c+ -I. /O+ $<

.idl.xh:
         sc -sxh  $*.idl
.idl.xih:
         sc -sxih  $*.idl
.idl.cpp:
         sc -sxc $*.idl
.idl.def:
         sc -sdef $*.idl

error.obj : error.cpp
error.cpp : error.idl error.xh error.xih
error.xh  : error.idl
error.xih : error.idl

testerr.obj: testerr.cpp
        icc /c+ -I. /O+ testerr.cpp

testerr.exe: $(OBJS)
        icc /Fe"testerr.exe" $(OBJS) somtk.lib

