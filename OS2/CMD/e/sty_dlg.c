/*************************************************************************
*
*  File Name   : STY_DLG.C
*
*  Description : This module contains the Dialog Procedures for the user
*                defined dialogs as well as any support code they need.
*
*  Concepts    : Dialog procedures for user defined dialogs,
*                initialization of controls in the dialog boxs,
*                use of presentation parameters.
*
*  API's       :  DosAllocMem             WinQueryFocus
*                 DosFreeMem              WinQueryPointerPos
*                 GpiQueryFonts           WinQueryPresParams
*                 WinCreateWindow         WinQueryWindowText
*                 WinDefDlgProc           WinQueryWindowUShort
*                 WinDismissDlg           WinReleasePS
*                 WinGetPS                WinRemovePresParams
*                 WinLoadMenu             WinSendDlgItemMsg
*                 WinLoadPointer          WinSendMsg
*                 WinLoadString           WinSetDlgItemText
*                 WinMapWindowPoints      WinSetPresParams
*                 WinPopupMenu            WinWindowFromID
*
*  Copyright (C) 1992 IBM Corporation
*
*      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is
*      sample code created by IBM Corporation. This sample code is not
*      part of any standard or IBM product and is provided to you solely
*      for  the purpose of assisting you in the development of your
*      applications.  The code is provided "AS IS", without
*      warranty of any kind.  IBM shall not be liable for any damages
*      arising out of your use of the sample code, even if they have been
*      advised of the possibility of such damages.                                                    *
************************************************************************/

/*  Include files, macros, defined constants, and externs              */

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINPOINTERS
#define INCL_WININPUT
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS
#define INCL_WINSTDSPIN
#define INCL_WINSTDBOOK
#define INCL_WINMLE
#define INCL_WINMENUS
#define INCL_WINSTATICS
#define INCL_WINSTDCNR
#define INCL_WINSTDSLIDER
#define INCL_WINSTDVALSET
#define INCL_WINDIALOGS
#define INCL_WINSYS
#define INCL_GPILCIDS
#define INCL_GPIBITMAPS
#define INCL_DOSMEMMGR

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "sty_main.h"
#include "sty_dlg.h"
#include "sty_xtrn.h"
#include "sty_help.h"

typedef struct _USERRECORD
{
  RECORDCORE  recordCore;
  CDATE       date;
  CTIME       time;
  PSZ         recordData;
} USERRECORD, *PUSERRECORD;

#define COLORNAMELEN    20
#define NUMCOLORS       15  /* number of colors used in PP Demo */
#define NUMSPINCOLORS   16  /* number of colors used in SPIN Demo */
#define NUMFONTS         4  /* number of fonts used in PP Demo */
#define FONTPOINTSIZE   80  /* point size to use in PP Demo */
#define PPFONTPOINTSIZE  2              /* length of font point prefix */

/*  Global variables                                                   */
                                        /* array of clr names          */
static CHAR apszPresParamColor[NUMCOLORS][COLORNAMELEN];
static PSZ *apszPresParamFont = NULL;
static LONG lNumFonts;
static CNRINFO cnrinfo;
static ULONG ulCnrAttributes;
static RECORDINSERT recordInsert;
static PUSERRECORD pUserRecord;
static PFIELDINFO pFieldInfo, firstFieldInfo;
static FIELDINFOINSERT fieldInfoInsert;
static PFIELDINFOINSERT pFieldInfoInsert;
static CHAR pszViewText[] = "Text for Text View.";
static CHAR pszViewIcon[] = "Text for Icon View.";
static CHAR pszViewName[] = "Text for Name View.";
static CHAR pszColumnText1[]= "Data";
static CHAR pszColumnText2[]= "Date";
static CHAR pszColumnText3[]= "Time";
static CHAR pszSampleData[] = "Sample Data";
static CHAR pszCnrTitle[] = "Sample Container";

/*
 * Set up an array of colors in the same order as the stringtable
 * constants
 */
static ULONG aulColor[NUMCOLORS-1] =
              { CLR_BLUE, CLR_RED, CLR_PINK, CLR_GREEN, CLR_CYAN,
                CLR_YELLOW, CLR_DARKGRAY, CLR_DARKBLUE, CLR_DARKRED,
                CLR_DARKPINK, CLR_DARKGREEN, CLR_DARKCYAN, CLR_BROWN,
                CLR_PALEGRAY };

/*  Entry point declarations                                           */

MRESULT EXPENTRY ProdInfoDlgProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DemoDlgProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY PresParamDemoDlgProc(HWND hwnd, USHORT msg, MPARAM mp1,
                                      MPARAM mp2);
BOOL InitializeValueSet(HWND hwnd);
BOOL InitializeSpinButton(HWND hwnd);
BOOL InitializeSlider(HWND hwnd);
BOOL InitializeNoteBook(HWND hwnd);
BOOL InitializeContainer(HWND hwnd);
BOOL LoadListBox(HWND hwnd);
BOOL InitPresParamDemoDlg(HWND hwnd);
BOOL UpdatePresParamDemoDlg(HWND hwnd);
VOID ShowDlgHelp(HWND hwnd);
VOID SetSysMenu(HWND hwnd);


/*********************************************************************
 *  Name : ProdInfoDlgProc
 *
 *  Description : Processes all messages sent to the Product
 *                Information dialog box.
 *
 *  Concepts : Called for each message sent to the Product
 *             Information dialog box.  The Product
 *             Information box only has a button control so
 *             this routine only processes WM_COMMAND
 *             messages.  Any WM_COMMAND posted must have come
 *             from the Ok button so we dismiss the dialog
 *             upon receiving it.
 *
 *  API's : WinDismissDlg
 *          WinDefDlgProc
 *
 * Parameters   : hwnd - Window handle to which message is addressed
 *                msg - Message type
 *                mp1 - First message parameter
 *                mp2 - Second message parameter
 *
 *  Returns : Dependent upon message sent
 *
 ****************************************************************/
MRESULT EXPENTRY ProdInfoDlgProc( HWND hwnd, USHORT msg, MPARAM mp1,
                                  MPARAM mp2)
{
   switch(msg)
   {
      case WM_COMMAND:
         /*
          * No matter what the command, close the dialog
          */
         WinDismissDlg(hwnd, TRUE);
         break;

      case WM_HELP:
         ShowDlgHelp(hwnd);
         break;

      default:
         return(WinDefDlgProc(hwnd, msg, mp1, mp2));
         break;
   }
   return (MRESULT)0;
}           /* End of ProdInfolDlgProc()                               */


/*********************************************************************
 *  Name : DemoDlgProc
 *
 *  Description : Processes all messages sent to all Demo dialogs.
 *
 *  Concepts : Called for each message sent to a demo dialog box.
 *             A switch statement branches to the routines to be
 *             performed for each message processed.  Any messages
 *             not specifically process are passed to the default
 *             window procedure WinDefDlgProc().
 *
 *  API's : WinDismissDlg
 *          WinLoadMenu
 *          WinQueryPointerPos
 *          WinMapWindowPoints
 *          WinPopupMenu
 *          WinSendDlgItemMsg
 *          WinSetDlgItemText
 *          WinDefDlgProc
 *
 * Parameters   : hwnd - Window handle to which message is addressed
 *                msg - Message type
 *                mp1 - First message parameter
 *                mp2 - Second message parameter
 *
 *  Returns : Dependent upon message sent
 *
 ****************************************************************/
