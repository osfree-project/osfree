PROJ     = pmwp
TRGT     = $(PROJ).dll
DESC     = WPS
srcfiles = $(p)init$(e) $(p)shl$(e) $(p)wpobject$(e) $(p)wpabs$(e) $(p)wpdisk$(e) $(p)wppgm$(e) $(p)wpshadow$(e)
ADD_SCOPT = -I$(%OS2TK)$(SEP)som$(SEP)include -I$(%OS2TK)$(SEP)idl
ADD_COPT = -sg -I$(%OS2TK)$(SEP)som$(SEP)include -I$(%OS2TK)$(SEP)h
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

IDLS2 = $(DEST_DIR)$(SEP)wpabs.idl

HEADERS=$(DESTH_DIR)$(SEP)wpobject.h &
			$(DESTH_DIR)$(SEP)wpobject.xh &
			$(DESTH_DIR)$(SEP)wpdisk.h &
			$(DESTH_DIR)$(SEP)wpdisk.xh &
			$(DESTH_DIR)$(SEP)wppgm.h &
			$(DESTH_DIR)$(SEP)wppgm.xh &
			$(DESTH_DIR)$(SEP)wpshadow.h &
			$(DESTH_DIR)$(SEP)wpshadow.xh &
			$(DESTH_DIR)$(SEP)wptypes.h &
			$(DESTH_DIR)$(SEP)wptypes.xh &
			$(DESTH_DIR)$(SEP)wpabs.h &
			$(DESTH_DIR)$(SEP)wpabs.xh


IHEADERS=$(PATH)$(SEP)wpobject.ih &
			$(PATH)$(SEP)wpobject.xih &
			$(PATH)$(SEP)wpdisk.ih &
			$(PATH)$(SEP)wpdisk.xih &
			$(PATH)$(SEP)wppgm.ih &
			$(PATH)$(SEP)wppgm.xih &
			$(PATH)$(SEP)wpshadow.ih &
			$(PATH)$(SEP)wpshadow.xih &
			$(PATH)$(SEP)wptypes.ih &
			$(PATH)$(SEP)wptypes.xih &
			$(PATH)$(SEP)wpabs.ih &
			$(PATH)$(SEP)wpabs.xih

EXPORTS      = &
#		LoadPMWP 	                  .1 	, &
#		FSQueryDriveData              .2 	, &
#		WPFileSystemCClassData 	      .10 , &
#		WPFilesystemClassData         .11 , &
#		WPFilesystemNewClass 	      .12 , &
#		WPFolderCClassData            .13 , &
#		WPFolderClassData 	          .14 , &
#		WPFolderNewClass              .15 , &
#		WPDataFileCClassData 	      .16 , &
#		WPDataFileClassData           .17 , &
#		WPDataFileNewClass 	          .18 , &
		WPObjectCClassData            .22 , &
		WPObjectClassData 	          .23 , &
		WPObjectNewClass              .24 , &
		WPAbstractCClassData 	      .25 , &
		WPAbstractClassData           .26 , &
		WPAbstractNewClass 	          .27 , &
#		WPTemplatesCClassData         .28 , &
#		WPTemplatesClassData 	      .29 , &
#		WPTemplatesNewClass           .30 , &
#		WPShadowCClassData 	          .31 , &
#		WPShadowClassData             .32 , &
#		WPShadowNewClass 	          .33 , &
		WPDiskCClassData              .34 , &
		WPDiskClassData 	          .35 , &
		WPDiskNewClass                .36 , &
#		WPStartupCClassData 	      .37 , &
#		WPStartupClassData            .38 , &
#		WPStartupNewClass 	          .39 , &
#		WPTransientCClassData         .40 , &
#		WPTransientClassData 	      .41 , &
#		WPTransientNewClass           .42 , &
#		WPDrivesCClassData 	          .43 , &
#		WPDrivesClassData             .44 , &
#		WPDrivesNewClass 	          .45 , &
		WPProgramCClassData           .46 , &
		WPProgramClassData 	          .47 , &
		WPProgramNewClass             .48 , &
