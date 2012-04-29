#
# A Makefile for pmdrag.dll
# (c) osFree project,
# author, date
#

PROJ     = pmwin
DESC     = Drag and Drop API
srcfiles = $(p)..$(SEP)shared$(SEP)stub$(e)
ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata, caseexact
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib
IMPORTS = WinInitialize      PMMERGE.5201,  &
          WinTerminate       PMMERGE.5396,  &
          WinCreateMsgQueue  PMMERGE.5136,  &
          WinDestroyMsgQueue PMMERGE.5154,  &
          WinDispatchMsg     PMMERGE.5158,  &
          WinGetMsg          PMMERGE.5192,  &
          WinSendMsg         PMMERGE.5334,  &
          WinRegisterClass   PMMERGE.5315,  &
          WinCreateStdWindow PMMERGE.5139,  &
          WinDefWindowProc   PMMERGE.5147,  &
          WinDestroyWindow   PMMERGE.5156,  &
          WINQUERYDESKTOPWINDOW     PMMERGE.3047,  &
          WINQUERYOBJECTWINDOW      PMMERGE.3193,  &
          WINREGISTERCLASS          PMMERGE.3058,  &
          WINQUERYCLASSNAME         PMMERGE.3178,  &
          WINQUERYCLASSINFO         PMMERGE.2065,  &
          WINCREATEWINDOW           PMMERGE.3022,  &
          WINDESTROYWINDOW          PMMERGE.3024,  &
          WINSETWINDOWPOS           PMMERGE.3072,  &
          WINSETMULTWINDOWPOS       PMMERGE.3066,  &
          WINQUERYWINDOWPOS         PMMERGE.3053,  &
          WINENABLEWINDOW           PMMERGE.3113,  &
          WINISWINDOWENABLED        PMMERGE.3154,  &
          WINSHOWWINDOW             PMMERGE.3074,  &
          WINENABLEWINDOWUPDATE     PMMERGE.3114,  &
          WINISWINDOWVISIBLE        PMMERGE.3036,  &
          WINSETWINDOWTEXT          PMMERGE.3073,  &
          WINQUERYWINDOWTEXT        PMMERGE.3216,  &
          WINQUERYWINDOWTEXTLENGTH  PMMERGE.3217,  &
          WINWINDOWFROMID           PMMERGE.3078,  &
          WINMULTWINDOWFROMIDS      PMMERGE.2066,  &
          WINISWINDOW               PMMERGE.3035,  &
          WINISCHILD                PMMERGE.3034,  &
          WINSETPARENT              PMMERGE.3067,  &
          WINSETOWNER               PMMERGE.3262,  &
          WINQUERYWINDOW            PMMERGE.3052,  &
          WINQUERYWINDOWRECT        PMMERGE.3055,  &
          WINQUERYWINDOWPROCESS     PMMERGE.3054,  &
          WINSETWINDOWUSHORT        PMMERGE.3277,  &
          WINQUERYWINDOWUSHORT      PMMERGE.3057,  &
          WINSETWINDOWULONG         PMMERGE.3276,  &
          WINQUERYWINDOWULONG       PMMERGE.3056,  &
          WINBEGINENUMWINDOWS       PMMERGE.3015,  &
          WINGETNEXTWINDOW          PMMERGE.3031,  &
          WINENDENUMWINDOWS         PMMERGE.3115,  &
          WINWINDOWFROMPOINT        PMMERGE.3308,  &
          WINMAPWINDOWPOINTS        PMMERGE.3166,  &
          WINSUBCLASSWINDOW         PMMERGE.3075,  &
          WINLOCKWINDOW             PMMERGE.2067,  &
          WINQUERYWINDOWLOCKCOUNT   PMMERGE.2068,  &
          WINREGISTERWINDOWDESTROY  PMMERGE.3225,  &
          WINOPENWINDOWDC           PMMERGE.3041,  &
          WINGETSCREENPS            PMMERGE.3139,  &
          WINGETPS                  PMMERGE.3137,  &
          WINRELEASEPS              PMMERGE.3228,  &
          WINBEGINPAINT             PMMERGE.3016,  &
          WINENDPAINT               PMMERGE.3027,  &
          WININVALIDATERECT         PMMERGE.3033,  &
          WININVALIDATEREGION       PMMERGE.3149,  &
          WINVALIDATERECT           PMMERGE.3302,  &
          WINVALIDATEREGION         PMMERGE.3303,  &
          WINQUERYUPDATERECT        PMMERGE.3210,  &
          WINQUERYUPDATEREGION      PMMERGE.3211,  &
          WINUPDATEWINDOW           PMMERGE.3301,  &
          WINEXCLUDEUPDATEREGION    PMMERGE.3122,  &
          WINLOCKWINDOWUPDATE       PMMERGE.3164,  &
          WINLOCKVISREGIONS         PMMERGE.3163,  &
          WINWINDOWFROMDC           PMMERGE.3307,  &
          WINCREATEMSGQUEUE         PMMERGE.3020,  &
          WINDESTROYMSGQUEUE        PMMERGE.3023,  &
          WINQUERYQUEUESTATUS       PMMERGE.3200,  &
          WINSENDMSG                PMMERGE.3062,  &
          WINBROADCASTMSG          PMMERGE.3017,  &
          WININSENDMSG             PMMERGE.3146,  &
          WINGETMSG                PMMERGE.3030,  &
          WINPEEKMSG               PMMERGE.3042,  &
          WINWAITMSG               PMMERGE.3305,  &
          WINDISPATCHMSG           PMMERGE.3025,  &
          WINPOSTMSG               PMMERGE.3043,  &
          WINPOSTQUEUEMSG          PMMERGE.3173,  &
          WINQUERYMSGPOS           PMMERGE.3191,  &
          WINQUERYMSGTIME          PMMERGE.3192,  &
          WINCALLMSGFILTER         PMMERGE.3084,  &
          WINSETMSGINTEREST        PMMERGE.3260,  &
          WINSETCAPTURE            PMMERGE.3245,  &
          WINQUERYCAPTURE          PMMERGE.3176,  &
          WINSETFOCUS              PMMERGE.3065,  &
          WINQUERYFOCUS            PMMERGE.3048,  &
          WINSETACTIVEWINDOW       PMMERGE.3063,  &
          WINQUERYACTIVEWINDOW     PMMERGE.3045,  &
          WINSETSYSMODALWINDOW     PMMERGE.3069,  &
          WINQUERYSYSMODALWINDOW   PMMERGE.3050,  &
          WINISTHREADACTIVE        PMMERGE.3153,  &
          WINSTARTTIMER            PMMERGE.3282,  &
          WINSTOPTIMER             PMMERGE.3284,  &
          WINGETCURRENTTIME        PMMERGE.3129,  &
          WINLOADPOINTER           PMMERGE.2069,  &
          WINCREATEPOINTER         PMMERGE.2070,  &
          WINDESTROYPOINTER        PMMERGE.2071,  &
          OLDWINQUERYPOINTERINFO   PMMERGE.2072,  &
          WINQUERYSYSPOINTER       PMMERGE.2073,  &
          WINSETPOINTER            PMMERGE.3263,  &
          WINQUERYPOINTER          PMMERGE.3194,  &
          WINSHOWPOINTER           PMMERGE.3279,  &
          WINSETPOINTERPOS         PMMERGE.3265,  &
          WINQUERYPOINTERPOS       PMMERGE.3195,  &
          WINGETSYSBITMAP          PMMERGE.2074,  &
          WINCREATECURSOR          PMMERGE.3091,  &
          WINDESTROYCURSOR         PMMERGE.3104,  &
          WINSHOWCURSOR            PMMERGE.3278,  &
          WINQUERYCURSORINFO       PMMERGE.2075,  &
          WINLOADACCELTABLE        PMMERGE.2076,  &
          WINCREATEACCELTABLE      PMMERGE.2077,  &
          WINDESTROYACCELTABLE     PMMERGE.2078,  &
          WINTRANSLATEACCEL        PMMERGE.3298,  &
          WINSETACCELTABLE         PMMERGE.3241,  &
          WINQUERYACCELTABLE       PMMERGE.3174,  &
          WINCOPYACCELTABLE        PMMERGE.2079,  &
          WINSETHOOK               PMMERGE.3257,  &
          WINRELEASEHOOK           PMMERGE.3227,  &
          WINOPENCLIPBRD           PMMERGE.3170,  &
          WINCLOSECLIPBRD          PMMERGE.3089,  &
          WINEMPTYCLIPBRD          PMMERGE.3111,  &
          WINSETCLIPBRDOWNER       PMMERGE.3249,  &
          WINQUERYCLIPBRDOWNER     PMMERGE.3181,  &
          WINSETCLIPBRDDATA        PMMERGE.2080,  &
          WINQUERYCLIPBRDDATA      PMMERGE.3180,  &
          WINENUMCLIPBRDFMTS       PMMERGE.3119,  &
          WINQUERYCLIPBRDFMTINFO   PMMERGE.2081,  &
          WINSETCLIPBRDVIEWER      PMMERGE.3250,  &
          WINQUERYCLIPBRDVIEWER    PMMERGE.3182,  &
          WINLOADDLG               PMMERGE.3037,  &
          WINCREATEDLG             PMMERGE.3092,  &
          WINPROCESSDLG            PMMERGE.3044,  &
          WINDLGBOX                PMMERGE.3107,  &
          WINDISMISSDLG            PMMERGE.3106,  &
          WINSENDDLGITEMMSG        PMMERGE.3061,  &
          WINSETDLGITEMSHORT       PMMERGE.3252,  &
          WINQUERYDLGITEMSHORT     PMMERGE.3185,  &
          WINMAPDLGPOINTS          PMMERGE.3165,  &
          WINSUBSTITUTESTRINGS     PMMERGE.3287,  &
          WINENUMDLGITEM           PMMERGE.3120,  &
          WINSETDLGITEMTEXT        PMMERGE.3064,  &
          WINQUERYDLGITEMTEXT      PMMERGE.3186,  &
          WINLOADMENU              PMMERGE.3159,  &
          WINCREATEMENU            PMMERGE.3095,  &
          WINALARM                 PMMERGE.3014,  &
          WINMESSAGEBOX            PMMERGE.3039,  &
          WINCREATESTDWINDOW       PMMERGE.3021,  &
          WINCREATEFRAMECONTROLS   PMMERGE.3093,  &
          WINFORMATFRAME           PMMERGE.3127,  &
          WINCALCFRAMERECT         PMMERGE.3082,  &
          WINFLASHWINDOW           PMMERGE.3124,  &
          WINGETMINPOSITION        PMMERGE.3135,  &
          WINGETMAXPOSITION        PMMERGE.3134,  &
          WINQUERYSYSVALUE         PMMERGE.3051,  &
          WINSETSYSVALUE           PMMERGE.3070,  &
          WINSETSYSCOLORS          PMMERGE.3270,  &
          WINQUERYSYSCOLOR         PMMERGE.3207,  &
          WINSCROLLWINDOW          PMMERGE.3235,  &
          WINTRACKRECT             PMMERGE.3077,  &
          WINSHOWTRACKRECT         PMMERGE.3280,  &
          GPILOADBITMAP            PMMERGE.2082,  &
          WINLOADSTRING            PMMERGE.2083,  &
          WINLOADMESSAGE           PMMERGE.2084,  &
          WINSETRECTEMPTY          PMMERGE.2085,  &
          WINSETRECT               PMMERGE.2086,  &
          WINCOPYRECT              PMMERGE.2087,  &
          WINISRECTEMPTY           PMMERGE.2088,  &
          WINEQUALRECT             PMMERGE.3121,  &
          WINPTINRECT              PMMERGE.2089,  &
          WINOFFSETRECT            PMMERGE.2090,  &
          WININFLATERECT           PMMERGE.2091,  &
          WININTERSECTRECT         PMMERGE.3148,  &
          WINUNIONRECT             PMMERGE.3300,  &
          WINSUBTRACTRECT          PMMERGE.3288,  &
          WINMAKERECT              PMMERGE.2092,  &
          WINMAKEPOINTS            PMMERGE.2093,  &
          WININVERTRECT            PMMERGE.3150,  &
          WINFILLRECT              PMMERGE.3028,  &
          KBDPACKET                PMMERGE.6000,  &
          WINDRAWPOINTER           PMMERGE.3110,  &
          WINDEFWINDOWPROC         PMMERGE.2094,  &
          WINDEFDLGPROC            PMMERGE.2095,  &
          WINSYNCSTARTPGM          PMMERGE.3293,  &
          WINSCREENSWITCH          PMMERGE.3060,  &
          WINRESETSYSTEM           PMMERGE.3059,  &
          WINQUERYHUNGPROCESS      PMMERGE.2096,  &
          WINLOADSTRINGTABLE       PMMERGE.2097,  &
          WINLOADVKEYGLYPHXLATETBL PMMERGE.2098,  &
          WINLOADCHARXLATETBL      PMMERGE.2099,  &
          WINSETVKEYGLYPHXLATETBL  PMMERGE.2100,  &
          WINSETCHARXLATETBL       PMMERGE.3246,  &
          WINQUERYVKEYGLYPHXLATETBL PMMERGE.2101,  &
          WINQUERYCHARXLATETBL     PMMERGE.3177,  &
          WINVKEYGLYPHCHARXLATE    PMMERGE.3304,  &
          WINVKEYTOSCANID          PMMERGE.2102,  &
          WINQUERYPROCESSCP        PMMERGE.2103,  &
          WINGETKEYSTATE           PMMERGE.2104,  &
          WINGETPHYSKEYSTATE       PMMERGE.3136,  &
          WINSETKEYBOARDSTATETABLE PMMERGE.3258,  &
          WINENABLEPHYSINPUT       PMMERGE.3112,  &
          WINSETCP                 PMMERGE.2105,  &
          WINQUERYCP               PMMERGE.2106,  &
          WINQUERYCPLIST           PMMERGE.2107,  &
          WINCPTRANSLATESTRING     PMMERGE.2108,  &
          WINCPTRANSLATECHAR       PMMERGE.2109,  &
          WINCOMPARESTRINGS        PMMERGE.2110,  &
          WINUPPER                 PMMERGE.2111,  &
          WINUPPERCHAR             PMMERGE.2112,  &
          WINNEXTCHAR              PMMERGE.2113,  &
          WINPREVCHAR              PMMERGE.2114,  &
          WINCREATEHEAP            PMMERGE.2115,  &
          WINDESTROYHEAP           PMMERGE.2116,  &
          WINAVAILMEM              PMMERGE.2117,  &
          WINALLOCMEM              PMMERGE.2118,  &
          WINREALLOCMEM            PMMERGE.2119,  &
          WINFREEMEM               PMMERGE.2120,  &
          WINLOCKHEAP              PMMERGE.2121,  &
          WINCREATEATOMTABLE       PMMERGE.2122,  &
          WINDESTROYATOMTABLE      PMMERGE.2123,  &
          WINADDATOM               PMMERGE.2124,  &
          WINFINDATOM              PMMERGE.2125,  &
          WINDELETEATOM            PMMERGE.2126,  &
          WINQUERYATOMUSAGE        PMMERGE.2127,  &
          WINQUERYATOMLENGTH       PMMERGE.2128,  &
          WINQUERYATOMNAME         PMMERGE.2129,  &
          WINQUERYSYSTEMATOMTABLE  PMMERGE.2130,  &
          WINSETLASTERROR          PMMERGE.2131,  &
          WINGETLASTERROR          PMMERGE.2132,  &
          WINGETERRORINFO          PMMERGE.2133,  &
          WINFREEERRORINFO         PMMERGE.2134,  &
          WININITIALIZE            PMMERGE.2135,  &
          WINTERMINATE             PMMERGE.2136,  &
          WINCATCH                 PMMERGE.2137,  &
          WINTHROW                 PMMERGE.2138,  &
          WINQUERYVERSION          PMMERGE.2139,  &
          FSRSEMENTER              PMMERGE.4011,  &
          FSRSEMLEAVE              PMMERGE.4013,  &
          FSRSEMEXIT               PMMERGE.2140,  &
          FSRSEMCHECK              PMMERGE.2141,  &
          FSRSEMENTERP             PMMERGE.4012,  &
          AABGETREG                PMMERGE.2142,  &
          AABSETREG                PMMERGE.2143,  &
          VALIDATESELECTOR         PMMERGE.2144,  &
          LOADMODULEHANDLE         PMMERGE.2145,  &
          GETSCREENSELECTOR        PMMERGE.2146,  &
          IDENTIFYCODESELECTOR     PMMERGE.2147,  &
          WINQUERYWINDOWSGID       PMMERGE.3215,  &
          _WINSETERRORINFO         PMMERGE.2148,  &
          WINISPHYSINPUTENABLED    PMMERGE.3151,  &
          WINQUERYWINDOWDC         PMMERGE.3212,  &
          WINDRAWBORDER            PMMERGE.3109,  &
          WINDRAWTEXT              PMMERGE.3026,  &
          WINDRAWBITMAP            PMMERGE.3108,  &
          WINQUERYWINDOWPTR        PMMERGE.3214,  &
          WINSETWINDOWPTR          PMMERGE.3274,  &
          WIN16INITIALIZESYSTEM    PMMERGE.3032,  &
          WINTERMINATESYSTEM       PMMERGE.2149,  &
          WIN16LOCKSTARTINPUT      PMMERGE.3038,  &
          WINMSGSEMWAIT            PMMERGE.3040,  &
          WINMSGMUXSEMWAIT         PMMERGE.3168,  &
          WINSYSTEMSHUTDOWN        PMMERGE.3294,  &
          WINCANCELSHUTDOWN        PMMERGE.3019,  &
          WINSETWINDOWBITS         PMMERGE.3071,  &
          WINGETCLIPPS             PMMERGE.3128,  &
          WINSENDQUEUEMSG          PMMERGE.3240,  &
          WINSETQUEUEPROC          PMMERGE.2183,  &
          WINDEFQUEUEPROC          PMMERGE.2150,  &
          WINSAVEWINDOWPOS         PMMERGE.3234,  &
          WINFOCUSCHANGE           PMMERGE.3125,  &
          WINQUERYQUEUEINFO        PMMERGE.3049,  &
          WINCALLHELPHOOK          PMMERGE.3083,  &
          WINSETATOMTABLEOWNER     PMMERGE.3244,  &
          WINISSHUTTINGDOWN        PMMERGE.3152,  &
          WINTERMINATESHUTDOWN     PMMERGE.3076,  &
          WINSETCLASSMSGINTEREST   PMMERGE.3247,  &
          VISREGIONCALLBACK        PMMERGE.2151,  &
          WINQUERYDLGITEMTEXTLENGTH PMMERGE.3187,  &
          WINQUERYKBDLAYOUT        PMMERGE.2152,  &
          WINSETKBDLAYOUT          PMMERGE.2153,  &
          WINDDEINITIATE           PMMERGE.3097,  &
          WINDDERESPOND            PMMERGE.3099,  &
          WINDDEPOSTMSG            PMMERGE.3098,  &
          WIN16CANCELMODE          PMMERGE.3018,  &
          WINSETPRESPARAM          PMMERGE.3068,  &
          WINQUERYPRESPARAM        PMMERGE.3196,  &
          WINREMOVEPRESPARAM       PMMERGE.3230,  &
          WINTHREADASSOCQUEUE      PMMERGE.3296,  &
          WINQUEUEFROMID           PMMERGE.3220,  &
          WINQUERYSENDMSG          PMMERGE.3201,  &
          WINREPLYMSG              PMMERGE.3233,  &
          WINWAKETHREAD            PMMERGE.3306,  &
          WINLOCKINPUT             PMMERGE.3161,  &
          WINBUILDPTRHANDLE        PMMERGE.2154,  &
          DUMWINCREATEHELPINSTANCE     PMMERGE.2155,  &
          DUMWINDESTROYHELPINSTANCE    PMMERGE.2156,  &
          DUMWINASSOCIATEHELPINSTANCE  PMMERGE.2157,  &
          DUMWINCREATEHELPTABLE        PMMERGE.2158,  &
          DUMWINLOADHELPTABLE          PMMERGE.2159,  &
          DUMWINQUERYHELPINSTANCE      PMMERGE.2160,  &
          WINQUERYWINDOWMODEL          PMMERGE.3213,  &
          WINSETDESKTOPBKGND           PMMERGE.3251,  &
          WINQUERYDESKTOPBKGND         PMMERGE.3184,  &
          WINPOPUPMENU                 PMMERGE.3172,  &
          WINREALIZEPALETTE            PMMERGE.3221,  &
          DUMMYHELPENTRY               PMMERGE.2161,  &
          GETPMDDCODESELECTOR          PMMERGE.2162,  &
          WINSEMENTER                  PMMERGE.3237,  &
          WINSEMLEAVE                  PMMERGE.3238,  &
          WINQUERYPWND                 PMMERGE.2163,  &
          WINQUERYSTATE                PMMERGE.3203,  &
          WINLOADANIPOINTER            PMMERGE.3157,  &
          WINDESTROYANIPOINTER         PMMERGE.3103,  &
          WINSETANIPOINTER             PMMERGE.3243,  &
          WINENUMVKEYSCANIDS           PMMERGE.2164,  &
          WINSEMQUERY                  PMMERGE.3239,  &
          WINLOADTRANSLATIONTABLE      PMMERGE.2165,  &
          WINSETKBDLAYOUT2             PMMERGE.2166,  &
          WINSEAMLESSBOX               PMMERGE.3236,  &
          WINJIGGLEMOUSE               PMMERGE.3156,  &
          WINTRACKSEAMLESS             PMMERGE.3297,  &
          WINREGISTERSEAMLESSPROC      PMMERGE.3222,  &
          ABNORMALPROCESSEXIT          PMMERGE.5002,  &
          WINPROFCONTROL               PMMERGE.5422,  &
          DEBUGINPUTENABLE             PMMERGE.2167,  &
          DEBUGINPUT                   PMMERGE.2168,  &
          DEBUGOUTPUT                  PMMERGE.2169,  &
          DEBUGCURPOS                  PMMERGE.2170,  &
          DEBUGCLS                     PMMERGE.2171,  &
          WINDEBUGBREAK                PMMERGE.3100,  &
          DEBUGREDIRECT                PMMERGE.2172,  &
          INPUTGETDEVICEINFO           PMMERGE.3002,  &
          STARTTIMER                   PMMERGE.5107,  &
          DESTROYTIMERS                PMMERGE.5007,  &
          WINDELETELIBRARY             PMMERGE.3101,  &
          WINLOADPROCEDURE             PMMERGE.3160,  &
          WINDELETEPROCEDURE           PMMERGE.3102,  &
          WINSETMSGMODE                PMMERGE.3261,  &
          WINSETSYNCHROMODE            PMMERGE.3268,  &
          WINGETDLGMSG                 PMMERGE.3130,  &
          WINREGISTERUSERMSG           PMMERGE.3224,  &
          WINQUERYANCHORBLOCK          PMMERGE.3046,  &
          WINLANGSUPWINDOWPROC         PMMERGE.2173,  &
          WINLANGSUPDLGPROC            PMMERGE.2174,  &
          WINREGISTERUSERDATATYPE      PMMERGE.3223,  &
          FARCALLHOOK                  PMMERGE.5045,  &
          WINISWINDOWSHOWING           PMMERGE.3155,  &
          WINLOADLIBRARY               PMMERGE.3158,  &
          WINCREATEPOINTERINDIRECT     PMMERGE.2175,  &
          WINQUERYPOINTERINFO          PMMERGE.2176,  &
          WINSETMENUDATA               PMMERGE.3259,  &
          WINQUERYMENUDATA             PMMERGE.3190,  &
          WINMENUINDEXFROMID           PMMERGE.3167,  &
          WINFORCEMENUREDRAW           PMMERGE.3126,  &
          WINGETSCANSTATE              PMMERGE.3138,  &
          WINSETSCANSTATETABLE         PMMERGE.3266,  &
          WINGETERASEPS                PMMERGE.3131,  &
          WINRELEASEERASEPS            PMMERGE.3226,  &
          WINSETWINDOWDC               PMMERGE.3273,  &
          WINTRANSLATECHAR             PMMERGE.3299,  &
          WINPORTHOLEUTIL              PMMERGE.2177,  &
          WINEXPLODEWINDOW             PMMERGE.3123,  &
          WINSETACCELTABLEVALUE        PMMERGE.3242,  &
          WINQUERYACCELTABLEVALUE      PMMERGE.3175,  &
          WINSTRETCHPOINTER            PMMERGE.3286,  &
          WINSETPOINTEROWNER           PMMERGE.3264,  &
          WINGETFRAMETREEPPSIZE        PMMERGE.3133,  &
          WINGETFRAMETREEPPS           PMMERGE.3132,  &
          WINSETFRAMETREEPPS           PMMERGE.3256,  &
          WIN16GETMINWINDOWVIEWER      PMMERGE.3029,  &
          WINQUERYWORKPLACEWINDOW      PMMERGE.3219,  &
          WINLOCKPOINTERUPDATE         PMMERGE.3162,  &
          WINCHECKINPUT                PMMERGE.3088,  &
          FARFLUSHBUFHOOK              PMMERGE.3000,  &
          WinAddAtom                   PMMERGE.5111,  &
          WinAlarm                     PMMERGE.5113,  &
          WinBeginEnumWindows          PMMERGE.5114,  &
          WinBeginPaint                PMMERGE.5115,  &
          WinCalcFrameRect             PMMERGE.5118,  &
          WinCancelShutdown            PMMERGE.5121,  &
          WinCloseClipbrd              PMMERGE.5124,  &
          WinCompareStrings            PMMERGE.5125,  &
          WinCopyAccelTable            PMMERGE.5126,  &
          WinCopyRect                  PMMERGE.5127,  &
          WinCpTranslateChar           PMMERGE.5128,  &
          WinCpTranslateString         PMMERGE.5129,  &
          WinCreateAccelTable          PMMERGE.5130,  &
          WinCreateAtomTable           PMMERGE.5131,  &
          WinCreateCursor              PMMERGE.5132,  &
          WinCreatePointer             PMMERGE.5137,  &
          WinDdeInitiate               PMMERGE.5142,  &
          WinDdePostMsg                PMMERGE.5143,  &
          WinDdeRespond                PMMERGE.5144,  &
          WinDeleteAtom                PMMERGE.5148,  &
          WinDeleteLibrary             PMMERGE.5149,  &
          WinDestroyAccelTable         PMMERGE.5151,  &
          WinDestroyAtomTable          PMMERGE.5152,  &
          WinDestroyCursor             PMMERGE.5153,  &
          WinDestroyPointer            PMMERGE.5155,  &
          WinDismissDlg                PMMERGE.5157,  &
          WinDrawBitmap                PMMERGE.5160,  &
          WinDrawBorder                PMMERGE.5161,  &
          WinDrawPointer               PMMERGE.5162,  &
          WinEmptyClipbrd              PMMERGE.5164,  &
          WinEnablePhysInput           PMMERGE.5165,  &
          WinEnableWindow              PMMERGE.5166,  &
          WinEnableWindowUpdate        PMMERGE.5167,  &
          WinEndEnumWindows            PMMERGE.5168,  &
          WinEndPaint                  PMMERGE.5169,  &
          WinEnumClipbrdFmts           PMMERGE.5171,  &
          WinEnumDlgItem               PMMERGE.5172,  &
          WinEqualRect                 PMMERGE.5173,  &
          WinExcludeUpdateRegion       PMMERGE.5174,  &
          WinFillRect                  PMMERGE.5176,  &
          WinFindAtom                  PMMERGE.5177,  &
          WinFlashWindow               PMMERGE.5178,  &
          WinFocusChange               PMMERGE.5179,  &
          WinFreeErrorInfo             PMMERGE.5180,  &
          WinGetClipPS                 PMMERGE.5181,  &
          WinGetCurrentTime            PMMERGE.5182,  &
          WinGetErrorInfo              PMMERGE.5185,  &
          WinGetKeyState               PMMERGE.5188,  &
          WinGetLastError              PMMERGE.5189,  &
          WinGetMaxPosition            PMMERGE.5190,  &
          WinGetMinPosition            PMMERGE.5191,  &
          WinGetNextWindow             PMMERGE.5193,  &
          WinGetPS                     PMMERGE.5195,  &
          WinGetPhysKeyState           PMMERGE.5194,  &
          WinGetScreenPS               PMMERGE.5196,  &
          WinGetSysBitmap              PMMERGE.5197,  &
          WinInSendMsg                 PMMERGE.5202,  &
          WinInflateRect               PMMERGE.5200,  &
          WinIntersectRect             PMMERGE.5203,  &
          WinInvalidateRect            PMMERGE.5204,  &
          WinInvalidateRegion          PMMERGE.5205,  &
          WinInvertRect                PMMERGE.5206,  &
          WinIsChild                   PMMERGE.5207,  &
          WinIsPhysInputEnabled        PMMERGE.5208,  &
          WinIsRectEmpty               PMMERGE.5209,  &
          WinIsThreadActive            PMMERGE.5210,  &
          WinIsWindow                  PMMERGE.5211,  &
          WinIsWindowEnabled           PMMERGE.5212,  &
          WinIsWindowShowing           PMMERGE.5213,  &
          WinIsWindowVisible           PMMERGE.5214,  &
          WinLoadAccelTable            PMMERGE.5216,  &
          WinLoadLibrary               PMMERGE.5219,  &
          WinLoadMenu                  PMMERGE.5220,  &
          WinLoadMessage               PMMERGE.5221,  &
          WinLoadPointer               PMMERGE.5222,  &
          WinLoadString                PMMERGE.5224,  &
          WinLockVisRegions            PMMERGE.5227,  &
          WinLockWindowUpdate          PMMERGE.5228,  &
          WinMakePoints                PMMERGE.5229,  &
          WinMakeRect                  PMMERGE.5230,  &
          WinMapDlgPoints              PMMERGE.5231,  &
          WinMapWindowPoints           PMMERGE.5232,  &
          WinMessageBox                PMMERGE.5233,  &
          WinMsgSemWait                PMMERGE.5236,  &
          WinNextChar                  PMMERGE.5238,  &
          WinOffsetRect                PMMERGE.5239,  &
          WinOpenClipbrd               PMMERGE.5240,  &
          WinOpenWindowDC              PMMERGE.5241,  &
          WinPrevChar                  PMMERGE.5246,  &
          WinProcessDlg                PMMERGE.5247,  &
          WinPtInRect                  PMMERGE.5248,  &
          WinQueryAccelTable           PMMERGE.5249,  &
          WinQueryActiveWindow         PMMERGE.5251,  &
          WinQueryAnchorBlock          PMMERGE.5252,  &
          WinQueryAtomLength           PMMERGE.5253,  &
          WinQueryAtomName             PMMERGE.5254,  &
          WinQueryAtomUsage            PMMERGE.5255,  &
          WinQueryCapture              PMMERGE.5256,  &
          WinQueryClassName            PMMERGE.5258,  &
          WinQueryClipbrdData          PMMERGE.5260,  &
          WinQueryClipbrdFmtInfo       PMMERGE.5261,  &
          WinQueryClipbrdOwner         PMMERGE.5262,  &
          WinQueryClipbrdViewer        PMMERGE.5263,  &
          WinQueryCp                   PMMERGE.5264,  &
          WinQueryCpList               PMMERGE.5265,  &
          WinQueryCursorInfo           PMMERGE.5266,  &
          WinQueryDesktopWindow        PMMERGE.5268,  &
          WinQueryDlgItemShort         PMMERGE.5269,  &
          WinQueryDlgItemText          PMMERGE.5270,  &
          WinQueryDlgItemTextLength    PMMERGE.5271,  &
          WinQueryFocus                PMMERGE.5272,  &
          WinQueryMsgPos               PMMERGE.5273,  &
          WinQueryMsgTime              PMMERGE.5274,  &
          WinQueryObjectWindow         PMMERGE.5275,  &
          WinQueryPointer              PMMERGE.5277,  &
          WinQueryPointerInfo          PMMERGE.5278,  &
          WinQueryPointerPos           PMMERGE.5279,  &
          WinQueryQueueInfo            PMMERGE.5282,  &
          WinQueryQueueStatus          PMMERGE.5283,  &
          WinQuerySysColor             PMMERGE.5289,  &
          WinQuerySysModalWindow       PMMERGE.5290,  &
          WinQuerySysPointer           PMMERGE.5291,  &
          WinQuerySysValue             PMMERGE.5294,  &
          WinQuerySystemAtomTable      PMMERGE.5293,  &
          WinQueryUpdateRect           PMMERGE.5297,  &
          WinQueryUpdateRegion         PMMERGE.5298,  &
          WinQueryVersion              PMMERGE.5299,  &
          WinQueryWindow               PMMERGE.5301,  &
          WinQueryWindowDC             PMMERGE.5302,  &
          WinQueryWindowPos            PMMERGE.5304,  &
          WinQueryWindowProcess        PMMERGE.5305,  &
          WinQueryWindowPtr            PMMERGE.5306,  &
          WinQueryWindowRect           PMMERGE.5307,  &
          WinQueryWindowText           PMMERGE.5308,  &
          WinQueryWindowTextLength     PMMERGE.5309,  &
          WinQueryWindowULong          PMMERGE.5311,  &
          WinQueryWindowUShort         PMMERGE.5312,  &
          WinRegisterUserDatatype      PMMERGE.5318,  &
          WinRegisterUserMsg           PMMERGE.5319,  &
          WinReleasePS                 PMMERGE.5322,  &
          WinScrollWindow              PMMERGE.5328,  &
          WinSetAccelTable             PMMERGE.5335,  &
          WinSetActiveWindow           PMMERGE.5337,  &
          WinSetCapture                PMMERGE.5338,  &
          WinSetClassMsgInterest       PMMERGE.5340,  &
          WinSetClipbrdData            PMMERGE.5342,  &
          WinSetClipbrdOwner           PMMERGE.5343,  &
          WinSetClipbrdViewer          PMMERGE.5344,  &
          WinSetCp                     PMMERGE.5345,  &
          WinSetDlgItemShort           PMMERGE.5347,  &
          WinSetDlgItemText            PMMERGE.5348,  &
          WinSetFocus                  PMMERGE.5350,  &
          WinSetMsgInterest            PMMERGE.5354,  &
          WinSetMsgMode                PMMERGE.5355,  &
          WinSetMultWindowPos          PMMERGE.5356,  &
          WinSetOwner                  PMMERGE.5357,  &
          WinSetParent                 PMMERGE.5359,  &
          WinSetPointer                PMMERGE.5360,  &
          WinSetPointerPos             PMMERGE.5362,  &
          WinSetRect                   PMMERGE.5364,  &
          WinSetRectEmpty              PMMERGE.5365,  &
          WinSetSynchroMode            PMMERGE.5366,  &
          WinSetSysColors              PMMERGE.5368,  &
          WinSetSysModalWindow         PMMERGE.5370,  &
          WinSetSysValue               PMMERGE.5372,  &
          WinSetWindowBits             PMMERGE.5374,  &
          WinSetWindowPos              PMMERGE.5375,  &
          WinSetWindowPtr              PMMERGE.5376,  &
          WinSetWindowText             PMMERGE.5377,  &
          WinSetWindowULong            PMMERGE.5379,  &
          WinSetWindowUShort           PMMERGE.5380,  &
          WinShowCursor                PMMERGE.5381,  &
          WinShowPointer               PMMERGE.5382,  &
          WinShowTrackRect             PMMERGE.5383,  &
          WinShowWindow                PMMERGE.5384,  &
          WinStartTimer                PMMERGE.5386,  &
          WinStopTimer                 PMMERGE.5387,  &
          WinSubstituteStrings         PMMERGE.5390,  &
          WinSubtractRect              PMMERGE.5391,  &
          WinTrackRect                 PMMERGE.5399,  &
          WinUnionRect                 PMMERGE.5402,  &
          WinUpdateWindow              PMMERGE.5403,  &
          WinUpper                     PMMERGE.5404,  &
          WinUpperChar                 PMMERGE.5405,  &
          WinValidateRect              PMMERGE.5406,  &
          WinValidateRegion            PMMERGE.5407,  &
          WinWaitMsg                   PMMERGE.5410,  &
          WinWindowFromDC              PMMERGE.5413,  &
          WinWindowFromID              PMMERGE.5414,  &
          WinWindowFromPoint           PMMERGE.5415,  &
          WinBroadcastMsg              PMMERGE.5116,  &
          WinPostQueueMsg              PMMERGE.5245,  &
          WinSendDlgItemMsg            PMMERGE.5333,  &
          WinTranslateAccel            PMMERGE.5401,  &
          WinCallMsgFilter             PMMERGE.5120,  &
          WinCreateFrameControls       PMMERGE.5134,  &
          WinCreateMenu                PMMERGE.5135,  &
          WinCreateWindow              PMMERGE.5141,  &
          WinDefDlgProc                PMMERGE.5146,  &
          WinDrawText                  PMMERGE.5163,  &
          WinGetDlgMsg                 PMMERGE.5183,  &
          WinMsgMuxSemWait             PMMERGE.5235,  &
          WinMultWindowFromIDs         PMMERGE.5237,  &
          WinPeekMsg                   PMMERGE.5242,  &
          WinPostMsg                   PMMERGE.5244,  &
          WinSetKeyboardStateTable     PMMERGE.5353,  &
          WinCreateDlg                 PMMERGE.5133,  &
          WinDlgBox                    PMMERGE.5159,  &
          WinLoadDlg                   PMMERGE.5218,  &
          WinQueryClassInfo            PMMERGE.5257,  &
          WinReleaseHook               PMMERGE.5321,  &
          WinSetHook                   PMMERGE.5352,  &
          WinSubclassWindow            PMMERGE.5389,  &
          WinSetClassThunkProc         PMMERGE.5341,  &
          WinQueryClassThunkProc       PMMERGE.5259,  &
          WinSetWindowThunkProc        PMMERGE.5378,  &
          WinQueryWindowThunkProc      PMMERGE.5310,  &
          WinQueryWindowModel          PMMERGE.5303,  &
          WinSetDesktopBkgnd           PMMERGE.5346,  &
          WinQueryDesktopBkgnd         PMMERGE.5267,  &
          WinPopupMenu                 PMMERGE.5243,  &
          WinSetPresParam              PMMERGE.5363,  &
          WinQueryPresParam            PMMERGE.5280,  &
          WinRemovePresParam           PMMERGE.5323,  &
          WinRealizePalette            PMMERGE.5314,  &
          WinCreatePointerIndirect     PMMERGE.5138,  &
          WinSaveWindowPos             PMMERGE.5327,  &
          WINCREATEFARHEAP             PMMERGE.2178,  &
          WINDESTROYFARHEAP            PMMERGE.2179,  &
          WINALLOCFARMEM               PMMERGE.2180,  &
          WINFREEFARMEM                PMMERGE.2181,  &
          WINREALLOCFARMEM             PMMERGE.2182,  &
          WINGETERASEPS                PMMERGE.5184,  &
          WINRELEASEERASEPS            PMMERGE.5320,  &
          DEBUG32INPUT                 PMMERGE.5005,  &
          DEBUG32OUTPUT                PMMERGE.5006,  &
          DEBUG32CURPOS                PMMERGE.5004,  &
          DEBUG32CLS                   PMMERGE.5003,  &
          Win32DebugBreak              PMMERGE.5145,  &
          WINSETCLASSTHUNKPROC         PMMERGE.3248,  &
          WINQUERYCLASSTHUNKPROC       PMMERGE.3179,  &
          WINSETWINDOWTHUNKPROC        PMMERGE.3275,  &
          WINQUERYWINDOWTHUNKPROC      PMMERGE.3218,  &
          WinSystemShutdown            PMMERGE.5395,  &
          WinBuildPtrHandle            PMMERGE.5117,  &
          WinExplodeWindow             PMMERGE.5175,  &
          WinSetAccelTableValue        PMMERGE.5336,  &
          WinQueryAccelTableValue      PMMERGE.5250,  &
          WinStretchPointer            PMMERGE.5388,  &
          AAB32GETREG                  PMMERGE.5000,  &
          AAB32SETREG                  PMMERGE.5001,  &
          WinSetPointerOwner           PMMERGE.5361,  &
          WinGetFrameTreePPSize        PMMERGE.5187,  &
          WinGetFrameTreePPs           PMMERGE.5186,  &
          WinSetFrameTreePPs           PMMERGE.5351,  &
          WinQueryProcessCP            PMMERGE.5281,  &
          WinVisRegionNotify           PMMERGE.5408,  &
          WinSetErrorInfo              PMMERGE.5349,  &
          WinWaitEventSem              PMMERGE.5409,  &
          WinRequestMutexSem           PMMERGE.5326,  &
          WinWaitMuxWaitSem            PMMERGE.5411,  &
          WINSETSYSCLIPBRDVIEWER       PMMERGE.3269,  &
          WinSetSysClipbrdViewer       PMMERGE.5367,  &
          WinSemQuery                  PMMERGE.5332,  &
          WinSemEnter                  PMMERGE.5330,  &
          WinSemLeave                  PMMERGE.5331,  &
          WinLoadProcedure             PMMERGE.5223,  &
          WinDeleteProcedure           PMMERGE.5150,  &
          WinMoveCursor                PMMERGE.5234,  &
          WinQuerySendMsg              PMMERGE.5284,  &
          WinReplyMsg                  PMMERGE.5325,  &
          WinWakeThread                PMMERGE.5412,  &
          WinQueueFromID               PMMERGE.5313,  &
          WinThreadAssocQueue          PMMERGE.5398,  &
          WinLockInput                 PMMERGE.5225,  &
          WinSetSysFont                PMMERGE.5369,  &
          WinLockPointerUpdate         PMMERGE.5226,  &
          WinCheckInput                PMMERGE.5123,  &
          Far32LockupHook              PMMERGE.5044,  &
          WinQueryVisibleRegion        PMMERGE.5300,  &
          WinSetVisibleRegionNotify    PMMERGE.5373,  &
          WinSetSysPointerData         PMMERGE.5371,  &
          WinQuerySysPointerData       PMMERGE.5292,  &
          WinSeamlessBox               PMMERGE.5329,  &
          WinJiggleMouse               PMMERGE.5215,  &
          WinTrackSeamless             PMMERGE.5400,  &
          WinRegisterSeamlessProc      PMMERGE.5317,  &
          WinSetCharXlateTbl           PMMERGE.5339,  &
          WinMspInit                   PMMERGE.5420,  &
          WinMspAlloc                  PMMERGE.5418,  &
          WinMspFree                   PMMERGE.5419,  &
          ThkDos16SetFgnd              PMMERGE.5109,  &
          InputGetDeviceInfo           PMMERGE.5068,  &
          WinRegisterPen               PMMERGE.5316,  &
          WinMessageBox2               PMMERGE.5417,  &
          fPenAvail                    PMMERGE.6201,  &
          WinDrawTabbedText            PMMERGE.5432,  &
          PMREQUESTMUTEXSEM            PMMERGE.5071,  &
          PMRELEASEMUTEXSEM            PMMERGE.5070,  &
          WinQueryPointerClipRect      PMMERGE.5433,  &
          WinSetPointerClipRect        PMMERGE.5434,  &
          WinWaitMsgTimeout            PMMERGE.5436,  &
          WinIsWindowOfClass           PMMERGE.5437,  &
          WinSetAtomTableOwner         PMMERGE.5438,  &
          WinXxCreateHeap              PMMERGE.5425,  &
          WinXxDestroyHeap             PMMERGE.5426,  &
          WinXxAllocMem                PMMERGE.5424,  &
          WinXxFreeMem                 PMMERGE.5427,  &
          WinXxAllocFill               PMMERGE.5423,  &
          WinXxReallocMem              PMMERGE.5429,  &
          WinXxQueryMemSize            PMMERGE.5439,  &
          WinSetClipRegion             PMMERGE.5440,  &
          WinQueryClipRegion           PMMERGE.5441,  &
          Win32AddClipbrdViewer        PMMERGE.5442,  &
          Win32RemoveClipbrdViewer     PMMERGE.5443,  &
          Win32QueryClipbrdViewerChain PMMERGE.5444,  &
          Win32IncrementAtomUsage      PMMERGE.5445,  &
          DaxQueryOpenClipbrdWindow    PMMERGE.5446,  &
          WinSetDaxData                PMMERGE.5448,  &
          WinQueryDaxData              PMMERGE.5447,  &
          WINENUMPROPERTIES            PMMERGE.5449,  &
          WINQUERYPROPERTY             PMMERGE.5450,  &
          WINREMOVEPROPERTY            PMMERGE.5451,  &
          WINSETPROPERTY               PMMERGE.5452,  &
          WINENUMCLASSPROPERTIES       PMMERGE.5453,  &
          WINQUERYCLASSPROPERTY        PMMERGE.5454,  &
          WINREMOVECLASSPROPERTY       PMMERGE.5455,  &
          WINSETCLASSPROPERTY          PMMERGE.5456,  &
          Win32SetKbdLayout            PMMERGE.5458,  &
          Win32GetAFCValue             PMMERGE.5461,  &
          Win32SetAFCValue             PMMERGE.5462,  &
          WinLoadVkeyGlyphXlatetbl     PMMERGE.5463,  &
          WinSetVKeyGlyphXlatetbl      PMMERGE.5464,  &
          WinQueryVKeyGlyphXlatetbl    PMMERGE.5465,  &
          WinTrackWindow               PMMERGE.5466,  &
          WinSemEnterTimeout           PMMERGE.5467,  &
          WinSetDesktopWorkArea        PMMERGE.5468,  &
          WinQueryDesktopWorkArea      PMMERGE.5469,  &
          Win32TranslateChar2          PMMERGE.5473,  &
          Win32WaitForIdleThread       PMMERGE.5474,  &
          WinXXCreateHeap2             PMMERGE.5475,  &
          WinQuerySharedHeapData       PMMERGE.5476,  &
          Win32QueryActiveOtherWindow  PMMERGE.5477,  &
          Win32EnumVkeyScanIDs         PMMERGE.5478,  &
          WINSETLANGINFO               PMMERGE.3500,  &
          WINQUERYLANGINFO             PMMERGE.3501,  &
          WINSETKBDLAYER               PMMERGE.3502,  &
          WINQUERYKBDLAYER             PMMERGE.3503,  &
          WINSETLANGVIEWER             PMMERGE.3504,  &
          WINQUERYLANGVIEWER           PMMERGE.3505,  &
          WINPOUREVENT                 PMMERGE.3600,  &
          WINSETMSGQIMINFO             PMMERGE.3601,  &
          WINQUERYMSGQIMINFO           PMMERGE.3602,  &
          DaxCreateCaret               PMMERGE.10000,  &
          DaxDestroyCaret              PMMERGE.10001,  &
          DaxGetCaretBlinkTime         PMMERGE.10002,  &
          DaxHideCaret                 PMMERGE.10004,  &
          DaxSetCaretBlinkTime         PMMERGE.10005,  &
          DaxShowCaret                 PMMERGE.10007,  &
          DaxShowCursor                PMMERGE.10018,  &
          DaxSwapMouseButton           PMMERGE.10019,  &
          LookupAtom                   PMMERGE.10020,  &
          DaxOpenClipbrd               PMMERGE.10021

