/*
 $Id: FreePM_winConstants.hpp,v 1.2 2003/06/18 22:51:49 evgen2 Exp $
*/
#ifndef FREEPM_WINCONSTANTS
  #define FREEPM_WINCONSTANTS
/*** Standard Window Classes ***********************************************/
#ifdef __cplusplus
      extern "C" {
#endif

#define WC_NULL              (0x0000)

#define WC_FRAME             ((PSZ)0xffff0001L)
#define WC_COMBOBOX          ((PSZ)0xffff0002L)
#define WC_BUTTON            ((PSZ)0xffff0003L)
#define WC_MENU              ((PSZ)0xffff0004L)
#define WC_STATIC            ((PSZ)0xffff0005L)
#define WC_ENTRYFIELD        ((PSZ)0xffff0006L)
#define WC_LISTBOX           ((PSZ)0xffff0007L)
#define WC_SCROLLBAR         ((PSZ)0xffff0008L)
#define WC_TITLEBAR          ((PSZ)0xffff0009L)
#define WC_MLE               ((PSZ)0xffff000AL)
/* 000B to 000F reserved */
#define WC_APPSTAT           ((PSZ)0xffff0010L)
#define WC_KBDSTAT           ((PSZ)0xffff0011L)
#define WC_PECIC             ((PSZ)0xffff0012L)
#define WC_DBE_KKPOPUP       ((PSZ)0xffff0013L)
/* 0014 to 001F reserved */
#define WC_SPINBUTTON        ((PSZ)0xffff0020L)
/* 0021 to 0024 reserved */
#define WC_CONTAINER         ((PSZ)0xffff0025L)
#define WC_SLIDER            ((PSZ)0xffff0026L)
#define WC_VALUESET          ((PSZ)0xffff0027L)
#define WC_NOTEBOOK          ((PSZ)0xffff0028L)
/* 0029 to 002C used by PEN */
#define WC_PENFIRST          ((PSZ)0xffff0029L)
#define WC_PENLAST           ((PSZ)0xffff002CL)
/* 002D to 0030 reserved */
/* 0030 to 003F reserved */
#define WC_MMPMFIRST         ((PSZ)0xffff0040L)
#define WC_CIRCULARSLIDER    ((PSZ)0xffff0041L)
#define WC_MMPMLAST          ((PSZ)0xffff004fL)
/* 0050 to 005f reserved */
#define WC_PRISTDDLGFIRST    ((PSZ)0xffff0050L)
#define WC_PRISTDDLGLAST     ((PSZ)0xffff0057L)
#define WC_PUBSTDDLGFIRST    ((PSZ)0xffff0058L)
#define WC_PUBSTDDLGLAST     ((PSZ)0xffff005fL)
/* 0060 to 006f reserved */

/*** Standard Window Styles ************************************************/

#define WS_VISIBLE                 0x80000000L
#define WS_DISABLED                0x40000000L
#define WS_CLIPCHILDREN            0x20000000L
#define WS_CLIPSIBLINGS            0x10000000L
#define WS_PARENTCLIP              0x08000000L
#define WS_SAVEBITS                0x04000000L
#define WS_SYNCPAINT               0x02000000L
#define WS_MINIMIZED               0x01000000L
#define WS_MAXIMIZED               0x00800000L
#define WS_ANIMATE                 0x00400000L

/*** Dialog manager styles *************************************************/

#define WS_GROUP                   0x00010000L
#define WS_TABSTOP                 0x00020000L
#define WS_MULTISELECT             0x00040000L

/*** Class styles *********************************************************/

#define CS_MOVENOTIFY              0x00000001L
#define CS_SIZEREDRAW              0x00000004L
#define CS_HITTEST                 0x00000008L
#define CS_PUBLIC                  0x00000010L
#define CS_FRAME                   0x00000020L
#define CS_CLIPCHILDREN            0x20000000L
#define CS_CLIPSIBLINGS            0x10000000L
#define CS_PARENTCLIP              0x08000000L
#define CS_SAVEBITS                0x04000000L
#define CS_SYNCPAINT               0x02000000L

/* FRAME */
   /* Frame window related messages */
   
   #define WM_FLASHWINDOW             0x0040
   #define WM_FORMATFRAME             0x0041
   #define WM_UPDATEFRAME             0x0042
   #define WM_FOCUSCHANGE             0x0043
   
   #define WM_SETBORDERSIZE           0x0044
   #define WM_TRACKFRAME              0x0045
   #define WM_MINMAXFRAME             0x0046
   #define WM_SETICON                 0x0047
   #define WM_QUERYICON               0x0048
   #define WM_SETACCELTABLE           0x0049
   #define WM_QUERYACCELTABLE         0x004a
   #define WM_TRANSLATEACCEL          0x004b
   #define WM_QUERYTRACKINFO          0x004c
   #define WM_QUERYBORDERSIZE         0x004d
   #define WM_NEXTMENU                0x004e
   #define WM_ERASEBACKGROUND         0x004f
   #define WM_QUERYFRAMEINFO          0x0050
   #define WM_QUERYFOCUSCHAIN         0x0051
   #define WM_OWNERPOSCHANGE          0x0052
   #define WM_CALCFRAMERECT           0x0053
   /* Note 0x0054 is reserved */
   #define WM_WINDOWPOSCHANGED        0x0055
   #define WM_ADJUSTFRAMEPOS          0x0056
   #define WM_QUERYFRAMECTLCOUNT      0x0059

      /* Note 0x005A is reserved */
      #define WM_QUERYHELPINFO        0x005B
      #define WM_SETHELPINFO          0x005C
      #define WM_ERROR                0x005D
      #define WM_REALIZEPALETTE       0x005E
      
      
      /* WM_QUERYFRAMEINFO constants */
      
      #define FI_FRAME                0x00000001L
      #define FI_OWNERHIDE            0x00000002L
      #define FI_ACTIVATEOK           0x00000004L
      #define FI_NOMOVEWITHOWNER      0x00000008L
      

   /* Frame control IDs    */
   
   #define FID_SYSMENU                0x8002
   #define FID_TITLEBAR               0x8003
   #define FID_MINMAX                 0x8004
   #define FID_MENU                   0x8005
   #define FID_VERTSCROLL             0x8006
   #define FID_HORZSCROLL             0x8007
   #define FID_CLIENT                 0x8008
   /* Note 0x8009 is reserved */
   
   #define FID_DBE_APPSTAT            0x8010
   #define FID_DBE_KBDSTAT            0x8011
   #define FID_DBE_PECIC              0x8012
   #define FID_DBE_KKPOPUP            0x8013
   
   /* Standard WM_SYSCOMMAND command values */
   
   #define SC_SIZE                    0x8000
   #define SC_MOVE                    0x8001
   #define SC_MINIMIZE                0x8002
   #define SC_MAXIMIZE                0x8003
   #define SC_CLOSE                   0x8004
   #define SC_NEXT                    0x8005
   #define SC_APPMENU                 0x8006
   #define SC_SYSMENU                 0x8007
   #define SC_RESTORE                 0x8008
   #define SC_NEXTFRAME               0x8009
   #define SC_NEXTWINDOW              0x8010

      #define SC_TASKMANAGER             0x8011
      #define SC_HELPKEYS                0x8012
      #define SC_HELPINDEX               0x8013
      #define SC_HELPEXTENDED            0x8014
      #define SC_SWITCHPANELIDS          0x8015
      #define SC_DBE_FIRST               0x8018
      #define SC_DBE_LAST                0x801F
      
      #define SC_BEGINDRAG               0x8020
      #define SC_ENDDRAG                 0x8021
      #define SC_SELECT                  0x8022
      #define SC_OPEN                    0x8023
      #define SC_CONTEXTMENU             0x8024
      #define SC_CONTEXTHELP             0x8025
      #define SC_TEXTEDIT                0x8026
      #define SC_BEGINSELECT             0x8027
      #define SC_ENDSELECT               0x8028
      #define SC_WINDOW                  0x8029
      #define SC_HIDE                    0x802a
      
#ifdef __cplusplus
      }
#endif

#endif
  /* FREEPM_WINCONSTANTS */