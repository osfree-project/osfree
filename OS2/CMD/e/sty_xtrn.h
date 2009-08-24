/*************************************************************************
*
*  File Name   : STY_XTRN.H
*
*  Description : This header file contains the application wide function
*                declarations and global external variables.
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
*
************************************************************************/

/*--------------------------------------------------------------*\
 *  Global variables                                            *
\*--------------------------------------------------------------*/
extern HWND hwndMainFrame;
extern HWND hwndMain;
extern HAB hab;
extern HMQ hmq;
extern HDC hdcMain;
extern CHAR szAppName[];
extern CHAR szUntitled[];
extern BOOL fHelpEnabled;
extern HWND hwndMLE;

/*--------------------------------------------------------------*\
 *  Entry point declarations                                    *
\*--------------------------------------------------------------*/

/* from sample.c */
MRESULT EXPENTRY MainWndProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);
SHORT MessageBox(HWND hwndOwner, SHORT idMsg, SHORT fsStyle, BOOL fBeep);
VOID MainCommand(MPARAM mp1, MPARAM mp2);

/* from init.c */
BOOL Init(VOID);
MRESULT InitMainWindow(HWND hwnd, MPARAM mp1, MPARAM mp2);
VOID APIENTRY ExitProc(USHORT usTermCode);

/* from file.c */
VOID FileNew(MPARAM mp2);
VOID FileOpen(MPARAM mp2);
VOID FileSave(MPARAM mp2);
VOID FileSaveAs(MPARAM mp2);
VOID WriteFileToDisk(HFILE hf);
BOOL GetFileName(VOID);
VOID UpdateTitleText(HWND hwnd);
MRESULT APIENTRY OpenSaveFilterDlgProc(HWND hwnd, USHORT msg, MPARAM mp1,
                                    MPARAM mp2);

/* from edit.c */
VOID EditUndo(MPARAM mp2);
VOID EditCut(MPARAM mp2);
VOID EditCopy(MPARAM mp2);
VOID EditPaste(MPARAM mp2);
VOID EditClear(MPARAM mp2);

/* from user.c */
VOID UserCommand(MPARAM mp1, MPARAM mp2);
MRESULT UserWndProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);
VOID InitMenu(MPARAM mp1, MPARAM mp2);
VOID EnableMenuItem(HWND hwndMenu, SHORT idItem, BOOL fEnable);

/* from pnt.c */
VOID MainPaint(HWND hwnd);

/* from help.c */
VOID InitHelp(VOID);
VOID HelpUsingHelp(MPARAM mp2);
VOID HelpGeneral(MPARAM mp2);
VOID HelpKeys(MPARAM mp2);
VOID HelpIndex(MPARAM mp2);
VOID HelpTutorial(MPARAM mp2);
VOID HelpProdInfo(MPARAM mp2);
VOID DisplayHelpPanel(SHORT nPanelId);
VOID DestroyHelpInstance(VOID);
