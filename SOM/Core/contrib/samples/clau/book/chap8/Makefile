.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj .def .c
#
# Need /Ge- to build DLL
#
all: browse.exe som.ir

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

browse.exe: browse.obj
        icc /Fe"browse.exe" browse.obj somtk.lib

som.ir: test.idl
        sc -sir -u test.idl
