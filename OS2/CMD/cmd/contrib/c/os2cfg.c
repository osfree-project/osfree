// OS2CFG.C - 4OS2 configuration notebooks
// Copyright 1996 - 1997, JP Software Inc., All Rights Reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "4all.h"
#include "inifile.h"

char FMT_LONG[] = "%d";
char FMT_ULONG[] = "%u";
#define FMT_INT FMT_LONG
#define FMT_UINT FMT_ULONG
char NULLSTR[] = "";
char INI_SECTION_NAME[] = "4OS2";

char WinColorDefault[] = "(Default)";

char *WinColors[] = {
    "Black",
    "Blue",
    "Green",
    "Cyan",
    "Red",
    "Magenta",
    "Yellow",
    "White",
    "Bright Black",
    "Bright Blue",
    "Bright Green",
    "Bright Cyan",
    "Bright Red",
    "Bright Magenta",
    "Bright Yellow",
    "Bright White"
};

int AddQuotes( char * );
char * strins( char *, char * );
extern int _help(char *, char *);


// INI file control data, used to find data in INIItemList
typedef struct {
    unsigned short usINIItemNum;            // index into INIItemList
    unsigned short fINIUpdate;                      // type of update if data has been changed
} IDI_CONTROL;

#define CONTROLDATASIZE ((IDI_ID_MAX - IDI_BASE + 1) * sizeof(IDI_CONTROL))

MRESULT APIENTRY wpConfigDlg(HWND, ULONG, MPARAM, MPARAM);

// Define configuration notebook pages and header
NBPAGE ConfigNBPages[] = {
    //        Procedure     Status line      Tab text           Dialog box ID          ID    First control       Last control      Focus control       Parent  Tab type    Help name
    //        -----------   --------------   ---------------    -----------------      ---   -----------------   ---------------   ------------------  ------  --------    ---------
    { wpConfigDlg,  "",              "Startup",         CONFIG_STARTUPDLGBOX,  0L,   IDI_STARTUP_START,  IDI_STARTUP_END,  IDI_STARTUP_FOCUS,  FALSE,  BKA_MAJOR,  "\"Startup Options Page\""},
    { wpConfigDlg,  "",              "Display",         CONFIG_DISPLAYDLGBOX,  0L,   IDI_DISPLAY_START,  IDI_DISPLAY_END,  IDI_DISPLAY_FOCUS,  FALSE,  BKA_MAJOR,  "\"Display Options Page\""},
    { wpConfigDlg,  "",              "Command Line 1",  CONFIG_CMDLINEDLGBOX,  0L,   IDI_CMDLINE_START,  IDI_CMDLINE_END,  IDI_CMDLINE_FOCUS,  FALSE,  BKA_MAJOR,  "\"Command Line 1 Options Page\""},
    { wpConfigDlg,  "",              "Command Line 2",  CONFIG_CMDLINE2DLGBOX, 0L,   IDI_CMDLINE2_START, IDI_CMDLINE2_END, IDI_CMDLINE2_FOCUS, FALSE,  BKA_MAJOR,  "\"Command Line 2 Options Page\""},
    { wpConfigDlg,  "",              "Options 1",       CONFIG_OPT1DLGBOX,     0L,   IDI_CONFIG1_START,  IDI_CONFIG1_END,  IDI_CONFIG1_FOCUS,  FALSE,  BKA_MAJOR,  "\"Options 1 Page\""},
    { wpConfigDlg,  "",              "Options 2",       CONFIG_OPT2DLGBOX,     0L,   IDI_CONFIG2_START,  IDI_CONFIG2_END,  IDI_CONFIG2_FOCUS,  FALSE,  BKA_MAJOR,  "\"Options 2 Page\""},
    { wpConfigDlg,  "",              "Commands",        CONFIG_CMDDLGBOX,      0L,   IDI_CMD_START,      IDI_CMD_END,      IDI_CMD_FOCUS,      FALSE,  BKA_MAJOR,  "\"Command Options Page\""},
};

