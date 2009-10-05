/*    
	O2CompObj.c	1.7
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

 */
/*
STDAPI  CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved, 
STDAPI CoMarshalInterface(LPSTREAM pStm, REFIID riid, LPUNKNOWN pUnk, 
STDAPI CoUnmarshalInterface(LPSTREAM pStm, REFIID riid, LPVOID FAR* ppv); 
STDAPI CoMarshalHresult(LPSTREAM pstm, HRESULT hresult); 
STDAPI CoUnmarshalHresult(LPSTREAM pstm, HRESULT FAR * phresult); 
STDAPI CoReleaseMarshalData(LPSTREAM pStm); 
STDAPI CoGetStandardMarshal(REFIID riid, LPUNKNOWN pUnk,  
STDAPI_(BOOL) CoIsHandlerConnected(LPUNKNOWN pUnk); 
STDAPI_(BOOL) CoIsOle1Class(REFCLSID rclsid); 
STDAPI CoGetTreatAsClass(REFCLSID clsidOld, LPCLSID pClsidNew); 
STDAPI CoTreatAsClass(REFCLSID clsidOld, REFCLSID clsidNew); 
STDAPI  DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv); 
STDAPI  DllCanUnloadNow(void); 
STDAPI FnAssert(LPSTR lpstrExpr, LPSTR lpstrMsg, LPSTR lpstrFileName, UINT iLine); 
*/

#include "windows.h"
#include "windowsx.h"

#define	INITGUID
#include "compobj.h"
/* INCLUDE #include "coguid.h" */
#define __COGUID_H__
#include "Willows.h"
#undef	INITGUID

#include "ole2ver.h"

typedef struct tagIUnknownObj
{
	LPUNKNOWN	pIUnknown;
	DWORD		dwRefCount;
	IID		iid;
} IUnknownObj;

typedef struct tagIMallocObj
{
	LPMALLOC	pIMalloc;
	DWORD		dwRefCount;
	IID		iid;
} IMallocObj;
typedef IMallocObj *LPMALLOCOBJ;

static HRESULT COStdMalloc_QueryInterface(LPMALLOCOBJ, REFIID, LPVOID *);
static ULONG   COStdMalloc_AddRef(LPMALLOCOBJ);
static ULONG   COStdMalloc_Release(LPMALLOCOBJ);

static LPVOID  COStdMalloc_Alloc(LPMALLOCOBJ, ULONG);
static LPVOID  COStdMalloc_Realloc(LPMALLOCOBJ, LPVOID, ULONG);
static void    COStdMalloc_Free(LPMALLOCOBJ, LPVOID);
static ULONG   COStdMalloc_GetSize(LPMALLOCOBJ, LPVOID);
static int     COStdMalloc_DidAlloc(LPMALLOCOBJ, LPVOID);
static void    COStdMalloc_HeapMinimize(LPMALLOCOBJ);

static IMallocVtbl COStdMallocVtbl = 
    {
	(HRESULT (*)())COStdMalloc_QueryInterface,
	(ULONG (*)())COStdMalloc_AddRef,
	(ULONG (*)())COStdMalloc_Release,

	(LPVOID (*)())COStdMalloc_Alloc,
	(LPVOID (*)())COStdMalloc_Realloc,
	(void (*)())COStdMalloc_Free,
	(ULONG (*)())COStdMalloc_GetSize,
	(int (*)())COStdMalloc_DidAlloc,
	(void (*)())COStdMalloc_HeapMinimize
    };

static IMalloc COStdMalloc = 
    { &COStdMallocVtbl };

LPMALLOC pCOStdMalloc = 0;

static HRESULT COShrdMalloc_QueryInterface(LPMALLOCOBJ, REFIID, LPVOID *);
static ULONG   COShrdMalloc_AddRef(LPMALLOCOBJ);
static ULONG   COShrdMalloc_Release(LPMALLOCOBJ);

static LPVOID  COShrdMalloc_Alloc(LPMALLOCOBJ, ULONG);
static LPVOID  COShrdMalloc_Realloc(LPMALLOCOBJ, LPVOID, ULONG);
static void    COShrdMalloc_Free(LPMALLOCOBJ, LPVOID);
static ULONG   COShrdMalloc_GetSize(LPMALLOCOBJ, LPVOID);
static int     COShrdMalloc_DidAlloc(LPMALLOCOBJ, LPVOID);
static void    COShrdMalloc_HeapMinimize(LPMALLOCOBJ);

static IMallocVtbl COShrdMallocVtbl = 
    {
	(HRESULT (*)())COShrdMalloc_QueryInterface,
	(ULONG (*)())COShrdMalloc_AddRef,
	(ULONG (*)())COShrdMalloc_Release,

	(LPVOID (*)())COShrdMalloc_Alloc,
	(LPVOID (*)())COShrdMalloc_Realloc,
	(void (*)())COShrdMalloc_Free,
	(ULONG (*)())COShrdMalloc_GetSize,
	(int (*)())COShrdMalloc_DidAlloc,
	(void (*)())COShrdMalloc_HeapMinimize
    };

