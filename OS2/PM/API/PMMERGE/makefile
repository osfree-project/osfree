#
# A Makefile for pmmerge.dll
# (c) osFree project,
# author, date
#

PROJ     = pmmerge
DESC     = OS/2 PM Merged Library
DIRS     = rectangles habmgr messagemgr framemgr windowmgr input load dialogs errors
srcfiles = $(p)init$(e) $(p)framemgr$(SEP)framemgr$(e) $(p)habmgr$(SEP)habmgr$(e) $(p)rectangles$(SEP)rectangles$(e) &
           $(p)input$(SEP)input$(e) $(p)messagemgr$(SEP)queue$(e) $(p)messagemgr$(SEP)messages$(e) &
           $(p)windowmgr$(SEP)windowmgr$(e) $(p)windowmgr$(SEP)WindowClass$(e) &
           $(p)errors$(SEP)errors$(e) $(p)errors$(SEP)f_errors$(e) $(p)load$(SEP)load$(e) &
           $(p)Fc_config$(e) $(p)util$(e) $(p)F_debug$(e) $(p)unimpl$(e) # $(p)dialogs$(SEP)windlgbox$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)include  -sg -of+
ADD_LINKOPT  = segment   type DATA nonshared lib os2386.lib lib clib3r.lib
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata, caseexact, start=dll_initterm

IMPORTS = RectCopy utlapi.1,                    &
          RectSet utlapi.2,                     &
          RectIsEmpty utlapi.3,                 &
          RectEqual utlapi.4,                   &
          RectSetEmpty utlapi.5,                &
          RectOffset utlapi.6,                  &
          RectInflate utlapi.7,                 &
          RectPtIn utlapi.8,                    &
          RectIntersect utlapi.9,               &
          RectUnion utlapi.10,                  &
          RectSubtract utlapi.11,               &
          RectMakeRect utlapi.12,               &
          RectMakePoints utlapi.13

EXPORTS = &
# pmshapi
        ShePIInitialise				.2000, &
	WinQueryProfileInt			.2001, &
	WinQueryProfileString			.2002, &
	WinWriteProfileString			.2003, &
	WinQueryProfileSize			.2004, &
	WinQueryProfileData			.2005, &
	WinWriteProfileData			.2006, &
	PrfChangeWriteThru			.2007, &
	PMExecRegister				.2008, &
	ShePIInitIniFiles			.2009, &
	SheConvert190ProgramListTo300		.2010, &
	PrfQueryProfileInt			.2011, &
	PrfQueryProfileString			.2012, &
	PrfWriteProfileString			.2013, &
	PrfQueryProfileSize			.2014, &
	PrfQueryProfileData			.2015, &
	PrfWriteProfileData			.2016, &
	PrfOpenProfile				.2017, &
	PrfCloseProfile				.2018, &
	PrfReset				.2019, &
	PrfQueryProfile				.2020, &
	PrfAddProgram				.2021, &
	PrfRemoveProgram			.2022, &
	PrfChangeProgram			.2023, &
	PrfQueryDefinition			.2024, &
	PrfQueryProgramTitles			.2025, &
	PrfCreateGroup				.2026, &
	PrfQueryProgramHandle			.2027, &
	PrfQueryProgramCategory			.2028, &
	PrfDestroyGroup				.2029, &
	SheProfileHook				.2030, &
# os2sm, pmshapi, spl1b
	SheInitializeIniFile			.2031, &
# pmshapi
	ShpPI16LockIniFile			.2032, &
	ShpPI16Shutdown				.2033, &
#
	EnablePMPre				.2034, &
# pmgre
	InnerGreEntry				.2035, &
	GreInitialize				.2036, &
	GreEntry2				.2037, &
	GreEntry3				.2038, &
	GreEntry4				.2039, &
	GreEntry5				.2040, &
	GreEntry6				.2041, &
	GreEntry7				.2042, &
	GreEntry8				.2043, &
	GreEntry9				.2044, &
	GreEntry10				.2045, &
	GetDriverInfo				.2046, &
	SetDriverInfo				.2047, &
	PostDeviceModes				.2048, &
	BitmapLimitWarning			.2049, &
	PmWinGreEntry2				.2050, &
	PmWinGreEntry3				.2051, &
	PmWinGreEntry4				.2052, &
	PmWinGreEntry5				.2053, &
	PmWinGreEntry6				.2054, &
	PmWinGreEntry7				.2055, &
	PmWinGreEntry8				.2056, &
	PmWinGreEntry9				.2057, &
	PmWinGreEntry10				.2058, &
	SSAllocSeg				.2059, &
	SSAllocHuge				.2060, &
	SSFreeSeg				.2061, &
	SSFlushSegs				.2062, &
	RamSemRequest16				.2063, &
	RamSemClear16				.2064, &