MRESULT EXPENTRY DemoDlgProc( HWND hwnd, USHORT msg, MPARAM mp1,
                              MPARAM mp2)
{
   CHAR      acBuffer[4];
   CHAR      *cData;
   PSHORT    psIdDlg;

   cnrinfo.pszCnrTitle = pszCnrTitle;
   ulCnrAttributes = CA_CONTAINERTITLE | CA_TITLESEPARATOR;

   switch(msg)
   {
      case WM_INITDLG:

         SetSysMenu(hwnd);
         psIdDlg = (PSHORT) mp2;

         switch(*psIdDlg)
         {
        /*
         * The ShowDemoDlg() filled mp2 with a pointer to the id of
         * the dialog template used to create the dialog box.
         */
            case IDD_LISTBOXDLG:
            /*
             * Load some text into the list boxes.  If any of the
             * LoadListBox() routines fail, abort the dialog
             */
               if (!LoadListBox(WinWindowFromID(hwnd, IDC_LISTBOX)) ||
                   !LoadListBox(WinWindowFromID(hwnd, IDC_MULTISELLISTBOX)))
               {
                  WinDismissDlg(hwnd, FALSE);
               }
               break;

            case IDD_COMBOBOXDLG:
            /*
             * Load some text into the combo boxes.  If any of the
             * LoadListBox() routines fail, abort the dialog
             */
               if (!LoadListBox(WinWindowFromID(hwnd, IDC_SIMPLE)) ||
                   !LoadListBox(WinWindowFromID(hwnd, IDC_DROPDOWN)) ||
                   !LoadListBox(WinWindowFromID(hwnd, IDC_DROPDOWNLIST)))
               {
                  WinDismissDlg(hwnd, FALSE);
               }
               break;

            case IDD_SPINBUTTONDLG:
            /*
             * Initialize the master and servant Spinbuttons.
             * If the initialization routine fails, abort
             * the dialog
             */
               if (!InitializeSpinButton(hwnd))
               {
                  WinDismissDlg(hwnd, FALSE);
               }
               break;

            case IDD_VALUESETDLG:
            /*
             * Initialize the Valueset control.
             * If the initialization routine fails, abort
             * the dialog
             */
               if (!InitializeValueSet(hwnd))
               {
                  WinDismissDlg(hwnd, FALSE);
               }
               break;

            case IDD_CONTAINERDLG:
            /*
             * Initialize the Container control.
             * If the initialization routine fails, abort
             * the dialog
             */
               cnrinfo.flWindowAttr = CV_ICON | ulCnrAttributes;
               if (!InitializeContainer(hwnd))
               {
                  WinDismissDlg(hwnd, FALSE);
               }
               break;

            case IDD_NOTEBOOKDLG:
            /*
             * Initialize the Notebook control.
             * If the initialization routine fails, abort
             * the dialog
             */
               if (!InitializeNoteBook(hwnd))
               {
                  WinDismissDlg(hwnd, FALSE);
               }
               break;

            case IDD_SLIDERDLG:
            /*
             * Initialize the Slider control.
             * If the initialization routine fails, abort
             * the dialog
             */
               if (!InitializeSlider(hwnd))
               {
                  WinDismissDlg(hwnd, FALSE);
               }
               break;

            default:
               break;
         }
         break;

      case WM_CONTROL:
         /*
          * Process the WM_CONTROL messages for the slider and valueset
          * set controls.
          */
         switch(SHORT2FROMMP(mp1))
         {
            case CN_CONTEXTMENU:
            {
               POINTL point;
               HWND hwndMenu;
               HWND hwndContainer;
               /*
                * If we receive a EN_CONTEXTMENU MESSAGE and if the
                * current dialog if the container dialog, display a
                * popup menu that allows the user to change the
                * container view.
                */
                hwndContainer = WinWindowFromID(hwnd,IDC_CONTAINER);

                hwndMenu = WinLoadMenu(hwnd, 0, IDD_CONTAINERMENU);
                WinQueryPointerPos(HWND_DESKTOP, &point);
                WinMapWindowPoints(HWND_DESKTOP,hwndContainer,&point,1);

                WinPopupMenu(hwndContainer, hwnd, hwndMenu,
                             point.x, point.y,
                             IDM_ICON, PU_NONE | PU_SELECTITEM |
                             PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 | PU_KEYBOARD);
               break;
            }
            case SLN_CHANGE:
            {
               LONG ulValue;

               if (SHORT1FROMMP(mp1) == IDC_HSLIDER)
               {
                  ulValue = (ULONG) WinSendDlgItemMsg(hwnd, IDC_HSLIDER,
                     SLM_QUERYSLIDERINFO,
                     MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),
                     NULL);
                  cData = _ltoa(ulValue,acBuffer,10);
                  WinSetDlgItemText(hwnd,IDC_HSLIDERDATA, cData);
               }
               else
               {
                  ulValue = (ULONG) WinSendDlgItemMsg(hwnd, IDC_VSLIDER,
                     SLM_QUERYSLIDERINFO,
                     MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),
                     NULL);
                  cData = _ltoa(ulValue,acBuffer,10);
                  WinSetDlgItemText(hwnd,IDC_VSLIDERDATA, cData);
               }
               break;
            }
            case VN_SELECT:
            case VN_ENTER:
            {
               CHAR  pszNotificationTxt[256];
               CHAR  pszTmp[5];
               ULONG ulRowCol;

               ulRowCol = (ULONG)WinSendDlgItemMsg(hwnd, IDC_VALUESET,
                  VM_QUERYSELECTEDITEM, NULL, NULL);

               if ((SHORT2FROMMP(mp1)) == VN_SELECT)
                  strcpy (pszNotificationTxt, "VN_SELECT for item at row ");
               else
                  strcpy (pszNotificationTxt, "VN_ENTER for item at row ");

               _itoa(SHORT1FROMMP(ulRowCol), pszTmp, 10);
               strcat (pszNotificationTxt, pszTmp);
               strcat (pszNotificationTxt, ",Column ");
               _itoa(SHORT2FROMMP(ulRowCol), pszTmp, 10);
               strcat (pszNotificationTxt, pszTmp);
               WinSetDlgItemText(hwnd,IDC_VALUESETTEXT, pszNotificationTxt);
               break;
            }
         }
         break;
      case WM_COMMAND:
         switch( SHORT1FROMMP( mp1 ) )     /* Extract the command value    */
         {
            /*
             * Process the command messages from the container popup menu
             */
            case IDM_ICON:
               cnrinfo.flWindowAttr = CV_ICON | ulCnrAttributes;
               WinSendDlgItemMsg(hwnd,IDC_CONTAINER,CM_SETCNRINFO,&cnrinfo,
                              MPFROMLONG(CMA_FLWINDOWATTR));
               break;

            case IDM_NAME:
               cnrinfo.flWindowAttr = CV_NAME | ulCnrAttributes;
               WinSendDlgItemMsg(hwnd,IDC_CONTAINER,CM_SETCNRINFO,&cnrinfo,
                             MPFROMLONG(CMA_FLWINDOWATTR));
               break;

            case IDM_TEXT:
               cnrinfo.flWindowAttr = CV_TEXT | ulCnrAttributes;
               WinSendDlgItemMsg(hwnd,IDC_CONTAINER,CM_SETCNRINFO,&cnrinfo,
                                MPFROMLONG(CMA_FLWINDOWATTR));
               break;

            case IDM_DETAIL:
               cnrinfo.flWindowAttr = CV_DETAIL | ulCnrAttributes |
                  CA_DETAILSVIEWTITLES;

               WinSendDlgItemMsg(hwnd,IDC_CONTAINER,CM_SETCNRINFO,&cnrinfo,
                                MPFROMLONG(CMA_FLWINDOWATTR));

               WinSendDlgItemMsg(hwnd,IDC_CONTAINER,
                                 CM_INVALIDATEDETAILFIELDINFO,      /*@@@RBS*/
                                 NULL, NULL);

               break;

            case DID_OK:
            case DID_CANCEL:          /* The Cancel pushbutton or Escape key */
               WinDismissDlg( hwnd, TRUE );     /* Removes the dialog box    */
               return (MRESULT)FALSE;

            default:
               break;
         }
         break;

      case WM_HELP:
         ShowDlgHelp(hwnd);
         break;

      default:
         return(WinDefDlgProc(hwnd, msg, mp1, mp2));
         break;
   }
   return (MRESULT)0;
}                                            /* End of DemoDlgProc()        */

/*********************************************************************
 *  Name : LoadListBox
 *
 *  Description : Loads the demo list box with 0 through 9
 *
 *
 *  Concepts : Called each time a demo list box is
 *             initialized.  A for 0 to 9 loop is converts
 *             the INT to a string which is added to the
 *             listbox via LM_INSERTITEM
 *
 *  API's : WinLoadString
 *          WinSendMsg
 *
 *  Parameters : hwnd - Window handle of the list box.
 *
 *  Returns: TRUE if list box is loaded successfully, FALSE otherwise
 *
 ****************************************************************/
