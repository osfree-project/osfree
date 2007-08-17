/* Unix semaphore functions for regutil
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is regutil.
 *
 * The Initial Developer of the Original Code is Patrick TJ McPhee.
 * Portions created by Patrick McPhee are Copyright © 1998, 2001
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/regsemux.c,v 1.3 2004/08/21 14:48:40 prokushev Exp $
 */
#include "regutil.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#ifdef THREAD_SAFE
# include <pthread.h>
#else
# include <sys/time.h>
# include <signal.h>
#endif


/* ******************************************************************** */
/* **************************** Semaphores **************************** */
/* ******************************************************************** */

/* rexxutil maps semaphores onto the semaphores provided by the NT and OS/2.
 * These fall into the category of event semaphores, which allow one or
 * more processes to wait until another signals that it's time to procede,
 * and mutex semaphores, which are simple mutual-exclusion locks.
 *
 * There are three acquire/release operations: create, open, and close.
 * create either creates and opens a semaphore, or opens it if it exists
 * already. open opens a semaphore if exists already. close releases the
 * semaphore (this is done implicitly if the process exits). The semaphore
 * goes away when all processes which had previously opened it close it.
 *
 * There are three operations which affect the state of an event semaphore:
 * post, pulse, and reset. Post sets the semaphore so that wait operations
 * return. reset sets the semaphore so that wait operations block. pulse
 * releases one wait operation and then resets the semaphore.
 *
 * There is a wait operation, which blocks the semaphore until it is posted.
 *
 * If the semaphore was created with the `auto-reset' flag set to true,
 * post and pulse operate a bit differently -- pulse releases all waiting
 * processes while post releases only one waiting process. In both cases,
 * the semaphore is reset after the waiting processes are released.
 * 
 * Mutex semaphores have the same acquire/release operations, and two
 * state-changing operations: request and release. A request blocks
 * until the mutex is available, then locks it. A release releases
 * the mutex. */

/* Unix systems provide counting semaphores, with operations which
 * wait for the semaphore to reach some value, wait for the semaphore
 * to be non-zero, then decrement, and set the semaphore to some value.
 * Some systems provide mutex semaphores which can be shared between
 * processes, and some provide condition variables, which are a general
 * signalling mechanism. The challenge is to map the Windows semantics
 * onto the Unix semaphores, in a reasonably portable way.
 *
 * the most common semaphore implementation, sys/v semaphores, allow for
 * groups of semaphores to be allocated en masses and treated as a single
 * entity.
 */

/* In this library, windows semaphores are represented using SYS/V
 * semaphores. Each regutil semaphore takes 3 semaphores on 1 handle.
 * Semaphore 0 stores the state of the semaphore (posted or reset).
 * Semaphore 1 is used to keep a reference count.
 * Semaphore 2 stores the kind of Win semaphore we're emulating.
 *
 * SYS/V Semaphores are expensive resources, since the kernel limits the
 * number of semaphores, as well as the number of semaphore handles. So sue
 * me.
 * 
 * The semaphore is keyed off a file called /tmp/regutil.sem.<name>.  I was
 * thinking of putting the usage count in this file (mmapped), and then
 * controlling access to it using flock.  Instead, I don't bother creating
 * the files so I don't have to delete them...
 */

static const char notimp[] = "not implemented";
#define what() memcpy(result->strptr, notimp, sizeof(notimp)-1), result->strlength = sizeof(notimp)-1

static void (*oldalarmhandler)(int) = NULL;

static void alrmhdlr(int sig)
{
   if (oldalarmhandler)
      oldalarmhandler(sig);
}

#ifndef _SEMUN_DEFINED
union semun {
   int val;
   struct semid_ds *buf;
   unsigned short *array;
};
#endif

static const char sembasename[] = "/tmp/regutil.sem";

/* return a check-sum of a string. This returned unique numbers
 * for every filename on my system, so I use this in place of ftok
 * which has some messy cleanup issues, and isn't any better at
 * ensuring uniqueness, anyway. */
 static key_t checksum(const char * s, register int l)
 {
    register int i, cs;

    for (cs = i = 0; i < l; i++)
       cs += s[i]*(i+1);

    return cs;
 }

static void dropsems(void);

static int lockid = -2;
#ifdef THREAD_SAFE
static pthread_mutex_t lockmux = PTHREAD_MUTEX_INITIALIZER;
#endif

/* set up a semaphore to use as a general mutex around semaphore creation
 * and deletion. There's a potential race -- if I want to acquire the
 * lock at the instant another process is throwing it away, I could end
 * up losing the semaphore unexpectedly. We always have two semaphores.
 * Number 0 is the mutex and number 1 is the usage count.
 * Returns 0 if the semaphore was acquired, and so has to be locked,
 * 1 if it was created, and so doesn't have to be locked, and -1 if
 * there was an error. 
 */
