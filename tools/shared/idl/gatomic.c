/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * g_atomic_*: atomic operations.
 * Copyright (C) 2003 Sebastian Wilhelmi
 * Copyright (C) 2007 Nokia Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


//#include "config.h"
//
#include "util.h"
//#include "gthreadprivate.h"
//#include "galias.h"



#if defined (__GNUC__)
# if defined (G_ATOMIC_I486)
/* Adapted from CVS version 1.10 of glibc's sysdeps/i386/i486/bits/atomic.h
 */
int
g_atomic_int_exchange_and_add (volatile int *atomic,
                               int           val)
{
  int result;

  __asm__ __volatile__ ("lock; xaddl %0,%1"
                        : "=r" (result), "=m" (*atomic)
                        : "0" (val), "m" (*atomic));
  return result;
}

void
g_atomic_int_add (volatile int *atomic,
                  int           val)
{
  __asm__ __volatile__ ("lock; addl %1,%0"
                        : "=m" (*atomic)
                        : "ir" (val), "m" (*atomic));
}

int
g_atomic_int_compare_and_exchange (volatile int *atomic,
                                   int           oldval,
                                   int           newval)
{
  int result;

  __asm__ __volatile__ ("lock; cmpxchgl %2, %1"
                        : "=a" (result), "=m" (*atomic)
                        : "r" (newval), "m" (*atomic), "0" (oldval));

  return result == oldval;
}

/* The same code as above, as on i386 void * is 32 bit as well.
 * Duplicating the code here seems more natural than casting the
 * arguments and calling the former function */

int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  void * result;

  __asm__ __volatile__ ("lock; cmpxchgl %2, %1"
                        : "=a" (result), "=m" (*atomic)
                        : "r" (newval), "m" (*atomic), "0" (oldval));

  return result == oldval;
}

# elif defined (G_ATOMIC_SPARCV9)
/* Adapted from CVS version 1.3 of glibc's sysdeps/sparc/sparc64/bits/atomic.h
 */
#  define ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)                   \
  ({                                                                    \
     int __result;                                                     \
     __asm__ __volatile__ ("cas [%4], %2, %0"                           \
                           : "=r" (__result), "=m" (*(atomic))          \
                           : "r" (oldval), "m" (*(atomic)), "r" (atomic),\
                           "0" (newval));                               \
     __result == oldval;                                                \
  })

#  if GLIB_SIZEOF_VOID_P == 4 /* 32-bit system */
int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  void * result;
  __asm__ __volatile__ ("cas [%4], %2, %0"
                        : "=r" (result), "=m" (*atomic)
                        : "r" (oldval), "m" (*atomic), "r" (atomic),
                        "0" (newval));
  return result == oldval;
}
#  elif GLIB_SIZEOF_VOID_P == 8 /* 64-bit system */
int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  void * result;
  void * *a = atomic;
  __asm__ __volatile__ ("casx [%4], %2, %0"
                        : "=r" (result), "=m" (*a)
                        : "r" (oldval), "m" (*a), "r" (a),
                        "0" (newval));
  return result == oldval;
}
#  else /* What's that */
#    error "Your system has an unsupported pointer size"
#  endif /* GLIB_SIZEOF_VOID_P */
#  define G_ATOMIC_MEMORY_BARRIER                                       \
  __asm__ __volatile__ ("membar #LoadLoad | #LoadStore"                 \
                        " | #StoreLoad | #StoreStore" : : : "memory")

# elif defined (G_ATOMIC_ALPHA)
/* Adapted from CVS version 1.3 of glibc's sysdeps/alpha/bits/atomic.h
 */