#		WPProgramFileCClassData 	  .49 , &
#		WPProgramFileClassData        .50 , &
#		WPProgramFileNewClass 	      .51 , &
#		WPShredderCClassData          .52 , &
#		WPShredderClassData 	      .53 , &
#		WPShredderNewClass            .54 , &
#		WPDesktopCClassData 	      .55 , &
#		WPDesktopClassData            .56 , &
#		WPDesktopNewClass 	          .57 , &
#		WPRootFolderCClassData        .58 , &
#		WPRootFolderClassData 	      .59 , &
#		WPRootFolderNewClass          .60 , &
#		WPIconCClassData 	          .61 , &
#		WPIconClassData               .62 , &
#		WPIconNewClass 	              .63 , &
#		WPPointerCClassData           .64 , &
#		WPPointerClassData 	          .65 , &
#		WPPointerNewClass             .66 , &
#		WPBitmapCClassData 	          .67 , &
#		WPBitmapClassData             .68 , &
#		WPBitmapNewClass 	          .69 , &
#		WPCommandFileCClassData       .85 , &
#		WPCommandFileClassData 	      .86 , &
#		WPCommandFileNewClass         .87 , &
#		WPClassManagerCClassData 	  .88 , &
#		WPClassManagerClassData       .89 , &
#		WPClassManagerNewClass 	      .90 , &
#		WPPifCClassData               .91 , &
#		WPPifClassData 	              .92 , &
#		WPPifNewClass                 .93 , &
#		WPMetCClassData 	          .94 , &
#		WPMetClassData                .95 , &
#		WPMetNewClass 	              .96 , &
#		RestoreDesktop                .101, &
#		Strcat 	                      .102, &
#		Memset                        .103, &
#		Strcpy 	                      .105, &
#		Strlen                        .106, &
#		Itoa 	                      .107, &
#		Strchr                        .108, &
#		Strrchr 	                  .109, &
#		Memcpy                        .110, &
#		Strcmp 	                      .111, &
#		Atoi                          .112, &
#		Ultoa 	                      .113, &
#		Isdbcs                        .114, &
#		Nextpch 	                  .115, &
#		Prevpch                       .116, &
#		Strcmpi 	                  .117, &
#		Strncmpi                      .118, &
#		Strncat_trunc 	              .119, &
#		Strncat                       .120, &
#		Strncpy_trunc 	              .121, &
#		Strncpy                       .122, &
#		Strstr 	                      .123, &
#		Strupr                        .124, &
#		Xlatblank 	                  .125, &
#		Atol                          .126, &
#		Strncmp 	                  .127, &
#		Strpbrk                       .128, &
#		Stripblanks 	              .129, &
#		Cleanupdesktop                .130, &
#		WinStartShell 	              .145, &
#		ShlInsertContextMenuItems     .147, &
#		ShlInsertSettingsPage 	      .148, &
#		WinShutdownSystem             .149, &
#		Allocmem 	                  .150, &
#		Freemem                       .151, &
#		WinShutdownAndReboot 	      .152, &
#		WinShutdown                   .153, &
#		WinRegisterObjectClass 	      .200, &
#		WinDeRegisterObjectClass      .201, &
#		WinCreateObject  	          .202, & # - LA???
#		MriLoadResource               .203, &
#		ShlCreateContextMenu 	      .204, &
#		WinEnumObjectClasses          .205, &
#		ShlQueryFolderFromPath 	      .206, &
#		WinStoreWindowPos             .207, &
#		WinRestoreWindowPos 	      .208, &
#		WinLoadFileIcon               .209, &
#		WinSetFileIcon 	              .210, &
		ShlStartWorkplace             .211, &
		ShlExceptionHandler 	      .212, &
		ShlSaveEnv                    .213, &
		ShlRestoreEnv 	              .214, &
