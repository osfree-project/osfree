.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj

#
# Need /Ge- to build DLL
#
OBJS = animal.obj fly.obj robin.obj meta.obj
all: meta.exe

.cpp.obj:
        icc /c+ -I. $<
.idl.xh:
         sc -sxh  $*.idl
.idl.xih:
         sc -sxih  $*.idl
.idl.cpp:
         sc -sxc $*.idl
.idl.def:
         sc -sdef $*.idl

animal.cpp : animal.idl animal.xh animal.xih
animal.obj : animal.cpp     
animal.xh  : animal.idl
animal.xih : animal.idl

fly.obj : fly.cpp
fly.cpp : fly.idl fly.xh fly.xih
fly.xh  : fly.idl
fly.xih : fly.idl

robin.obj : robin.cpp
robin.cpp : robin.idl robin.xh robin.xih
robin.xh  : robin.idl
robin.xih : robin.idl

meta.obj: meta.cpp

meta.exe: $(OBJS)
        icc /Fe"meta.exe" $(OBJS) somtk.lib
