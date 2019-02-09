#ifndef __OS3_KAL_H__
#define __OS3_KAL_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/types.h>
#include <os3/processmgr.h>
#include <os3/loader.h>

struct kal_init_struct
{
  unsigned long stack;
  void *shared_memory_base;
  unsigned long shared_memory_size;
  unsigned long long shared_memory_area;
  unsigned long service_lthread;
  void *l4rm_detach;
  void *l4rm_do_attach;
  void *l4rm_lookup;
  void *l4rm_lookup_region;
  void *l4rm_do_reserve;
  void *l4rm_do_reserve_in_area;
  void *l4rm_set_userptr;
  void *l4rm_get_userptr;
  void *l4rm_area_release;
  void *l4rm_area_release_addr;
  void *l4rm_show_region_list;
  void *l4rm_region_mapper_id;
  void *l4dm_memphys_copy;
  void *l4env_get_default_dsm;
  void *l4thread_exit;
  void *l4thread_on_exit;
  void *l4thread_create_long;
  void *l4thread_shutdown;
  void *l4thread_l4_id;
  void *l4thread_get_parent;
  void *fiasco_gdt_set;
  void *fiasco_gdt_get_entry_offset;
  char *logtag;
};

/* trampoline() params    */
struct param
{
  /* a system-dependent structure*/
  //IXFSYSDEP *sysdep;

  /* OS/2-specific params */
  void           *eip;
  void           *esp;
  //unsigned short sel;
  PTIB           tib;
  PPIB           pib;
  BYTE           curdisk;
};

/* GDT/LDT descriptor structure */
struct desc
{
  short limit_lo :16;
  short base_lo1 :16;
  short base_lo2 :8;
  short acc_lo   :8;
  short limit_hi :4;
  short acc_hi   :4;
  short base_hi  :8;
};

typedef struct vmdata
{
  char             name[256]; // name for named shared mem
  char             is_shared; // is shared
  //l4_os3_cap_idx_t owner;     // shared memory owner thread
  PID              owner;     // shared memory owner thread
  unsigned long    refcnt;    // reference count for shared mem
  unsigned long    rights;    // OS/2-style access flags
  unsigned long long area;    // area id
  void             *addr;     // area address
  unsigned long    size;      // area size
  struct vmdata    *next;     // link to the next record
  struct vmdata    *prev;     // link to the previous record
} vmdata_t;

struct mod_list;

struct mod_list
{
  struct mod_list *next, *prev;
  unsigned long hmod;
};

typedef struct mod_list mod_list_t;

mod_list_t *module_add(mod_list_t *list, unsigned long hmod);
void module_del(mod_list_t *list, unsigned long hmod);
void module_list_free(mod_list_t *list);
BOOL module_present(mod_list_t *list, unsigned long hmod);

int
trampoline(struct param *param);

void __fiasco_gdt_set(void *desc, unsigned int size,
                      unsigned int entry_number_start, l4_os3_cap_idx_t tid);
unsigned __fiasco_gdt_get_entry_offset(void);

int strlstlen(char *p);
int char_to_disknum(unsigned char chr_dsk);
unsigned char disknum_to_char(int i_dsk);
int isRelativePath(char *path);
unsigned char parse_drv(char *path);
char *parse_path(char *path, char *ret_buffer, int buf_len);
long attach_module (ULONG hmod, unsigned long long area);
long attach_all (mod_list_t **list, ULONG hmod, unsigned long long area);
APIRET CDECL KalMapInfoBlocks(PTIB *pptib, PPIB *pppib);


void KalInit(struct kal_init_struct *s);

void KalEnter(void);

void KalQuit(void);

APIRET CDECL KalStartApp(char *name, char *pszLoadError, ULONG cbLoadError);

APIRET CDECL KalDlOpen(PSZ name, PULONG handle);
APIRET CDECL KalDlSym(ULONG handle, PSZ sym, PPVOID addr);
APIRET CDECL KalDlRoute(ULONG handle, PSZ name, ...);