#  define ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)                   \
  ({                                                                    \
     int __result;                                                     \
     int __prev;                                                       \
     __asm__ __volatile__ (                                             \
        "       mb\n"                                                   \
        "1:     ldl_l   %0,%2\n"                                        \
        "       cmpeq   %0,%3,%1\n"                                     \
        "       beq     %1,2f\n"                                        \
        "       mov     %4,%1\n"                                        \
        "       stl_c   %1,%2\n"                                        \
        "       beq     %1,1b\n"                                        \
        "       mb\n"                                                   \
        "2:"                                                            \
        : "=&r" (__prev),                                               \
          "=&r" (__result)                                              \
        : "m" (*(atomic)),                                              \
          "Ir" (oldval),                                                \
          "Ir" (newval)                                                 \
        : "memory");                                                    \
     __result != 0;                                                     \
  })
#  if GLIB_SIZEOF_VOID_P == 4 /* 32-bit system */
int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  int result;
  void * prev;
  __asm__ __volatile__ (
        "       mb\n"
        "1:     ldl_l   %0,%2\n"
        "       cmpeq   %0,%3,%1\n"
        "       beq     %1,2f\n"
        "       mov     %4,%1\n"
        "       stl_c   %1,%2\n"
        "       beq     %1,1b\n"
        "       mb\n"
        "2:"
        : "=&r" (prev),
          "=&r" (result)
        : "m" (*atomic),
          "Ir" (oldval),
          "Ir" (newval)
        : "memory");
  return result != 0;
}
#  elif GLIB_SIZEOF_VOID_P == 8 /* 64-bit system */
int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  int result;
  void * prev;
  __asm__ __volatile__ (
        "       mb\n"
        "1:     ldq_l   %0,%2\n"
        "       cmpeq   %0,%3,%1\n"
        "       beq     %1,2f\n"
        "       mov     %4,%1\n"
        "       stq_c   %1,%2\n"
        "       beq     %1,1b\n"
        "       mb\n"
        "2:"
        : "=&r" (prev),
          "=&r" (result)
        : "m" (*atomic),
          "Ir" (oldval),
          "Ir" (newval)
        : "memory");
  return result != 0;
}
#  else /* What's that */
#   error "Your system has an unsupported pointer size"
#  endif /* GLIB_SIZEOF_VOID_P */
#  define G_ATOMIC_MEMORY_BARRIER  __asm__ ("mb" : : : "memory")
# elif defined (G_ATOMIC_X86_64)
/* Adapted from CVS version 1.9 of glibc's sysdeps/x86_64/bits/atomic.h
 */
int
g_atomic_int_exchange_and_add (volatile int *atomic,
                               int           val)
{
  int result;

  __asm__ __volatile__ ("lock; xaddl %0,%1"
                        : "=r" (result), "=m" (*atomic)
                        : "0" (val), "m" (*atomic));
  return result;
}

void
g_atomic_int_add (volatile int *atomic,
                  int           val)
{
  __asm__ __volatile__ ("lock; addl %1,%0"
                        : "=m" (*atomic)
                        : "ir" (val), "m" (*atomic));
}

int
g_atomic_int_compare_and_exchange (volatile int *atomic,
                                   int           oldval,
                                   int           newval)
{
  int result;

  __asm__ __volatile__ ("lock; cmpxchgl %2, %1"
                        : "=a" (result), "=m" (*atomic)
                        : "r" (newval), "m" (*atomic), "0" (oldval));

  return result == oldval;
}

int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  void * result;

  __asm__ __volatile__ ("lock; cmpxchgq %q2, %1"
                        : "=a" (result), "=m" (*atomic)
                        : "r" (newval), "m" (*atomic), "0" (oldval));

  return result == oldval;
}

# elif defined (G_ATOMIC_POWERPC)
/* Adapted from CVS version 1.16 of glibc's sysdeps/powerpc/bits/atomic.h
 * and CVS version 1.4 of glibc's sysdeps/powerpc/powerpc32/bits/atomic.h
 * and CVS version 1.7 of glibc's sysdeps/powerpc/powerpc64/bits/atomic.h
 */
#   ifdef __OPTIMIZE__
/* Non-optimizing compile bails on the following two asm statements
 * for reasons unknown to the author */
