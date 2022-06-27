#
# A Makefile for Unified API
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/mk/all.mk

OUT = $(%ROOT)build$(SEP)include$(SEP)
LIBOUT = $(%ROOT)build$(SEP)lib$(SEP)

OS2TKH=$(%OS2TK)$(SEP)h$(SEP)

install: h .symbolic

outdirs = $(OUT) $(LIBOUT) $(OUT)$(SEP)dos $(OS2TKH) $(OUT)$(SEP)shared

h: pre workaround $(OUT)$(SEP)osfree.h os2 os2libs dos .symbolic

pre: .symbolic
 # @$(SAY) Creating directories...
 @for %i in ($(outdirs)) do @$(MDHIER) %i $(BLACKHOLE)

$(OUT)$(SEP)osfree.h: $(MYDIR)osfree.uni
# Main osFree file

workaround: $(MYDIR)cdeftypes2.h $(MYDIR)dosfilemgrcommon.h $(MYDIR)exe386.h $(MYDIR)newexe.h .symbolic
# Workaround of uniplemented features of uni2h tool
  @$(CP) $(MYDIR)cdeftypes2.h $(OS2TKH) $(BLACKHOLE)
  @$(CP) $(MYDIR)newexe.h $(OS2TKH) $(BLACKHOLE)
  @$(CP) $(MYDIR)exe386.h $(OS2TKH) $(BLACKHOLE)
  @$(CP) $(MYDIR)dosfilemgrcommon.h $(OS2TKH) $(BLACKHOLE)

## OS/2 Personality files
os2: $(OS2TKH)os2.h &
     $(OS2TKH)$(SEP)os2def.h &
     $(OS2TKH)$(SEP)bse.h &
     $(OS2TKH)$(SEP)base.h &
     $(OS2TKH)$(SEP)bseord.h &
     $(OS2TKH)$(SEP)bsememf.h &
     $(OS2TKH)$(SEP)bsedos.h &
     $(OS2TKH)$(SEP)bsedev.h &
     $(OS2TKH)$(SEP)hevdef.h &
     $(OS2TKH)$(SEP)dos.h &
     $(OS2TKH)$(SEP)dosdevices.h &
     $(OS2TKH)$(SEP)bsesub.h &
     $(OS2TKH)$(SEP)sub.h &
     $(OS2TKH)$(SEP)vio.h &
     $(OS2TKH)$(SEP)kbd.h &
     $(OS2TKH)$(SEP)mou.h &
     $(OS2TKH)$(SEP)win.h &
     $(OS2TKH)$(SEP)winsei.h &
     $(OS2TKH)$(SEP)bseerr.h &
     $(OS2TKH)$(SEP)doserrors.h &
     $(OS2TKH)$(SEP)bsexcpt.h &
     $(OS2TKH)$(SEP)dosexcptfunc.h &
     $(OS2TKH)$(SEP)basemid.h &
     $(OS2TKH)$(SEP)bsetib.h &
     $(OS2TKH)$(SEP)dosqueues.h &
     $(OS2TKH)$(SEP)dosdebug.h &
     $(OS2TKH)$(SEP)dosmisc.h &
     $(OS2TKH)$(SEP)dosmvdm.h &
     $(OS2TKH)$(SEP)dosnls.h &
     $(OS2TKH)$(SEP)dosnmpipes.h &
     $(OS2TKH)$(SEP)dosmodulemgr.h &
     $(OS2TKH)$(SEP)dossesmgr.h &
     $(OS2TKH)$(SEP)dosfilemgr.h &