# pmwin
	WinQueryClassInfo			.2065, &
	WinMultWindowFromIDs			.2066, &
	WinLockWindow				.2067, &
	WinQueryWindowLockCount			.2068, &
	WinLoadPointer				.2069, &
	WinCreatePointer			.2070, &
	WinDestroyPointer			.2071, &
	OldWinQueryPointerInfo			.2072, &
	WinQuerySysPointer			.2073, &
	WinGetSysBitmap				.2074, &
	WinQueryCursorInfo			.2075, &
	WinLoadAccelTable			.2076, &
	WinCreateAccelTable			.2077, &
	WinDestroyAccelTable			.2078, &
	WinCopyAccelTable			.2079, &
	WinSetClipbrdData			.2080, &
	WinQueryClipbrdFmtInfo			.2081, &
	GpiLoadBitmap				.2082, &
	WinLoadString				.2083, &
	WinLoadMessage				.2084, &
	WinSetRectEmpty				.2085, &
	WinSetRect				.2086, &
	WinCopyRect				.2087, &
	WinIsRectEmpty				.2088, &
	WinPtInRect				.2089, &
	WinOffsetRect				.2090, &
	WinInflateRect				.2091, &
	WinMakeRect				.2092, &
	WinMakePoints				.2093, &
	WinDefWindowProc			.2094, &
	WinDefDlgProc				.2095, &
	WinQueryHungProcess			.2096, &
	WinLoadStringTable			.2097, &
	WinLoadVkeyGlyphXlatetbl		.2098, &
	WinLoadCharXlatetbl			.2099, &
	WinSetVkeyGlyphXlatetbl			.2100, &
	WinQueryVkeyGlyphXlatetbl		.2101, &
	WinVkeyToScanID				.2102, &
	WinQueryProcessCP			.2103, &
	WinGetKeyState				.2104, &
	WinSetCp				.2105, &
	WinQueryCp				.2106, &
	WinQueryCpList				.2107, &
	WinCpTranslateString			.2108, &
	WinCpTranslateChar			.2109, &
	WinCompareStrings			.2110, &
	WinUpper				.2111, &
	WinUpperChar				.2112, &
	WinNextChar				.2113, &
	WinPrevChar				.2114, &
	WinCreateHeap				.2115, &
	WinDestroyHeap				.2116, &
	WinAvailMem				.2117, &
	WinAllocMem				.2118, &
	WinReallocMem				.2119, &
	WinFreeMem				.2120, &
	WinLockHeap				.2121, &
	WinCreateAtomTable			.2122, &
	WinDestroyAtomTable			.2123, &
	WinAddAtom				.2124, &
	WinFindAtom				.2125, &
	WinDeleteAtom				.2126, &
	WinQueryAtomUsage			.2127, &
	WinQueryAtomLength			.2128, &
	WinQueryAtomName			.2129, &
	WinQuerySystemAtomTable			.2130, &
	WinSetLastError				.2131, &
	WinGetLastError				.2132, &
	WinGetErrorInfo				.2133, &
	WinFreeErrorInfo			.2134, &
	WinInitialize				.2135, &
	WinTerminate				.2136, &
	WinCatch				.2137, &
	WinThrow				.2138, &
	WinQueryVersion				.2139, &
	FsrSemExit				.2140, &
	FsrSemCheck				.2141, &
	AabGetReg				.2142, &
	AabSetReg				.2143, &
	ValidateSelector			.2144, &
	LoadModuleHandle			.2145, &
	GetScreenSelector			.2146, &
	IdentifyCodeSelector			.2147, &
	_WinSetErrorInfo			.2148, &
	WinTerminateSystem			.2149, &
	WinDefQueueProc				.2150, &
	WinVisRegionNotify			.2151, &
	WinQueryKbdLayout			.2152, &
	WinSetKbdLayout				.2153, &
	WinBuildPtrHandle			.2154, &
	DumWinCreateHelpInstance		.2155, &
	DimWinDestroyHelpInstance		.2156, &
	DumWinAssociateHelpInstance		.2157, &
	DimWinCreateHelpTable			.2158, &
	DumWinLoadHelpTable			.2159, &
	DumWinQueryHelpInstance			.2160, &
	DummyHelpEntry				.2161, &
	GetPMDDCodeSelector			.2162, &
	WinQueryPwnd				.2163, &
	WinEnumVKeyScanIDs			.2164, &
	WinLoadTranslationTable			.2165, &
	WinSetKbdLayout2			.2166, &
	DebugInputEnable			.2167, &
	DebugInput				.2168, &
	DebugOutput				.2169, &
	DebugCurPos				.2170, &
	DebugCls				.2171, &
	DebugRedirect				.2172, &
	WinLangsupWindowProc			.2173, &
	WinLangsupDlgProc			.2174, &
	WinCreatePointerIndirect		.2175, &
	WinQueryPointerInfo			.2176, &
	WinPortHoleUtil				.2177, &
	WinCreateFarHeap			.2178, &
	WinDestroyFarHeap			.2179, &
	WinAllocFarMem				.2180, &
	WinFreeFarMem				.2181, &
	WinReallocFarMem			.2182, &
	WinSetQueueProc				.2183, &
# pmwin
	FarFlushBufHook				.3000, &
# os2sm, pmshapi, spl1b
	fnBadAppDlgProc				.3001, &
# pmwin
	InputGetDeviceInfo16			.3002, &
# os2sm, pmshapi, spl1b
	lpfnIconWndProc				.3003, &
	lpfnShellWndProc			.3004, &
	SheStartLongProgram			.3005, &
	SheSystemShutdown			.3006, &
# pmshapi
	SheUpdateIniFile			.3007, &
# os2sm, pmshapi, spl1b
	ShlLoadPublicFonts			.3008, &
	ShlUnloadPublicFonts			.3009, &
# pmshapi
	ShpLockProgramList			.3010, &
	ShpUnlockProgramList			.3011, &
# os2sm, pmshapi, spl1b
	StartStartUpDotCmd			.3012, &
	StartSystemExecutables			.3013, &
# pmwin
	WinAlarm				.3014, &
	WinBeginEnumWindows			.3015, &
	WinBeginPaint				.3016, &
	WinBroadcastMsg				.3017, &
	WinCancelMode				.3018, &
	WinCancelShutdown			.3019, &
	WinCreateMsgQueue			.3020, &
	WinCreateStdWindow			.3021, &
	WinCreateWindow				.3022, &
	WinDestroyMsgQueue			.3023, &
	WinDestroyWindow			.3024, &
	WinDispatchMsg				.3025, &
	WinDrawText				.3026, &
	WinEndPaint				.3027, &
	WinFillRect				.3028, &
	WinGetMinWindowViewer			.3029, &
	WinGetMsg				.3030, &
	WinGetNextWindow			.3031, &
	WinInitializeSystem			.3032, &
	WinInvalidateRect			.3033, &
	WinIsChild				.3034, &
	WinIsWindow				.3035, &
	WinIsWindowVisible			.3036, &
	WinLoadDlg				.3037, &
	WinLockStartInput			.3038, &
	WinMessageBox				.3039, &
	WinMsgSemWait				.3040, &
	WinOpenWindowDC				.3041, &
	WinPeekMsg				.3042, &
	WinPostMsg				.3043, &
	WinProcessDlg				.3044, &
	WinQueryActiveWindow			.3045, &
	WinQueryAnchorBlock			.3046, &
	WinQueryDesktopWindow			.3047, &
	WinQueryFocus				.3048, &
	WinQueryQueueInfo			.3049, &
	WinQuerySysModalWindow			.3050, &
	WinQuerySysValue			.3051, &
	WinQueryWindow				.3052, &
	WinQueryWindowPos			.3053, &
	WinQueryWindowProcess			.3054, &
	WinQueryWindowRect			.3055, &
	WinQueryWindowULong			.3056, &
	WinQueryWindowUShort			.3057, &
	WinRegisterClass			.3058, &
	WinResetSystem				.3059, &
	WinScreenSwitch				.3060, &
	WinSendDlgItemMsg			.3061, &
	WinSendMsg				.3062, &
	WinSetActiveWindow			.3063, &
	WinSetDlgItemText			.3064, &
	WinSetFocus				.3065, &
	WinSetMultWindowPos			.3066, &
	WinSetParent				.3067, &
	WinSetPresParam				.3068, &
	WinSetSysModalWindow			.3069, &
	WinSetSysValue				.3070, &
	WinSetWindowBits			.3071, &
	WinSetWindowPos				.3072, &
	WinSetWindowText			.3073, &
	WinShowWindow				.3074, &
	WinSubclassWindow			.3075, &
	WinTerminateShutdown			.3076, &
	WinTrackRect				.3077, &
	WinWindowFromID				.3078, &
