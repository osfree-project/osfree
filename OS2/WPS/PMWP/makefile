PROJ     = pmwp
TRGT     = $(PROJ).dll
DESC     = WPS
srcfiles = $(p)init$(e) $(p)shl$(e) $(p)wpobject$(e) $(p)wpfsys$(e) $(p)wpfolder$(e) $(p)wpdataf$(e) $(p)wptrans$(e) $(p)wpabs$(e) $(p)wpdisk$(e) $(p)wppgm$(e) $(p)wppgmf$(e) $(p)wpshadow$(e) 
ADD_SCOPT = -I$(%OS2TK)$(SEP)som$(SEP)include -I$(%OS2TK)$(SEP)idl
ADD_COPT = -I$(%OS2TK)$(SEP)som$(SEP)include -I$(%OS2TK)$(SEP)h
ADD_LINKOPT  = segment   type DATA nonshared lib $(%OS2TK)$(SEP)som$(SEP)lib$(SEP)somtk.lib
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata, caseexact, start=dll_initterm
#UNI2H = 1

DEST_DIR =$(%OS2TK)$(SEP)idl
DESTH_DIR =$(%OS2TK)$(SEP)h
SRC_DIR =$(MYDIR)..$(SEP)contrib$(SEP)idl$(SEP)wps

IDLS = $(DEST_DIR)$(SEP)wpobject.idl &
			$(DEST_DIR)$(SEP)wpdisk.idl &
			$(DEST_DIR)$(SEP)wppgm.idl &
			$(DEST_DIR)$(SEP)wpshadow.idl &
			$(DEST_DIR)$(SEP)wptypes.idl

IDLS2 = $(DEST_DIR)$(SEP)wpabs.idl &
			$(DEST_DIR)$(SEP)wpfsys.idl &
			$(DEST_DIR)$(SEP)wpfolder.idl &
			$(DEST_DIR)$(SEP)wpdataf.idl &
			$(DEST_DIR)$(SEP)wpdesk.idl &
			$(DEST_DIR)$(SEP)wppgmf.idl &
			$(DEST_DIR)$(SEP)wptrans.idl

HEADERS=$(DESTH_DIR)$(SEP)wpobject.h &
			$(DESTH_DIR)$(SEP)wpobject.xh &
			$(DESTH_DIR)$(SEP)wpfsys.h &
			$(DESTH_DIR)$(SEP)wpfsys.xh &
			$(DESTH_DIR)$(SEP)wpfolder.h &
			$(DESTH_DIR)$(SEP)wpfolder.xh &
			$(DESTH_DIR)$(SEP)wpdataf.h &
			$(DESTH_DIR)$(SEP)wpdataf.xh &
			$(DESTH_DIR)$(SEP)wpdisk.h &
			$(DESTH_DIR)$(SEP)wpdisk.xh &
			$(DESTH_DIR)$(SEP)wpdesk.h &
			$(DESTH_DIR)$(SEP)wpdesk.xh &
			$(DESTH_DIR)$(SEP)wppgm.h &
			$(DESTH_DIR)$(SEP)wppgm.xh &
			$(DESTH_DIR)$(SEP)wppgmf.h &
			$(DESTH_DIR)$(SEP)wppgmf.xh &
			$(DESTH_DIR)$(SEP)wpshadow.h &
			$(DESTH_DIR)$(SEP)wpshadow.xh &
			$(DESTH_DIR)$(SEP)wptypes.h &
			$(DESTH_DIR)$(SEP)wptypes.xh &
			$(DESTH_DIR)$(SEP)wptrans.h &
			$(DESTH_DIR)$(SEP)wptrans.xh &
			$(DESTH_DIR)$(SEP)wpabs.h &
			$(DESTH_DIR)$(SEP)wpabs.xh


