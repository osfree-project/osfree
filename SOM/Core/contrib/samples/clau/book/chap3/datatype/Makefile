.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj

#
# Need /Ge- to build DLL
#
OBJS = employee.obj company.obj seq.obj

all: seq.exe any.exe

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

employee.obj : employee.cpp
employee.cpp : employee.idl employee.xh employee.xih
employee.xh  : employee.idl
employee.xih : employee.idl

company.obj : company.cpp                              
company.cpp : company.idl company.xh company.xih       
company.xh  : company.idl                              
company.xih : company.idl                              

seq.obj: seq.cpp
seq.exe: $(OBJS)
        icc /Fe"seq.exe" $(OBJS) somtk.lib

any.obj: any.cpp
any.exe: any.obj
        icc /Fe"any.exe" any.obj somtk.lib

