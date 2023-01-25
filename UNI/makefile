#
# A Makefile for Unified API
# (c) osFree project,
# valerius, 2006/10/30
#

!ifndef TARGETBLD
PLATFORM = host$(SEP)$(%HOST)$(SEP)
OS2TKH = $(%OS2TK)$(SEP)h$(SEP)
OS2TKINC = $(%OS2TK)$(SEP)inc$(SEP)
SRC = $(MYDIR)
!else
PLATFORM =
OS2TKH = $(%ROOT)build$(SEP)bin$(SEP)os2tk45$(SEP)h$(SEP)
OS2TKINC = $(%ROOT)build$(SEP)bin$(SEP)os2tk45$(SEP)inc$(SEP)
SRC = $(MYDIR)..$(SEP)..$(SEP)..$(SEP)UNI$(SEP)
!endif

!include $(%ROOT)tools/mk/all.mk

OUT = $(%ROOT)build$(SEP)include$(SEP)
LIBOUT = $(%ROOT)build$(SEP)lib$(SEP)

install: inc h .symbolic

outdirs = $(OUT) $(OS2TKINC) $(LIBOUT) $(OUT)$(SEP)dos $(OS2TKH) $(OUT)$(SEP)shared

inc: pre .symbolic

h: pre workaround $(OUT)osfree.h os2 os2libs dos .symbolic

pre: .symbolic
 # @$(SAY) Creating directories...
 @for %i in ($(outdirs)) do @$(MDHIER) %i $(BLACKHOLE)

$(OUT)osfree.h: $(SRC)osfree.uni
# Main osFree file

workaround: $(SRC)cdeftypes2.h $(SRC)dosfilemgrcommon.h $(SRC)exe386.h $(SRC)newexe.h .symbolic
# Workaround of uniplemented features of uni2h tool
  @$(CP) $(SRC)cdeftypes2.h $(OS2TKH) $(BLACKHOLE)
  @$(CP) $(SRC)newexe.h $(OS2TKH) $(BLACKHOLE)
  @$(CP) $(SRC)exe386.h $(OS2TKH) $(BLACKHOLE)
  @$(CP) $(SRC)dosfilemgrcommon.h $(OS2TKH) $(BLACKHOLE)

## OS/2 Personality INC files
os2inc: $(OS2TKINC)os2.inc &
	$(OS2TKINC)bseerr.inc &
	$(OS2TKINC)bsedos.inc &
	$(OS2TKINC)bsesub.inc &
	$(OS2TKINC)sas.inc &
	$(OS2TKINC)mac.inc &
	.symbolic

## OS/2 Personality H files
os2: $(OS2TKH)os2.h &
     $(OS2TKH)os2def.h &
     $(OS2TKH)bse.h &
     $(OS2TKH)base.h &
     $(OS2TKH)bseord.h &
     $(OS2TKH)bsememf.h &
     $(OS2TKH)bsedos.h &
     $(OS2TKH)bsedev.h &
     $(OS2TKH)hevdef.h &
     $(OS2TKH)dos.h &
     $(OS2TKH)dosdevices.h &
     $(OS2TKH)bsesub.h &
     $(OS2TKH)sub.h &
     $(OS2TKH)vio.h &
     $(OS2TKH)kbd.h &
     $(OS2TKH)mou.h &
     $(OS2TKH)win.h &
     $(OS2TKH)winsei.h &
     $(OS2TKH)bseerr.h &
     $(OS2TKH)doserrors.h &
     $(OS2TKH)bsexcpt.h &
     $(OS2TKH)dosexcptfunc.h &
     $(OS2TKH)basemid.h &
     $(OS2TKH)bsetib.h &
     $(OS2TKH)dosqueues.h &
     $(OS2TKH)dosdebug.h &
     $(OS2TKH)dosmisc.h &
     $(OS2TKH)dosmvdm.h &
     $(OS2TKH)dosnls.h &
     $(OS2TKH)dosnmpipes.h &
     $(OS2TKH)dosmodulemgr.h &
     $(OS2TKH)dossesmgr.h &
     $(OS2TKH)dosfilemgr.h &
