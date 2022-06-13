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
#include <stdio.h>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <unistd.h>
#endif

#include <somkern.h>

#ifdef SOM_DEBUG_MEMORY
	#define SOM_DEBUG_EXTRA		32
	#define SOM_DEBUG_BEFORE	0xab
	#define SOM_DEBUG_AFTER		0xaf
#endif

#ifdef SOM_DEBUG_MEMORY

#if defined(_WIN32) && defined(_DEBUG) && defined(USE_THREADS) && defined(USE_PTHREADS)
	#define USE_DEBUG_SNAPSHOT
#endif

static unsigned long _mem_ptrs;
typedef struct tag_memblock
{
		char *ptr;
		long len;
#ifdef USE_DEBUG_SNAPSHOT
		struct pthread_debug_snapshot *snap;
#endif /* USE_DEBUG_SNAPSHOT */
} memblock;
static memblock _mem_buffer[32768<<2];
static unsigned long _mem_max;
#define MAGIC_END_CHAR  0xAA
#endif

#ifdef SOM_DEBUG_MEMORY
static void dump_mem(void)
{
	unsigned int i=0;

	while (i < _mem_ptrs)
	{
		octet * pv=(octet *)_mem_buffer[i].ptr;
#ifdef _WIN32
		long pid=GetCurrentProcessId();
#else
		long pid=getpid();
#endif

		somPrintf("[%ld],len=%ld,%i=%p=",
			pid,
			_mem_buffer[i].len,
			i,
			pv);

		while (*pv)
		{
			if (*pv != 0x7)
			{
				printf("%c",*pv);
			}
			pv++;
		}

		somPrintf("\n");

#ifdef USE_DEBUG_SNAPSHOT
		pthread_debug_snapshot_print(_mem_buffer[i].snap);
#endif
		i++;
	}

#ifdef _WIN32
#	ifdef _M_IX86
	__asm int 3
#	endif
#endif
}
#endif

#ifdef SOM_DEBUG_MEMORY
static void SOMKERN_add_mem(void *pv,size_t len)
{
	unsigned int i=0;

	while (i < _mem_ptrs)
	{
		RHBOPT_ASSERT(_mem_buffer[i].ptr!=pv)

		i++;
	}

	if (i > (sizeof(_mem_buffer)/sizeof(_mem_buffer[0])))
	{
		somPrintf("allocation overrun %d malloc'd elements\n",i);
		exit(1);
	}

	_mem_buffer[i].ptr=pv;
	_mem_buffer[i].len=(long)len;
#ifdef USE_DEBUG_SNAPSHOT
	_mem_buffer[i].snap=pthread_debug_snapshot_create();
#endif

	if (pv)
	{
		char *cp=pv;
		memset(cp+len,SOM_DEBUG_AFTER,SOM_DEBUG_EXTRA);
	}

	if (i > _mem_max)
	{
		_mem_max=i;
	}

	_mem_ptrs++;
	
#ifdef _WIN32
/*	if (pv==(char *)(0xbd4d68))
	{
		__asm int 3
	}*/
#endif

}
#endif

#ifdef SOM_DEBUG_MEMORY
/*void SOMKERN_check_mem(void)
{
	unsigned int i=0;

	SOMKERN_guard_memory

	while (i < _mem_ptrs)
	{
		octet *op=_mem_buffer[i].ptr;
		size_t len=_mem_buffer[i].len;

		op+=len;
		len=SOM_DEBUG_EXTRA;

		while (len--)
		{
			if (*op != SOM_DEBUG_AFTER)
			{
				somPrintf("mem_ptr=%p\n",_mem_buffer[i].ptr);

				__asm int 3;

				break;
			}

			op++;
		}

		i++;
	}

	SOMKERN_unguard_memory
}
*/
static size_t SOMKERN_remove_mem(void *pv)
{
	unsigned int i=0;

	while (i < _mem_ptrs)
	{
		if (_mem_buffer[i].ptr==pv)
		{
			unsigned char *up;
			size_t len;

#ifdef USE_DEBUG_SNAPSHOT
			pthread_debug_snapshot_destroy(_mem_buffer[i].snap);
#endif
			len=_mem_buffer[i].len;

			_mem_ptrs--;

			if (i < _mem_ptrs)
			{
				_mem_buffer[i]=_mem_buffer[_mem_ptrs];
			}

			up=pv;

			i=0;

			while (i < SOM_DEBUG_EXTRA)
			{
				if (up[len+i] != SOM_DEBUG_AFTER) 
				{
					somPrintf("%p[%d+%d]\n",up,len,i);

					RHBOPT_ASSERT(up[len+i] != SOM_DEBUG_AFTER);
				}

				i++;
			}

			return len;
		}

		i++;
	}

	RHBOPT_ASSERT(!pv);

	return 0;
}
#endif