BOOL LoadListBox(HWND hwnd)
{
   ULONG ulCounter;
   CHAR pszTemp[MESSAGELEN];
   SHORT sResult;

   for(ulCounter = 0; ulCounter < 10; ulCounter++)
   {
      if (!WinLoadString(hab, (HMODULE)0, (IDS_LISTBOX1 + ulCounter),
                        MESSAGELEN, (PSZ)pszTemp))
      {
         MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, FALSE);
         return FALSE;
      }
      sResult = (SHORT) WinSendMsg(hwnd, LM_INSERTITEM,
                               MPFROMSHORT(LIT_SORTASCENDING),
                               MPFROMP((PSZ)pszTemp));

      if (sResult == LIT_ERROR || sResult == LIT_MEMERROR)
      {
         MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, FALSE);

         return FALSE;
      }
    }
    return TRUE;

}                                          /*    End of LoadListBox()       */


/*********************************************************************
 *  Name : PresParamDemoDlgProc
 *
 *  Description : Processes all messages sent to the PresParamDemo
 *                dialog.
 *
 *  Concepts : Called for each message sent to the PresParamDemo
 *             dialog box.  A switch statement branches to the
 *             routines to be performed for each message
 *             processed.  Any messages not specifically process
 *             are passed to the default window procedure
 *             WinDefDlgProc().
 *
 *  API's : WinDismissDlg
 *          WinDefDlgProc
 *
 *  Parameters   : hwnd - Window handle to which message is addressed
 *                 msg - Message type
 *                 mp1 - First message parameter
 *                 mp2 - Second message parameter
 *
 *  Returns: Dependent upon message sent
 *
 ****************************************************************/
MRESULT EXPENTRY PresParamDemoDlgProc( HWND hwnd, USHORT msg,
                                       MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
      case WM_INITDLG:
         /*
          * If initialization fails, abort the dialog
          */
         if (!InitPresParamDemoDlg(hwnd))
             WinDismissDlg(hwnd, FALSE);
         return (MRESULT)FALSE;
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case IDC_CANCEL:
            /*
             * If done, close the dialog
             */
               WinDismissDlg(hwnd, TRUE);
               break;

            case IDC_APPLY:
            /*
             * Update the dialog box
             */
               UpdatePresParamDemoDlg(hwnd);
               break;

            default:
               break;
         }
         break;

      case WM_HELP:
         ShowDlgHelp(hwnd);
         break;

      default:
         return(WinDefDlgProc(hwnd, msg, mp1, mp2));
         break;
   }
   return (MRESULT)0;
}                                      /* End of PresParamDemoDlgProc() */

/*********************************************************************
 *  Name : InitPresParamDemoDlg
 *
 *  Description : Places the colors and fonts into the
 *                PresParamDemoDlg's comboboxes.
 *
 *  Concepts : Called when the PP demo dialog is initialized.
 *             LM_INSERTITEM is sent to each combobox for the
 *             color or font string added to the combobox.
 *
 *  API's : WinLoadString
 *          WinWindowFromID
 *          WinSendMsg
 *          GpiQueryFonts
 *          WinGetPS
 *          WinReleasePS
 *          DosAllocMem
 *          DosFreeMem
 *
 *  Parameters : hwnd - Window handle of the dialog
 *
 *  Returns: TRUE if initialization is successfull, FALSE if not
 *
 ****************************************************************/
BOOL InitPresParamDemoDlg(HWND hwnd)
{
   HWND hwndTemp;
   ULONG ulCounter;
   MRESULT mresult;
   PFONTMETRICS pfm, pfmSave;
   LONG lTemp, lFonts;
   HPS hps;
   PSZ *ppszT;
   BOOL bHaveFont;

   /*
    * Load the color table
    */
   for (ulCounter = 0; ulCounter < NUMCOLORS; ulCounter++)
   {
      if (!WinLoadString(hab, (HMODULE)0, (ulCounter + IDS_FIRSTCOLOR),
             COLORNAMELEN, (PSZ)apszPresParamColor[ulCounter]))
      {
         MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, FALSE);
         return FALSE;
      }
   }
   /*
    * Load foreground color combo box
    */
   hwndTemp = WinWindowFromID(hwnd, IDC_FORECOLORLIST);

   for(ulCounter = 0; ulCounter < NUMCOLORS; ulCounter++)
   {
      mresult = WinSendMsg(hwndTemp, LM_INSERTITEM,
                          MPFROMSHORT(LIT_SORTASCENDING),
                          MPFROMP((PSZ)apszPresParamColor[ulCounter]));

      if (((SHORT)mresult) == LIT_ERROR || ((SHORT)mresult) == LIT_MEMERROR)
      {
         MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR,
              FALSE);
         return FALSE;
      }
   }

   /*
   * Load background color combo box
   */
   hwndTemp = WinWindowFromID(hwnd, IDC_BACKCOLORLIST);

   for(ulCounter = 0; ulCounter < NUMCOLORS; ulCounter++)
   {
      mresult = WinSendMsg(hwndTemp, LM_INSERTITEM,
                            MPFROMSHORT(LIT_SORTASCENDING),
                            MPFROMP((PSZ)apszPresParamColor[ulCounter]));

       if (((SHORT)mresult) == LIT_ERROR || ((SHORT)mresult) == LIT_MEMERROR)
       {
          MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR,
                FALSE);
          return FALSE;
       }
   }
   /*
    * Fonts.  Find all of the fonts of point size desired and normal
    * weight.  Put the facenames in the combo box
    */
   hwndTemp = WinWindowFromID(hwnd, IDC_FONTLIST);
   hps = WinGetPS(hwndTemp);

   /*
    * Determine the number of fonts available
    */
   lFonts = 0;
   lTemp = 0;
   lFonts = GpiQueryFonts(hps, QF_PUBLIC, NULL, &lTemp,
                  (ULONG)sizeof(FONTMETRICS), (PFONTMETRICS)NULL);

   if (lFonts == GPI_ALTERROR)
   {
       MessageBox(hwndMain, IDMSG_CANNOTLOADFONTS, MB_OK | MB_ERROR, FALSE);
       WinReleasePS(hps);
       return FALSE;
   }

   /*
    * Allocate buffer for fontmetric structures
    */
   if (DosAllocMem((PPVOID)&pfm, lFonts * (ULONG)sizeof(FONTMETRICS),
           fALLOC))
   {
      MessageBox(hwndMain, IDMSG_CANNOTALLOCATEMEMORY, MB_OK | MB_ERROR,
            FALSE);
      WinReleasePS(hps);
      return FALSE;
   }
   pfmSave = pfm;

   /*
    * Get all fonts
    */
   lTemp = GpiQueryFonts(hps, QF_PUBLIC, NULL, &lFonts,
                (ULONG)sizeof(FONTMETRICS), (PFONTMETRICS)pfm);

   WinReleasePS(hps);

   if (lTemp == GPI_ALTERROR)
   {
      MessageBox(hwndMain, IDMSG_CANNOTLOADFONTS, MB_OK | MB_ERROR, FALSE);
      return FALSE;
   }

   /*
    * Allocate buffer for array of string names, freeing the first
    * buffer if necessary
    */
   if ((BOOL)apszPresParamFont)
   {
      ppszT = apszPresParamFont;
      while ((BOOL)*ppszT)
      {
         DosFreeMem(*ppszT++);
      }

      DosFreeMem(apszPresParamFont);
   }

   if (DosAllocMem((PPVOID)&apszPresParamFont,
          (lFonts + 1) * (ULONG)sizeof(PSZ), fALLOC))
   {
      MessageBox(hwndMain, IDMSG_CANNOTALLOCATEMEMORY, MB_OK | MB_ERROR,
           FALSE);
      DosFreeMem((PVOID)pfmSave);
      apszPresParamFont = NULL;
      return FALSE;
   }

   /*
    * Initialize array to NULL
    */
   lTemp = 0;
   ppszT = apszPresParamFont;
   while (lTemp++ < lFonts)
       *ppszT++ = (PSZ)NULL;
   *ppszT = (PSZ)NULL;


   /*
    * Walk through all fonts.  If the font matches the point size
    * desired and has a weight and width class of normal (5) and
    * no special attributes (e. g. italic, underline, etc.).  If
    * it does, then add its string to the combo box
    */
   lNumFonts = 0;
   ppszT = apszPresParamFont;

   while (lFonts--)
   {
      if (pfm->sNominalPointSize == FONTPOINTSIZE &&
             pfm->usWeightClass == 5 && pfm->usWidthClass == 5 &&
             pfm->fsSelection == 0)
      {
      /*
       * Make sure we don't have this font.  If we don't,
       * then add the font to the list
       */
         lTemp = 0;
         bHaveFont = FALSE;
         while (lTemp < lNumFonts)
         {
            if (!strcmp(pfm->szFacename, apszPresParamFont[(INT)lTemp]))
            {
               bHaveFont = TRUE;
               break;
            }
            else
               lTemp++;
         }

         if (!bHaveFont)
         {
            if (DosAllocMem((PPVOID)ppszT,
                (LONG)(FACESIZE * sizeof(CHAR)), fALLOC))
            {
               MessageBox(hwndMain, IDMSG_CANNOTALLOCATEMEMORY,
                  MB_OK | MB_ERROR, FALSE);
               DosFreeMem((PVOID)pfmSave);
               return FALSE;
             }

             strcpy(*ppszT++, pfm->szFacename);
             lNumFonts++;
         }
      }
      pfm++;
   }

   /*
    * Install the name of each font into the combo box
    */
   ppszT = apszPresParamFont;
   while ((BOOL)*ppszT)
   {
      mresult = WinSendMsg(hwndTemp, LM_INSERTITEM,
                 MPFROMSHORT(LIT_SORTASCENDING), MPFROMP((PSZ)*ppszT++));

      if (((SHORT)mresult) == LIT_ERROR || ((SHORT)mresult) == LIT_MEMERROR)
      {
         MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR,
              FALSE);
         DosFreeMem((PVOID)pfmSave);
         return FALSE;
      }
   }

   /*
    * Add "DefaulTemp" text onto the end
    */
   mresult = WinSendMsg(hwndTemp, LM_INSERTITEM,
             MPFROMSHORT(LIT_SORTASCENDING),
             MPFROMP(apszPresParamColor[IDS_DEFAULT - IDS_FIRSTCOLOR]));

   if (((SHORT)mresult) == LIT_ERROR || ((SHORT)mresult) == LIT_MEMERROR)
   {
      MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR, FALSE);
      DosFreeMem((PVOID)pfmSave);
      return FALSE;
   }
   DosFreeMem((PVOID)pfmSave);
   return TRUE;
}                                      /* End of InitPresParamDemoDlg()   */