#     $(OS2TKH)dosfilemgrcommon.h &
     $(OS2TKH)dosmemmgr.h &
     $(OS2TKH)dosmemmgrcommon.h &
     $(OS2TKH)dosmonitors.h &
     $(OS2TKH)dosprocess.h &
     $(OS2TKH)dosdevioctl.h &
     $(OS2TKH)dosprofile.h &
     $(OS2TKH)dossemaphores.h &
     $(OS2TKH)dosprocesscommon.h &
     $(OS2TKH)dosresources.h &
     $(OS2TKH)dosresourcescommon.h &
     $(OS2TKH)dosexceptions.h &
     $(OS2TKH)dosdatetime.h &
     $(OS2TKH)pm.h &
     $(OS2TKH)pmdef.h &
     $(OS2TKH)pmavio.h &
     $(OS2TKH)pmbase.h &
     $(OS2TKH)pmdev.h &
     $(OS2TKH)devcommon.h &
     $(OS2TKH)dev.h &
     $(OS2TKH)pmshl.h &
     $(OS2TKH)winswitchlistcommon.h &
     $(OS2TKH)winswitchlist.h &
     $(OS2TKH)pmwp.h &
     $(OS2TKH)pmwin.h &
     $(OS2TKH)pmstddlg.h &
     $(OS2TKH)winstdbook.h &
     $(OS2TKH)winstddlgs.h &
     $(OS2TKH)winstdfile.h &
     $(OS2TKH)winstdslider.h &
     $(OS2TKH)winstdspin.h &
     $(OS2TKH)winstdvalset.h &
     $(OS2TKH)winsys.h &
     $(OS2TKH)winhprogram.h &
     $(OS2TKH)wininput.h &
     $(OS2TKH)wincircularslider.h &
     $(OS2TKH)winstdcnr.h &
     $(OS2TKH)winstdcontainer.h &
     $(OS2TKH)winstdfont.h &
     $(OS2TKH)wintrackrect.h &
     $(OS2TKH)wininputcommon.h &
     $(OUT)shared$(SEP)rexxsaa.h &
     $(OS2TKH)winatom.h &
     $(OS2TKH)winthunkapi.h &
     $(OS2TKH)winmenus.h &
     $(OS2TKH)winaccelerators.h &
     $(OS2TKH)winload.h &
     $(OS2TKH)winstddrag.h &
     $(OS2TKH)winclipboard.h &
     $(OS2TKH)winmessagemgr.h &
     $(OS2TKH)winmessagemgrcommon.h &
     $(OS2TKH)winwindowmgrcommon.h &
     $(OS2TKH)winwindowmgr.h &
     $(OS2TKH)winclipboarddde.h &
     $(OS2TKH)winprogramlist.h &
     $(OS2TKH)winframectls.h &
     $(OS2TKH)winframemgr.h &
     $(OS2TKH)winframemgrcommon.h &
     $(OS2TKH)winshelldata.h &
     $(OS2TKH)winerrors.h &
     $(OS2TKH)shlerrors.h &
     $(OS2TKH)pmerr.h &
     $(OS2TKH)wperrors.h &
     $(OS2TKH)gpierrors.h &
     $(OS2TKH)splerrors.h &
     $(OS2TKH)picerrors.h &
     $(OS2TKH)winhooks.h &
     $(OS2TKH)wintimer.h &
     $(OS2TKH)wincountry.h &
     $(OS2TKH)wincursors.h &
     $(OS2TKH)windde.h &
     $(OS2TKH)winstatics.h &
     $(OS2TKH)winmle.h &
     $(OS2TKH)wintypes.h &
     $(OS2TKH)pmtypes.h &
     $(OS2TKH)winpalette.h &
     $(OS2TKH)winpointers.h &
     $(OS2TKH)windesktop.h &
     $(OS2TKH)winentryfields.h &
     $(OS2TKH)windialogs.h &
     $(OS2TKH)windialogscommon.h &
     $(OS2TKH)winrectangles.h &
     $(OS2TKH)winscrollbars.h &
     $(OS2TKH)winbuttons.h &
     $(OS2TKH)winlistboxes.h &
     $(OS2TKH)pmhelp.h &
     $(OS2TKH)pmsei.h &
