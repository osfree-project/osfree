/* Thread API's */

#define  INCL_BASE
#include <os2.h>

#include <os3/io.h>
#include <os3/kal.h>
#include <os3/thread.h>
#include <os3/segment.h>

#include <l4/sys/syscalls.h>
#include <l4/sys/types.h>
#include <l4/env/errno.h>

/* Last used thread id */
static ULONG ulThread = 1;

void exit_func(l4thread_t tid, void *data);
static void thread_func(void *data);

/* Thread IDs array               */
extern l4_os3_thread_t ptid[MAX_TID];
/* Thread Info Block pointer      */
extern PTIB ptib[MAX_TID];

l4_os3_thread_t KalNativeID(void)
{
  l4_os3_thread_t id;

  id.thread = l4_myself();
  return id;
}

void exit_func(l4thread_t tid, void *data)
{
  l4_threadid_t t = l4thread_l4_id(l4thread_get_parent());
  l4_msgdope_t dope;

  // notify parent about our termination
  l4_ipc_send(t, (void *)(L4_IPC_SHORT_MSG | L4_IPC_DECEIT_MASK),
              tid, 0, L4_IPC_SEND_TIMEOUT_0, &dope);
}
L4THREAD_EXIT_FN_STATIC(fn, exit_func);

struct start_data
{
  PFNTHREAD pfn;
  ULONG param;
};

static void thread_func(void *data)
{
  struct start_data *start_data = (struct start_data *)data;
  PFNTHREAD pfn = start_data->pfn;
  ULONG param   = start_data->param;
  //l4thread_t        id;
  l4_os3_thread_t   thread;
  unsigned long     base;
  unsigned short    sel;
  struct desc       desc;
  PID pid;
  TID tid;

  //register exit func
  if ( l4thread_on_exit(&fn, NULL) < 0 )
    io_log("error setting the exit function!\n");
  else
    io_log("exit function set successfully\n");

  // get current process id
  KalGetPID(&pid);
  // get current thread id
  KalGetTID(&tid);
  // get l4thread thread id
  KalGetNativeID(pid, tid, &thread);

  /* initialize tid array */
  ptid[tid - 1] = thread;

  /* TIB base */
  base = (unsigned long)ptib[tid - 1];

  /* Prepare TIB GDT descriptor */
  desc.limit_lo = 0x30; desc.limit_hi = 0;
  desc.acc_lo   = 0xF3; desc.acc_hi   = 0;
  desc.base_lo1 = base & 0xffff;
  desc.base_lo2 = (base >> 16) & 0xff;
  desc.base_hi  = base >> 24;

  /* Allocate a GDT descriptor */
  //fiasco_gdt_set(&desc, sizeof(struct desc), 0, thread.thread);
  segment_gdt_set(&desc, sizeof(struct desc), 0, thread);

  /* Get a selector */
  sel = (sizeof(struct desc)) * segment_gdt_get_entry_offset();

  // set fs register to TIB selector
  //enter_kdebug("debug");
  asm(
      "movw  %[sel], %%dx \n"
      "movw  %%dx, %%fs \n"
      :
      :[sel]  "m"  (sel));

  // execute OS/2 thread function
  (*pfn)(param);
}

static void wait_func(void)
{
  l4_threadid_t src;
  l4_umword_t dw1, dw2;
  l4_msgdope_t dope;

  for (;;)
  {
    if ( l4_ipc_wait(&src, L4_IPC_SHORT_MSG,
                     &dw1, &dw2, L4_IPC_NEVER,
                     &dope) < 0 )
    {
      io_log("IPC error\n");
    }
  }
}

APIRET CDECL
KalCreateThread(PTID tid,
                PFNTHREAD pfn,
                ULONG param,
                ULONG flag,
                ULONG cbStack)
{
  l4_uint32_t flags = L4THREAD_CREATE_ASYNC;
  l4thread_t rc;
  l4_os3_thread_t thread;
  struct start_data data;
  PTIB tib;
  PID pid;

  KalEnter();

  if (flag & STACK_COMMITED)
    flags |= L4THREAD_CREATE_MAP;

  data.pfn = pfn;
  data.param = param;

  if ( (rc = l4thread_create_long(L4THREAD_INVALID_ID,
                       thread_func, "OS/2 thread",
                       L4THREAD_INVALID_SP, cbStack, L4THREAD_DEFAULT_PRIO,
                       &data, flags)) > 0)
  {
    // @todo watch the thread ids to be in [1..128] range
    ulThread++;
    *tid = ulThread;

    // get pid
    KalGetPID(&pid);
    // crsate TIB, update PTDA
    thread.thread = l4thread_l4_id(rc);
    KalNewTIB(pid, ulThread, thread);
    // get new TIB
    KalGetTIB(&ptib[ulThread - 1]);
    tib = ptib[ulThread - 1];
    tib->tib_eip_saved = 0;
    tib->tib_esp_saved = 0;

    // suspend thread if needed
    if (flag & CREATE_SUSPENDED)
      KalSuspendThread(ulThread);

    rc = NO_ERROR;
  }
  else
  {
    io_log("Thread creation error: %d\n", rc);

    switch (-rc)
    {
      case L4_EINVAL:     rc = ERROR_INVALID_PARAMETER; break;
      case L4_ENOTHREAD:  rc = ERROR_MAX_THRDS_REACHED; break;
      case L4_ENOMAP:
      case L4_ENOMEM:     rc = ERROR_NOT_ENOUGH_MEMORY; break;
      default:            rc = ERROR_INVALID_PARAMETER; // ???
    }
  }

  KalQuit();
  return rc;
}

