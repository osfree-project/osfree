#
# A Makefile for Unified API
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/mk/all.mk

OUT = $(%ROOT)build$(SEP)include
LIBOUT = $(%ROOT)build$(SEP)lib$(SEP)
#TARGETS = h

install: h .symbolic

outdirs = $(OUT) $(LIBOUT) $(OUT)$(SEP)dos $(OUT)$(SEP)os2 $(OUT)$(SEP)shared

h: pre workaround $(OUT)$(SEP)osfree.h os2 os2libs dos .symbolic

pre: .symbolic
 # @$(SAY) Creating directories...
 @for %i in ($(outdirs)) do @$(MDHIER) %i $(BLACKHOLE)

$(OUT)$(SEP)osfree.h: $(MYDIR)osfree.uni
# Main osFree file

workaround: $(MYDIR)cdeftypes2.h $(MYDIR)dosfilemgrcommon.h .symbolic
# Workaround of uniplemented features of uni2h tool
  @$(CP) $(MYDIR)cdeftypes2.h $(OUT) $(BLACKHOLE)
  @$(CP) $(MYDIR)dosfilemgrcommon.h $(OUT)$(SEP)os2 $(BLACKHOLE)

## OS/2 Personality files
os2: $(OUT)$(SEP)os2$(SEP)os2.h &
     $(OUT)$(SEP)os2$(SEP)os2def.h &
     $(OUT)$(SEP)os2$(SEP)bse.h &
     $(OUT)$(SEP)os2$(SEP)base.h &
     $(OUT)$(SEP)os2$(SEP)bseord.h &
     $(OUT)$(SEP)os2$(SEP)bsememf.h &
     $(OUT)$(SEP)os2$(SEP)bsedos.h &
     $(OUT)$(SEP)os2$(SEP)bsedev.h &
     $(OUT)$(SEP)os2$(SEP)hevdef.h &
     $(OUT)$(SEP)os2$(SEP)dos.h &
     $(OUT)$(SEP)os2$(SEP)dosdevices.h &
     $(OUT)$(SEP)os2$(SEP)bsesub.h &
     $(OUT)$(SEP)os2$(SEP)sub.h &
     $(OUT)$(SEP)os2$(SEP)vio.h &
     $(OUT)$(SEP)os2$(SEP)kbd.h &
     $(OUT)$(SEP)os2$(SEP)mou.h &
     $(OUT)$(SEP)os2$(SEP)win.h &
     $(OUT)$(SEP)os2$(SEP)winsei.h &
     $(OUT)$(SEP)os2$(SEP)bseerr.h &
     $(OUT)$(SEP)os2$(SEP)doserrors.h &
     $(OUT)$(SEP)os2$(SEP)bsexcpt.h &
     $(OUT)$(SEP)os2$(SEP)dosexcptfunc.h &
     $(OUT)$(SEP)os2$(SEP)basemid.h &
     $(OUT)$(SEP)os2$(SEP)bsetib.h &
     $(OUT)$(SEP)os2$(SEP)dosqueues.h &
     $(OUT)$(SEP)os2$(SEP)dosdebug.h &
     $(OUT)$(SEP)os2$(SEP)dosmisc.h &
     $(OUT)$(SEP)os2$(SEP)dosmvdm.h &
     $(OUT)$(SEP)os2$(SEP)dosnls.h &
     $(OUT)$(SEP)os2$(SEP)dosnmpipes.h &
     $(OUT)$(SEP)os2$(SEP)dosmodulemgr.h &
     $(OUT)$(SEP)os2$(SEP)dossesmgr.h &
     $(OUT)$(SEP)os2$(SEP)dosfilemgr.h &