#     $(OS2TKH)winei.h &
     $(OS2TKH)winhelp.h &
     $(OS2TKH)pmgpi.h &
     $(OS2TKH)gpiprimitives.h &
     $(OS2TKH)gpiprimitives2.h &
     $(OS2TKH)gpilcids.h &
     $(OS2TKH)gpicorrelation.h &
     $(OS2TKH)gpicontrol.h &
     $(OS2TKH)gpimetafiles.h &
     $(OS2TKH)gpicontrolcommon.h &
     $(OS2TKH)gpidefaults.h &
     $(OS2TKH)gpisegediting.h &
     $(OS2TKH)gpisegments.h &
     $(OS2TKH)gpitransforms.h &
     $(OS2TKH)gpibitmapscommon.h &
     $(OS2TKH)gpibitmaps.h &
     $(OS2TKH)gpipaths.h &
     $(OS2TKH)gpilogcoltable.h &
     $(OS2TKH)gpipolygons.h &
     $(OS2TKH)gpiregions.h &
     $(OS2TKH)os2me.h &
     $(OS2TKH)os2mm.h &
     $(OS2TKH)meerror.h &
     $(OS2TKH)udsk.h &
     $(OS2TKH)unidef.h &
     $(OS2TKH)ulserrno.h &
     $(OS2TKH)errcmpat.h &
     $(OS2TKH)ulsitem.h &
     $(OS2TKH)uconv.h &
     $(OS2TKH)stddef.h &
     $(OS2TKH)time.h &
     $(OS2TKH)utlapi.h &
     $(OS2TKH)utlrectangles.h &
     os2inc &
     .symbolic

#     $(OS2TKH)bsexcpt.h &

os2libs: $(LIBOUT)sub32.lib &
         .symbolic

$(LIBOUT)sub32.lib: $(SRC)os2$(SEP)vio.uni
 @$(SAY) LIB      $^.
 @uni2h.exe -e def -a $(SRC)os2$(SEP)os2386.abi $< $^*.def
 @$(LIB) $(LIBOPT) $^@ @$^*.def
# @$(DC) $^*.def $(BLACKHOLE)

$(OS2TKH)os2.h: $(SRC)os2$(SEP)os2.uni

### OS/2 Personality base type and macros definitions
$(OS2TKH)os2def.h: $(SRC)os2$(SEP)os2def.uni

### OS/2 Personality base API
$(OS2TKH)bse.h: $(SRC)os2$(SEP)cpi$(SEP)bse.uni
$(OS2TKH)base.h: $(SRC)os2$(SEP)cpi$(SEP)base.uni

### OS/2 Personality base DOS API ordinals
$(OS2TKH)bsedos.h: $(SRC)os2$(SEP)cpi$(SEP)bsedos.uni

### OS/2 Personality base error codes and messages
$(OS2TKH)bseerr.h: $(SRC)os2$(SEP)cpi$(SEP)bseerr.uni

### OS/2 Personality base API ordinals
$(OS2TKH)bseord.h: $(SRC)os2$(SEP)cpi$(SEP)bseord.uni

### OS/2 Personality memory API flags
$(OS2TKH)bsememf.h: $(SRC)os2$(SEP)cpi$(SEP)bsememf.uni

$(OS2TKH)bsedev.h: $(SRC)os2$(SEP)cpi$(SEP)bsedev.uni

$(OS2TKH)bsetib.h: $(SRC)os2$(SEP)cpi$(SEP)bsetib.uni

$(OS2TKH)bsexcpt.h: $(SRC)os2$(SEP)cpi$(SEP)bsexcpt.uni

$(OS2TKH)bsesub.h: $(SRC)os2$(SEP)cpi$(SEP)bsesub.uni

$(OS2TKH)sub.h: $(SRC)os2$(SEP)sub.uni

$(OS2TKH)vio.h: $(SRC)os2$(SEP)vio.uni

$(OS2TKH)kbd.h: $(SRC)os2$(SEP)kbd.uni

$(OS2TKH)mou.h: $(SRC)os2$(SEP)mou.uni

$(OS2TKH)dos.h: $(SRC)os2$(SEP)cpi$(SEP)dos.uni

$(OS2TKH)win.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)win.uni

$(OS2TKH)dosprocesscommon.h: $(SRC)os2$(SEP)cpi$(SEP)dosprocesscommon.uni

$(OS2TKH)dosprocess.h: $(SRC)os2$(SEP)cpi$(SEP)dosprocess.uni

$(OS2TKH)dosprofile.h: $(SRC)os2$(SEP)cpi$(SEP)dosprofile.uni

$(OS2TKH)dosmisc.h: $(SRC)os2$(SEP)cpi$(SEP)dosmisc.uni

$(OS2TKH)basemid.h: $(SRC)os2$(SEP)cpi$(SEP)basemid.uni

$(OS2TKH)dosnmpipes.h: $(SRC)os2$(SEP)cpi$(SEP)dosnmpipes.uni

$(OS2TKH)dosmvdm.h: $(SRC)os2$(SEP)cpi$(SEP)dosmvdm.uni

