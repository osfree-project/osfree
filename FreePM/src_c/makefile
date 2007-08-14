# makefile
# Created by IBM WorkFrame/2 MakeMake at 11:42:43 on 6 Nov 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES:

.SUFFIXES: \
    .cpp .obj 

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\include /Ss /Wpar /Ti /G5 /Gu /C %s

{M:\Evgen\FreePM\lib}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\include /Ss /Wpar /Ti /G5 /Gu /C %s

{M:\Evgen\FreePM\Client}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\include /Ss /Wpar /Ti /G5 /Gu /C %s

all: \
    .\FreePM.exe

.\FreePM.exe: \
    .\F_WinStartApp.obj \
    .\WIN1.obj \
    .\F_GPI.obj \
    .\F_hab.obj \
    .\F_StdWindow.obj \
    .\F_Window.obj \
    .\snprintf.obj \
    .\Fc_main.obj \
    .\FreePM_win.obj \
    .\F_Client.obj \
    .\F_debug.obj \
    .\F_errors.obj \
    .\F_globals.obj \
    .\F_messages.obj \
    .\F_utils.obj \
    .\F_win1.obj
    @echo " Link::Linker "
    icc.exe @<<
     /B" /de /exepack:2 /packd /optfunc /noe"
     /FeFreePM.exe 
     .\F_WinStartApp.obj
     .\WIN1.obj
     .\F_GPI.obj
     .\F_hab.obj
     .\F_StdWindow.obj
     .\F_Window.obj
     .\snprintf.obj
     .\Fc_main.obj
     .\FreePM_win.obj
     .\F_Client.obj
     .\F_debug.obj
     .\F_errors.obj
     .\F_globals.obj
     .\F_messages.obj
     .\F_utils.obj
     .\F_win1.obj
<<

.\F_WinStartApp.obj: \
    M:\Evgen\FreePM\Client\F_WinStartApp.cpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_win.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_err.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_winConstants.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_shl.hpp

.\F_win1.obj: \
    M:\Evgen\FreePM\Client\F_win1.cpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_win.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_err.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_winConstants.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_globals.hpp

.\F_utils.obj: \
    M:\Evgen\FreePM\Client\F_utils.cpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_utils.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_globals.hpp

.\F_messages.obj: \
    M:\Evgen\FreePM\Client\F_messages.cpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM.hpp

.\F_globals.obj: \
    M:\Evgen\FreePM\Client\F_globals.cpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_globals.hpp

.\F_errors.obj: \
    M:\Evgen\FreePM\Client\F_errors.cpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_err.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}snprintf.h \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}os2_ErrCodes.h \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_ErrCodes.h

.\F_debug.obj: \
    M:\Evgen\FreePM\Client\F_debug.cpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_pipe.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}snprintf.h

.\F_Client.obj: \
    M:\Evgen\FreePM\Client\F_Client.cpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_win.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_err.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_winConstants.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_pipe.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_globals.hpp

.\FreePM_win.obj: \
    M:\Evgen\FreePM\Client\FreePM_win.cpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_win.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_err.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_winConstants.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_pipe.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_utils.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_hab.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_cmd.hpp

.\Fc_main.obj: \
    M:\Evgen\FreePM\Client\Fc_main.cpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_pipe.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_utils.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_globals.hpp

.\snprintf.obj: \
    M:\Evgen\FreePM\lib\snprintf.cpp

.\F_Window.obj: \
    M:\Evgen\FreePM\lib\F_Window.cpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_hab.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_pipe.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}FreePM_cmd.hpp

.\F_StdWindow.obj: \
    M:\Evgen\FreePM\lib\F_StdWindow.cpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}FreePM_winConstants.hpp

.\F_hab.obj: \
    M:\Evgen\FreePM\lib\F_hab.cpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_hab.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_globals.hpp

.\F_GPI.obj: \
    M:\Evgen\FreePM\lib\F_GPI.cpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_GPI.HPP \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_basedef.HPP

.\WIN1.obj: \
    M:\Evgen\FreePM\Client\WIN1.CPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_win.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_err.hpp \
    {M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}FreePM_winConstants.hpp
