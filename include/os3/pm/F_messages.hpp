/* 
 $Id: F_messages.hpp,v 1.6 2003/06/18 22:51:49 evgen2 Exp $ 
*/
/* F_messages.hpp  */
/* messages - related stuff */
/* ver 0.00 16.08.2002     */

#ifndef FREEPM_MESSAGES
  #define FREEPM_MESSAGES

#ifdef __cplusplus
      extern "C" {
#endif

   /* QMSG structure */
   typedef struct _QMSG    /* qmsg */
   {
      HWND    hwnd;
      ULONG   msg;
      MPARAM  mp1;
      MPARAM  mp2;
      ULONG   time;   /* compatibility time of message in msec since system boot */
      POINTL  ptl;
      ULONG   uid;    /* user id - reserved in old PM _QMSG */
/* =======> end of old PM _QMSG structure <======= */
      ULONG   remoteId; /* 0- local message, else remote connection id */
      double  dtime;  /* time of message in sec */
   } QMSG;
   typedef QMSG *PQMSG;
   
   /* SQMSG structure */
   typedef struct _SQMSG    /* sqmsg */
   {
      QMSG    qmsg;         /* message */
      HAB     ihto;         /* iHABto, this field is server writeble */
      HAB     ihfrom;       /* iHABfrom, this field is client writeble */
   } SQMSG;
   typedef SQMSG *PSQMSG;
   
   /*** Standard Window Messages *******************************************/
   #define WM_NULL                    0x0000
   #define WM_CREATE                  0x0001
   #define WM_DESTROY                 0x0002
   
   #ifndef F_INCL_SAADEFS
   #endif /* !F_INCL_SAADEFS */
   
   #define WM_ENABLE                  0x0004
   #define WM_SHOW                    0x0005
   #define WM_MOVE                    0x0006
   #define WM_SIZE                    0x0007
   #define WM_ADJUSTWINDOWPOS         0x0008
   
   #define WM_CALCVALIDRECTS          0x0009
   
   #define WM_SETWINDOWPARAMS         0x000a
   #define WM_QUERYWINDOWPARAMS       0x000b
   #define WM_HITTEST                 0x000c
   #define WM_ACTIVATE                0x000d
   #define WM_SETFOCUS                0x000f
   #define WM_SETSELECTION            0x0010
   
   /* language support Winproc */
   #define WM_PPAINT                  0x0011
   #define WM_PSETFOCUS               0x0012
   #define WM_PSYSCOLORCHANGE         0x0013
   #define WM_PSIZE                   0x0014
   #define WM_PACTIVATE               0x0015
   #define WM_PCONTROL                0x0016
   
   #define WM_COMMAND                 0x0020
   #define WM_SYSCOMMAND              0x0021
   #define WM_HELP                    0x0022
   #define WM_PAINT                   0x0023
   
   #ifndef F_INCL_SAADEFS
      #define WM_TIMER                0x0024
      #define WM_SEM1                 0x0025
      #define WM_SEM2                 0x0026
      #define WM_SEM3                 0x0027
      #define WM_SEM4                 0x0028
   #endif /* !F_INCL_SAADEFS */
   
   #define WM_CLOSE                   0x0029
   #define WM_QUIT                    0x002a
   #define WM_SYSCOLORCHANGE          0x002b
   #define WM_SYSVALUECHANGED         0x002d
   #define WM_APPTERMINATENOTIFY      0x002e
   #define WM_PRESPARAMCHANGED        0x002f
   
   /* Control notification messages */
   #define WM_CONTROL                 0x0030
   #define WM_VSCROLL                 0x0031
   #define WM_HSCROLL                 0x0032
   #define WM_INITMENU                0x0033
   #define WM_MENUSELECT              0x0034
   #define WM_MENUEND                 0x0035
   #define WM_DRAWITEM                0x0036
   #define WM_MEASUREITEM             0x0037
   #define WM_CONTROLPOINTER          0x0038
   #define WM_QUERYDLGCODE            0x003a
   #define WM_INITDLG                 0x003b
   #define WM_SUBSTITUTESTRING        0x003c
   #define WM_MATCHMNEMONIC           0x003d
   #define WM_SAVEAPPLICATION         0x003e
   
   #define WM_SEMANTICEVENT           0x0490

   /*** Mouse input messages *****************/
   #define WM_MOUSEMOVE               0x0070
   #define WM_BUTTON1DOWN             0x0071
   #define WM_BUTTON1UP               0x0072
   #define WM_BUTTON1DBLCLK           0x0073
   #define WM_BUTTON2DOWN             0x0074
   #define WM_BUTTON2UP               0x0075
   #define WM_BUTTON2DBLCLK           0x0076
      #define WM_BUTTON3DOWN          0x0077
      #define WM_BUTTON3UP            0x0078
      #define WM_BUTTON3DBLCLK        0x0079

   /*** Key/Character input messages *********/
   #define WM_CHAR                    0x007a
   #define WM_VIOCHAR                 0x007b

   /* WM_CHAR fs field bits */
   #define KC_NONE                    0x0000     /* Reserved */
   #define KC_CHAR                    0x0001
   #define KC_VIRTUALKEY              0x0002
   #define KC_SCANCODE                0x0004
   
   #define KC_SHIFT                   0x0008
   #define KC_CTRL                    0x0010
   #define KC_ALT                     0x0020
   #define KC_KEYUP                   0x0040
   #define KC_PREVDOWN                0x0080
   #define KC_LONEKEY                 0x0100
   #define KC_DEADKEY                 0x0200
   #define KC_COMPOSITE               0x0400
   #define KC_INVALIDCOMP             0x0800
   

#ifdef __cplusplus
      }
#endif

#endif  /* FREEPM_MESSAGES */