$(OS2TKH)dossesmgr.h: $(SRC)os2$(SEP)cpi$(SEP)dossesmgr.uni

$(OS2TKH)dosnls.h: $(SRC)os2$(SEP)cpi$(SEP)dosnls.uni

$(OS2TKH)dosdevioctl.h: $(SRC)os2$(SEP)cpi$(SEP)dosdevioctl.uni

$(OS2TKH)dosqueues.h: $(SRC)os2$(SEP)cpi$(SEP)dosqueues.uni

$(OS2TKH)doserrors.h: $(SRC)os2$(SEP)cpi$(SEP)doserrors.uni

$(OS2TKH)dosmodulemgr.h: $(SRC)os2$(SEP)cpi$(SEP)dosmodulemgr.uni

$(OS2TKH)dosfilemgr.h: $(SRC)os2$(SEP)cpi$(SEP)dosfilemgr.uni

$(OS2TKH)dossemaphores.h: $(SRC)os2$(SEP)cpi$(SEP)dossemaphores.uni

$(OS2TKH)dosdatetime.h: $(SRC)os2$(SEP)cpi$(SEP)dosdatetime.uni

$(OS2TKH)dosdevices.h: $(SRC)os2$(SEP)cpi$(SEP)dosdevices.uni

$(OS2TKH)dosfilemgrcommon.h: $(SRC)os2$(SEP)cpi$(SEP)dosfilemgrcommon.uni

$(OS2TKH)dosmemmgr.h: $(SRC)os2$(SEP)cpi$(SEP)dosmemmgr.uni

$(OS2TKH)dosmonitors.h: $(SRC)os2$(SEP)cpi$(SEP)dosmonitors.uni

$(OS2TKH)dosmemmgrcommon.h: $(SRC)os2$(SEP)cpi$(SEP)dosmemmgrcommon.uni

$(OS2TKH)dosexceptions.h: $(SRC)os2$(SEP)cpi$(SEP)dosexceptions.uni

$(OS2TKH)dosexcptfunc.h: $(SRC)os2$(SEP)cpi$(SEP)dosexcptfunc.uni

$(OS2TKH)dosresources.h: $(SRC)os2$(SEP)cpi$(SEP)dosresources.uni

$(OS2TKH)dosresourcescommon.h: $(SRC)os2$(SEP)cpi$(SEP)dosresourcescommon.uni

$(OS2TKH)dosdebug.h: $(SRC)os2$(SEP)cpi$(SEP)dosdebug.uni

$(OS2TKH)hevdef.h: $(SRC)os2$(SEP)hevdef.uni

$(OS2TKH)pm.h: $(SRC)os2$(SEP)pm$(SEP)pm.uni

$(OS2TKH)pmbase.h: $(SRC)os2$(SEP)pm$(SEP)pmbase.uni

$(OS2TKH)pmdef.h: $(SRC)os2$(SEP)pm$(SEP)pmdef.uni

$(OS2TKH)pmshl.h: $(SRC)os2$(SEP)pm$(SEP)pmshl.uni

$(OS2TKH)pmstddlg.h: $(SRC)os2$(SEP)pm$(SEP)pmstddlg.uni

$(OS2TKH)pmhelp.h: $(SRC)os2$(SEP)pm$(SEP)pmhelp.uni

$(OS2TKH)winhelp.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winhelp.uni

$(OS2TKH)pmtypes.h: $(SRC)os2$(SEP)pm$(SEP)pmtypes.uni

$(OS2TKH)wintypes.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)wintypes.uni

$(OS2TKH)winstddrag.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstddrag.uni

$(OS2TKH)wincircularslider.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)wincircularslider.uni

$(OS2TKH)winstdcnr.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstdcnr.uni

$(OS2TKH)winstdcontainer.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstdcontainer.uni

$(OS2TKH)winstatics.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstatics.uni

$(OS2TKH)winrectangles.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winrectangles.uni

$(OS2TKH)winscrollbars.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winscrollbars.uni

$(OS2TKH)winstdbook.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstdbook.uni

$(OS2TKH)winstdfont.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstdfont.uni

$(OS2TKH)winstdvalset.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstdvalset.uni

$(OS2TKH)winstdspin.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstdspin.uni

$(OS2TKH)winstdslider.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstdslider.uni

$(OS2TKH)winsei.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winsei.uni

$(OS2TKH)pmsei.h: $(SRC)os2$(SEP)pm$(SEP)pmsei.uni

$(OS2TKH)winswitchlist.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winswitchlist.uni

