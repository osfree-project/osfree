# makefile
# Created by IBM WorkFrame/2 MakeMake at 12:55:32 on 15 Nov 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#
# Adapted to Open Watcom in may 2007 by Sven-Erik Rosén

!include ../watcom.mif

#wpp
#-bm             build target is a multi-thread environment
#  $(sep)  -d2
#sep=/
CC= wcc386
CPP= wpp386
CFLAGS= -i..$(sep)include -i..$(sep)lib$(sep)GDLib $(SERVER_CFLAGS) -dFREPM_SERVER

# $(OBJEXT)

.SUFFIXES:

.SUFFIXES: &
    $(OBJEXT) .cpp

#.c.obj :
#    wcc386 -zq -oaxt $<
#
#hello.obj :
#  $[@

.cpp$(OBJEXT) :
    @echo $[@
    wpp386  $(CFLAGS) -oaxt $[@

#../lib/F_GPI.cpp.o :
#    @echo $[@
#    wpp386  $(CFLAGS) -oaxt $[@

#.cpp.obj:
#    echo " Compile::C++ Compiler " &
    #icc.exe /I../include /DFREPM_SERVER /Ss /Ti /Gm /G5 /Tm /Gu /C %s

#{../lib}.cpp.obj:
#    echo " Compile::C++ Compiler "
    #icc.exe /I../include /DFREPM_SERVER /Ss /Ti /Gm /G5 /Tm /Gu /C %s

#{../src_s}.cpp.obj:
#    echo " Compile::C++ Compiler "
    #icc.exe /I../include /DFREPM_SERVER /Ss /Ti /Gm /G5 /Tm /Gu /C %s



all: clean_lib &
    .$(sep)$(SERVER_EXE)

clean: .SYMBOLIC
    rm -f *.o *.obj .$(sep)$(SERVER_EXE) ..$(sep)lib$(sep)*.o ..$(sep)lib$(sep)*.obj

clean_lib: .SYMBOLIC
        rm -f  ..$(sep)lib$(sep)*$(OBJEXT)

#     #./snprintf$(OBJEXT) &
#    ./Fs_GD$(OBJEXT) &
#    ..$(sep)src_c$(sep)Fc_main$(OBJEXT) &

.$(sep)$(SERVER_EXE): &
    ..$(sep)lib$(sep)Zlib$(sep)Zlib.lib &
    ..$(sep)lib$(sep)JpegLib$(sep)JpegLib.lib &
    ..$(sep)lib$(sep)lpng$(sep)lpng.lib &
    ..$(sep)lib$(sep)GDLib$(sep)gdlib.lib
        #    echo " Link::Linker "
        #    icc.exe @<<
        #     /B" /de /exepack:2 /packd /optfunc"
        #     /FeFreePMs.exe
        # file ./snprintf$(OBJEXT) &
        #      file ./Fs_GD$(OBJEXT) &
        #     file ..$(sep)src_c$(sep)Fc_main$(OBJEXT) &
        #file ..$(sep)src_c$(sep)Fc_config$(OBJEXT) &

     wlink system $(SYSTEM_TARGET) name $(SERVER_EXE) $(DEBUG_TARGET) $(SERVER_LDFLAGS) &
     file .$(sep)F_session$(OBJEXT) &
     file .$(sep)F_utils$(OBJEXT) &
     file ..$(sep)lib$(sep)F_GPI$(OBJEXT) &
     file ..$(sep)lib$(sep)F_hab$(OBJEXT) &
     file ..$(sep)lib$(sep)F_StdWindow$(OBJEXT) &
     file ..$(sep)lib$(sep)F_Window$(OBJEXT) &
     file .$(sep)Fs_main$(OBJEXT) &
     file .$(sep)Fs_config$(OBJEXT) &
     file .$(sep)Fs_globals$(OBJEXT) &
     file .$(sep)Fs_hab$(OBJEXT) &
     file .$(sep)Fs_pipe$(OBJEXT) &
     file .$(sep)Fs_PMdev$(OBJEXT) &
     file .$(sep)Fs_queue$(OBJEXT) &
     file .$(sep)Fs_wnd$(OBJEXT) &
     file .$(sep)F_debug$(OBJEXT) &
     file .$(sep)F_DeskTop$(OBJEXT) &
     file .$(sep)F_errors$(OBJEXT) &
     file ..$(sep)lib$(sep)Zlib$(sep)Zlib.lib &
     file ..$(sep)lib$(sep)JpegLib$(sep)JpegLib.lib &
     file ..$(sep)lib$(sep)GDLib$(sep)gdlib.lib  &
     file ..$(sep)lib$(sep)lpng$(sep)lpng.lib
#<<


#./F_session.obj: &
#    ../src_s/F_session.cpp &
#    ../include/F_def.hpp &
#    ../include/F_config.hpp &
#    ../include/F_globals.hpp &
#    ../include/FreePM.hpp &
#    ../include/F_basedef.hpp &
#    ../include/F_PresPar.hpp &
#    ../include/F_messages.hpp &
#    Fs_globals.hpp &
#    ../include/FreePMs.hpp &
#    Fs_WND.hpp &
#    ../include/Fs_queue.hpp &
#    Fs_hab.hpp

.$(sep)F_session$(OBJEXT): &
    .$(sep)F_session.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_globals.hpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    Fs_globals.hpp &
    ..$(sep)include$(sep)FreePMs.hpp &
    Fs_WND.hpp &
    ..$(sep)include$(sep)Fs_queue.hpp &
    Fs_hab.hpp
    #wcc386  -oaxt $<
    #echo $<

#    ../include/snprintf.h &

.$(sep)F_errors$(OBJEXT): &
    ..$(sep)src_s$(sep)F_errors.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_globals.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)FreePM_err.hpp &
    ..$(sep)include$(sep)os2_ErrCodes.h &
    ..$(sep)include$(sep)FreePM_ErrCodes.h

.$(sep)F_DeskTop$(OBJEXT): &
    ..$(sep)src_s$(sep)F_DeskTop.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_globals.hpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_utils.hpp &
    ..$(sep)include$(sep)F_OS2.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)include$(sep)F_hab.hpp &
    Fs_globals.hpp &
    ..$(sep)include$(sep)FreePMs.hpp &
    ..$(sep)src_s$(sep)Fs_WND.hpp &
    ..$(sep)include$(sep)Fs_queue.hpp &
    ..$(sep)src_s$(sep)Fs_hab.hpp &
    ..$(sep)include$(sep)FreePM_err.hpp &
    ..$(sep)include$(sep)F_GPI.hpp

