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

/* $Id$ */

#include <rhbopt.h>

#define SOMTHUNK_FASTCALL
#define SOMLINK_STDCALL

#ifdef SOMTHUNK_FASTCALL
#	define NEED_SOMKERN_RESOLVE_INDEX_MI
#endif

#include <somkern.h>

#ifdef _WIN32
#else
	#error should be win32 only
#endif

static void 
#	ifdef SOMLINK_STDCALL
__stdcall 
#	else
__cdecl
#	endif
test_func(int x)
{
}

static void (SOMLINK *callstyle_test)(int)=test_func;

#ifdef SOMLINK_STDCALL
	#define ASM_REMOVE_ARGS(n)
#else
	#define ASM_REMOVE_ARGS(n)		add	esp,n
#endif

/* welcome to the WIN32/i386 __stdcall rollercoaster

  this file implements the WIN32 ABI compatible thunking layer
  for i386 processors

  this file exists for a number of reasons

  1. IBM's SOM.DLL for WIN32/i386 and OS2/i386 supports native thunking, 
		so in order to use objects compiled using IBM's SC we 
		have to be able to support this calling mechanism

  2. to prove that the implementation can support thunking
		as long as someone has the time and inclination to
		sort out the details

  3. note, you can't just convert this to 'gas' and use it
		on Linux, NetBSD etc because the ABI on those platforms
		is different to WIN32/i386, __stdcall is right to left push with
		callee popping off the args from the stack, whereas __cdecl
		has the caller popping off the args
		also return of structs has special rules in WIN32/i386

  4. I don't encourage the use of assembler on anything other than
	  i386/WIN32, because life is too short to try and come up
	  with assembler of i386/powerpc/sparc/mips etc for all the ABIs
	  of the various platforms when that is what C compilers are
	  for in the first place

  5. Not intending doing an OS2 specific port of SOM.DLL as have no intention
      of inadvertently breaking WPS.

  6. There is an exception to 4, that is if you want to run SOM on an embedded
	  device or a generally a machine with small memory footprint then
	  you may want to save on the stubs and also save a few bytes per som[Data]Resolve()

  the four areas this explicitly covers are

  1. by-passing somResolve by the caller		SOM_METHOD_THUNKS
  2. by-passing somDataResolve by the caller	SOM_DATA_THUNKS
  3. blank apply stub							SOM_METHOD_STUBS
  4. blank redispatch stub						SOM_METHOD_STUBS

*/

#ifdef SOMTHUNK_FASTCALL
static somMethodPtr SOMLINK somthunk_somResolve(SOMObject SOMSTAR somSelf,somMToken mToken)
{
	/* no checks done for speed (unless debugging!) */

	RHBOPT_ASSERT(SOMMTOKEN_DATA(mToken)==mToken);

#ifdef _DEBUG
	if (!somTestCls(somSelf,somGetClassFromMToken(mToken),__FILE__,__LINE__))
	{
		const char *p=*(mToken->defined.somId_methodDescriptor);
		somPrintf("somResolve(%p,%s,%s)\n",somSelf,somSelf->mtab->className,p);
		return NULL;
	}
#endif

	RHBOPT_ASSERT(somClassInfoFromMtab(somSelf->mtab)==somSelf->mtab->classInfo)

	return SOMKERN_resolve(somMethodTabFromObject(somSelf),mToken);
}
#endif

#ifdef _M_IX86
static __declspec(naked) void resolve_exec_standard(void)
{
	/* the use of "6" refers to the size of the call instruction
		used to get us here, by subtracting 6 we get the start
		of the somMToken structure, with the call at the start */

	/* 0 = saved ebp */
	/* 4 = method+6 */
	/* 8 = return IP */
	/* 12 = somSelf */

	__asm 
	{
#ifdef SOMTHUNK_FASTCALL
		pop		eax									; ret address points to mToken
		push	[eax]								; get token
		mov		eax,[esp+8]							; get somSelf from original call to thunk
		push	eax									; push as arg for somResolve
		call	somthunk_somResolve
		ASM_REMOVE_ARGS(8)
		jmp		eax		

#else
		; old version follows

		push	ebp									; save ebp
		mov		ebp,esp								; point ebp at stack
		sub		dword ptr [ebp+4],somthunk_jump_ro_offset	; evaluate token from ret addr
		mov		eax,[ebp+12]						; get somSelf
		xchg	eax,[ebp]							; swap saved ebp with somSelf
		mov		ebp,eax								; put into ebp
		call	somResolve							; resolve the function
		ASM_REMOVE_ARGS(8)
		jmp		eax									; jump to the resolved address
#endif
	}
}
#endif