$(OS2TKH)winswitchlistcommon.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winswitchlistcommon.uni

$(OS2TKH)winframemgr.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winframemgr.uni

$(OS2TKH)winframectls.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winframectls.uni

$(OS2TKH)winload.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winload.uni

$(OS2TKH)winpalette.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winpalette.uni

$(OS2TKH)winstddlgs.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstddlgs.uni

$(OS2TKH)winstdfile.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winstdfile.uni

$(OS2TKH)winatom.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winatom.uni

$(OS2TKH)wincursors.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)wincursors.uni

$(OS2TKH)wintrackrect.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)wintrackrect.uni

$(OS2TKH)winpointers.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winpointers.uni

$(OS2TKH)winframemgrcommon.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winframemgrcommon.uni

$(OS2TKH)pmwp.h: $(SRC)os2$(SEP)pm$(SEP)pmwp.uni

$(OS2TKH)pmwin.h: $(SRC)os2$(SEP)pm$(SEP)pmwin.uni

$(OUT)$(SEP)shared$(SEP)rexxsaa.h: $(SRC)shared$(SEP)rexxsaa.uni

$(OS2TKH)winprogramlist.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winprogramlist.uni

$(OS2TKH)winsys.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winsys.uni

$(OS2TKH)windde.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)windde.uni

$(OS2TKH)winmle.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winmle.uni

$(OS2TKH)windesktop.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)windesktop.uni

$(OS2TKH)winwindowmgrcommon.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winwindowmgrcommon.uni

$(OS2TKH)winwindowmgr.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winwindowmgr.uni

$(OS2TKH)winmessagemgrcommon.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winmessagemgrcommon.uni

$(OS2TKH)winmessagemgr.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winmessagemgr.uni

$(OS2TKH)winclipboard.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winclipboard.uni

$(OS2TKH)winclipboarddde.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winclipboarddde.uni

$(OS2TKH)wininput.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)wininput.uni

$(OS2TKH)winaccelerators.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winaccelerators.uni

$(OS2TKH)wininputcommon.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)wininputcommon.uni

$(OS2TKH)winshelldata.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winshelldata.uni

$(OS2TKH)winmenus.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winmenus.uni

$(OS2TKH)winerrors.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winerrors.uni

$(OS2TKH)shlerrors.h: $(SRC)os2$(SEP)shlerrors.uni

$(OS2TKH)pmerr.h: $(SRC)os2$(SEP)pm$(SEP)pmerr.uni

$(OS2TKH)wperrors.h: $(SRC)os2$(SEP)wperrors.uni

$(OS2TKH)gpierrors.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpierrors.uni

$(OS2TKH)splerrors.h: $(SRC)os2$(SEP)splerrors.uni

$(OS2TKH)picerrors.h: $(SRC)os2$(SEP)picerrors.uni

$(OS2TKH)winhooks.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winhooks.uni

$(OS2TKH)wintimer.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)wintimer.uni

$(OS2TKH)winthunkapi.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winthunkapi.uni

$(OS2TKH)winhprogram.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winhprogram.uni

$(OS2TKH)wincountry.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)wincountry.uni

$(OS2TKH)windialogscommon.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)windialogscommon.uni

$(OS2TKH)windialogs.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)windialogs.uni

$(OS2TKH)winentryfields.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winentryfields.uni

$(OS2TKH)winbuttons.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winbuttons.uni

$(OS2TKH)winlistboxes.h: $(SRC)os2$(SEP)pm$(SEP)win$(SEP)winlistboxes.uni

$(OS2TKH)pmgpi.h: $(SRC)os2$(SEP)pm$(SEP)pmgpi.uni

$(OS2TKH)pmdev.h: $(SRC)os2$(SEP)pm$(SEP)pmdev.uni

$(OS2TKH)devcommon.h: $(SRC)os2$(SEP)cpi$(SEP)devcommon.uni

$(OS2TKH)dev.h: $(SRC)os2$(SEP)cpi$(SEP)dev.uni

$(OS2TKH)gpiprimitives.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpiprimitives.uni

$(OS2TKH)gpilcids.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpilcids.uni

$(OS2TKH)gpipaths.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpipaths.uni

$(OS2TKH)gpibitmapscommon.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpibitmapscommon.uni

$(OS2TKH)gpibitmaps.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpibitmaps.uni

$(OS2TKH)gpicorrelation.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpicorrelation.uni

$(OS2TKH)gpicontrol.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpicontrol.uni

$(OS2TKH)gpicontrolcommon.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpicontrolcommon.uni

