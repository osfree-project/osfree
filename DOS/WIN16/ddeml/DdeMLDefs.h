/********************************************************************
   
	@(#)DdeMLDefs.h	1.3
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

********************************************************************/
 
#ifndef DdeML__h
#define DdeML__h

DECLARE_HANDLE32(DDEML_HANDLE);

static LRESULT ClientWndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT ServerWndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT DdeMLWndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT ConvListWndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT MonWndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT SubFrameWndProc(HWND, UINT, WPARAM, LPARAM);

#define	DDEMLCLASSEXTRA		sizeof(LONG)
#define	DDEMLCLIENTEXTRA	sizeof(LPSTR)
#define	DDEMLSERVEREXTRA	sizeof(LPSTR)
#define	DDEMLFRAMEEXTRA		sizeof(LONG)
#define	DDEMLMONITOREXTRA	0
#define	DDEMLHOLDEXTRA		sizeof(LPSTR)

#define	DDEMLINSTID_INDEX	0
#define	DDEMLHCONV_INDEX	0

static char DdeMLClassName[] = 	"DDEMLCLASS";
static char ClientClassName[] =	"DDEMLCLIENT";
static char ServerClassName[] =	"DDEMLSERVER";
static char FrameClassName[] = 	"DDEMLFRAME";
static char MonClassName[] =	"DDEMLMONITOR";
static char ConvListClassName[] = "DDEMLHOLD";

#define	DDEMLATOM_INDEXi	0
#define	CLIENTATOM_INDEX	1
#define	SERVERATOM_INDEX	2
#define	FRAMEATOM_INDEX		3
#define	MONATOM_INDEX		4
#define	CONVLISTATOM_INDEX	5

typedef struct tagDDEMLWNDCLASS
{
    WNDCLASS	WndClass;
    ATOM	atmClass;
} DDEMLWNDCLASS;

static DDEMLWNDCLASS DdeMLClasses[] = {
    {   {
	0,			/* style	*/
	(WNDPROC)DdeMLWndProc,	/* class proc	*/
	0,			/* class extra	*/
	DDEMLCLASSEXTRA,	/* window extra	*/
	(HINSTANCE)0,
	(HICON)0,
	(HCURSOR)0,
	(HBRUSH)0,
	(LPSTR)0,		/* menu name	*/
	DdeMLClassName,		/* class name	*/
	},
	0,
    },
    {   {
	0,			/* style	*/
	(WNDPROC)ClientWndProc,	/* class proc	*/
	0,			/* class extra	*/
	DDEMLCLIENTEXTRA,	/* window extra	*/
	(HINSTANCE)0,
	(HICON)0,
	(HCURSOR)0,
	(HBRUSH)0,
	(LPSTR)0,		/* menu name	*/
	ClientClassName,	/* class name	*/
	},
	0,
    },
    {   {
	0,			/* style	*/
	(WNDPROC)ServerWndProc,	/* class proc	*/
	0,			/* class extra	*/
	DDEMLSERVEREXTRA,	/* window extra	*/
	(HINSTANCE)0,
	(HICON)0,
	(HCURSOR)0,
	(HBRUSH)0,
	(LPSTR)0,		/* menu name	*/
	ServerClassName,	/* class name	*/
	},
	0,
    },
    {   {
	0,			/* style	*/
	(WNDPROC)SubFrameWndProc, /* class proc	*/
	0,			/* class extra	*/
	DDEMLFRAMEEXTRA,	/* window extra	*/
	(HINSTANCE)0,
	(HICON)0,
	(HCURSOR)0,
	(HBRUSH)0,
	(LPSTR)0,		/* menu name	*/
	FrameClassName,		/* class name	*/
	},
	0,
    },
    {   {
	0,			/* style	*/
	(WNDPROC)MonWndProc,	/* class proc	*/
	0,			/* class extra	*/
	DDEMLMONITOREXTRA,	/* window extra	*/
	(HINSTANCE)0,
	(HICON)0,
	(HCURSOR)0,
	(HBRUSH)0,
	(LPSTR)0,		/* menu name	*/
	MonClassName,		/* class name	*/
	},
	0,
    },
    {   {
	0,			/* style	*/
	(WNDPROC)ConvListWndProc, /* class proc	*/
	0,			/* class extra	*/
	DDEMLHOLDEXTRA,		/* window extra	*/
	(HINSTANCE)0,
	(HICON)0,
	(HCURSOR)0,
	(HBRUSH)0,
	(LPSTR)0,		/* menu name	*/
	ConvListClassName,	/* class name	*/
	},
	0,
    },
    {   {
	0,			/* style	*/
	(WNDPROC)0,		/* class proc	*/
	0,			/* class extra	*/
	0,			/* window extra	*/
	(HINSTANCE)0,
	(HICON)0,
	(HCURSOR)0,
	(HBRUSH)0,
	(LPSTR)0,		/* menu name	*/
	(LPSTR)0,		/* class name	*/
	},
	0,
    }
};

