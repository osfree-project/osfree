/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/*
 * $Id$
 */

#include <rhbopt.h>
#include <som.h>
#include <somd.h>
#include <rhbsomdd.h>
#include <objbase.h>

typedef struct ACTRL_ACCESSW *PACTRL_ACCESSW;
typedef struct ACTRL_ACCESSW_ALLOCATE_ALL_NODES *PACTRL_ACCESSW_ALLOCATE_ALL_NODES;
typedef struct TRUSTEEW TRUSTEEW,*PTRUSTEEW;
typedef ULONG ACCESS_RIGHTS;
typedef struct IAccessControl
{
	struct IAccessControlVtbl *lpVtbl;
} IAccessControl;

typedef struct IAccessControlVtbl
{
HRESULT (STDMETHODCALLTYPE *QueryInterface)(IAccessControl *,REFIID,void **);
DWORD (STDMETHODCALLTYPE *AddRef)(IAccessControl *);
DWORD (STDMETHODCALLTYPE *Release)(IAccessControl *);
HRESULT (STDMETHODCALLTYPE *GrantAccessRights)(IAccessControl *,PACTRL_ACCESSW pAccessList);
HRESULT (STDMETHODCALLTYPE *SetAccessRights)(IAccessControl *,PACTRL_ACCESSW pAccessList);
HRESULT (STDMETHODCALLTYPE *SetOwner)(IAccessControl *,PTRUSTEEW pOwner,PTRUSTEEW pGroup);
HRESULT (STDMETHODCALLTYPE *RevokeAccessRights)(IAccessControl *,LPWSTR lpProperty,ULONG cTrustees,TRUSTEEW prgTrustees[  ]);
HRESULT (STDMETHODCALLTYPE *GetAllAccessRights)(IAccessControl *,LPWSTR lpProperty,PACTRL_ACCESSW_ALLOCATE_ALL_NODES __RPC_FAR *ppAccessList,PTRUSTEEW __RPC_FAR *ppOwner,PTRUSTEEW __RPC_FAR *ppGroup);
HRESULT (STDMETHODCALLTYPE *IsAccessAllowed)(IAccessControl *,PTRUSTEEW pTrustee,LPWSTR lpProperty,ACCESS_RIGHTS AccessRights,BOOL __RPC_FAR *pfAccessAllowed);
} IAccessControlVtbl;

static HRESULT STDMETHODCALLTYPE acc_QueryInterface(IAccessControl *,REFIID,void **);
static DWORD STDMETHODCALLTYPE acc_AddRef(IAccessControl *);
static DWORD STDMETHODCALLTYPE acc_Release(IAccessControl *);
static HRESULT STDMETHODCALLTYPE acc_GrantAccessRights(IAccessControl *,PACTRL_ACCESSW pAccessList);
static HRESULT STDMETHODCALLTYPE acc_SetAccessRights(IAccessControl *,PACTRL_ACCESSW pAccessList);
static HRESULT STDMETHODCALLTYPE acc_SetOwner(IAccessControl *,PTRUSTEEW pOwner,PTRUSTEEW pGroup);
static HRESULT STDMETHODCALLTYPE acc_RevokeAccessRights(IAccessControl *,LPWSTR lpProperty,ULONG cTrustees,TRUSTEEW prgTrustees[  ]);
static  HRESULT STDMETHODCALLTYPE acc_GetAllAccessRights(IAccessControl *,LPWSTR lpProperty,PACTRL_ACCESSW_ALLOCATE_ALL_NODES __RPC_FAR *ppAccessList,PTRUSTEEW __RPC_FAR *ppOwner,PTRUSTEEW __RPC_FAR *ppGroup);
static  HRESULT STDMETHODCALLTYPE acc_IsAccessAllowed(IAccessControl *,PTRUSTEEW pTrustee,LPWSTR lpProperty,ACCESS_RIGHTS AccessRights,BOOL __RPC_FAR *pfAccessAllowed);

static IAccessControlVtbl accessVtbl=
{
	acc_QueryInterface,
	acc_AddRef,
	acc_Release,
	acc_GrantAccessRights,
	acc_SetAccessRights,
	acc_SetOwner,
	acc_RevokeAccessRights,
	acc_GetAllAccessRights,
	acc_IsAccessAllowed
};