# pmshapi
	WinAddProgram				.3079, &
# os2sm, pmshapi, spl1b
	WinAddSwitchEntry			.3080, &
# pmshapi
	WinAddToGroup				.3081, &
# pmwin
	WinCalcFrameRect			.3082, &
	WinCallHelpHook				.3083, &
	WinCallMsgFilter			.3084, &
# os2sm, pmshapi, spl1b
	WinCanTerminate				.3085, &
# pmshapi
	WinChangeProgram			.3086, &
# os2sm, pmshapi, spl1b
	WinChangeSwitchEntry			.3087, &
# pmwin
	WinCheckInput				.3088, &
	WinCloseClipbrd				.3089, &
# os2sm, pmshapi, spl1b
	WinCplRegister				.3090, &
# pmwin
	WinCreateCursor				.3091, &
	WinCreateDlg				.3092, &
	WinCreateFrameControls			.3093, &
# pmshapi
	WinCreateGroup				.3094, &
# pmwin
	WinCreateMenu				.3095, &
# os2sm, pmshapi, spl1b
	WinCreateSwitchEntry			.3096, &
# pmwin
	WinDdeInitiate				.3097, &
	WinDdePostMsg				.3098, &
	WinDdeRespond				.3099, &
	WinDebugBreak				.3100, &
	WinDeleteLibrary			.3101, &
	WinDeleteProcedure			.3102, &
	WinDestroyAniPointer			.3103, &
	WinDestroyCursor			.3104, &
# pmshapi
	WinDestroyGroup				.3105, &
# pmwin
	WinDismissDlg				.3106, &
	WinDlgBox				.3107, &
	WinDrawBitmap				.3108, &
	WinDrawBorder				.3109, &
	WinDrawPointer				.3110, &
	WinEmptyClipbrd				.3111, &
	WinEnablePhysInput			.3112, &
	WinEnableWindow				.3113, &
	WinEnableWindowUpdate			.3114, &
	WinEndEnumWindows			.3115, &
# os2sm, pmshapi, spl1b
	WinEndProgram				.3116, &
	WinEndSession				.3117, &
	WinEndWindowSession			.3118, &
# pmwin
	WinEnumClipbrdFmts			.3119, &
	WinEnumDlgItem				.3120, &
	WinEqualRect				.3121, &
	WinExcludeUpdateRegion			.3122, &
	WinExplodeWindow			.3123, &
	WinFlashWindow				.3124, &
	WinFocusChange				.3125, &
	WinForceMenuRedraw			.3126, &
	WinFormatFrame				.3127, &
	WinGetClipPS				.3128, &
	WinGetCurrentTime			.3129, &
	WinGetDlgMsg				.3130, &
	WinGetErasePS				.3131, &
	WinGetFrameTreePPs			.3132, &
	WinGetFrameTreePPSize			.3133, &
	WinGetMaxPosition			.3134, &
	WinGetMinPosition			.3135, &
	WinGetPhysKeyState			.3136, &
	WinGetPS				.3137, &
	WinGetScanState				.3138, &
	WinGetScreenPS				.3139, &
# pmshapi
	WinHAPPfromPID				.3140, &
	WinHSWITCHfromHAPP			.3141, &
	WinInitializePL				.3142, &
# os2sm, pmshapi, spl1b
	WinInitSession				.3143, &
	WinInitSessionMgr			.3144, &
	WinInitSwEntry				.3145, &
# pmwin
	WinInSendMsg				.3146, &
# os2sm, pmshapi, spl1b
	WinInstStartApp				.3147, &
# pmwin
	WinIntersectRect			.3148, &
	WinInvalidateRegion			.3149, &
	WinInvertRect				.3150, &
	WinIsPhysInputEnabled			.3151, &
	WinIsShuttingDown			.3152, &
	WinIsThreadActive			.3153, &
	WinIsWindowEnabled			.3154, &
	WinIsWindowShowing			.3155, &
	WinJiggleMouse				.3156, &
	WinLoadAniPointer			.3157, &
	WinLoadLibrary				.3158, &
	WinLoadMenu				.3159, &
	WinLoadProcedure			.3160, &
	WinLockInput				.3161, &
	WinLockPointerUpdate			.3162, &
	WinLockVisRegions			.3163, &
	WinLockWindowUpdate			.3164, &
	WinMapDlgPoints				.3165, &
	WinMapWindowPoints			.3166, &
	WinMenuIndexFromID			.3167, &
	WinMsgMuxSemWait			.3168, &
# os2sm, pmshapi, spl1b
	WinNoShutdown				.3169, &
# pmwin
	WinOpenClipbrd				.3170, &
# os2sm, pmshapi, spl1b
	WinPMFileRegister			.3171, &
# pmwin
	WinPopupMenu				.3172, &
	WinPostQueueMsg				.3173, &
	WinQueryAccelTable			.3174, &
	WinQueryAccelTableValue			.3175, &
	WinQueryCapture				.3176, &
	WinQueryCharXlatetbl			.3177, &
	WinQueryClassName			.3178, &
	WinQueryClassThunkProc			.3179, &
	WinQueryClipbrdData			.3180, &
	WinQueryClipbrdOwner			.3181, &
	WinQueryClipbrdViewer			.3182, &
# pmshapi
	WinQueryDefinition			.3183, &
# pmwin
	WinQueryDesktopBkgnd			.3184, &
	WinQueryDlgItemShort			.3185, &
	WinQueryDlgItemText			.3186, &
	WinQueryDlgItemTextLength		.3187, &
# os2sm, pmshapi, spl1b
	WinQueryExtIDFocus			.3188, &
# pmshapi
	WinQueryFileExtOpts			.3189, &