static int makemetalock(void)
{
   struct sembuf sbf;
   int rc;

#  ifdef THREAD_SAFE
      pthread_mutex_lock(&mux);
#  endif


   /* create or use a general mutex semaphore */
   if (lockid < -1) {
      key_t key;

      atexit(dropsems);

#     ifndef THREAD_SAFE
      /* for timeouts, we use alarm(). This is not viable in a multi-threaded
       * program. For mt, we need to kick off a back-ground thread which implements
       * all the time-outs, but I don't feel like it right now. */
      if (oldalarmhandler == NULL) {
         oldalarmhandler = signal(SIGALRM, alrmhdlr);
      }
#     endif


      key = checksum(sembasename, sizeof(sembasename)-1);
      lockid = semget(key, 2, 0666);

      if (lockid == -1) {
         lockid = semget(key, 2, IPC_CREAT|0666);
         if (lockid > -1) {
            rc = 1;
         }
         else
            rc = -1;
      }
      else {
         rc = 0;
      }

      /* increment the usage count */
      sbf.sem_num = 1;
      sbf.sem_op = 1;
      sbf.sem_flg = 0;
      semop(lockid, &sbf, 1);
   }

#  ifdef THREAD_SAFE
      pthread_mutex_lock(&mux);
#  endif

   return rc;
}

static void metalock(void)
{
   struct sembuf sbf;

   sbf.sem_num = 0;
   sbf.sem_op = -1;
   sbf.sem_flg = 0;

   semop(lockid, &sbf, 1);
}


static void metaunlock(void)
{
   struct sembuf sbf;

   sbf.sem_num = 0;
   sbf.sem_op = 1;
   sbf.sem_flg = 0;

   semop(lockid, &sbf, 1);
}


static void dropmetalock(void)
{
   struct sembuf sbf;
   union semun arg;

#  ifdef THREAD_SAFE
      pthread_mutex_lock(&mux);
#  endif

   if (lockid > -1) {
      metalock();

      /* decrement the usage count */
      sbf.sem_num = 1;
      sbf.sem_op = -1;
      sbf.sem_flg = 0;
      semop(lockid, &sbf, 1);
      arg.val = 0;

      if (semctl(lockid, 1, GETVAL, arg) == 0) {
         semctl(lockid, 0, IPC_RMID, arg);
      }
      else {
         metaunlock();
      }
      lockid = -2;
   }

#  ifdef THREAD_SAFE
      pthread_mutex_lock(&mux);
#  endif
}


/* array of semaphores. Semaphore handles returned by this libary
 * are indices into this array */
static int nosems = 0, semsalloc = 0, * semarray;

typedef enum {
   sem_mutex = 1,
   sem_event,
   sem_ar_event
} semtype_t;



/* create a semaphore. Each win semaphore maps to three Unix semaphores:
 *  0 holds the actual sem value
 *  1 is a usage count
 *  2 holds the type
 */

int makesem(const char * name, const int namel, semtype_t type, rxbool create)
{
   struct sembuf sbf;
   union semun arg;
   key_t key = -1;
   int flags = 0666;
   int semid;
   semtype_t oldtype;

   if (name) {
      key = checksum(name, namel);
   }

   if (create)
      flags |= IPC_CREAT;

   metalock();

   /* don't bother trying to create semaphore without CREAT flag,
    * since we have other wise of figuring out whether we're acquiring or creating */
   semid = semget(key, 3, flags);
   
   if (semid > -1) {
      /* set the semaphore type -- everybody better be working together
       * here ! */
      oldtype = (semtype_t)semctl(semid, 2, GETVAL);


      /* semaphore must be freshly created */
      if ((int)oldtype == 0) {
         arg.val = (int)type;
         semctl(semid, 2, SETVAL, arg);

         /* create mutex in released state */
         if (type == sem_mutex) {
            arg.val = 1;
            semctl(semid, 0, SETVAL, arg);
         }
      }

      /* removed test that type be correct for existing semaphores, since
       * it's too much trouble to test for all the possiblities */


      /* if everything went well, increment the reference count, and keep
       * track of it so we can get rid of it at exit time. */
      sbf.sem_num = 1;
      sbf.sem_op = 1;
      sbf.sem_flg = 0;

      semop(semid, &sbf, 1);

      /* stick it in the semaphore array */
      if (nosems >= semsalloc) {
         semsalloc += 10;
         semarray = realloc(semarray, sizeof(*semarray)*semsalloc);
      }
      semarray[nosems++] = semid;
   }

   metaunlock();

   return semid;
}



