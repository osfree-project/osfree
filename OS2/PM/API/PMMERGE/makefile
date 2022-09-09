#
# A Makefile for pmmerge.dll
# (c) osFree project,
# author, date
#

PROJ     = pmmerge
TRGT     = $(PROJ).dll
DESC     = OS/2 PM Merged Library
DIRS     = rectangles habmgr messagemgr framemgr windowmgr input load dialogs errors gre
srcfiles = $(p)init$(e) $(p)framemgr$(SEP)framemgr$(e) $(p)habmgr$(SEP)habmgr$(e) $(p)rectangles$(SEP)rectangles$(e) &
           $(p)input$(SEP)input$(e) $(p)messagemgr$(SEP)queue$(e) $(p)messagemgr$(SEP)messages$(e) &
           $(p)gre$(SEP)gre$(e) &
           $(p)errors$(SEP)errors$(e) $(p)errors$(SEP)f_errors$(e) $(p)windowmgr$(SEP)windowmgr$(e) &
           $(p)windowmgr$(SEP)WindowClass$(e) $(p)load$(SEP)load$(e) $(p)Fc_config$(e) $(p)util$(e) &
           $(p)F_debug$(e) $(p)unimpl$(e) # $(p)dialogs$(SEP)windlgbox$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)include  -sg -of+
# ADD_LINKOPT  = segment   type DATA nonshared lib os2386.lib lib clib3r.lib
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata, caseexact, start=dll_initterm
UNI2H = 1