static IMalloc COShrdMalloc = 
    { &COShrdMallocVtbl };

LPMALLOC pCOShrdMalloc = &COShrdMalloc;

/*******************************************************/
/* Version/Init/Uninit                                 */
/*******************************************************/

STDAPI_(DWORD)
CoBuildVersion(void)
{
    return MAKELONG(rup,rmm);
}

STDAPI
CoInitialize(LPMALLOC pMalloc)
{
    if (pCOStdMalloc != 0)
	return S_OK;

    pCOStdMalloc = (pMalloc)?pMalloc:&COStdMalloc;

    return S_OK;
}

STDAPI_(void)
CoUninitialize(void)
{
}

STDAPI_(DWORD)
CoGetCurrentProcess(void)
{
    return 0L;
}

/*******************************************************/
/* Malloc stuff                                        */
/*******************************************************/

STDAPI
CoCreateStandardMalloc(DWORD memctx, IMalloc FAR* FAR* ppMalloc)
{
    return ResultFromScode(E_INVALIDARG);
}

STDAPI
CoGetMalloc(DWORD dwMemContext, LPMALLOC *ppMalloc)
{
    SCODE sCode;

    if (pCOStdMalloc == 0)
	return ResultFromScode(CO_E_NOTINITIALIZED);

    switch (dwMemContext) {
	case (DWORD)MEMCTX_TASK:
	    *ppMalloc = pCOStdMalloc;
	    sCode = S_OK;
	    break;
	case (DWORD)MEMCTX_SHARED:
	    *ppMalloc = pCOShrdMalloc;
	    sCode = S_OK;
	    break;
	default:
	    sCode = E_INVALIDARG;
	    break;
    }

    return ResultFromScode(sCode);
}

static HRESULT
COStdMalloc_QueryInterface(LPMALLOCOBJ This, REFIID refiid, LPVOID *ppvObj)
{
   return (HRESULT) 0;	/* just to satisfy my compiler */
}

static ULONG
COStdMalloc_AddRef(LPMALLOCOBJ This)
{
    return ++This->dwRefCount;
}

static ULONG
COStdMalloc_Release(LPMALLOCOBJ This)
{
   return (ULONG) 0;	/* just to satisfy my compiler */

}

static LPVOID
COStdMalloc_Alloc(LPMALLOCOBJ This, ULONG cb)
{
   return (LPVOID) 0;	/* just to satisfy my compiler */
}

static LPVOID
COStdMalloc_Realloc(LPMALLOCOBJ This, LPVOID pv, ULONG cb)
{
   return (LPVOID) 0;	/* just to satisfy my compiler */
}

static void
COStdMalloc_Free(LPMALLOCOBJ This, LPVOID pv)
{
}

static ULONG
COStdMalloc_GetSize(LPMALLOCOBJ This, LPVOID pv)
{
   return (ULONG) 0;	/* just to satisfy my compiler */
}

static int
COStdMalloc_DidAlloc(LPMALLOCOBJ This, LPVOID pv)
{
   return (int) 0;	/* just to satisfy my compiler */
}

static void
COStdMalloc_HeapMinimize(LPMALLOCOBJ This)
{
}

static HRESULT
COShrdMalloc_QueryInterface(LPMALLOCOBJ This, REFIID refiid, LPVOID *ppvObj)
{
   return (HRESULT) 0;	/* just to satisfy my compiler */
}

static ULONG
COShrdMalloc_AddRef(LPMALLOCOBJ This)
{
   return (ULONG) 0;	/* just to satisfy my compiler */
}

static ULONG
COShrdMalloc_Release(LPMALLOCOBJ This)
{
   return (ULONG) 0;	/* just to satisfy my compiler */
}

static LPVOID
COShrdMalloc_Alloc(LPMALLOCOBJ This, ULONG cb)
{
   return (LPVOID) 0;	/* just to satisfy my compiler */
}

static LPVOID
COShrdMalloc_Realloc(LPMALLOCOBJ This, LPVOID pv, ULONG cb)
{
   return (LPVOID) 0;	/* just to satisfy my compiler */
}

static void
COShrdMalloc_Free(LPMALLOCOBJ This, LPVOID pv)
{
}

static ULONG
COShrdMalloc_GetSize(LPMALLOCOBJ This, LPVOID pv)
{
   return (ULONG) 0;	/* just to satisfy my compiler */
}

static int
COShrdMalloc_DidAlloc(LPMALLOCOBJ This, LPVOID pv)
{
   return (int) 0;	/* just to satisfy my compiler */
}

static void
COShrdMalloc_HeapMinimize(LPMALLOCOBJ This)
{
}