/*********************************************************************
 *  Name : UpdatePresParamDemoDlg
 *
 *  Description : Sets/Removes the Presentation Parameter of the sample
 *                text window depending upon the parameters chosen,
 *
 *  Concepts : Called when user wants to update the window.  The string
 *             of each combobox is queried and then the color or font
 *             is set depending upon the string chosen.
 *
 *  API's : WinWindowFromID
 *          WinQueryWindowText
 *          WinSendMsg
 *          WinSetPresParams
 *          WinQueryPresParams
 *          WinRemovePresParams
 *
 *  Parameters : hwnd - Handle of Presentation Params dialog
 *
 *  Returns: TRUE if update is successfull, FALSE if not
 *
 ****************************************************************/
BOOL UpdatePresParamDemoDlg(HWND hwnd)
{
   HWND hwndTemp, hwndSampleText, hwndSampleButton;
   CHAR pszTemp[COLORNAMELEN];
   BYTE abBuf[FACESIZE + PPFONTPOINTSIZE];
   ULONG ulColor, ulPresParams;
   SHORT sCounter;

   /*
    * Get window handle of the sample text box
    */
   hwndSampleText = WinWindowFromID(hwnd, IDC_SAMPLETEXT);
   hwndSampleButton = WinWindowFromID(hwnd, IDC_CHECKBOX);

   /*
    * Get the text of the foreground color combobox
   */
   hwndTemp = WinWindowFromID(hwnd, IDC_FORECOLORLIST);

   if (WinQueryWindowText(hwndTemp, COLORNAMELEN, (PSZ)pszTemp))
   {
   /*
    * Find the text in the list of color names
    */
      sCounter = 0;
      while (sCounter < NUMCOLORS)
      {
         if (!strcmp(pszTemp, apszPresParamColor[sCounter]))
         {
            ulColor = aulColor[sCounter];
            break;
         }
         else
            sCounter++;
      }

      /*
       * If color is not default, set the color.  If the default is
       * selected, then remove the color presentation parameter if
       * it exists.  If the value is not a valid color, then don't
       * don't do anything
       */
       if (sCounter < NUMCOLORS)
       {
          sCounter += IDS_FIRSTCOLOR;
          if (sCounter < IDS_DEFAULT)
          {
             if (!WinSetPresParam(hwndSampleText, PP_FOREGROUNDCOLORINDEX,
                   (ULONG)sizeof(LONG), (PVOID)&ulColor) ||
                   !WinSetPresParam(hwndSampleButton,
                   PP_FOREGROUNDCOLORINDEX, (ULONG)sizeof(LONG),
                   (PVOID)&ulColor))
             {
                MessageBox(hwndMain, IDMSG_CANNOTSETPP, MB_OK | MB_ERROR, FALSE);
             }
          }
          else
          {
          /*
           * If setting presentation parameter to the default, remove
           * the presentation parameter, but only if it has been
           * set
           */
              if (sCounter == IDS_DEFAULT &&
                   WinQueryPresParam(hwndSampleText, PP_FOREGROUNDCOLORINDEX,
                   0, &ulPresParams, (ULONG)sizeof(LONG), (PVOID)&ulColor,
                   QPF_NOINHERIT) != 0)
              {
                 if (!WinRemovePresParam(hwndSampleText,
                     PP_FOREGROUNDCOLORINDEX) ||
                     !WinRemovePresParam(hwndSampleButton,
                     PP_FOREGROUNDCOLORINDEX))
                 {
                    MessageBox(hwndMain, IDMSG_CANNOTSETPP, MB_OK | MB_ERROR,
                         FALSE);
                 }
              }
          }
       }
   }

   /*
    * Do the same for the background color combobox
    */
   hwndTemp = WinWindowFromID(hwnd, IDC_BACKCOLORLIST);
   if (WinQueryWindowText(hwndTemp, COLORNAMELEN, (PSZ)pszTemp))
   {
   /*
    * Find the text in the list of color names
   */
      sCounter = 0;
      while (sCounter < NUMCOLORS)
      {
         if (!strcmp(pszTemp, apszPresParamColor[sCounter]))
         {
            ulColor = aulColor[sCounter];
            break;
         }
         else
            sCounter++;
      }

      /*
       * If color is not default, set the color.  If the default is
       * selected, then remove the color presentation parameter.  If
       * the value is not a valid color, the don't do anything
       */
       if (sCounter < NUMCOLORS)
       {
          sCounter += IDS_FIRSTCOLOR;
          if (sCounter < IDS_DEFAULT)
          {
             if (!WinSetPresParam(hwndSampleText, PP_BACKGROUNDCOLORINDEX,
                 (ULONG)sizeof(LONG), (PVOID)&ulColor) ||
                 !WinSetPresParam(hwndSampleButton, PP_BACKGROUNDCOLORINDEX,
                 (ULONG)sizeof(LONG), (PVOID)&ulColor))
             {
                MessageBox(hwndMain, IDMSG_CANNOTSETPP, MB_OK | MB_ERROR,
                      FALSE);
             }
          }
          else
          {
             if (sCounter == IDS_DEFAULT &&
                  WinQueryPresParam(hwndSampleText,
                  PP_BACKGROUNDCOLORINDEX, 0, &ulPresParams, (ULONG)sizeof(LONG),
                  (PVOID)&ulColor, QPF_NOINHERIT) != 0)
             {
                if (!WinRemovePresParam(hwndSampleText,
                    PP_BACKGROUNDCOLORINDEX) ||
                    !WinRemovePresParam(hwndSampleButton,
                    PP_BACKGROUNDCOLORINDEX))
                {
                   MessageBox(hwndMain, IDMSG_CANNOTSETPP, MB_OK | MB_ERROR,
                         FALSE);
                }
             }
          }
       }
   }

   /*
    * Get the text of the font combobox
    */
   hwndTemp = WinWindowFromID(hwnd, IDC_FONTLIST);
   if (WinQueryWindowText(hwndTemp, FACESIZE, (PSZ)pszTemp))
   {
   /*
    * If Font selected is "Default", remove font pres. param.
    */
      if (!strcmp(pszTemp, apszPresParamColor[IDS_DEFAULT - IDS_FIRSTCOLOR]))
      {
         if (WinQueryPresParam(hwndSampleText, PP_FONTNAMESIZE, 0,
              &ulPresParams, (ULONG)(FACESIZE + PPFONTPOINTSIZE), (PVOID)abBuf,
              QPF_NOINHERIT) != 0)
         {
            if (!WinRemovePresParam(hwndSampleText, PP_FONTNAMESIZE) ||
                !WinRemovePresParam(hwndSampleButton, PP_FONTNAMESIZE))
            {
               MessageBox(hwndMain, IDMSG_CANNOTSETPP, MB_OK | MB_ERROR,
                    FALSE);
            }
         }
      }
      else
      {
      /*
       * Font is not default.
       * AbBuf will hold the font point size and name in
       * the form <pt>.<name>.  First we fill abBuf with the
       * font point prefix and then append the font name
       * retrieved from the combobox.
       */

         if (!WinLoadString(hab, (HMODULE)0, IDS_PPFONTPOINT,
              COLORNAMELEN, (PSZ)abBuf))
         {
            MessageBox(hwndMain, IDMSG_CANNOTLOADSTRING, MB_OK | MB_ERROR,
                 FALSE);
            return FALSE;
         }
         strcat((PSZ)abBuf, pszTemp);

         if (!WinSetPresParam(hwndSampleText, PP_FONTNAMESIZE,
             (ULONG)strlen((PSZ)abBuf) + 1, (PVOID)abBuf) ||
             !WinSetPresParam(hwndSampleButton, PP_FONTNAMESIZE,
             (ULONG)strlen((PSZ)abBuf) + 1, (PVOID)abBuf))
         {
            MessageBox(hwndMain, IDMSG_CANNOTSETPP, MB_OK | MB_ERROR,
                 FALSE);
         }
      }
   }
   return TRUE;
}                                     /* End of UpdatePresParamDemoDlg() */

