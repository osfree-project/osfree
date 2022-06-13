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

#ifdef BUILD_SOM
#else
	#ifndef BUILD_SOMCDR
		#error this should be part of SOMCDR
	#endif
	#include <rhbopt.h>
	#include <rhbsomex.h>
	#include <rhbsomkd.h>
#endif

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		#define RHBSOMKD_lock(x)	{ pthread_mutex_lock(&x->guardian.mutex);
		#define RHBSOMKD_unlock(x)		pthread_mutex_unlock(&x->guardian.mutex); }
		#define RHBSOMKD_lock2(x)	pthread_mutex_lock(&x->guardian.mutex); }
		#define RHBSOMKD_unlock2(x)	{ pthread_mutex_unlock(&x->guardian.mutex); 
	#else
		#define RHBSOMKD_lock(x)	{ EnterCriticalSection(&x->guardian.crit_sect);
		#define RHBSOMKD_unlock(x)		LeaveCriticalSection(&x->guardian.crit_sect); }
		#define RHBSOMKD_lock2(x)	EnterCriticalSection(&x->guardian.crit_sect); }
		#define RHBSOMKD_unlock2(x)	{ LeaveCriticalSection(&x->guardian.crit_sect); 
	#endif
#else
	#define RHBSOMKD_lock(x)
	#define RHBSOMKD_unlock(x)
	#define RHBSOMKD_lock2(x)
	#define RHBSOMKD_unlock2(x)
#endif

static unsigned long RHBCDR_kds_binary_key(unsigned long l)
{
	unsigned long m=1;

	while (m < l)
	{
		m<<=1;
	}

	return m;
}

static long RHBCDR_kds_compare(
		RHBSOMUT_Key *k1,RHBSOMUT_Key *k2)
{
	long l=(k1->_length - k2->_length);

	if (!l)
	{
		l=memcmp(k1->_buffer,k2->_buffer,k1->_length);
	}

	return l;
}

static void rhbgioporb_kds_uninit(
		RHBSOMUT_KeyDataSet *kds)
{
	void *pv=NULL;

	RHBSOMKD_lock(kds)

	pv=kds->dataset._buffer;
	kds->dataset._buffer=NULL;
	kds->dataset._maximum=0;
	kds->dataset._length=0;
	kds->lpVtbl=NULL;

	RHBSOMKD_unlock(kds)

	if (pv) SOMFree(pv);

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_mutex_destroy(&kds->guardian.mutex);
	#else
		DeleteCriticalSection(&kds->guardian.crit_sect);
	#endif
#endif
}

static boolean RHBCDR_kds_where(
		RHBSOMUT_KeyDataSet *kds,
		RHBSOMUT_Key *kdp,
		unsigned long *posn)
{	
	unsigned long n=0;

	if (kds->dataset._length)
	{
		long l=0;
		unsigned long m=RHBCDR_kds_binary_key(kds->dataset._length);

		while (m)
		{
			unsigned long o=n+m;

			if (o < kds->dataset._length)
			{		
				l=RHBCDR_kds_compare(
						&kds->dataset._buffer[o]->key,kdp);

				if (l)
				{
					if (l < 0)
					{
						n=o;
					}
				}
				else
				{
					*posn=o;

					return 1;
				}
			}

			m>>=1;
		}

		l=RHBCDR_kds_compare(
			&kds->dataset._buffer[n]->key,kdp);

		if (l)
		{
			if (l < 0)
			{
				n++;
			}
		}
		else
		{
			*posn=n;

			return 1;
		}
	}

	*posn=n;

	return 0;
}

static unsigned long rhbsomkd__set_maximum(
		RHBSOMUT_KeyDataSet *kds,
		unsigned long maximum)
{
	void *free_me=NULL;

	/* this little piece of magic is to unlock 
		the mutex while we change the size of the table */

	if (maximum)
	{
		RHBSOMUT_KeyDataPtr *new_buffer=NULL;
		unsigned long old_max=kds->dataset._maximum;

		RHBSOMKD_unlock2(kds)
		new_buffer=SOMMalloc(maximum * sizeof(*new_buffer));
		RHBSOMKD_lock2(kds)

		if (old_max != kds->dataset._maximum)
		{
			free_me=new_buffer;
		}
		else
		{
			if (new_buffer)
			{
				unsigned long old_length=kds->dataset._length;

				if (old_length)
				{
					memcpy(new_buffer,
					   kds->dataset._buffer,
					   sizeof(new_buffer[0])*old_length);
				}
			}

			free_me=kds->dataset._buffer;
			kds->dataset._buffer=new_buffer;
			kds->dataset._maximum=maximum;
		}
	}
	else
	{
		free_me=kds->dataset._buffer;

		kds->dataset._buffer=NULL;
		kds->dataset._maximum=0;
		kds->dataset._length=0;
	}

	if (free_me)
	{
		RHBSOMKD_unlock2(kds)
		SOMFree(free_me);
		RHBSOMKD_lock2(kds)
	}

	return kds->dataset._maximum;
}

