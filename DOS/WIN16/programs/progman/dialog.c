/*
 * Program Manager
 *
 * Copyright 1996 Ulrich Schmid
 * Copyright 2002 Sylvain Petreolle
 * Copyright 2002 Andriy Palamarchuk
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#define WIN32_LEAN_AND_MEAN

#include "windows.h"
#include "commdlg.h"
#include "progman.h"
#include <string.h>
static BOOL    DIALOG_BrowsePrograms(HWND, LPSTR, int);
static BOOL    DIALOG_BrowseSymbols(HWND, LPSTR, int);
static int CALLBACK DIALOG_NEW_DlgProc(HWND, unsigned int, WPARAM, LPARAM);
static int CALLBACK DIALOG_COPY_MOVE_DlgProc(HWND, unsigned int, WPARAM, LPARAM);
static int CALLBACK DIALOG_GROUP_DlgProc(HWND, unsigned int, WPARAM, LPARAM);
static int CALLBACK DIALOG_PROGRAM_DlgProc(HWND, unsigned int, WPARAM, LPARAM);
static int CALLBACK DIALOG_SYMBOL_DlgProc(HWND, unsigned int, WPARAM, LPARAM);
static int CALLBACK DIALOG_EXECUTE_DlgProc(HWND, unsigned int, WPARAM, LPARAM);

/***********************************************************************
 *
 *           DIALOG_New
 */

static struct
{
  int nDefault;
} New;