IMPORTS = &
# utlapi
	  RectCopy utlapi.1,                    &
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
	Win16QueryProfileInt			.2001, &
	Win16QueryProfileString			.2002, &
	Win16WriteProfileString			.2003, &
	Win16QueryProfileSize			.2004, &
	Win16QueryProfileData			.2005, &
	Win16WriteProfileData			.2006, &
	Prf16ChangeWriteThru			.2007, &
	PMExecRegister				.2008, &
	ShePIInitIniFiles			.2009, &
	SheConvert190ProgramListTo300		.2010, &
	Prf16QueryProfileInt			.2011, &
	Prf16QueryProfileString			.2012, &
	Prf16WriteProfileString			.2013, &
	Prf16QueryProfileSize			.2014, &
	Prf16QueryProfileData			.2015, &
	Prf16WriteProfileData			.2016, &
	Prf16OpenProfile			.2017, &
	Prf16CloseProfile			.2018, &
	Prf16Reset				.2019, &
	Prf16QueryProfile			.2020, &
	Prf16AddProgram				.2021, &
	Prf16RemoveProgram			.2022, &
	Prf16ChangeProgram			.2023, &
	Prf16QueryDefinition			.2024, &
	Prf16QueryProgramTitles			.2025, &
	Prf16CreateGroup			.2026, &
	Prf16QueryProgramHandle			.2027, &
	Prf16QueryProgramCategory		.2028, &
	Prf16DestroyGroup			.2029, &
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
	Win16QueryClassInfo			.2065, &
	Win16MultWindowFromIDs			.2066, &
	Win16LockWindow				.2067, &
	Win16QueryWindowLockCount		.2068, &
	Win16LoadPointer			.2069, &
	Win16CreatePointer			.2070, &
	Win16DestroyPointer			.2071, &
	OldWin16QueryPointerInfo		.2072, &
	Win16QuerySysPointer			.2073, &
	Win16GetSysBitmap			.2074, &
	Win16QueryCursorInfo			.2075, &
	Win16LoadAccelTable			.2076, &
	Win16CreateAccelTable			.2077, &
	Win16DestroyAccelTable			.2078, &
	Win16CopyAccelTable			.2079, &
	Win16SetClipbrdData			.2080, &
	Win16QueryClipbrdFmtInfo		.2081, &
	Gpi16LoadBitmap				.2082, &
	Win16LoadString				.2083, &
	Win16LoadMessage			.2084, &
	Win16SetRectEmpty			.2085, &
	Win16SetRect				.2086, &
	Win16CopyRect				.2087, &
	Win16IsRectEmpty			.2088, &
	Win16PtInRect				.2089, &
	Win16OffsetRect				.2090, &
	Win16InflateRect			.2091, &
	Win16MakeRect				.2092, &
	Win16MakePoints				.2093, &
	Win16DefWindowProc			.2094, &
	Win16DefDlgProc				.2095, &
	Win16QueryHungProcess			.2096, &
	Win16LoadStringTable			.2097, &
	Win16LoadVkeyGlyphXlatetbl		.2098, &
	Win16LoadCharXlatetbl			.2099, &
	Win16SetVkeyGlyphXlatetbl		.2100, &
	Win16QueryVkeyGlyphXlatetbl		.2101, &
	Win16VkeyToScanID			.2102, &
	Win16QueryProcessCP			.2103, &
	Win16GetKeyState			.2104, &
	Win16SetCp				.2105, &
	Win16QueryCp				.2106, &
	Win16QueryCpList			.2107, &
	Win16CpTranslateString			.2108, &
	Win16CpTranslateChar			.2109, &
	Win16CompareStrings			.2110, &
	Win16Upper				.2111, &
	Win16UpperChar				.2112, &
	Win16NextChar				.2113, &
	Win16PrevChar				.2114, &
	Win16CreateHeap				.2115, &
	Win16DestroyHeap			.2116, &
	Win16AvailMem				.2117, &
	Win16AllocMem				.2118, &
	Win16ReallocMem				.2119, &
	Win16FreeMem				.2120, &
	Win16LockHeap				.2121, &
	Win16CreateAtomTable			.2122, &
	Win16DestroyAtomTable			.2123, &
	Win16AddAtom				.2124, &
	Win16FindAtom				.2125, &
	Win16DeleteAtom				.2126, &
	Win16QueryAtomUsage			.2127, &
	Win16QueryAtomLength			.2128, &
	Win16QueryAtomName			.2129, &
	Win16QuerySystemAtomTable		.2130, &
	Win16SetLastError			.2131, &
	Win16GetLastError			.2132, &
	Win16GetErrorInfo			.2133, &
	Win16FreeErrorInfo			.2134, &
	Win16Initialize				.2135, &
	Win16Terminate				.2136, &
	Win16Catch				.2137, &
	Win16Throw				.2138, &
	Win16QueryVersion			.2139, &
	FsrSemExit				.2140, &
	FsrSemCheck				.2141, &
	AabGetReg				.2142, &
	AabSetReg				.2143, &
	ValidateSelector			.2144, &
	LoadModuleHandle			.2145, &
	GetScreenSelector			.2146, &
	IdentifyCodeSelector			.2147, &
	Win16SetErrorInfo			.2148, &
	Win16TerminateSystem			.2149, &
	Win16DefQueueProc			.2150, &
	Win16VisRegionNotify			.2151, &
	Win16QueryKbdLayout			.2152, &
	Win16SetKbdLayout			.2153, &
	Win16BuildPtrHandle			.2154, &
	DumWinCreateHelpInstance		.2155, &
	DimWinDestroyHelpInstance		.2156, &
	DumWinAssociateHelpInstance		.2157, &
	DimWinCreateHelpTable			.2158, &
	DumWinLoadHelpTable			.2159, &
	DumWinQueryHelpInstance			.2160, &
	DummyHelpEntry				.2161, &
	GetPMDDCodeSelector			.2162, &
	Win16QueryPwnd				.2163, &
	Win16EnumVKeyScanIDs			.2164, &
	Win16LoadTranslationTable		.2165, &
	Win16SetKbdLayout2			.2166, &
	DebugInputEnable			.2167, &
	DebugInput				.2168, &
	DebugOutput				.2169, &
	DebugCurPos				.2170, &
	DebugCls				.2171, &
	DebugRedirect				.2172, &
	Win16LangsupWindowProc			.2173, &
	Win16LangsupDlgProc			.2174, &
	Win16CreatePointerIndirect		.2175, &
	Win16QueryPointerInfo			.2176, &
	Win16PortHoleUtil			.2177, &
	Win16CreateFarHeap			.2178, &
	Win16DestroyFarHeap			.2179, &
	Win16AllocFarMem			.2180, &
	Win16FreeFarMem				.2181, &
	Win16ReallocFarMem			.2182, &
	Win16SetQueueProc			.2183, &
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
	Win16Alarm				.3014, &
	Win16BeginEnumWindows			.3015, &
	Win16BeginPaint				.3016, &
	Win16BroadcastMsg			.3017, &
	Win16CancelMode				.3018, &
	Win16CancelShutdown			.3019, &
	Win16CreateMsgQueue			.3020, &
	Win16CreateStdWindow			.3021, &
	Win16CreateWindow			.3022, &
	Win16DestroyMsgQueue			.3023, &
	Win16DestroyWindow			.3024, &
	Win16DispatchMsg			.3025, &
	Win16DrawText				.3026, &
	Win16EndPaint				.3027, &
	Win16FillRect				.3028, &
	Win16GetMinWindowViewer			.3029, &
	Win16GetMsg				.3030, &
	Win16GetNextWindow			.3031, &
	Win16InitializeSystem			.3032, &
	Win16InvalidateRect			.3033, &
	Win16IsChild				.3034, &
	Win16IsWindow				.3035, &
	Win16IsWindowVisible			.3036, &
	Win16LoadDlg				.3037, &
	Win16LockStartInput			.3038, &
	Win16MessageBox				.3039, &
	Win16MsgSemWait				.3040, &
	Win16OpenWindowDC			.3041, &
	Win16PeekMsg				.3042, &
	Win16PostMsg				.3043, &
	Win16ProcessDlg				.3044, &
	Win16QueryActiveWindow			.3045, &
	Win16QueryAnchorBlock			.3046, &
	Win16QueryDesktopWindow			.3047, &
	Win16QueryFocus				.3048, &
	Win16QueryQueueInfo			.3049, &
	Win16QuerySysModalWindow		.3050, &
	Win16QuerySysValue			.3051, &
	Win16QueryWindow			.3052, &
	Win16QueryWindowPos			.3053, &
	Win16QueryWindowProcess			.3054, &
	Win16QueryWindowRect			.3055, &
	Win16QueryWindowULong			.3056, &
	Win16QueryWindowUShort			.3057, &
	Win16RegisterClass			.3058, &
	Win16ResetSystem			.3059, &
	Win16ScreenSwitch			.3060, &
	Win16SendDlgItemMsg			.3061, &
	Win16SendMsg				.3062, &
	Win16SetActiveWindow			.3063, &
	Win16SetDlgItemText			.3064, &
	Win16SetFocus				.3065, &
	Win16SetMultWindowPos			.3066, &
	Win16SetParent				.3067, &
	Win16SetPresParam			.3068, &
	Win16SetSysModalWindow			.3069, &
	Win16SetSysValue			.3070, &
	Win16SetWindowBits			.3071, &
	Win16SetWindowPos			.3072, &
	Win16SetWindowText			.3073, &
	Win16ShowWindow				.3074, &
	Win16SubclassWindow			.3075, &
	Win16TerminateShutdown			.3076, &
	Win16TrackRect				.3077, &
	Win16WindowFromID			.3078, &
