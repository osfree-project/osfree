# makefile
# Created by IBM WorkFrame/2 MakeMake at 11:42:43 on 6 Nov 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

!include ../watcom.mif

#  $(sep)  -d2 
#sep=/
CC=wcc386
CPP=wpp386
CFLAGS= -bt=$(SYSTEM_TARGET) -i..$(sep)include -i. $(CLIENT_CFLAGS)  -dHAVE_LIBJPEG -dHAVE_LIBPNG
# -bm  -5s

# '.obj' is changed to '$(OBJEXT)' 
# ' \' is changed to ' &'
# '.\' is changed to './'
# '{M:\Evgen\FreePM\lib}' is changed to '$(freepm_lib_dir)'
# '{M:\Evgen\FreePM\Client;..\include;$(INCLUDE);}'   to  '$(freepm_client_dir)'
# 'M:\Evgen\FreePM\Client\'    to    '$(freepm_client_dir)/'
# '{M:\Evgen\FreePM\lib;..\include;$(INCLUDE);}'  to   '$(freepm_lib_dir)/'
# 'M:\Evgen\FreePM\lib\'                to    '$(freepm_lib_dir)/'

#lpng_dir=$(sep)pub$(sep)FreePM$(sep)FreePM$(sep)lib$(sep)lpng
#jpeg_dir=$(sep)pub$(sep)FreePM$(sep)FreePM$(sep)lib$(sep)JpegLib
#freepm_client_dir=$(sep)pub$(sep)FreePM$(sep)FreePM$(sep)src_c
#freepm_lib_dir=$(sep)pub$(sep)FreePM$(sep)FreePM$(sep)lib
#freepm_inc_dir=$(sep)pub$(sep)FreePM$(sep)FreePM$(sep)include


.SUFFIXES:

.SUFFIXES: &
    $(OBJEXT) .cpp 

.cpp$(OBJEXT):
    #icc.exe /I..\include /Ss /Wpar /Ti /G5 /Gu /C %s
    $(CPP)  $(CFLAGS) -oaxt $[@

#$(freepm_lib_dir).cpp$(OBJEXT):
#   echo " Compile::C++ Compiler "
    #icc.exe /I..\include /Ss /Wpar /Ti /G5 /Gu /C %s

#$(freepm_client_dir).cpp$(OBJEXT):
#    echo " Compile::C++ Compiler "
    #icc.exe /I..\include /Ss /Wpar /Ti /G5 /Gu /C %s

all: clean_lib &
    .$(sep)$(CLIENT_EXE)

clean:
    rm -f *.o *.obj
    rm -f ..$(sep)lib$(sep)*.o ..$(sep)lib$(sep)*.obj .$(sep)$(CLIENT_EXE)

clean_lib: .SYMBOLIC
	rm -f  ..$(sep)lib$(sep)*$(OBJEXT) 

# .$(sep)snprintf$(OBJEXT) &

.$(sep)$(CLIENT_EXE): &
    .$(sep)F_WinStartApp$(OBJEXT) &
    .$(sep)FreePM_win$(OBJEXT) &
    .$(sep)WIN1$(OBJEXT) &
    ..$(sep)lib$(sep)F_GPI$(OBJEXT) &
    ..$(sep)lib$(sep)F_hab$(OBJEXT) &
    ..$(sep)lib$(sep)F_StdWindow$(OBJEXT) &
    .$(sep)Fc_config$(OBJEXT) &
    .$(sep)Fc_main$(OBJEXT) &
    .$(sep)F_Client$(OBJEXT) &
    .$(sep)F_debug$(OBJEXT) &
    .$(sep)F_errors$(OBJEXT) &
    .$(sep)F_globals$(OBJEXT) &
    .$(sep)F_messages$(OBJEXT) &
    .$(sep)F_utils$(OBJEXT) &
    .$(sep)F_win1$(OBJEXT) &
    ..$(sep)lib$(sep)F_Window$(OBJEXT)
    @echo " Link::Linker "
    #icc.exe @<<
    # /B" /de /exepack:2 /packd /optfunc /noe"
    # /FeFreePM.exe 
    #         file ./WIN1$(OBJEXT) &
    wlink system $(SYSTEM_TARGET) $(CLIENT_LDFLAGS) name $(CLIENT_EXE) $(DEBUG_TARGET) &
     file ..$(sep)lib$(sep)Zlib$(sep)Zlib.lib &
     file ..$(sep)lib$(sep)JpegLib$(sep)JpegLib.lib &
     file ..$(sep)lib$(sep)lpng$(sep)lpng.lib &
     file ..$(sep)lib$(sep)GDLib$(sep)gdlib.lib  &
     file .$(sep)F_WinStartApp$(OBJEXT) &
     file ..$(sep)lib$(sep)F_GPI$(OBJEXT) &
     file ..$(sep)lib$(sep)F_hab$(OBJEXT) &
     file ..$(sep)lib$(sep)F_StdWindow$(OBJEXT) &
     file .$(sep)Fc_config$(OBJEXT) &
     file .$(sep)Fc_main$(OBJEXT) &
     file .$(sep)FreePM_win$(OBJEXT) &
     file .$(sep)F_Client$(OBJEXT) &
     file .$(sep)F_debug$(OBJEXT) &
     file .$(sep)F_errors$(OBJEXT) &
     file .$(sep)F_globals$(OBJEXT) &
     file .$(sep)F_messages$(OBJEXT) &
     file .$(sep)F_utils$(OBJEXT) &
     file .$(sep)F_win1$(OBJEXT) &
     file .$(sep)WIN1$(OBJEXT) &
     file ..$(sep)lib$(sep)F_Window$(OBJEXT)
#<<

.$(sep)F_WinStartApp$(OBJEXT): &
    $(freepm_client_dir)$(sep)F_WinStartApp.cpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_win.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)FreePM_err.hpp &
    $(freepm_inc_dir)$(sep)FreePM_winConstants.hpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_shl.hpp