/*********************************************************************
 *  Name : ShowDlgHelp
 *
 *  Description : Displays the help panel for the current selected
 *                item in the dialog window
 *
 *  Concepts : Called each time a WM_HELP message is posted to a
 *             dialog gets the id value of the window and determine
 *             which help panel to display.  Then sends a message
 *             to the help instance to display the panel.  If the
 *             dialog or item is not included here, then the
 *             unknown dialog or unknown item panel is displayed.
 *
 *  API's : WinQueryFocus
 *          WinQueryWindowUShort
 *          WinWindowFromID
 *
 *  Parameters : hwnd - Handle of the dialog box
 *
 *  Returns: Void
 *
 ****************************************************************/
VOID ShowDlgHelp(HWND hwnd)
{
   SHORT idPanel, idDlg, idItem;
   HWND hwndFocus;

   /*
   * Get the id of the dialog box
   */
   idDlg = WinQueryWindowUShort(hwnd, QWS_ID);

   /*
    * Finds which window has the focus and gets its id
    */
   hwndFocus = WinQueryFocus(HWND_DESKTOP);
   idItem = WinQueryWindowUShort(hwndFocus, QWS_ID);

   switch(idDlg)
   {
      case IDD_BUTTONSDLG:
         switch(idItem)
         {
            case IDC_RADIO1:
            case IDC_RADIO2:
            case IDC_RADIO3:
               idPanel = PANEL_BUTTONSDLG_RADIO;
               break;

            case IDC_CHECKBOX:
               idPanel = PANEL_BUTTONSDLG_CHECKBOX;
               break;

            case IDC_3STATE:
               idPanel = PANEL_BUTTONSDLG_THREESTATE;
               break;

            case IDC_PUSHBUTTON:
               idPanel = PANEL_BUTTONSDLG_PUSHBUTTON;
               break;

            case IDC_OK:
               idPanel = PANEL_BUTTONSDLG_OK;
               break;

            case IDC_HELP:
               idPanel = PANEL_BUTTONSDLG_HELP;
               break;

            default:
               idPanel = PANEL_UNKNOWN;
               break;
         }
         break;

      case IDD_LISTBOXDLG:
         switch(idItem)
         {
            case IDC_LISTBOX:
               idPanel = PANEL_LISTBOXDLG_SINGLE;
               break;

            case IDC_MULTISELLISTBOX:
               idPanel = PANEL_LISTBOXDLG_MULTIPLE;
               break;

            case IDC_OK:
               idPanel = PANEL_LISTBOXDLG_OK;
               break;

            case IDC_HELP:
               idPanel = PANEL_LISTBOXDLG_HELP;
               break;

            default:
               idPanel = PANEL_UNKNOWN;
               break;
         }
         break;

      case IDD_COMBOBOXDLG:
         switch(idItem)
         {
            case IDC_SIMPLE:
               idPanel = PANEL_COMBOBOXDLG_SIMPLE;
               break;

            case IDC_DROPDOWN:
               idPanel = PANEL_COMBOBOXDLG_DROPDOWN;
               break;

            case IDC_DROPDOWNLIST:
               idPanel = PANEL_COMBOBOXDLG_DROPDOWNLIST;
               break;

            case IDC_OK:
               idPanel = PANEL_COMBOBOXDLG_OK;
               break;

            case IDC_HELP:
               idPanel = PANEL_COMBOBOXDLG_HELP;
               break;

            default:
               /*
                * Check to see if window that has the focus is the
                * entry field of the combobox.  If it is, then
                * call the appropriate combobox help panel.  If
                * not, then call the unknown panel
                */
               if (WinWindowFromID(WinWindowFromID(hwnd, IDC_SIMPLE),
                   CBID_EDIT) == hwndFocus)
               {
                  idPanel = PANEL_COMBOBOXDLG_SIMPLE;
               }
               else
                  if (WinWindowFromID(WinWindowFromID(hwnd, IDC_DROPDOWN),
                     CBID_EDIT) == hwndFocus)
                  {
                       idPanel = PANEL_COMBOBOXDLG_DROPDOWN;
                  }
                  else
                       if (WinWindowFromID(
                           WinWindowFromID(hwnd, IDC_DROPDOWNLIST),
                           CBID_EDIT) == hwndFocus)
                       {
                          idPanel = PANEL_COMBOBOXDLG_DROPDOWNLIST;
                       }
                       else
                          idPanel = PANEL_UNKNOWN;
              break;
         }
         break;

      case IDD_ENTRYFIELDDLG:
         switch(idItem)
         {
            case IDC_ENTRY:
               idPanel = PANEL_ENTRYFIELDDLG_ENTRY;
               break;

            case IDC_MLE:
               idPanel = PANEL_ENTRYFIELDDLG_MLE;
               break;

            case IDC_OK:
               idPanel = PANEL_ENTRYFIELDDLG_OK;
               break;

            case IDC_HELP:
               idPanel = PANEL_ENTRYFIELDDLG_HELP;
               break;

            default:
               idPanel = PANEL_UNKNOWN;
               break;
         }
         break;

      case IDD_STATICDLG:
         switch(idItem)
         {
            case IDC_OK:
               idPanel = PANEL_STATICDLG_OK;
               break;

            case IDC_HELP:
               idPanel = PANEL_STATICDLG_HELP;
               break;

            default:
               idPanel = PANEL_UNKNOWN;
               break;
         }
         break;

      case IDD_PPDEMODLG:
         switch(idItem)
         {
            case IDC_FORECOLORLIST:
               idPanel = PANEL_PPDEMODLG_FORECOLORLIST;
               break;

            case IDC_BACKCOLORLIST:
               idPanel = PANEL_PPDEMODLG_BACKCOLORLIST;
               break;

            case IDC_FONTLIST:
               idPanel = PANEL_PPDEMODLG_FONTLIST;
               break;

            case IDC_CHECKBOX:
               idPanel = PANEL_PPDEMODLG_TESTBUTTON;
               break;

            case IDC_CANCEL:
               idPanel = PANEL_PPDEMODLG_CANCEL;
               break;

            case IDC_APPLY:
               idPanel = PANEL_PPDEMODLG_APPLY;
               break;

            case IDC_HELP:
               idPanel = PANEL_PPDEMODLG_HELP;
               break;

            default:
            /*
             * Check to see if window that has the focus is the
             * entry field of the combobox.  If it is, then
             * call the appropriate combobox help panel.  If
             * not, then call the unknown panel
             */
               if (WinWindowFromID(
                   WinWindowFromID(hwnd, IDC_FORECOLORLIST),
                   CBID_EDIT) == hwndFocus)
               {
                  idPanel = PANEL_PPDEMODLG_FORECOLORLIST;
               }
               else
                  if (WinWindowFromID(
                      WinWindowFromID(hwnd, IDC_BACKCOLORLIST),
                      CBID_EDIT) == hwndFocus)
                  {
                     idPanel = PANEL_PPDEMODLG_BACKCOLORLIST;
                  }
                  else
                     if(WinWindowFromID(
                        WinWindowFromID(hwnd, IDC_FONTLIST),
                        CBID_EDIT) == hwndFocus)
                     {
                        idPanel = PANEL_PPDEMODLG_FONTLIST;
                     }
                     else
                        idPanel = PANEL_UNKNOWN;
               break;
         }
         break;

      case IDD_CONTAINERDLG:
         switch(idItem)
         {
            case IDC_CONTAINER:
               idPanel = PANEL_CONTAINERDLG_CONTAINER;
               break;

            case IDC_OK:
               idPanel = PANEL_CONTAINERDLG_OK;
               break;

            case IDC_CANCEL:
               idPanel = PANEL_CONTAINERDLG_CANCEL;
               break;

            case IDC_HELP:
               idPanel = PANEL_CONTAINERDLG_HELP;
               break;

            default:
               idPanel = PANEL_UNKNOWN;
               break;
         }
         break;
      case IDD_NOTEBOOKDLG:
         switch(idItem)
         {
            case IDC_NOTEBOOK:
               idPanel = PANEL_NOTEBOOKDLG_NOTEBOOK;
               break;

            case IDC_OK:
               idPanel = PANEL_NOTEBOOKDLG_OK;
               break;

            case IDC_CANCEL:
               idPanel = PANEL_NOTEBOOKDLG_CANCEL;
               break;

            case IDC_HELP:
               idPanel = PANEL_NOTEBOOKDLG_HELP;
               break;

            default:
               idPanel = PANEL_UNKNOWN;
               break;
        }
        break;
      case IDD_SLIDERDLG:
         switch(idItem)
         {
            case IDC_VSLIDER:
               idPanel = PANEL_SLIDERDLG_VSLIDER;
               break;

            case IDC_HSLIDER:
               idPanel = PANEL_SLIDERDLG_HSLIDER;
               break;

            case IDC_OK:
               idPanel = PANEL_SLIDERDLG_OK;
               break;

            case IDC_CANCEL:
               idPanel = PANEL_SLIDERDLG_CANCEL;
               break;

            case IDC_HELP:
               idPanel = PANEL_SLIDERDLG_HELP;
               break;

            default:
               idPanel = PANEL_UNKNOWN;
               break;
         }
         break;
      case IDD_VALUESETDLG:
         switch(idItem)
         {
            case IDC_VALUESET:
               idPanel = PANEL_VALUESETDLG_VALUESET;
               break;

            case IDC_OK:
               idPanel = PANEL_VALUESETDLG_OK;
               break;

            case IDC_CANCEL:
               idPanel = PANEL_VALUESETDLG_CANCEL;
               break;

            case IDC_HELP:
               idPanel = PANEL_VALUESETDLG_HELP;
               break;

            default:
               idPanel = PANEL_UNKNOWN;
               break;
         }
         break;
      case IDD_SPINBUTTONDLG:
         switch(idItem)
         {
            case IDC_MASTER:
               idPanel = PANEL_SPINBUTTONDLG_MASTER;
               break;

            case IDC_SERVANT:
               idPanel = PANEL_SPINBUTTONDLG_SERVANT;
               break;

            case IDC_OK:
               idPanel = PANEL_SPINBUTTONDLG_OK;
               break;

            case IDC_CANCEL:
               idPanel = PANEL_SPINBUTTONDLG_CANCEL;
               break;

            case IDC_HELP:
               idPanel = PANEL_SPINBUTTONDLG_HELP;
               break;

            default:
               idPanel = PANEL_UNKNOWN;
               break;
         }
         break;

      case IDD_PRODINFO:
         idPanel = PANEL_ABOUTBOX;
         break;

      default:
         idPanel = PANEL_UNKNOWNDLG;
         break;
   }
   DisplayHelpPanel(idPanel);
}                                       /* End of ShowDlgHelp()        */