# pmshapi
	Win16AddProgram				.3079, &
# os2sm, pmshapi, spl1b
	Win16AddSwitchEntry			.3080, &
# pmshapi
	Win16AddToGroup				.3081, &
# pmwin
	Win16CalcFrameRect			.3082, &
	Win16CallHelpHook			.3083, &
	Win16CallMsgFilter			.3084, &
# os2sm, pmshapi, spl1b
	Win16CanTerminate			.3085, &
# pmshapi
	Win16ChangeProgram			.3086, &
# os2sm, pmshapi, spl1b
	Win16ChangeSwitchEntry			.3087, &
# pmwin
	Win16CheckInput				.3088, &
	Win16CloseClipbrd			.3089, &
# os2sm, pmshapi, spl1b
	Win16CplRegister			.3090, &
# pmwin
	Win16CreateCursor			.3091, &
	Win16CreateDlg				.3092, &
	Win16CreateFrameControls		.3093, &
# pmshapi
	Win16CreateGroup			.3094, &
# pmwin
	Win16CreateMenu				.3095, &
# os2sm, pmshapi, spl1b
	Win16CreateSwitchEntry			.3096, &
# pmwin
	Win16DdeInitiate			.3097, &
	Win16DdePostMsg				.3098, &
	Win16DdeRespond				.3099, &
	Win16DebugBreak				.3100, &
	Win16DeleteLibrary			.3101, &
	Win16DeleteProcedure			.3102, &
	Win16DestroyAniPointer			.3103, &
	Win16DestroyCursor			.3104, &