int
g_atomic_int_exchange_and_add (volatile int *atomic,
                               int           val)
{
  int result, temp;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("1:       lwarx   %0,0,%3\n"
                        "         add     %1,%0,%4\n"
                        "         stwcx.  %1,0,%3\n"
                        "         bne-    1b"
                        : "=&b" (result), "=&r" (temp), "=m" (*atomic)
                        : "b" (atomic), "r" (val), "m" (*atomic)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ (".Lieaa%=:       lwarx   %0,0,%3\n"
                        "         add     %1,%0,%4\n"
                        "         stwcx.  %1,0,%3\n"
                        "         bne-    .Lieaa%="
                        : "=&b" (result), "=&r" (temp), "=m" (*atomic)
                        : "b" (atomic), "r" (val), "m" (*atomic)
                        : "cr0", "memory");
#endif
  return result;
}

/* The same as above, to save a function call repeated here */
void
g_atomic_int_add (volatile int *atomic,
                  int           val)
{
  int result, temp;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("1:       lwarx   %0,0,%3\n"
                        "         add     %1,%0,%4\n"
                        "         stwcx.  %1,0,%3\n"
                        "         bne-    1b"
                        : "=&b" (result), "=&r" (temp), "=m" (*atomic)
                        : "b" (atomic), "r" (val), "m" (*atomic)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ (".Lia%=:       lwarx   %0,0,%3\n"
                        "         add     %1,%0,%4\n"
                        "         stwcx.  %1,0,%3\n"
                        "         bne-    .Lia%="
                        : "=&b" (result), "=&r" (temp), "=m" (*atomic)
                        : "b" (atomic), "r" (val), "m" (*atomic)
                        : "cr0", "memory");
#endif
}
#   else /* !__OPTIMIZE__ */
int
g_atomic_int_exchange_and_add (volatile int *atomic,
                               int           val)
{
  int result;
  do
    result = *atomic;
  while (!g_atomic_int_compare_and_exchange (atomic, result, result + val));

  return result;
}

void
g_atomic_int_add (volatile int *atomic,
                  int           val)
{
  int result;
  do
    result = *atomic;
  while (!g_atomic_int_compare_and_exchange (atomic, result, result + val));
}
#   endif /* !__OPTIMIZE__ */

#   if GLIB_SIZEOF_VOID_P == 4 /* 32-bit system */
int
g_atomic_int_compare_and_exchange (volatile int *atomic,
                                   int           oldval,
                                   int           newval)
{
  int result;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("sync\n"
                        "1: lwarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     2f\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    1b\n"
                        "2: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ ("sync\n"
                        ".L1icae%=: lwarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     .L2icae%=\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    .L1icae%=\n"
                        ".L2icae%=: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#endif
  return result == 0;
}

int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  void * result;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("sync\n"
                        "1: lwarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     2f\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    1b\n"
                        "2: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ ("sync\n"
                        ".L1pcae%=: lwarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     .L2pcae%=\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    .L1pcae%=\n"
                        ".L2pcae%=: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#endif
  return result == 0;
}
#   elif GLIB_SIZEOF_VOID_P == 8 /* 64-bit system */
int
g_atomic_int_compare_and_exchange (volatile int *atomic,
                                   int           oldval,
                                   int           newval)
{
  void * result;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("sync\n"
                        "1: lwarx   %0,0,%1\n"
                        "   extsw   %0,%0\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     2f\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    1b\n"
                        "2: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ ("sync\n"
                        ".L1icae%=: lwarx   %0,0,%1\n"
                        "   extsw   %0,%0\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     .L2icae%=\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    .L1icae%=\n"
                        ".L2icae%=: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#endif
  return result == 0;
}

int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  void * result;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("sync\n"
                        "1: ldarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     2f\n"
                        "   stdcx.  %3,0,%1\n"
                        "   bne-    1b\n"
                        "2: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ ("sync\n"
                        ".L1pcae%=: ldarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     .L2pcae%=\n"
                        "   stdcx.  %3,0,%1\n"
                        "   bne-    .L1pcae%=\n"
                        ".L2pcae%=: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#endif
  return result == 0;
}
#  else /* What's that */
#   error "Your system has an unsupported pointer size"
#  endif /* GLIB_SIZEOF_VOID_P */

#  define G_ATOMIC_MEMORY_BARRIER __asm__ ("sync" : : : "memory")

# elif defined (G_ATOMIC_IA64)
/* Adapted from CVS version 1.8 of glibc's sysdeps/ia64/bits/atomic.h
 */
int
g_atomic_int_exchange_and_add (volatile int *atomic,
                               int           val)
{
  return __sync_fetch_and_add (atomic, val);
}

void
g_atomic_int_add (volatile int *atomic,
                  int val)
{
  __sync_fetch_and_add (atomic, val);
}

int
g_atomic_int_compare_and_exchange (volatile int *atomic,
                                   int           oldval,
                                   int           newval)
{
  return __sync_bool_compare_and_swap (atomic, oldval, newval);
}

int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  return __sync_bool_compare_and_swap ((long *)atomic,
                                       (long)oldval, (long)newval);
}

#  define G_ATOMIC_MEMORY_BARRIER __sync_synchronize ()
# elif defined (G_ATOMIC_S390)
/* Adapted from glibc's sysdeps/s390/bits/atomic.h
 */
#  define ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)                   \
  ({                                                                    \
     int __result = oldval;                                    \
     __asm__ __volatile__ ("cs %0, %2, %1"                              \
                           : "+d" (__result), "=Q" (*(atomic))          \
                           : "d" (newval), "m" (*(atomic)) : "cc" );    \
     __result == oldval;                                                \
  })

#  if GLIB_SIZEOF_VOID_P == 4 /* 32-bit system */
int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  void * result = oldval;
  __asm__ __volatile__ ("cs %0, %2, %1"
                        : "+d" (result), "=Q" (*(atomic))
                        : "d" (newval), "m" (*(atomic)) : "cc" );
  return result == oldval;
}
#  elif GLIB_SIZEOF_VOID_P == 8 /* 64-bit system */
int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  void * result = oldval;
  void * *a = atomic;
  __asm__ __volatile__ ("csg %0, %2, %1"
                        : "+d" (result), "=Q" (*a)
                        : "d" ((long)(newval)), "m" (*a) : "cc" );
  return result == oldval;
}
#  else /* What's that */
#    error "Your system has an unsupported pointer size"
#  endif /* GLIB_SIZEOF_VOID_P */
# elif defined (G_ATOMIC_ARM)
static volatile int atomic_spin = 0;

static int atomic_spin_trylock (void)
{
  int result;

  asm volatile (
    "swp %0, %1, [%2]\n"
    : "=&r,&r" (result)
    : "r,0" (1), "r,r" (&atomic_spin)
    : "memory");
  if (result == 0)
    return 0;
  else
    return -1;
}

static void atomic_spin_lock (void)
{
  while (atomic_spin_trylock())
    sched_yield();
}

static void atomic_spin_unlock (void)
{
  atomic_spin = 0;
}

int
g_atomic_int_exchange_and_add (volatile int *atomic,
                               int           val)
{
  int result;

  atomic_spin_lock();
  result = *atomic;
  *atomic += val;
  atomic_spin_unlock();

  return result;
}

void
g_atomic_int_add (volatile int *atomic,
                  int           val)
{
  atomic_spin_lock();
  *atomic += val;
  atomic_spin_unlock();
}

int
g_atomic_int_compare_and_exchange (volatile int *atomic,
                                   int           oldval,
                                   int           newval)
{
  int result;

  atomic_spin_lock();
  if (*atomic == oldval)
    {
      result = TRUE;
      *atomic = newval;
    }
  else
    result = FALSE;
  atomic_spin_unlock();

  return result;
}

int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  int result;

  atomic_spin_lock();
  if (*atomic == oldval)
    {
      result = TRUE;
      *atomic = newval;
    }
  else
    result = FALSE;
  atomic_spin_unlock();

  return result;
}
# else /* !G_ATOMIC_ARM */
#  define DEFINE_WITH_MUTEXES
# endif /* G_ATOMIC_IA64 */
#else /* !__GNUC__ */
# ifdef G_PLATFORM_WIN32
#  define DEFINE_WITH_WIN32_INTERLOCKED
# else
#  define DEFINE_WITH_MUTEXES
# endif
#endif /* __GNUC__ */