#ifdef _M_IX86
static __declspec(naked) void resolve_exec_aggregate(void)
{
	/* 0 = saved ebp */
	/* 4 = method+6 */
	/* 8 = return IP */
	/* 12 = ptr to result */
	/* 16 = somSelf */

	__asm 
	{
#ifdef SOMTHUNK_FASTCALL
		pop		eax									; ret address points to mToken
		push	[eax]								; get token
		mov		eax,[esp+12]						; get somSelf from original call to thunk
		push	eax									; push as arg for somResolve
		call	somthunk_somResolve
		ASM_REMOVE_ARGS(8)
		jmp		eax		
#else
		; old version follows

		push	ebp									; save ebp
		mov		ebp,esp								; point ebp at stack
		sub		dword ptr [ebp+4],somthunk_jump_ro_offset	; evaluate token from ret addr
		mov		eax,[ebp+16]						; get somSelf
		xchg	eax,[ebp]							; swap saved ebp with somSelf
		mov		ebp,eax								; put into ebp
		call	somResolve							; resolve the function
		ASM_REMOVE_ARGS(8)
		jmp		eax									; jump to the resolved address
#endif
	}
}
#endif

#ifdef _M_IX86
static __declspec(naked) void resolve_exec_root(void)
{
	/* this only works for SOMObject, 
		bypasses somResolve and does not have any aggregates */

#ifdef SOMTHUNK_FASTCALL
	/* onstack ,somSelf,retAddr,somMTokenPtr */
	__asm 
	{
		xchg	ebx,[esp]						; swap ebx with ptr to mToken, saving ebx
		mov		ebx,[ebx]						; get method token
		xor		eax,eax							; zero eax
		mov		ax,[ebx]somMTokenData.index		; get offset
		mov		ebx,[esp+8]						; get somSelf
		mov		ebx,[ebx]						; get mtab
		add		eax,eax							; shift left x 2
		add		eax,eax	
		add		ebx,eax							; get to method table
		mov		eax,[ebx]somMethodTab.entries	; get specific entry
		pop		ebx								; restore ebx
		jmp		eax								; jmp to method
	}

	/* 0 = saved ebp */
	/* 4 = return */
	/* 8 = somSelf */
#else
	__asm 
	{
		pop		eax								; eax has method+jump_ro_offset
		push	ebp								; save ebp
		mov		ebp,esp							; point to stack
		push	ebx								; save ebx

		mov		ebx,[eax]						; get method token
		xor		eax,eax							; zero eax
		mov		ax,[ebx]somMTokenData.index		; get offset
		mov		ebx,[ebp+8]						; get somSelf
		mov		ebx,[ebx]						; get mtab
		add		eax,eax							; shift left x 2
		add		eax,eax	
		add		ebx,eax							; get to method table
		mov		eax,[ebx]somMethodTab.entries	; get specific entry

		pop		ebx								; restore ebx
		pop		ebp								; restore ebp
		jmp		eax								; jmp to method
	}
#endif
}
#endif

#ifdef somthunk_jump_ap_offset
static int SOMLINK apply_alloca(somMToken token)
{
	if (-1L == (long)token->defined.redispatchStub)
	{
		somApRdInfo *info=(void *)token->defined.applyStub;

		if (info->stubInfo)
		{
			/* if it returns an aggregate, we need room for that as well */
			return info->stubInfo->va_listSize +
					(((info->stubInfo->callType & ~1)==SOMRdRetaggregate)
					? sizeof(void *) : 0);
		}
	}

#ifdef _M_IX86
	__asm int 3;
#endif

	return 0;
}
#endif

