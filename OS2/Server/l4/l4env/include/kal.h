#ifndef __OS3_KAL_H__
#define __OS3_KAL_H__

#ifdef __cplusplus
  extern "C" {
#endif

#include <l4/os3/gcc_os2def.h>
#include <l4/os3/processmgr.h>
#include <l4/os3/loader.h>
#include <l4/os3/types.h>
#include <l4/os3/kal2.h>

struct kal_init_struct
{
  unsigned long stack;
  void *l4rm_detach;
  void *l4rm_do_attach;
  void *l4rm_lookup;
  void *l4rm_lookup_region;
  void *l4rm_do_reserve;
  void *l4rm_set_userptr;
  void *l4rm_get_userptr;
  void *l4rm_area_release;
  void *l4rm_area_release_addr;
  void *l4rm_show_region_list;
  void *l4rm_region_mapper_id;
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
  unsigned long  eip;
  unsigned long  esp;
  //unsigned short sel;
  PTIB           tib;
  PPIB           pib;
  BYTE		 curdisk;
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

int
trampoline(struct param *param);

void __fiasco_gdt_set(void *desc, unsigned int size,
                      unsigned int entry_number_start, l4os3_cap_idx_t tid);
unsigned __fiasco_gdt_get_entry_offset(void);

int strlstlen(char *p);
int char_to_disknum(unsigned char chr_dsk);
unsigned char disknum_to_char(int i_dsk);
int isRelativePath(char *path);
unsigned char parse_drv(char *path);
char *parse_path(char *path, char *ret_buffer, int buf_len);
int attach_module (ULONG hmod);
int attach_all (ULONG hmod);
APIRET CDECL kalMapInfoBlocks(PTIB *pptib, PPIB *pppib);


void kalInit(struct kal_init_struct *s);

void kalEnter(void);

void kalQuit(void);

APIRET CDECL kalStartApp(char *name, char *pszLoadError, ULONG cbLoadError);

APIRET CDECL
kalOpenL (PSZ pszFileName,
          HFILE *phFile,
	  ULONG *pulAction,
	  LONGLONG cbFile,
	  ULONG ulAttribute,
	  ULONG fsOpenFlags,
	  ULONG fsOpenMode,
	  PEAOP2 peaop2);

APIRET CDECL
kalFSCtl (PVOID pData,
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
kalRead (HFILE hFile, PVOID pBuffer,
            ULONG cbRead, PULONG pcbActual);

APIRET CDECL
kalWrite (HFILE hFile, PVOID pBuffer,
              ULONG cbWrite, PULONG pcbActual);

APIRET CDECL
kalLogWrite (PSZ s);

VOID CDECL
kalExit(ULONG action, ULONG result);

APIRET CDECL
kalQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical);

APIRET CDECL
kalSetCurrentDir(PSZ pszDir);

APIRET CDECL
kalSetDefaultDisk(ULONG disknum);

APIRET CDECL
kalQueryCurrentDir(ULONG disknum,
                       PBYTE pBuf,
                       PULONG pcbBuf);

APIRET CDECL
kalQueryProcAddr(ULONG hmod,
                     ULONG ordinal,
                     const PSZ  pszName,
                     void  **ppfn);

APIRET CDECL
kalQueryModuleHandle(const char *pszModname,
                     unsigned long *phmod);

APIRET CDECL
kalQueryModuleName(unsigned long hmod, unsigned long cbBuf, char *pBuf);

unsigned long
kalPvtLoadModule(char *pszName,
              unsigned long cbName,
              char const *pszModname,
              os2exec_module_t *s,
              unsigned long *phmod);

APIRET CDECL
kalLoadModule(PSZ pszName,
                  ULONG cbName,
                  char const *pszModname,
                  PULONG phmod);

APIRET CDECL
kalQueryProcType(HMODULE hmod,
                 ULONG ordinal,
		 PSZ pszName,
		 PULONG pulProcType);

APIRET CDECL
kalQueryAppType(PSZ pszName,
                PULONG pFlags);

APIRET CDECL
kalExecPgm(char *pObjname,
           long cbObjname,
           unsigned long execFlag,
           char *pArg,
           char *pEnv,
           struct _RESULTCODES *pRes,
           char *pName);

APIRET CDECL
kalError(ULONG error);

APIRET CDECL
kalAllocMem(PVOID *ppb,
            ULONG cb,
	    ULONG flags);

APIRET CDECL
kalFreeMem(PVOID pb);

APIRET CDECL
kalSetMem(PVOID pb,
          ULONG cb,
	  ULONG flags);

APIRET CDECL
kalQueryMem(PVOID  pb,
            PULONG pcb,
	    PULONG pflags);

APIRET CDECL
kalAllocSharedMem(PPVOID ppb,
                  PSZ    pszName,
		  ULONG  cb,
		  ULONG  flags);

APIRET CDECL
kalResetBuffer(HFILE handle);

APIRET CDECL
kalSetFilePtrL(HFILE handle,
               LONGLONG ib,
	       ULONG method,
	       PULONGLONG ibActual);

APIRET CDECL
kalClose(HFILE handle);

APIRET CDECL
kalQueryHType(HFILE handle,
              PULONG pType,
	      PULONG pAttr);

APIRET CDECL
kalQueryDBCSEnv(ULONG cb,
                COUNTRYCODE *pcc,
		PBYTE pBuf);

APIRET CDECL
kalQueryCp(ULONG cb,
           PULONG arCP,
	   PULONG pcCP);

APIRET CDECL
kalMapInfoBlocks(PTIB *pptib, PPIB *pppib);

APIRET CDECL
kalGetInfoBlocks(PTIB *pptib, PPIB *pppib);

APIRET CDECL
kalSetMaxFH(ULONG cFH);

APIRET CDECL
kalSetRelMaxFH(PLONG pcbReqCount, PULONG pcbCurMaxFH);

APIRET CDECL
kalSleep(ULONG ms);

APIRET CDECL
kalDupHandle(HFILE hFile, HFILE *phFile2);

APIRET CDECL
kalDelete(PSZ pszFileName);

APIRET CDECL
kalForceDelete(PSZ pszFileName);

APIRET CDECL
kalDeleteDir(PSZ pszDirName);

APIRET CDECL
kalCreateDir(PSZ pszDirName, PEAOP2 peaop2);

APIRET CDECL
kalFindFirst(char  *pszFileSpec,
             HDIR  *phDir,
             ULONG flAttribute,
             PVOID pFindBuf,
             ULONG cbBuf,
             ULONG *pcFileNames,
             ULONG ulInfolevel);

APIRET CDECL
kalFindNext(HDIR  hDir,
            PVOID pFindBuf,
            ULONG cbBuf,
            ULONG *pcFileNames);

APIRET CDECL
kalFindClose(HDIR hDir);

APIRET CDECL
kalQueryFHState(HFILE hFile,
                PULONG pMode);

APIRET CDECL
kalSetFHState(HFILE hFile,
              ULONG pMode);

APIRET CDECL
kalQueryFileInfo(HFILE hf,
                 ULONG ulInfoLevel,
                 char *pInfo,
                 ULONG cbInfoBuf);

APIRET CDECL
kalQueryPathInfo(PSZ pszPathName,
                 ULONG ulInfoLevel,
                 PVOID pInfo,
                 ULONG cbInfoBuf);

APIRET CDECL
kalSetFileSizeL(HFILE hFile,
                long long cbSize);
APIRET CDECL
kalMove(PSZ pszOld, PSZ pszNew);

APIRET CDECL
kalOpenEventSem(PSZ pszName,
                PHEV phev);

APIRET CDECL
kalCloseEventSem(HEV hev);

APIRET CDECL
kalCreateEventSem(PSZ pszName,
                  PHEV phev,
                  ULONG flags,
                  BOOL32 fState);

APIRET CDECL
kalCreateThread(PTID ptid,
                PFNTHREAD pfn,
                ULONG param,
                ULONG flag,
                ULONG cbStack);

APIRET CDECL
kalSuspendThread(TID tid);

APIRET CDECL
kalResumeThread(TID tid);

APIRET CDECL
kalWaitThread(PTID ptid, ULONG option);

APIRET CDECL
kalKillThread(TID tid);

APIRET CDECL
kalGetTID(TID *ptid);

APIRET CDECL
kalGetPID(PID *ppid);

APIRET CDECL
kalGetL4ID(PID pid, TID tid, l4thread_t *id);

APIRET CDECL
kalGetTIDL4(l4_threadid_t id, TID *ptid);

APIRET CDECL
kalNewTIB(PID pid, TID tid, l4thread_t id);

APIRET CDECL
kalDestroyTIB(PID pid, TID tid);

APIRET CDECL
kalGetTIB(PTIB *ptib);

APIRET CDECL
kalGetPIB(PPIB *ppib);

#ifdef __cplusplus
  }
#endif

#endif
