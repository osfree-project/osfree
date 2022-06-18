.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj

OBJS = context.obj testctx.obj

all: testctx.exe

.cpp.obj:
        icc /c+ -I. /O+ $<
.idl.xh:
         sc -sxh  $*.idl
.idl.xih:
         sc -sxih  $*.idl
.idl.cpp:
         sc -sxc $*.idl

context.obj : context.cpp
context.cpp : context.idl context.xh context.xih
context.xh  : context.idl
context.xih : context.idl

testctx.obj: testctx.cpp
testctx.exe: $(OBJS)
        icc /Fe"testctx.exe" $(OBJS) somtk.lib