#ifdef somthunk_jump_ap_offset
static somMethodPtr SOMLINK apply_exec(void **ebp,void **dataArea)
{
	somMToken mToken=ebp[-1];
	SOMObject SOMSTAR somSelf=ebp[2];
	somToken retVal=ebp[3];
	somMethodPtr mPtr=(somMethodPtr)ebp[4];
	va_list ap=ebp[5];
	somApRdInfo *info=(somApRdInfo *)(mToken->defined.applyStub);
	octet *dataPush;
	unsigned long offset=0;
	unsigned long k=info->stubInfo->va_listSize;

	if ((info->stubInfo->callType & ~SOMRdFloatArgs)==SOMRdRetaggregate)
	{
		*dataArea++=retVal;
	}

	dataPush=(octet *)dataArea;

	if (info->stubInfo->callType & SOMRdFloatArgs)
	{
		unsigned long *floatMap=*(info->stubInfo->float_map);

		while (offset < k)
		{
/*			if (floatMap) somPrintf("offset=%lx,floatMap=%lx\n",offset,floatMap[0]);*/
					
			if (floatMap && (offset == (floatMap[0] & ~(SOMFMSingle|SOMFMLast))))
			{
				double d=va_arg(ap,double);

				if (floatMap[0] & SOMFMSingle)
				{
					((float *)(dataPush+offset))[0]=(float)d;
					offset+=sizeof(float);
				}
				else
				{
					((double *)(dataPush+offset))[0]=d;
					offset+=sizeof(double);
				}

				if (floatMap[0] & SOMFMLast)
				{
					floatMap=NULL;
				}
				else
				{
					floatMap++;
				}
			}
			else
			{
				((int *)(dataPush+offset))[0]=va_arg(ap,int);
				offset+=sizeof(int);
			}
		}
	}
	else
	{
		memcpy(dataPush,ap,k);
	}

	return mPtr;
}
#endif

#ifdef somthunk_jump_ap_offset

static void SOMLINK apply_retval(void **ebp,__int64 value)
{
	somMToken mToken=ebp[-1];
	somApRdInfo *info=(somApRdInfo *)(mToken->defined.applyStub);
	somToken retVal=ebp[3];

	if (retVal) switch (info->stubInfo->callType & ~1)
	{
	case SOMRdRetsimple:
		((int *)(retVal))[0]=(int)value;
		break;
	case SOMRdRetfloat:
		__asm
		{
			mov eax,retVal;
			fstp dword ptr [eax];
		}
		break;
	case SOMRdRetdouble:
		__asm
		{
			mov eax,retVal;
			fstp qword ptr [eax];
		}
		break;
	case SOMRdRetlongdouble:
		__asm
		{
			mov eax,retVal;
			fstp qword ptr [eax];
		}
		break;
	case SOMRdRetaggregate:
		break;
	case SOMRdRetbyte:
		((char *)(retVal))[0]=(char)value;
		break;
	case SOMRdRethalf:
		((short *)(retVal))[0]=(short)value;
		break;
	case SOMRdRetsmallaggregate:
		((__int64 *)(retVal))[0]=value;
		break;
	default:
		__asm int 3;
		break;
	}
}
#endif

#ifdef somthunk_jump_ap_offset
static __declspec(naked) void apply_exec_standard()
{
	/*	ebp-4	somMToken
		ebp+0	old ebp
		ebp+4	retaddress to call to mtoken
		ebp+8	somSelf
		ebp+16	retvalptr
		ebp+20	methodptr
		ebp+24	va_list
	*/
	__asm
	{
		pop		eax
		sub		eax,somthunk_jump_ap_offset
		push	ebp
		mov		ebp,esp
		push	eax				; mtoken

		push	eax
		call	apply_alloca
		ASM_REMOVE_ARGS(4)

		sub		esp,eax
		mov		eax,esp
		push	eax				; data
		push	ebp				; stack
		call	apply_exec
		ASM_REMOVE_ARGS(8)
		call	eax				; call the function ptr returned

		push	edx
		push    eax
		push	ebp
		call	apply_retval	; sort out the return arg

		mov		esp,ebp			; in theory this sorts all the stack out
		pop		ebp
#ifdef SOMLINK_STDCALL
		ret		16				; always four args to apply stub
#else
		ret
#endif
	}
}
#endif

