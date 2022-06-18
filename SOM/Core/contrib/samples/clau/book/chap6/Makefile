.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj .def 
#
# Need /Ge- to build DLL
#
OBJS  = workitem.obj day.obj caldir.obj initfunc.obj
POBJS = perserv.obj 
UILIB = dde4muii.lib dde4cci.lib dde4mbsi.lib os2386.lib

all: calendar.dll perserv.dll planner.exe som.ir somdimpl

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
        -regimpl -D -i pChris
        -regimpl -D -i pKevin

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

perserv.obj: perserv.xih perserv.xh perserv.cpp
perserv.xih: perserv.idl                             
perserv.xh:  perserv.idl                             
perserv.cpp: perserv.xih                             

#
# Build the Calendar DLL
#
calendar.dll: $(OBJS) calendar.def
        icc @<< 
        /Fe"calendar.dll" $(OBJS) calendar.def
        somtk.lib 
<<
        implib calendar.lib calendar.def

#
# Build the Persistent Server DLL
#
perserv.dll: $(POBJS) perserv.def
        icc @<<                                     
        /Fe"perserv.dll" $(POBJS) perserv.def
        somtk.lib                                   
<<                                                  
        implib perserv.lib perserv.def
                                                    
#
# Build the executables
#
planwin.obj:  planwin.cpp planwin.hpp week.h
        icc /c+ /Gd+ /Gm+ /Si+ -I. planwin.cpp
planner.exe: planwin.obj week.res
        icc /Fe"planner.exe" planwin.obj /B" /pm:pm /noi" \
        $(UILIB) somtk.lib calendar.lib perserv.lib planner.def
        rc week.res planner.exe

week.res: week.rc week.h
          rc -r week.rc
#
# Put the IDL descriptions into the Interface Repository
#
som.ir: workitem.idl day.idl caldir.idl perserv.idl
        sc -sir -u workitem.idl
        sc -sir -u day.idl
        sc -sir -u caldir.idl
        sc -sir -u perserv.idl
#
# Build the DSOM Implementation Repository.
# Register two servers: pChris and pKevin
#
somdimpl: workitem.idl day.idl caldir.idl perserv.idl
        -regimpl -A -i pChris -p $(SOMBASE)\bin\somdsvr.exe -v PersistentServer
        -regimpl -a -i pChris -c WorkItem -c Day -c CalendarDir
        -regimpl -L -i pChris
#
        -regimpl -A -i pKevin -p $(SOMBASE)\bin\somdsvr.exe -v PersistentServer
        -regimpl -a -i pKevin -c WorkItem -c Day -c CalendarDir
        -regimpl -L -i pKevin
        @echo x > somdimpl