# pmshapi
	Win16DestroyGroup			.3105, &
# pmwin
	Win16DismissDlg				.3106, &
	Win16DlgBox				.3107, &
	Win16DrawBitmap				.3108, &
	Win16DrawBorder				.3109, &
	Win16DrawPointer			.3110, &
	Win16EmptyClipbrd			.3111, &
	Win16EnablePhysInput			.3112, &
	Win16EnableWindow			.3113, &
	Win16EnableWindowUpdate			.3114, &
	Win16EndEnumWindows			.3115, &
# os2sm, pmshapi, spl1b
	Win16EndProgram				.3116, &
	Win16EndSession				.3117, &
	Win16EndWindowSession			.3118, &
# pmwin
	Win16EnumClipbrdFmts			.3119, &
	Win16EnumDlgItem			.3120, &
	Win16EqualRect				.3121, &
	Win16ExcludeUpdateRegion		.3122, &
	Win16ExplodeWindow			.3123, &
	Win16FlashWindow			.3124, &
	Win16FocusChange			.3125, &
	Win16ForceMenuRedraw			.3126, &
	Win16FormatFrame			.3127, &
	Win16GetClipPS				.3128, &
	Win16GetCurrentTime			.3129, &
	Win16GetDlgMsg				.3130, &
	Win16GetErasePS				.3131, &
	Win16GetFrameTreePPs			.3132, &
	Win16GetFrameTreePPSize			.3133, &
	Win16GetMaxPosition			.3134, &
	Win16GetMinPosition			.3135, &
	Win16GetPhysKeyState			.3136, &
	Win16GetPS				.3137, &
	Win16GetScanState			.3138, &
	Win16GetScreenPS			.3139, &
# pmshapi
	Win16HAPPfromPID			.3140, &
	Win16HSWITCHfromHAPP			.3141, &
	Win16InitializePL			.3142, &
# os2sm, pmshapi, spl1b
	Win16InitSession			.3143, &
	Win16InitSessionMgr			.3144, &
	Win16InitSwEntry			.3145, &
# pmwin
	Win16InSendMsg				.3146, &
# os2sm, pmshapi, spl1b
	Win16InstStartApp			.3147, &
# pmwin
	Win16IntersectRect			.3148, &
	Win16InvalidateRegion			.3149, &
	Win16InvertRect				.3150, &
	Win16IsPhysInputEnabled			.3151, &
	Win16IsShuttingDown			.3152, &
	Win16IsThreadActive			.3153, &
	Win16IsWindowEnabled			.3154, &
	Win16IsWindowShowing			.3155, &
	Win16JiggleMouse			.3156, &
	Win16LoadAniPointer			.3157, &
	Win16LoadLibrary			.3158, &
	Win16LoadMenu				.3159, &
	Win16LoadProcedure			.3160, &
	Win16LockInput				.3161, &
	Win16LockPointerUpdate			.3162, &
	Win16LockVisRegions			.3163, &
	Win16LockWindowUpdate			.3164, &
	Win16MapDlgPoints			.3165, &
	Win16MapWindowPoints			.3166, &
	Win16MenuIndexFromID			.3167, &
	Win16MsgMuxSemWait			.3168, &
