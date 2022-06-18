.SUFFIXES:
.SUFFIXES: .idl .xih .xh .cpp .obj .def 
#
# Need /Ge- to build DLL
#
OBJS  = caldir.obj workitem.obj day.obj initfunc.obj
OBJS2 = dayview.obj planhdr.obj

UILIB = dde4muii.lib dde4cci.lib dde4mbsi.lib os2386.lib

all: calendar.dll planner.exe som.ir

.cpp.obj:
        icc /c+ /Ge- -I. $<
.idl.xh:
         sc -sxh  $*.idl
.idl.xih:
         sc -sxih  $*.idl
.idl.cpp:
         sc -sxc $*.idl

caldir.obj: caldir.xih caldir.xh caldir.cpp        
caldir.xih: caldir.idl                             
caldir.xh:  caldir.idl                             
caldir.cpp: caldir.xih                             

workitem.obj: workitem.xih workitem.xh workitem.cpp
workitem.xih: workitem.idl
workitem.xh:  workitem.idl
workitem.cpp: workitem.xih

day.obj: day.xih day.xh day.cpp       
day.xih: day.idl
day.xh:  day.idl                            
day.cpp: day.xih                            

dayview.xih: dayview.idl                            
dayview.xh:  dayview.idl                            
dayview.cpp: dayview.xih                            

planview.obj: planview.xih planview.xh planview.cpp  
planview.xih: planview.idl                 
planview.xh:  planview.idl                 
planview.cpp: planview.xih                 

initfunc.obj: initfunc.cpp

#
# Build the DLL
#
calendar.dll: $(OBJS) calendar.def
        icc @<< 
        /Fe"calendar.dll" $(OBJS) calendar.def
        somtk.lib 
<<
        implib calendar.lib calendar.def
#
# Build the executables
#
planwin.obj:  planwin.cpp planwin.hpp week.h
        icc /c+ /Gd+ /Gm+ /Si+ -I. planwin.cpp
planner.exe: $(OBJS2) planwin.obj week.res
        icc /Fe"planner.exe" $(OBJS2) planwin.obj /B" /pm:pm /noi" \
        $(UILIB) somtk.lib calendar.lib planner.def
        rc week.res planner.exe

planhdr.obj: planhdr.cpp planhdr.hpp
        icc /c+ /Gd+ /Gm+ /Si+ -I. planhdr.cpp
dayview.obj: dayview.cpp dayview.xh dayview.xih
        icc /c+ /Gd+ /Gm+ /Si+ -I. dayview.cpp

week.res: week.rc week.h
          rc -r week.rc
#
# Put the IDL descriptions into the Interface Repository
#
som.ir: caldir.idl workitem.idl day.idl
        sc -sir -u workitem.idl
        sc -sir -u day.idl
        sc -sir -u caldir.idl
