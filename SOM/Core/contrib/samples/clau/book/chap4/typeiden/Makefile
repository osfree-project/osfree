.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj

#
# Need /Ge- to build DLL
#
OBJS = empl.obj prg.obj mgr.obj calc.obj
all: calc.exe

.cpp.obj:
        icc /c+ /Ge- -I. $<
.idl.xh:
         sc -sxh  $*.idl
.idl.xih:
         sc -sxih $*.idl
.idl.cpp:
         sc -sxc $*.idl

empl.obj : empl.cpp empl.xh empl.xih 
mgr.obj:   mgr.cpp  mgr.xh  mgr.xih
prg.obj:   prg.cpp prg.xh prg.xih

calc.obj:
        icc /c+ /I. calc.cpp

calc.exe: $(OBJS)
        icc /Fe"calc.exe" $(OBJS) somtk.lib

