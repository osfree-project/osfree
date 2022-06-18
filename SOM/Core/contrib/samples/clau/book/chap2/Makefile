.SUFFIXES:
.SUFFIXES: .idl .xih .xh .obj .cpp .h .c 

all: chello.exe cpphello.exe

.cpp.obj:
        icc -I. /c+ $<
.c.obj:
        icc -I. /c+ $<

.idl.xh:
          sc -sxh $*.idl
.idl.xih:
          sc -sxih $*.idl
.idl.cpp:
          sc -sxc $*.idl
.idl.h:
          sc -sh $*.idl

# Link cHello
cHello.exe: hello.obj cMain.obj
        icc /Fe"cHello.exe" hello.obj cMain.obj somtk.lib

# Link cppHello
cppHello.exe: hello.obj cppMain.obj
        icc /Fe"cppHello.exe" hello.obj cppMain.obj somtk.lib

# Compile Hello SOM class
hello.obj:  hello.cpp hello.idl hello.xih hello.xh

# Compile C client
cMain.obj: cMain.c hello.h

# Compile C++ client
cppMain.obj: cppMain.cpp hello.xh