/* redispatch is harder because we have to

	pop off retval
	add to stack
	push retval
	return value

*/

#ifdef somthunk_jump_rd_offset
static int SOMLINK redispatch_alloca(somMToken mToken)
{
	if (-1L == (long)mToken->defined.redispatchStub)
	{
		somApRdInfo *info=(void *)mToken->defined.applyStub;

		if (info->stubInfo && (info->stubInfo->callType & SOMRdFloatArgs))
		{
			/* if it returns an aggregate, we need room for that as well */
			unsigned long len=info->stubInfo->va_listSize;
			unsigned long *pl=info->stubInfo->float_map[0];

			/* single floats need more room as promoted to doubles */

			while (pl)
			{
				if (*pl & SOMFMSingle) len+=4;
				if (*pl & SOMFMLast) break;
				pl++;
			}

			return len;
		}
	}

	return 0;
}
#endif

#define ebp_token		-4
#define ebp_saved		0
#define ebp_ecx			4
#define ebp_retaddr		12
#define ebp_args		16

#define ebp_index(n)	((n) >> 2)

#ifdef somthunk_jump_rd_offset
static __int64 SOMLINK redispatch_exec(void **ebp,void *dataArea)
{
	__int64 retVal=0;
	somMToken mToken=ebp[-1];
	void *ecx=ebp[ebp_index(ebp_ecx)];
	somApRdInfo *info=(void *)mToken->defined.applyStub;
	boolean retByPtr=(boolean)((info->stubInfo->callType & ~1)==SOMRdRetaggregate);
	boolean floatArgs=(boolean)(info->stubInfo->callType & SOMRdFloatArgs);
	void *retPtr=(retByPtr ? ebp[ebp_index(ebp_args)] : &retVal);
	SOMObject SOMSTAR somSelf=(retByPtr ? ebp[ebp_index(ebp_args)+1] : ebp[ebp_index(ebp_args)]);
	va_list ap=(void *)(floatArgs ? dataArea : (retByPtr ? ebp+ebp_index(ebp_args)+1 : ebp+ebp_index(ebp_args)));
#ifdef SOMLINK_STDCALL
	long argLength=info->stubInfo->va_listSize+(retByPtr ? sizeof(void *) : 0);
#else
	long argLength=(retByPtr ? sizeof(void *) : 0);
#endif
	void *retAddress=ebp[ebp_index(ebp_retaddr)];
	if (retByPtr) retVal=(long)retPtr;

#ifdef _DEBUG
	SOM_Assert(somIsObj(somSelf),SOM_Fatal);
#endif
/*	SOMObject_somPrintSelf(somSelf);*/

	if (floatArgs)
	{
		va_list dest=ap;
		unsigned long args=info->stubInfo->va_listSize;
		unsigned long offset=0;
		unsigned long *lp=info->stubInfo->float_map[0];
		char *argPtr=(void *)(retByPtr ? ebp+ebp_index(ebp_args)+1 : ebp+ebp_index(ebp_args));

		while (offset < args)
		{
			if (lp && (offset==(*lp & ~(SOMFMLast|SOMFMSingle))))
			{
				if (*lp & SOMFMSingle)
				{
					va_arg(dest,double)=*((float *)(argPtr+offset));
					offset+=4;
				}
				else
				{
					va_arg(dest,double)=*((double *)(argPtr+offset));
					offset+=8;
				}

				if (*lp & SOMFMLast) 
				{
					lp=NULL;
				}
				else
				{
					lp++;
				}
			}
			else
			{
				va_arg(dest,int)=*((int *)(argPtr+offset));
				offset+=4;
			}
		}
	}

	SOMObject_somDispatch(somSelf,
			retPtr,
			(somId)mToken->defined.somId_methodId,
			ap);

/*	somPrintf("need to ret by %ld, retAddress=%p,ecx=%p\n",
			argLength,retAddress,ecx);
*/
	/* this will be popped of into ecx to add to esp */

	ebp[ebp_index(ebp_ecx)]=(void *)(argLength);

	{
		char *p=(char *)(&ebp[ebp_index(ebp_ecx)+1]);
		p+=argLength;
		((void **)p)[0]=ecx;
		((void **)p)[1]=retAddress;
	}

	switch (info->stubInfo->callType & ~1)
	{
	case SOMRdRetfloat:
		__asm
		{
			fld dword ptr [retVal]
		}
		break;
	case SOMRdRetlongdouble:
	case SOMRdRetdouble:
		__asm
		{
			fld qword ptr [retVal]
		}
		break;
	}

/*	{
		int i=0;
		while (i < 5)
		{
			somPrintf("ebp[%d]=%lx\n",i,ebp[i]);
			i++;
		}
	}
*/
	return retVal;
}
#endif