#		Memcmp                        .215, &
#		WinFreeFileIcon 	          .216, &
#		ShlQueryIconGivenFEAl2        .217, &
#		ShlLoadIcon 	              .218, &
#		WinReplaceObjectClass         .219, &
#		DiskFilter 	                  .220, &
#		WinRestartWorkplace           .221, &
#		ReportMessage 	              .223, &
#		ShlGetUserWordPtr             .224, &
#		DoNetLogon 	                  .225, &
#		DoNetLogoff                   .226, &
#		DoNetError 	                  .227, &
#		DoNetConnect                  .228, &
#		DoNetDisconnect 	          .229, &
#		NwiEnumCache                  .230, &
#		NwiEnumNetwork 	              .231, &
#		NwiQueryCache                 .232, &
#		NwiQueryNetwork 	          .233, &
#		NwiGetNewName                 .234, &
#		NwiAuthDefaults 	          .235, &
#		NwiAuthenticate               .236, &
#		NwiUnauthenticate 	          .237, &
#		NwiConnect                    .238, &
#		NwiDisconnect 	              .239, &
#		NwiAllocUnc                   .240, &
#		NwiFreeUnc 	                  .241, &
#		NwiGetConnList                .242, &
#		NwiIL 	                      .243, &
#		NwiIP                         .244, &
#		NwtFillCombobox 	          .245, &
#		NwiQueryNetCaps               .246, &
#		NwiQueryServerVersion 	      .247, &
#		NwiEnumInfo                   .248, &
#		NwiQueryInfo 	              .249, &
#		WinSetObjectData              .250, &
#		WinDestroyObject 	          .251, &
#		WinQueryObject                .252, &
#		_vaPsz 	                      .253, &
#		_AddRunningProgram            .254, &
#		_CreateDuplicatePointer 	  .255, &
#		_GetFrameHandle               .256, &
#		_RelocateObjectMinWindows 	  .257, &
#		_SetSysMenu                   .258, &
#		_SetupFontSample 	          .260, &
#		_SubstituteStrings            .261, &
#		WinQueryActiveDesktopPathname .262, &
#		WinQueryObjectPath            .263, &
#		_pszData 	                  .264, &
#		_vButtonType                  .265, &
#		_vDefButtonType 	          .266, &
#		_vDefMinWindows               .267, &
#		_vMinWindows 	              .268, &
#		_vCxIcon                      .269, &
#		_vCxScreen 	                  .270, &
#		_vCxScroll                    .271, &
#		_vCyIcon 	                  .272, &
#		_vCyScreen                    .273, &
#		_vCyScroll 	                  .274, &
#		_vhModMri                     .276, &
#		_vhwndDesktop 	              .277, &
#		_vhwndDesktopFrame            .278, &
#		_vhwndHelpInstance 	          .279, &
#		_vhwndTaskList                .280, &
#		WinCreateObject      	      .281, & #- GA?
#		WinUnlockSystem               .282, &
#		WinLockupSystem 	          .283, &
		MessageLoopProc               .284, &
#		WinSaveObject 	              .285, &
#		WinOpenObject                 .286, &
#		WinMoveObject 	              .287, &
#		WinCopyObject                 .288, &
#		WinCreateShadow 	          .289, &
#		M_WPFileSystemCClassData      .310, &
#		M_WPFileSystemClassData 	  .311, &
#		M_WPFileSystemNewClass        .312, &
#		M_WPFolderCClassData 	      .313, &
#		M_WPFolderClassData           .314, &
#		M_WPFolderNewClass 	          .315, &
#		M_WPDataFileCClassData        .316, &
#		M_WPDataFileClassData 	      .317, &
#		M_WPDataFileNewClass          .318, &
#		M_WPTemplatesCClassData 	  .319, &
#		M_WPTemplatesClassData        .320, &
#		M_WPTemplatesNewClass 	      .321, &
		M_WPObjectCClassData          .322, &
		M_WPObjectClassData 	      .323, &
		M_WPObjectNewClass            .324, &
		M_WPAbstractCClassData 	      .325, &
		M_WPAbstractClassData         .326, &
		M_WPAbstractNewClass 	      .327
#		M_WPShadowCClassData          .328, &
#		M_WPShadowClassData 	      .329, &
#		M_WPShadowNewClass            .330, &
		M_WPDiskCClassData 	          .331, &
		M_WPDiskClassData             .332, &
		M_WPDiskNewClass 	          .333, &