# os2sm, pmshapi, spl1b
	Win16NoShutdown				.3169, &
# pmwin
	Win16OpenClipbrd			.3170, &
# os2sm, pmshapi, spl1b
	Win16PMFileRegister			.3171, &
# pmwin
	Win16PopupMenu				.3172, &
	Win16PostQueueMsg			.3173, &
	Win16QueryAccelTable			.3174, &
	Win16QueryAccelTableValue		.3175, &
	Win16QueryCapture			.3176, &
	Win16QueryCharXlatetbl			.3177, &
	Win16QueryClassName			.3178, &
	Win16QueryClassThunkProc		.3179, &
	Win16QueryClipbrdData			.3180, &
	Win16QueryClipbrdOwner			.3181, &
	Win16QueryClipbrdViewer			.3182, &
# pmshapi
	Win16QueryDefinition			.3183, &
# pmwin
	Win16QueryDesktopBkgnd			.3184, &
	Win16QueryDlgItemShort			.3185, &
	Win16QueryDlgItemText			.3186, &
	Win16QueryDlgItemTextLength		.3187, &
# os2sm, pmshapi, spl1b
	Win16QueryExtIDFocus			.3188, &
# pmshapi
	Win16QueryFileExtOpts			.3189, &
# pmwin
	Win16QueryMenuData			.3190, &
	Win16QueryMsgPos			.3191, &
	Win16QueryMsgTime			.3192, &
	Win16QueryObjectWindow			.3193, &
	Win16QueryPointer			.3194, &
	Win16QueryPointerPos			.3195, &
	Win16QueryPresParam			.3196, &
# pmshapi
	Win16QueryProgramTitles			.3197, &
	Win16QueryProgramType			.3198, &
	Win16QueryProgramUse			.3199, &
# pmwin
	Win16QueryQueueStatus			.3200, &
	Win16QuerySendMsg			.3201, &
# os2sm, pmshapi, spl1b
	Win16QuerySessionTitle			.3202, &
# pmwin
	Win16QueryState				.3203, &
# os2sm, pmshapi, spl1b
	Win16QuerySwitchEntry			.3204, &
	Win16QuerySwitchHandle			.3205, &
	Win16QuerySwitchList			.3206, &
# pmwin
	Win16QuerySysColor			.3207, &
# os2sm, pmshapi, spl1b
	Win16QueryTaskSizePos			.3208, &
	Win16QueryTaskTitle			.3209, &
# pmwin
	Win16QueryUpdateRect			.3210, &
	Win16QueryUpdateRegion			.3211, &
	Win16QueryWindowDC			.3212, &
	Win16QueryWindowModel			.3213, &
	Win16QueryWindowPtr			.3214, &
	Win16QueryWindowSGID			.3215, &
	Win16QueryWindowText			.3216, &
	Win16QueryWindowTextLength		.3217, &
	Win16QueryWindowThunkProc		.3218, &
	Win16QueryWorkplaceWindow		.3219, &
	Win16QueueFromID			.3220, &
	Win16RealizePalette			.3221, &
	Win16RegisterSeamlessProc		.3222, &
	Win16RegisterUserDatatype		.3223, &
	Win16RegisterUserMsg			.3224, &
	Win16RegisterWindowDestroy		.3225, &
	Win16ReleaseErasePS			.3226, &
	Win16ReleaseHook			.3227, &
	Win16ReleasePS				.3228, &
# pmshapi
	Win16RemoveFromGroup			.3229, &