#ifdef somthunk_jump_rd_offset
static __declspec(naked) void redispatch_exec_standard()
{
/*	this is executed with the complete argument list
	with somToken+jump_rd_offset pushed on as the first argument

	we need to allocate an array for the va_list (on the stack)
	then build up the va_list from the arguments

	we also have to pop off all the arguments plus any pushed SOMRdRetaggregate pointer

	best way is for C routine to manipulate the stack then let the assembler
	just clean ebp etc, add the required offset to the stack and return
	note eax:edx must not be affected by cleanup so use ecx for this manipulation

		-4	mToken
		0	ebp saved
		4	saved ecx
		8	saved ecx
		12	ret address
		
		if retValPtr
		16	return valPtr
		20	somSelf,
		else
		16	somSelf
		args...


	*/
	__asm
	{
		pop		eax						; get the mToken
		sub		eax,somthunk_jump_rd_offset
		push	ecx						; saved ecx
		push	ecx						; room for pop-magic
		push	ebp
		mov		ebp,esp
		push	eax
		
		push	eax
		call	redispatch_alloca		; calculate room needed 
		ASM_REMOVE_ARGS(4)

		sub		esp,eax					; make the room on the stack
		mov		eax,esp					; push pointer to this space on stack
		push	eax
		mov		eax,ebp					; push pointer to frame and args on stack
		push	eax
		call	redispatch_exec

		mov		esp,ebp					; throw away allocated stacked
		pop		ebp						; restore ebp
		pop		ecx						; get length of arguments to zap
		add		esp,ecx					; pop all the arguments
		pop		ecx						; restore ecx
		ret
	}
}
#endif

#ifdef somthunk_jump_data_offset
static __declspec(naked) void resolve_data()
{
	/* somDToken called with single argument, somSelf
		so stack has
			0 <retaddr>
			4 <somSelf>

		extra call from somDToken gives us
			0 <somDToken+jump_data_offset>
			4 <retaddr>
			8 <somSelf>

		goal is to get stack looking like

			0 <retaddr>
			4 <somSelf>
			8 <somDToken>

		then jump directly to somDataResolve
	*/

	__asm 
	{
		mov		eax,[esp]
		sub		eax,somthunk_jump_data_offset
		xchg	eax,[esp+8]
		xchg	eax,[esp+4]
		mov		[esp],eax
		jmp		somDataResolve
	}
}
#endif

#ifdef somthunk_jump_rd_offset
somMethodPtr somkern_jump_redispatch=(somMethodPtr)redispatch_exec_standard;
#endif
#ifdef somthunk_jump_ap_offset
somMethodPtr somkern_jump_apply=(somMethodPtr)apply_exec_standard;
#endif
#ifdef somthunk_jump_data_offset
somMethodPtr somkern_jump_data=(somMethodPtr)resolve_data;
#endif
#ifdef SOM_METHOD_THUNKS
somMethodPtr somkern_jump_root=(somMethodPtr)resolve_exec_root;
somMethodPtr somkern_jump_standard=(somMethodPtr)resolve_exec_standard;
somMethodPtr somkern_jump_aggregate=(somMethodPtr)resolve_exec_aggregate;
#endif