#     $(OS2TKH)$(SEP)dosfilemgrcommon.h &
     $(OS2TKH)$(SEP)dosmemmgr.h &
     $(OS2TKH)$(SEP)dosmemmgrcommon.h &
     $(OS2TKH)$(SEP)dosmonitors.h &
     $(OS2TKH)$(SEP)dosprocess.h &
     $(OS2TKH)$(SEP)dosdevioctl.h &
     $(OS2TKH)$(SEP)dosprofile.h &
     $(OS2TKH)$(SEP)dossemaphores.h &
     $(OS2TKH)$(SEP)dosprocesscommon.h &
     $(OS2TKH)$(SEP)dosresources.h &
     $(OS2TKH)$(SEP)dosresourcescommon.h &
     $(OS2TKH)$(SEP)dosexceptions.h &
     $(OS2TKH)$(SEP)dosdatetime.h &
     $(OS2TKH)$(SEP)pm.h &
     $(OS2TKH)$(SEP)pmdef.h &
     $(OS2TKH)$(SEP)pmavio.h &
     $(OS2TKH)$(SEP)pmbase.h &
     $(OS2TKH)$(SEP)pmdev.h &
     $(OS2TKH)$(SEP)devcommon.h &
     $(OS2TKH)$(SEP)pmshl.h &
     $(OS2TKH)$(SEP)winswitchlistcommon.h &
     $(OS2TKH)$(SEP)winswitchlist.h &
     $(OS2TKH)$(SEP)pmwp.h &
     $(OS2TKH)$(SEP)pmwin.h &
     $(OS2TKH)$(SEP)pmstddlg.h &
     $(OS2TKH)$(SEP)winstdbook.h &
     $(OS2TKH)$(SEP)winstddlgs.h &
     $(OS2TKH)$(SEP)winstdfile.h &
     $(OS2TKH)$(SEP)winstdslider.h &
     $(OS2TKH)$(SEP)winstdspin.h &
     $(OS2TKH)$(SEP)winstdvalset.h &
     $(OS2TKH)$(SEP)winsys.h &
     $(OS2TKH)$(SEP)winhprogram.h &
     $(OS2TKH)$(SEP)wininput.h &
     $(OS2TKH)$(SEP)wincircularslider.h &
     $(OS2TKH)$(SEP)winstdcnr.h &
     $(OS2TKH)$(SEP)winstdcontainer.h &
     $(OS2TKH)$(SEP)winstdfont.h &
     $(OS2TKH)$(SEP)wintrackrect.h &
     $(OS2TKH)$(SEP)wininputcommon.h &
     $(OUT)$(SEP)shared$(SEP)rexxsaa.h &
     $(OS2TKH)$(SEP)winatom.h &
     $(OS2TKH)$(SEP)winthunkapi.h &
     $(OS2TKH)$(SEP)winmenus.h &
     $(OS2TKH)$(SEP)winaccelerators.h &
     $(OS2TKH)$(SEP)winload.h &
     $(OS2TKH)$(SEP)winstddrag.h &
     $(OS2TKH)$(SEP)winclipboard.h &
     $(OS2TKH)$(SEP)winmessagemgr.h &
     $(OS2TKH)$(SEP)winmessagemgrcommon.h &
     $(OS2TKH)$(SEP)winwindowmgrcommon.h &
     $(OS2TKH)$(SEP)winwindowmgr.h &
     $(OS2TKH)$(SEP)winclipboarddde.h &
     $(OS2TKH)$(SEP)winprogramlist.h &
     $(OS2TKH)$(SEP)winframectls.h &
     $(OS2TKH)$(SEP)winframemgr.h &
     $(OS2TKH)$(SEP)winframemgrcommon.h &
     $(OS2TKH)$(SEP)winshelldata.h &
     $(OS2TKH)$(SEP)winerrors.h &
     $(OS2TKH)$(SEP)shlerrors.h &
     $(OS2TKH)$(SEP)pmerr.h &
     $(OS2TKH)$(SEP)wperrors.h &
     $(OS2TKH)$(SEP)gpierrors.h &
     $(OS2TKH)$(SEP)splerrors.h &
     $(OS2TKH)$(SEP)picerrors.h &
     $(OS2TKH)$(SEP)winhooks.h &
     $(OS2TKH)$(SEP)wintimer.h &
     $(OS2TKH)$(SEP)wincountry.h &
     $(OS2TKH)$(SEP)wincursors.h &
     $(OS2TKH)$(SEP)windde.h &
     $(OS2TKH)$(SEP)winstatics.h &
     $(OS2TKH)$(SEP)winmle.h &
     $(OS2TKH)$(SEP)wintypes.h &
     $(OS2TKH)$(SEP)pmtypes.h &
     $(OS2TKH)$(SEP)winpalette.h &
     $(OS2TKH)$(SEP)winpointers.h &
     $(OS2TKH)$(SEP)windesktop.h &
     $(OS2TKH)$(SEP)winentryfields.h &
     $(OS2TKH)$(SEP)windialogs.h &
     $(OS2TKH)$(SEP)windialogscommon.h &
     $(OS2TKH)$(SEP)winrectangles.h &
     $(OS2TKH)$(SEP)winscrollbars.h &
     $(OS2TKH)$(SEP)winbuttons.h &
     $(OS2TKH)$(SEP)winlistboxes.h &
     $(OS2TKH)$(SEP)pmhelp.h &
     $(OS2TKH)$(SEP)pmsei.h &
