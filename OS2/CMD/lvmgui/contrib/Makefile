# $Id$
# LVMPM makefile for ICC + NMAKE32
#
# To compile English version simply run 'nmake32'.  For other supported
# languages, first set environment variable NLV to desired country code.
# (Always run 'nmake32 nlvclean' before building a different language.)
#
# NOTE: This build requires helpers.lib (XWPHelpers); if you have the
# XWPHelpers CVS tree installed it can be built using 'nmake32 xwphelpers'
# (make sure to edit config.in as appropriate first).  If not, a prebuilt
# copy should be provided here.
#

!include local.inc

CC      = icc.exe
LINK    = ilink.exe
RC      = rc.exe
IPFC    = ipfc.exe
MKDESC  = $(MAKEDIR)/make/makedesc.cmd

CFLAGS  = /Gm /Q /Ss /Sp /Wuse /Wpar
LFLAGS  = /NOE /PMTYPE:PM /NOLOGO
OBJS    = lvmpm.obj lvm_ctls.obj logging.obj disk.obj partition.obj volume.obj utils.obj airboot.obj bootmgr.obj
LIBS    = lvm.lib helpers.lib
NAME    = lvmpm
MRI     = lvmpmmri

# National language version to be built
LANGDIR = 001
!ifdef NLV
    LANGDIR  = $(NLV)
!endif
# RC.EXE won't handle Taiwanese DBCS lead bytes without this option:
!if "$(NLV)" == "088"
    RFLAGS = $(RFLAGS) -cp 950
!endif

# Set environment variable DEBUG to build with debugging symbols
!ifdef DEBUG
    CFLAGS = $(CFLAGS) /Ti+ /Tm+
    LFLAGS = $(LFLAGS) /DEBUG
!endif

!ifdef LVM_TK
    CFLAGS = $(CFLAGS) /I:$(LVM_TK)\include
    LFLAGS = $(LFLAGS) /INC:$(LVM_TK)\lib
!endif

# Definitions required for xwphelpers
!include config.in
LVMPM_BASE  = $(MAKEDIR)

all                  : $(NAME).exe $(MRI).dll $(NAME).hlp

$(NAME).exe          : $(OBJS) $(NAME).res
                        $(MKDESC) -D"Logical Volume Manager PM" -N"Alexander Taylor" -V"^#define=SZ_VERSION,lvmpm.h" $(NAME).def
                        $(LINK) $(LFLAGS) /MAP $(OBJS) $(LIBS) $(NAME).def /OUT:$@
                        $(RC) $(NAME).res $@


lvmpm.obj            : lvmpm.h lvmcalls.h lvm_ctls.h ids.h

lvm_ctls.obj         : lvm_ctls.h ids.h

utils.obj            : lvmpm.h ids.h

logging.obj          : lvmpm.h ids.h

airboot.obj          : lvmpm.h lvmcalls.h ids.h

bootmgr.obj          : lvmpm.h lvmcalls.h ids.h

disk.obj             : lvmpm.h lvmcalls.h lvm_ctls.h ids.h

partition.obj        : lvmpm.h lvmcalls.h lvm_ctls.h ids.h

volume.obj           : lvmpm.h lvmcalls.h lvm_ctls.h ids.h

$(NAME).res          : $(NAME).rc ids.h
                        $(RC) -r $(NAME).rc

$(NAME).hlp          : {$(LANGDIR)}$(NAME).ipf {$(LANGDIR)}errors.ipf {$(LANGDIR)}license.ipf
                        $(IPFC) -d:$(LANGDIR) $< $@

$(MRI).dll           : $(MRI).obj {$(LANGDIR)}$(MRI).res
                        $(MKDESC) -D"LVMPM language resources $(LANGDIR)" -N"Alexander Taylor" -V"^#define=SZ_VERSION,lvmpm.h" $(MRI).def
                        $(LINK) $(LFLAGS) /DLL $(MRI).def $< /OUT:$@
                        $(RC) -x2 $(LANGDIR)\$(MRI).res $@

$(MRI).obj           : $(MRI).c $(MRI).def
                        $(CC) $(CFLAGS) /Ge- $**

$(LANGDIR)\$(MRI).res: {$(LANGDIR)}$(MRI).rc {$(LANGDIR)}$(MRI).dlg ids.h
                        %cd $(LANGDIR)
                        $(RC) $(RFLAGS) -i .. -r $(MRI).rc
                        %cd ..

# This builds helpers.lib, which requires the XWPHelpers source tree.  We use
# a pseudotarget here to prevent NMAKE32 from trying to build it automatically.
xwphelpers            :
                        @setlocal
                        @ $[c,$(XWPHLP_BASE),1,2]
                        %cd $(XWPHLP_BASE)
                        @SET INCLUDE=$(XWPHLP_BASE)\include;$(LVMPM_BASE)\include;$(ACPI_TK)\h;$(INCLUDE)
                        @nmake32 -nologo "PROJECT_BASE_DIR=$(LVMPM_BASE)" "MAINMAKERUNNING=YES"
                        @ $[c,$(LVMPM_BASE),1,2]
                        %cd $(LVMPM_BASE)
                        @del *.obj
                        @endlocal

# Delete only language-dependent binaries
nlvclean              :
                        rm -f $(MRI).dll $(LANGDIR)\$(MRI).res $(NAME).hlp

# Delete all binaries (other than the .lib files)
clean                 : nlvclean
                        rm -f *.obj $(NAME).exe $(NAME).res