/*********************************************************************
 *  Name : InitializeValueSet
 *
 *  Description : Loads the demo Value Set with 4 different icons
 *                and with 4 colors derived form the color index.
 *
 *  Concepts : Called each time a demo Value Set is initialized A
 *             for 1 to 4 loop sets an Icon in each column of the
 *             First row of the Value Set via a VM_SETITEM
 *             message.  The Second row is set to a color by
 *             sending the VM_SETITEMATTR message.
 *
 *  API's : WinSendDlgItemMsg
 *
 *  Parameters : hwnd - Handle of the Valueset dialog.
 *
 *  Returns: TRUE if Value Set is loaded successfully, FALSE otherwise
 *
 ****************************************************************/
BOOL InitializeValueSet(HWND hwnd)
{
   USHORT usIndex;
   for (usIndex = 1; usIndex <= 4;usIndex++)
   {
     if ( !WinSendDlgItemMsg(hwnd, IDC_VALUESET, VM_SETITEM,
           MPFROM2SHORT(1,usIndex),
           MPFROMLONG( WinLoadPointer(HWND_DESKTOP, 0,
           (IDR_MAIN + usIndex)))))
         return FALSE;

     if ( !WinSendDlgItemMsg( hwnd,IDC_VALUESET, VM_SETITEMATTR,
          MPFROM2SHORT(2,usIndex),
          MPFROM2SHORT( VIA_COLORINDEX, TRUE )))
        return FALSE;

     if ( !WinSendDlgItemMsg(hwnd, IDC_VALUESET, VM_SETITEM,
          MPFROM2SHORT(2,usIndex),
          MPFROMSHORT(usIndex)))
        return FALSE;
   }
   return TRUE;
}                                       /* End of InitializeValueset    */
/*********************************************************************
 *  Name : InitializeSlider
 *
 *  Description :  Set the Sliders Tick size and Scale Text.
 *
 *  Concepts : Called each time a demo Slider controls ar initialized
 *             Ses the he initail value of the sliders output display
 *             to 0. A for 0 to 10 for loop sets the ruler text and
 *             tick size for both the Horizontal and Vertical Sliders
 *             via SLM_SETSCALETEXT and SLM_SETTICKSIZE message.
 *
 *  API's : WinSendDlgItemMsg
 *
 *  Parameters : hwnd - Handle of the Slider dialog.
 *
 *  Returns: TRUE if Sliders are initialized successfully, FALSE otherwise
 *
 *******************************************************************/