IHEADERS=$(PATH)$(SEP)wpobject.ih &
			$(PATH)$(SEP)wpobject.xih &
			$(PATH)$(SEP)wpfsys.ih &
			$(PATH)$(SEP)wpfsys.xih &
			$(PATH)$(SEP)wpfolder.ih &
			$(PATH)$(SEP)wpfolder.xih &
			$(PATH)$(SEP)wpdataf.ih &
			$(PATH)$(SEP)wpdataf.xih &
			$(PATH)$(SEP)wpdesk.ih &
			$(PATH)$(SEP)wpdesk.xih &
			$(PATH)$(SEP)wpdisk.ih &
			$(PATH)$(SEP)wpdisk.xih &
			$(PATH)$(SEP)wppgm.ih &
			$(PATH)$(SEP)wppgm.xih &
			$(PATH)$(SEP)wppgmf.ih &
			$(PATH)$(SEP)wppgmf.xih &
			$(PATH)$(SEP)wpshadow.ih &
			$(PATH)$(SEP)wpshadow.xih &
			$(PATH)$(SEP)wptypes.ih &
			$(PATH)$(SEP)wptypes.xih &
			$(PATH)$(SEP)wptrans.ih &
			$(PATH)$(SEP)wptrans.xih &
			$(PATH)$(SEP)wpabs.ih &
			$(PATH)$(SEP)wpabs.xih

EXPORTS      = &
		ShlStartWorkplace .211, &
		WPFileSystemCClassData.10, &
		WPFileSystemClassData.11, &
		WPFileSystemNewClass.12, &
		WPFolderCClassData.13, &
		WPFolderClassData.14, &
		WPFolderNewClass.15, &
		WPDataFileCClassData.16, &
		WPDataFileClassData.17, &
		WPDataFileNewClass.18, &
		WPObjectCClassData.22, &
		WPObjectClassData.23, &
		WPObjectNewClass.24, &
		WPAbstractCClassData.25, &
		WPAbstractClassData.26, &
		WPAbstractNewClass.27, &
		WPShadowCClassData.31, &
		WPShadowClassData.32, &
		WPShadowNewClass.33, &
		WPDiskCClassData.34, &
		WPDiskClassData.35, &
		WPDiskNewClass.36, &
		WPTransientCClassData.40, &
		WPTransientClassData.41, &
		WPTransientNewClass.42, &
		WPProgramCClassData.46, &
		WPProgramClassData.47, &
		WPProgramNewClass.48, &
		WPProgramFileCClassData.49, &
		WPProgramFileClassData.50, &
		WPProgramFileNewClass.51, &
		WPDesktopCClassData.55, &
		WPDesktopClassData.56, &
		WPDesktopNewClass

DEST     = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_pm.mk

TARGETS   = $(IDLS) $(IDLS2) $(HEADERS) $(IHEADERS) $(OBJS)
			
SOURCES =	$(SRC_DIR)$(SEP)wpobject.idl &
			$(SRC_DIR)$(SEP)wpdisk.idl &
			$(SRC_DIR)$(SEP)wppgm.idl &
			$(SRC_DIR)$(SEP)wpshadow.idl &
			$(SRC_DIR)$(SEP)wptypes.idl

SOURCES2 =	$(MYDIR)$(SEP)wpabs.idl &
		$(MYDIR)$(SEP)wpfsys.idl &
		$(MYDIR)$(SEP)wpfolder.idl &
		$(MYDIR)$(SEP)wpdataf.idl &
		$(MYDIR)$(SEP)wpdesk.idl &
		$(MYDIR)$(SEP)wppgmf.idl &
		$(MYDIR)$(SEP)wptrans.idl

$(HEADERS): $(IDLS) $(IDLS2)

$(IHEADERS): $(IDLS) $(IDLS2)

$(IDLS): $(SOURCES)
 @$(MDHIER) $(DEST_DIR)
 $(verbose)$(CP) $(SRC_DIR)$(SEP)$^. $^@ $(BLACKHOLE)

$(IDLS2): $(SOURCES2)
 @$(MDHIER) $(DEST_DIR)
 $(verbose)$(CP) $(MYDIR)$(SEP)$^. $^@ $(BLACKHOLE)


.idl: $(SRC_DIR)
.h: $(DEST_DIR)
.xh: $(DEST_DIR)
.ih: $(PATH)
#.xih: $(PATH)