#ifdef DEFINE_WITH_WIN32_INTERLOCKED
# include <windows.h>
/* Following indicates that InterlockedCompareExchangePointer is
 * declared in winbase.h (included by windows.h) and needs to be
 * commented out if not true. It is defined iff WINVER > 0x0400,
 * which is usually correct but can be wrong if WINVER is set before
 * windows.h is included.
 */
# if WINVER > 0x0400
#  define HAVE_INTERLOCKED_COMPARE_EXCHANGE_POINTER
# endif

gint32
g_atomic_int_exchange_and_add (volatile gint32 *atomic,
                               gint32           val)
{
  return InterlockedExchangeAdd (atomic, val);
}

void
g_atomic_int_add (volatile gint32 *atomic,
                  gint32           val)
{
  InterlockedExchangeAdd (atomic, val);
}

int
g_atomic_int_compare_and_exchange (volatile gint32 *atomic,
                                   gint32           oldval,
                                   gint32           newval)
{
#ifndef HAVE_INTERLOCKED_COMPARE_EXCHANGE_POINTER
  return (guint32) InterlockedCompareExchange ((PVOID*)atomic,
                                               (PVOID)newval,
                                               (PVOID)oldval) == oldval;
#else
  return InterlockedCompareExchange (atomic,
                                     newval,
                                     oldval) == oldval;
#endif
}