BOOL InitializeSlider(HWND hwnd)
{
   USHORT usIndex;
   CHAR   acBuffer[4];
   CHAR   *cData;


   cData = _ltoa(0,acBuffer,10);
   WinSetDlgItemText(hwnd,IDC_HSLIDERDATA, cData);
   WinSetDlgItemText(hwnd,IDC_VSLIDERDATA, cData);
   for (usIndex = 0;usIndex < 10 ;usIndex ++ )
   {
      _itoa(usIndex,acBuffer,10);
      if ( !WinSendDlgItemMsg(hwnd, IDC_HSLIDER, SLM_SETTICKSIZE,
               MPFROM2SHORT(usIndex, 5), NULL))
        return FALSE;

      if ( !WinSendDlgItemMsg(hwnd, IDC_HSLIDER, SLM_SETSCALETEXT,
               MPFROMSHORT(usIndex), MPFROMP(acBuffer)))
        return FALSE;

      if ( !WinSendDlgItemMsg(hwnd, IDC_VSLIDER, SLM_SETTICKSIZE,
               MPFROM2SHORT(usIndex, 5), NULL))
        return FALSE;

      if ( !WinSendDlgItemMsg(hwnd, IDC_VSLIDER, SLM_SETSCALETEXT,
                 MPFROMSHORT(usIndex), MPFROMP(acBuffer)))
        return FALSE;
   }
   return TRUE;
}                                       /* End of InitializeSlider      */

/*********************************************************************
 *  Name : InitializeSpinButton
 *
 *  Description : Set the intital values for the spin button controls.
 *
 *  Concepts : Sends a SPBM_SETARRAY to the Master Spin Button to set
 *             the values (text strings) available to the control.
 *             Sets the initial value to the array index of 0 by
 *             sending a SPBM_SETCURRENTVALUE.  The Servant Spin Button
 *             is initialized to the range 0 to 100 by sending a
 *             SPBM_SETLIMITS message.  Sends a SPBM_SETMASTER message
 *             to set the MASTER/SERVANT relationship between the two
 *             spin buttons.
 *
 *  API's : WinSendDlgItemMsg
 *          WinWindowFromID
 *
 *  Parameters : hwnd - Handle of the Spinnbutton dialog.
 *
 *  Returns: TRUE if Spinbuttons are initialized successfully, FALSE otherwise
 *
 ****************************************************************/
BOOL InitializeSpinButton(HWND hwnd)
{
   static PSZ   spinColors[] = {"White", "Black", "Blue", "Red"," Pink",
                     "Green", "Cyan", "Yellow", "Dark Grey", "Dark Blue",
                     "Dark Red", "Dark Pink", "Dark Green", "Dark Cyan",
                     "Brown", "Pale Grey"};

   if ( !WinSendDlgItemMsg( hwnd, IDC_MASTER, SPBM_SETARRAY, spinColors,
             MPFROMLONG(NUMSPINCOLORS)))
      return FALSE;

   if ( !WinSendDlgItemMsg( hwnd, IDC_MASTER, SPBM_SETCURRENTVALUE,
             MPFROMLONG(0), NULL))
      return FALSE;

   if ( !WinSendDlgItemMsg( hwnd, IDC_SERVANT, SPBM_SETLIMITS,
             MPFROMLONG(100), MPFROMLONG(0)))
      return FALSE;

   if ( !WinSendDlgItemMsg( hwnd, IDC_SERVANT, SPBM_SETCURRENTVALUE,
             MPFROMLONG(0), NULL))
      return FALSE;

   if (!WinSendDlgItemMsg(
           hwnd,
           IDC_SERVANT,
           SPBM_SETMASTER,
           MPFROMHWND( WinWindowFromID(hwnd, IDC_MASTER)),
           MPVOID)
      )
      return FALSE;
   return TRUE;
}                                       /* End of InitializeSpinbutton  */

/*********************************************************************
 *  Name : InitializeNotebook
 *
 *  Description : Set the intital pages for a notebook control.
 *
 *  Concepts : Called each time a demo NoteBook Control is
 *             initialized Sends a BKM_INSERTPAGE message to the
 *             notebook control to insert the first page into the
 *             control.  Sets the status line text for the first page
 *             by sending a BKM_SETSTATUSLINETEXT message.  Creates a
 *             window containing a bitmap and associates the whidow
 *             with the notebook by sending a BKM_SETPAGEWINDOWHWND
 *             message.  Repeats these steps with the execption of
 *             createing a MLE for use in the second notebook page.
 *
 *  API's : WinSendDlgItemMsg
 *          WinCreateWindow
 *          WinSendMsg
 *
 *  Parameters :  hwnd - Handle of the Notebook dialog
 *
 *  Returns: TRUE if The NoteBook is initialized successfully, FALSE otherwise
 *
 ****************************************************************/
BOOL InitializeNoteBook(HWND hwnd)
{
   HWND hwndPage;
   ULONG ulPageId;
   ULONG ipt = 0;
   CHAR pszMleBuffer[512];
   PSZ   pszNoteBookText =
   {"Applications written for the Presentation Manager have full\
    access to the complete set of user interface tools: menus,\
    icons, scroll bars, etc., and often present a WYSIWYG\
    (what-you-see-is-what-you-get) view of their data.\
    Presentation Manager programs often make extensive use\
    of a mouse and display, and they have access to all the\
    32-bit features of version 2.0."};

   /*
    * Insert the first page.
    */
   ulPageId = (LONG)WinSendDlgItemMsg(hwnd, IDC_NOTEBOOK,
        BKM_INSERTPAGE, NULL,
        MPFROM2SHORT((BKA_STATUSTEXTON | BKA_AUTOPAGESIZE | BKA_MAJOR),
        BKA_LAST));

   if ( !ulPageId)
     return FALSE;

   if ( !WinSendDlgItemMsg(hwnd, IDC_NOTEBOOK,
        BKM_SETSTATUSLINETEXT, MPFROMLONG(ulPageId),
        MPFROMP("Page 1 of 2")))
     return FALSE;

   if ( !WinSendDlgItemMsg(hwnd, IDC_NOTEBOOK,
        BKM_SETTABTEXT, MPFROMLONG(ulPageId),
        MPFROMP("~1")))
     return FALSE;

   hwndPage =
   WinCreateWindow(               /* parent-window handle                    */
      hwnd,                       /* pointer to registered class name        */
      WC_STATIC,                  /* pointer to window text                  */
      "#6",                       /* window style                            */
      WS_VISIBLE | SS_BITMAP,     /* horizontal position of window           */
      0,                          /* vertical position of window             */
      0,                          /* window width                            */
      0,                          /* window height                           */
      0,                          /* owner-window handle                     */
      NULLHANDLE,                 /* handle to sibling window                */
      HWND_TOP,                   /* window identifier                       */
      0,                          /* pointer to buffer                       */
      NULL,                       /* pointer to structure with pres. params. */
      NULL);

   if (!hwndPage)
     return FALSE;

   if ( !WinSendDlgItemMsg(hwnd, IDC_NOTEBOOK,
         BKM_SETPAGEWINDOWHWND, MPFROMLONG(ulPageId),
         MPFROMHWND(hwndPage)))
     return FALSE;

   /*
    * Insert the second page.
    */
    ulPageId = (LONG)WinSendDlgItemMsg(hwnd, IDC_NOTEBOOK,
         BKM_INSERTPAGE, NULL,
         MPFROM2SHORT((BKA_STATUSTEXTON | BKA_AUTOPAGESIZE | BKA_MAJOR),
         BKA_LAST));

   if (!ulPageId)
     return FALSE;

   if ( !WinSendDlgItemMsg(hwnd, IDC_NOTEBOOK,
         BKM_SETSTATUSLINETEXT, MPFROMLONG(ulPageId),
         MPFROMP("Page 2 of 2")))
     return FALSE;

   if ( !WinSendDlgItemMsg(hwnd, IDC_NOTEBOOK,
        BKM_SETTABTEXT, MPFROMLONG(ulPageId),
        MPFROMP("~2")))
     return FALSE;

   hwndPage =
   WinCreateWindow(
      hwnd,                       /* parent-window handle                    */
      WC_MLE,                     /* pointer to registered class name        */
      NULL,                       /* pointer to window text                  */
      WS_VISIBLE | MLS_WORDWRAP | /* window style                            */
         MLS_READONLY,
      0,                          /* horizontal position of window           */
      0,                          /* vertical position of window             */
      0,                          /* window width                            */
      0,                          /* window height                           */
      NULLHANDLE,                 /* owner-window handle                     */
      HWND_TOP,                   /* handle to sibling window                */
      0,                          /* window identifier                       */
      NULL,                       /* pointer to buffer                       */
      NULL);                      /* pointer to structure with pres. params. */

   if (!hwndPage)
     return FALSE;

   if ( !WinSendMsg(hwndPage, MLM_SETIMPORTEXPORT,
         MPFROMP(pszMleBuffer),
         MPFROMSHORT(sizeof(pszMleBuffer))))
     return FALSE;

     memset(pszMleBuffer,'\0',sizeof(pszMleBuffer));
     strcpy(pszMleBuffer, pszNoteBookText);

   if ( !WinSendMsg(hwndPage, MLM_IMPORT, &ipt,
         MPFROMSHORT(sizeof(pszMleBuffer))))
     return FALSE;

   if( !WinSendDlgItemMsg(hwnd, IDC_NOTEBOOK,
         BKM_SETPAGEWINDOWHWND, MPFROMLONG(ulPageId),
         MPFROMHWND(hwndPage)))
     return FALSE;

   return TRUE;
}                                       /* End of InitializeNotebook    */