# pmwin
	WinQueryMenuData			.3190, &
	WinQueryMsgPos				.3191, &
	WinQueryMsgTime				.3192, &
	WinQueryObjectWindow			.3193, &
	WinQueryPointer				.3194, &
	WinQueryPointerPos			.3195, &
	WinQueryPresParam			.3196, &
# pmshapi
	WinQueryProgramTitles			.3197, &
	WinQueryProgramType			.3198, &
	WinQueryProgramUse			.3199, &
# pmwin
	WinQueryQueueStatus			.3200, &
	WinQuerySendMsg				.3201, &
# os2sm, pmshapi, spl1b
	WinQuerySessionTitle			.3202, &
# pmwin
	WinQueryState				.3203, &
# os2sm, pmshapi, spl1b
	WinQuerySwitchEntry			.3204, &
	WinQuerySwitchHandle			.3205, &
	WinQuerySwitchList			.3206, &
# pmwin
	WinQuerySysColor			.3207, &
# os2sm, pmshapi, spl1b
	WinQueryTaskSizePos			.3208, &
	WinQueryTaskTitle			.3209, &
# pmwin
	WinQueryUpdateRect			.3210, &
	WinQueryUpdateRegion			.3211, &
	WinQueryWindowDC			.3212, &
	WinQueryWindowModel			.3213, &
	WinQueryWindowPtr			.3214, &
	WinQueryWindowSGID			.3215, &
	WinQueryWindowText			.3216, &
	WinQueryWindowTextLength		.3217, &
	WinQueryWindowThunkProc			.3218, &
	WinQueryWorkplaceWindow			.3219, &
	WinQueueFromID				.3220, &
	WinRealizePalette			.3221, &
	WinRegisterSeamlessProc			.3222, &
	WinRegisterUserDatatype			.3223, &
	WinRegisterUserMsg			.3224, &
	WinRegisterWindowDestroy		.3225, &
	WinReleaseErasePS			.3226, &
	WinReleaseHook				.3227, &
	WinReleasePS				.3228, &
# pmshapi
	WinRemoveFromGroup			.3229, &
# pmwin
	WinRemovePresParam			.3230, &
# pmshapi
	WinRemoveProgram			.3231, &
# os2sm, pmshapi, spl1b
	WinRemoveSwitchEntry			.3232, &
# pmwin
	WinReplyMsg				.3233, &
	WinSaveWindowPos			.3234, &
	WinScrollWindow				.3235, &
	WinSeamlessBox				.3236, &
	WinSemEnter				.3237, &
	WinSemLeave				.3238, &
	WinSemQuery				.3239, &
	WinSendQueueMsg				.3240, &
	WinSetAccelTable			.3241, &
	WinSetAccelTableValue			.3242, &
	WinSetAniPointer			.3243, &
	WinSetAtomTableOwner			.3244, &
	WinSetCapture				.3245, &
	WinSetCharXlatetbl			.3246, &
	WinSetClassMsgInterest			.3247, &
	WinSetClassThunkProc			.3248, &
	WinSetClipbrdOwner			.3249, &
	WinSetClipbrdViewer			.3250, &
	WinSetDesktopBkgnd			.3251, &
	WinSetDlgItemShort			.3252, &
# os2sm, pmshapi, spl1b
	WinSetExtIDFocus			.3253, &
	WinSetFgndWindow			.3254, &
# pmshapi
	WinSetFileExtOpts			.3255, &
# pmwin
	WinSetFrameTreePPs			.3256, &
	WinSetHook				.3257, &
	WinSetKeyboardStateTable		.3258, &
	WinSetMenuData				.3259, &
	WinSetMsgInterest			.3260, &
	WinSetMsgMode				.3261, &
	WinSetOwner				.3262, &
	WinSetPointer				.3263, &
	WinSetPointerOwner			.3264, &
	WinSetPointerPos			.3265, &
	WinSetScanStateTable			.3266, &
# os2sm, pmshapi, spl1b
	WinSetSwEntry				.3267, &
# pmwin
	WinSetSynchroMode			.3268, &
	WinSetSysClipbrdViewer			.3269, &
	WinSetSysColors				.3270, &
# os2sm, pmshapi, spl1b
	WinSetTitle				.3271, &
# pmshapi
	WinSetTitleAndIcon			.3272, &
# pmwin
	WinSetWindowDC				.3273, &
	WinSetWindowPtr				.3274, &
	WinSetWindowThunkProc			.3275, &
	WinSetWindowULong			.3276, &
	WinSetWindowUShort			.3277, &
	WinShowCursor				.3278, &
	WinShowPointer				.3279, &
	WinShowTrackRect			.3280, &
# pmshapi
	WinStartApp				.3281, &
# pmwin
	WinStartTimer				.3282, &
# os2sm, pmshapi, spl1b
	WinStopProgram				.3283, &
# pmwin
	WinStopTimer				.3284, &
# os2sm, pmshapi, spl1b
 	WinStpRegister				.3285, &
# pmwin
	WinStretchPointer			.3286, &
	WinSubstituteStrings			.3287, &
	WinSubtractRect				.3288, &
# os2sm, pmshapi, spl1b
	WinSwitchProgramRegister		.3289, &
	WinSwitchToProgram2			.3290, &
	WinSwitchToProgram			.3291, &
	WinSwitchToTaskManager			.3292, &
# pmwin
	WinSyncStartPgm				.3293, &
	WinSystemShutdown			.3294, &
# os2sm, pmshapi, spl1b
	WinTerminateApp				.3295, &
# pmwin
	WinThreadAssocQueue			.3296, &
	WinTrackSeamless			.3297, &
	WinTranslateAccel			.3298, &
	WinTranslateChar			.3299, &
	WinUnionRect				.3300, &
	WinUpdateWindow				.3301, &
	WinValidateRect				.3302, &
	WinValidateRegion			.3303, &
	WinVkeyGlyphCharXlate			.3304, &
	WinWaitMsg				.3305, &
	WinWakeThread				.3306, &
	WinWindowFromDC				.3307, &
	WinWindowFromPoint			.3308, &
# pmshapi
	WinSetTitleAndHwndIcon			.3309, &
#
	WinTrackWindow				.3310, &
# pmwin
	WinSetLangInfo				.3500, &
	WinQueryLangInfo			.3501, &
	WinSetKbdLayer				.3502, &
	WinQueryKbdLayer			.3503, &
	WinSetLangViewer			.3504, &
	WinQueryLangViewer			.3505, &