# &
#    ../include/snprintf.h

.$(sep)F_debug$(OBJEXT): &
    ..$(sep)src_s$(sep)F_debug.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_globals.hpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_OS2.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)include$(sep)F_pipe.hpp

.$(sep)Fs_wnd$(OBJEXT): &
    ..$(sep)src_s$(sep)Fs_wnd.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_globals.hpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_OS2.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)src_s$(sep)Fs_WND.hpp

.$(sep)Fs_queue$(OBJEXT): &
    ..$(sep)src_s$(sep)Fs_queue.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_OS2.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)include$(sep)Fs_queue.hpp

#  &
#    os2.h

.$(sep)Fs_PMdev$(OBJEXT): &
    ..$(sep)src_s$(sep)Fs_PMdev.cpp

.$(sep)Fs_pipe$(OBJEXT): &
    ..$(sep)src_s$(sep)Fs_pipe.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_OS2.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)include$(sep)F_pipe.hpp
# &
#    ..$(sep)include$(sep)snprintf.h

.$(sep)Fs_main$(OBJEXT): &
    ..$(sep)src_s$(sep)Fs_main.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_globals.hpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_utils.hpp &
    ..$(sep)include$(sep)F_OS2.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)src_s$(sep)Fs_config.hpp &
    ..$(sep)include$(sep)F_hab.hpp &
    Fs_globals.hpp &
    ..$(sep)include$(sep)FreePMs.hpp &
    ..$(sep)src_s$(sep)Fs_WND.hpp &
    ..$(sep)include$(sep)Fs_queue.hpp &
    ..$(sep)src_s$(sep)Fs_hab.hpp &
    ..$(sep)include$(sep)F_pipe.hpp &
    ..$(sep)include$(sep)FreePM_cmd.hpp