#     $(OUT)$(SEP)os2$(SEP)dosfilemgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)dosmemmgr.h &
     $(OUT)$(SEP)os2$(SEP)dosmemmgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)dosmonitors.h &
     $(OUT)$(SEP)os2$(SEP)dosprocess.h &
     $(OUT)$(SEP)os2$(SEP)dosdevioctl.h &
     $(OUT)$(SEP)os2$(SEP)dosprofile.h &
     $(OUT)$(SEP)os2$(SEP)dossemaphores.h &
     $(OUT)$(SEP)os2$(SEP)dosprocesscommon.h &
     $(OUT)$(SEP)os2$(SEP)dosresources.h &
     $(OUT)$(SEP)os2$(SEP)dosresourcescommon.h &
     $(OUT)$(SEP)os2$(SEP)dosexceptions.h &
     $(OUT)$(SEP)os2$(SEP)dosdatetime.h &
     $(OUT)$(SEP)os2$(SEP)pm.h &
     $(OUT)$(SEP)os2$(SEP)pmdef.h &
     $(OUT)$(SEP)os2$(SEP)pmavio.h &
     $(OUT)$(SEP)os2$(SEP)pmbase.h &
     $(OUT)$(SEP)os2$(SEP)pmdev.h &
     $(OUT)$(SEP)os2$(SEP)devcommon.h &
     $(OUT)$(SEP)os2$(SEP)pmshl.h &
     $(OUT)$(SEP)os2$(SEP)winswitchlistcommon.h &
     $(OUT)$(SEP)os2$(SEP)winswitchlist.h &
     $(OUT)$(SEP)os2$(SEP)pmwp.h &
     $(OUT)$(SEP)os2$(SEP)pmwin.h &
     $(OUT)$(SEP)os2$(SEP)pmstddlg.h &
     $(OUT)$(SEP)os2$(SEP)winstdbook.h &
     $(OUT)$(SEP)os2$(SEP)winstddlgs.h &
     $(OUT)$(SEP)os2$(SEP)winstdfile.h &
     $(OUT)$(SEP)os2$(SEP)winstdslider.h &
     $(OUT)$(SEP)os2$(SEP)winstdspin.h &
     $(OUT)$(SEP)os2$(SEP)winstdvalset.h &
     $(OUT)$(SEP)os2$(SEP)winsys.h &
     $(OUT)$(SEP)os2$(SEP)winhprogram.h &
     $(OUT)$(SEP)os2$(SEP)wininput.h &
     $(OUT)$(SEP)os2$(SEP)wincircularslider.h &
     $(OUT)$(SEP)os2$(SEP)winstdcnr.h &
     $(OUT)$(SEP)os2$(SEP)winstdcontainer.h &
     $(OUT)$(SEP)os2$(SEP)winstdfont.h &
     $(OUT)$(SEP)os2$(SEP)wintrackrect.h &
     $(OUT)$(SEP)os2$(SEP)wininputcommon.h &
     $(OUT)$(SEP)shared$(SEP)rexxsaa.h &
     $(OUT)$(SEP)os2$(SEP)winatom.h &
     $(OUT)$(SEP)os2$(SEP)winthunkapi.h &
     $(OUT)$(SEP)os2$(SEP)winmenus.h &
     $(OUT)$(SEP)os2$(SEP)winaccelerators.h &
     $(OUT)$(SEP)os2$(SEP)winload.h &
     $(OUT)$(SEP)os2$(SEP)winstddrag.h &
     $(OUT)$(SEP)os2$(SEP)winclipboard.h &
     $(OUT)$(SEP)os2$(SEP)winmessagemgr.h &
     $(OUT)$(SEP)os2$(SEP)winmessagemgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)winwindowmgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)winwindowmgr.h &
     $(OUT)$(SEP)os2$(SEP)winclipboarddde.h &
     $(OUT)$(SEP)os2$(SEP)winprogramlist.h &
     $(OUT)$(SEP)os2$(SEP)winframectls.h &
     $(OUT)$(SEP)os2$(SEP)winframemgr.h &
     $(OUT)$(SEP)os2$(SEP)winframemgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)winshelldata.h &
     $(OUT)$(SEP)os2$(SEP)winerrors.h &
     $(OUT)$(SEP)os2$(SEP)shlerrors.h &
     $(OUT)$(SEP)os2$(SEP)pmerr.h &
     $(OUT)$(SEP)os2$(SEP)wperrors.h &
     $(OUT)$(SEP)os2$(SEP)gpierrors.h &
     $(OUT)$(SEP)os2$(SEP)splerrors.h &
     $(OUT)$(SEP)os2$(SEP)picerrors.h &
     $(OUT)$(SEP)os2$(SEP)winhooks.h &
     $(OUT)$(SEP)os2$(SEP)wintimer.h &
     $(OUT)$(SEP)os2$(SEP)wincountry.h &
     $(OUT)$(SEP)os2$(SEP)wincursors.h &
     $(OUT)$(SEP)os2$(SEP)windde.h &
     $(OUT)$(SEP)os2$(SEP)winstatics.h &
     $(OUT)$(SEP)os2$(SEP)winmle.h &
     $(OUT)$(SEP)os2$(SEP)wintypes.h &
     $(OUT)$(SEP)os2$(SEP)pmtypes.h &
     $(OUT)$(SEP)os2$(SEP)winpalette.h &
     $(OUT)$(SEP)os2$(SEP)winpointers.h &
     $(OUT)$(SEP)os2$(SEP)windesktop.h &
     $(OUT)$(SEP)os2$(SEP)winentryfields.h &
     $(OUT)$(SEP)os2$(SEP)windialogs.h &
     $(OUT)$(SEP)os2$(SEP)windialogscommon.h &
     $(OUT)$(SEP)os2$(SEP)winrectangles.h &
     $(OUT)$(SEP)os2$(SEP)winscrollbars.h &
     $(OUT)$(SEP)os2$(SEP)winbuttons.h &
     $(OUT)$(SEP)os2$(SEP)winlistboxes.h &
     $(OUT)$(SEP)os2$(SEP)pmhelp.h &
     $(OUT)$(SEP)os2$(SEP)pmsei.h &