int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
# ifdef HAVE_INTERLOCKED_COMPARE_EXCHANGE_POINTER
  return InterlockedCompareExchangePointer (atomic, newval, oldval) == oldval;
# else
#  if GLIB_SIZEOF_VOID_P != 4 /* no 32-bit system */
#   error "InterlockedCompareExchangePointer needed"
#  else
   return InterlockedCompareExchange (atomic, newval, oldval) == oldval;
#  endif
# endif
}
#endif /* DEFINE_WITH_WIN32_INTERLOCKED */

#ifdef DEFINE_WITH_MUTEXES
/* We have to use the slow, but safe locking method */
//static GMutex *g_atomic_mutex;

int
g_atomic_int_exchange_and_add (volatile int *atomic,
                               int           val)
{
  int result;

//!!TODO!!  g_mutex_lock (g_atomic_mutex);
  result = *atomic;
  *atomic += val;
//!!TODO!!  g_mutex_unlock (g_atomic_mutex);

  return result;
}


void
g_atomic_int_add (volatile int *atomic,
                  int           val)
{
//!!TODO!!  g_mutex_lock (g_atomic_mutex);
  *atomic += val;
//!!TODO!!  g_mutex_unlock (g_atomic_mutex);
}

int
g_atomic_int_compare_and_exchange (volatile int *atomic,
                                   int           oldval,
                                   int           newval)
{
  int result;

//!!TODO!!  g_mutex_lock (g_atomic_mutex);
  if (*atomic == oldval)
    {
      result = TRUE;
      *atomic = newval;
    }
  else
    result = FALSE;
//!!TODO!!  g_mutex_unlock (g_atomic_mutex);

  return result;
}

int
g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                       void *           oldval,
                                       void *           newval)
{
  int result;

//!!TODO!!  g_mutex_lock (g_atomic_mutex);
  if (*atomic == oldval)
    {
      result = TRUE;
      *atomic = newval;
    }
  else
    result = FALSE;
//!!TODO!!  g_mutex_unlock (g_atomic_mutex);

  return result;
}