# pmwin
	WinPourEvent				.3600, &
	WinSetMsgQimInfo			.3601, &
	WinQueryMsgQimInfo			.3602, &
# pmgre
	Devmode_Call_16				.4000, &
	Devname_Call_16				.4001, &
	Dispatch16Gate10			.4002, &
	Dispatch16Gate3				.4003, &
	Dispatch16Gate4				.4004, &
	Dispatch16Gate5				.4005, &
	Dispatch16Gate6				.4006, &
	Dispatch16Gate7				.4007, &
	Dispatch16Gate8				.4008, &
	Dispatch16Gate9				.4009, &
	Enable_Call_16				.4010, &
# pmwin
	FsrSemEnter				.4011, &
	FsrSemEnterP				.4012, &
	FsrSemLeave				.4013, &
# pmgre
	PFN10R2					.4014, &
	PFN4R2					.4015, &
	PFN5R2					.4016, &
	PFN6R2					.4017, &
	PFN7R2					.4018, &
	PFN8R2					.4019, &
	PFN9R2					.4020, &
	DeleteBitmap16Call			.4021, &
#
	Do16CLI					.4022, &
	Do16STI					.4023, &
	GRE16Callback				.4024, &
	Dispatch32Gate				.4025, &
	GRE16CallForwardReturn			.4026, &
	FirePSProbe				.4027, &
# pmwin
	AAB32GETREG				.5000, &
	AAB32SETREG				.5001, &
	AbnormalProcessExit			.5002, &
	DEBUG32CLS				.5003, &
	DEBUG32CURPOS				.5004, &
	DEBUG32INPUT				.5005, &
	DEBUG32OUTPUT				.5006, &
	DestroyTimers				.5007, &
#
	EDITAllocSpace				.5008, &
	EditAutoSize				.5009, &
	EditBackSpace				.5010, &
	EditChangeFocus				.5011, &
	EditChangeSelection			.5012, &
	EditChar				.5013, &
	EditCheckScroll				.5014, &
	EditClear				.5015, &
	EditCopy				.5016, &
	EditCreate				.5017, &
	EditCreateCursor			.5018, &
	EditCreatePassword			.5019, &
	EditCut					.5020, &
	EditCXFromICH				.5021, &
	EditDrawSubString			.5022, &
	EditGetFlags				.5023, &
	EditGetWindowRect			.5024, &
	EditICHFromPt				.5025, &
	EditNotify				.5026, &
	EditPaint				.5027, &
	EditPaste				.5028, &
	EditPtFromICH				.5029, &
	EditQueryPresParams			.5030, &
	EditReplace				.5031, &
	EditSetCursorPos			.5032, &
	EditSetFirstIndex			.5033, &
	EditSetInsertMode			.5034, &
	EditSetSelection			.5035, &
	EditSetText				.5036, &
	EditSetWindowParams			.5037, &
	EditShowChange				.5038, &
	EditStopMouseTracking			.5039, &
	EditTimerScroll				.5040, &
	EditTrackMouse				.5041, &
	EditVirtualKey				.5042, &
	EditWMChar				.5043, &
# pmwin
	Far32LockupHook				.5044, &
	FarCallHook				.5045, &
# pmgre
	GetDriverInfo32				.5046, &
#
	Gpi32LoadBitmap				.5047, &
# pmgre
	Gre32Entry10				.5048, &
	Gre32Entry2				.5049, &
	Gre32Entry3				.5050, &
	Gre32Entry4				.5051, &
	Gre32Entry5				.5052, &
	Gre32Entry6				.5053, &
	Gre32Entry7				.5054, &
	Gre32Entry8				.5055, &
	Gre32Entry9				.5056, &
#
	GetIconWindowsEXE			.5057, &
	GetIconWindowsICO			.5058, &
# pmgre
	InnerGre32Entry10			.5059, &
	InnerGre32Entry2			.5060, &
	InnerGre32Entry3			.5061, &
	InnerGre32Entry4			.5062, &
	InnerGre32Entry5			.5063, &
	InnerGre32Entry6			.5064, &
	InnerGre32Entry7			.5065, &
	InnerGre32Entry8			.5066, &
	InnerGre32Entry9			.5067, &
# pmwin
	InputGetDeviceInfo			.5068, &
# pmshapi
	PM32ExecRegister			.5069, &
# pmwin
	PMReleaseMutexSem			.5070, &
	PMRequestMutexSem			.5071, &
#
	PMRequestMutexSemSetWait		.5072, &
# pmgre
	PostDeviceModes32			.5073, &
# pmshapi
	Prf32AddProgram				.5074, &
	Prf32ChangeProgram			.5075, &
	Prf32ChangeWriteThru			.5076, &
	Prf32CloseProfile			.5077, &
	Prf32CreateGroup			.5078, &
	Prf32DestroyGroup			.5079, &
	Prf32OpenProfile			.5080, &
	Prf32QueryDefinition			.5081, &
	Prf32QueryProfile			.5082, &
	Prf32QueryProfileData			.5083, &
	Prf32QueryProfileInt			.5084, &
	Prf32QueryProfileSize			.5085, &
	Prf32QueryProfileString			.5086, &
	Prf32QueryProgramCategory		.5087, &
	Prf32QueryProgramHandle			.5088, &
	Prf32QueryProgramTitles			.5089, &
	Prf32RemoveProgram			.5090, &
	Prf32Reset				.5091, &
	Prf32WriteProfileData			.5092, &
	Prf32WriteProfileString			.5093, &
	PrfSuspendWriteToDisk			.5094, &
	PrfWriteToDisk				.5095, &
# pmgre
	RamSemClear32				.5096, &
	RamSemRequest32				.5097, &
	SetDriverInfo32				.5098, &
# pmshapi
	She32PIInitIniFiles			.5099, &
	She32SystemShutdown			.5100, &
	ShpPILockIniFile			.5101, &
	ShpPIShutdown				.5102, &
#
	SLEC_InitPen				.5103, &
# pmgre
	SSAllocMem				.5104, &
	SSFreeMem				.5105, &
# pmshapi
	Start32SystemExecutables		.5106, &
# pmwin
	StartTimer				.5107, &
# pmgre
	SetDeviceSurface			.5108, &
# pmwin
	ThkDos16SetFgnd				.5109, &