APIRET CDECL
KalSuspendThread(TID tid)
{
  l4_threadid_t preempter = L4_INVALID_ID;
  l4_threadid_t pager     = L4_INVALID_ID;
  l4_os3_thread_t id;
  l4_umword_t eflags, eip, esp;
  PTIB tib;
  PID pid;

  KalEnter();

  // get pid
  KalGetPID(&pid);
  // get L4 native thread id
  KalGetNativeID(pid, tid, &id);

  if (l4_thread_equal(id.thread, L4_INVALID_ID))
  {
    KalQuit();
    return ERROR_INVALID_THREADID;
  }

  // suspend thread execution: set eip to -1
  l4_thread_ex_regs(id.thread, (l4_umword_t)wait_func, ~0,
                    &preempter, &pager,
                    &eflags, &eip, &esp);

  tib = ptib[tid - 1];
  tib->tib_eip_saved = eip;
  tib->tib_esp_saved = esp;
  KalQuit();
  return NO_ERROR;
}

APIRET CDECL
KalResumeThread(TID tid)
{
  l4_os3_thread_t id;
  l4_threadid_t preempter = L4_INVALID_ID;
  l4_threadid_t pager     = L4_INVALID_ID;
  l4_umword_t eflags, eip, esp, new_eip, new_esp;
  PTIB tib;
  PID pid;

  KalEnter();

  // get pid
  KalGetPID(&pid);
  // get L4 native thread id
  KalGetNativeID(pid, tid, &id);

  if (l4_thread_equal(id.thread, L4_INVALID_ID))
  {
    KalQuit();
    return ERROR_INVALID_THREADID;
  }

  tib = ptib[tid - 1];

  if (! tib->tib_eip_saved)
    return ERROR_NOT_FROZEN;

  new_eip = tib->tib_eip_saved;
  new_esp = tib->tib_esp_saved;

  // resume thread
  l4_thread_ex_regs(id.thread, new_eip, new_esp,
                    &preempter, &pager,
                    &eflags, &eip, &esp);

  tib->tib_eip_saved = 0;
  tib->tib_esp_saved = 0;
  KalQuit();
  return NO_ERROR;
}

APIRET CDECL
KalWaitThread(PTID ptid, ULONG option)
{
  l4_threadid_t me = l4_myself();
  l4_os3_thread_t id, src;
  l4_umword_t   dw1, dw2;
  l4_msgdope_t  dope;
  APIRET        rc = NO_ERROR;
  TID           tid = 0;
  PID           pid;

  KalEnter();

  // get pid
  KalGetPID(&pid);

  if (! ptid)
    ptid = &tid;

  // get native L4 id
  KalGetNativeID(pid, *ptid, &id);

  // wait until needed thread terminates
  switch (option)
  {
    case DCWW_WAIT:
      for (;;)
      {
        if (! l4_ipc_wait(&src.thread, L4_IPC_SHORT_MSG,
                          &dw1, &dw2, L4_IPC_NEVER,
                          &dope) &&
            l4_task_equal(src.thread, me) )
        {
          if (*ptid)
          {
            if (l4_thread_equal(id.thread, L4_INVALID_ID))
            {
              rc = ERROR_INVALID_THREADID;
              break;
            }

            if (l4_thread_equal(src.thread, id.thread))
              break;
          }
          else
          {
            KalGetTIDNative(src, ptid);
            break;
          }
        }
      }
      break;

    case DCWW_NOWAIT: // ???
      if (l4_thread_equal(id.thread, L4_INVALID_ID))
        rc = ERROR_INVALID_THREADID;
      else
        rc = ERROR_THREAD_NOT_TERMINATED;
      break;

    default:
      rc = ERROR_INVALID_PARAMETER;
  }

  KalQuit();
  return rc;
}

APIRET CDECL
KalKillThread(TID tid)
{
  l4_os3_thread_t id;
  PID pid;
  APIRET rc = NO_ERROR;

  KalEnter();

  // get current task pid
  KalGetPID(&pid);
  // get L4 native thread ID
  KalGetNativeID(pid, tid, &id);

  if (l4_thread_equal(id.thread, L4_INVALID_ID))
  {
    KalQuit();
    return ERROR_INVALID_THREADID;
  }

  if (! (rc = l4thread_shutdown(l4thread_id(id.thread))) )
    io_log("thread killed\n");
  else
    io_log("thread kill failed!\n");

  // free thread TIB
  KalDestroyTIB(pid, tid);

  KalQuit();
  return rc;
}
