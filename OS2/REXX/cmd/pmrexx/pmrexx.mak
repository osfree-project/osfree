#===================================================================
#
#   PMREXX.MAK - PM REXX Interface Sample Makefile
#   Copyright  IBM Corporation 1996
#
#===================================================================
#===================================================================
# Include the file ibmsamp.inc which defines the various macros
# depending on the target specified.
#
# The following macros are defined in ibmsamp.inc:
#
#       OBJDIR   - Directory where object files are created
#       EXEDIR   - Directory where executable files are created
#       RC       - Resource compiler
#       RCFLAGS  - Resource compiler flags
#       LINK     - Linker
#       LFLAGS   - Linker flags
#       MTLIBS   - Multithreaded versions of the runtime libraries
#       REXXLIBS - REXX libraries
#===================================================================

!include ..\..\..\ibmsamp.inc

HEADERS = pmrexx.h pmrexxio.h pmrxhlp.h pmrxiodt.h rhdtatyp.h rxhdluse.h
#-------------------------------------------------------------------
#
#   A list of all of the object files
#
#-------------------------------------------------------------------

EXE_OBJS = $(OBJDIR)\pmrexx.obj   $(OBJDIR)\rxhafile.obj
DLL_OBJS = $(OBJDIR)\pmrexxio.obj $(OBJDIR)\pmrxsubs.obj

ALL_IPF = pmrexxio.ipf
#-------------------------------------------------------------------
#   This section lists all files to be built by the make.  The
#   makefile builds the executible as well as its associated help
#   file.
#-------------------------------------------------------------------
all: $(EXEDIR)\pmrexx.exe $(EXEDIR)\pmrexxio.dll $(EXEDIR)\pmrexxio.hlp

#-------------------------------------------------------------------
#   This section creates the command file used by the linker.  This
#   command file is recreated automatically every time you change
#   the object file list, linker flags, or library list.
#-------------------------------------------------------------------
#-------------------------------------------------------------------
#   Dependencies
#     This section lists all object files needed to be built for the
#     application, along with the files it is dependent upon (e.g.
#     its source and any header files).
#-------------------------------------------------------------------
$(OBJDIR)\pmrexx.res: pmrexx.rc $(HEADERS) pmrexx.ico prodinfo.bmp
                $(RC) $(RCFLAGS) $(@B).rc $@

$(OBJDIR)\pmrexxio.res: pmrexxio.rc rxiohlp.rc $(HEADERS) prodinfo.bmp
                $(RC) $(RCFLAGS) $(@B).rc $@
                @del prodinfo.bmp

prodinfo.bmp:
           copy ..\..\..\prodinfo.bmp $@

$(EXEDIR)\pmrexxio.hlp: $(ALL_IPF)

$(OBJDIR)\pmrexxio.obj: pmrexxio.c  $(HEADERS)
   $(CC) -c $(CFLAGS) $(CINCLUDE) $(DLLCFLAGS) -Fo"$(OBJDIR)\$(@B).obj" $(@B).c

$(OBJDIR)\pmrexx.obj:   pmrexx.c    $(HEADERS)

$(OBJDIR)\rxhafile.obj: rxhafile.c  $(HEADERS)

$(OBJDIR)\pmrxsubs.obj: pmrxsubs.c  $(HEADERS)
   $(CC) -c $(CFLAGS) $(CINCLUDE) $(DLLCFLAGS) -Fo"$(OBJDIR)\$(@B).obj" $(@B).c

$(EXEDIR)\pmrexx.exe: $(EXE_OBJS) pmrexx.def $(OBJDIR)\pmrexx.res
   -$(CREATE_PATH)
   $(LINK) $@ pmrexx.def $(MTLIBS) + $(REXXLIBS) $(EXE_OBJS)
   $(RC) -p -x $(OBJDIR)\$(@B).res $@

$(EXEDIR)\pmrexxio.dll: $(DLL_OBJS) pmrexxio.def $(OBJDIR)\pmrexxio.res
   -$(CREATE_PATH)
   $(LINK) $@ $(DLLCFLAGS) pmrexxio.def $(MTLIBS) + $(REXXLIBS) $(DLL_OBJS)
   $(RC) -p -x $(OBJDIR)\$(@B).res $@

clean :
        @if exist *.obj del *.obj
        @if exist *.dll del *.dll
        @if exist *.exe del *.exe