void dropsem(const int semid)
{
   struct sembuf sbf;
   register int i;

   metalock();

   for (i = 0; i < nosems; i++) {
      if (semarray[i] == semid) {
         break;
      }
   }

   if (i >= 0 && i < nosems) {
      /* decrement the usage count */
      sbf.sem_num = 1;
      sbf.sem_op = -1;
      sbf.sem_flg = 0;
      semop(semid, &sbf, 1);

      if (semctl(semid, 1, GETVAL) == 0) {
         semctl(semid, 0, IPC_RMID);
      }

      if (i == (nosems - 1)) {
         nosems--;
         for (--i; i > -1 && semarray[i] == -1; nosems--, i--)
            ;
      }
      else
         semarray[i] = -1;


   }
   metaunlock();
}


int waitsem(const int semid, int mseconds)
{
   union semun arg;
   struct sembuf sbf = {0, -1, 0};
   u_short vals[3];
   semtype_t type;
   int status;
   int rc = 0;
   
   arg.array = vals;

   metalock();

   /* figure out what's going on */
   semctl(semid, 0, GETALL, arg);
   type = (semtype_t)vals[2];
   status = vals[0];

   metaunlock();

   /* go through normal wait mechanism for mutex or auto-reset
    * event semaphore, and for manual-reset semaphore
    * which is not posted. */
   if (type == sem_mutex || type == sem_ar_event || !status) {
#     ifndef THREAD_SAFE
      if (mseconds) {
         struct itimerval itv = { { 0, 0}, {0, 0}};

         itv.it_value.tv_sec = mseconds/1000;
         itv.it_value.tv_usec = (mseconds % 1000)*1000;

         setitimer(ITIMER_REAL, &itv, NULL);
      }
#     endif

      rc = semop(semid, &sbf, 1);

#     ifndef THREAD_SAFE
      if (mseconds) {
         struct itimerval itv = { { 0, 0}, { 0, 0}};
         setitimer(ITIMER_REAL, &itv, NULL);
      }
#     endif
   }

   /* if it's a posted manual reset event semaphore, do nothing */
   return rc;
}

typedef enum {
  ttd_reset,
  ttd_post,
  ttd_pulse
} thingtodo_t;


int setsem(const int semid, thingtodo_t thingtodo)
{
   union semun arg;
   u_short vals[3];
   semtype_t type;
   int count;

   arg.array = vals;

   metalock();

   if (thingtodo == ttd_reset) {
      arg.val = 0;
   }
   else {
      /* what kind of semaphore is this, and who's waiting for it? */
      type = (semtype_t)semctl(semid, 2, GETVAL);
      count = semctl(semid, 0, GETNCNT);

      /* if we are pulsing and have nobody to release, just return */
      if (thingtodo == ttd_pulse && count == 0) {
         metaunlock();
         return 0;
      }

      /* mutex && pulse auto-reset mutex, we want to release 1 waiter. */
      if ((type == sem_mutex && thingtodo == ttd_post) || (type == sem_ar_event && thingtodo == ttd_pulse)) {
         arg.val = 1;
      }
      /* post auto-reset mutex or pulse manual-reset mutex, we want to
       * release all waiters */
      else if ((type ==sem_ar_event && thingtodo == ttd_post) ||
               (type == sem_event && thingtodo == ttd_pulse) ) {
         arg.val = count;
      }
      /* post of manual-reset mutex, want to release all waiters & leave the semaphore
       * posted */
      else if (type == sem_event && thingtodo == ttd_post) {
         arg.val = count + 1;
      }
   }

   semctl(semid, 0, SETVAL, arg);

   metaunlock();

   return 0;
}

#define result_six() result->strlength = 1, *result->strptr = '6'
static const char timeoutval[] = "121";
#define result_timeout() result->strlength = 3, memcpy(result->strptr, timeoutval, 3)

/* syscloseeventsem(handle) */
rxfunc(syscloseeventsem)
{
   int semid;

   checkparam(1,1);

   if (argv[0].strlength != sizeof(semid) || !argv[0].strptr) {
      result_six();
      return 0;
   }

   memcpy(&semid, argv[0].strptr, sizeof(semid));

   dropsem(semid);

   result_one();

   return 0;
}

/* sysclosemutexsem(handle) */
rxfunc(sysclosemutexsem)
{
   return syscloseeventsem(fname, argc, argv, pSomething, result);
}