#     $(OS2TKH)$(SEP)winei.h &
     $(OS2TKH)$(SEP)winhelp.h &
     $(OS2TKH)$(SEP)pmgpi.h &
     $(OS2TKH)$(SEP)gpiprimitives.h &
     $(OS2TKH)$(SEP)gpiprimitives2.h &
     $(OS2TKH)$(SEP)gpilcids.h &
     $(OS2TKH)$(SEP)gpicontrolcommon.h &
     $(OS2TKH)$(SEP)gpibitmapscommon.h &
     $(OS2TKH)$(SEP)gpibitmaps.h &
     $(OS2TKH)$(SEP)gpilogcoltable.h &
     $(OS2TKH)$(SEP)gpipolygons.h &
     $(OS2TKH)$(SEP)gpiregions.h &
     $(OS2TKH)$(SEP)os2me.h &
     $(OS2TKH)$(SEP)os2mm.h &
     $(OS2TKH)$(SEP)meerror.h &
     $(OS2TKH)$(SEP)udsk.h &
     $(OS2TKH)$(SEP)unidef.h &
     $(OS2TKH)$(SEP)ulserrno.h &
     $(OS2TKH)$(SEP)errcmpat.h &
     $(OS2TKH)$(SEP)ulsitem.h &
     $(OS2TKH)$(SEP)uconv.h &
     $(OS2TKH)$(SEP)stddef.h &
     $(OS2TKH)$(SEP)time.h &
     $(OS2TKH)$(SEP)utlapi.h &
     $(OS2TKH)$(SEP)utlrectangles.h &
     .symbolic

#     $(OS2TKH)$(SEP)bsexcpt.h &

os2libs: $(LIBOUT)sub32.lib &
         .symbolic

$(LIBOUT)sub32.lib: $(MYDIR)os2$(SEP)vio.uni
 @$(SAY) LIB      $^.
 @uni2h.exe -e def -a $(MYDIR)os2$(SEP)os2386.abi $< $^*.def
 @$(LIB) $(LIBOPT) $^@ @$^*.def
# @$(DC) $^*.def $(BLACKHOLE)

$(OS2TKH)$(SEP)os2.h: $(MYDIR)os2$(SEP)os2.uni

### OS/2 Personality base type and macros definitions
$(OS2TKH)$(SEP)os2def.h: $(MYDIR)os2$(SEP)os2def.uni

### OS/2 Personality base API
$(OS2TKH)$(SEP)bse.h: $(MYDIR)os2$(SEP)cpi$(SEP)bse.uni
$(OS2TKH)$(SEP)base.h: $(MYDIR)os2$(SEP)cpi$(SEP)base.uni

### OS/2 Personality base DOS API ordinals
$(OS2TKH)$(SEP)bsedos.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsedos.uni

### OS/2 Personality base error codes and messages
$(OS2TKH)$(SEP)bseerr.h: $(MYDIR)os2$(SEP)cpi$(SEP)bseerr.uni

### OS/2 Personality base API ordinals
$(OS2TKH)$(SEP)bseord.h: $(MYDIR)os2$(SEP)cpi$(SEP)bseord.uni

### OS/2 Personality memory API flags
$(OS2TKH)$(SEP)bsememf.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsememf.uni

$(OS2TKH)$(SEP)bsedev.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsedev.uni

$(OS2TKH)$(SEP)bsetib.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsetib.uni

$(OS2TKH)$(SEP)bsexcpt.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsexcpt.uni

