/*****************************************************************************
 ** LVMPM - utils.h                                                         **
 *****************************************************************************
 * This header file defines the various utility routines in utils.c          *
 *****************************************************************************
 *                                                                           *
 * Copyright (C) 2011-2019 Alexander Taylor.                                 *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify it  *
 *  under the terms of the GNU General Public License as published by the    *
 *  Free Software Foundation; either version 2 of the License, or (at your   *
 *  option) any later version.                                               *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful, but      *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  General Public License for more details.                                 *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License along  *
 *  with this program; if not, write to the Free Software Foundation, Inc.,  *
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                  *
 *                                                                           *
 *****************************************************************************/

void             CentreWindow( HWND hwnd, HWND hwndRelative, ULONG ulFlags );
void             ResizeDialog( HWND hwnd, LONG cx, LONG cy );
BOOL             CheckDBCS( void );
void             DrawInsetBorder( HPS hps, RECTL rcl );
void             DrawNice3DBorder( HPS hps, RECTL rcl );
void             DrawOutlineBorder( HPS hps, RECTL rcl );
BOOL             FileExists( PSZ pszFile );
MRESULT EXPENTRY FontPreviewProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
BOOL             GetLanguageFile( PSZ pszFullPath, PSZ pszName );
MRESULT EXPENTRY HorizontalRuleProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY InsetBorderProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY OutlineBorderProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
BOOL             MenuItemAddCnd(  HWND  hwndMenu, SHORT sPos, SHORT sID, PSZ pszTitle, SHORT sfStyle );
void             MenuItemEnable( HWND hwndMenu1, HWND hwndMenu2, SHORT sID, BOOL fEnable );
void             SetContainerFieldTitle( HWND hwndCnr, ULONG ulOffset, PSZ pszNew );
BOOL             GetSelectedPartition( HWND hwndDV, PPVCTLDATA ppvd );