$(OS2TKH)gpilogcoltable.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpilogcoltable.uni

$(OS2TKH)gpipolygons.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpipolygons.uni

$(OS2TKH)gpiregions.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpiregions.uni

$(OS2TKH)gpisegediting.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpisegediting.uni

$(OS2TKH)gpisegments.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpisegments.uni

$(OS2TKH)gpitransforms.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpitransforms.uni

$(OS2TKH)gpidefaults.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpidefaults.uni

$(OS2TKH)gpimetafiles.h: $(SRC)os2$(SEP)pm$(SEP)gpi$(SEP)gpimetafiles.uni

$(OUT)$(SEP)dos$(SEP)os2me.h: $(SRC)dos$(SEP)os2me.uni

$(OUT)$(SEP)dos$(SEP)os2mm.h: $(SRC)dos$(SEP)os2mm.uni

$(OUT)$(SEP)dos$(SEP)meerror.h: $(SRC)dos$(SEP)meerror.uni

$(OS2TKH)udsk.h: $(SRC)os2$(SEP)udsk.uni

$(OS2TKH)ulserrno.h: $(SRC)os2$(SEP)ulserrno.uni

$(OS2TKH)errcmpat.h: $(SRC)os2$(SEP)errcmpat.uni

$(OS2TKH)ulsitem.h: $(SRC)os2$(SEP)ulsitem.uni

$(OS2TKH)unidef.h: $(SRC)os2$(SEP)unidef.uni

$(OS2TKH)uconv.h: $(SRC)os2$(SEP)uconv.uni

$(OS2TKH)utlapi.h: $(SRC)shared$(SEP)utlapi.uni

$(OS2TKH)utlrectangles.h: $(SRC)shared$(SEP)utlapi$(SEP)utlrectangles.uni

### OS/2 Personality
$(OS2TKINC)os2.inc: $(SRC)inc$(SEP)os2.inc
	$(verbose)$(SAY) CP       $^. $(LOG)
	$(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(OS2TKINC)bseerr.inc: $(SRC)inc$(SEP)bseerr.inc
	$(verbose)$(SAY) CP       $^. $(LOG)
	$(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(OS2TKINC)bsedos.inc: $(SRC)inc$(SEP)bsedos.inc
	$(verbose)$(SAY) CP       $^. $(LOG)
	$(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(OS2TKINC)bsesub.inc: $(SRC)inc$(SEP)bsesub.inc
	$(verbose)$(SAY) CP       $^. $(LOG)
	$(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(OS2TKINC)sas.inc: $(SRC)inc$(SEP)sas.inc
	$(verbose)$(SAY) CP       $^. $(LOG)
	$(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(OS2TKINC)mac.inc: $(SRC)inc$(SEP)mac.inc
	$(verbose)$(SAY) CP       $^. $(LOG)
	$(verbose)$(CP) $< $^@ $(BLACKHOLE)

## POSIX API

$(OS2TKH)stddef.h: $(SRC)os2$(SEP)stddef.uni

$(OS2TKH)time.h: $(SRC)os2$(SEP)time.uni


## DOS Personality files
dos: $(OUT)dos$(SEP)dos.h .symbolic # $(OUT)$dos$(SEP)os2vdm.h

$(OUT)dos$(SEP)dos.h: $(SRC)dos$(SEP)dos.uni

$(OUT)dos$(SEP)os2vdm.h: $(SRC)dos$(SEP)os2vdm.uni

.SUFFIXES:
.SUFFIXES: .lib .h .uni

.uni: $(SRC)shared
.uni: $(SRC)shared$(SEP)utlapi
.uni: $(SRC)os2
.uni: $(SRC)os2$(SEP)cpi
.uni: $(SRC)os2$(SEP)pm
.uni: $(SRC)os2$(SEP)pm$(SEP)gpi
.uni: $(SRC)os2$(SEP)pm$(SEP)win
.uni: $(SRC)dos

.lib: $(PATH)

.uni.h: .AUTODEPEND
 @$(SAY) UNI2H    $^. $(LOG)
 $(verbose)uni2h.exe -e h -a $(SRC)os2$(SEP)os2386.abi $< $^@ $(LOG2)

.uni.lib: .AUTODEPEND
 @$(SAY) LIB      $^. $(LOG)
 $(verbose)uni2h.exe -e def -a $(SRC)os2$(SEP)os2386.abi $< $^*.def $(LOG2)
# $(verbose)$(LIB) $(LIBOPT) $^@ @$^*.def
