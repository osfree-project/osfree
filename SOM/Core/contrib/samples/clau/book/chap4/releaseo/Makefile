.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj

all: main.exe
.cpp.obj:
        icc -I. /Ti+ /c+ $<

.idl.xh:
          sc -sxh $*.idl
.idl.xih:
          sc -sxih $*.idl
.idl.cpp:
          sc -sxc $*.idl

a.obj : a.xh a.xih a.cpp

#
# Do not recompile main if class A change so that we
# can illustrate the point
#
main.obj: main.cpp

main.exe: a.obj main.obj
        icc /Fe"main.exe" /Ti+ a.obj main.obj somtk.lib