#define	FEATURE_NOT_SUPPORTED(m,s) MessageBox((HWND)0,s,m,MB_OK|MB_ICONSTOP)

/* definitions for profile access */

static char DdeMLSection[] = "DDEML";

#define	SHUTDOWNTIMEOUT_DEFAULT		3000
static char ShutdownTimeout_String[] = "ShutdownTimeout";
static char ShutdownTimeout_Default[] = "3000";
static int ShutdownTimeout = SHUTDOWNTIMEOUT_DEFAULT;

#define	SHUTDOWNRETRYTIMEOUT_DEFAULT	30000
static char ShutdownRetryTimeout_String[] = "ShutdownRetryTimeout";
static char ShutdownRetryTimeout_Default[] = "30000";
static int ShutdownRetryTimeout = SHUTDOWNRETRYTIMEOUT_DEFAULT;

typedef struct tagDDEMLSERVICE
{
    struct tagDDEMLSERVICE *lpNextService;
    HSZ		hszService;
} DDEMLSERVICE;
typedef DDEMLSERVICE *LPDDEMLSERVICE;

/* magic value for idInst (kept in HIWORD) */
#define	DDEML_INSTID_MAGIC	0xDDE0

#define	DDEML_MAX_INSTANCES	32

typedef struct tagDDEMLINSTANCE
{
    UINT	uiIndex;
    DWORD	dwFlags;
    LPDDEMLSERVICE lpService;
    PFNCALLBACK lpfnCallback;
    UINT	LastError;
    HWND	hWndDdeML;
    HTASK	hTask;
} DDEMLINSTANCE;

typedef DDEMLINSTANCE *LPDDEMLINSTANCE;

static DDEMLINSTANCE DdeMLInstances[DDEML_MAX_INSTANCES];

static BOOL DdeMLInitialized = FALSE;

typedef struct tagDDEMLDATA
{
    LPDDEMLINSTANCE	lpInst;
    UINT		uFlags;
    HGLOBAL		hGlobal;
} DDEMLDATA;

typedef DDEMLDATA *LPDDEMLDATA;

#define	DDEML_HCONV_MAGIC	0xDDECDDEC
#define	DDEML_HCONVLIST_MAGIC	0xDDECDDED
typedef struct tagDDEMLCONV
{
    CONVINFO	ConvInfo;
    struct tagDDEMLCONV	*lpNextConv;
    UINT	uMagic;
    DWORD	idInst;
    HWND	hWndConv;
} DDEMLCONV;

typedef DDEMLCONV *LPDDEMLCONV;

#ifdef 0
static DWORD DdeTransactionFailMasks[] =
  {
	0,				/* XTYP_ERROR		*/
	0,				/* XTYP_ADVDATA		*/
	0,				/* XTYP_ADVREQ		*/
	CBF_FAIL_ADVISES,		/* XTYP_ADVSTART	*/
	0,				/* XTYP_ADVSTOP		*/
	CBF_FAIL_EXECUTES,		/* XTYP_EXECUTE		*/
	CBF_FAIL_CONNECTIONS,		/* XTYP_CONNECT		*/
	CBF_SKIP_CONNECT_CONFIRMS,	/* XTYP_CONNECT_CONFIRM */
	0,				/* XTYP_XACT_COMPLETE	*/
	CBF_FAIL_POKES,			/* XTYP_POKE		*/
	CBF_SKIP_REGISTRATIONS,		/* XTYP_REGISTER	*/
	CBF_FAIL_REQUESTS,		/* XTYP_REQUEST		*/
	CBF_SKIP_DISCONNECTS,		/* XTYP_DISCONNECT	*/
	CBF_SKIP_UNREGISTRATIONS,	/* XTYP_UNREGISTER	*/
	CBF_FAIL_CONNECTIONS,		/* XTYP_WILDCONNECT	*/
  }; 
#endif

#endif /* DdeML__h */