/***********************************************************/
/* CLSID/GUID <-> String                                   */
/***********************************************************/

static HRESULT CLSIDFromOle1Class(LPSTR,LPCLSID);
static BOOL GUIDFromString(LPSTR,LPGUID);

STDAPI
CLSIDFromString(LPSTR lpsz, LPCLSID pclsid)
{
    if (IsBadWritePtr((LPVOID)pclsid,sizeof(CLSID)))
	return ResultFromScode(E_INVALIDARG);

    if (!lpsz) {
	*pclsid = GUID_NULL;
	return ResultFromScode(S_OK);
    }

    if (IsBadReadPtr(lpsz,1))
	return ResultFromScode(E_INVALIDARG);

    if (lpsz[0] != '{')
	return CLSIDFromOle1Class(lpsz,pclsid);

    if (GUIDFromString(lpsz,(LPGUID)pclsid))
	return ResultFromScode(S_OK);
    else
	return ResultFromScode(CO_E_CLASSSTRING);
}

static HRESULT
CLSIDFromOle1Class(LPSTR lpsz, LPCLSID pclsid)
{
    return ResultFromScode(E_INVALIDARG);
}

static BOOL
GUIDFromString(LPSTR lpsz, LPGUID pguid)
{
    return FALSE;
}

STDAPI_(BOOL)
IsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
    return FALSE;
}

STDAPI
StringFromCLSID(REFCLSID rclsid, LPSTR FAR* lplpsz)
{
    return ResultFromScode(E_INVALIDARG);
}

STDAPI
StringFromIID(REFIID rclsid, LPSTR FAR* lplpsz)
{
    return ResultFromScode(E_INVALIDARG);
}

STDAPI
IIDFromString(LPSTR lpsz, LPIID lpiid)
{
    return ResultFromScode(E_INVALIDARG);
}

STDAPI
ProgIDFromCLSID(REFCLSID clsid, LPSTR FAR* lplpszProgID)
{
    return ResultFromScode(E_INVALIDARG);
}

STDAPI
CLSIDFromProgID(LPCSTR lpszProgID, LPCLSID lpclsid)
{
    return ResultFromScode(E_INVALIDARG);
}

STDAPI_(int)
StringFromGUID2(REFGUID rguid, LPSTR lpsz, int cbMax)
{
    return 0;
}

STDAPI
CoCreateGuid(GUID FAR *pguid)
{
    return ResultFromScode(E_INVALIDARG);
}

/***********************************************************/
/* FileTime APIs                                           */
/***********************************************************/

STDAPI_(BOOL)
CoFileTimeToDosDateTime(FILETIME FAR* lpFileTime,
			LPWORD lpDosDate, LPWORD lpDosTime)
{
    return FALSE;
}

STDAPI_(BOOL)
CoDosDateTimeToFileTime(WORD nDosDate, WORD nDosTime,
			FILETIME FAR* lpFileTime)
{
    return FALSE;
}

STDAPI
CoFileTimeNow( FILETIME FAR* lpFileTime )
{
    return ResultFromScode(E_INVALIDARG);
}

/***********************************************************/
/* Library management                                      */
/***********************************************************/

STDAPI_(HINSTANCE)
CoLoadLibrary(LPSTR lpszLibName, BOOL bAutoFree)
{
    return (HINSTANCE)0;
}

STDAPI_(void)
CoFreeLibrary(HINSTANCE hInst)
{
}

STDAPI_(void)
CoFreeAllLibraries(void)
{
}

STDAPI_(void)
CoFreeUnusedLibraries(void)
{
}

/***********************************************************/
/* IClassFactory stuff                                     */
/***********************************************************/

STDAPI
CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, 
		DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv)
{
    return ResultFromScode(E_INVALIDARG);
}
STDAPI
CoRegisterClassObject(REFCLSID rclsid, LPUNKNOWN pUnk, 
		DWORD dwClsContext, DWORD flags, LPDWORD lpdwRegister)
{
    return ResultFromScode(E_INVALIDARG);
}

STDAPI
CoRevokeClassObject(DWORD dwRegister)
{
    return ResultFromScode(E_INVALIDARG);
}

STDAPI
CoLockObjectExternal(LPUNKNOWN pUnk, BOOL fLock, BOOL fLastUnlockReleases)
{
    return ResultFromScode(E_INVALIDARG);
}

/***********************************************************/
/* Custom marshaling                                       */
/***********************************************************/

STDAPI
CoDisconnectObject(LPUNKNOWN pUnk, DWORD dwReserved)
{
    return ResultFromScode(E_INVALIDARG);
}

/***********************************************************/
/* IMEssageFilter                                          */
/***********************************************************/

STDAPI
CoRegisterMessageFilter( LPMESSAGEFILTER lpMessageFilter, 
			LPMESSAGEFILTER FAR* lplpMessageFilter )
{
    return ResultFromScode(S_FALSE);
}