static boolean rhbgioporb_kds_add(
		RHBSOMUT_KeyDataSet *kds,
		RHBSOMUT_KeyDataPtr kdp)
{
	unsigned long l,k=0;
	boolean added=0;

/*	RHBOPT_ASSERT(!kdp->count) */

	RHBSOMKD_lock(kds)

	while (!RHBCDR_kds_where(kds,&kdp->key,&k))
	{
		if (kds->dataset._length == kds->dataset._maximum)
		{
			rhbsomkd__set_maximum(kds,kds->dataset._maximum+32);
		}
		else
		{
			kdp->count++;

			l=k;

			while (l < kds->dataset._length)
			{
				RHBSOMUT_KeyDataPtr o=kds->dataset._buffer[l];

				kds->dataset._buffer[l]=kdp;

				kdp=o;

				l++;
			}

			kds->dataset._length++;

			kds->dataset._buffer[l]=kdp;

			added=1;

			break;
		}
	}

	RHBSOMKD_unlock(kds)

	return added;
}

static boolean rhbgioporb_kds_remove(
		RHBSOMUT_KeyDataSet *kds,
		RHBSOMUT_KeyDataPtr kdp)
{
	unsigned long l=0;
	boolean removed=0;
	void *free_me=NULL;

	RHBSOMKD_lock(kds)

	RHBOPT_ASSERT(kdp->count)

	if (RHBCDR_kds_where(kds,&kdp->key,&l))
	{
		if (kds->dataset._buffer[l]==kdp)
		{
			kds->dataset._length--;

			while (l < kds->dataset._length)
			{
				kds->dataset._buffer[l]=kds->dataset._buffer[l+1];
				l++;
			}

			if (kds->dataset._length)
			{
				if ((kds->dataset._length+32) < kds->dataset._maximum)
				{
					rhbsomkd__set_maximum(kds,kds->dataset._length);
				}
			}
			else
			{
				free_me=kds->dataset._buffer;
				kds->dataset._maximum=0;
				kds->dataset._length=0;
				kds->dataset._buffer=NULL;
			}

			kdp->count--;

			removed=1;
		}
		else
		{
			RHBOPT_ASSERT(!kdp);
		}
	}

	RHBSOMKD_unlock(kds)
	
	if (free_me) 
	{
		SOMFree(free_me);
	}

	return removed;
}

static RHBSOMUT_KeyDataPtr rhbgioporb_kds_find(
		RHBSOMUT_KeyDataSet *kds,
		RHBSOMUT_Key *key)
{
	unsigned long l=0;
	RHBSOMUT_KeyDataPtr p=NULL;
	
	RHBSOMKD_lock(kds)

	if (RHBCDR_kds_where(kds,key,&l))
	{
		p=kds->dataset._buffer[l];
	}

	RHBSOMKD_unlock(kds)

	return p;
}

static RHBSOMUT_KeyDataPtr rhbgioporb_kds_get(
		RHBSOMUT_KeyDataSet *kds,
		unsigned long index)
{
	RHBSOMUT_KeyDataPtr p=NULL;
	RHBSOMKD_lock(kds)

	if (index < kds->dataset._length)
	{
		p=kds->dataset._buffer[index];
	}

	RHBSOMKD_unlock(kds)

	return p;
}

static unsigned long rhbgioporb_kds_count(
		RHBSOMUT_KeyDataSet *kds)
{
	unsigned long ul=0;

	RHBSOMKD_lock(kds)

	ul=kds->dataset._length;

	RHBSOMKD_unlock(kds)

	return ul;
}

static boolean rhbgioporb_kds_contains(RHBSOMUT_KeyDataSet *kds,
									   RHBSOMUT_KeyData *key)
{
	unsigned long index=0;
	boolean result=0;

	RHBSOMKD_lock(kds)

	while (index < kds->dataset._length)
	{
		if (key==kds->dataset._buffer[index++])
		{
			result=1;
			break;
		}
	}

	RHBSOMKD_unlock(kds)

	return result;
}

static RHBSOMUT_KeyDataSetVtbl RHBSOMUT_KeyDataSet_vtbl={
	rhbgioporb_kds_uninit,
	rhbgioporb_kds_add,
	rhbgioporb_kds_remove,
	rhbgioporb_kds_count,
	rhbgioporb_kds_find,
	rhbgioporb_kds_get,
	rhbgioporb_kds_contains};

#define RHBSOMUT_KeyDataSetInit	{&RHBSOMUT_KeyDataSet_vtbl,{0,0,NULL},RHBSOMUT_CriticalSectionInit}

#ifdef RHBSOMKD_STATIC
static
#else
SOMEXTERN 
#endif
RHBSOMUT_KeyDataSet * SOMLINK RHBCDR_kds_init(
		RHBSOMUT_KeyDataSet *kds)
{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_mutex_init(&kds->guardian.mutex,RHBOPT_pthread_mutexattr_default);
	#else
		InitializeCriticalSection(&kds->guardian.crit_sect);
	#endif
#endif

	RHBSOMKD_lock(kds)

	kds->dataset._maximum=0;
	kds->dataset._length=0;
	kds->dataset._buffer=NULL;
	kds->lpVtbl=&RHBSOMUT_KeyDataSet_vtbl;

	RHBSOMKD_unlock(kds)

	return kds;
}

#if defined(BUILD_SOM) && defined(_WIN32)
static void rhbsomkd_init(void)
{
}
static void rhbsomkd_term(void)
{
}
#endif