.$(sep)F_win1$(OBJEXT): &
    $(freepm_client_dir)$(sep)F_win1.cpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_win.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)FreePM_err.hpp &
    $(freepm_inc_dir)$(sep)FreePM_winConstants.hpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_globals.hpp

.$(sep)F_utils$(OBJEXT): &
    $(freepm_client_dir)$(sep)F_utils.cpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_utils.hpp &
    $(freepm_inc_dir)$(sep)F_globals.hpp

.$(sep)F_messages$(OBJEXT): &
    $(freepm_client_dir)$(sep)F_messages.cpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp

.$(sep)F_globals$(OBJEXT): &
    $(freepm_client_dir)$(sep)F_globals.cpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_globals.hpp

.$(sep)F_errors$(OBJEXT): &
    $(freepm_client_dir)$(sep)F_errors.cpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)FreePM_err.hpp &
    $(freepm_inc_dir)$(sep)F_globals.hpp &
    $(freepm_inc_dir)$(sep)os2_ErrCodes.h &
    $(freepm_inc_dir)$(sep)FreePM_ErrCodes.h

.$(sep)F_debug$(OBJEXT): &
    $(freepm_client_dir)$(sep)F_debug.cpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_pipe.hpp &
    $(freepm_inc_dir)$(sep)F_globals.hpp

.$(sep)F_Client$(OBJEXT): &
    $(freepm_client_dir)$(sep)F_Client.cpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_win.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)FreePM_err.hpp &
    $(freepm_inc_dir)$(sep)FreePM_winConstants.hpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_pipe.hpp &
    $(freepm_inc_dir)$(sep)F_globals.hpp

.$(sep)FreePM_win$(OBJEXT): &
    $(freepm_client_dir)$(sep)FreePM_win.cpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_win.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)FreePM_err.hpp &
    $(freepm_inc_dir)$(sep)FreePM_winConstants.hpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_pipe.hpp &
    $(freepm_inc_dir)$(sep)F_utils.hpp &
    $(freepm_inc_dir)$(sep)F_globals.hpp &
    $(freepm_inc_dir)$(sep)F_hab.hpp &
    $(freepm_inc_dir)$(sep)FreePM_cmd.hpp

.$(sep)Fc_main$(OBJEXT): &
    $(freepm_client_dir)$(sep)Fc_main.cpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_pipe.hpp &
    $(freepm_inc_dir)$(sep)F_utils.hpp &
    $(freepm_inc_dir)$(sep)F_globals.hpp

#.$(sep)snprintf$(OBJEXT): &
#    $(freepm_lib_dir)$(sep)snprintf.cpp

..$(sep)lib$(sep)F_Window$(OBJEXT): &
    ..$(sep)lib$(sep)F_Window.cpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_hab.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_globals.hpp &
    $(freepm_inc_dir)$(sep)F_pipe.hpp &
    $(freepm_inc_dir)$(sep)FreePM_cmd.hpp
    cd ..$(sep)lib
    $(CPP)  $(CFLAGS) -oaxt $[@
    cd ..$(sep)src_c

..$(sep)lib$(sep)F_StdWindow$(OBJEXT): &
    ..$(sep)lib$(sep)F_StdWindow.cpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)FreePM_winConstants.hpp
    cd ..$(sep)lib
    $(CPP)  $(CFLAGS) -oaxt $[@
    cd ..$(sep)src_c

..$(sep)lib$(sep)F_hab$(OBJEXT): &
    ..$(sep)lib$(sep)F_hab.cpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_hab.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_globals.hpp
    cd ..$(sep)lib
    $(CPP)  $(CFLAGS) -oaxt $[@
    cd ..$(sep)src_c

..$(sep)lib$(sep)F_GPI$(OBJEXT): &
    ..$(sep)lib$(sep)F_GPI.cpp &
    $(freepm_inc_dir)$(sep)FreePM.hpp &
    $(freepm_inc_dir)$(sep)F_GPI.hpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp
    cd ..$(sep)lib
    $(CPP)  $(CFLAGS) -oaxt $[@
    cd ..$(sep)src_c

.$(sep)WIN1$(OBJEXT): &
    $(freepm_client_dir)$(sep)WIN1.cpp &
    $(freepm_inc_dir)$(sep)F_win.hpp &
    $(freepm_inc_dir)$(sep)F_config.hpp &
    $(freepm_inc_dir)$(sep)F_def.hpp &
    $(freepm_inc_dir)$(sep)F_OS2.hpp &
    $(freepm_inc_dir)$(sep)F_PresPar.hpp &
    $(freepm_inc_dir)$(sep)F_basedef.hpp &
    $(freepm_inc_dir)$(sep)F_messages.hpp &
    $(freepm_inc_dir)$(sep)FreePM_err.hpp &
    $(freepm_inc_dir)$(sep)FreePM_winConstants.hpp


    