APIRET CDECL
KalOpenL (PSZ pszFileName,
          HFILE *phFile,
	  ULONG *pulAction,
	  LONGLONG cbFile,
	  ULONG ulAttribute,
	  ULONG fsOpenFlags,
	  ULONG fsOpenMode,
	  PEAOP2 peaop2);

APIRET CDECL
KalFSCtl (PVOID pData,
          ULONG cbData,
	  PULONG pcbData,
	  PVOID pParms,
	  ULONG cbParms,
	  PULONG pcbParms,
	  ULONG function,
	  PSZ pszRoute,
	  HFILE hFile,
	  ULONG method);

APIRET CDECL
KalRead (HFILE hFile, PVOID pBuffer,
            ULONG cbRead, PULONG pcbActual);

APIRET CDECL
KalWrite (HFILE hFile, PVOID pBuffer,
              ULONG cbWrite, PULONG pcbActual);

APIRET CDECL
KalLogWrite (PSZ s);

VOID CDECL
KalExit(ULONG action, ULONG result);

APIRET CDECL
KalQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical);

APIRET CDECL
KalSetCurrentDir(PSZ pszDir);

APIRET CDECL
KalSetDefaultDisk(ULONG disknum);

APIRET CDECL
KalQueryCurrentDir(ULONG disknum,
                       PBYTE pBuf,
                       PULONG pcbBuf);

APIRET CDECL
KalQueryProcAddr(ULONG hmod,
                     ULONG ordinal,
                     const PSZ  pszName,
                     void  **ppfn);

APIRET CDECL
KalQueryModuleHandle(const char *pszModname,
                     unsigned long *phmod);

APIRET CDECL
KalQueryModuleName(unsigned long hmod, unsigned long cbBuf, char *pBuf);

unsigned long
KalPvtLoadModule(char *pszName,
              unsigned long *pcbName,
              char const *pszModname,
              os2exec_module_t *s,
              unsigned long *phmod);

APIRET CDECL
KalLoadModule(PSZ pszName,
                  ULONG cbName,
                  char const *pszModname,
                  PULONG phmod);

APIRET CDECL
KalFreeModule(ULONG hmod);

APIRET CDECL
KalQueryProcType(HMODULE hmod,
                 ULONG ordinal,
		 PSZ pszName,
		 PULONG pulProcType);

APIRET CDECL
KalQueryAppType(PSZ pszName,
                PULONG pFlags);

APIRET CDECL
KalExecPgm(char *pObjname,
           long cbObjname,
           unsigned long execFlag,
           char *pArg,
           char *pEnv,
           struct _RESULTCODES *pRes,
           char *pName);

APIRET CDECL
KalError(ULONG error);

APIRET CDECL
KalAllocMem(PVOID *ppb,
            ULONG cb,
	    ULONG flags);

APIRET CDECL
KalFreeMem(PVOID pb);

APIRET CDECL
KalSetMem(PVOID pb,
          ULONG cb,
	  ULONG flags);

APIRET CDECL
KalQueryMem(PVOID  pb,
            PULONG pcb,
	    PULONG pflags);

APIRET CDECL
KalAllocSharedMem(PPVOID ppb,
                  PSZ    pszName,
		  ULONG  cb,
		  ULONG  flags);

APIRET CDECL
KalGetSharedMem(PVOID pb,
                ULONG flag);

APIRET CDECL
KalGetNamedSharedMem(PPVOID ppb,
                     PSZ pszName,
                     ULONG flag);

APIRET CDECL
KalGiveSharedMem(PVOID pb,
                 PID pid,
                 ULONG flag);

APIRET CDECL
KalResetBuffer(HFILE handle);

APIRET CDECL
KalSetFilePtrL(HFILE handle,
               LONGLONG ib,
	       ULONG method,
	       PULONGLONG ibActual);

APIRET CDECL
KalClose(HFILE handle);