#
	TidCurrent				.5110, &
# pmwin
	Win32AddAtom				.5111, &
# pmshapi
	Win32AddSwitchEntry			.5112, &
# pmwin
	Win32Alarm				.5113, &
	Win32BeginEnumWindows			.5114, &
	Win32BeginPaint				.5115, &
	Win32BroadcastMsg			.5116, &
	Win32BuildPtrHandle			.5117, &
	Win32CalcFrameRect			.5118, &
#
	Win32CallHelpHook			.5119, &
# pmwin
	Win32CallMsgFilter			.5120, &
	Win32CancelShutdown			.5121, &
# pmshapi
	Win32ChangeSwitchEntry			.5122, &
# pmwin
	Win32CheckInput				.5123, &
	Win32CloseClipbrd			.5124, &
	Win32CompareStrings			.5125, &
	Win32CopyAccelTable			.5126, &
	Win32CopyRect				.5127, &
	Win32CpTranslateChar			.5128, &
	Win32CpTranslateString			.5129, &
	Win32CreateAccelTable			.5130, &
	Win32CreateAtomTable			.5131, &
	Win32CreateCursor			.5132, &
	Win32CreateDlg				.5133, &
	Win32CreateFrameControls		.5134, &
	Win32CreateMenu				.5135, &
	Win32CreateMsgQueue			.5136, &
	Win32CreatePointer			.5137, &
	Win32CreatePointerIndirect		.5138, &
	Win32CreateStdWindow			.5139, &
# pmshapi
	Win32CreateSwitchEntry			.5140, &
# pmwin
	Win32CreateWindow			.5141, &
	Win32DdeInitiate			.5142, &
	Win32DdePostMsg				.5143, &
	Win32DdeRespond				.5144, &
	Win32DebugBreak				.5145, &
	Win32DefDlgProc				.5146, &
	Win32DefWindowProc			.5147, &
	Win32DeleteAtom				.5148, &
	Win32DeleteLibrary			.5149, &
	Win32DeleteProcedure			.5150, &
	Win32DestroyAccelTable			.5151, &
	Win32DestroyAtomTable			.5152, &
	Win32DestroyCursor			.5153, &
	Win32DestroyMsgQueue			.5154, &
	Win32DestroyPointer			.5155, &
	Win32DestroyWindow			.5156, &
	Win32DismissDlg				.5157, &
	Win32DispatchMsg			.5158, &
	Win32DlgBox				.5159, &
	Win32DrawBitmap				.5160, &
	Win32DrawBorder				.5161, &
	Win32DrawPointer			.5162, &
	Win32DrawText				.5163, &
	Win32EmptyClipbrd			.5164, &
	Win32EnablePhysInput			.5165, &
	Win32EnableWindow			.5166, &
	Win32EnableWindowUpdate			.5167, &
	Win32EndEnumWindows			.5168, &
	Win32EndPaint				.5169, &
# pmshapi
	Win32EndProgram				.5170, &
# pmwin
	Win32EnumClipbrdFmts			.5171, &
	Win32EnumDlgItem			.5172, &
	Win32EqualRect				.5173, &
	Win32ExcludeUpdateRegion		.5174, &
	Win32ExplodeWindow			.5175, &
	Win32FillRect				.5176, &
	Win32FindAtom				.5177, &
	Win32FlashWindow			.5178, &
	Win32FocusChange			.5179, &
	Win32FreeErrorInfo			.5180, &
	Win32GetClipPS				.5181, &
	Win32GetCurrentTime			.5182, &
	Win32GetDlgMsg				.5183, &
	Win32GetErasePS				.5184, &
	Win32GetErrorInfo			.5185, &
	Win32GetFrameTreePPs			.5186, &
	Win32GetFrameTreePPSize			.5187, &
	Win32GetKeyState			.5188, &
	Win32GetLastError			.5189, &
	Win32GetMaxPosition			.5190, &
	Win32GetMinPosition			.5191, &
	Win32GetMsg				.5192, &
	Win32GetNextWindow			.5193, &
	Win32GetPhysKeyState			.5194, &
	Win32GetPS				.5195, &
	Win32GetScreenPS			.5196, &
	Win32GetSysBitmap			.5197, &
# pmshapi
	Win32HAPPfromPID			.5198, &
	Win32HSWITCHfromHAPP			.5199, &
# pmiwn
	Win32InflateRect			.5200, &
	Win32Initialize				.5201, &
	Win32InSendMsg				.5202, &
	Win32IntersectRect			.5203, &
	Win32InvalidateRect			.5204, &
	Win32InvalidateRegion			.5205, &
	Win32InvertRect				.5206, &
	Win32IsChild				.5207, &
	Win32IsPhysInputEnabled			.5208, &
	Win32IsRectEmpty			.5209, &
	Win32IsThreadActive			.5210, &
	Win32IsWindow				.5211, &
	Win32IsWindowEnabled			.5212, &
	Win32IsWindowShowing			.5213, &
	Win32IsWindowVisible			.5214, &
	Win32JiggleMouse			.5215, &
	Win32LoadAccelTable			.5216, &
#
	Win32LoadCharXlatetbl			.5217, &