#		M_WPStartupCClassData         .334, &
#		M_WPStartupClassData 	      .335, &
#		M_WPStartupNewClass           .336, &
#		M_WPTransientCClassData 	  .337, &
#		M_WPTransientClassData        .338, &
#		M_WPTransientNewClass 	      .339, &
#		M_WPDrivesCClassData          .340, &
#		M_WPDrivesClassData 	      .341, &
#		M_WPDrivesNewClass            .342, &
		M_WPProgramCClassData 	      .343, &
		M_WPProgramClassData          .344, &
		M_WPProgramNewClass 	      .345, &
#		M_WPProgramFileCClassData     .346, &
#		M_WPProgramFileClassData 	  .347, &
#		M_WPProgramFileNewClass       .348, &
#		M_WPShredderCClassData 	      .349, &
#		M_WPShredderClassData         .350, &
#		M_WPShredderNewClass 	      .351, &
#		M_WPDesktopCClassData         .355, &
#		M_WPDesktopClassData 	      .356, &
#		M_WPDesktopNewClass           .357, &
#		M_WPRootFolderCClassData 	  .358, &
#		M_WPRootFolderClassData       .359, &
#		M_WPRootFolderNewClass 	      .360, &
#		M_WPIconCClassData            .361, &
#		M_WPIconClassData 	          .362, &
#		M_WPIconNewClass              .363, &
#		M_WPPointerCClassData 	      .364, &
#		M_WPPointerClassData          .365, &
#		M_WPPointerNewClass 	      .366, &
#		M_WPBitmapCClassData          .367, &
#		M_WPBitmapClassData 	      .368, &
#		M_WPBitmapNewClass            .369, &
#		M_WPCommandFileCClassData 	  .385, &
#		M_WPCommandFileClassData      .386, &
#		M_WPCommandFileNewClass 	  .387, &
#		M_WPNetworkCClassData         .388, &
#		M_WPNetworkClassData 	      .389, &
#		M_WPNetworkNewClass           .390, &
#		M_WPNetgrpCClassData 	      .391, &
#		M_WPNetgrpClassData           .392, &
#		M_WPNetgrpNewClass 	          .393, &
#		M_WPServerCClassData          .394, &
#		M_WPServerClassData 	      .395, &
#		M_WPServerNewClass            .396, &
#		M_WPSharedDirCClassData 	  .397, &
#		M_WPSharedDirClassData        .398, &
#		M_WPSharedDirNewClass 	      .399, &
#		M_WPNetLinkCClassData         .400, &
#		M_WPNetLinkClassData 	      .401, &
#		M_WPNetLinkNewClass           .402, &
#		M_WPPifCClassData 	          .403, &
#		M_WPPifClassData              .404, &
#		M_WPPifNewClass 	          .405, &
#		M_WPMetCClassData             .406, &
#		M_WPMetClassData 	          .407, &
#		M_WPMetNewClass               .408, &
#		WPNetworkCClassData 	      .409, &
#		WPNetworkClassData            .410, &
#		WPNetworkNewClass 	          .411, &
#		WPSharedDirCClassData         .412, &
#		WPSharedDirClassData 	      .413, &
#		WPSharedDirNewClass           .414, &
#		WPServerCClassData 	          .415, &
#		WPServerClassData             .416, &
#		WPServerNewClass 	          .417, &
#		WPNetLinkCClassData           .418, &
#		WPNetLinkClassData 	          .419, &
#		WPNetLinkNewClass             .420, &
#		WPNetgrpCClassData 	          .421, &
#		WPNetgrpClassData             .422, &
#		WPNetgrpNewClass 	          .423, &
#		VPIDWorkplace                 .424, &
#		SchemeBackgroundDlgProc 	  .427, &
#		ShlStoreString                .428, &
#		ShlFreeString 	              .429, &
#		EncodeBackgroundData          .430, &
#		DecodeBackgroundData 	      .431, &
#		UnloadUnitBitmap              .432, &
#		WPMinWindowCClassData 	      .433, &
#		WPMinWindowClassData          .434, &
#		M_WPMinWindowNewClass 	      .435, &
#		WPMinWindowNewClass           .436, &
#		WPCnrViewCClassData 	      .437, &
#		WPCnrViewClassData            .438, &
#		M_WPCnrViewNewClass 	      .439, &
#		WPCnrViewNewClass             .440, &
#		M_WPFindFolderNewClass 	      .441, &
#		WPFindFolderNewClass          .442, &
#		M_WPProgramGroupNewClass 	  .443, &
#		WPDiskCVNewClass              .444, &
#		RWMonitorCClassData 	      .445, &
#		RWMonitorClassData            .446, &
#		RWMonitorNewClass 	          .447, &
#		WPFilterCClassData            .448, &
#		WPFilterClassData 	          .449, &
#		WPFilterNewClass              .450, &
#		M_WPFilterNewClass 	          .451, &
#		WPFinderCClassData            .452, &
#		WPFinderClassData 	          .453, &
#		WPFinderNewClass              .454, &
#		M_WPFinderNewClass 	          .455, &
#		WPMinWinViewerCClassData      .456, &
#		WPMinWinViewerClassData 	  .457, &
#		WPMinWinViewerNewClass        .458, &
#		M_WPMinWinViewerNewClass 	  .459, &
#		WPProgramGroupCClassData      .460, &
#		WPProgramGroupClassData 	  .461, &
#		WPProgramGroupNewClass        .462, &
#		WinRestartWPDServer 	      .463, &
#		WinRestartSomDD               .464, &
#		WinIsWPDServerReady 	      .465, &
#		VHwndWorkplaceWindow          .466, &
#		WPLaunchpadCClassData 	      .467, &
#		WPLaunchpadClassData          .468, &
#		WPLaunchpadNewClass 	      .469, &
#		M_WPLaunchpadCClassData       .470, &
#		M_WPLaunchpadClassData 	      .471, &
#		M_WPLaunchpadNewClass         .472, &
#		ShlRealizeDefaultPalette 	  .473, &
#		VHPalDefaultColors            .474, &
#		VFPaletteManagedDisplay 	  .475, &
#		FindOneObject                 .476, &
#		ShlCreateTempFile 	          .477, &
#		ShlDeleteTempFile             .478, &
#		VSZDesktopFolder 	          .479, &
#		WinIsSomDDReady               .480, &
#		ShlUnloadServer 	          .481, &
#		NotifyObjectInCnrOpened       .482, &
#		PMWPDateToString 	          .483, &
#		M_WPCnrViewCClassData         .484, &
#		M_WPCnrViewClassData 	      .485, &
#		VFConfirmations               .486, &
#		VFConfirmationsLoaded 	      .487, &
#		VULWPSDSOMThreadID            .488, &
#		LWPDOpenedOnWPDServer 	      .489, &
#		ShlStartApp                   .490, &
#		WPImageFileCClassData 	      .491, &
#		WPImageFileClassData          .492, &
#		WPImageFileNewClass 	      .493, &
#		M_WPImageFileCClassData       .494, &
#		M_WPImageFileClassData 	      .495, &
#		M_WPImageFileNewClass         .496, &
#		ODWPSCClassData 	          .497, &
#		ODWPSClassData                .498, &
#		ODWPSNewClass 	              .499, &
#		M_ODWPSCClassData             .500, &
#		M_ODWPSClassData 	          .501, &
#		M_ODWPSNewClass               .502, &
#		FileSysQueryAwakeObject 	  .503, &
#		NodeMovedNotify               .504, &
#		Ultoah 	                      .505, &
#		RunOnThread                   .506, &
#		SHCRT_ToUpper 	              .507, &
#		SubstituteString              .508, &
#		BuildShortFilename 	          .509, &
#		BuildNameFromNumber           .510, &
#		WinNotebookButtonFromID 	  .511, &
#		WinWaitForShell               .512, &
#		VDriveData                    .513


DEST     = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_pm.mk

#TARGETS   = $(IDLS) $(IDLS2) $(HEADERS) $(IHEADERS) $(OBJS)
			
SOURCES =	$(SRC_DIR)$(SEP)wpobject.idl &
			$(SRC_DIR)$(SEP)wpdisk.idl &
			$(SRC_DIR)$(SEP)wppgm.idl &
			$(SRC_DIR)$(SEP)wpshadow.idl &
			$(SRC_DIR)$(SEP)wptypes.idl

SOURCES2 =	$(MYDIR)$(SEP)wpabs.idl

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