/*********************************************************************
 *  Name : InitializeContainer
 *
 *  Description : Initialize and insert a record into the container.
 *
 *
 *  Concepts : Called each time a demo Container Control is initialized.
 *             Allocates and inserts a user record into a container.
 *             Allocates and inserst  details view information.
 *             Sets the containers info.
 *
 *  API's : WinLoadPointer
 *          WinSendDlgItemMsg
 *
 *  Parameters : hwnd - Handle of the container dialog.
 *
 *  Returns: TRUE if The Container is initialized successfully, FALSE otherwise
 *
 ****************************************************************/
BOOL InitializeContainer(HWND hwnd)
{

  USHORT            nRecords = 1;
  HPOINTER          hptr;
  ULONG             cbRecordData;

  hptr = WinLoadPointer(HWND_DESKTOP, 0, IDR_MAIN);

  cbRecordData = (LONG) (sizeof(USERRECORD) - sizeof(RECORDCORE));
  pUserRecord = WinSendDlgItemMsg(hwnd,IDC_CONTAINER,
                   CM_ALLOCRECORD, MPFROMLONG(cbRecordData) ,
                   MPFROMSHORT(nRecords));

  pUserRecord->recordCore.cb = sizeof(RECORDCORE);              /*RBS*/
  pUserRecord->recordCore.pszText = pszViewText;
  pUserRecord->recordCore.pszIcon = pszViewIcon;
  pUserRecord->recordCore.pszName = pszViewName;
  pUserRecord->recordCore.hptrIcon = hptr;

  pUserRecord->date.day = 11;
  pUserRecord->date.month = 11;
  pUserRecord->date.year = 11;
  pUserRecord->time.seconds    = 12;
  pUserRecord->time.minutes    = 12;
  pUserRecord->time.hours      = 12;
  pUserRecord->recordData = (PSZ)pszSampleData;

  recordInsert.cb = sizeof(RECORDINSERT);                   /*RBS*/
  recordInsert.pRecordParent= NULL;
  recordInsert.pRecordOrder = (PRECORDCORE)CMA_END;
  recordInsert.zOrder = CMA_TOP;
  recordInsert.cRecordsInsert = 1;
  recordInsert.fInvalidateRecord = TRUE;

  WinSendDlgItemMsg(hwnd,IDC_CONTAINER, CM_INSERTRECORD,
                   (PRECORDCORE)pUserRecord, &recordInsert);

  pFieldInfo = WinSendDlgItemMsg(hwnd, IDC_CONTAINER,
                        CM_ALLOCDETAILFIELDINFO,MPFROMLONG(3), NULL);

  firstFieldInfo = pFieldInfo;

  pFieldInfo->cb = sizeof(FIELDINFO);                       /*RBS*/
  pFieldInfo->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_CENTER |
                         CFA_SEPARATOR;
  pFieldInfo->flTitle = CFA_CENTER;
  pFieldInfo->pTitleData = (PVOID) pszColumnText1;
  pFieldInfo->offStruct = FIELDOFFSET(USERRECORD,recordData);
  pFieldInfo = pFieldInfo->pNextFieldInfo;

  pFieldInfo->cb = sizeof(FIELDINFO);                       /*RBS*/
  pFieldInfo->flData = CFA_DATE | CFA_HORZSEPARATOR | CFA_CENTER |
                         CFA_SEPARATOR;
  pFieldInfo->flTitle = CFA_CENTER;
  pFieldInfo->pTitleData =  (PVOID) pszColumnText2;
  pFieldInfo->offStruct = FIELDOFFSET(USERRECORD,date);
  pFieldInfo = pFieldInfo->pNextFieldInfo;

  pFieldInfo->cb = sizeof(FIELDINFO);                       /*RBS*/
  pFieldInfo->flData = CFA_TIME | CFA_HORZSEPARATOR | CFA_CENTER |
                         CFA_SEPARATOR;
  pFieldInfo->flTitle = CFA_CENTER;
  pFieldInfo->pTitleData = (PVOID) pszColumnText3;
  pFieldInfo->offStruct = FIELDOFFSET(USERRECORD,time);


  fieldInfoInsert.cb = (ULONG)(sizeof(FIELDINFOINSERT));    /*RBS*/
  fieldInfoInsert.pFieldInfoOrder = (PFIELDINFO)CMA_FIRST;
  fieldInfoInsert.cFieldInfoInsert = 3;
  fieldInfoInsert.fInvalidateFieldInfo = TRUE;              /*RBS*/

  pFieldInfoInsert = &fieldInfoInsert;

  WinSendDlgItemMsg(hwnd,IDC_CONTAINER, CM_INSERTDETAILFIELDINFO,
                    MPFROMP(firstFieldInfo),
                    MPFROMP(pFieldInfoInsert));

  WinSendDlgItemMsg(hwnd,IDC_CONTAINER,CM_SETCNRINFO,&cnrinfo,
                     MPFROMLONG(CMA_FLWINDOWATTR | CMA_CNRTITLE));

return TRUE;
}                                       /* End of InitializeContainer    */
/***********************************************************
 * Name         : SetSysMenu
 *
 * Description  : Procedure to remove unselectable items from
 *                the window system menu.
 *
 * Concepts     : This routine determines the number of items
 *                in the system menu, loops through those
 *                items removing disabled menu items and menu
 *                seperators.
 *
 * API's        : WinMessageBox
 *
 * Parameters   : None
 *
 * Return       : None
 *
 **************************************************************/
VOID SetSysMenu(HWND hwndDlg)
{
  SHORT sMenuItems;
  USHORT usItemid;
  MENUITEM menuItem;
  HWND  hwndSubMenu;

  /* Determine the definition of the system menu */
  WinSendDlgItemMsg(hwndDlg, FID_SYSMENU, MM_QUERYITEM,
                    MPFROM2SHORT((SHORT)SC_SYSMENU,FALSE),
                    MPFROMP(&menuItem));

  hwndSubMenu = menuItem.hwndSubMenu;

  /* Find the number of items in the in the submenu */
  sMenuItems = (SHORT)WinSendMsg(hwndSubMenu,
                                 MM_QUERYITEMCOUNT,NULL,NULL);
  /*
   * Loop through the submenu items and remove disabled
   * menu items and menu separators.
   */
  for (sMenuItems - 1 ; sMenuItems >= 0 ;sMenuItems-- )
  {
    /* Find the item ID for the current position. */
    usItemid = (USHORT)WinSendMsg(hwndSubMenu,MM_ITEMIDFROMPOSITION,
                         MPFROMSHORT(sMenuItems), NULL);

    /* Query the definition of the current item*/
    WinSendMsg(hwndSubMenu, MM_QUERYITEM,
               MPFROM2SHORT(usItemid,FALSE), MPFROMP(&menuItem));

   /*
    * If the menu item is disabled or the item has a style
    * of MIS_SEPARATOR - delete it.
    */
    if ((BOOL)(WinSendMsg(hwndSubMenu, MM_QUERYITEMATTR,
          MPFROM2SHORT(usItemid,FALSE), (MPARAM)MIA_DISABLED)) ||
          (menuItem.afStyle & MIS_SEPARATOR))
    {
      WinSendMsg(hwndSubMenu, MM_DELETEITEM,
                MPFROM2SHORT(usItemid,FALSE),NULL);
    }
  }
}                                       /* End of SetSysMenu()           */