$(OS2TKH)$(SEP)bsesub.h: $(MYDIR)os2$(SEP)cpi$(SEP)bsesub.uni

$(OS2TKH)$(SEP)sub.h: $(MYDIR)os2$(SEP)sub.uni

$(OS2TKH)$(SEP)vio.h: $(MYDIR)os2$(SEP)vio.uni

$(OS2TKH)$(SEP)kbd.h: $(MYDIR)os2$(SEP)kbd.uni

$(OS2TKH)$(SEP)mou.h: $(MYDIR)os2$(SEP)mou.uni

$(OS2TKH)$(SEP)dos.h: $(MYDIR)os2$(SEP)cpi$(SEP)dos.uni

$(OS2TKH)$(SEP)win.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)win.uni

$(OS2TKH)$(SEP)dosprocesscommon.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosprocesscommon.uni

$(OS2TKH)$(SEP)dosprocess.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosprocess.uni

$(OS2TKH)$(SEP)dosprofile.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosprofile.uni

$(OS2TKH)$(SEP)dosmisc.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmisc.uni

$(OS2TKH)$(SEP)basemid.h: $(MYDIR)os2$(SEP)cpi$(SEP)basemid.uni

$(OS2TKH)$(SEP)dosnmpipes.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosnmpipes.uni

$(OS2TKH)$(SEP)dosmvdm.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmvdm.uni

$(OS2TKH)$(SEP)dossesmgr.h: $(MYDIR)os2$(SEP)cpi$(SEP)dossesmgr.uni

$(OS2TKH)$(SEP)dosnls.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosnls.uni

$(OS2TKH)$(SEP)dosdevioctl.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosdevioctl.uni

$(OS2TKH)$(SEP)dosqueues.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosqueues.uni

$(OS2TKH)$(SEP)doserrors.h: $(MYDIR)os2$(SEP)cpi$(SEP)doserrors.uni

$(OS2TKH)$(SEP)dosmodulemgr.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmodulemgr.uni

$(OS2TKH)$(SEP)dosfilemgr.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosfilemgr.uni

$(OS2TKH)$(SEP)dossemaphores.h: $(MYDIR)os2$(SEP)cpi$(SEP)dossemaphores.uni

$(OS2TKH)$(SEP)dosdatetime.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosdatetime.uni

$(OS2TKH)$(SEP)dosdevices.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosdevices.uni

$(OS2TKH)$(SEP)dosfilemgrcommon.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosfilemgrcommon.uni

$(OS2TKH)$(SEP)dosmemmgr.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmemmgr.uni

$(OS2TKH)$(SEP)dosmonitors.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmonitors.uni

$(OS2TKH)$(SEP)dosmemmgrcommon.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosmemmgrcommon.uni

$(OS2TKH)$(SEP)dosexceptions.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosexceptions.uni

$(OS2TKH)$(SEP)dosexcptfunc.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosexcptfunc.uni

$(OS2TKH)$(SEP)dosresources.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosresources.uni

$(OS2TKH)$(SEP)dosresourcescommon.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosresourcescommon.uni

$(OS2TKH)$(SEP)dosdebug.h: $(MYDIR)os2$(SEP)cpi$(SEP)dosdebug.uni

$(OS2TKH)$(SEP)hevdef.h: $(MYDIR)os2$(SEP)hevdef.uni

$(OS2TKH)$(SEP)pm.h: $(MYDIR)os2$(SEP)pm$(SEP)pm.uni

$(OS2TKH)$(SEP)pmbase.h: $(MYDIR)os2$(SEP)pm$(SEP)pmbase.uni

$(OS2TKH)$(SEP)pmdef.h: $(MYDIR)os2$(SEP)pm$(SEP)pmdef.uni

$(OS2TKH)$(SEP)pmshl.h: $(MYDIR)os2$(SEP)pm$(SEP)pmshl.uni

$(OS2TKH)$(SEP)pmstddlg.h: $(MYDIR)os2$(SEP)pm$(SEP)pmstddlg.uni

$(OS2TKH)$(SEP)pmhelp.h: $(MYDIR)os2$(SEP)pm$(SEP)pmhelp.uni

$(OS2TKH)$(SEP)winhelp.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winhelp.uni