# pmwin
	Win16RemovePresParam			.3230, &
# pmshapi
	Win16RemoveProgram			.3231, &
# os2sm, pmshapi, spl1b
	Win16RemoveSwitchEntry			.3232, &
# pmwin
	Win16ReplyMsg				.3233, &
	Win16SaveWindowPos			.3234, &
	Win16ScrollWindow			.3235, &
	Win16SeamlessBox			.3236, &
	Win16SemEnter				.3237, &
	Win16SemLeave				.3238, &
	Win16SemQuery				.3239, &
	Win16SendQueueMsg			.3240, &
	Win16SetAccelTable			.3241, &
	Win16SetAccelTableValue			.3242, &
	Win16SetAniPointer			.3243, &
	Win16SetAtomTableOwner			.3244, &
	Win16SetCapture				.3245, &
	Win16SetCharXlatetbl			.3246, &
	Win16SetClassMsgInterest		.3247, &
	Win16SetClassThunkProc			.3248, &
	Win16SetClipbrdOwner			.3249, &
	Win16SetClipbrdViewer			.3250, &
	Win16SetDesktopBkgnd			.3251, &
	Win16SetDlgItemShort			.3252, &
# os2sm, pmshapi, spl1b
	Win16SetExtIDFocus			.3253, &
	Win16SetFgndWindow			.3254, &
# pmshapi
	Win16SetFileExtOpts			.3255, &
# pmwin
	Win16SetFrameTreePPs			.3256, &
	Win16SetHook				.3257, &
	Win16SetKeyboardStateTable		.3258, &
	Win16SetMenuData			.3259, &
	Win16SetMsgInterest			.3260, &
	Win16SetMsgMode				.3261, &
	Win16SetOwner				.3262, &
	Win16SetPointer				.3263, &
	Win16SetPointerOwner			.3264, &
	Win16SetPointerPos			.3265, &
	Win16SetScanStateTable			.3266, &
# os2sm, pmshapi, spl1b
	Win16SetSwEntry				.3267, &
# pmwin
	Win16SetSynchroMode			.3268, &
	Win16SetSysClipbrdViewer		.3269, &
	Win16SetSysColors			.3270, &
# os2sm, pmshapi, spl1b
	Win16SetTitle				.3271, &
# pmshapi
	Win16SetTitleAndIcon			.3272, &
# pmwin
	Win16SetWindowDC			.3273, &
	Win16SetWindowPtr			.3274, &
	Win16SetWindowThunkProc			.3275, &
	Win16SetWindowULong			.3276, &
	Win16SetWindowUShort			.3277, &
	Win16ShowCursor				.3278, &
	Win16ShowPointer			.3279, &
	Win16ShowTrackRect			.3280, &
# pmshapi
	Win16StartApp				.3281, &
# pmwin
	Win16StartTimer				.3282, &
# os2sm, pmshapi, spl1b
	Win16StopProgram			.3283, &
# pmwin
	Win16StopTimer				.3284, &
# os2sm, pmshapi, spl1b
 	Win16StpRegister			.3285, &
# pmwin
	Win16StretchPointer			.3286, &
	Win16SubstituteStrings			.3287, &
	Win16SubtractRect			.3288, &
# os2sm, pmshapi, spl1b
	Win16SwitchProgramRegister		.3289, &
	Win16SwitchToProgram2			.3290, &
	Win16SwitchToProgram			.3291, &
	Win16SwitchToTaskManager		.3292, &
# pmwin
	Win16SyncStartPgm			.3293, &
	Win16SystemShutdown			.3294, &
# os2sm, pmshapi, spl1b
	Win16TerminateApp			.3295, &