APIRET CDECL
KalQueryHType(HFILE handle,
              PULONG pType,
	      PULONG pAttr);

APIRET CDECL
KalQueryDBCSEnv(ULONG cb,
                COUNTRYCODE *pcc,
		PBYTE pBuf);

APIRET CDECL
KalQueryCp(ULONG cb,
           PULONG arCP,
	   PULONG pcCP);

APIRET CDECL
KalMapInfoBlocks(PTIB *pptib, PPIB *pppib);

APIRET CDECL
KalGetInfoBlocks(PTIB *pptib, PPIB *pppib);

APIRET CDECL
KalSetMaxFH(ULONG cFH);

APIRET CDECL
KalSetRelMaxFH(PLONG pcbReqCount, PULONG pcbCurMaxFH);

APIRET CDECL
KalSleep(ULONG ms);

APIRET CDECL
KalDupHandle(HFILE hFile, HFILE *phFile2);

APIRET CDECL
KalDelete(PSZ pszFileName);

APIRET CDECL
KalForceDelete(PSZ pszFileName);

APIRET CDECL
KalDeleteDir(PSZ pszDirName);

APIRET CDECL
KalCreateDir(PSZ pszDirName, PEAOP2 peaop2);

APIRET CDECL
KalFindFirst(char  *pszFileSpec,
             HDIR  *phDir,
             ULONG flAttribute,
             PVOID pFindBuf,
             ULONG cbBuf,
             ULONG *pcFileNames,
             ULONG ulInfolevel);

APIRET CDECL
KalFindNext(HDIR  hDir,
            PVOID pFindBuf,
            ULONG cbBuf,
            ULONG *pcFileNames);

APIRET CDECL
KalFindClose(HDIR hDir);

APIRET CDECL
KalQueryFHState(HFILE hFile,
                PULONG pMode);

APIRET CDECL
KalSetFHState(HFILE hFile,
              ULONG pMode);

APIRET CDECL
KalQueryFileInfo(HFILE hf,
                 ULONG ulInfoLevel,
                 char *pInfo,
                 ULONG cbInfoBuf);

APIRET CDECL
KalQueryPathInfo(PSZ pszPathName,
                 ULONG ulInfoLevel,
                 PVOID pInfo,
                 ULONG cbInfoBuf);

APIRET CDECL
KalSetFileSizeL(HFILE hFile,
                long long cbSize);
APIRET CDECL
KalMove(PSZ pszOld, PSZ pszNew);

APIRET CDECL
KalOpenEventSem(PSZ pszName,
                PHEV phev);

APIRET CDECL
KalCloseEventSem(HEV hev);

APIRET CDECL
KalCreateEventSem(PSZ pszName,
                  PHEV phev,
                  ULONG flags,
                  BOOL32 fState);

APIRET CDECL
KalCreateThread(PTID ptid,
                PFNTHREAD pfn,
                ULONG param,
                ULONG flag,
                ULONG cbStack);

APIRET CDECL
KalSuspendThread(TID tid);

APIRET CDECL
KalResumeThread(TID tid);

APIRET CDECL
KalWaitThread(PTID ptid, ULONG option);

APIRET CDECL
KalKillThread(TID tid);

l4_os3_thread_t CDECL
KalNativeID(void);

APIRET CDECL
KalGetTID(TID *ptid);

APIRET CDECL
KalGetPID(PID *ppid);

APIRET CDECL
KalGetNativeID(PID pid, TID tid, l4_os3_thread_t *id);

APIRET CDECL
KalGetTIDNative(l4_os3_thread_t id, TID *ptid);

APIRET CDECL
KalNewTIB(PID pid, TID tid, l4_os3_thread_t id);

APIRET CDECL
KalDestroyTIB(PID pid, TID tid);

APIRET CDECL
KalGetTIB(PTIB *ptib);

APIRET CDECL
KalGetPIB(PPIB *ppib);

#ifdef __cplusplus
  }
#endif

#endif