/* syscreateeventsem([name],[manual_reset]) */
rxfunc(syscreateeventsem)
{
   char * reset;
   semtype_t type = sem_ar_event;
   int rc;

   checkparam(0,2);

   if (argc > 1 && argv[1].strptr) {
      rxstrdup(reset, argv[1]);
      if (atoi(reset))
         type = sem_event;
   }

   if (makemetalock() == 1)
      metaunlock();

   if (argc > 0)
      rc = makesem(argv[0].strptr, argv[0].strlength, type, true);
   else
      rc = makesem(NULL, 0, type, true);

   if (rc == -1) {
      result->strlength = 0;
   }
   else {
      result->strlength = sizeof(rc);
      memcpy(result->strptr, &rc, sizeof(rc));
   }

   return 0;
}

/* syscreatemutexsem([name]) */
rxfunc(syscreatemutexsem)
{
   int rc;

   checkparam(0,1);

   if (makemetalock() == 1)
      metaunlock();

   if (argc > 0)
      rc = makesem(argv[0].strptr, argv[0].strlength, sem_mutex, true);
   else
      rc = makesem(NULL, 0, sem_mutex, true);

   if (rc == -1) {
      result->strlength = 0;
   }
   else {
      result->strlength = sizeof(rc);
      memcpy(result->strptr, &rc, sizeof(rc));
   }

   return 0;
}

/* sysopeneventsem(name) */
rxfunc(sysopeneventsem)
{
   int rc;

   checkparam(1,1);

   if (makemetalock() == 1)
      metaunlock();

   rc = makesem(argv[0].strptr, argv[0].strlength, 0, false);

   if (rc == -1) {
      result->strlength = 0;
   }
   else {
      result->strlength = sizeof(rc);
      memcpy(result->strptr, &rc, sizeof(rc));
   }

   return 0;
}

/* sysopenmutexsem(name) */
rxfunc(sysopenmutexsem)
{
   return sysopeneventsem(fname, argc, argv, pSomething, result);
}

/* sysposteventsem(handle) */
rxfunc(sysposteventsem)
{
   int semid, rc;

   checkparam(1,1);

   if (argv[0].strlength != sizeof(semid) || !argv[0].strptr) {
      result_six();
      return 0;
   }

   memcpy(&semid, argv[0].strptr, sizeof(semid));

   rc = setsem(semid, ttd_post);

   if (rc == -1) {
      result_one();
   }
   else {
      result_zero();
   }

   return 0;
}

/* syspulseeventsem(handle) */
rxfunc(syspulseeventsem)
{
   int semid, rc;

   checkparam(1,1);

   if (argv[0].strlength != sizeof(semid) || !argv[0].strptr) {
      result_six();
      return 0;
   }

   memcpy(&semid, argv[0].strptr, sizeof(semid));

   rc = setsem(semid, ttd_pulse);

   if (rc == -1) {
      result_one();
   }
   else {
      result_zero();
   }

   return 0;
}

/* sysreleasemutexsem(handle) */
rxfunc(sysreleasemutexsem)
{
   int semid, rc;

   checkparam(1,1);

   if (argv[0].strlength != sizeof(semid) || !argv[0].strptr) {
      result_six();
      return 0;
   }

   memcpy(&semid, argv[0].strptr, sizeof(semid));

   rc = setsem(semid, ttd_post);

   if (rc == -1) {
      result_one();
   }
   else {
      result_zero();
   }

   return 0;
}

/* sysrequestmutexsem(handle, [timeout]) */
rxfunc(sysrequestmutexsem)
{
   int semid, rc;
   int mseconds = 0;
   char * msecs;

   checkparam(1,2);

   if (argv[0].strlength != sizeof(semid) || !argv[0].strptr) {
      result_six();
      return 0;
   }

   memcpy(&semid, argv[0].strptr, sizeof(semid));

   if (argc > 1 && argv[1].strptr) {
      rxstrdup(msecs, argv[1]);
      mseconds = atoi(msecs);
   }

   rc = waitsem(semid, mseconds);

   if (rc == -1) {
      result_timeout();
   }
   else {
      result_zero();
   }

   return 0;
}

/* sysreseteventsem(handle) */
rxfunc(sysreseteventsem)
{
   int semid, rc;

   checkparam(1,1);

   if (argv[0].strlength != sizeof(semid) || !argv[0].strptr) {
      result_six();
      return 0;
   }

   memcpy(&semid, argv[0].strptr, sizeof(semid));

   rc = setsem(semid, ttd_reset);

   if (rc == -1) {
      result_one();
   }
   else {
      result_zero();
   }

   return 0;
}

/* syswaiteventsem(handle, timeout) */
rxfunc(syswaiteventsem)
{
   return sysrequestmutexsem(fname, argc, argv, pSomething, result);
}

/* drop all semaphores. This is done when the process exits */
static void dropsems()
{
   register int i;

   if (lockid > -1) {
      for (i = 0; i < nosems; i++) {
         dropsem(semarray[i]);
      }
      dropmetalock();
   }
}