IDI_CONTROL *gpINIControlData;

#define CONFIG_PAGE_CNT (sizeof(ConfigNBPages) / sizeof(NBPAGE))
NBHDR ConfigNBHdr = {IDI_NOTEBOOK, CONFIG_PAGE_CNT, ConfigNBPages, (HWND)0, (HWND)0};

extern INI_ITEM gaINIItemList[];
extern unsigned int guINIItemCount;

void ConfigNotebook(void)
{
    extern HWND hParentFrame;

    static int fControlDataActive = 0;
    unsigned int i, nIDIOffsetValue;
    UINT nCtrlSize, nBitByte;

    // Allocate space for the map array to map dialog IDs to entries in
    // the INI file item list, and initialize the array
    if ( fControlDataActive == 0 ) {
        nCtrlSize = CONTROLDATASIZE;
        gpINIControlData = (IDI_CONTROL *)AllocMem( &nCtrlSize );
        memset( gpINIControlData, (char)0xFF, CONTROLDATASIZE );
        for ( i = 0; ( i < guINIItemCount ); i++ ) {
            if ( (nIDIOffsetValue = (int)gaINIItemList[i].uControlID) > 0 ) {
                gpINIControlData[nIDIOffsetValue].usINIItemNum = (USHORT)i;
                gpINIControlData[nIDIOffsetValue].fINIUpdate = 0;

                // Set the update flag if this value was previously modified
                nBitByte = nIDIOffsetValue / 8;
                if ( (nBitByte < gpIniptr->OBCount) && ((gpIniptr->OptBits[nBitByte] & (1 << (nIDIOffsetValue % 8))) != 0) )
                    gpINIControlData[nIDIOffsetValue].fINIUpdate = 1;
            }
        }
        fControlDataActive = 1;
    }

    WinDlgBox( HWND_DESKTOP, hParentFrame, NotebookDlgProc, 0L, CONFIG_NOTEBOOKDLGBOX, (PVOID *)&ConfigNBHdr );
}