# pmwin
	Win32LoadDlg				.5218, &
	Win32LoadLibrary			.5219, &
	Win32LoadMenu				.5220, &
	Win32LoadMessage			.5221, &
	Win32LoadPointer			.5222, &
	Win32LoadProcedure			.5223, &
	Win32LoadString				.5224, &
	Win32LockInput				.5225, &
	Win32LockPointerUpdate			.5226, &
	Win32LockVisRegions			.5227, &
	Win32LockWindowUpdate			.5228, &
	Win32MakePoints				.5229, &
	Win32MakeRect				.5230, &
	Win32MapDlgPoints			.5231, &
	Win32MapWindowPoints			.5232, &
	Win32MessageBox				.5233, &
	Win32MoveCursor				.5234, &
	Win32MsgMuxSemWait			.5235, &
	Win32MsgSemWait				.5236, &
	Win32MultWindowFromIDs			.5237, &
	Win32NextChar				.5238, &
	Win32OffsetRect				.5239, &
	Win32OpenClipbrd			.5240, &
	Win32OpenWindowDC			.5241, &
	Win32PeekMsg				.5242, &
	Win32PopupMenu				.5243, &
	Win32PostMsg				.5244, &
	Win32PostQueueMsg			.5245, &
	Win32PrevChar				.5246, &
	Win32ProcessDlg				.5247, &
	Win32PtInRect				.5248, &
	Win32QueryAccelTable			.5249, &
	Win32QueryAccelTableValue		.5250, &
	Win32QueryActiveWindow			.5251, &
	Win32QueryAnchorBlock			.5252, &
	Win32QueryAtomLength			.5253, &
	Win32QueryAtomName			.5254, &
	Win32QueryAtomUsage			.5255, &
	Win32QueryCapture			.5256, &
	Win32QueryClassInfo			.5257, &
	Win32QueryClassName			.5258, &
	Win32QueryClassThunkProc		.5259, &
	Win32QueryClipbrdData			.5260, &
	Win32QueryClipbrdFmtInfo		.5261, &
	Win32QueryClipbrdOwner			.5262, &
	Win32QueryClipbrdViewer			.5263, &
	Win32QueryCp				.5264, &
	Win32QueryCpList			.5265, &
	Win32QueryCursorInfo			.5266, &
	Win32QueryDesktopBkgnd			.5267, &
	Win32QueryDesktopWindow			.5268, &
	Win32QueryDlgItemShort			.5269, &
	Win32QueryDlgItemText			.5270, &
	Win32QueryDlgItemTextLength		.5271, &
	Win32QueryFocus				.5272, &
	Win32QueryMsgPos			.5273, &
	Win32QueryMsgTime			.5274, &
	Win32QueryObjectWindow			.5275, &
	WinQueryPaletteMode			.5276, &
	Win32QueryPointer			.5277, &
	Win32QueryPointerInfo			.5278, &
	Win32QueryPointerPos			.5279, &
	Win32QueryPresParam			.5280, &
	Win32QueryProcessCP			.5281, &
	Win32QueryQueueInfo			.5282, &
	Win32QueryQueueStatus			.5283, &
	Win32QuerySendMsg			.5284, &
# pmshapi
	Win32QuerySessionTitle			.5285, &
	Win32QuerySwitchEntry			.5286, &
	Win32QuerySwitchHandle			.5287, &
	Win32QuerySwitchList			.5288, &
# pmwin
	Win32QuerySysColor			.5289, &
	Win32QuerySysModalWindow		.5290, &
	Win32QuerySysPointer			.5291, &
	Win32QuerySysPointerData		.5292, &
	Win32QuerySystemAtomTable		.5293, &
	Win32QuerySysValue			.5294, &
# pmshapi
	Win32QueryTaskSizePos			.5295, &
	Win32QueryTaskTitle			.5296, &
# pmwin
	Win32QueryUpdateRect			.5297, &
	Win32QueryUpdateRegion			.5298, &
	Win32QueryVersion			.5299, &
	Win32QueryVisibleRegion			.5300, &
	Win32QueryWindow			.5301, &
	Win32QueryWindowDC			.5302, &
	Win32QueryWindowModel			.5303, &
	Win32QueryWindowPos			.5304, &
	Win32QueryWindowProcess			.5305, &
	Win32QueryWindowPtr			.5306, &
	Win32QueryWindowRect			.5307, &
	Win32QueryWindowText			.5308, &
	Win32QueryWindowTextLength		.5309, &
	Win32QueryWindowThunkProc		.5310, &
	Win32QueryWindowULong			.5311, &
	Win32QueryWindowUShort			.5312, &
	Win32QueueFromID			.5313, &
	Win32RealizePalette			.5314, &
	Win32RegisterClass			.5315, &
	Win32RegisterPen			.5316, &
	Win32RegisterSeamlessProc		.5317, &
	Win32RegisterUserDatatype		.5318, &
	Win32RegisterUserMsg			.5319, &
	Win32ReleaseErasePS			.5320, &
	Win32ReleaseHook			.5321, &
	Win32ReleasePS				.5322, &
	Win32RemovePresParam			.5323, &
# pmshapi
	Win32RemoveSwitchEntry			.5324, &
# pwmin
	Win32ReplyMsg				.5325, &
	Win32RequestMutexSem			.5326, &
	Win32SaveWindowPos			.5327, &
	Win32ScrollWindow			.5328, &
	Win32SeamlessBox			.5329, &
	Win32SemEnter				.5330, &
	Win32SemLeave				.5331, &
	Win32SemQuery				.5332, &
	Win32SendDlgItemMsg			.5333, &
	Win32SendMsg				.5334, &
	Win32SetAccelTable			.5335, &
	Win32SetAccelTableValue			.5336, &
	Win32SetActiveWindow			.5337, &
	Win32SetCapture				.5338, &
	Win32SetCharXlateTbl			.5339, &
	Win32SetClassMsgInterest		.5340, &
	Win32SetClassThunkProc			.5341, &
	Win32SetClipbrdData			.5342, &
	Win32SetClipbrdOwner			.5343, &
	Win32SetClipbrdViewer			.5344, &
	Win32SetCp				.5345, &
	Win32SetDesktopBkgnd			.5346, &
	Win32SetDlgItemShort			.5347, &
	Win32SetDlgItemText			.5348, &
	Win32SetErrorInfo			.5349, &
	Win32SetFocus				.5350, &
	Win32SetFrameTreePPs			.5351, &
	Win32SetHook				.5352, &
	Win32SetKeyboardStateTable		.5353, &
	Win32SetMsgInterest			.5354, &
	Win32SetMsgMode				.5355, &
	Win32SetMultWindowPos			.5356, &
	Win32SetOwner				.5357, &
#
	WinSetPaletteMode			.5358, &
# pmwin
	Win32SetParent				.5359, &
	Win32SetPointer				.5360, &
	Win32SetPointerOwner			.5361, &
	Win32SetPointerPos			.5362, &
	Win32SetPresParam			.5363, &
	Win32SetRect				.5364, &
	Win32SetRectEmpty			.5365, &
	Win32SetSynchroMode			.5366, &
	Win32SetSysClipbrdViewer		.5367, &
	Win32SetSysColors			.5368, &
	Win32SetSysFont				.5369, &
	Win32SetSysModalWindow			.5370, &
	Win32SetSysPointerData			.5371, &
	Win32SetSysValue			.5372, &
	Win32SetVisibleRegionNotify		.5373, &
	Win32SetWindowBits			.5374, &
	Win32SetWindowPos			.5375, &
	Win32SetWindowPtr			.5376, &
	Win32SetWindowText			.5377, &
	Win32SetWindowThunkProc			.5378, &
	Win32SetWindowULong			.5379, &
	Win32SetWindowUShort			.5380, &
	Win32ShowCursor				.5381, &
	Win32ShowPointer			.5382, &
	Win32ShowTrackRect			.5383, &
	Win32ShowWindow				.5384, &
