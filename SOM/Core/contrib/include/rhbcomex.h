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

#ifndef _RHBCOMEX_H_
#define _RHBCOMEX_H_

#ifndef BEGIN_INTERFACE
#	ifdef __APPLE__
#		define BEGIN_INTERFACE    void *_reserved;
#	else
#		define BEGIN_INTERFACE
#	endif
#endif

#ifndef END_INTERFACE
#	define END_INTERFACE
#endif

#ifdef __APPLE__
/*
 *  Apple ABI defines an additional _reserved pointer at the start of the Vtbl
 */
#	define RHBCOM_IUNKNOWN_PREFIX		NULL,
#else
#	define RHBCOM_IUNKNOWN_PREFIX
#endif

#ifdef IsEqualIID
	#undef IsEqualIID
#endif

/* 
 * designed to work with REFIID 
 */
#ifdef __APPLE__
#	define IsEqualIID(x,y)   ((				\
		((x).byte0==(y).byte0) &&			\
		((x).byte1==(y).byte1) &&			\
		((x).byte2==(y).byte2) &&			\
		((x).byte3==(y).byte3) &&			\
		((x).byte4==(y).byte4) &&			\
		((x).byte5==(y).byte5) &&			\
		((x).byte6==(y).byte6) &&			\
		((x).byte7==(y).byte7) &&			\
		((x).byte8==(y).byte8) &&			\
		((x).byte9==(y).byte9) &&			\
		((x).byte10==(y).byte10) &&			\
		((x).byte11==(y).byte11) &&			\
		((x).byte12==(y).byte12) &&			\
		((x).byte13==(y).byte13) &&			\
		((x).byte14==(y).byte14) &&			\
		((x).byte15==(y).byte15)			\
		) ? 1 : 0)
#else
#	define IsEqualIID(x,y)   ((				\
		((x)->Data1==(y)->Data1) &&			\
		((x)->Data2==(y)->Data2) &&			\
		((x)->Data3==(y)->Data3) &&			\
		((x)->Data4[0]==(y)->Data4[0]) &&	\
		((x)->Data4[1]==(y)->Data4[1]) &&	\
		((x)->Data4[2]==(y)->Data4[2]) &&	\
		((x)->Data4[3]==(y)->Data4[3]) &&	\
		((x)->Data4[4]==(y)->Data4[4]) &&	\
		((x)->Data4[5]==(y)->Data4[5]) &&	\
		((x)->Data4[6]==(y)->Data4[6]) &&	\
		((x)->Data4[7]==(y)->Data4[7])		\
		) ? 1 : 0)
#endif

#ifdef __APPLE__
#	define RHBCOM_COPY_GUID(tgt,src)		\
	{	(tgt).byte0=(src).byte0;			\
		(tgt).byte1=(src).byte1;			\
		(tgt).byte2=(src).byte2;			\
		(tgt).byte3=(src).byte3;			\
		(tgt).byte4=(src).byte4;			\
		(tgt).byte5=(src).byte5;			\
		(tgt).byte6=(src).byte6;			\
		(tgt).byte7=(src).byte7;			\
		(tgt).byte8=(src).byte8;			\
		(tgt).byte9=(src).byte9;			\
		(tgt).byte10=(src).byte10;			\
		(tgt).byte11=(src).byte11;			\
		(tgt).byte12=(src).byte12;			\
		(tgt).byte13=(src).byte13;			\
		(tgt).byte14=(src).byte14;			\
		(tgt).byte15=(src).byte15; }
#else
#	define RHBCOM_COPY_GUID(tgt,src)						\
	{	int i=sizeof((tgt).Data4)/sizeof((tgt).Data4[0]);	\
		(tgt).Data1=(src).Data1;							\
		(tgt).Data2=(src).Data2;							\
		(tgt).Data3=(src).Data3;							\
		while (i--) (tgt).Data4[i]=(src).Data4[i]; }
#endif

#define RHBCOM_EMBEDDED_INTERFACE(i)    i com__##i;  
#define RHBCOM_CAST(p,i)				&(p)->com__##i
#define RHBCOM_OFFSET(c,i)				(int)(size_t)&((c *)0)->com__##i
#define RHBCOM_this_CAST(c,i,p)			((c *)(((char *)(p))-RHBCOM_OFFSET(c,i)))
#if _DEBUG
	#define RHBCOM_THIS_CAST(c,i,p)			((p==&RHBCOM_this_CAST(c,i,p)->com__##i) ? RHBCOM_this_CAST(c,i,p) : NULL)
#else
	#define RHBCOM_THIS_CAST(c,i,p)			RHBCOM_this_CAST(c,i,p)
#endif

#ifdef __APPLE__
#	define RHBCOM_REFIID(ifd)		IID_##ifd
#	define RHBCOM_REFCLSID(cld)		CLSID_##cld
#else
#	define RHBCOM_REFIID(ifd)		&IID_##ifd
#	define RHBCOM_REFCLSID(cld)		&CLSID_##cld
#endif

#define RHBCOM_INIT_VTBL(p,c,i)   p->com__##i.lpVtbl=&c##_##i##_Vtbl

#define RHBCOM_QI_BEGIN  { IUnknown *ref=NULL; *ppvObject=NULL;
#define RHBCOM_QI(ic,ir) if (IsEqualIID(riid,RHBCOM_REFIID(ic))) { ref=(void *)&self->com__##ir; } else
#define RHBCOM_QI_END	   ref=NULL; if (ref && ref->lpVtbl) { IUnknown_AddRef(ref); *ppvObject=ref; return S_OK; } }

#define RHBCOM_DELETE(x)		if (x) { IMalloc *a=x->alloc; a->lpVtbl->Free(a,x); a->lpVtbl->Release(a); }

#define RHBCOM_VTBL_BEGIN(c,i) static i##Vtbl c##_##i##_Vtbl={ RHBCOM_IUNKNOWN_PREFIX \
		c##_##i##_QueryInterface,c##_##i##_AddRef,c##_##i##_Release
#define RHBCOM_VTBL_ENTRY(c,i,m)  ,c##_##i##_##m
#define RHBCOM_VTBL_END };

#define RHBCOM_IUNKNOWN_DELEGATE(cls,iface)															\
	static STDMETHODIMP cls##_##iface##_QueryInterface(iface *pThis,REFIID riid,void **ppvObject)	\
	{ cls *self=RHBCOM_THIS_CAST(cls,iface,pThis);													\
	return IUnknown_QueryInterface(self->outer,riid,ppvObject); }									\
	static STDMETHODIMP_(ULONG) cls##_##iface##_AddRef(iface *pThis)								\
	{ cls *self=RHBCOM_THIS_CAST(cls,iface,pThis);													\
	return IUnknown_AddRef(self->outer); }															\
	static STDMETHODIMP_(ULONG) cls##_##iface##_Release(iface *pThis)								\
	{ cls *self=RHBCOM_THIS_CAST(cls,iface,pThis);													\
	return IUnknown_Release(self->outer); }												

#endif
