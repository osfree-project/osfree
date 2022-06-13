.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj

#
# Need /Ge- to build DLL
#

all: domestic.lib dynaload.exe

.cpp.obj:
        icc /c+ /Ge- -I. $<
.idl.xh:
         sc -sxh  $*.idl
.idl.xih:
         sc -sxih  $*.idl
.idl.cpp:
         sc -sxc $*.idl
.idl.def:
         sc -sdef $*.idl

initfunc.obj: initfunc.cpp

cat.obj : cat.cpp cat.xh cat.xih cat.idl cat.def
dog.obj : dog.cpp dog.xh dog.xih dog.idl dog.def

dynaload.obj:
        icc /c+ dynaload.cpp

dynaload.exe: dynaload.obj
        icc /Fe"dynaload.exe" dynaload.obj somtk.lib

domestic.lib: cat.obj dog.obj initfunc.obj
        icc @<<
        /Fe"domestic.dll" cat.obj dog.obj initfunc.obj domestic.def
        os2386.lib somtk.lib
<<
        implib domestic.lib domestic.def