#     $(OUT)$(SEP)os2$(SEP)winei.h &
     $(OUT)$(SEP)os2$(SEP)winhelp.h &
     $(OUT)$(SEP)os2$(SEP)pmgpi.h &
     $(OUT)$(SEP)os2$(SEP)gpiprimitives.h &
     $(OUT)$(SEP)os2$(SEP)gpiprimitives2.h &
     $(OUT)$(SEP)os2$(SEP)gpilcids.h &
     $(OUT)$(SEP)os2$(SEP)gpicontrolcommon.h &
     $(OUT)$(SEP)os2$(SEP)gpibitmapscommon.h &
     $(OUT)$(SEP)os2$(SEP)gpibitmaps.h &
     $(OUT)$(SEP)os2$(SEP)gpilogcoltable.h &
     $(OUT)$(SEP)os2$(SEP)gpipolygons.h &
     $(OUT)$(SEP)os2$(SEP)gpiregions.h &
     $(OUT)$(SEP)os2$(SEP)os2me.h &
     $(OUT)$(SEP)os2$(SEP)os2mm.h &
     $(OUT)$(SEP)os2$(SEP)meerror.h &
     $(OUT)$(SEP)os2$(SEP)udsk.h &
     $(OUT)$(SEP)os2$(SEP)unidef.h &
     $(OUT)$(SEP)os2$(SEP)ulserrno.h &
     $(OUT)$(SEP)os2$(SEP)errcmpat.h &
     $(OUT)$(SEP)os2$(SEP)ulsitem.h &
     $(OUT)$(SEP)os2$(SEP)uconv.h &
     $(OUT)$(SEP)os2$(SEP)stddef.h &
     $(OUT)$(SEP)os2$(SEP)time.h &
     $(OUT)$(SEP)shared$(SEP)utlapi.h &
     $(OUT)$(SEP)shared$(SEP)utlrectangles.h &
     .symbolic

#     $(OUT)$(SEP)os2$(SEP)bsexcpt.h &

os2libs: $(LIBOUT)sub32.lib &
         .symbolic

$(LIBOUT)sub32.lib: $(MYDIR)os2$(SEP)vio.uni
 @$(SAY) LIB      $^.
 @uni2h.exe -e def -a $(MYDIR)os2$(SEP)os2386.abi $< $^*.def
 @$(LIB) $(LIBOPT) $^@ @$^*.def
# @$(DC) $^*.def $(BLACKHOLE)

$(OUT)$(SEP)os2$(SEP)os2.h: $(MYDIR)os2$(SEP)os2.uni

### OS/2 Personality base type and macros definitions
$(OUT)$(SEP)os2$(SEP)os2def.h: $(MYDIR)os2$(SEP)os2def.uni