#ifdef G_ATOMIC_OP_MEMORY_BARRIER_NEEDED
int
g_atomic_int_get (volatile int *atomic)
{
  int result;

//!!TODO!!  g_mutex_lock (g_atomic_mutex);
  result = *atomic;
//!!TODO!!  g_mutex_unlock (g_atomic_mutex);

  return result;
}

void
g_atomic_int_set (volatile int *atomic,
                  int           newval)
{
//!!TODO!!  g_mutex_lock (g_atomic_mutex);
  *atomic = newval;
//!!TODO!!  g_mutex_unlock (g_atomic_mutex);
}

void *
g_atomic_pointer_get (volatile void * *atomic)
{
  void * result;

//!!TODO!!  g_mutex_lock (g_atomic_mutex);
  result = *atomic;
//!!TODO!!  g_mutex_unlock (g_atomic_mutex);

  return result;
}

void
g_atomic_pointer_set (volatile void * *atomic,
                      void *           newval)
{
//!!TODO!!  g_mutex_lock (g_atomic_mutex);
  *atomic = newval;
//!!TODO!!  g_mutex_unlock (g_atomic_mutex);
}
#endif /* G_ATOMIC_OP_MEMORY_BARRIER_NEEDED */
#elif defined (G_ATOMIC_OP_MEMORY_BARRIER_NEEDED)
int
g_atomic_int_get (volatile int *atomic)
{
  G_ATOMIC_MEMORY_BARRIER;
  return *atomic;
}

void
g_atomic_int_set (volatile int *atomic,
                  int           newval)
{
  *atomic = newval;
  G_ATOMIC_MEMORY_BARRIER;
}

void *
g_atomic_pointer_get (volatile void * *atomic)
{
  G_ATOMIC_MEMORY_BARRIER;
  return *atomic;
}

void
g_atomic_pointer_set (volatile void * *atomic,
                      void *           newval)
{
  *atomic = newval;
  G_ATOMIC_MEMORY_BARRIER;
}
#endif /* DEFINE_WITH_MUTEXES || G_ATOMIC_OP_MEMORY_BARRIER_NEEDED */

#ifdef ATOMIC_INT_CMP_XCHG
int
g_atomic_int_compare_and_exchange (volatile int *atomic,
                                   int           oldval,
                                   int           newval)
{
  return ATOMIC_INT_CMP_XCHG (atomic, oldval, newval);
}

int
g_atomic_int_exchange_and_add (volatile int *atomic,
                               int           val)
{
  int result;
  do
    result = *atomic;
  while (!ATOMIC_INT_CMP_XCHG (atomic, result, result + val));

  return result;
}

void
g_atomic_int_add (volatile int *atomic,
                  int           val)
{
  int result;
  do
    result = *atomic;
  while (!ATOMIC_INT_CMP_XCHG (atomic, result, result + val));
}
#endif /* ATOMIC_INT_CMP_XCHG */

void
_g_atomic_thread_init (void)
{
#ifdef DEFINE_WITH_MUTEXES
//  g_atomic_mutex = g_mutex_new ();
#endif /* DEFINE_WITH_MUTEXES */
}

#ifndef G_ATOMIC_OP_MEMORY_BARRIER_NEEDED
int
(g_atomic_int_get) (volatile int *atomic)
{
  return g_atomic_int_get (atomic);
}

void
(g_atomic_int_set) (volatile int *atomic,
                    int           newval)
{
  g_atomic_int_set (atomic, newval);
}

void *
(g_atomic_pointer_get) (volatile void * *atomic)
{
  return (void *)g_atomic_pointer_get (atomic);
}

void
(g_atomic_pointer_set) (volatile void * *atomic,
                        void *  newval)
{
  g_atomic_pointer_set (atomic, newval);
}
#endif /* G_ATOMIC_OP_MEMORY_BARRIER_NEEDED */

#define __G_ATOMIC_C__
//#include "galiasdef.c"
