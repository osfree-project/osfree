.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj

#
# Need /Ge- to build DLL
#
OBJS = car.obj leftcar.obj rightcar.obj resolve.obj

all: resolve.exe

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

car.obj : car.cpp
car.cpp : car.idl car.xh car.xih
car.xh  : car.idl
car.xih : car.idl

leftcar.obj : leftcar.cpp
leftcar.cpp : leftcar.idl leftcar.xh leftcar.xih
leftcar.xh  : leftcar.idl
leftcar.xih : leftcar.idl

rightcar.obj : rightcar.cpp
rightcar.cpp : rightcar.idl rightcar.xh rightcar.xih
rightcar.xh  : rightcar.idl
rightcar.xih : rightcar.idl

resolve.obj: resolve.cpp

resolve.exe: $(OBJS)
        icc /Fe"resolve.exe" $(OBJS) somtk.lib
