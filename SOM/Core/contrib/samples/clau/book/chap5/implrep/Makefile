.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj .def 
#
# Need /Ge- to build DLL
#
all: impdef.exe

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

impdef.obj: impdef.cpp
impdef.exe: impdef.obj
        icc /Fe"impdef.exe" impdef.obj somtk.lib