typedef struct CAccessControl
{
	IAccessControlVtbl *lpVtbl;
	long lUsage;
} CAccessControl;

static CAccessControl acc={&accessVtbl,0};

static IAccessControl *get_access(void)
{
	return (void *)&acc;
}

const IID IID_IAccessControl={0xEEDD23E0,0x8410,0x11CE,{0xA1,0xC3,0x08,0x00,0x2B,0x2B,0x8D,0x8F}};

static HRESULT STDMETHODCALLTYPE acc_QueryInterface(IAccessControl *ref,REFIID iid,void **ppv)
{
	IUnknown *res=NULL;

	if (!memcmp(iid,&IID_IUnknown,sizeof(IID_IUnknown))) res=(IUnknown *)ref;
	if (!memcmp(iid,&IID_IAccessControl,sizeof(IID_IAccessControl))) res=(IUnknown *)ref;

	if (res)
	{
		*ppv=res;
		res->lpVtbl->AddRef(res);
		return 0;
	}

	return E_NOINTERFACE;
}

static DWORD STDMETHODCALLTYPE acc_AddRef(IAccessControl *ref)
{
	CAccessControl *self=(void *)ref;
	DWORD dw=InterlockedIncrement(&self->lUsage);
	return dw;
}

static DWORD STDMETHODCALLTYPE acc_Release(IAccessControl *ref)
{
	CAccessControl *self=(void *)ref;
	DWORD dw=InterlockedDecrement(&self->lUsage);
	return dw;
}

static HRESULT STDMETHODCALLTYPE acc_GrantAccessRights(IAccessControl *ref,PACTRL_ACCESSW pAccessList)
{
	return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE acc_SetAccessRights(IAccessControl *ref,PACTRL_ACCESSW pAccessList)
{
	return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE acc_SetOwner(IAccessControl *ref,PTRUSTEEW pOwner,PTRUSTEEW pGroup)
{
	return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE acc_RevokeAccessRights(IAccessControl *ref,LPWSTR lpProperty,ULONG cTrustees,TRUSTEEW prgTrustees[  ])
{
	return E_NOTIMPL;
}

static  HRESULT STDMETHODCALLTYPE acc_GetAllAccessRights(IAccessControl *ref,LPWSTR lpProperty,PACTRL_ACCESSW_ALLOCATE_ALL_NODES __RPC_FAR *ppAccessList,PTRUSTEEW __RPC_FAR *ppOwner,PTRUSTEEW __RPC_FAR *ppGroup)
{
	return E_NOTIMPL;
}

static  HRESULT STDMETHODCALLTYPE acc_IsAccessAllowed(IAccessControl *ref,PTRUSTEEW pTrustee,LPWSTR lpProperty,ACCESS_RIGHTS AccessRights,BOOL *pfAccessAllowed)
{
	*pfAccessAllowed=1;
	return S_OK;
}

#ifndef EOAC_ACCESS_CONTROL
#define EOAC_ACCESS_CONTROL  0x4
HRESULT __stdcall CoInitializeSecurity(
		void *a,
		long b,
		void *c,
		void *d,
		DWORD e,
		DWORD f,
		void *g,
		DWORD h,
		void *i)
{
	typedef HRESULT (__stdcall *proc_t)(
		void *a,
		long b,
		void *c,
		void *d,
		DWORD e,
		DWORD f,
		void *g,
		DWORD h,
		void *i);
	proc_t proc=(proc_t)GetProcAddress(GetModuleHandle("OLE32"),"CoInitializeSecurity");
	if (proc) return proc(a,b,c,d,e,f,g,h,i);
	return E_NOTIMPL;
}
#endif

long SOMDD_init_security(void)
{
	HRESULT r=CoInitializeSecurity(
		get_access(),
		-1,NULL, /* let the thing choose */
		NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT /* 0 */, /* as server */
		0 /*RPC_C_IMP_LEVEL_DEFAULT*/, /* as client */
		NULL,
		EOAC_ACCESS_CONTROL /* 0x4 */,
		NULL);

	return r;
}