### OS/2 Personality base API
$(OUT)$(SEP)os2$(SEP)bse.h: $(MYDIR)os2$(SEP)cpi$(SEP)bse.uni
$(OUT)$(SEP)os2$(SEP)base.h: $(MYDIR)os2$(SEP)cpi$(SEP)base.uni

### OS/2 Personality base DOS API ordinals
$(OUT)$(SEP)os2$(SEP)bsedos.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsedos.uni

### OS/2 Personality base error codes and messages
$(OUT)$(SEP)os2$(SEP)bseerr.h: $(MYDIR)os2$(SEP)cpi$(SEP)bseerr.uni

### OS/2 Personality base API ordinals
$(OUT)$(SEP)os2$(SEP)bseord.h: $(MYDIR)os2$(SEP)cpi$(SEP)bseord.uni

### OS/2 Personality memory API flags
$(OUT)$(SEP)os2$(SEP)bsememf.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsememf.uni

$(OUT)$(SEP)os2$(SEP)bsedev.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsedev.uni

$(OUT)$(SEP)os2$(SEP)bsetib.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsetib.uni

$(OUT)$(SEP)os2$(SEP)bsexcpt.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsexcpt.uni

$(OUT)$(SEP)os2$(SEP)bsesub.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsesub.uni

$(OUT)$(SEP)os2$(SEP)sub.h: $(MYDIR)os2$(SEP)sub.uni

$(OUT)$(SEP)os2$(SEP)vio.h: $(MYDIR)os2$(SEP)vio.uni

$(OUT)$(SEP)os2$(SEP)kbd.h: $(MYDIR)os2$(SEP)kbd.uni

$(OUT)$(SEP)os2$(SEP)mou.h: $(MYDIR)os2$(SEP)mou.uni

$(OUT)$(SEP)os2$(SEP)dos.h: $(MYDIR)os2$(SEP)cpi$(SEP)dos.uni

$(OUT)$(SEP)os2$(SEP)win.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)win.uni

$(OUT)$(SEP)os2$(SEP)dosprocesscommon.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosprocesscommon.uni

$(OUT)$(SEP)os2$(SEP)dosprocess.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosprocess.uni

$(OUT)$(SEP)os2$(SEP)dosprofile.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosprofile.uni

$(OUT)$(SEP)os2$(SEP)dosmisc.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmisc.uni

$(OUT)$(SEP)os2$(SEP)basemid.h: $(MYDIR)os2$(SEP)cpi$(SEP)basemid.uni

$(OUT)$(SEP)os2$(SEP)dosnmpipes.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosnmpipes.uni

$(OUT)$(SEP)os2$(SEP)dosmvdm.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmvdm.uni

$(OUT)$(SEP)os2$(SEP)dossesmgr.h: $(MYDIR)os2$(SEP)cpi$(SEP)dossesmgr.uni

$(OUT)$(SEP)os2$(SEP)dosnls.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosnls.uni

$(OUT)$(SEP)os2$(SEP)dosdevioctl.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosdevioctl.uni

$(OUT)$(SEP)os2$(SEP)dosqueues.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosqueues.uni

$(OUT)$(SEP)os2$(SEP)doserrors.h: $(MYDIR)os2$(SEP)cpi$(SEP)doserrors.uni

$(OUT)$(SEP)os2$(SEP)dosmodulemgr.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmodulemgr.uni

$(OUT)$(SEP)os2$(SEP)dosfilemgr.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosfilemgr.uni

$(OUT)$(SEP)os2$(SEP)dossemaphores.h: $(MYDIR)os2$(SEP)cpi$(SEP)dossemaphores.uni

$(OUT)$(SEP)os2$(SEP)dosdatetime.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosdatetime.uni

$(OUT)$(SEP)os2$(SEP)dosdevices.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosdevices.uni

$(OUT)$(SEP)os2$(SEP)dosfilemgrcommon.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosfilemgrcommon.uni

$(OUT)$(SEP)os2$(SEP)dosmemmgr.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmemmgr.uni

$(OUT)$(SEP)os2$(SEP)dosmonitors.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmonitors.uni

