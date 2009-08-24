/*****************************************************************************\
*                                                                             *
* prsht.h - - Interface for the Windows Property Sheet Pages                  *
*                                                                             *
* Version 1.0                                                                 *
*                                                                             *
* Copyright (c) 1991-1995, Microsoft Corp.      All rights reserved.          *
*
*	@(#)prsht.h	1.3 10/30/96 18:39:12 /users/sccs/src/win/s.prsht.h
*                                                                             *
\*****************************************************************************/

#ifndef _PRSHT_H_
#define _PRSHT_H_

/* */
/* Define API decoration for direct importing of DLL references. */
/* */
#ifndef WINCOMMCTRLAPI
#if !defined(_COMCTL32_) && defined(_WIN32)
#define WINCOMMCTRLAPI DECLSPEC_IMPORT
#else
#define WINCOMMCTRLAPI
#endif
#endif /* WINCOMMCTRLAPI */

#ifdef __cplusplus
extern "C" {
#endif

#if (WINVER >= 0x0400)

#define MAXPROPPAGES            100

struct _PSP;
typedef struct _PSP FAR* HPROPSHEETPAGE;

#ifndef UNICODE_ONLY
typedef struct _PROPSHEETPAGEA FAR *LPPROPSHEETPAGEA;
#endif /*!UNICODE_ONLY */
#ifndef ANSI_ONLY
typedef struct _PROPSHEETPAGEW FAR *LPPROPSHEETPAGEW;
#endif /*!ANSI_ONLY */

#ifndef UNICODE_ONLY
typedef UINT (CALLBACK FAR * LPFNPSPCALLBACKA)(HWND hwnd, UINT uMsg, LPPROPSHEETPAGEA ppsp);
#endif /*!UNICODE_ONLY */
#ifndef ANSI_ONLY
typedef UINT (CALLBACK FAR * LPFNPSPCALLBACKW)(HWND hwnd, UINT uMsg, LPPROPSHEETPAGEW ppsp);
#endif /*!ANSI_ONLY */

#ifdef UNICODE
#define LPFNPSPCALLBACK         LPFNPSPCALLBACKW
#else
#define LPFNPSPCALLBACK         LPFNPSPCALLBACKA
#endif

#define PSP_DEFAULT             0x0000
#define PSP_DLGINDIRECT         0x0001
#define PSP_USEHICON            0x0002
#define PSP_USEICONID           0x0004
#define PSP_USETITLE            0x0008
#define PSP_RTLREADING          0x0010

#define PSP_HASHELP             0x0020
#define PSP_USEREFPARENT        0x0040
#define PSP_USECALLBACK         0x0080

#define PSPCB_RELEASE           1
#define PSPCB_CREATE            2


#ifndef UNICODE_ONLY
typedef struct _PROPSHEETPAGEA {
        DWORD           dwSize;
        DWORD           dwFlags;
        HINSTANCE       hInstance;
#if 0	/* TWIN: Hack to allow g++ to recognize anonymous unions */
        union _tagTemplateA {
#else	/* TWIN */
        union /*_tagTemplateA*/ {
#endif	/* TWIN */
            LPCSTR          pszTemplate;
#ifdef _WIN32
            LPCDLGTEMPLATE  pResource;
#else
            const VOID FAR *pResource;
#endif
        } u;
#if 0	/* TWIN: Hack to allow g++ to recognize anonymous unions */
        union _tagIconA {
#else	/* TWIN */
        union /*_tagIconA*/ {
#endif	/* TWIN */
            HICON       hIcon;
            LPCSTR      pszIcon;
        } u2;
        LPCSTR          pszTitle;
        DLGPROC         pfnDlgProc;
        LPARAM          lParam;
        LPFNPSPCALLBACKA pfnCallback;
        UINT FAR * pcRefParent;
} PROPSHEETPAGEA; /*, FAR *LPPROPSHEETPAGEA;*/
#endif /*!UNICODE_ONLY */
#ifndef UNICODE_ONLY
typedef const PROPSHEETPAGEA FAR *LPCPROPSHEETPAGEA;
#endif /*!UNICODE_ONLY */

#ifndef ANSI_ONLY
typedef struct _PROPSHEETPAGEW {
        DWORD           dwSize;
        DWORD           dwFlags;
        HINSTANCE       hInstance;
#if 0	/* TWIN: Hack to allow g++ to recognize anonymous unions */
        union _tagTemplateW {
#else	/* TWIN */
        union /*_tagTemplateW*/ {
#endif	/* TWIN */
            LPCWSTR          pszTemplate;
#ifdef _WIN32
            LPCDLGTEMPLATE  pResource;
#else
            const VOID FAR *pResource;
#endif
        } u;
#if 0	/* TWIN: Hack to allow g++ to recognize anonymous unions */
        union _tagIconW {
#else	/* TWIN */
        union /*_tagIconW*/ {
#endif	/* TWIN */
            HICON       hIcon;
            LPCWSTR      pszIcon;
        } u2;
        LPCWSTR          pszTitle;
        DLGPROC         pfnDlgProc;
        LPARAM          lParam;
        LPFNPSPCALLBACKW pfnCallback;
        UINT FAR * pcRefParent;
} PROPSHEETPAGEW; /*, FAR *LPPROPSHEETPAGEW;*/
#endif /*!ANSI_ONLY */
#ifndef ANSI_ONLY
typedef const PROPSHEETPAGEW FAR *LPCPROPSHEETPAGEW;
#endif /*!ANSI_ONLY */

#ifdef UNICODE
#define PROPSHEETPAGE           PROPSHEETPAGEW
#define LPPROPSHEETPAGE         LPPROPSHEETPAGEW
#define LPCPROPSHEETPAGE        LPCPROPSHEETPAGEW
#else
#define PROPSHEETPAGE           PROPSHEETPAGEA
#define LPPROPSHEETPAGE         LPPROPSHEETPAGEA
#define LPCPROPSHEETPAGE        LPCPROPSHEETPAGEA
#endif


#define PSH_DEFAULT             0x0000
#define PSH_PROPTITLE           0x0001
#define PSH_USEHICON            0x0002
#define PSH_USEICONID           0x0004
#define PSH_PROPSHEETPAGE       0x0008
#define PSH_WIZARD              0x0020
#define PSH_USEPSTARTPAGE       0x0040
#define PSH_NOAPPLYNOW          0x0080
#define PSH_USECALLBACK         0x0100
#define PSH_HASHELP             0x0200
#define PSH_MODELESS            0x0400
#define PSH_RTLREADING          0x0800
#define PSH_MULTILINETABS       0x0010 /*added 05/01/96 by Hung */

typedef int (CALLBACK *PFNPROPSHEETCALLBACK)(HWND, UINT, LPARAM);

#ifndef UNICODE_ONLY
typedef struct _PROPSHEETHEADERA {
        DWORD           dwSize;
        DWORD           dwFlags;
        HWND            hwndParent;
        HINSTANCE       hInstance;
#if 0	/* TWIN: Hack to allow g++ to recognize anonymous unions */
        union _tagIcnA {
#else	/* TWIN */
        union /*_tagIcnA*/ {
#endif	/* TWIN */
            HICON       hIcon;
            LPCSTR      pszIcon;
        } u;
        LPCSTR          pszCaption;


        UINT            nPages;
#if 0	/* TWIN: Hack to allow g++ to recognize anonymous unions */
        union _tagPageIDA {
#else	/* TWIN */
        union /*_tagPageIDA*/ {
#endif	/* TWIN */
            UINT        nStartPage;
            LPCSTR      pStartPage;
        } u2;
#if 0	/* TWIN: Hack to allow g++ to recognize anonymous unions */
        union _tagPagesA {
#else	/* TWIN */
        union /*_tagPagesA*/ {
#endif	/* TWIN */
            LPCPROPSHEETPAGEA ppsp;
            HPROPSHEETPAGE FAR *phpage;
        } u3;
        PFNPROPSHEETCALLBACK pfnCallback;
} PROPSHEETHEADERA, FAR *LPPROPSHEETHEADERA;
#endif /*!UNICODE_ONLY */
#ifndef UNICODE_ONLY
typedef const PROPSHEETHEADERA FAR *LPCPROPSHEETHEADERA;
#endif /*!UNICODE_ONLY */

#ifndef ANSI_ONLY
typedef struct _PROPSHEETHEADERW {
        DWORD           dwSize;
        DWORD           dwFlags;
        HWND            hwndParent;
        HINSTANCE       hInstance;
#if 0	/* TWIN: Hack to allow g++ to recognize anonymous unions */
        union _tagIcnW {
#else	/* TWIN */
        union /*_tagIcnW*/ {
#endif	/* TWIN */
            HICON       hIcon;
            LPCWSTR     pszIcon;
        } u;
        LPCWSTR         pszCaption;


        UINT            nPages;
#if 0	/* TWIN: Hack to allow g++ to recognize anonymous unions */
        union _tagPageIDW {
#else	/* TWIN */
        union /*_tagPageIDW*/ {
#endif	/* TWIN */
            UINT        nStartPage;
            LPCWSTR     pStartPage;
        } u2;
#if 0	/* TWIN: Hack to allow g++ to recognize anonymous unions */
        union _tagPagesW {
#else	/* TWIN */
        union /*_tagPagesW*/ {
#endif	/* TWIN */
            LPCPROPSHEETPAGEW ppsp;
            HPROPSHEETPAGE FAR *phpage;
        } u3;
        PFNPROPSHEETCALLBACK pfnCallback;
} PROPSHEETHEADERW, FAR *LPPROPSHEETHEADERW;
#endif /*!ANSI_ONLY */
#ifndef ANSI_ONLY
typedef const PROPSHEETHEADERW FAR *LPCPROPSHEETHEADERW;
#endif /*!ANSI_ONLY */

#ifdef UNICODE
#define PROPSHEETHEADER         PROPSHEETHEADERW
#define LPPROPSHEETHEADER       LPPROPSHEETHEADERW
#define LPCPROPSHEETHEADER      LPCPROPSHEETHEADERW
#else
#define PROPSHEETHEADER         PROPSHEETHEADERA
#define LPPROPSHEETHEADER       LPPROPSHEETHEADERA
#define LPCPROPSHEETHEADER      LPCPROPSHEETHEADERA
#endif


#define PSCB_INITIALIZED  1
#define PSCB_PRECREATE    2

#ifndef UNICODE_ONLY
/*WINCOMMCTRLAPI*/ HPROPSHEETPAGE WINAPI CreatePropertySheetPageA(LPCPROPSHEETPAGEA);
#endif /*!UNICODE_ONLY */
#ifndef ANSI_ONLY
/*WINCOMMCTRLAPI*/ HPROPSHEETPAGE WINAPI CreatePropertySheetPageW(LPCPROPSHEETPAGEW);
#endif /*!ANSI_ONLY */
/*WINCOMMCTRLAPI*/ BOOL           WINAPI DestroyPropertySheetPage(HPROPSHEETPAGE);
#ifndef UNICODE_ONLY
/*WINCOMMCTRLAPI*/ int            WINAPI PropertySheetA(LPCPROPSHEETHEADERA);
#endif /*!UNICODE_ONLY */
#ifndef ANSI_ONLY
/*WINCOMMCTRLAPI*/ int            WINAPI PropertySheetW(LPCPROPSHEETHEADERW);
#endif /*!ANSI_ONLY */

#ifdef UNICODE
#define CreatePropertySheetPage  CreatePropertySheetPageW
#define PropertySheet            PropertySheetW
#else
#define CreatePropertySheetPage  CreatePropertySheetPageA
#define PropertySheet            PropertySheetA
#endif



typedef BOOL (CALLBACK FAR * LPFNADDPROPSHEETPAGE)(HPROPSHEETPAGE, LPARAM);
typedef BOOL (CALLBACK FAR * LPFNADDPROPSHEETPAGES)(LPVOID, LPFNADDPROPSHEETPAGE, LPARAM);


typedef struct _PSHNOTIFY
{
    NMHDR hdr;
    LPARAM lParam;
} PSHNOTIFY, FAR *LPPSHNOTIFY;


#define PSN_FIRST               (0U-200U)
#define PSN_LAST                (0U-299U)


#define PSN_SETACTIVE           (PSN_FIRST-0)
#define PSN_KILLACTIVE          (PSN_FIRST-1)
/* #define PSN_VALIDATE            (PSN_FIRST-1) */
#define PSN_APPLY               (PSN_FIRST-2)
#define PSN_RESET               (PSN_FIRST-3)
/* #define PSN_CANCEL              (PSN_FIRST-3) */
#define PSN_HELP                (PSN_FIRST-5)
#define PSN_WIZBACK             (PSN_FIRST-6)
#define PSN_WIZNEXT             (PSN_FIRST-7)
#define PSN_WIZFINISH           (PSN_FIRST-8)
#define PSN_QUERYCANCEL         (PSN_FIRST-9)


#define PSNRET_NOERROR              0
#define PSNRET_INVALID              1
#define PSNRET_INVALID_NOCHANGEPAGE 2


#define PSM_SETCURSEL           (WM_USER + 101)
#define PropSheet_SetCurSel(hDlg, hpage, index) \
        SendMessage(hDlg, PSM_SETCURSEL, (WPARAM)index, (LPARAM)hpage)


#define PSM_REMOVEPAGE          (WM_USER + 102)
#define PropSheet_RemovePage(hDlg, index, hpage) \
        SendMessage(hDlg, PSM_REMOVEPAGE, index, (LPARAM)hpage)


#define PSM_ADDPAGE             (WM_USER + 103)
#define PropSheet_AddPage(hDlg, hpage) \
        SendMessage(hDlg, PSM_ADDPAGE, 0, (LPARAM)hpage)


#define PSM_CHANGED             (WM_USER + 104)
#define PropSheet_Changed(hDlg, hwnd) \
        SendMessage(hDlg, PSM_CHANGED, (WPARAM)hwnd, 0L)


#define PSM_RESTARTWINDOWS      (WM_USER + 105)
#define PropSheet_RestartWindows(hDlg) \
        SendMessage(hDlg, PSM_RESTARTWINDOWS, 0, 0L)


#define PSM_REBOOTSYSTEM        (WM_USER + 106)
#define PropSheet_RebootSystem(hDlg) \
        SendMessage(hDlg, PSM_REBOOTSYSTEM, 0, 0L)


#define PSM_CANCELTOCLOSE       (WM_USER + 107)
#define PropSheet_CancelToClose(hDlg) \
        SendMessage(hDlg, PSM_CANCELTOCLOSE, 0, 0L)


#define PSM_QUERYSIBLINGS       (WM_USER + 108)
#define PropSheet_QuerySiblings(hDlg, wParam, lParam) \
        SendMessage(hDlg, PSM_QUERYSIBLINGS, wParam, lParam)


#define PSM_UNCHANGED           (WM_USER + 109)
#define PropSheet_UnChanged(hDlg, hwnd) \
        SendMessage(hDlg, PSM_UNCHANGED, (WPARAM)hwnd, 0L)


#define PSM_APPLY               (WM_USER + 110)
#define PropSheet_Apply(hDlg) \
        SendMessage(hDlg, PSM_APPLY, 0, 0L)


#define PSM_SETTITLEA           (WM_USER + 111)
#define PSM_SETTITLEW           (WM_USER + 120)

#ifdef UNICODE
#define PSM_SETTITLE            PSM_SETTITLEW
#else
#define PSM_SETTITLE            PSM_SETTITLEA
#endif

#define PropSheet_SetTitle(hDlg, wStyle, lpszText)\
        SendMessage(hDlg, PSM_SETTITLE, wStyle, (LPARAM)(LPCTSTR)lpszText)


#define PSM_SETWIZBUTTONS       (WM_USER + 112)
#define PropSheet_SetWizButtons(hDlg, dwFlags) \
        PostMessage(hDlg, PSM_SETWIZBUTTONS, 0, (LPARAM)dwFlags)



#define PSWIZB_BACK             0x00000001
#define PSWIZB_NEXT             0x00000002
#define PSWIZB_FINISH           0x00000004
#define PSWIZB_DISABLEDFINISH   0x00000008


#define PSM_PRESSBUTTON         (WM_USER + 113)
#define PropSheet_PressButton(hDlg, iButton) \
        SendMessage(hDlg, PSM_PRESSBUTTON, (WPARAM)iButton, 0)


#define PSBTN_BACK              0
#define PSBTN_NEXT              1
#define PSBTN_FINISH            2
#define PSBTN_OK                3
#define PSBTN_APPLYNOW          4
#define PSBTN_CANCEL            5
#define PSBTN_HELP              6
#define PSBTN_MAX               6



#define PSM_SETCURSELID         (WM_USER + 114)
#define PropSheet_SetCurSelByID(hDlg, id) \
        SendMessage(hDlg, PSM_SETCURSELID, 0, (LPARAM)id)


#define PSM_SETFINISHTEXTA      (WM_USER + 115)
#define PSM_SETFINISHTEXTW      (WM_USER + 121)

#ifdef UNICODE
#define PSM_SETFINISHTEXT       PSM_SETFINISHTEXTW
#else
#define PSM_SETFINISHTEXT       PSM_SETFINISHTEXTA
#endif

#define PropSheet_SetFinishText(hDlg, lpszText) \
        SendMessage(hDlg, PSM_SETFINISHTEXT, 0, (LPARAM)lpszText)


#define PSM_GETTABCONTROL       (WM_USER + 116)
#define PropSheet_GetTabControl(hDlg) \
        (HWND)SendMessage(hDlg, PSM_GETTABCONTROL, 0, 0)

#define PSM_ISDIALOGMESSAGE     (WM_USER + 117)
#define PropSheet_IsDialogMessage(hDlg, pMsg) \
        (BOOL)SendMessage(hDlg, PSM_ISDIALOGMESSAGE, 0, (LPARAM)pMsg)

#define PSM_GETCURRENTPAGEHWND  (WM_USER + 118)
#define PropSheet_GetCurrentPageHwnd(hDlg) \
        (HWND)SendMessage(hDlg, PSM_GETCURRENTPAGEHWND, 0, 0L)

#define ID_PSRESTARTWINDOWS     0x2
#define ID_PSREBOOTSYSTEM       (ID_PSRESTARTWINDOWS | 0x1)


#define WIZ_CXDLG               276
#define WIZ_CYDLG               140

#define WIZ_CXBMP               80

#define WIZ_BODYX               92
#define WIZ_BODYCX              184

#define PROP_SM_CXDLG           212
#define PROP_SM_CYDLG           188

#define PROP_MED_CXDLG          227
#define PROP_MED_CYDLG          215

#define PROP_LG_CXDLG           252
#define PROP_LG_CYDLG           218

#endif /* (WINVER >= 0x0400) */

#ifdef __cplusplus
}
#endif

#endif