// Dialog procedure for the configuration dialog pages
MRESULT APIENTRY wpConfigDlg(HWND hDlg, ULONG uMsg, MPARAM mp1, MPARAM mp2 )
{
    unsigned int i, nIDIValue;
    int nCtlType, nRadioCnt, nIntDataValue, nIntNewValue;
    int nOldFG, nOldBG, nNewFG, nNewBG, fRewriteINI = FALSE;
    unsigned short usINIItemNum;
    unsigned int fStringChanged;
    unsigned short *pfUpdate;
    unsigned int uIntDataValue, nPage;
    ULONG ulCurrentPage;
    void *pDataPtr;
    TL_HEADER *pValid;
    char szBuffer[512];
    HWND hwndNB, hwndPage;
    PNBPAGE pThisPage;
    UINT nBitByte, nIDIOffsetValue;

    switch ( uMsg ) {
        case WM_INITDLG:

            // Figure out which page we are on, and the range of IDs for this page
            hwndNB = (HWND)(*(PULONG)(PVOIDFROMMP(mp2)));
            ulCurrentPage = (ULONG) WinSendMsg(hwndNB, BKM_QUERYPAGEID, NULL, MPFROM2SHORT(BKA_TOP, 0));
            pThisPage = (PNBPAGE) WinSendMsg(hwndNB, BKM_QUERYPAGEDATA, MPFROMLONG(ulCurrentPage), NULL);

            // 20090513 AB set correct text for IDI_POPUPWINTOPLABEL depending on PopupWinPosRelative
            if ( gaInifile.PopupPosRelative ) {
                WinSetDlgItemText(hDlg, IDI_POPUPWINTOPLABEL, IDI_POPUPWINTOPLABEL_ABOVE_TEXT);
            } else {
                WinSetDlgItemText(hDlg, IDI_POPUPWINTOPLABEL, IDI_POPUPWINTOPLABEL_TOP_TEXT);
            }

            // Initialize the data in the controls for this dialog
            for ( nIDIValue = (int)(pThisPage->idFirst); nIDIValue <= (int)(pThisPage->idLast); nIDIValue++ ) {

                // Get the item number, initialize the item if it is in use
                usINIItemNum = gpINIControlData[nIDIValue - IDI_BASE].usINIItemNum;
                if ( usINIItemNum != (USHORT)-1 ) {

                    nCtlType = (int)gaINIItemList[usINIItemNum].cControlType;
                    nRadioCnt = (int)gaINIItemList[usINIItemNum].cRadioCnt;
                    if ( (pDataPtr = gaINIItemList[usINIItemNum].pItemData) != NULL ) {
                        nIntDataValue = *((int *)pDataPtr);
                        uIntDataValue = *((unsigned int *)pDataPtr);
                    }

                    // initialize the control
                    switch ( nCtlType ) {
                        case INI_CTL_INT:
                        case INI_CTL_UINT:
                            WinSendDlgItemMsg(hDlg, nIDIValue, EM_SETTEXTLIMIT, MPFROMSHORT(8), MP0);
                            sprintf(szBuffer, ((nCtlType == INI_CTL_INT) ? FMT_LONG : FMT_ULONG), nIntDataValue);
                            WinSetDlgItemText(hDlg, nIDIValue, szBuffer);
                            break;

                        case INI_CTL_TEXT:
                            // if it's a character make it a string first
                            if ( (gaINIItemList[usINIItemNum].cParseType & INI_PTMASK) == INI_CHAR ) {
                                szBuffer[0] = *((char *)pDataPtr);
                                szBuffer[1] = '\0';
                                WinSendDlgItemMsg(hDlg, nIDIValue, EM_SETTEXTLIMIT, MPFROMSHORT(1), MP0);
                                WinSetDlgItemText(hDlg, nIDIValue, szBuffer);
                            } else {
                                WinSendDlgItemMsg(hDlg, nIDIValue, EM_SETTEXTLIMIT, MPFROMSHORT(511), MP0);
                                if ( (*(unsigned int *)pDataPtr) != INI_EMPTYSTR )
                                    WinSetDlgItemText(hDlg, nIDIValue, (PSZ)(gpIniptr->StrData + *(unsigned int *)pDataPtr));
                                else
                                    WinSetDlgItemText(hDlg, nIDIValue, NULLSTR);
                            }
                            break;

                        case INI_CTL_CHECK:
                            WinCheckButton(hDlg, nIDIValue, ((*(char *)pDataPtr) ? 1 : 0));
                            break;

                        case INI_CTL_RADIO:
                            // Check the appropriate radio button if the value is in range
                            i = (unsigned int)(*(char *)pDataPtr);
                            if ( i < nRadioCnt )
                                WinCheckButton(hDlg, nIDIValue + i, 1);
                            break;

                        case INI_CTL_COLOR:
                            // set foreground and background limits
                            WinSendDlgItemMsg(hDlg, nIDIValue, EM_SETTEXTLIMIT, MPFROMSHORT(15), MP0);
                            WinSendDlgItemMsg(hDlg, nIDIValue + 1, EM_SETTEXTLIMIT, MPFROMSHORT(15), MP0);

                            // load list boxes with color strings (FG and BG are the same)
                            WinSendDlgItemMsg(hDlg, nIDIValue, LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(WinColorDefault));
                            WinSendDlgItemMsg(hDlg, nIDIValue + 1, LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(WinColorDefault));

                            for ( i = 0; (i < 16); i++ ) {
                                WinSendDlgItemMsg(hDlg, nIDIValue, LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(WinColors[i]));
                                WinSendDlgItemMsg(hDlg, nIDIValue + 1, LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(WinColors[i]));
                            }

                            // set current foreground and background colors
                            nNewFG = nNewBG = 0;
                            if ( nIntDataValue != 0 ) {
                                nNewFG = (nIntDataValue & 0xF) + 1;
                                nNewBG = ((nIntDataValue >> 4) & 0xF) + 1;
                            }
                            WinSendDlgItemMsg(hDlg, nIDIValue, LM_SELECTITEM, MPFROMSHORT(nNewFG), MPFROMSHORT(TRUE));
                            WinSendDlgItemMsg(hDlg, nIDIValue + 1, LM_SELECTITEM, MPFROMSHORT(nNewBG), MPFROMSHORT(TRUE));
                            break;
                    }
                }
            }

            return(MRESULT)TRUE;

        case WM_HELP:
            goto ShowSetupHelp;

        case WM_COMMAND:

            nIDIValue = SHORT1FROMMP(mp1);

            switch ( nIDIValue ) {
                case IDI_SAVE:
                    fRewriteINI = TRUE;
                    // fall through to OK processing

                case DID_OK:

                    // Save changed data in all the controls, and flag each changed item
                    for ( nPage = 0; (nPage < CONFIG_PAGE_CNT); nPage++ ) {

                        // Only process this page if it was loaded
                        if ( (hwndPage = ConfigNBPages[nPage].hwndPageDlg) != (HWND)0L ) {

                            for ( nIDIValue = ConfigNBPages[nPage].idFirst; nIDIValue <= ConfigNBPages[nPage].idLast; nIDIValue++ ) {

                                // Get the item number, initialize data for the item if it is in use
                                usINIItemNum = gpINIControlData[nIDIValue - IDI_BASE].usINIItemNum;
                                pfUpdate = &(gpINIControlData[nIDIValue - IDI_BASE].fINIUpdate);

                                if ( usINIItemNum != (USHORT)-1 ) {

                                    nCtlType = (int)gaINIItemList[usINIItemNum].cControlType;
                                    nRadioCnt = (int)gaINIItemList[usINIItemNum].cRadioCnt;
                                    if ( (pDataPtr = gaINIItemList[usINIItemNum].pItemData) != NULL ) {
                                        nIntDataValue = *((int *)pDataPtr);
                                        uIntDataValue = *((int *)pDataPtr);
                                    }

                                    // Default buffer to a null string
                                    szBuffer[0] = '\0';

                                    // Get the data for the control, see if it has changed, and
                                    // if so store it back in the INI file data structure
                                    switch ( nCtlType ) {
                                        case INI_CTL_INT:
                                        case INI_CTL_UINT:
                                            i = WinQueryDlgItemText(hwndPage, nIDIValue, 511, szBuffer);
                                            if ( (sscanf(szBuffer, ((nCtlType == INI_CTL_INT) ? FMT_LONG : FMT_ULONG), &nIntNewValue) != 0) && (nIntNewValue != nIntDataValue) ) {
                                                *((int *)pDataPtr) = nIntNewValue;
                                                *pfUpdate = 1;
                                            }
                                            break;

                                        case INI_CTL_TEXT:
                                            // Get the text from the control
                                            i = WinQueryDlgItemText(hwndPage, nIDIValue, 511, szBuffer);
                                            // Handle characters and strings separately
                                            if ( (gaINIItemList[usINIItemNum].cParseType & INI_PTMASK) == INI_CHAR ) {

                                                if ( szBuffer[0] != *((char *)pDataPtr) ) {
                                                    *((char *)pDataPtr) = szBuffer[0];
                                                    *pfUpdate = 1;
                                                }

                                            } else {

                                                // check if the string changed before we write it out
                                                fStringChanged = FALSE;

                                                // if old string is empty then set flag based on whether
                                                // new string is also empty
                                                if ( (*(unsigned int *)pDataPtr) == INI_EMPTYSTR )
                                                    fStringChanged = (szBuffer[0] != '\0');

                                                // if old string is not empty and new string is empty
                                                // then set flag TRUE
                                                else if ( szBuffer[0] == '\0' )
                                                    fStringChanged = TRUE;

                                                // both strings are not empty, compare them
                                                else
                                                    fStringChanged = (stricmp((gpIniptr->StrData + *(unsigned int *)pDataPtr), szBuffer) != 0);

                                                // save the string in the INI file string area if it changed
                                                if ( fStringChanged ) {
                                                    ini_string(gpIniptr, (int *)pDataPtr, szBuffer, i);
                                                    *pfUpdate = 1;
                                                }
                                            }
                                            break;

                                        case INI_CTL_CHECK:
                                            if ( (unsigned char)(nIntNewValue = ((WinQueryButtonCheckstate(hwndPage, nIDIValue) == 1) ? 1 : 0)) != (*(char *)pDataPtr) ) {
                                                (*(unsigned char *)pDataPtr) = (unsigned char)nIntNewValue;
                                                *pfUpdate = 1;
                                            }
                                            break;

                                        case INI_CTL_RADIO:
                                            if ( ((nIntNewValue = (int)WinSendDlgItemMsg(hwndPage, nIDIValue, BM_QUERYCHECKINDEX, MP0, MP0)) != -1) && ((unsigned char)nIntNewValue != (*(unsigned char *)pDataPtr)) ) {
                                                *(unsigned char *)pDataPtr = (unsigned char)nIntNewValue;
                                                *pfUpdate = 1;
                                            }

                                            break;

                                        case INI_CTL_COLOR:
                                            // set new foreground and background colors
                                            nOldFG = (nIntDataValue & 0xF) + 1;
                                            nOldBG = ((nIntDataValue >> 4) & 0xF) + 1;
                                            nNewFG = (int)WinSendDlgItemMsg(hwndPage, nIDIValue, LM_QUERYSELECTION, MP0, MP0);
                                            nNewBG = (int)WinSendDlgItemMsg(hwndPage, nIDIValue + 1, LM_QUERYSELECTION, MP0, MP0);
                                            if ( nNewFG == LIT_NONE )
                                                nNewFG = nOldFG;
                                            if ( nNewBG == LIT_NONE )
                                                nNewBG = nOldBG;
                                            if ( (nNewFG == 0) || (nNewBG == 0) )
                                                nIntNewValue = nNewFG = nNewBG = 0;
                                            else
                                                nIntNewValue = (((nNewBG - 1) << 4) | (nNewFG - 1));
                                            if ( nIntNewValue != nIntDataValue ) {
                                                *((int *)pDataPtr) = nIntNewValue;
                                                *pfUpdate = 1;
                                            }
                                    }
                                }
                            }

                            // Save data for this page back to the INI file if requested
                            if ( fRewriteINI ) {

                                // Loop through all dialog items on the page
                                for ( nIDIValue = ConfigNBPages[nPage].idFirst; nIDIValue <= ConfigNBPages[nPage].idLast; nIDIValue++ ) {

                                    // If the item is in the INI file and was modified, write it back
                                    if ( ((usINIItemNum = gpINIControlData[nIDIValue - IDI_BASE].usINIItemNum) != (USHORT)-1) && (gpINIControlData[nIDIValue - IDI_BASE].fINIUpdate != 0) ) {

                                        if ( (pDataPtr = gaINIItemList[usINIItemNum].pItemData) != NULL ) {
                                            nIntDataValue = *((int *)pDataPtr);
                                            uIntDataValue = *((int *)pDataPtr);
                                        }

                                        pValid = gaINIItemList[usINIItemNum].pValidate;

                                        // Assume we are writing an empty string
                                        szBuffer[0] = '\0';

                                        // Select appropriate writeback string based on INI data type
                                        switch ( gaINIItemList[usINIItemNum].cParseType & INI_PTMASK ) {
                                            case INI_INT:
                                                sprintf( szBuffer, FMT_INT, nIntDataValue );
                                                break;

                                            case INI_UINT:
                                                sprintf(szBuffer, FMT_UINT, uIntDataValue);
                                                break;

                                            case INI_CHAR:
                                                szBuffer[0] = *((char *)pDataPtr);
                                                szBuffer[1] = '\0';
                                                break;

                                            case INI_STR:
                                            case INI_PATH:
                                                if ( (unsigned int)nIntDataValue != INI_EMPTYSTR ) {
                                                    strcpy(szBuffer, (char *)(gpIniptr->StrData + nIntDataValue));
                                                    if ( (gaINIItemList[usINIItemNum].cParseType & INI_PTMASK) == INI_PATH )
                                                        AddQuotes( szBuffer );
                                                }
                                                break;

                                            case INI_CHOICE:
                                                strcpy(szBuffer, ((pValid->elist)[*((unsigned char *)pDataPtr)]));
                                                break;

                                            case INI_COLOR:
                                                if ( nIntDataValue != 0 )
                                                    sprintf(szBuffer,"%s on %s", WinColors[nIntDataValue & 0xF], WinColors[(nIntDataValue >> 4) & 0xF]);
                                        }

                                        // Delete the item if it is empty, otherwise write the new value
                                        TCWritePrivateProfileStr(INI_SECTION_NAME, gaINIItemList[usINIItemNum].pszItemName, ((szBuffer[0] == '\0') ? NULL : szBuffer));

                                        // Clear the item's update flag
                                        gpINIControlData[nIDIValue - IDI_BASE].fINIUpdate = 0;
                                    }
                                }
                            }
                        }
                    }

                    // On a save in 4OS2 OPTION, clear all the bit flags

                    if ( fRewriteINI )
                        memset( gpIniptr->OptBits, '\0', gpIniptr->OBCount );

                    // On an OK in 4OS2 OPTION, save any item update flags in the option bit flags array
                    else {
                        for ( i = 0; ( i < guINIItemCount ); i++ ) {
                            if ( (nIDIOffsetValue = (int)gaINIItemList[i].uControlID) > 0 ) {
                                nBitByte = nIDIOffsetValue / 8;
                                if ( (nBitByte < gpIniptr->OBCount) && (gpINIControlData[nIDIOffsetValue].fINIUpdate != 0) )
                                    gpIniptr->OptBits[nBitByte] |= (1 << (nIDIOffsetValue % 8));
                            }
                        }
                    }

                    // we have to close the notebook here rather than
                    //   falling through to DID_CANCEL to prevent a
                    //   problem with the toolbar not being restored
                    //   in Take Command for OS/2
                    WinPostMsg(ConfigNBHdr.hwndNBDlg, WM_CLOSE_NOTEBOOK, MP0, MP0);
                    break;

                case DID_CANCEL:
                    WinPostMsg(ConfigNBHdr.hwndNBDlg, WM_CLOSE_NOTEBOOK, MP0, MP0);
                    break;

                case IDD_HELP:
                    ShowSetupHelp:
                    ulCurrentPage = (ULONG) WinSendMsg(ConfigNBHdr.hwndNB, BKM_QUERYPAGEID, NULL, MPFROM2SHORT(BKA_TOP, 0));
                    pThisPage = (PNBPAGE) WinSendMsg(ConfigNBHdr.hwndNB, BKM_QUERYPAGEDATA, MPFROMLONG(ulCurrentPage), NULL);
                    _help( pThisPage->pszHelpName, NULL );
                    //                      break;
                    return(MRESULT)TRUE;
            }

        default:
            return(WinDefDlgProc( hDlg, uMsg, mp1, mp2 ));
    }

    return(MRESULT)FALSE;
}


// Code placed here to avoid dragging MISC.C into 4OS2 OPTION
int AddQuotes( char *pszFileName )
{
    // adjust for an LFN name with embedded whitespace
    if ( ( strpbrk( pszFileName, " \t,=&+<>|" ) != NULL ) && ( *pszFileName != '"' ) ) {
        strins( pszFileName, "\"" );
        strcat( pszFileName, "\"" );
        return 1;
    }
    return 0;
}


// insert a string inside another one
char * strins( char *str, char *insert_str )
{
    unsigned int inslen;

    // length of insert string
    if ( ( inslen = strlen( insert_str )) > 0 ) {

        // move original
        memmove( str+inslen, str, ( strlen( str ) + 1 ));

        // insert the new string into the hole
        memmove( str, insert_str, inslen );
    }

    return( str);
}