# pmwin
	Win16ThreadAssocQueue			.3296, &
	Win16TrackSeamless			.3297, &
	Win16TranslateAccel			.3298, &
	Win16TranslateChar			.3299, &
	Win16UnionRect				.3300, &
	Win16UpdateWindow			.3301, &
	Win16ValidateRect			.3302, &
	Win16ValidateRegion			.3303, &
	Win16VkeyGlyphCharXlate			.3304, &
	Win16WaitMsg				.3305, &
	Win16WakeThread				.3306, &
	Win16WindowFromDC			.3307, &
	Win16WindowFromPoint			.3308, &
# pmshapi
	Win16SetTitleAndHwndIcon		.3309, &
#
	Win16TrackWindow			.3310, &
# pmwin
	Win16SetLangInfo			.3500, &
	Win16QueryLangInfo			.3501, &
	Win16SetKbdLayer			.3502, &
	Win16QueryKbdLayer			.3503, &
	Win16SetLangViewer			.3504, &
	Win16QueryLangViewer			.3505, &
# pmwin
	Win16PourEvent				.3600, &
	Win16SetMsgQimInfo			.3601, &
	Win16QueryMsgQimInfo			.3602, &
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
	Win32MessageBox2			.5417, &
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

!include $(%ROOT)/mk/appsos2_fpm.mk

$(PATH)rectangles$(SEP)rectangles.obj: $(MYDIR)rectangles$(SEP)rectangles.c
$(PATH)habmgr$(SEP)habmgr.obj: $(MYDIR)habmgr$(SEP)habmgr.cpp
$(PATH)messagemgr$(SEP)messages.obj: $(MYDIR)messagemgr$(SEP)messages.cpp
$(PATH)messagemgr$(SEP)queue.obj: $(MYDIR)messagemgr$(SEP)queue.cpp
$(PATH)framemgr$(SEP)framemgr.obj: $(MYDIR)framemgr$(SEP)framemgr.cpp
$(PATH)windowmgr$(SEP)WindowClass.obj: $(MYDIR)windowmgr$(SEP)WindowClass.cpp
$(PATH)windowmgr$(SEP)windowmgr.obj: $(MYDIR)windowmgr$(SEP)windowmgr.cpp
$(PATH)input$(SEP)input.obj: $(MYDIR)input$(SEP)input.cpp
$(PATH)load$(SEP)load.obj: $(MYDIR)load$(SEP)load.c
$(PATH)dialogs$(SEP)windlgbox.obj: $(MYDIR)dialogs$(SEP)windlgbox.c
$(PATH)errors$(SEP)errors.obj: $(MYDIR)errors$(SEP)errors.cpp
$(PATH)errors$(SEP)f_errors.obj: $(MYDIR)errors$(SEP)f_errors.cpp
$(PATH)gre$(SEP)gre.obj: $(MYDIR)gre$(SEP)gre.cpp

.$(O): $(PATH)rectangles
.$(O): $(PATH)habmgr
.$(O): $(PATH)messagemgr
.$(O): $(PATH)framemgr
.$(O): $(PATH)windowmgr
.$(O): $(PATH)input
.$(O): $(PATH)load
.$(O): $(PATH)dialogs
.$(O): $(PATH)errors
.$(O): $(PATH)gre

.c: $(MYDIR)rectangles
.c: $(MYDIR)habmgr
.c: $(MYDIR)messagemgr
.c: $(MYDIR)framemgr
.c: $(MYDIR)windowmgr
.c: $(MYDIR)input
.c: $(MYDIR)load
.c: $(MYDIR)dialogs
.c: $(MYDIR)errors
.c: $(MYDIR)gre

.cpp: $(MYDIR)rectangles
.cpp: $(MYDIR)habmgr
.cpp: $(MYDIR)messagemgr
.cpp: $(MYDIR)framemgr
.cpp: $(MYDIR)windowmgr
.cpp: $(MYDIR)input
.cpp: $(MYDIR)load
.cpp: $(MYDIR)dialogs
.cpp: $(MYDIR)errors
.cpp: $(MYDIR)gre

#TARGETS = subdirs $(PATH)$(PROJ).dll
#TARGET = all
