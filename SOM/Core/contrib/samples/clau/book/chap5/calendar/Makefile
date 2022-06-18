.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj .def 
#
# Need /Ge- to build DLL
#
OBJS  = workitem.obj day.obj caldir.obj initfunc.obj
UILIB = dde4muii.lib dde4cci.lib dde4mbsi.lib os2386.lib

all: calendar.dll planner.exe som.ir somdimpl

#
# mysvr.cpp contains a sample server. You can use it
# instead of the generic dsom server
#
tstsvr: mysvr.exe myimpl

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

clnimpl:
        -regimpl -D -i Chris
        -regimpl -D -i Kevin

workitem.obj: workitem.xih workitem.xh workitem.cpp
workitem.xih: workitem.idl
workitem.xh:  workitem.idl
workitem.cpp: workitem.xih

day.obj: day.xih day.xh day.cpp       
day.xih: day.idl
day.xh:  day.idl                            
day.cpp: day.xih                            

caldir.obj: caldir.xih caldir.xh caldir.cpp       
caldir.xih: caldir.idl                            
caldir.xh:  caldir.idl                            
caldir.cpp: caldir.xih                            

initfunc.obj: initfunc.cpp

#
# Build the DLL
#
calendar.dll: $(OBJS) calendar.def
        icc @<< 
        /Fe"calendar.dll" $(OBJS) calendar.def somtk.lib
<<
        implib calendar.lib calendar.def

#
# Build the executable
#
planwin.obj:  planwin.cpp planwin.hpp week.h
        icc /c+ /Gd+ /Gm+ /Si+ -I. planwin.cpp
planner.exe: planwin.obj week.res
        icc /Fe"planner.exe" planwin.obj /B" /pm:pm /noi" \
        $(UILIB) somtk.lib calendar.lib planner.def
        rc week.res planner.exe

#
# Build the test server.
#
mysvr.obj: mysvr.cpp
        icc /c+ /Gd+ /Gm+ /Si+ mysvr.cpp
mysvr.exe: mysvr.obj
        icc /Fe"mysvr.exe" mysvr.obj somtk.lib
#
# You can use "mySvr" instead of "Chris" or "Kevin"
#
myimpl: mysvr.cpp
        -regimpl -A -i mySvr -p mysvr.exe
        -regimpl -a -i mySvr -c WorkItem -c Day -c CalendarDir
        -regimpl -L -i mySvr

week.res: week.rc week.h
          rc -r week.rc

#
# Put the IDL descriptions into the Interface Repository
#
som.ir: workitem.idl day.idl caldir.idl
        sc -sir -u workitem.idl
        sc -sir -u day.idl
        sc -sir -u caldir.idl
#
# Build the DSOM Implementation Repository.
# Register two servers: Chris and Kevin
#
somdimpl: workitem.idl day.idl caldir.idl
        -regimpl -A -i Chris
        -regimpl -a -i Chris -c WorkItem -c Day -c CalendarDir
        -regimpl -L -i Chris
#
        -regimpl -A -i Kevin
        -regimpl -a -i Kevin -c WorkItem -c Day -c CalendarDir
        -regimpl -L -i Kevin
        @echo x > somdimpl