$(OUT)$(SEP)os2$(SEP)dosmemmgrcommon.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmemmgrcommon.uni

$(OUT)$(SEP)os2$(SEP)dosexceptions.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosexceptions.uni

$(OUT)$(SEP)os2$(SEP)dosexcptfunc.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosexcptfunc.uni

$(OUT)$(SEP)os2$(SEP)dosresources.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosresources.uni

$(OUT)$(SEP)os2$(SEP)dosresourcescommon.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosresourcescommon.uni

$(OUT)$(SEP)os2$(SEP)dosdebug.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosdebug.uni

$(OUT)$(SEP)os2$(SEP)hevdef.h: $(MYDIR)os2$(SEP)hevdef.uni

$(OUT)$(SEP)os2$(SEP)pm.h: $(MYDIR)os2$(SEP)pm$(SEP)pm.uni

$(OUT)$(SEP)os2$(SEP)pmbase.h: $(MYDIR)os2$(SEP)pm$(SEP)pmbase.uni

$(OUT)$(SEP)os2$(SEP)pmdef.h: $(MYDIR)os2$(SEP)pm$(SEP)pmdef.uni

$(OUT)$(SEP)os2$(SEP)pmshl.h: $(MYDIR)os2$(SEP)pm$(SEP)pmshl.uni

$(OUT)$(SEP)os2$(SEP)pmstddlg.h: $(MYDIR)os2$(SEP)pm$(SEP)pmstddlg.uni

$(OUT)$(SEP)os2$(SEP)pmhelp.h: $(MYDIR)os2$(SEP)pm$(SEP)pmhelp.uni

$(OUT)$(SEP)os2$(SEP)winhelp.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winhelp.uni

$(OUT)$(SEP)os2$(SEP)pmtypes.h: $(MYDIR)os2$(SEP)pm$(SEP)pmtypes.uni

$(OUT)$(SEP)os2$(SEP)wintypes.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wintypes.uni

$(OUT)$(SEP)os2$(SEP)winstddrag.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstddrag.uni

$(OUT)$(SEP)os2$(SEP)wincircularslider.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wincircularslider.uni

$(OUT)$(SEP)os2$(SEP)winstdcnr.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdcnr.uni

$(OUT)$(SEP)os2$(SEP)winstdcontainer.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdcontainer.uni

$(OUT)$(SEP)os2$(SEP)winstatics.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstatics.uni

$(OUT)$(SEP)os2$(SEP)winrectangles.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winrectangles.uni

$(OUT)$(SEP)os2$(SEP)winscrollbars.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winscrollbars.uni

$(OUT)$(SEP)os2$(SEP)winstdbook.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdbook.uni

$(OUT)$(SEP)os2$(SEP)winstdfont.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdfont.uni

$(OUT)$(SEP)os2$(SEP)winstdvalset.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdvalset.uni

$(OUT)$(SEP)os2$(SEP)winstdspin.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdspin.uni

$(OUT)$(SEP)os2$(SEP)winstdslider.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdslider.uni

$(OUT)$(SEP)os2$(SEP)winsei.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winsei.uni

$(OUT)$(SEP)os2$(SEP)pmsei.h: $(MYDIR)os2$(SEP)pm$(SEP)pmsei.uni

$(OUT)$(SEP)os2$(SEP)winswitchlist.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winswitchlist.uni

$(OUT)$(SEP)os2$(SEP)winswitchlistcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winswitchlistcommon.uni

$(OUT)$(SEP)os2$(SEP)winframemgr.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winframemgr.uni

$(OUT)$(SEP)os2$(SEP)winframectls.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winframectls.uni

$(OUT)$(SEP)os2$(SEP)winload.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winload.uni

$(OUT)$(SEP)os2$(SEP)winpalette.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winpalette.uni

$(OUT)$(SEP)os2$(SEP)winstddlgs.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstddlgs.uni

$(OUT)$(SEP)os2$(SEP)winstdfile.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdfile.uni

$(OUT)$(SEP)os2$(SEP)winatom.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winatom.uni

$(OUT)$(SEP)os2$(SEP)wincursors.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wincursors.uni

$(OUT)$(SEP)os2$(SEP)wintrackrect.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wintrackrect.uni

