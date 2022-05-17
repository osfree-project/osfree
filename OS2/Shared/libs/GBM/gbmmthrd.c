/*

gbmmthrd.c - GBM Multithreading support library

Author: Heiko Nitzsche

History
-------
27-Aug-2010: Initial version
29-Apr-2012: Use DosWaitEventSem on OS/2 instead of WinWaitEventSem
             to get rid of dependency to PMWIN.DLL. For GBM internal
             threads there is no need to use Win function.
*/

#include "gbmmthrd.h"

#include <assert.h>
#include <stdlib.h>

#if defined(__OS2__) || defined(OS2)

  #include <process.h>
  #define INCL_DOS
  #define INCL_DOSERRORS
  #define INCL_DOSSEMAPHORES
  #include <os2.h>
  typedef int    THREAD;
  typedef HEV    EVENT_SEM;
  #define SEM_NULLHANDLE  NULLHANDLE
  #define THREAD_NULL ((int)-1)

#elif defined(__WIN32__) || defined(WIN32)

  #include <process.h>
  #include <windows.h>
  typedef HANDLE THREAD;
  #define THREAD_NULL ((HANDLE)0L)

#elif defined(LINUX)

  #include <unistd.h>
  #include <pthread.h>
  typedef pthread_t THREAD;

#else

  Unsupported architecture detected.

#endif

/* --------------------------------------------------- */

typedef struct
{
    GBM_THREAD_FUNC function;
    GBM_THREAD_ARG  function_arg;
   #if defined(__OS2__) || defined(OS2)
    EVENT_SEM eventSem;
   #else
    THREAD threadId;
   #endif
} GBM_THREAD_DEF;

/* --------------------------------------------------- */

#ifdef __IBMCPP__
 static void _Optlink Thread_threadFunc(void * arg)
#elif WIN32
 static unsigned __stdcall Thread_threadFunc(void * arg)
#elif LINUX
 static void * Thread_threadFunc(void * arg)
#else
 static void Thread_threadFunc(void * arg)
#endif
{
    GBM_THREAD_DEF * pThreadArgs = (GBM_THREAD_DEF *) arg;
    pThreadArgs->function(pThreadArgs->function_arg);

   #if defined(__OS2__) || defined(OS2)
    DosPostEventSem(pThreadArgs->eventSem);
   #endif

   #if defined(__OS2__) || defined(OS2)
    _endthread();
   #elif defined(__WIN32__) || defined(WIN32)
    _endthreadex(0);
    return 0;
   #elif defined(LINUX)
    pthread_exit(0);
   #endif
}

/* --------------------------------------------------- */
GBM_THREAD gbmmthrd_scheduleTask(GBM_THREAD_FUNC function,
                                 GBM_THREAD_ARG  function_arg,
                                 int initStackSize)
{
    GBM_THREAD_DEF * pThread = NULL;
    if (NULL == function)
    {
        return NULL;
    }
    pThread = (GBM_THREAD_DEF *)malloc(sizeof(GBM_THREAD_DEF));
    if (NULL == pThread)
    {
       return NULL;
    }
    pThread->function     = function;
    pThread->function_arg = function_arg;

   #if defined(__OS2__) || defined(OS2)
    /* DosWaitThread() is not reliable -> using event semaphore instead */
    pThread->eventSem = SEM_NULLHANDLE;
    if (NO_ERROR != DosCreateEventSem(NULL, &(pThread->eventSem), 0L, FALSE))
    {
        free(pThread);
        return NULL;
    }
    if (SEM_NULLHANDLE == pThread->eventSem)
    {
        free(pThread);
        return NULL;
    }
    {
        THREAD threadH = _beginthread(Thread_threadFunc, NULL,
                                      initStackSize, pThread);
        if (threadH == THREAD_NULL)
        {
            DosCloseMutexSem(pThread->eventSem);
            free(pThread);
            return NULL;
        }
    }
   #elif defined(__WIN32__) || defined(WIN32)
    {
        unsigned threadId;
        pThread->threadId = (THREAD) _beginthreadex(NULL, initStackSize,
                                                    Thread_threadFunc,
                                                    pThread, 0, &threadId);
        if (pThread->threadId == THREAD_NULL)
        {
            free(pThread);
            return NULL;
        }
    }
   #elif defined(LINUX)
    {
        pThread->threadId = 0;
        if (pthread_create(&(pThread->threadId), NULL,
                           &Thread_threadFunc, pThread))
        {
            free(pThread);
            return NULL;
        }
    }
   #else
    free(pThread);
    pThread = NULL;
   #endif

    return pThread;
}

/* --------------------------------------------------- */
void gbmmthrd_finishTask(GBM_THREAD *thread)
{
    if ((NULL != thread) && (NULL != *thread))
    {
       GBM_THREAD_DEF * pThread = (GBM_THREAD_DEF *) *thread;

       #if defined(__OS2__) || defined(OS2)
        /* DosWaitThread() is not reliable -> using event semaphore instead */
        DosWaitEventSem(pThread->eventSem, SEM_INDEFINITE_WAIT);
       #elif defined(__WIN32__) || defined(WIN32)
        WaitForSingleObject(pThread->threadId, INFINITE);
       #elif defined(LINUX)
        {
            void * retval = NULL;
            pthread_join(pThread->threadId, &retval);
        }
       #endif

       free(pThread);
       *thread = NULL;
    }
}

/* --------------------------------------------------- */
gbm_u32 gbmmthrd_getNumberOfCores(void)
{
    gbm_u32 numCores = 1;

   #if defined(__OS2__) || defined(OS2)
    {
        ULONG processors = 1;
        if (NO_ERROR == DosQuerySysInfo(QSV_NUMPROCESSORS,
                                        QSV_NUMPROCESSORS,
                                        &processors,
                                        sizeof(ULONG)))
        {
            numCores = (gbm_u32)processors;
        }
    }
   #elif defined(__WIN32__) || defined(WIN32)
    {
        SYSTEM_INFO sysinfo = { 0 };
        GetSystemInfo(&sysinfo);
        if (sysinfo.dwNumberOfProcessors >= 1)
        {
            DWORD i;
            numCores = 0;
            for (i = 1; i <= sysinfo.dwNumberOfProcessors; ++i)
            {
                if (i & sysinfo.dwActiveProcessorMask)
                {
                    ++numCores;
                }
            }
        }
    }
    #elif defined (LINUX)
    {
        numCores = sysconf(_SC_NPROCESSORS_ONLN); /* _SC_NPROCESSORS_CONF for all */
    }
    #endif

    assert(numCores >= 1);
    return (numCores >= 1 ? numCores : 1);
}