int DIALOG_New(int nDefault)
{
  DLGPROC lpfnDlg = MakeProcInstance(DIALOG_NEW_DlgProc, Globals.hInstance);
  int ret;

  New.nDefault = nDefault;

  ret = DialogBox(Globals.hInstance,  STRING_NEW,
                  Globals.hMainWnd, lpfnDlg);
  FreeProcInstance(lpfnDlg);
  return ret;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *           DIALOG_NEW_DlgProc
 */

static int CALLBACK DIALOG_NEW_DlgProc(HWND hDlg, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
    {
    case WM_INITDIALOG:
      CheckRadioButton(hDlg, PM_NEW_GROUP, PM_NEW_PROGRAM, New.nDefault);
      break;

    case WM_COMMAND:
      switch (wParam)
        {
        case PM_NEW_GROUP:
        case PM_NEW_PROGRAM:
          CheckRadioButton(hDlg, PM_NEW_GROUP, PM_NEW_PROGRAM, wParam);
          return TRUE;

        case IDOK:
          EndDialog(hDlg, IsDlgButtonChecked(hDlg, PM_NEW_GROUP) ?
                    PM_NEW_GROUP : PM_NEW_PROGRAM);
          return TRUE;

        case IDCANCEL:
          EndDialog(hDlg, IDCANCEL);
          return TRUE;
        }
    }
  return FALSE;
}

/***********************************************************************
 *
 *           DIALOG_CopyMove
 */

static struct
{
  LPCSTR lpszProgramName, lpszFromGroupName;
  HLOCAL hToGroup;
} CopyMove;

HLOCAL DIALOG_CopyMove(LPCSTR lpszProgramName, LPCSTR lpszFromGroupName,
                     BOOL bMove)
{
  DLGPROC lpfnDlg = MakeProcInstance(DIALOG_COPY_MOVE_DlgProc, Globals.hInstance);
  int ret;

  CopyMove.lpszProgramName   = lpszProgramName;
  CopyMove.lpszFromGroupName = lpszFromGroupName;
  CopyMove.hToGroup          = 0;

  ret = DialogBox(Globals.hInstance,
                  bMove ? STRING_MOVE : STRING_COPY,
                  Globals.hMainWnd, lpfnDlg);
  FreeProcInstance(lpfnDlg);

  return((ret == IDOK) ? CopyMove.hToGroup : 0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *           DIALOG_COPY_MOVE_DlgProc
 */

static int CALLBACK DIALOG_COPY_MOVE_DlgProc(HWND hDlg, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  HLOCAL hGroup;

  switch (msg)
    {
    case WM_INITDIALOG:
      /* List all group names */
      for (hGroup = GROUP_FirstGroup(); hGroup; hGroup = GROUP_NextGroup(hGroup))
        SendDlgItemMessage(hDlg, PM_TO_GROUP, CB_ADDSTRING, 0,
                           (LPARAM) GROUP_GroupName(hGroup));

      SetDlgItemText(hDlg, PM_PROGRAM,    CopyMove.lpszProgramName);
      SetDlgItemText(hDlg, PM_FROM_GROUP, CopyMove.lpszFromGroupName);
      break;

    case WM_COMMAND:
      switch (wParam)
        {
        case IDOK:
        {
          /* Get selected group */
          int nCurSel    = SendDlgItemMessage(hDlg, PM_TO_GROUP, CB_GETCURSEL, 0, 0);
          int nLen       = SendDlgItemMessage(hDlg, PM_TO_GROUP, CB_GETLBTEXTLEN, nCurSel, 0);
          HLOCAL hBuffer = LocalAlloc(LMEM_FIXED, nLen + 1);
          LPSTR   buffer = LocalLock(hBuffer);

          SendDlgItemMessage(hDlg, PM_TO_GROUP, CB_GETLBTEXT, nCurSel, (LPARAM)buffer);
          for (hGroup = GROUP_FirstGroup(); hGroup; hGroup = GROUP_NextGroup(hGroup))
            if (!lstrcmp(buffer, GROUP_GroupName(hGroup))) break;
          LocalFree(hBuffer);

          CopyMove.hToGroup = hGroup;
          EndDialog(hDlg, IDOK);
          return TRUE;
        }

        case IDCANCEL:
          EndDialog(hDlg, IDCANCEL);
          return TRUE;
        }
    }
  return FALSE;
}

/***********************************************************************
 *
 *           DIALOG_Delete
 */

BOOL DIALOG_Delete(unsigned int ids_text_s, LPCSTR lpszName)
{
  return (IDYES == MAIN_MessageBoxIDS_s(ids_text_s, lpszName, IDS_DELETE,
                                        MB_YESNO | MB_DEFBUTTON2));
}


/***********************************************************************
 *
 *           DIALOG_GroupAttributes
 */

static struct
{
  LPSTR lpszTitle, lpszGrpFile;
  int   nSize;
} GroupAttributes;

BOOL DIALOG_GroupAttributes(LPSTR lpszTitle, LPSTR lpszGrpFile, int nSize)
{
  DLGPROC lpfnDlg = MakeProcInstance(DIALOG_GROUP_DlgProc, Globals.hInstance);
  int ret;

  GroupAttributes.nSize       = nSize;
  GroupAttributes.lpszTitle   = lpszTitle;
  GroupAttributes.lpszGrpFile = lpszGrpFile;

  ret = DialogBox(Globals.hInstance,  STRING_GROUP,
                  Globals.hMainWnd, lpfnDlg);
  FreeProcInstance(lpfnDlg);
  return(ret == IDOK);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *           DIALOG_GROUP_DlgProc
 */

static int CALLBACK DIALOG_GROUP_DlgProc(HWND hDlg, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
    {
    case WM_INITDIALOG:
      SetDlgItemText(hDlg, PM_DESCRIPTION, GroupAttributes.lpszTitle);
      SetDlgItemText(hDlg, PM_FILE, GroupAttributes.lpszGrpFile);
      break;

    case WM_COMMAND:
      switch (wParam)
        {
        case IDOK:
          GetDlgItemText(hDlg, PM_DESCRIPTION, GroupAttributes.lpszTitle,
                         GroupAttributes.nSize);
          GetDlgItemText(hDlg, PM_FILE, GroupAttributes.lpszGrpFile,
                         GroupAttributes.nSize);
          EndDialog(hDlg, IDOK);
          return TRUE;

        case IDCANCEL:
          EndDialog(hDlg, IDCANCEL);
          return TRUE;
        }
    }
  return FALSE;
}

/***********************************************************************
 *
 *           DIALOG_ProgramAttributes
 */

static struct
{
  LPSTR lpszTitle, lpszCmdLine, lpszWorkDir, lpszIconFile, lpszTmpIconFile;
  int   nSize;
  int   *lpnCmdShow;
  int   *lpnHotKey;
  HWND  hSelGroupWnd;
  HICON *lphIcon, hTmpIcon;
  int   *lpnIconIndex, nTmpIconIndex;
} ProgramAttributes;

BOOL DIALOG_ProgramAttributes(LPSTR lpszTitle, LPSTR lpszCmdLine,
                              LPSTR lpszWorkDir, LPSTR lpszIconFile,
                              HICON *lphIcon, int *lpnIconIndex,
                              int *lpnHotKey, int *lpnCmdShow, int nSize)
{
  char szTmpIconFile[MAX_PATHNAME_LEN];
  DLGPROC lpfnDlg = MakeProcInstance(DIALOG_PROGRAM_DlgProc, Globals.hInstance);
  int ret;

  ProgramAttributes.nSize = nSize;
  ProgramAttributes.lpszTitle = lpszTitle;
  ProgramAttributes.lpszCmdLine = lpszCmdLine;
  ProgramAttributes.lpszWorkDir = lpszWorkDir;
  ProgramAttributes.lpszIconFile = lpszIconFile;
  ProgramAttributes.lpnCmdShow = lpnCmdShow;
  ProgramAttributes.lpnHotKey = lpnHotKey;
  ProgramAttributes.lphIcon = lphIcon;
  ProgramAttributes.lpnIconIndex = lpnIconIndex;

#if 0
  ProgramAttributes.hTmpIcon = 0;
#else
  ProgramAttributes.hTmpIcon = *lphIcon;
#endif
  ProgramAttributes.nTmpIconIndex = *lpnIconIndex;
  ProgramAttributes.lpszTmpIconFile = szTmpIconFile;
  lstrcpyn(ProgramAttributes.lpszTmpIconFile, lpszIconFile, MAX_PATHNAME_LEN);

  ret = DialogBox(Globals.hInstance,  STRING_PROGRAM,
                  Globals.hMainWnd, lpfnDlg);
  FreeProcInstance(lpfnDlg);

  return(ret == IDOK);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *           DIALOG_PROGRAM_DlgProc
 */

static int CALLBACK DIALOG_PROGRAM_DlgProc(HWND hDlg, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  char buffer[MAX_STRING_LEN];
  switch (msg)
    {
    case WM_INITDIALOG:
      SetDlgItemText(hDlg, PM_DESCRIPTION, ProgramAttributes.lpszTitle);
      SetDlgItemText(hDlg, PM_COMMAND_LINE, ProgramAttributes.lpszCmdLine);
      SetDlgItemText(hDlg, PM_DIRECTORY, ProgramAttributes.lpszWorkDir);
      if (!*ProgramAttributes.lpnHotKey)
        {
          LoadString(Globals.hInstance, IDS_NO_HOT_KEY, buffer, sizeof(buffer));
          SetDlgItemText(hDlg, PM_HOT_KEY, buffer);
        }

      CheckDlgButton(hDlg, PM_SYMBOL,
                     (*ProgramAttributes.lpnCmdShow == SW_SHOWMINIMIZED));
      SendDlgItemMessage(hDlg, PM_ICON, STM_SETICON,
                         (WPARAM) ProgramAttributes.hTmpIcon, 0);
      break;

    case WM_COMMAND:
      switch (wParam)
        {
        case PM_SYMBOL:
          CheckDlgButton(hDlg, PM_SYMBOL, !IsDlgButtonChecked(hDlg, PM_SYMBOL));
          return TRUE;

        case PM_BROWSE:
          {
            char filename[MAX_PATHNAME_LEN];
            filename[0] = 0;
            if (DIALOG_BrowsePrograms(hDlg, filename, sizeof(filename)))
              SetDlgItemText(hDlg, PM_COMMAND_LINE, filename);
            return TRUE;
          }

        case PM_OTHER_SYMBOL:
          {
            DIALOG_Symbol(&ProgramAttributes.hTmpIcon,
                          ProgramAttributes.lpszTmpIconFile,
                          &ProgramAttributes.nTmpIconIndex,
                          MAX_PATHNAME_LEN);

            SendDlgItemMessage(hDlg, PM_ICON, STM_SETICON,
                               (WPARAM) ProgramAttributes.hTmpIcon, 0);
            return TRUE;
          }

        case IDOK:
          GetDlgItemText(hDlg, PM_DESCRIPTION,
                         ProgramAttributes.lpszTitle,
                         ProgramAttributes.nSize);
          GetDlgItemText(hDlg, PM_COMMAND_LINE,
                         ProgramAttributes.lpszCmdLine,
                         ProgramAttributes.nSize);
          GetDlgItemText(hDlg, PM_DIRECTORY,
                         ProgramAttributes.lpszWorkDir,
                         ProgramAttributes.nSize);

          if (ProgramAttributes.hTmpIcon)
            {
#if 0
              if (*ProgramAttributes.lphIcon)
                DestroyIcon(*ProgramAttributes.lphIcon);
#endif
              *ProgramAttributes.lphIcon = ProgramAttributes.hTmpIcon;
              *ProgramAttributes.lpnIconIndex = ProgramAttributes.nTmpIconIndex;
              lstrcpyn(ProgramAttributes.lpszIconFile,
                       ProgramAttributes.lpszTmpIconFile,
                       ProgramAttributes.nSize);
            }

          *ProgramAttributes.lpnCmdShow =
            IsDlgButtonChecked(hDlg, PM_SYMBOL) ?
            SW_SHOWMINIMIZED : SW_SHOWNORMAL;
          EndDialog(hDlg, IDOK);
          return TRUE;

        case IDCANCEL:
          EndDialog(hDlg, IDCANCEL);
          return TRUE;
        }
      return FALSE;
    }
  return FALSE;
}

/***********************************************************************
 *
 *           DIALOG_Symbol
 */

static struct
{
  LPSTR  lpszIconFile;
  int    nSize;
  HICON  *lphIcon;
  int    *lpnIconIndex;
} Symbol;

VOID DIALOG_Symbol(HICON *lphIcon, LPSTR lpszIconFile,
                   int *lpnIconIndex, int nSize)
{
  DLGPROC lpfnDlg = MakeProcInstance(DIALOG_SYMBOL_DlgProc, Globals.hInstance);

  Symbol.nSize = nSize;
  Symbol.lpszIconFile = lpszIconFile;
  Symbol.lphIcon = lphIcon;
  Symbol.lpnIconIndex = lpnIconIndex;

  DialogBox(Globals.hInstance, STRING_SYMBOL,
            Globals.hMainWnd, lpfnDlg);
  FreeProcInstance(lpfnDlg);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *           DIALOG_SYMBOL_DlgProc
 */

static int CALLBACK DIALOG_SYMBOL_DlgProc(HWND hDlg, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
    {
    case WM_INITDIALOG:
      SetDlgItemText(hDlg, PM_ICON_FILE, Symbol.lpszIconFile);
      SendDlgItemMessage(hDlg, PM_SYMBOL_LIST, CB_SETITEMHEIGHT, 0, (LPARAM) 32);
      SendDlgItemMessage(hDlg, PM_SYMBOL_LIST, CB_ADDSTRING, 0, (LPARAM)*Symbol.lphIcon);
      SendDlgItemMessage(hDlg, PM_SYMBOL_LIST, CB_ADDSTRING, 0, (LPARAM)Globals.hDefaultIcon);
      SendDlgItemMessage(hDlg, PM_SYMBOL_LIST, CB_SETCURSEL, 0, 0);
      return TRUE;

    case WM_MEASUREITEM:
      {
        PMEASUREITEMSTRUCT measure = (PMEASUREITEMSTRUCT) lParam;
        measure->itemWidth = 32;
        measure->itemHeight = 32;
        return TRUE;
      }

    case WM_DRAWITEM:
      {
        PDRAWITEMSTRUCT dis = (PDRAWITEMSTRUCT) lParam;
        DrawIcon(dis->hDC, dis->rcItem.left, dis->rcItem.top, (HICON)dis->itemData);
        return TRUE;
      }

    case WM_COMMAND:
      switch (wParam)
        {
        case PM_BROWSE:
          {
            char filename[MAX_PATHNAME_LEN];
            filename[0] = 0;
            if (DIALOG_BrowseSymbols(hDlg, filename, sizeof(filename)))
              SetDlgItemText(hDlg, PM_ICON_FILE, filename);
            return TRUE;
          }

        case PM_HELP:
          MAIN_MessageBoxIDS(IDS_NOT_IMPLEMENTED, IDS_ERROR, MB_OK);
          return TRUE;

        case IDOK:
          {
            int nCurSel = SendDlgItemMessage(hDlg, PM_SYMBOL_LIST, CB_GETCURSEL, 0, 0);

            GetDlgItemText(hDlg, PM_ICON_FILE, Symbol.lpszIconFile, Symbol.nSize);

            *Symbol.lphIcon = (HICON)SendDlgItemMessage(hDlg, PM_SYMBOL_LIST,
                                                        CB_GETITEMDATA,
                                                        (WPARAM) nCurSel, 0);
#if 0
            *Symbol.lphIcon = CopyIcon(*Symbol.lphIcon);
#endif

            EndDialog(hDlg, IDOK);
            return TRUE;
          }

        case IDCANCEL:
          EndDialog(hDlg, IDCANCEL);
          return TRUE;
        }
    }
  return FALSE;
}

/***********************************************************************
 *
 *           DIALOG_Execute
 */

VOID DIALOG_Execute(void)
{
  int z;
  DLGPROC lpfnDlg = MakeProcInstance(DIALOG_EXECUTE_DlgProc, Globals.hInstance);
  z=DialogBox(Globals.hInstance, STRING_EXECUTE,
              Globals.hMainWnd, lpfnDlg);
              FreeProcInstance(lpfnDlg);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *           DIALOG_EXECUTE_DlgProc
 */

static int CALLBACK DIALOG_EXECUTE_DlgProc(HWND hDlg, unsigned int msg,
                                      WPARAM wParam, LPARAM lParam)
{
  switch (msg)
    {
    case WM_COMMAND:
      switch (wParam)
        {
        case PM_SYMBOL:
          CheckDlgButton(hDlg, PM_SYMBOL, !IsDlgButtonChecked(hDlg, PM_SYMBOL));
          return TRUE;

        case PM_BROWSE:
          {
            char filename[MAX_PATHNAME_LEN];
            filename[0] = 0;
            if (DIALOG_BrowsePrograms(hDlg, filename, sizeof(filename)))
              SetDlgItemText(hDlg, PM_COMMAND, filename);
            return TRUE;
          }

        case PM_HELP:
          MAIN_MessageBoxIDS(IDS_NOT_IMPLEMENTED, IDS_ERROR, MB_OK);
          return TRUE;

        case IDOK:
          {
            char cmdline[MAX_PATHNAME_LEN];
            GetDlgItemText(hDlg, PM_COMMAND, cmdline, sizeof(cmdline));

            WinExec(cmdline, IsDlgButtonChecked(hDlg, PM_SYMBOL) ?
                    SW_SHOWMINIMIZED : SW_SHOWNORMAL);
            if (Globals.bMinOnRun) CloseWindow(Globals.hMainWnd);

            EndDialog(hDlg, IDOK);
            return TRUE;
          }

        case IDCANCEL:
          EndDialog(hDlg, IDCANCEL);
          return TRUE;
        }
    }
  return FALSE;
}

/***********************************************************************
 *
 *           DIALOG_Browse
 */

static BOOL DIALOG_Browse(HWND hDlg, LPCSTR lpszzFilter,
                          LPSTR lpstrFile, int nMaxFile)

{
    OPENFILENAME openfilename;

    char szDir[MAX_PATH];
    char szDefaultExt[] = "exe";

//    ZeroMemory(&openfilename, sizeof(openfilename));

    GetCurrentDirectory(sizeof(szDir), szDir);

    openfilename.lStructSize       = sizeof(openfilename);
    openfilename.hwndOwner         = Globals.hMainWnd;
    openfilename.hInstance         = Globals.hInstance;
    openfilename.lpstrFilter       = lpszzFilter;
    openfilename.lpstrFile         = lpstrFile;
    openfilename.nMaxFile          = nMaxFile;
    openfilename.lpstrInitialDir   = szDir;
    openfilename.Flags             = 0;
    openfilename.lpstrDefExt       = szDefaultExt;
    openfilename.lpstrCustomFilter = 0;
    openfilename.nMaxCustFilter    = 0;
    openfilename.nFilterIndex      = 0;
    openfilename.lpstrFileTitle    = 0;
    openfilename.nMaxFileTitle     = 0;
    openfilename.lpstrInitialDir   = 0;
    openfilename.lpstrTitle        = 0;
    openfilename.nFileOffset       = 0;
    openfilename.nFileExtension    = 0;
    openfilename.lCustData         = 0;
    openfilename.lpfnHook          = 0;
    openfilename.lpTemplateName    = 0;

    return GetOpenFileName(&openfilename);
}

/***********************************************************************
 *
 *           DIALOG_AddFilterItem
 */

static VOID DIALOG_AddFilterItem(LPSTR *p, unsigned int ids, LPCSTR filter)
{
  LoadString(Globals.hInstance, ids, *p, MAX_STRING_LEN);
  *p += strlen(*p) + 1;
  lstrcpy(*p, filter);
  *p += strlen(*p) + 1;
  **p = '\0';
}

/***********************************************************************
 *
 *           DIALOG_BrowsePrograms
 */

static BOOL DIALOG_BrowsePrograms(HWND hDlg, LPSTR lpszFile, int nMaxFile)
{
  char  szzFilter[2 * MAX_STRING_LEN + 100];
  LPSTR p = szzFilter;

  DIALOG_AddFilterItem(&p, IDS_PROGRAMS,  "*.exe;*.pif;*.com;*.bat");
  DIALOG_AddFilterItem(&p, IDS_ALL_FILES, "*.*");

  return(DIALOG_Browse(hDlg, szzFilter, lpszFile, nMaxFile));
}

/***********************************************************************
 *
 *           DIALOG_BrowseSymbols
 */

static BOOL DIALOG_BrowseSymbols(HWND hDlg, LPSTR lpszFile, int nMaxFile)
{
  char  szzFilter[5 * MAX_STRING_LEN + 100];
  LPSTR p = szzFilter;

  DIALOG_AddFilterItem(&p, IDS_SYMBOL_FILES,  "*.ico;*.exe;*.dll");
  DIALOG_AddFilterItem(&p, IDS_PROGRAMS,      "*.exe");
  DIALOG_AddFilterItem(&p, IDS_LIBRARIES_DLL, "*.dll");
  DIALOG_AddFilterItem(&p, IDS_SYMBOLS_ICO,   "*.ico");
  DIALOG_AddFilterItem(&p, IDS_ALL_FILES,     "*.*");

  return(DIALOG_Browse(hDlg, szzFilter, lpszFile, nMaxFile));
}

/* Local Variables:    */
/* c-file-style: "GNU" */
/* End:                */
