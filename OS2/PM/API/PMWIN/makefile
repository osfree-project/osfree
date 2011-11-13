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
IMPORTS = WinInitialize      FMMERGE.5201,  &
          WinTerminate       FMMERGE.5396,  &
          WinCreateMsgQueue  FMMERGE.5136,  &
          WinDestroyMsgQueue FMMERGE.5154,  &
          WinDispatchMsg     FMMERGE.5158,  &
          WinGetMsg          FMMERGE.5192,  &
          WinSendMsg         FMMERGE.5334,  &
          WinRegisterClass   FMMERGE.5315,  &
          WinCreateStdWindow FMMERGE.5139,  &
          WinDefWindowProc   FMMERGE.5147,  &
          WinDestroyWindow   FMMERGE.5156,  &
          WINQUERYDESKTOPWINDOW     FMMERGE.3047,  &
          WINQUERYOBJECTWINDOW      FMMERGE.3193,  &
          WINREGISTERCLASS          FMMERGE.3058,  &
          WINQUERYCLASSNAME         FMMERGE.3178,  &
          WINQUERYCLASSINFO         FMMERGE.2065,  &
          WINCREATEWINDOW           FMMERGE.3022,  &
          WINDESTROYWINDOW          FMMERGE.3024,  &
          WINSETWINDOWPOS           FMMERGE.3072,  &
          WINSETMULTWINDOWPOS       FMMERGE.3066,  &
          WINQUERYWINDOWPOS         FMMERGE.3053,  &
          WINENABLEWINDOW           FMMERGE.3113,  &
          WINISWINDOWENABLED        FMMERGE.3154,  &
          WINSHOWWINDOW             FMMERGE.3074,  &
          WINENABLEWINDOWUPDATE     FMMERGE.3114,  &
          WINISWINDOWVISIBLE        FMMERGE.3036,  &
          WINSETWINDOWTEXT          FMMERGE.3073,  &
          WINQUERYWINDOWTEXT        FMMERGE.3216,  &
          WINQUERYWINDOWTEXTLENGTH  FMMERGE.3217,  &
          WINWINDOWFROMID           FMMERGE.3078,  &
          WINMULTWINDOWFROMIDS      FMMERGE.2066,  &
          WINISWINDOW               FMMERGE.3035,  &
          WINISCHILD                FMMERGE.3034,  &
          WINSETPARENT              FMMERGE.3067,  &
          WINSETOWNER               FMMERGE.3262,  &
          WINQUERYWINDOW            FMMERGE.3052,  &
          WINQUERYWINDOWRECT        FMMERGE.3055,  &
          WINQUERYWINDOWPROCESS     FMMERGE.3054,  &
          WINSETWINDOWUSHORT        FMMERGE.3277,  &
          WINQUERYWINDOWUSHORT      FMMERGE.3057,  &
          WINSETWINDOWULONG         FMMERGE.3276,  &
          WINQUERYWINDOWULONG       FMMERGE.3056,  &
          WINBEGINENUMWINDOWS       FMMERGE.3015,  &
          WINGETNEXTWINDOW          FMMERGE.3031,  &
          WINENDENUMWINDOWS         FMMERGE.3115,  &
          WINWINDOWFROMPOINT        FMMERGE.3308,  &
          WINMAPWINDOWPOINTS        FMMERGE.3166,  &
          WINSUBCLASSWINDOW         FMMERGE.3075,  &
          WINLOCKWINDOW             FMMERGE.2067,  &
          WINQUERYWINDOWLOCKCOUNT   FMMERGE.2068,  &
          WINREGISTERWINDOWDESTROY  FMMERGE.3225,  &
          WINOPENWINDOWDC           FMMERGE.3041,  &
          WINGETSCREENPS            FMMERGE.3139,  &
          WINGETPS                  FMMERGE.3137,  &
          WINRELEASEPS              FMMERGE.3228,  &
          WINBEGINPAINT             FMMERGE.3016,  &
          WINENDPAINT               FMMERGE.3027,  &
          WININVALIDATERECT         FMMERGE.3033,  &
          WININVALIDATEREGION       FMMERGE.3149,  &
          WINVALIDATERECT           FMMERGE.3302,  &
          WINVALIDATEREGION         FMMERGE.3303,  &
          WINQUERYUPDATERECT        FMMERGE.3210,  &
          WINQUERYUPDATEREGION      FMMERGE.3211,  &
          WINUPDATEWINDOW           FMMERGE.3301,  &
          WINEXCLUDEUPDATEREGION    FMMERGE.3122,  &
          WINLOCKWINDOWUPDATE       FMMERGE.3164,  &
          WINLOCKVISREGIONS         FMMERGE.3163,  &
          WINWINDOWFROMDC           FMMERGE.3307,  &
          WINCREATEMSGQUEUE         FMMERGE.3020,  &
          WINDESTROYMSGQUEUE        FMMERGE.3023,  &
          WINQUERYQUEUESTATUS       FMMERGE.3200,  &
          WINSENDMSG                FMMERGE.3062,  &
          WINBROADCASTMSG          FMMERGE.3017,  &
          WININSENDMSG             FMMERGE.3146,  &
          WINGETMSG                FMMERGE.3030,  &
          WINPEEKMSG               FMMERGE.3042,  &
          WINWAITMSG               FMMERGE.3305,  &
          WINDISPATCHMSG           FMMERGE.3025,  &
          WINPOSTMSG               FMMERGE.3043,  &
          WINPOSTQUEUEMSG          FMMERGE.3173,  &
          WINQUERYMSGPOS           FMMERGE.3191,  &
          WINQUERYMSGTIME          FMMERGE.3192,  &
          WINCALLMSGFILTER         FMMERGE.3084,  &
          WINSETMSGINTEREST        FMMERGE.3260,  &
          WINSETCAPTURE            FMMERGE.3245,  &
          WINQUERYCAPTURE          FMMERGE.3176,  &
          WINSETFOCUS              FMMERGE.3065,  &
          WINQUERYFOCUS            FMMERGE.3048,  &
          WINSETACTIVEWINDOW       FMMERGE.3063,  &
          WINQUERYACTIVEWINDOW     FMMERGE.3045,  &
          WINSETSYSMODALWINDOW     FMMERGE.3069,  &
          WINQUERYSYSMODALWINDOW   FMMERGE.3050,  &
          WINISTHREADACTIVE        FMMERGE.3153,  &
          WINSTARTTIMER            FMMERGE.3282,  &
          WINSTOPTIMER             FMMERGE.3284,  &
          WINGETCURRENTTIME        FMMERGE.3129,  &
          WINLOADPOINTER           FMMERGE.2069,  &
          WINCREATEPOINTER         FMMERGE.2070,  &
          WINDESTROYPOINTER        FMMERGE.2071,  &
          OLDWINQUERYPOINTERINFO   FMMERGE.2072,  &
          WINQUERYSYSPOINTER       FMMERGE.2073,  &
          WINSETPOINTER            FMMERGE.3263,  &
          WINQUERYPOINTER          FMMERGE.3194,  &
          WINSHOWPOINTER           FMMERGE.3279,  &
          WINSETPOINTERPOS         FMMERGE.3265,  &
          WINQUERYPOINTERPOS       FMMERGE.3195,  &
          WINGETSYSBITMAP          FMMERGE.2074,  &
          WINCREATECURSOR          FMMERGE.3091,  &
          WINDESTROYCURSOR         FMMERGE.3104,  &
          WINSHOWCURSOR            FMMERGE.3278,  &
          WINQUERYCURSORINFO       FMMERGE.2075,  &
          WINLOADACCELTABLE        FMMERGE.2076,  &
          WINCREATEACCELTABLE      FMMERGE.2077,  &
          WINDESTROYACCELTABLE     FMMERGE.2078,  &
          WINTRANSLATEACCEL        FMMERGE.3298,  &
          WINSETACCELTABLE         FMMERGE.3241,  &
          WINQUERYACCELTABLE       FMMERGE.3174,  &
          WINCOPYACCELTABLE        FMMERGE.2079,  &
          WINSETHOOK               FMMERGE.3257,  &
          WINRELEASEHOOK           FMMERGE.3227,  &
          WINOPENCLIPBRD           FMMERGE.3170,  &
          WINCLOSECLIPBRD          FMMERGE.3089,  &
          WINEMPTYCLIPBRD          FMMERGE.3111,  &
          WINSETCLIPBRDOWNER       FMMERGE.3249,  &
          WINQUERYCLIPBRDOWNER     FMMERGE.3181,  &
          WINSETCLIPBRDDATA        FMMERGE.2080,  &
          WINQUERYCLIPBRDDATA      FMMERGE.3180,  &
          WINENUMCLIPBRDFMTS       FMMERGE.3119,  &
          WINQUERYCLIPBRDFMTINFO   FMMERGE.2081,  &
          WINSETCLIPBRDVIEWER      FMMERGE.3250,  &
          WINQUERYCLIPBRDVIEWER    FMMERGE.3182,  &
          WINLOADDLG               FMMERGE.3037,  &
          WINCREATEDLG             FMMERGE.3092,  &
          WINPROCESSDLG            FMMERGE.3044,  &
          WINDLGBOX                FMMERGE.3107,  &
          WINDISMISSDLG            FMMERGE.3106,  &
          WINSENDDLGITEMMSG        FMMERGE.3061,  &
          WINSETDLGITEMSHORT       FMMERGE.3252,  &
          WINQUERYDLGITEMSHORT     FMMERGE.3185,  &
          WINMAPDLGPOINTS          FMMERGE.3165,  &
          WINSUBSTITUTESTRINGS     FMMERGE.3287,  &
          WINENUMDLGITEM           FMMERGE.3120,  &
          WINSETDLGITEMTEXT        FMMERGE.3064,  &
          WINQUERYDLGITEMTEXT      FMMERGE.3186,  &
          WINLOADMENU              FMMERGE.3159,  &
          WINCREATEMENU            FMMERGE.3095,  &
          WINALARM                 FMMERGE.3014,  &
          WINMESSAGEBOX            FMMERGE.3039,  &
          WINCREATESTDWINDOW       FMMERGE.3021,  &
          WINCREATEFRAMECONTROLS   FMMERGE.3093,  &
          WINFORMATFRAME           FMMERGE.3127,  &
          WINCALCFRAMERECT         FMMERGE.3082,  &
          WINFLASHWINDOW           FMMERGE.3124,  &
          WINGETMINPOSITION        FMMERGE.3135,  &
          WINGETMAXPOSITION        FMMERGE.3134,  &
          WINQUERYSYSVALUE         FMMERGE.3051,  &
          WINSETSYSVALUE           FMMERGE.3070,  &
          WINSETSYSCOLORS          FMMERGE.3270,  &
          WINQUERYSYSCOLOR         FMMERGE.3207,  &
          WINSCROLLWINDOW          FMMERGE.3235,  &
          WINTRACKRECT             FMMERGE.3077,  &
          WINSHOWTRACKRECT         FMMERGE.3280,  &
          GPILOADBITMAP            FMMERGE.2082,  &
          WINLOADSTRING            FMMERGE.2083,  &
          WINLOADMESSAGE           FMMERGE.2084,  &
          WINSETRECTEMPTY          FMMERGE.2085,  &
          WINSETRECT               FMMERGE.2086,  &
          WINCOPYRECT              FMMERGE.2087,  &
          WINISRECTEMPTY           FMMERGE.2088,  &
          WINEQUALRECT             FMMERGE.3121,  &
          WINPTINRECT              FMMERGE.2089,  &
          WINOFFSETRECT            FMMERGE.2090,  &
          WININFLATERECT           FMMERGE.2091,  &
          WININTERSECTRECT         FMMERGE.3148,  &
          WINUNIONRECT             FMMERGE.3300,  &
          WINSUBTRACTRECT          FMMERGE.3288,  &
          WINMAKERECT              FMMERGE.2092,  &
          WINMAKEPOINTS            FMMERGE.2093,  &
          WININVERTRECT            FMMERGE.3150,  &
          WINFILLRECT              FMMERGE.3028,  &
          KBDPACKET                FMMERGE.6000,  &
          WINDRAWPOINTER           FMMERGE.3110,  &
          WINDEFWINDOWPROC         FMMERGE.2094,  &
          WINDEFDLGPROC            FMMERGE.2095,  &
          WINSYNCSTARTPGM          FMMERGE.3293,  &
          WINSCREENSWITCH          FMMERGE.3060,  &
          WINRESETSYSTEM           FMMERGE.3059,  &
          WINQUERYHUNGPROCESS      FMMERGE.2096,  &
          WINLOADSTRINGTABLE       FMMERGE.2097,  &
          WINLOADVKEYGLYPHXLATETBL FMMERGE.2098,  &
          WINLOADCHARXLATETBL      FMMERGE.2099,  &
          WINSETVKEYGLYPHXLATETBL  FMMERGE.2100,  &
          WINSETCHARXLATETBL       FMMERGE.3246,  &
          WINQUERYVKEYGLYPHXLATETBL FMMERGE.2101,  &
          WINQUERYCHARXLATETBL     FMMERGE.3177,  &
          WINVKEYGLYPHCHARXLATE    FMMERGE.3304,  &
          WINVKEYTOSCANID          FMMERGE.2102,  &
          WINQUERYPROCESSCP        FMMERGE.2103,  &
          WINGETKEYSTATE           FMMERGE.2104,  &
          WINGETPHYSKEYSTATE       FMMERGE.3136,  &
          WINSETKEYBOARDSTATETABLE FMMERGE.3258,  &
          WINENABLEPHYSINPUT       FMMERGE.3112,  &
          WINSETCP                 FMMERGE.2105,  &
          WINQUERYCP               FMMERGE.2106,  &
          WINQUERYCPLIST           FMMERGE.2107,  &
          WINCPTRANSLATESTRING     FMMERGE.2108,  &
          WINCPTRANSLATECHAR       FMMERGE.2109,  &
          WINCOMPARESTRINGS        FMMERGE.2110,  &
          WINUPPER                 FMMERGE.2111,  &
          WINUPPERCHAR             FMMERGE.2112,  &
          WINNEXTCHAR              FMMERGE.2113,  &
          WINPREVCHAR              FMMERGE.2114,  &
          WINCREATEHEAP            FMMERGE.2115,  &
          WINDESTROYHEAP           FMMERGE.2116,  &
          WINAVAILMEM              FMMERGE.2117,  &
          WINALLOCMEM              FMMERGE.2118,  &
          WINREALLOCMEM            FMMERGE.2119,  &
          WINFREEMEM               FMMERGE.2120,  &
          WINLOCKHEAP              FMMERGE.2121,  &
          WINCREATEATOMTABLE       FMMERGE.2122,  &
          WINDESTROYATOMTABLE      FMMERGE.2123,  &
          WINADDATOM               FMMERGE.2124,  &
          WINFINDATOM              FMMERGE.2125,  &
          WINDELETEATOM            FMMERGE.2126,  &
          WINQUERYATOMUSAGE        FMMERGE.2127,  &
          WINQUERYATOMLENGTH       FMMERGE.2128,  &
          WINQUERYATOMNAME         FMMERGE.2129,  &
          WINQUERYSYSTEMATOMTABLE  FMMERGE.2130,  &
          WINSETLASTERROR          FMMERGE.2131,  &
          WINGETLASTERROR          FMMERGE.2132,  &
          WINGETERRORINFO          FMMERGE.2133,  &
          WINFREEERRORINFO         FMMERGE.2134,  &
          WININITIALIZE            FMMERGE.2135,  &
          WINTERMINATE             FMMERGE.2136,  &
          WINCATCH                 FMMERGE.2137,  &
          WINTHROW                 FMMERGE.2138,  &
          WINQUERYVERSION          FMMERGE.2139,  &
          FSRSEMENTER              FMMERGE.4011,  &
          FSRSEMLEAVE              FMMERGE.4013,  &
          FSRSEMEXIT               FMMERGE.2140,  &
          FSRSEMCHECK              FMMERGE.2141,  &
          FSRSEMENTERP             FMMERGE.4012,  &
          AABGETREG                FMMERGE.2142,  &
          AABSETREG                FMMERGE.2143,  &
          VALIDATESELECTOR         FMMERGE.2144,  &
          LOADMODULEHANDLE         FMMERGE.2145,  &
          GETSCREENSELECTOR        FMMERGE.2146,  &
          IDENTIFYCODESELECTOR     FMMERGE.2147,  &
          WINQUERYWINDOWSGID       FMMERGE.3215,  &
          _WINSETERRORINFO         FMMERGE.2148,  &
          WINISPHYSINPUTENABLED    FMMERGE.3151,  &
          WINQUERYWINDOWDC         FMMERGE.3212,  &
          WINDRAWBORDER            FMMERGE.3109,  &
          WINDRAWTEXT              FMMERGE.3026,  &
          WINDRAWBITMAP            FMMERGE.3108,  &
          WINQUERYWINDOWPTR        FMMERGE.3214,  &
          WINSETWINDOWPTR          FMMERGE.3274,  &
          WIN16INITIALIZESYSTEM    FMMERGE.3032,  &
          WINTERMINATESYSTEM       FMMERGE.2149,  &
          WIN16LOCKSTARTINPUT      FMMERGE.3038,  &
          WINMSGSEMWAIT            FMMERGE.3040,  &
          WINMSGMUXSEMWAIT         FMMERGE.3168,  &
          WINSYSTEMSHUTDOWN        FMMERGE.3294,  &
          WINCANCELSHUTDOWN        FMMERGE.3019,  &
          WINSETWINDOWBITS         FMMERGE.3071,  &
          WINGETCLIPPS             FMMERGE.3128,  &
          WINSENDQUEUEMSG          FMMERGE.3240,  &
          WINSETQUEUEPROC          FMMERGE.2183,  &
          WINDEFQUEUEPROC          FMMERGE.2150,  &
          WINSAVEWINDOWPOS         FMMERGE.3234,  &
          WINFOCUSCHANGE           FMMERGE.3125,  &
          WINQUERYQUEUEINFO        FMMERGE.3049,  &
          WINCALLHELPHOOK          FMMERGE.3083,  &
          WINSETATOMTABLEOWNER     FMMERGE.3244,  &
          WINISSHUTTINGDOWN        FMMERGE.3152,  &
          WINTERMINATESHUTDOWN     FMMERGE.3076,  &
          WINSETCLASSMSGINTEREST   FMMERGE.3247,  &
          VISREGIONCALLBACK        FMMERGE.2151,  &
          WINQUERYDLGITEMTEXTLENGTH FMMERGE.3187,  &
          WINQUERYKBDLAYOUT        FMMERGE.2152,  &
          WINSETKBDLAYOUT          FMMERGE.2153,  &
          WINDDEINITIATE           FMMERGE.3097,  &
          WINDDERESPOND            FMMERGE.3099,  &
          WINDDEPOSTMSG            FMMERGE.3098,  &
          WIN16CANCELMODE          FMMERGE.3018,  &
          WINSETPRESPARAM          FMMERGE.3068,  &
          WINQUERYPRESPARAM        FMMERGE.3196,  &
          WINREMOVEPRESPARAM       FMMERGE.3230,  &
          WINTHREADASSOCQUEUE      FMMERGE.3296,  &
          WINQUEUEFROMID           FMMERGE.3220,  &
          WINQUERYSENDMSG          FMMERGE.3201,  &
          WINREPLYMSG              FMMERGE.3233,  &
          WINWAKETHREAD            FMMERGE.3306,  &
          WINLOCKINPUT             FMMERGE.3161,  &
          WINBUILDPTRHANDLE        FMMERGE.2154,  &
          DUMWINCREATEHELPINSTANCE     FMMERGE.2155,  &
          DUMWINDESTROYHELPINSTANCE    FMMERGE.2156,  &
          DUMWINASSOCIATEHELPINSTANCE  FMMERGE.2157,  &
          DUMWINCREATEHELPTABLE        FMMERGE.2158,  &
          DUMWINLOADHELPTABLE          FMMERGE.2159,  &
          DUMWINQUERYHELPINSTANCE      FMMERGE.2160,  &
          WINQUERYWINDOWMODEL          FMMERGE.3213,  &
          WINSETDESKTOPBKGND           FMMERGE.3251,  &
          WINQUERYDESKTOPBKGND         FMMERGE.3184,  &
          WINPOPUPMENU                 FMMERGE.3172,  &
          WINREALIZEPALETTE            FMMERGE.3221,  &
          DUMMYHELPENTRY               FMMERGE.2161,  &
          GETPMDDCODESELECTOR          FMMERGE.2162,  &
          WINSEMENTER                  FMMERGE.3237,  &
          WINSEMLEAVE                  FMMERGE.3238,  &
          WINQUERYPWND                 FMMERGE.2163,  &
          WINQUERYSTATE                FMMERGE.3203,  &
          WINLOADANIPOINTER            FMMERGE.3157,  &
          WINDESTROYANIPOINTER         FMMERGE.3103,  &
          WINSETANIPOINTER             FMMERGE.3243,  &
          WINENUMVKEYSCANIDS           FMMERGE.2164,  &
          WINSEMQUERY                  FMMERGE.3239,  &
          WINLOADTRANSLATIONTABLE      FMMERGE.2165,  &
          WINSETKBDLAYOUT2             FMMERGE.2166,  &
          WINSEAMLESSBOX               FMMERGE.3236,  &
          WINJIGGLEMOUSE               FMMERGE.3156,  &
          WINTRACKSEAMLESS             FMMERGE.3297,  &
          WINREGISTERSEAMLESSPROC      FMMERGE.3222,  &
          ABNORMALPROCESSEXIT          FMMERGE.5002,  &
          WINPROFCONTROL               FMMERGE.5422,  &
          DEBUGINPUTENABLE             FMMERGE.2167,  &
          DEBUGINPUT                   FMMERGE.2168,  &
          DEBUGOUTPUT                  FMMERGE.2169,  &
          DEBUGCURPOS                  FMMERGE.2170,  &
          DEBUGCLS                     FMMERGE.2171,  &
          WINDEBUGBREAK                FMMERGE.3100,  &
          DEBUGREDIRECT                FMMERGE.2172,  &
          INPUTGETDEVICEINFO           FMMERGE.3002,  &
          STARTTIMER                   FMMERGE.5107,  &
          DESTROYTIMERS                FMMERGE.5007,  &
          WINDELETELIBRARY             FMMERGE.3101,  &
          WINLOADPROCEDURE             FMMERGE.3160,  &
          WINDELETEPROCEDURE           FMMERGE.3102,  &
          WINSETMSGMODE                FMMERGE.3261,  &
          WINSETSYNCHROMODE            FMMERGE.3268,  &
          WINGETDLGMSG                 FMMERGE.3130,  &
          WINREGISTERUSERMSG           FMMERGE.3224,  &
          WINQUERYANCHORBLOCK          FMMERGE.3046,  &
          WINLANGSUPWINDOWPROC         FMMERGE.2173,  &
          WINLANGSUPDLGPROC            FMMERGE.2174,  &
          WINREGISTERUSERDATATYPE      FMMERGE.3223,  &
          FARCALLHOOK                  FMMERGE.5045,  &
          WINISWINDOWSHOWING           FMMERGE.3155,  &
          WINLOADLIBRARY               FMMERGE.3158,  &
          WINCREATEPOINTERINDIRECT     FMMERGE.2175,  &
          WINQUERYPOINTERINFO          FMMERGE.2176,  &
          WINSETMENUDATA               FMMERGE.3259,  &
          WINQUERYMENUDATA             FMMERGE.3190,  &
          WINMENUINDEXFROMID           FMMERGE.3167,  &
          WINFORCEMENUREDRAW           FMMERGE.3126,  &
          WINGETSCANSTATE              FMMERGE.3138,  &
          WINSETSCANSTATETABLE         FMMERGE.3266,  &
          WINGETERASEPS                FMMERGE.3131,  &
          WINRELEASEERASEPS            FMMERGE.3226,  &
          WINSETWINDOWDC               FMMERGE.3273,  &
          WINTRANSLATECHAR             FMMERGE.3299,  &
          WINPORTHOLEUTIL              FMMERGE.2177,  &
          WINEXPLODEWINDOW             FMMERGE.3123,  &
          WINSETACCELTABLEVALUE        FMMERGE.3242,  &
          WINQUERYACCELTABLEVALUE      FMMERGE.3175,  &
          WINSTRETCHPOINTER            FMMERGE.3286,  &
          WINSETPOINTEROWNER           FMMERGE.3264,  &
          WINGETFRAMETREEPPSIZE        FMMERGE.3133,  &
          WINGETFRAMETREEPPS           FMMERGE.3132,  &
          WINSETFRAMETREEPPS           FMMERGE.3256,  &
          WIN16GETMINWINDOWVIEWER      FMMERGE.3029,  &
          WINQUERYWORKPLACEWINDOW      FMMERGE.3219,  &
          WINLOCKPOINTERUPDATE         FMMERGE.3162,  &
          WINCHECKINPUT                FMMERGE.3088,  &
          FARFLUSHBUFHOOK              FMMERGE.3000,  &
          WinAddAtom                   FMMERGE.5111,  &
          WinAlarm                     FMMERGE.5113,  &
          WinBeginEnumWindows          FMMERGE.5114,  &
          WinBeginPaint                FMMERGE.5115,  &
          WinCalcFrameRect             FMMERGE.5118,  &
          WinCancelShutdown            FMMERGE.5121,  &
          WinCloseClipbrd              FMMERGE.5124,  &
          WinCompareStrings            FMMERGE.5125,  &
          WinCopyAccelTable            FMMERGE.5126,  &
          WinCopyRect                  FMMERGE.5127,  &
          WinCpTranslateChar           FMMERGE.5128,  &
          WinCpTranslateString         FMMERGE.5129,  &
          WinCreateAccelTable          FMMERGE.5130,  &
          WinCreateAtomTable           FMMERGE.5131,  &
          WinCreateCursor              FMMERGE.5132,  &
          WinCreatePointer             FMMERGE.5137,  &
          WinDdeInitiate               FMMERGE.5142,  &
          WinDdePostMsg                FMMERGE.5143,  &
          WinDdeRespond                FMMERGE.5144,  &
          WinDeleteAtom                FMMERGE.5148,  &
          WinDeleteLibrary             FMMERGE.5149,  &
          WinDestroyAccelTable         FMMERGE.5151,  &
          WinDestroyAtomTable          FMMERGE.5152,  &
          WinDestroyCursor             FMMERGE.5153,  &
          WinDestroyPointer            FMMERGE.5155,  &
          WinDismissDlg                FMMERGE.5157,  &
          WinDrawBitmap                FMMERGE.5160,  &
          WinDrawBorder                FMMERGE.5161,  &
          WinDrawPointer               FMMERGE.5162,  &
          WinEmptyClipbrd              FMMERGE.5164,  &
          WinEnablePhysInput           FMMERGE.5165,  &
          WinEnableWindow              FMMERGE.5166,  &
          WinEnableWindowUpdate        FMMERGE.5167,  &
          WinEndEnumWindows            FMMERGE.5168,  &
          WinEndPaint                  FMMERGE.5169,  &
          WinEnumClipbrdFmts           FMMERGE.5171,  &
          WinEnumDlgItem               FMMERGE.5172,  &
          WinEqualRect                 FMMERGE.5173,  &
          WinExcludeUpdateRegion       FMMERGE.5174,  &
          WinFillRect                  FMMERGE.5176,  &
          WinFindAtom                  FMMERGE.5177,  &
          WinFlashWindow               FMMERGE.5178,  &
          WinFocusChange               FMMERGE.5179,  &
          WinFreeErrorInfo             FMMERGE.5180,  &
          WinGetClipPS                 FMMERGE.5181,  &
          WinGetCurrentTime            FMMERGE.5182,  &
          WinGetErrorInfo              FMMERGE.5185,  &
          WinGetKeyState               FMMERGE.5188,  &
          WinGetLastError              FMMERGE.5189,  &
          WinGetMaxPosition            FMMERGE.5190,  &
          WinGetMinPosition            FMMERGE.5191,  &
          WinGetNextWindow             FMMERGE.5193,  &
          WinGetPS                     FMMERGE.5195,  &
          WinGetPhysKeyState           FMMERGE.5194,  &
          WinGetScreenPS               FMMERGE.5196,  &
          WinGetSysBitmap              FMMERGE.5197,  &
          WinInSendMsg                 FMMERGE.5202,  &
          WinInflateRect               FMMERGE.5200,  &
          WinIntersectRect             FMMERGE.5203,  &
          WinInvalidateRect            FMMERGE.5204,  &
          WinInvalidateRegion          FMMERGE.5205,  &
          WinInvertRect                FMMERGE.5206,  &
          WinIsChild                   FMMERGE.5207,  &
          WinIsPhysInputEnabled        FMMERGE.5208,  &
          WinIsRectEmpty               FMMERGE.5209,  &
          WinIsThreadActive            FMMERGE.5210,  &
          WinIsWindow                  FMMERGE.5211,  &
          WinIsWindowEnabled           FMMERGE.5212,  &
          WinIsWindowShowing           FMMERGE.5213,  &
          WinIsWindowVisible           FMMERGE.5214,  &
          WinLoadAccelTable            FMMERGE.5216,  &
          WinLoadLibrary               FMMERGE.5219,  &
          WinLoadMenu                  FMMERGE.5220,  &
          WinLoadMessage               FMMERGE.5221,  &
          WinLoadPointer               FMMERGE.5222,  &
          WinLoadString                FMMERGE.5224,  &
          WinLockVisRegions            FMMERGE.5227,  &
          WinLockWindowUpdate          FMMERGE.5228,  &
          WinMakePoints                FMMERGE.5229,  &
          WinMakeRect                  FMMERGE.5230,  &
          WinMapDlgPoints              FMMERGE.5231,  &
          WinMapWindowPoints           FMMERGE.5232,  &
          WinMessageBox                FMMERGE.5233,  &
          WinMsgSemWait                FMMERGE.5236,  &
          WinNextChar                  FMMERGE.5238,  &
          WinOffsetRect                FMMERGE.5239,  &
          WinOpenClipbrd               FMMERGE.5240,  &
          WinOpenWindowDC              FMMERGE.5241,  &
          WinPrevChar                  FMMERGE.5246,  &
          WinProcessDlg                FMMERGE.5247,  &
          WinPtInRect                  FMMERGE.5248,  &
          WinQueryAccelTable           FMMERGE.5249,  &
          WinQueryActiveWindow         FMMERGE.5251,  &
          WinQueryAnchorBlock          FMMERGE.5252,  &
          WinQueryAtomLength           FMMERGE.5253,  &
          WinQueryAtomName             FMMERGE.5254,  &
          WinQueryAtomUsage            FMMERGE.5255,  &
          WinQueryCapture              FMMERGE.5256,  &
          WinQueryClassName            FMMERGE.5258,  &
          WinQueryClipbrdData          FMMERGE.5260,  &
          WinQueryClipbrdFmtInfo       FMMERGE.5261,  &
          WinQueryClipbrdOwner         FMMERGE.5262,  &
          WinQueryClipbrdViewer        FMMERGE.5263,  &
          WinQueryCp                   FMMERGE.5264,  &
          WinQueryCpList               FMMERGE.5265,  &
          WinQueryCursorInfo           FMMERGE.5266,  &
          WinQueryDesktopWindow        FMMERGE.5268,  &
          WinQueryDlgItemShort         FMMERGE.5269,  &
          WinQueryDlgItemText          FMMERGE.5270,  &
          WinQueryDlgItemTextLength    FMMERGE.5271,  &
          WinQueryFocus                FMMERGE.5272,  &
          WinQueryMsgPos               FMMERGE.5273,  &
          WinQueryMsgTime              FMMERGE.5274,  &
          WinQueryObjectWindow         FMMERGE.5275,  &
          WinQueryPointer              FMMERGE.5277,  &
          WinQueryPointerInfo          FMMERGE.5278,  &
          WinQueryPointerPos           FMMERGE.5279,  &
          WinQueryQueueInfo            FMMERGE.5282,  &
          WinQueryQueueStatus          FMMERGE.5283,  &
          WinQuerySysColor             FMMERGE.5289,  &
          WinQuerySysModalWindow       FMMERGE.5290,  &
          WinQuerySysPointer           FMMERGE.5291,  &
          WinQuerySysValue             FMMERGE.5294,  &
          WinQuerySystemAtomTable      FMMERGE.5293,  &
          WinQueryUpdateRect           FMMERGE.5297,  &
          WinQueryUpdateRegion         FMMERGE.5298,  &
          WinQueryVersion              FMMERGE.5299,  &
          WinQueryWindow               FMMERGE.5301,  &
          WinQueryWindowDC             FMMERGE.5302,  &
          WinQueryWindowPos            FMMERGE.5304,  &
          WinQueryWindowProcess        FMMERGE.5305,  &
          WinQueryWindowPtr            FMMERGE.5306,  &
          WinQueryWindowRect           FMMERGE.5307,  &
          WinQueryWindowText           FMMERGE.5308,  &
          WinQueryWindowTextLength     FMMERGE.5309,  &
          WinQueryWindowULong          FMMERGE.5311,  &
          WinQueryWindowUShort         FMMERGE.5312,  &
          WinRegisterUserDatatype      FMMERGE.5318,  &
          WinRegisterUserMsg           FMMERGE.5319,  &
          WinReleasePS                 FMMERGE.5322,  &
          WinScrollWindow              FMMERGE.5328,  &
          WinSetAccelTable             FMMERGE.5335,  &
          WinSetActiveWindow           FMMERGE.5337,  &
          WinSetCapture                FMMERGE.5338,  &
          WinSetClassMsgInterest       FMMERGE.5340,  &
          WinSetClipbrdData            FMMERGE.5342,  &
          WinSetClipbrdOwner           FMMERGE.5343,  &
          WinSetClipbrdViewer          FMMERGE.5344,  &
          WinSetCp                     FMMERGE.5345,  &
          WinSetDlgItemShort           FMMERGE.5347,  &
          WinSetDlgItemText            FMMERGE.5348,  &
          WinSetFocus                  FMMERGE.5350,  &
          WinSetMsgInterest            FMMERGE.5354,  &
          WinSetMsgMode                FMMERGE.5355,  &
          WinSetMultWindowPos          FMMERGE.5356,  &
          WinSetOwner                  FMMERGE.5357,  &
          WinSetParent                 FMMERGE.5359,  &
          WinSetPointer                FMMERGE.5360,  &
          WinSetPointerPos             FMMERGE.5362,  &
          WinSetRect                   FMMERGE.5364,  &
          WinSetRectEmpty              FMMERGE.5365,  &
          WinSetSynchroMode            FMMERGE.5366,  &
          WinSetSysColors              FMMERGE.5368,  &
          WinSetSysModalWindow         FMMERGE.5370,  &
          WinSetSysValue               FMMERGE.5372,  &
          WinSetWindowBits             FMMERGE.5374,  &
          WinSetWindowPos              FMMERGE.5375,  &
          WinSetWindowPtr              FMMERGE.5376,  &
          WinSetWindowText             FMMERGE.5377,  &
          WinSetWindowULong            FMMERGE.5379,  &
          WinSetWindowUShort           FMMERGE.5380,  &
          WinShowCursor                FMMERGE.5381,  &
          WinShowPointer               FMMERGE.5382,  &
          WinShowTrackRect             FMMERGE.5383,  &
          WinShowWindow                FMMERGE.5384,  &
          WinStartTimer                FMMERGE.5386,  &
          WinStopTimer                 FMMERGE.5387,  &
          WinSubstituteStrings         FMMERGE.5390,  &
          WinSubtractRect              FMMERGE.5391,  &
          WinTrackRect                 FMMERGE.5399,  &
          WinUnionRect                 FMMERGE.5402,  &
          WinUpdateWindow              FMMERGE.5403,  &
          WinUpper                     FMMERGE.5404,  &
          WinUpperChar                 FMMERGE.5405,  &
          WinValidateRect              FMMERGE.5406,  &
          WinValidateRegion            FMMERGE.5407,  &
          WinWaitMsg                   FMMERGE.5410,  &
          WinWindowFromDC              FMMERGE.5413,  &
          WinWindowFromID              FMMERGE.5414,  &
          WinWindowFromPoint           FMMERGE.5415,  &
          WinBroadcastMsg              FMMERGE.5116,  &
          WinPostQueueMsg              FMMERGE.5245,  &
          WinSendDlgItemMsg            FMMERGE.5333,  &
          WinTranslateAccel            FMMERGE.5401,  &
          WinCallMsgFilter             FMMERGE.5120,  &
          WinCreateFrameControls       FMMERGE.5134,  &
          WinCreateMenu                FMMERGE.5135,  &
          WinCreateWindow              FMMERGE.5141,  &
          WinDefDlgProc                FMMERGE.5146,  &
          WinDrawText                  FMMERGE.5163,  &
          WinGetDlgMsg                 FMMERGE.5183,  &
          WinMsgMuxSemWait             FMMERGE.5235,  &
          WinMultWindowFromIDs         FMMERGE.5237,  &
          WinPeekMsg                   FMMERGE.5242,  &
          WinPostMsg                   FMMERGE.5244,  &
          WinSetKeyboardStateTable     FMMERGE.5353,  &
          WinCreateDlg                 FMMERGE.5133,  &
          WinDlgBox                    FMMERGE.5159,  &
          WinLoadDlg                   FMMERGE.5218,  &
          WinQueryClassInfo            FMMERGE.5257,  &
          WinReleaseHook               FMMERGE.5321,  &
          WinSetHook                   FMMERGE.5352,  &
          WinSubclassWindow            FMMERGE.5389,  &
          WinSetClassThunkProc         FMMERGE.5341,  &
          WinQueryClassThunkProc       FMMERGE.5259,  &
          WinSetWindowThunkProc        FMMERGE.5378,  &
          WinQueryWindowThunkProc      FMMERGE.5310,  &
          WinQueryWindowModel          FMMERGE.5303,  &
          WinSetDesktopBkgnd           FMMERGE.5346,  &
          WinQueryDesktopBkgnd         FMMERGE.5267,  &
          WinPopupMenu                 FMMERGE.5243,  &
          WinSetPresParam              FMMERGE.5363,  &
          WinQueryPresParam            FMMERGE.5280,  &
          WinRemovePresParam           FMMERGE.5323,  &
          WinRealizePalette            FMMERGE.5314,  &
          WinCreatePointerIndirect     FMMERGE.5138,  &
          WinSaveWindowPos             FMMERGE.5327,  &
          WINCREATEFARHEAP             FMMERGE.2178,  &
          WINDESTROYFARHEAP            FMMERGE.2179,  &
          WINALLOCFARMEM               FMMERGE.2180,  &
          WINFREEFARMEM                FMMERGE.2181,  &
          WINREALLOCFARMEM             FMMERGE.2182,  &
          WINGETERASEPS                FMMERGE.5184,  &
          WINRELEASEERASEPS            FMMERGE.5320,  &
          Undocumented1                FMMERGE.5005,  &
          Undocumented2                FMMERGE.5006,  &
          Undocumented3                FMMERGE.5004,  &
          Undocumented4                FMMERGE.5003,  &
          Undocumented5                FMMERGE.5145,  &
          WINSETCLASSTHUNKPROC         FMMERGE.3248,  &
          WINQUERYCLASSTHUNKPROC       FMMERGE.3179,  &
          WINSETWINDOWTHUNKPROC        FMMERGE.3275,  &
          WINQUERYWINDOWTHUNKPROC      FMMERGE.3218,  &
          WinSystemShutdown            FMMERGE.5395,  &
          WinBuildPtrHandle            FMMERGE.5117,  &
          WinExplodeWindow             FMMERGE.5175,  &
          WinSetAccelTableValue        FMMERGE.5336,  &
          WinQueryAccelTableValue      FMMERGE.5250,  &
          WinStretchPointer            FMMERGE.5388,  &
          Undocumented6                 FMMERGE.5000,  &
          Undocumented7                 FMMERGE.5001,  &
          WinSetPointerOwner           FMMERGE.5361,  &
          WinGetFrameTreePPSize        FMMERGE.5187,  &
          WinGetFrameTreePPs           FMMERGE.5186,  &
          WinSetFrameTreePPs           FMMERGE.5351,  &
          WinQueryProcessCP            FMMERGE.5281,  &
          WinVisRegionNotify           FMMERGE.5408,  &
          WinSetErrorInfo              FMMERGE.5349,  &
          WinWaitEventSem              FMMERGE.5409,  &
          WinRequestMutexSem           FMMERGE.5326,  &
          WinWaitMuxWaitSem            FMMERGE.5411,  &
          WINSETSYSCLIPBRDVIEWER       FMMERGE.3269,  &
          WinSetSysClipbrdViewer       FMMERGE.5367,  &
          WinSemQuery                  FMMERGE.5332,  &
          WinSemEnter                  FMMERGE.5330,  &
          WinSemLeave                  FMMERGE.5331,  &
          WinLoadProcedure             FMMERGE.5223,  &
          WinDeleteProcedure           FMMERGE.5150,  &
          WinMoveCursor                FMMERGE.5234,  &
          WinQuerySendMsg              FMMERGE.5284,  &
          WinReplyMsg                  FMMERGE.5325,  &
          WinWakeThread                FMMERGE.5412,  &
          WinQueueFromID               FMMERGE.5313,  &
          WinThreadAssocQueue          FMMERGE.5398,  &
          WinLockInput                 FMMERGE.5225,  &
          WinSetSysFont                FMMERGE.5369,  &
          WinLockPointerUpdate         FMMERGE.5226,  &
          WinCheckInput                FMMERGE.5123,  &
          Undocumented8                FMMERGE.5044,  &
          WinQueryVisibleRegion        FMMERGE.5300,  &
          WinSetVisibleRegionNotify    FMMERGE.5373,  &
          WinSetSysPointerData         FMMERGE.5371,  &
          WinQuerySysPointerData       FMMERGE.5292,  &
          WinSeamlessBox               FMMERGE.5329,  &
          WinJiggleMouse               FMMERGE.5215,  &
          WinTrackSeamless             FMMERGE.5400,  &
          WinRegisterSeamlessProc      FMMERGE.5317,  &
          WinSetCharXlateTbl           FMMERGE.5339,  &
          WinMspInit                   FMMERGE.5420,  &
          WinMspAlloc                  FMMERGE.5418,  &
          WinMspFree                   FMMERGE.5419,  &
          Undocumented9                FMMERGE.5109,  &
          Undocumented10               FMMERGE.5068,  &
          WinRegisterPen               FMMERGE.5316,  &
          WinMessageBox2               FMMERGE.5417,  &
          fPenAvail                    FMMERGE.6201,  &
          WinDrawTabbedText            FMMERGE.5432,  &
          PMREQUESTMUTEXSEM            FMMERGE.5071,  &
          PMRELEASEMUTEXSEM            FMMERGE.5070,  &
          WinQueryPointerClipRect      FMMERGE.5433,  &
          WinSetPointerClipRect        FMMERGE.5434,  &
          WinWaitMsgTimeout            FMMERGE.5436,  &
          WinIsWindowOfClass           FMMERGE.5437,  &
          WinSetAtomTableOwner         FMMERGE.5438,  &
          WinXxCreateHeap              FMMERGE.5425,  &
          WinXxDestroyHeap             FMMERGE.5426,  &
          WinXxAllocMem                FMMERGE.5424,  &
          WinXxFreeMem                 FMMERGE.5427,  &
          WinXxAllocFill               FMMERGE.5423,  &
          WinXxReallocMem              FMMERGE.5429,  &
          WinXxQueryMemSize            FMMERGE.5439,  &
          WinSetClipRegion             FMMERGE.5440,  &
          WinQueryClipRegion           FMMERGE.5441,  &
          Undocumented11               FMMERGE.5442,  &
          Undocumented12               FMMERGE.5443,  &
          Undocumented13               FMMERGE.5444,  &
          Undocumented14               FMMERGE.5445,  &
          Undocumented15               FMMERGE.5446,  &
          Undocumented16               FMMERGE.5448,  &
          Undocumented17               FMMERGE.5447,  &
          WINENUMPROPERTIES            FMMERGE.5449,  &
          WINQUERYPROPERTY             FMMERGE.5450,  &
          WINREMOVEPROPERTY            FMMERGE.5451,  &
          WINSETPROPERTY               FMMERGE.5452,  &
          WINENUMCLASSPROPERTIES       FMMERGE.5453,  &
          WINQUERYCLASSPROPERTY        FMMERGE.5454,  &
          WINREMOVECLASSPROPERTY       FMMERGE.5455,  &
          WINSETCLASSPROPERTY          FMMERGE.5456,  &
          Undocumented18               FMMERGE.5458,  &
          Undocumented19               FMMERGE.5461,  &
          Undocumented20               FMMERGE.5462,  &
          WinLoadVkeyGlyphXlatetbl     FMMERGE.5463,  &
          WinSetVKeyGlyphXlatetbl      FMMERGE.5464,  &
          WinQueryVKeyGlyphXlatetbl    FMMERGE.5465,  &
          WinTrackWindow               FMMERGE.5466,  &
          WinSemEnterTimeout           FMMERGE.5467,  &
          Undocumented21               FMMERGE.5468,  &
          Undocumented22               FMMERGE.5469,  &
          Undocumented23               FMMERGE.5473,  &
          Undocumented24               FMMERGE.5474,  &
          Undocumented25               FMMERGE.5475,  &
          Undocumented26               FMMERGE.5476,  &
          Undocumented27               FMMERGE.5477,  &
          Undocumented28               FMMERGE.5478,  &
          WINSETLANGINFO               FMMERGE.3500,  &
          WINQUERYLANGINFO             FMMERGE.3501,  &
          WINSETKBDLAYER               FMMERGE.3502,  &
          WINQUERYKBDLAYER             FMMERGE.3503,  &
          WINSETLANGVIEWER             FMMERGE.3504,  &
          WINQUERYLANGVIEWER           FMMERGE.3505,  &
          WINPOUREVENT                 FMMERGE.3600,  &
          WINSETMSGQIMINFO             FMMERGE.3601,  &
          WINQUERYMSGQIMINFO           FMMERGE.3602,  &
          Undocumented29               FMMERGE.10000,  &
          Undocumented30               FMMERGE.10001,  &
          Undocumented31               FMMERGE.10002,  &
          Undocumented32               FMMERGE.10004,  &
          Undocumented33               FMMERGE.10005,  &
          Undocumented34               FMMERGE.10007,  &
          Undocumented35               FMMERGE.10018,  &
          Undocumented36               FMMERGE.10019,  &
          Undocumented37               FMMERGE.10020,  &
          Undocumented38               FMMERGE.10021
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
          Undocumented1               .00954,  &
          Undocumented2               .00955,  &
          Undocumented3               .00956,  &
          Undocumented4               .00957,  &
          Undocumented5               .00958,  &
          WinSystemShutdown           .00963,  &
          WinBuildPtrHandle           .00964,  &
          WinExplodeWindow            .00965,  &
          WinSetAccelTableValue       .00966,  &
          WinQueryAccelTableValue     .00967,  &
          WinStretchPointer           .00968,  &
          Undocumented6               .00969,  &
          Undocumented7               .00970,  &
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
          Undocumented8               .00999,  &
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
          Undocumented9               .01012,  &
          Undocumented10              .01013,  &
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
          Undocumented11              .01045,  &
          Undocumented12              .01046,  &
          Undocumented13              .01047,  &
          Undocumented14              .01048,  &
          Undocumented15              .01049,  &
          Undocumented16              .01050,  &
          Undocumented17              .01051,  &
          Undocumented18              .01060,  &
          Undocumented19              .01061,  &
          Undocumented20              .01062,  &
          WinLoadVkeyGlyphXlatetbl    .01063,  &
          WinSetVKeyGlyphXlatetbl     .01064,  &
          WinQueryVKeyGlyphXlatetbl   .01065,  &
          WinTrackWindow              .01066,  &
          WinSemEnterTimeout          .01067,  &
          Undocumented21              .01068,  &
          Undocumented22              .01069,  &
          Undocumented23              .01070,  &
          Undocumented24              .01071,  &
          Undocumented25              .01072,  &
          Undocumented26              .01073,  &
          Undocumented27              .01074,  &
          Undocumented28              .01075,  &
          Undocumented29              .10000,  &
          Undocumented30              .10001,  &
          Undocumented31              .10002,  &
          Undocumented32              .10004,  &
          Undocumented33              .10005,  &
          Undocumented34              .10007,  &
          Undocumented35              .10018,  &
          Undocumented36              .10019,  &
          Undocumented37              .10020,  &
          Undocumented38              .10021
DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