$(OUT)$(SEP)os2$(SEP)winpointers.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winpointers.uni

$(OUT)$(SEP)os2$(SEP)winframemgrcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winframemgrcommon.uni

$(OUT)$(SEP)os2$(SEP)pmwp.h: $(MYDIR)os2$(SEP)pm$(SEP)pmwp.uni

$(OUT)$(SEP)os2$(SEP)pmwin.h: $(MYDIR)os2$(SEP)pm$(SEP)pmwin.uni

$(OUT)$(SEP)shared$(SEP)rexxsaa.h: $(MYDIR)shared$(SEP)rexxsaa.uni

$(OUT)$(SEP)os2$(SEP)winprogramlist.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winprogramlist.uni

$(OUT)$(SEP)os2$(SEP)winsys.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winsys.uni

$(OUT)$(SEP)os2$(SEP)windde.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)windde.uni

$(OUT)$(SEP)os2$(SEP)winmle.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winmle.uni

$(OUT)$(SEP)os2$(SEP)windesktop.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)windesktop.uni

$(OUT)$(SEP)os2$(SEP)winwindowmgrcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winwindowmgrcommon.uni

$(OUT)$(SEP)os2$(SEP)winwindowmgr.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winwindowmgr.uni

$(OUT)$(SEP)os2$(SEP)winmessagemgrcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winmessagemgrcommon.uni

$(OUT)$(SEP)os2$(SEP)winmessagemgr.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winmessagemgr.uni

$(OUT)$(SEP)os2$(SEP)winclipboard.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winclipboard.uni

$(OUT)$(SEP)os2$(SEP)winclipboarddde.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winclipboarddde.uni

$(OUT)$(SEP)os2$(SEP)wininput.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wininput.uni

$(OUT)$(SEP)os2$(SEP)winaccelerators.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winaccelerators.uni

$(OUT)$(SEP)os2$(SEP)wininputcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wininputcommon.uni

$(OUT)$(SEP)os2$(SEP)winshelldata.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winshelldata.uni

$(OUT)$(SEP)os2$(SEP)winmenus.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winmenus.uni

$(OUT)$(SEP)os2$(SEP)winerrors.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winerrors.uni

$(OUT)$(SEP)os2$(SEP)shlerrors.h: $(MYDIR)os2$(SEP)shlerrors.uni

$(OUT)$(SEP)os2$(SEP)pmerr.h: $(MYDIR)os2$(SEP)pm$(SEP)pmerr.uni

$(OUT)$(SEP)os2$(SEP)wperrors.h: $(MYDIR)os2$(SEP)wperrors.uni

$(OUT)$(SEP)os2$(SEP)gpierrors.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpierrors.uni

$(OUT)$(SEP)os2$(SEP)splerrors.h: $(MYDIR)os2$(SEP)splerrors.uni

$(OUT)$(SEP)os2$(SEP)picerrors.h: $(MYDIR)os2$(SEP)picerrors.uni

$(OUT)$(SEP)os2$(SEP)winhooks.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winhooks.uni

$(OUT)$(SEP)os2$(SEP)wintimer.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wintimer.uni

$(OUT)$(SEP)os2$(SEP)winthunkapi.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winthunkapi.uni

$(OUT)$(SEP)os2$(SEP)winhprogram.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winhprogram.uni

$(OUT)$(SEP)os2$(SEP)wincountry.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wincountry.uni

$(OUT)$(SEP)os2$(SEP)windialogscommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)windialogscommon.uni

$(OUT)$(SEP)os2$(SEP)windialogs.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)windialogs.uni

$(OUT)$(SEP)os2$(SEP)winentryfields.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winentryfields.uni

$(OUT)$(SEP)os2$(SEP)winbuttons.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winbuttons.uni

$(OUT)$(SEP)os2$(SEP)winlistboxes.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winlistboxes.uni

$(OUT)$(SEP)os2$(SEP)pmgpi.h: $(MYDIR)os2$(SEP)pm$(SEP)pmgpi.uni

$(OUT)$(SEP)os2$(SEP)pmdev.h: $(MYDIR)os2$(SEP)pm$(SEP)pmdev.uni