EXPORTS = WinInitialize.              00763,  &
          WinTerminate.               00888,  &
          WinCreateMsgQueue.          00716,  &
          WinDestroyMsgQueue.         00726,  &
          WinDestroyWindow.           00728,  &
          WinCreateStdWindow.         00908,  &
          WinDefWindowProc.           00911,  &
          WinDispatchMsg.             00912,  &
          WinGetMsg.                  00915,  &
          WinSendMsg.                 00920,  &
          WinRegisterClass.           00926,  &
          WINQUERYDESKTOPWINDOW       .00001 RESIDENT,  &
          WINQUERYOBJECTWINDOW        .00002 RESIDENT,  &
          WINREGISTERCLASS            .00003 RESIDENT,  &
          WINQUERYCLASSNAME           .00004 RESIDENT,  &
          WINQUERYCLASSINFO           .00005 RESIDENT,  &
          WINCREATEWINDOW             .00006 RESIDENT,  &
          WINDESTROYWINDOW            .00007 RESIDENT,  &
          WINSETWINDOWPOS             .00008 RESIDENT,  &
          WINSETMULTWINDOWPOS         .00009 RESIDENT,  &
          WINQUERYWINDOWPOS           .00010 RESIDENT,  &
          WINENABLEWINDOW             .00011 RESIDENT,  &
          WINISWINDOWENABLED          .00012 RESIDENT,  &
          WINSHOWWINDOW               .00013 RESIDENT,  &
          WINENABLEWINDOWUPDATE       .00014 RESIDENT,  &
          WINISWINDOWVISIBLE          .00015 RESIDENT,  &
          WINSETWINDOWTEXT            .00016 RESIDENT,  &
          WINQUERYWINDOWTEXT          .00017 RESIDENT,  &
          WINQUERYWINDOWTEXTLENGTH    .00018 RESIDENT,  &
          WINWINDOWFROMID             .00019 RESIDENT,  &
          WINMULTWINDOWFROMIDS        .00020 RESIDENT,  &
          WINISWINDOW                 .00021 RESIDENT,  &
          WINISCHILD                  .00022 RESIDENT,  &
          WINSETPARENT                .00023 RESIDENT,  &
          WINSETOWNER                 .00024 RESIDENT,  &
          WINQUERYWINDOW              .00025 RESIDENT,  &
          WINQUERYWINDOWRECT          .00026 RESIDENT,  &
          WINQUERYWINDOWPROCESS       .00027 RESIDENT,  &
          WINSETWINDOWUSHORT          .00028 RESIDENT,  &
          WINQUERYWINDOWUSHORT        .00029 RESIDENT,  &
          WINSETWINDOWULONG           .00030 RESIDENT,  &
          WINQUERYWINDOWULONG         .00031 RESIDENT,  &
          WINBEGINENUMWINDOWS         .00032 RESIDENT,  &
          WINGETNEXTWINDOW            .00033 RESIDENT,  &
          WINENDENUMWINDOWS           .00034 RESIDENT,  &
          WINWINDOWFROMPOINT          .00035 RESIDENT,  &
          WINMAPWINDOWPOINTS          .00036 RESIDENT,  &
          WINSUBCLASSWINDOW           .00037 RESIDENT,  &
          WINLOCKWINDOW               .00038 RESIDENT,  &
          WINQUERYWINDOWLOCKCOUNT     .00039 RESIDENT,  &
          WINREGISTERWINDOWDESTROY    .00040 RESIDENT,  &
          WINOPENWINDOWDC             .00041 RESIDENT,  &
          WINGETSCREENPS              .00042 RESIDENT,  &
          WINGETPS                    .00043 RESIDENT,  &
          WINRELEASEPS                .00044 RESIDENT,  &
          WINBEGINPAINT               .00045 RESIDENT,  &
          WINENDPAINT                 .00046 RESIDENT,  &
          WININVALIDATERECT           .00047 RESIDENT,  &
          WININVALIDATEREGION         .00048 RESIDENT,  &
          WINVALIDATERECT             .00049 RESIDENT,  &
          WINVALIDATEREGION           .00050 RESIDENT,  &
          WINQUERYUPDATERECT          .00051 RESIDENT,  &
          WINQUERYUPDATEREGION        .00052 RESIDENT,  &
          WINUPDATEWINDOW             .00053 RESIDENT,  &
          WINEXCLUDEUPDATEREGION      .00054 RESIDENT,  &
          WINLOCKWINDOWUPDATE         .00055 RESIDENT,  &
          WINLOCKVISREGIONS           .00056 RESIDENT,  &
          WINWINDOWFROMDC             .00057 RESIDENT,  &
          WINCREATEMSGQUEUE           .00058 RESIDENT,  &
          WINDESTROYMSGQUEUE          .00059 RESIDENT,  &
          WINQUERYQUEUESTATUS         .00060 RESIDENT,  &
          WINSENDMSG                  .00061 RESIDENT,  &
          WINBROADCASTMSG             .00063 RESIDENT,  &
          WININSENDMSG                .00064 RESIDENT,  &
          WINGETMSG                   .00065 RESIDENT,  &
          WINPEEKMSG                  .00066 RESIDENT,  &
          WINWAITMSG                  .00067 RESIDENT,  &
          WINDISPATCHMSG              .00068 RESIDENT,  &
          WINPOSTMSG                  .00069 RESIDENT,  &
          WINPOSTQUEUEMSG             .00070 RESIDENT,  &
          WINQUERYMSGPOS              .00071 RESIDENT,  &
          WINQUERYMSGTIME             .00072 RESIDENT,  &
          WINCALLMSGFILTER            .00073 RESIDENT,  &
          WINSETMSGINTEREST           .00074 RESIDENT,  &
          WINSETCAPTURE               .00075 RESIDENT,  &
          WINQUERYCAPTURE             .00076 RESIDENT,  &
          WINSETFOCUS                 .00077 RESIDENT,  &
          WINQUERYFOCUS               .00078 RESIDENT,  &
          WINSETACTIVEWINDOW          .00079 RESIDENT,  &
          WINQUERYACTIVEWINDOW        .00080 RESIDENT,  &
          WINSETSYSMODALWINDOW        .00081 RESIDENT,  &
          WINQUERYSYSMODALWINDOW      .00082 RESIDENT,  &
          WINISTHREADACTIVE           .00083 RESIDENT,  &
          WINSTARTTIMER               .00084 RESIDENT,  &
          WINSTOPTIMER                .00085 RESIDENT,  &
          WINGETCURRENTTIME           .00086 RESIDENT,  &
          WINLOADPOINTER              .00087 RESIDENT,  &
          WINCREATEPOINTER            .00088 RESIDENT,  &
          WINDESTROYPOINTER           .00089 RESIDENT,  &
          WINQUERYSYSPOINTER          .00091 RESIDENT,  &
          WINSETPOINTER               .00092 RESIDENT,  &
          WINQUERYPOINTER             .00093 RESIDENT,  &
          WINSHOWPOINTER              .00094 RESIDENT,  &
          WINSETPOINTERPOS            .00095 RESIDENT,  &
          WINQUERYPOINTERPOS          .00096 RESIDENT,  &
          WINGETSYSBITMAP             .00097 RESIDENT,  &
          WINCREATECURSOR             .00098 RESIDENT,  &
          WINDESTROYCURSOR            .00099 RESIDENT,  &
          WINSHOWCURSOR               .00100 RESIDENT,  &
          WINQUERYCURSORINFO          .00101 RESIDENT,  &
          WINLOADACCELTABLE           .00102 RESIDENT,  &
          WINCREATEACCELTABLE         .00103 RESIDENT,  &
          WINDESTROYACCELTABLE        .00104 RESIDENT,  &
          WINTRANSLATEACCEL           .00105 RESIDENT,  &
          WINSETACCELTABLE            .00106 RESIDENT,  &
          WINQUERYACCELTABLE          .00107 RESIDENT,  &
          WINCOPYACCELTABLE           .00108 RESIDENT,  &
          WINSETHOOK                  .00109 RESIDENT,  &
          WINRELEASEHOOK              .00110 RESIDENT,  &
          WINOPENCLIPBRD              .00111 RESIDENT,  &
          WINCLOSECLIPBRD             .00112 RESIDENT,  &
          WINEMPTYCLIPBRD             .00113 RESIDENT,  &
          WINSETCLIPBRDOWNER          .00114 RESIDENT,  &
          WINQUERYCLIPBRDOWNER        .00115 RESIDENT,  &
          WINSETCLIPBRDDATA           .00116 RESIDENT,  &
          WINQUERYCLIPBRDDATA         .00117 RESIDENT,  &
          WINENUMCLIPBRDFMTS          .00118 RESIDENT,  &
          WINQUERYCLIPBRDFMTINFO      .00119 RESIDENT,  &
          WINSETCLIPBRDVIEWER         .00120 RESIDENT,  &
          WINQUERYCLIPBRDVIEWER       .00121 RESIDENT,  &
          WINLOADDLG                  .00122 RESIDENT,  &
          WINCREATEDLG                .00123 RESIDENT,  &
          WINPROCESSDLG               .00124 RESIDENT,  &
          WINDLGBOX                   .00125 RESIDENT,  &
          WINDISMISSDLG               .00126 RESIDENT,  &
          WINSENDDLGITEMMSG           .00127 RESIDENT,  &
          WINSETDLGITEMSHORT          .00128 RESIDENT,  &
          WINQUERYDLGITEMSHORT        .00129 RESIDENT,  &
          WINMAPDLGPOINTS             .00130 RESIDENT,  &
          WINSUBSTITUTESTRINGS        .00132 RESIDENT,  &
          WINENUMDLGITEM              .00133 RESIDENT,  &
          WINSETDLGITEMTEXT           .00134 RESIDENT,  &
          WINQUERYDLGITEMTEXT         .00135 RESIDENT,  &
          WINLOADMENU                 .00136 RESIDENT,  &
          WINCREATEMENU               .00137 RESIDENT,  &
          WINALARM                    .00138 RESIDENT,  &
          WINMESSAGEBOX               .00139 RESIDENT,  &
          WINCREATESTDWINDOW          .00140 RESIDENT,  &
          WINCREATEFRAMECONTROLS      .00141 RESIDENT,  &
          WINCALCFRAMERECT            .00143 RESIDENT,  &
          WINFLASHWINDOW              .00144 RESIDENT,  &
          WINGETMINPOSITION           .00146 RESIDENT,  &
          WINGETMAXPOSITION           .00147 RESIDENT,  &
          WINQUERYSYSVALUE            .00149 RESIDENT,  &
          WINSETSYSVALUE              .00150 RESIDENT,  &
          WINSETSYSCOLORS             .00151 RESIDENT,  &
          WINQUERYSYSCOLOR            .00152 RESIDENT,  &
          WINSCROLLWINDOW             .00153 RESIDENT,  &
          WINTRACKRECT                .00154 RESIDENT,  &
          WINSHOWTRACKRECT            .00155 RESIDENT,  &
          GPILOADBITMAP               .00156 RESIDENT,  &
          WINLOADSTRING               .00157 RESIDENT,  &
          WINLOADMESSAGE              .00158 RESIDENT,  &
          WINSETRECTEMPTY             .00159 RESIDENT,  &
          WINSETRECT                  .00160 RESIDENT,  &
          WINCOPYRECT                 .00161 RESIDENT,  &
          WINISRECTEMPTY              .00162 RESIDENT,  &
          WINEQUALRECT                .00163 RESIDENT,  &
          WINPTINRECT                 .00164 RESIDENT,  &
          WINOFFSETRECT               .00165 RESIDENT,  &
          WININFLATERECT              .00166 RESIDENT,  &
          WININTERSECTRECT            .00167 RESIDENT,  &
          WINUNIONRECT                .00168 RESIDENT,  &
          WINSUBTRACTRECT             .00169 RESIDENT,  &
          WINMAKERECT                 .00170 RESIDENT,  &
          WINMAKEPOINTS               .00171 RESIDENT,  &
          WININVERTRECT               .00172 RESIDENT,  &
          WINFILLRECT                 .00173 RESIDENT,  &
          KBDPACKET                   .00174 RESIDENT,  &
          WINDRAWPOINTER              .00177 RESIDENT,  &
          WINDEFWINDOWPROC            .00178 RESIDENT,  &
          WINDEFDLGPROC               .00179 RESIDENT,  &
          WINGETKEYSTATE              .00211 RESIDENT,  &
          WINGETPHYSKEYSTATE          .00212 RESIDENT,  &
          WINSETKEYBOARDSTATETABLE    .00213 RESIDENT,  &
          WINENABLEPHYSINPUT          .00214 RESIDENT,  &
          WINSETCP                    .00215 RESIDENT,  &
          WINQUERYCP                  .00216 RESIDENT,  &
          WINQUERYCPLIST              .00217 RESIDENT,  &
          WINCPTRANSLATESTRING        .00218 RESIDENT,  &
          WINCPTRANSLATECHAR          .00219 RESIDENT,  &
          WINCOMPARESTRINGS           .00220 RESIDENT,  &
          WINUPPER                    .00221 RESIDENT,  &
          WINUPPERCHAR                .00222 RESIDENT,  &
          WINNEXTCHAR                 .00223 RESIDENT,  &
          WINPREVCHAR                 .00224 RESIDENT,  &
          WINCREATEHEAP               .00225 RESIDENT,  &
          WINDESTROYHEAP              .00226 RESIDENT,  &
          WINAVAILMEM                 .00227 RESIDENT,  &
          WINALLOCMEM                 .00228 RESIDENT,  &
          WINREALLOCMEM               .00229 RESIDENT,  &
          WINFREEMEM                  .00230 RESIDENT,  &
          WINLOCKHEAP                 .00231 RESIDENT,  &
          WINCREATEATOMTABLE          .00233 RESIDENT,  &
          WINDESTROYATOMTABLE         .00234 RESIDENT,  &
          WINADDATOM                  .00235 RESIDENT,  &
          WINFINDATOM                 .00236 RESIDENT,  &
          WINDELETEATOM               .00237 RESIDENT,  &
          WINQUERYATOMUSAGE           .00238 RESIDENT,  &
          WINQUERYATOMLENGTH          .00239 RESIDENT,  &
          WINQUERYATOMNAME            .00240 RESIDENT,  &
          WINQUERYSYSTEMATOMTABLE     .00241 RESIDENT,  &
          WINGETLASTERROR             .00243 RESIDENT,  &
          WINGETERRORINFO             .00244 RESIDENT,  &
          WINFREEERRORINFO            .00245 RESIDENT,  &
          WININITIALIZE               .00246 RESIDENT,  &
          WINTERMINATE                .00247 RESIDENT,  &
          WINCATCH                    .00248 RESIDENT,  &
          WINTHROW                    .00249 RESIDENT,  &
          WINQUERYVERSION             .00250 RESIDENT,  &
          WINISPHYSINPUTENABLED       .00264 RESIDENT,  &
          WINQUERYWINDOWDC            .00265 RESIDENT,  &
          WINDRAWBORDER               .00266 RESIDENT,  &
          WINDRAWTEXT                 .00267 RESIDENT,  &
          WINDRAWBITMAP               .00268 RESIDENT,  &
          WINQUERYWINDOWPTR           .00269 RESIDENT,  &
          WINSETWINDOWPTR             .00270 RESIDENT,  &
          WINMSGSEMWAIT               .00274 RESIDENT,  &
          WINMSGMUXSEMWAIT            .00275 RESIDENT,  &
          WINCANCELSHUTDOWN           .00277 RESIDENT,  &
          WINSETWINDOWBITS            .00278 RESIDENT,  &
          WINGETCLIPPS                .00279 RESIDENT,  &
          WINSAVEWINDOWPOS            .00285 RESIDENT,  &
          WINFOCUSCHANGE              .00286 RESIDENT,  &
          WINQUERYQUEUEINFO           .00287 RESIDENT,  &
          WINSETCLASSMSGINTEREST      .00292 RESIDENT,  &
          WINQUERYDLGITEMTEXTLENGTH   .00294 RESIDENT,  &
          WINDDEINITIATE              .00297 RESIDENT,  &
          WINDDERESPOND               .00298 RESIDENT,  &
          WINDDEPOSTMSG               .00299 RESIDENT,  &
          WINSETPRESPARAM             .00301 RESIDENT,  &
          WINQUERYPRESPARAM           .00302 RESIDENT,  &
          WINREMOVEPRESPARAM          .00303 RESIDENT,  &
          WINBUILDPTRHANDLE           .00310 RESIDENT,  &
          WINQUERYWINDOWMODEL         .00317 RESIDENT,  &
          WINSETDESKTOPBKGND          .00318 RESIDENT,  &
          WINQUERYDESKTOPBKGND        .00319 RESIDENT,  &
          WINPOPUPMENU                .00320 RESIDENT,  &
          WINREALIZEPALETTE           .00321 RESIDENT,  &
          WINDELETELIBRARY            .00602 RESIDENT,  &
          WINLOADPROCEDURE            .00603 RESIDENT,  &
          WINDELETEPROCEDURE          .00604 RESIDENT,  &
          WINSETMSGMODE               .00605 RESIDENT,  &
          WINSETSYNCHROMODE           .00606 RESIDENT,  &
          WINGETDLGMSG                .00607 RESIDENT,  &
          WINREGISTERUSERMSG          .00608 RESIDENT,  &
          WINQUERYANCHORBLOCK         .00609 RESIDENT,  &
          WINREGISTERUSERDATATYPE     .00612 RESIDENT,  &
          WINISWINDOWSHOWING          .00614 RESIDENT,  &
          WINLOADLIBRARY              .00615 RESIDENT,  &
          WINCREATEPOINTERINDIRECT    .00616 RESIDENT,  &
          WINQUERYPOINTERINFO         .00617 RESIDENT,  &
          WINGETERASEPS               .00624 RESIDENT,  &
          WINRELEASEERASEPS           .00625 RESIDENT,  &
          WINSTRETCHPOINTER           .00632 RESIDENT,  &
          WINSETPOINTEROWNER          .00633 RESIDENT,  &
          WINLOCKPOINTERUPDATE        .00639 RESIDENT,  &
          WINCHECKINPUT               .00640 RESIDENT,  &
          WINSETCLASSTHUNKPROC        .00959 RESIDENT,  &
          WINQUERYCLASSTHUNKPROC      .00960 RESIDENT,  &
          WINSETWINDOWTHUNKPROC       .00961 RESIDENT,  &
          WINQUERYWINDOWTHUNKPROC     .00962 RESIDENT,  &
          WINENUMPROPERTIES           .01052 RESIDENT,  &
          WINQUERYPROPERTY            .01053 RESIDENT,  &
          WINREMOVEPROPERTY           .01054 RESIDENT,  &
          WINSETPROPERTY              .01055 RESIDENT,  &
          WINENUMCLASSPROPERTIES      .01056 RESIDENT,  &
          WINQUERYCLASSPROPERTY       .01057 RESIDENT,  &
          WINREMOVECLASSPROPERTY      .01058 RESIDENT,  &
          WINSETCLASSPROPERTY         .01059 RESIDENT,  &
          WINSETLANGINFO              .01500 RESIDENT,  &
          WINQUERYLANGINFO            .01501 RESIDENT,  &
          WINSETKBDLAYER              .01502 RESIDENT,  &
          WINQUERYKBDLAYER            .01503 RESIDENT,  &
          WINSETLANGVIEWER            .01504 RESIDENT,  &
          WINQUERYLANGVIEWER          .01505 RESIDENT,  &
          WINPOUREVENT                .01600 RESIDENT,  &
          WINSETMSGQIMINFO            .01601 RESIDENT,  &
          WINQUERYMSGQIMINFO          .01602 RESIDENT,  &
          OLDWINQUERYPOINTERINFO      .00090,  &
          WINFORMATFRAME              .00142,  &
          WINSYNCSTARTPGM             .00197,  &
          WINSCREENSWITCH             .00198,  &
          WINRESETSYSTEM              .00199,  &
          WINQUERYHUNGPROCESS         .00200,  &
          WINLOADSTRINGTABLE          .00201,  &
          WINLOADVKEYGLYPHXLATETBL    .00202,  &
          WINLOADCHARXLATETBL         .00203,  &
          WINSETVKEYGLYPHXLATETBL     .00204,  &
          WINSETCHARXLATETBL          .00205,  &
          WINQUERYVKEYGLYPHXLATETBL   .00206,  &
          WINQUERYCHARXLATETBL        .00207,  &
          WINVKEYGLYPHCHARXLATE       .00208,  &
          WINVKEYTOSCANID             .00209,  &
          WINQUERYPROCESSCP           .00210,  &
          WINSETLASTERROR             .00242,  &
          FSRSEMENTER                 .00251,  &
          FSRSEMLEAVE                 .00252,  &
          FSRSEMEXIT                  .00253,  &
          FSRSEMCHECK                 .00254,  &
          FSRSEMENTERP                .00255,  &
          AABGETREG                   .00256,  &
          AABSETREG                   .00257,  &
          VALIDATESELECTOR            .00258,  &
          LOADMODULEHANDLE            .00259,  &
          GETSCREENSELECTOR           .00260,  &
          IDENTIFYCODESELECTOR        .00261,  &
          WINQUERYWINDOWSGID          .00262,  &
          _WINSETERRORINFO            .00263,  &
          WIN16INITIALIZESYSTEM       .00271,  &
          WINTERMINATESYSTEM          .00272,  &
          WIN16LOCKSTARTINPUT         .00273,  &
          WINSYSTEMSHUTDOWN           .00276,  &
          WINSENDQUEUEMSG             .00281,  &
          WINSETQUEUEPROC             .00282,  &
          WINDEFQUEUEPROC             .00283,  &
          WINCALLHELPHOOK             .00288,  &
          WINSETATOMTABLEOWNER        .00289,  &
          WINISSHUTTINGDOWN           .00290,  &
          WINTERMINATESHUTDOWN        .00291,  &
          VISREGIONCALLBACK           .00293,  &
          WINQUERYKBDLAYOUT           .00295,  &
          WINSETKBDLAYOUT             .00296,  &
          WIN16CANCELMODE             .00300,  &
          WINTHREADASSOCQUEUE         .00304,  &
          WINQUEUEFROMID              .00305,  &
          WINQUERYSENDMSG             .00306,  &
          WINREPLYMSG                 .00307,  &
          WINWAKETHREAD               .00308,  &
          WINLOCKINPUT                .00309,  &
          DUMWINCREATEHELPINSTANCE    .00311,  &
          DUMWINDESTROYHELPINSTANCE   .00312,  &
          DUMWINASSOCIATEHELPINSTANCE .00313,  &
          DUMWINCREATEHELPTABLE       .00314,  &
          DUMWINLOADHELPTABLE         .00315,  &
          DUMWINQUERYHELPINSTANCE     .00316,  &
          DUMMYHELPENTRY              .00322,  &
          GETPMDDCODESELECTOR         .00325,  &
          WINSEMENTER                 .00326,  &
          WINSEMLEAVE                 .00327,  &
          WINQUERYPWND                .00328,  &
          WINQUERYSTATE               .00329,  &
          WINLOADANIPOINTER           .00330,  &
          WINDESTROYANIPOINTER        .00331,  &
          WINSETANIPOINTER            .00332,  &
          WINENUMVKEYSCANIDS          .00333,  &
          WINSEMQUERY                 .00334,  &
          WINLOADTRANSLATIONTABLE     .00400,  &
          WINSETKBDLAYOUT2            .00401,  &
          WINSEAMLESSBOX              .00495,  &
          WINJIGGLEMOUSE              .00497,  &
          WINTRACKSEAMLESS            .00498,  &
          WINREGISTERSEAMLESSPROC     .00499,  &
          ABNORMALPROCESSEXIT         .00500,  &
          WINPROFCONTROL              .00508,  &
          DEBUGINPUTENABLE            .00509,  &
          DEBUGINPUT                  .00510,  &
          DEBUGOUTPUT                 .00511,  &
          DEBUGCURPOS                 .00512,  &
          DEBUGCLS                    .00513,  &
          WINDEBUGBREAK               .00514,  &
          DEBUGREDIRECT               .00515,  &
          INPUTGETDEVICEINFO          .00590,  &
          STARTTIMER                  .00600,  &
          DESTROYTIMERS               .00601,  &
          WINLANGSUPWINDOWPROC        .00610,  &
          WINLANGSUPDLGPROC           .00611,  &
          FARCALLHOOK                 .00613,  &
          WINSETMENUDATA              .00618,  &
          WINQUERYMENUDATA            .00619,  &
          WINMENUINDEXFROMID          .00620,  &
          WINFORCEMENUREDRAW          .00621,  &
          WINGETSCANSTATE             .00622,  &
          WINSETSCANSTATETABLE        .00623,  &
          WINSETWINDOWDC              .00626,  &
          WINTRANSLATECHAR            .00627,  &
          WINPORTHOLEUTIL             .00628,  &
          WINEXPLODEWINDOW            .00629,  &
          WINSETACCELTABLEVALUE       .00630,  &
          WINQUERYACCELTABLEVALUE     .00631,  &
          WINGETFRAMETREEPPSIZE       .00634,  &
          WINGETFRAMETREEPPS          .00635,  &
          WINSETFRAMETREEPPS          .00636,  &
          WIN16GETMINWINDOWVIEWER     .00637,  &
          WINQUERYWORKPLACEWINDOW     .00638,  &
          FARFLUSHBUFHOOK             .00641,  &
          WinAddAtom                  .00700,  &
          WinAlarm                    .00701,  &
          WinBeginEnumWindows         .00702,  &
          WinBeginPaint               .00703,  &
          WinCalcFrameRect            .00704,  &
          WinCancelShutdown           .00705,  &
          WinCloseClipbrd             .00707,  &
          WinCompareStrings           .00708,  &
          WinCopyAccelTable           .00709,  &
          WinCopyRect                 .00710,  &
          WinCpTranslateChar          .00711,  &
          WinCpTranslateString        .00712,  &
          WinCreateAccelTable         .00713,  &
          WinCreateAtomTable          .00714,  &
          WinCreateCursor             .00715,  &
          WinCreatePointer            .00717,  &
          WinDdeInitiate              .00718,  &
          WinDdePostMsg               .00719,  &
          WinDdeRespond               .00720,  &
          WinDeleteAtom               .00721,  &
          WinDeleteLibrary            .00722,  &
          WinDestroyAccelTable        .00723,  &
          WinDestroyAtomTable         .00724,  &
          WinDestroyCursor            .00725,  &
          WinDestroyPointer           .00727,  &
          WinDismissDlg               .00729,  &
          WinDrawBitmap               .00730,  &
          WinDrawBorder               .00731,  &
          WinDrawPointer              .00732,  &
          WinEmptyClipbrd             .00733,  &
          WinEnablePhysInput          .00734,  &
          WinEnableWindow             .00735,  &
          WinEnableWindowUpdate       .00736,  &
          WinEndEnumWindows           .00737,  &
          WinEndPaint                 .00738,  &
          WinEnumClipbrdFmts          .00739,  &
          WinEnumDlgItem              .00740,  &
          WinEqualRect                .00741,  &
          WinExcludeUpdateRegion      .00742,  &
          WinFillRect                 .00743,  &
          WinFindAtom                 .00744,  &
          WinFlashWindow              .00745,  &
          WinFocusChange              .00746,  &
          WinFreeErrorInfo            .00748,  &
          WinGetClipPS                .00749,  &
          WinGetCurrentTime           .00750,  &
          WinGetErrorInfo             .00751,  &
          WinGetKeyState              .00752,  &
          WinGetLastError             .00753,  &
          WinGetMaxPosition           .00754,  &
          WinGetMinPosition           .00755,  &
          WinGetNextWindow            .00756,  &
          WinGetPS                    .00757,  &
          WinGetPhysKeyState          .00758,  &
          WinGetScreenPS              .00759,  &
          WinGetSysBitmap             .00760,  &
          WinInSendMsg                .00761,  &
          WinInflateRect              .00762,  &
          WinIntersectRect            .00764,  &
          WinInvalidateRect           .00765,  &
          WinInvalidateRegion         .00766,  &
          WinInvertRect               .00767,  &
          WinIsChild                  .00768,  &
          WinIsPhysInputEnabled       .00769,  &
          WinIsRectEmpty              .00770,  &
          WinIsThreadActive           .00771,  &
          WinIsWindow                 .00772,  &
          WinIsWindowEnabled          .00773,  &
          WinIsWindowShowing          .00774,  &
          WinIsWindowVisible          .00775,  &
          WinLoadAccelTable           .00776,  &
          WinLoadLibrary              .00777,  &
          WinLoadMenu                 .00778,  &
          WinLoadMessage              .00779,  &
          WinLoadPointer              .00780,  &
          WinLoadString               .00781,  &
          WinLockVisRegions           .00782,  &
          WinLockWindowUpdate         .00784,  &
          WinMakePoints               .00785,  &
          WinMakeRect                 .00786,  &
          WinMapDlgPoints             .00787,  &
          WinMapWindowPoints          .00788,  &
          WinMessageBox               .00789,  &
          WinMsgSemWait               .00790,  &
          WinNextChar                 .00791,  &
          WinOffsetRect               .00792,  &
          WinOpenClipbrd              .00793,  &
          WinOpenWindowDC             .00794,  &
          WinPrevChar                 .00795,  &
          WinProcessDlg               .00796,  &
          WinPtInRect                 .00797,  &
          WinQueryAccelTable          .00798,  &
          WinQueryActiveWindow        .00799,  &
          WinQueryAnchorBlock         .00800,  &
          WinQueryAtomLength          .00801,  &
          WinQueryAtomName            .00802,  &
          WinQueryAtomUsage           .00803,  &
          WinQueryCapture             .00804,  &
          WinQueryClassName           .00805,  &
          WinQueryClipbrdData         .00806,  &
          WinQueryClipbrdFmtInfo      .00807,  &
          WinQueryClipbrdOwner        .00808,  &
          WinQueryClipbrdViewer       .00809,  &
          WinQueryCp                  .00810,  &
          WinQueryCpList              .00811,  &
          WinQueryCursorInfo          .00812,  &
          WinQueryDesktopWindow       .00813,  &
          WinQueryDlgItemShort        .00814,  &
          WinQueryDlgItemText         .00815,  &
          WinQueryDlgItemTextLength   .00816,  &
          WinQueryFocus               .00817,  &
          WinQueryMsgPos              .00818,  &
          WinQueryMsgTime             .00819,  &
          WinQueryObjectWindow        .00820,  &
          WinQueryPointer             .00821,  &
          WinQueryPointerInfo         .00822,  &
          WinQueryPointerPos          .00823,  &
          WinQueryQueueInfo           .00824,  &
          WinQueryQueueStatus         .00825,  &
          WinQuerySysColor            .00826,  &
          WinQuerySysModalWindow      .00827,  &
          WinQuerySysPointer          .00828,  &
          WinQuerySysValue            .00829,  &
          WinQuerySystemAtomTable     .00830,  &
          WinQueryUpdateRect          .00831,  &
          WinQueryUpdateRegion        .00832,  &
          WinQueryVersion             .00833,  &
          WinQueryWindow              .00834,  &
          WinQueryWindowDC            .00835,  &
          WinQueryWindowPos           .00837,  &
          WinQueryWindowProcess       .00838,  &
          WinQueryWindowPtr           .00839,  &
          WinQueryWindowRect          .00840,  &
          WinQueryWindowText          .00841,  &
          WinQueryWindowTextLength    .00842,  &
          WinQueryWindowULong         .00843,  &
          WinQueryWindowUShort        .00844,  &
          WinRegisterUserDatatype     .00845,  &
          WinRegisterUserMsg          .00846,  &
          WinReleasePS                .00848,  &
          WinScrollWindow             .00849,  &
          WinSetAccelTable            .00850,  &
          WinSetActiveWindow          .00851,  &
          WinSetCapture               .00852,  &
          WinSetClassMsgInterest      .00853,  &
          WinSetClipbrdData           .00854,  &
          WinSetClipbrdOwner          .00855,  &
          WinSetClipbrdViewer         .00856,  &
          WinSetCp                    .00857,  &
          WinSetDlgItemShort          .00858,  &
          WinSetDlgItemText           .00859,  &
          WinSetFocus                 .00860,  &
          WinSetMsgInterest           .00861,  &
          WinSetMsgMode               .00862,  &
          WinSetMultWindowPos         .00863,  &
          WinSetOwner                 .00864,  &
          WinSetParent                .00865,  &
          WinSetPointer               .00866,  &
          WinSetPointerPos            .00867,  &
          WinSetRect                  .00868,  &
          WinSetRectEmpty             .00869,  &
          WinSetSynchroMode           .00870,  &
          WinSetSysColors             .00871,  &
          WinSetSysModalWindow        .00872,  &
          WinSetSysValue              .00873,  &
          WinSetWindowBits            .00874,  &
          WinSetWindowPos             .00875,  &
          WinSetWindowPtr             .00876,  &
          WinSetWindowText            .00877,  &
          WinSetWindowULong           .00878,  &
          WinSetWindowUShort          .00879,  &
          WinShowCursor               .00880,  &
          WinShowPointer              .00881,  &
          WinShowTrackRect            .00882,  &
          WinShowWindow               .00883,  &
          WinStartTimer               .00884,  &
          WinStopTimer                .00885,  &
          WinSubstituteStrings        .00886,  &
          WinSubtractRect             .00887,  &
          WinTrackRect                .00890,  &
          WinUnionRect                .00891,  &
          WinUpdateWindow             .00892,  &
          WinUpper                    .00893,  &
          WinUpperChar                .00894,  &
          WinValidateRect             .00895,  &
          WinValidateRegion           .00896,  &
          WinWaitMsg                  .00897,  &
          WinWindowFromDC             .00898,  &
          WinWindowFromID             .00899,  &
          WinWindowFromPoint          .00900,  &
          WinBroadcastMsg             .00901,  &
          WinPostQueueMsg             .00902,  &
          WinSendDlgItemMsg           .00903,  &
          WinTranslateAccel           .00904,  &
          WinCallMsgFilter            .00905,  &
          WinCreateFrameControls      .00906,  &
          WinCreateMenu               .00907,  &
          WinCreateWindow             .00909,  &
          WinDefDlgProc               .00910,  &
          WinDrawText                 .00913,  &
          WinGetDlgMsg                .00914,  &
          WinMsgMuxSemWait            .00916,  &
          WinMultWindowFromIDs        .00917,  &
          WinPeekMsg                  .00918,  &
          WinPostMsg                  .00919,  &
          WinSetKeyboardStateTable    .00921,  &
          WinCreateDlg                .00922,  &
          WinDlgBox                   .00923,  &
          WinLoadDlg                  .00924,  &
          WinQueryClassInfo           .00925,  &
          WinReleaseHook              .00927,  &
          WinSetHook                  .00928,  &
          WinSubclassWindow           .00929,  &
          WinSetClassThunkProc        .00930,  &
          WinQueryClassThunkProc      .00931,  &
          WinSetWindowThunkProc       .00932,  &
          WinQueryWindowThunkProc     .00933,  &
          WinQueryWindowModel         .00934,  &
          WinSetDesktopBkgnd          .00935,  &
          WinQueryDesktopBkgnd        .00936,  &
          WinPopupMenu                .00937,  &
          WinSetPresParam             .00938,  &
          WinQueryPresParam           .00939,  &
          WinRemovePresParam          .00940,  &
          WinRealizePalette           .00941,  &
          WinCreatePointerIndirect    .00942,  &
          WinSaveWindowPos            .00943,  &
          WINCREATEFARHEAP            .00947,  &
          WINDESTROYFARHEAP           .00948,  &
          WINALLOCFARMEM              .00949,  &
          WINFREEFARMEM               .00950,  &
          WINREALLOCFARMEM            .00951,  &
          WinGetErasePS               .00952,  &
          WinReleaseErasePS           .00953,  &
          DEBUG32INPUT                .00954,  &
          DEBUG32OUTPUT               .00955,  &
          DEBUG32CURPOS               .00956,  &
          DEBUG32CLS                  .00957,  &
          Win32DebugBreak             .00958,  &
          WinSystemShutdown           .00963,  &
          WinBuildPtrHandle           .00964,  &
          WinExplodeWindow            .00965,  &
          WinSetAccelTableValue       .00966,  &
          WinQueryAccelTableValue     .00967,  &
          WinStretchPointer           .00968,  &
          AAB32GETREG                 .00969,  &
          AAB32SETREG                 .00970,  &
          WinSetPointerOwner          .00971,  &
          WinGetFrameTreePPSize       .00972,  &
          WinGetFrameTreePPs          .00973,  &
          WinSetFrameTreePPs          .00974,  &
          WinQueryProcessCP           .00975,  &
          WinVisRegionNotify          .00976,  &
          WinSetErrorInfo             .00977,  &
          WinWaitEventSem             .00978,  &
          WinRequestMutexSem          .00979,  &
          WinWaitMuxWaitSem           .00980,  &
          WINSETSYSCLIPBRDVIEWER      .00981,  &
          WinSetSysClipbrdViewer      .00982,  &
          WinSemQuery                 .00983,  &
          WinSemEnter                 .00984,  &
          WinSemLeave                 .00985,  &
          WinLoadProcedure            .00986,  &
          WinDeleteProcedure          .00987,  &
          WinMoveCursor               .00988,  &
          WinQuerySendMsg             .00990,  &
          WinReplyMsg                 .00991,  &
          WinWakeThread               .00992,  &
          WinQueueFromID              .00993,  &
          WinThreadAssocQueue         .00994,  &
          WinLockInput                .00995,  &
          WinSetSysFont               .00996,  &
          WinLockPointerUpdate        .00997,  &
          WinCheckInput               .00998,  &
          Far32LockupHook             .00999,  &
          WinQueryVisibleRegion       .01000,  &
          WinSetVisibleRegionNotify   .01001,  &
          WinSetSysPointerData        .01002,  &
          WinQuerySysPointerData      .01003,  &
          WinSeamlessBox              .01004,  &
          WinJiggleMouse              .01005,  &
          WinTrackSeamless            .01006,  &
          WinRegisterSeamlessProc     .01007,  &
          WinSetCharXlateTbl          .01008,  &
          WinMspInit                  .01009,  &
          WinMspAlloc                 .01010,  &
          WinMspFree                  .01011,  &
          ThkDos16SetFgnd             .01012,  &
          InputGetDeviceInfo          .01013,  &
          WinRegisterPen              .01014,  &
          WinMessageBox2              .01015,  &
          fPenAvail                   .01026,  &
          WinDrawTabbedText           .01027,  &
          PMREQUESTMUTEXSEM           .01028,  &
          PMRELEASEMUTEXSEM           .01029,  &
          WinQueryPointerClipRect     .01030,  &
          WinSetPointerClipRect       .01031,  &
          WinWaitMsgTimeout           .01033,  &
          WinIsWindowOfClass          .01034,  &
          WinSetAtomTableOwner        .01035,  &
          WinXxCreateHeap             .01036,  &
          WinXxDestroyHeap            .01037,  &
          WinXxAllocMem               .01038,  &
          WinXxFreeMem                .01039,  &
          WinXxAllocFill              .01040,  &
          WinXxReallocMem             .01041,  &
          WinXxQueryMemSize           .01042,  &
          WinSetClipRegion            .01043,  &
          WinQueryClipRegion          .01044,  &
          Win32AddClipbrdViewer       .01045,  &
          Win32RemoveClipbrdViewer    .01046,  &
          Win32QueryClipbrdViewerChain .01047,  &
          Win32IncrementAtomUsage     .01048,  &
          DaxQueryOpenClipbrdWindow   .01049,  &
          WinSetDaxData               .01050,  &
          WinQueryDaxData             .01051,  &
          Win32SetKbdLayout           .01060,  &
          Win32GetAFCValue            .01061,  &
          Win32SetAFCValue            .01062,  &
          WinLoadVkeyGlyphXlatetbl    .01063,  &
          WinSetVKeyGlyphXlatetbl     .01064,  &
          WinQueryVKeyGlyphXlatetbl   .01065,  &
          WinTrackWindow              .01066,  &
          WinSemEnterTimeout          .01067,  &
          WinSetDesktopWorkArea       .01068,  &
          WinQueryDesktopWorkArea     .01069,  &
          Win32TranslateChar2         .01070,  &
          Win32WaitForIdleThread      .01071,  &
          WinXXCreateHeap2            .01072,  &
          WinQuerySharedHeapData      .01073,  &
          Win32QueryActiveOtherWindow .01074,  &
          Win32EnumVkeyScanIDs        .01075,  &
#
          DaxCreateCaret              .10000,  &
          DaxDestroyCaret             .10001,  &
          DaxGetCaretBlinkTime        .10002,  &
          DaxHideCaret                .10004,  &
          DaxSetCaretBlinkTime        .10005,  &
          DaxShowCaret                .10007,  &
          DaxShowCursor               .10018,  &
          DaxSwapMouseButton          .10019,  &
          LookupAtom                  .10020,  &
          DaxOpenClipbrd              .10021

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