SOM_Scope somToken SOMLINK SOMKERN_malloc(size_t s)
{
	octet *v;

/*	SOMKERN_check_mem();*/

	RHBOPT_ASSERT(s)

	SOMKERN_guard_memory

/* #define MAGIC_MEMPTR    (0xa2fe48) */

#ifdef SOM_DEBUG_MEMORY
	v=malloc(s+SOM_DEBUG_EXTRA+SOM_DEBUG_EXTRA);
	if (v)
	{
		memset(v,SOM_DEBUG_BEFORE,SOM_DEBUG_EXTRA);
		v+=SOM_DEBUG_EXTRA;
		SOMKERN_add_mem(v,s);
	}
#else
	v=malloc(s);
#endif

	SOMKERN_unguard_memory

#ifdef SOM_DEBUG_MEMORY_SPRINTF
	somPrintf("SOMMalloc(%p)\n",v);
#endif

	RHBOPT_ASSERT(v)

#ifdef MAGIC_MEMPTR
	if (v==(void *)MAGIC_MEMPTR)
	{
		__asm int 3;
	}
#endif

	return v;
}

SOM_Scope somToken SOMLINK SOMKERN_calloc(size_t a,size_t b)
{
	octet *v;

	RHBOPT_ASSERT(a)
	RHBOPT_ASSERT(b)

	SOMKERN_guard_memory

#ifdef SOM_DEBUG_MEMORY
	v=malloc((a*b) + SOM_DEBUG_EXTRA+SOM_DEBUG_EXTRA);
	if (v)
	{
		memset(v,SOM_DEBUG_BEFORE,SOM_DEBUG_EXTRA);
		v+=SOM_DEBUG_EXTRA;
		memset(v,0,(a*b));
		SOMKERN_add_mem(v,a*b);
	}
#else
	v=calloc(a,b);
#endif

	SOMKERN_unguard_memory

#ifdef SOM_DEBUG_MEMORY_SPRINTF
	somPrintf("SOMCalloc(%p)\n",v);
#endif

	RHBOPT_ASSERT(v)

#ifdef MAGIC_MEMPTR
	if (v==(void *)MAGIC_MEMPTR)
	{
		__asm int 3;
	}
#endif

	return v;
}

SOM_Scope somToken SOMLINK SOMKERN_realloc(somToken p,size_t v)
{
	SOMKERN_guard_memory

/*	bomb("Not using realloc I hope");*/

#ifdef SOM_DEBUG_MEMORY
	SOMKERN_remove_mem(p);
	p=((octet *)p)-SOM_DEBUG_EXTRA; 
#endif

#ifdef SOM_DEBUG_MEMORY
	p=realloc(p,v+SOM_DEBUG_EXTRA+SOM_DEBUG_EXTRA);
	if (p)
	{
		memset(p,SOM_DEBUG_BEFORE,SOM_DEBUG_EXTRA);
		p=((char *)p)+SOM_DEBUG_EXTRA;
		SOMKERN_add_mem(p,v);
	}
#else
	p=realloc(p,v);
#endif

	SOMKERN_unguard_memory

	return p;
}

SOM_Scope void SOMLINK SOMKERN_free(somToken p)
{
#ifdef SOM_DEBUG_MEMORY_SPRINTF
	somPrintf("SOMFree(%p)\n",p);
#endif

/*	SOMKERN_check_mem();*/

#ifdef _DEBUG
	if (p)
	{
		size_t len;
#endif
		SOMKERN_guard_memory

#ifdef SOM_DEBUG_MEMORY
		len=SOMKERN_remove_mem(p);
		memset(p,0xDD,len);
		p=((octet *)p)-SOM_DEBUG_EXTRA; /* go back past 
										'extra' before freeing */

		len=SOM_DEBUG_EXTRA;
		while (len--)
		{
			if (((octet *)p)[len]!=SOM_DEBUG_BEFORE)
			{
				somPrintf("underwritten memory %p,%d\n",p,len);
				RHBOPT_ASSERT(!p);
			}
		}
#endif

		free(p);

		SOMKERN_unguard_memory

#ifdef _DEBUG
	}
	else
	{
		RHBOPT_ASSERT(p)
	}
#endif
}

SOM_IMPORTEXPORT_som somTD_SOMCalloc  * SOMDLINK SOMCalloc=SOMKERN_calloc;
SOM_IMPORTEXPORT_som somTD_SOMFree    * SOMDLINK SOMFree=SOMKERN_free;
SOM_IMPORTEXPORT_som somTD_SOMMalloc  * SOMDLINK SOMMalloc=SOMKERN_malloc;
SOM_IMPORTEXPORT_som somTD_SOMRealloc * SOMDLINK SOMRealloc=SOMKERN_realloc;

#ifdef SOM_DEBUG_MEMORY
void som_dump_mem(void)
{
	printf("maximum allocations was %ld/%ld\n",
			(long)_mem_max,
			(long)(sizeof(_mem_buffer)/sizeof(_mem_buffer[0])));
	if (_mem_ptrs) 
	{
		somPrintf("SOM.DLL, memory left is %d\n",_mem_ptrs);
		dump_mem();
	}
}
#endif