$(OUT)$(SEP)os2$(SEP)devcommon.h: $(MYDIR)os2$(SEP)cpi$(SEP)devcommon.uni

$(OUT)$(SEP)os2$(SEP)gpiprimitives.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpiprimitives.uni

$(OUT)$(SEP)os2$(SEP)gpilcids.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpilcids.uni

$(OUT)$(SEP)os2$(SEP)gpibitmapscommon.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpibitmapscommon.uni

$(OUT)$(SEP)os2$(SEP)gpibitmaps.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpibitmaps.uni

$(OUT)$(SEP)os2$(SEP)gpicontrolcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpicontrolcommon.uni

$(OUT)$(SEP)os2$(SEP)gpilogcoltable.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpilogcoltable.uni

$(OUT)$(SEP)os2$(SEP)gpipolygons.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpipolygons.uni

$(OUT)$(SEP)os2$(SEP)gpiregions.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpiregions.uni

$(OUT)$(SEP)dos$(SEP)os2me.h: $(MYDIR)dos$(SEP)os2me.uni

$(OUT)$(SEP)dos$(SEP)os2mm.h: $(MYDIR)dos$(SEP)os2mm.uni

$(OUT)$(SEP)dos$(SEP)meerror.h: $(MYDIR)dos$(SEP)meerror.uni

$(OUT)$(SEP)os2$(SEP)udsk.h: $(MYDIR)os2$(SEP)udsk.uni

$(OUT)$(SEP)os2$(SEP)ulserrno.h: $(MYDIR)os2$(SEP)ulserrno.uni

$(OUT)$(SEP)os2$(SEP)errcmpat.h: $(MYDIR)os2$(SEP)errcmpat.uni

$(OUT)$(SEP)os2$(SEP)ulsitem.h: $(MYDIR)os2$(SEP)ulsitem.uni

$(OUT)$(SEP)os2$(SEP)unidef.h: $(MYDIR)os2$(SEP)unidef.uni

$(OUT)$(SEP)os2$(SEP)uconv.h: $(MYDIR)os2$(SEP)uconv.uni

$(OUT)$(SEP)shared$(SEP)utlapi.h: $(MYDIR)shared$(SEP)utlapi.uni

$(OUT)$(SEP)shared$(SEP)utlrectangles.h: $(MYDIR)shared$(SEP)utlapi$(SEP)utlrectangles.uni

## POSIX API

$(OUT)$(SEP)os2$(SEP)stddef.h: $(MYDIR)os2$(SEP)stddef.uni

$(OUT)$(SEP)os2$(SEP)time.h: $(MYDIR)os2$(SEP)time.uni


## DOS Personality files
dos: $(OUT)$(SEP)dos$(SEP)dos.h .symbolic # $(OUT)$(SEP)dos$(SEP)os2vdm.h

$(OUT)$(SEP)dos$(SEP)dos.h: $(MYDIR)dos$(SEP)dos.uni

$(OUT)$(SEP)dos$(SEP)os2vdm.h: $(MYDIR)dos$(SEP)os2vdm.uni

.SUFFIXES:
.SUFFIXES: .lib .h .uni

.uni: $(MYDIR)shared
.uni: $(MYDIR)shared$(SEP)utlapi
.uni: $(MYDIR)os2
.uni: $(MYDIR)os2$(SEP)cpi
.uni: $(MYDIR)os2$(SEP)pm
.uni: $(MYDIR)os2$(SEP)pm$(SEP)gpi
.uni: $(MYDIR)os2$(SEP)pm$(SEP)win
.uni: $(MYDIR)dos

.lib: $(PATH)

.uni.h: .AUTODEPEND
 @$(SAY) UNI2H    $^. $(LOG)
 $(verbose)uni2h.exe -e h -a $(MYDIR)os2$(SEP)os2386.abi $< $^@ $(LOG2)

.uni.lib: .AUTODEPEND
 @$(SAY) LIB      $^. $(LOG)
 $(verbose)uni2h.exe -e def -a $(MYDIR)os2$(SEP)os2386.abi $< $^*.def $(LOG2)
# $(verbose)$(LIB) $(LIBOPT) $^@ @$^*.def
