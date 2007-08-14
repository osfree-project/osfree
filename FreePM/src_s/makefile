# makefile
# Created by IBM WorkFrame/2 MakeMake at 12:55:32 on 15 Nov 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES:

.SUFFIXES: \
    .cpp .obj 

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\include /DFREPM_SERVER /Ss /Ti /Gm /G5 /Tm /Gu /C %s

{M:\Evgen\FreePM\lib}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\include /DFREPM_SERVER /Ss /Ti /Gm /G5 /Tm /Gu /C %s

{M:\Evgen\FreePM\Server}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\include /DFREPM_SERVER /Ss /Ti /Gm /G5 /Tm /Gu /C %s

all: \
    .\FreePMs.exe

.\FreePMs.exe: \
    .\F_session.obj \
    .\F_utils.obj \
    .\F_GPI.obj \
    .\F_hab.obj \
    .\F_StdWindow.obj \
    .\F_Window.obj \
    .\snprintf.obj \
    .\Fs_config.obj \
    .\Fs_GD.obj \
    .\Fs_globals.obj \
    .\Fs_hab.obj \
    .\Fs_main.obj \
    .\Fs_pipe.obj \
    .\Fs_PMdev.obj \
    .\Fs_queue.obj \
    .\Fs_wnd.obj \
    .\F_debug.obj \
    .\F_DeskTop.obj \
    .\F_errors.obj \
    ..\lib\GDLib\gdlib.lib
    @echo " Link::Linker "
    icc.exe @<<
     /B" /de /exepack:2 /packd /optfunc"
     /FeFreePMs.exe 
     ..\lib\GDLib\gdlib.lib 
     .\F_session.obj
     .\F_utils.obj
     .\F_GPI.obj
     .\F_hab.obj
     .\F_StdWindow.obj
     .\F_Window.obj
     .\snprintf.obj
     .\Fs_config.obj
     .\Fs_GD.obj
     .\Fs_globals.obj
     .\Fs_hab.obj
     .\Fs_main.obj
     .\Fs_pipe.obj
     .\Fs_PMdev.obj
     .\Fs_queue.obj
     .\Fs_wnd.obj
     .\F_debug.obj
     .\F_DeskTop.obj
     .\F_errors.obj
<<

.\F_session.obj: \
    M:\Evgen\FreePM\Server\F_session.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePMs.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_WND.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_queue.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_hab.hpp

.\F_errors.obj: \
    M:\Evgen\FreePM\Server\F_errors.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}snprintf.h \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM_err.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}os2_ErrCodes.h \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM_ErrCodes.h

.\F_DeskTop.obj: \
    M:\Evgen\FreePM\Server\F_DeskTop.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_utils.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_hab.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePMs.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_WND.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_queue.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_hab.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM_err.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_GPI.HPP

.\F_debug.obj: \
    M:\Evgen\FreePM\Server\F_debug.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_pipe.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}snprintf.h

.\Fs_wnd.obj: \
    M:\Evgen\FreePM\Server\Fs_wnd.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_WND.hpp

.\Fs_queue.obj: \
    M:\Evgen\FreePM\Server\Fs_queue.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_queue.hpp

.\Fs_PMdev.obj: \
    M:\Evgen\FreePM\Server\Fs_PMdev.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}OS2.h

.\Fs_pipe.obj: \
    M:\Evgen\FreePM\Server\Fs_pipe.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_pipe.hpp

.\Fs_main.obj: \
    M:\Evgen\FreePM\Server\Fs_main.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_utils.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_hab.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePMs.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_WND.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_queue.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_hab.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_pipe.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM_cmd.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}snprintf.h

.\Fs_hab.obj: \
    M:\Evgen\FreePM\Server\Fs_hab.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePMs.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_WND.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_queue.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_hab.hpp

.\Fs_globals.obj: \
    M:\Evgen\FreePM\Server\Fs_globals.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_messages.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_hab.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePMs.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_WND.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_queue.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_hab.hpp

.\Fs_GD.obj: \
    M:\Evgen\FreePM\Server\Fs_GD.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}GDLib/gd.h \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}GDLib/gdfontg.h \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}GDLib/gdfonts.h \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}GDLIB/gd_io.h

.\Fs_config.obj: \
    M:\Evgen\FreePM\Server\Fs_config.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}Fs_config.hpp

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
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}FreePM_winConstants.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}F_pipe.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}FreePM_cmd.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}FreePM_win.hpp \
    {M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}FreePM_err.hpp

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

.\F_utils.obj: \
    M:\Evgen\FreePM\Server\F_utils.cpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_def.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_config.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_globals.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}FreePM.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_utils.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_OS2.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_basedef.HPP \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_PresPar.hpp \
    {M:\Evgen\FreePM\Server;..\include;$(INCLUDE);}F_messages.hpp
