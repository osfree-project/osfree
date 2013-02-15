/*
 * Program Manager
 *
 * Copyright 1996 Ulrich Schmid
 * Copyright 2002 Sylvain Petreolle
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
#include "progman.h"

/* Class names */

char STRING_MAIN_WIN_CLASS_NAME[]    = "PMMain";
char STRING_MDI_WIN_CLASS_NAME[]     = "MDICLIENT";
char STRING_GROUP_WIN_CLASS_NAME[]   = "PMGroup";
char STRING_PROGRAM_WIN_CLASS_NAME[] = "PMProgram";

/* Resource names */
char STRING_ACCEL[]      = "ACCEL";
char STRING_MENU[]    = "MENU";
char STRING_NEW[]     = "DIALOG_NEW";
char STRING_OPEN[]    = "DIALOG_OPEN";
char STRING_MOVE[]    = "DIALOG_MOVE";
char STRING_COPY[]    = "DIALOG_COPY";
char STRING_DELETE[]  = "DIALOG_DELETE";
char STRING_GROUP[]   = "DIALOG_GROUP";
char STRING_PROGRAM[] = "DIALOG_PROGRAM";
char STRING_SYMBOL[]  = "DIALOG_SYMBOL";
char STRING_EXECUTE[] = "DIALOG_EXECUTE";


VOID STRING_LoadMenus(VOID)
{
  char   caption[MAX_STRING_LEN];
  HMENU  hMainMenu;

  /* Set frame caption */
  LoadString(Globals.hInstance, IDS_PROGRAM_MANAGER, caption, sizeof(caption));
  SetWindowText(Globals.hMainWnd, caption);

  /* Create menu */
  hMainMenu = LoadMenu(Globals.hInstance, MAKEINTRESOURCE(MAIN_MENU));
  Globals.hFileMenu     = GetSubMenu(hMainMenu, 0);
  Globals.hOptionMenu   = GetSubMenu(hMainMenu, 1);
  Globals.hWindowsMenu  = GetSubMenu(hMainMenu, 2);
  Globals.hLanguageMenu = GetSubMenu(hMainMenu, 3);

  if (Globals.hMDIWnd)
    SendMessage(Globals.hMDIWnd, WM_MDISETMENU,
                (WPARAM) hMainMenu,
                (LPARAM) Globals.hWindowsMenu);
  else SetMenu(Globals.hMainWnd, hMainMenu);

  /* Destroy old menu */
  if (Globals.hMainMenu) DestroyMenu(Globals.hMainMenu);
  Globals.hMainMenu = hMainMenu;
}

/* Local Variables:    */
/* c-file-style: "GNU" */
/* End:                */
