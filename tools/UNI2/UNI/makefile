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
SRC = $(MYDIR)..$(SEP)..$(SEP)UNI$(SEP)
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
	$(OS2TKINC)basemid.inc &
	$(OS2TKINC)bseerr.inc &
	$(OS2TKINC)bsedos.inc &
	$(OS2TKINC)bsesub.inc &
	$(OS2TKINC)sas.inc &
	$(OS2TKINC)mac.inc &
#	$(OS2TKINC)sub.inc &
#	$(OS2TKINC)kbd.inc &
#	$(OS2TKINC)mou.inc &
#	$(OS2TKINC)vio.inc &
	.symbolic

## OS/2 Personality H files
os2: $(OS2TKH)os2.h &
     $(OS2TKH)os2def.h &
     $(OS2TKH)bse.h &
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
     $(OS2TKH)dosfmsmcommon.h &
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
 @$(%INTERP)uni2h.exe -e def -a $(SRC)os2$(SEP)os2386.abi $< $^*.def
 @$(LIB) $(LIBOPT) $^@ @$^*.def
# @$(DC) $^*.def $(BLACKHOLE)


$(OUT)$(SEP)shared$(SEP)rexxsaa.h: $(SRC)shared$(SEP)rexxsaa.uni


### OS/2 Personality
$(OS2TKINC)os2.inc: $(SRC)inc$(SEP)os2.inc
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

$(OS2TKINC)bseerr.inc: $(SRC)inc$(SEP)bseerr.inc
	$(verbose)$(SAY) CP       $^. $(LOG)
	$(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(OS2TKINC)basemid.inc: $(SRC)inc$(SEP)basemid.inc
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
.SUFFIXES: .lib .h .inc .uni

.uni: $(SRC)shared
.uni: $(SRC)shared$(SEP)utlapi
.uni: $(SRC)os2
.uni: $(SRC)os2$(SEP)cpi
.uni: $(SRC)os2$(SEP)pm
.uni: $(SRC)os2$(SEP)pm$(SEP)gpi
.uni: $(SRC)os2$(SEP)pm$(SEP)win
.uni: $(SRC)dos

.lib: $(PATH)

os2hfiles = $(OS2TKH)os2.h $(OS2TKH)os2def.h $(OS2TKH)bse.h

$(os2hfiles): os2.uni
 @$(SAY) UNI2H    $^. $(LOG)
 uni2h.exe -c -p -i -e h -a $(SRC)os2$(SEP)os2386.abi -l os2.layout $< $^@ $(LOG2)
# $(verbose)$(%INTERP)uni2h.exe -e h -a $(SRC)os2$(SEP)os2386.abi -l os2.layout $< $^@ $(LOG2)

#.uni.inc: .AUTODEPEND
# @$(SAY) UNI2INC  $^. $(LOG)
# $(verbose)$(%INTERP)uni2h.exe -e inc -a $(SRC)os2$(SEP)os2386.abi $< $^@ $(LOG2)

.uni.lib: .AUTODEPEND
 @$(SAY) UNI2DEF  $^. $(LOG)
 $(verbose)$(%INTERP)uni2h.exe -e def -a $(SRC)os2$(SEP)os2386.abi $< $^*.def $(LOG2)
# $(verbose)$(LIB) $(LIBOPT) $^@ @$^*.def