.$(sep)Fs_hab$(OBJEXT): &
    ..$(sep)src_s$(sep)Fs_hab.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_globals.hpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_OS2.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    Fs_globals.hpp &
    ..$(sep)include$(sep)FreePMs.hpp &
    ..$(sep)src_s$(sep)Fs_WND.hpp &
    ..$(sep)include$(sep)Fs_queue.hpp &
    ..$(sep)src_s$(sep)Fs_hab.hpp

.$(sep)Fs_globals$(OBJEXT): &
    ..$(sep)src_s$(sep)Fs_globals.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_globals.hpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)include$(sep)F_hab.hpp &
    Fs_globals.hpp &
    ..$(sep)include$(sep)FreePMs.hpp &
    ..$(sep)src_s$(sep)Fs_WND.hpp &
    ..$(sep)include$(sep)Fs_queue.hpp &
    ..$(sep)src_s$(sep)Fs_hab.hpp

# ./Fs_GD.cpp &

#./Fs_GD$(OBJEXT): &
#    ../lib/GDLib/gd.h &
#    ../lib/GDLib/gdfontg.h &
#    ../lib/GDLib/gdfonts.h &
#    ../lib/GDLIB/gd_io.h

.$(sep)Fs_config$(OBJEXT): &
    ..$(sep)src_s$(sep)Fs_config.cpp &
    ..$(sep)src_s$(sep)Fs_config.hpp

#../lib/snprintf$(OBJEXT): &
    #../lib/snprintf.cpp

..$(sep)lib$(sep)F_Window$(OBJEXT): &
    ..$(sep)lib$(sep)F_Window.cpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_hab.hpp &
    ..$(sep)include$(sep)F_OS2.hpp &
    ..$(sep)include$(sep)F_globals.hpp &
    ..$(sep)include$(sep)FreePM_winConstants.hpp &
    ..$(sep)include$(sep)F_pipe.hpp &
    ..$(sep)include$(sep)FreePM_cmd.hpp &
    ..$(sep)src_c$(sep)FreePM_win.cpp &
    ..$(sep)include$(sep)FreePM_err.hpp
    cd ..$(sep)lib
    wpp386  $(CFLAGS) -oaxt $[@
    cd ..$(sep)src_s

..$(sep)lib$(sep)F_StdWindow$(OBJEXT): &
    ..$(sep)lib$(sep)F_StdWindow.cpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)FreePM_winConstants.hpp
    cd ..$(sep)lib
    $(CPP)  $(CFLAGS) -oaxt $[@
    cd ..$(sep)src_s


..$(sep)lib$(sep)F_hab$(OBJEXT): &
    ..$(sep)lib$(sep)F_hab.cpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_hab.hpp &
    ..$(sep)include$(sep)F_OS2.hpp &
    ..$(sep)include$(sep)F_globals.hpp
    cd ..$(sep)lib
    $(CPP)  $(CFLAGS) -oaxt $[@
    cd ..$(sep)src_s

..$(sep)lib$(sep)F_GPI$(OBJEXT): &
    ..$(sep)lib$(sep)F_GPI.cpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_GPI.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_messages.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_basedef.hpp
    cd ..$(sep)lib
    $(CPP)  $(CFLAGS) -oaxt $[@
    cd ..$(sep)src_s

.$(sep)F_utils$(OBJEXT): &
    ..$(sep)src_s$(sep)F_utils.cpp &
    ..$(sep)include$(sep)F_def.hpp &
    ..$(sep)include$(sep)F_config.hpp &
    ..$(sep)include$(sep)F_globals.hpp &
    ..$(sep)include$(sep)FreePM.hpp &
    ..$(sep)include$(sep)F_utils.hpp &
    ..$(sep)include$(sep)F_OS2.hpp &
    ..$(sep)include$(sep)F_basedef.hpp &
    ..$(sep)include$(sep)F_PresPar.hpp &
    ..$(sep)include$(sep)F_messages.hpp