$(OS2TKH)$(SEP)pmtypes.h: $(MYDIR)os2$(SEP)pm$(SEP)pmtypes.uni

$(OS2TKH)$(SEP)wintypes.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wintypes.uni

$(OS2TKH)$(SEP)winstddrag.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstddrag.uni

$(OS2TKH)$(SEP)wincircularslider.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wincircularslider.uni

$(OS2TKH)$(SEP)winstdcnr.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdcnr.uni

$(OS2TKH)$(SEP)winstdcontainer.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdcontainer.uni

$(OS2TKH)$(SEP)winstatics.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstatics.uni

$(OS2TKH)$(SEP)winrectangles.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winrectangles.uni

$(OS2TKH)$(SEP)winscrollbars.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winscrollbars.uni

$(OS2TKH)$(SEP)winstdbook.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdbook.uni

$(OS2TKH)$(SEP)winstdfont.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdfont.uni

$(OS2TKH)$(SEP)winstdvalset.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdvalset.uni

$(OS2TKH)$(SEP)winstdspin.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdspin.uni

$(OS2TKH)$(SEP)winstdslider.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdslider.uni

$(OS2TKH)$(SEP)winsei.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winsei.uni

$(OS2TKH)$(SEP)pmsei.h: $(MYDIR)os2$(SEP)pm$(SEP)pmsei.uni

$(OS2TKH)$(SEP)winswitchlist.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winswitchlist.uni

$(OS2TKH)$(SEP)winswitchlistcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winswitchlistcommon.uni

$(OS2TKH)$(SEP)winframemgr.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winframemgr.uni

$(OS2TKH)$(SEP)winframectls.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winframectls.uni

$(OS2TKH)$(SEP)winload.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winload.uni

$(OS2TKH)$(SEP)winpalette.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winpalette.uni

$(OS2TKH)$(SEP)winstddlgs.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstddlgs.uni

$(OS2TKH)$(SEP)winstdfile.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winstdfile.uni

$(OS2TKH)$(SEP)winatom.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winatom.uni

$(OS2TKH)$(SEP)wincursors.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wincursors.uni

$(OS2TKH)$(SEP)wintrackrect.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wintrackrect.uni

$(OS2TKH)$(SEP)winpointers.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winpointers.uni

$(OS2TKH)$(SEP)winframemgrcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winframemgrcommon.uni

$(OS2TKH)$(SEP)pmwp.h: $(MYDIR)os2$(SEP)pm$(SEP)pmwp.uni

$(OS2TKH)$(SEP)pmwin.h: $(MYDIR)os2$(SEP)pm$(SEP)pmwin.uni

$(OUT)$(SEP)shared$(SEP)rexxsaa.h: $(MYDIR)shared$(SEP)rexxsaa.uni

$(OS2TKH)$(SEP)winprogramlist.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winprogramlist.uni

$(OS2TKH)$(SEP)winsys.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winsys.uni

$(OS2TKH)$(SEP)windde.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)windde.uni

$(OS2TKH)$(SEP)winmle.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winmle.uni

$(OS2TKH)$(SEP)windesktop.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)windesktop.uni

$(OS2TKH)$(SEP)winwindowmgrcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winwindowmgrcommon.uni

$(OS2TKH)$(SEP)winwindowmgr.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winwindowmgr.uni

$(OS2TKH)$(SEP)winmessagemgrcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winmessagemgrcommon.uni

$(OS2TKH)$(SEP)winmessagemgr.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winmessagemgr.uni

$(OS2TKH)$(SEP)winclipboard.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winclipboard.uni

$(OS2TKH)$(SEP)winclipboarddde.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winclipboarddde.uni

$(OS2TKH)$(SEP)wininput.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wininput.uni

$(OS2TKH)$(SEP)winaccelerators.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winaccelerators.uni

$(OS2TKH)$(SEP)wininputcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wininputcommon.uni

$(OS2TKH)$(SEP)winshelldata.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winshelldata.uni

$(OS2TKH)$(SEP)winmenus.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winmenus.uni

$(OS2TKH)$(SEP)winerrors.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winerrors.uni

$(OS2TKH)$(SEP)shlerrors.h: $(MYDIR)os2$(SEP)shlerrors.uni