# pmshapi
	Win32StartApp				.5385, &
# pmwin
	Win32StartTimer				.5386, &
	Win32StopTimer				.5387, &
	Win32StretchPointer			.5388, &
	Win32SubclassWindow			.5389, &
	Win32SubstituteStrings			.5390, &
	Win32SubtractRect			.5391, &
# pmshapi
	Win32SwitchProgramRegister		.5392, &
	Win32SwitchToProgram			.5393, &
	Win32SwitchToProgram2			.5394, &
# pmwin
	Win32SystemShutdown			.5395, &
	Win32Terminate				.5396, &
# pmshapi
	Win32TerminateApp			.5397, &
# pmwin
	Win32ThreadAssocQueue			.5398, &
	Win32TrackRect				.5399, &
# pmwin
	Win32TrackSeamless			.5400, &
	Win32TranslateAccel			.5401, &
	Win32UnionRect				.5402, &
	Win32UpdateWindow			.5403, &
	Win32Upper				.5404, &
	Win32UpperChar				.5405, &
	Win32ValidateRect			.5406, &
	Win32ValidateRegion			.5407, &
	Win32VisRegionNotify			.5408, &
	Win32WaitEventSem			.5409, &
	Win32WaitMsg				.5410, &
	Win32WaitMuxWaitSem			.5411, &
	Win32WakeThread				.5412, &
	Win32WindowFromDC			.5413, &
	Win32WindowFromID			.5414, &
	Win32WindowFromPoint			.5415, &
#
	WinAugmentPointer			.5416, &
# pmwin
	WinMessageBox2				.5417, &
	WinMspAlloc				.5418, &
	WinMspFree				.5419, &
	WinMspInit				.5420, &
# os2sm, pmshapi, spl1b
	WinProcessHotKey			.5421, &
# pmwin
	WinProfControl				.5422, &
	WinXXAllocFill				.5423, &
	WinXXAllocMem				.5424, &
	WinXXCreateHeap				.5425, &
	WinXXDestroyHeap			.5426, &
	WinXXFreeMem				.5427, &
#
	WinXXIsMemInHeap			.5428, &
# pmwin
	WinXXReallocMem				.5429, &
#
	PMHeapIsMemInChain			.5430, &
# pmshapi
	Win32NoShutdown				.5431, &
# pmwin
	WinDrawTabbedText			.5432, &
	Win32QueryPointerClipRect		.5433, &
	Win32SetPointerClipRect			.5434, &
# pmwin
	Win32WaitMsgTimeout			.5436, &
	Win32IsWindowOfClass			.5437, &
	Win32SetAtomTableOwner			.5438, &
	WinXXQueryMemSize			.5439, &
	Win32SetClipRegion			.5440, &
	Win32QueryClipRegion			.5441, &
	Win32AddClipbrdViewer			.5442, &
	Win32RemoveClipbrdViewer		.5443, &
	Win32QueryClipbrdViewerChain		.5444, &
	Win32IncrementAtomUsage			.5445, &
	DaxQueryOpenClipbrdWindow		.5446, &
	WinQueryDaxData				.5447, &
	WinSetDaxData				.5448, &
	WinEnumProperties			.5449, &
	WinQueryProperty			.5450, &
	WinRemoveProperty			.5451, &
	WinSetProperty				.5452, &
	WinEnumClassProperties			.5453, &
	WinQueryClassProperty			.5454, &
	WinRemoveClassProperty			.5455, &
	WinSetClassProperty			.5456, &
#
	WinQueryHeapFlags			.5457, &
# pmwin
	Win32SetKbdLayout			.5458, &
#
	WinSetDAXDefWndProc			.5459, &
# pmgre
	PostEscape32				.5460, &
# pmwin
	Win32GetAFCValue			.5461, &
	Win32SetAFCValue			.5462, &
	Win32LoadVkeyGlyphXlateTbl		.5463, &
	Win32SetVkeyGlyphXlateTbl		.5464, &
	Win32QueryVkeyGlyphXlateTbl		.5465, &
	Win32TrackWindow			.5466, &
	Win32SemEnterTimeout			.5467, &
	WinSetDesktopWorkArea			.5468, &
	WinQueryDesktopWorkArea			.5469, &
#
	WinQueryControlColors			.5470, &
	WinSetControlColors			.5471, &
	CtlQueryColor				.5472, &
# pmwin
	Win32TranslateChar2			.5473, &
	Win32WaitForIdleThread			.5474, &
	WinXXCreateHeap2			.5475, &
	WinQuerySharedHeapData			.5476, &
	Win32QueryActiveOtherWindow		.5477, &
	Win32EnumVkeyScanIDs			.5478, &
#
	Win32QueryKbdLayout			.5479, &
	WinQueryCpType				.5480, &
	Win32QueryCharXlatetbl			.5481, &
# pmwin
	KbdPacket				.6000, &
#
	InitGameFrameProc			.6099, &
# pmgre
	GreNLS					.6100, &
#
	ValidateDCHandle			.6194, &
	HookMan					.6195, &
	CCHSetHailing				.6196, &
	EnableCoachMsgHook			.6197, &
	FCallGRECoach				.6198, &
	GRE32CallForward			.6199, &
# pmgre
	GreMemptrs				.6200, &
# pmwin
	FPen					.6201, &
# os2sm, pmshapi, spl1b
	ShellWindow				.6202, &
#
	PMSemaphores				.6203, &
	PHandleTable				.6204, &
	PSharedHeap				.6205, &
	VPAULDefaultColors			.6206, &
# pmwin
	DaxCreateCaret				.10000, &
	DaxDestroyCaret				.10001, &
	DaxGetCaretBlinkTime			.10002, &
# pmwin
	DaxHideCaret				.10004, &
	DaxSetCaretBlinkTime			.10005, &
# pmwin
	DaxShowCaret				.10007, &
# pmwin
	DaxShowCursor				.10018, &
	DaxSwapMouseButton			.10019, &
	LookupAtom				.10020, &
	DaxOpenClipbrd				.10021

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

TARGETS = subdirs $(PATH)$(PROJ).dll
TARGET = all