$(OS2TKH)$(SEP)pmerr.h: $(MYDIR)os2$(SEP)pm$(SEP)pmerr.uni

$(OS2TKH)$(SEP)wperrors.h: $(MYDIR)os2$(SEP)wperrors.uni

$(OS2TKH)$(SEP)gpierrors.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpierrors.uni

$(OS2TKH)$(SEP)splerrors.h: $(MYDIR)os2$(SEP)splerrors.uni

$(OS2TKH)$(SEP)picerrors.h: $(MYDIR)os2$(SEP)picerrors.uni

$(OS2TKH)$(SEP)winhooks.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winhooks.uni

$(OS2TKH)$(SEP)wintimer.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wintimer.uni

$(OS2TKH)$(SEP)winthunkapi.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winthunkapi.uni

$(OS2TKH)$(SEP)winhprogram.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winhprogram.uni

$(OS2TKH)$(SEP)wincountry.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)wincountry.uni

$(OS2TKH)$(SEP)windialogscommon.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)windialogscommon.uni

$(OS2TKH)$(SEP)windialogs.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)windialogs.uni

$(OS2TKH)$(SEP)winentryfields.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winentryfields.uni

$(OS2TKH)$(SEP)winbuttons.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winbuttons.uni

$(OS2TKH)$(SEP)winlistboxes.h: $(MYDIR)os2$(SEP)pm$(SEP)win$(SEP)winlistboxes.uni

$(OS2TKH)$(SEP)pmgpi.h: $(MYDIR)os2$(SEP)pm$(SEP)pmgpi.uni

$(OS2TKH)$(SEP)pmdev.h: $(MYDIR)os2$(SEP)pm$(SEP)pmdev.uni

$(OS2TKH)$(SEP)devcommon.h: $(MYDIR)os2$(SEP)cpi$(SEP)devcommon.uni

$(OS2TKH)$(SEP)gpiprimitives.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpiprimitives.uni

$(OS2TKH)$(SEP)gpilcids.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpilcids.uni

$(OS2TKH)$(SEP)gpibitmapscommon.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpibitmapscommon.uni

$(OS2TKH)$(SEP)gpibitmaps.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpibitmaps.uni

$(OS2TKH)$(SEP)gpicontrolcommon.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpicontrolcommon.uni

$(OS2TKH)$(SEP)gpilogcoltable.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpilogcoltable.uni

$(OS2TKH)$(SEP)gpipolygons.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpipolygons.uni

$(OS2TKH)$(SEP)gpiregions.h: $(MYDIR)os2$(SEP)pm$(SEP)gpi$(SEP)gpiregions.uni

$(OUT)$(SEP)dos$(SEP)os2me.h: $(MYDIR)dos$(SEP)os2me.uni

$(OUT)$(SEP)dos$(SEP)os2mm.h: $(MYDIR)dos$(SEP)os2mm.uni

$(OUT)$(SEP)dos$(SEP)meerror.h: $(MYDIR)dos$(SEP)meerror.uni

$(OS2TKH)$(SEP)udsk.h: $(MYDIR)os2$(SEP)udsk.uni

$(OS2TKH)$(SEP)ulserrno.h: $(MYDIR)os2$(SEP)ulserrno.uni

$(OS2TKH)$(SEP)errcmpat.h: $(MYDIR)os2$(SEP)errcmpat.uni

$(OS2TKH)$(SEP)ulsitem.h: $(MYDIR)os2$(SEP)ulsitem.uni

$(OS2TKH)$(SEP)unidef.h: $(MYDIR)os2$(SEP)unidef.uni

$(OS2TKH)$(SEP)uconv.h: $(MYDIR)os2$(SEP)uconv.uni

$(OS2TKH)$(SEP)utlapi.h: $(MYDIR)shared$(SEP)utlapi.uni

$(OS2TKH)$(SEP)utlrectangles.h: $(MYDIR)shared$(SEP)utlapi$(SEP)utlrectangles.uni

## POSIX API

$(OS2TKH)$(SEP)stddef.h: $(MYDIR)os2$(SEP)stddef.uni

$(OS2TKH)$(SEP)time.h: $(MYDIR)os2$(SEP)time.uni


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
