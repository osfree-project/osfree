/*
 *  API implementations
 *  parts (on the client side)
 *  These functions are those which are
 *  exported from the KAL.DLL virtual library.
 */

#include <l4/os3/gcc_os2def.h>
#include <l4/os3/apistub.h>

#include <dice/dice.h>

#include <l4/os3/os2server-client.h>
#include <l4/execsrv/os2exec-client.h>

#include <l4/l4rm/l4rm.h>
#include <l4/env/env.h>
#include <l4/env/errno.h>
#include <l4/log/l4log.h>
#include <l4/names/libnames.h>
#include <l4/thread/thread.h>
#include <l4/semaphore/semaphore.h>
#include <l4/dm_generic/consts.h>
#include <l4/dm_mem/dm_mem.h>
#include <l4/sys/kdebug.h>>
#include <l4/generic_ts/generic_ts.h>>

#include "../stacksw.h"

extern l4_threadid_t fs;
extern l4_threadid_t os2srv;
extern l4_threadid_t execsrv;
extern unsigned long __stack;

static ULONG CurMaxFH = 20;

unsigned long
PvtLoadModule(char *pszName,
              unsigned long cbName,
              char const *pszModname,
              os2exec_module_t *s,
              unsigned long *phmod);


int
strlstlen(char *p)
{
  int l, len = 0;

  if (!p || !*p)
    return 2;
    
  while (*p) // skip all lines; break if NULL
  {
    /* skip one string */
    l = strlen(p) + 1;
    /* skip NULL character */
    p   += l;
    len += l;
  }

  if (!len)
    len++;
    
  len++; // include NULL symbol

  return len;
}


APIRET CDECL
KalOpenL (PSZ pszFileName,
          HFILE *phFile,
	  ULONG *pulAction,
	  LONGLONG cbFile,
	  ULONG ulAttribute,
	  ULONG fsOpenFlags,
	  ULONG fsOpenMode,
	  PEAOP2 peaop2)
{
  CORBA_Environment env = dice_default_environment;
  EAOP2 eaop2;
  APIRET  rc;

  STKIN
  LOG("pszFileName=%s", pszFileName);
  LOG("ulAction=%x", *pulAction);
  LOG("cbFile=%u", cbFile);
  LOG("ulAttribute=%x", ulAttribute);
  LOG("fsOpenFlags=%x", fsOpenFlags);
  LOG("fsOpenMode=%x", fsOpenMode);
  if (peaop2 == NULL)
    peaop2 = &eaop2;
  rc = os2fs_dos_OpenL_call (&fs, pszFileName, phFile,
                      pulAction, cbFile, ulAttribute,
                      fsOpenFlags, fsOpenMode, peaop2, &env);
  LOG("hFile=%x", *phFile);
  LOG("exit");
  STKOUT
  return rc;
}

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
	  ULONG method)
{
  CORBA_Environment env = dice_default_environment;
  APIRET  rc;

  STKIN
  LOG("pData=%x", pData);
  LOG("cbData=%u", cbData);
  LOG("pParms=%x", pParms);
  LOG("cbParms=%u", cbParms);
  LOG("function=%x", function);
  LOG("pszRoute=%s", pszRoute);
  LOG("hFile=%x", hFile);
  LOG("method=%x", method);
  // ...
  LOG("*pcbData=%u", *pcbData);
  LOG("*pcbParms=%u", *pcbParms);
  STKOUT
  return rc;
}

APIRET CDECL
KalRead (HFILE hFile, PVOID pBuffer,
            ULONG cbRead, PULONG pcbActual)
{
  CORBA_Environment env = dice_default_environment;
  APIRET  rc;

  STKIN

  LOG("started");
  LOG("hFile=%x", hFile);
  LOG("pBuffer=%x", pBuffer);
  LOG("cbRead=%u", cbRead);
  
  if (!cbRead)
  {
    STKOUT
    return 0; /* NO_ERROR */
  }

  rc = os2fs_dos_Read_call(&fs, hFile, &pBuffer, &cbRead, &env);

  *pcbActual = cbRead;

  LOG("*pcbActual=%u", *pcbActual);
  LOG("ended");
  STKOUT
  return rc;
}


APIRET CDECL
KalWrite (HFILE hFile, PVOID pBuffer,
              ULONG cbWrite, PULONG pcbActual)
{
  CORBA_Environment env = dice_default_environment;
  APIRET  rc;

  STKIN

  LOG("started");
  LOG("hFile=%x", hFile);
  LOG("pBuffer=%x", pBuffer);
  LOG("cbWrite=%u", cbWrite);

  if (!cbWrite)
  {
    STKOUT
    return 0; /* NO_ERROR */
  }

  rc = os2fs_dos_Write_call(&fs, hFile, pBuffer, &cbWrite, &env);

  *pcbActual = cbWrite;

  LOG("*pcbActual=%u", *pcbActual);
  LOG("ended");
  STKOUT
  return rc;
}

APIRET CDECL
KalLogWrite (PSZ s)
{
  STKIN
  LOG_printf("%s", s);
  STKOUT
  return 0; /* NO_ERROR */
}

VOID CDECL
KalExit(ULONG action, ULONG result)
{
  CORBA_Environment env = dice_default_environment;

  STKIN
  // send OS/2 server a message that we want to terminate
  LOG("action=%u", action);
  LOG("result=%u", result);
  os2server_dos_Exit_send(&os2srv, action, result, &env);
  // tell L4 task server that we want to terminate
  //l4_ipc_sleep(L4_IPC_NEVER);
  l4ts_exit();
  STKOUT
}


APIRET CDECL
KalQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  rc = os2server_dos_QueryCurrentDisk_call(&os2srv, pdisknum, plogical, &env);
  LOG("*pdisknum=%u", *pdisknum);
  LOG("*logical=%x", *plogical);
  STKOUT
  return rc;
}

APIRET CDECL
KalSetCurrentDir(PSZ pszDir)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("pszDir=%s", pszDir);
  rc = os2server_dos_SetCurrentDir_call(&os2srv, pszDir, &env);
  STKOUT
  return rc;
}

APIRET CDECL
KalSetDefaultDisk(ULONG disknum)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("disknum=%u", disknum);
  rc = os2server_dos_SetDefaultDisk_call(&os2srv, disknum, &env);
  STKOUT
  return rc;
}

APIRET CDECL
KalQueryCurrentDir(ULONG disknum,
                       PBYTE pBuf,
                       PULONG pcbBuf)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  char buf[0x100];
  STKIN
  LOG("disknum=%u", disknum);
  rc = os2server_dos_QueryCurrentDir_call(&os2srv, disknum, &pBuf, pcbBuf, &env);
  strncpy(buf, pBuf, *pcbBuf);
  buf[*pcbBuf] = '\0';
  LOG("dir=%s", buf);
  STKOUT
  return rc;
}


APIRET CDECL
KalQueryProcAddr(ULONG hmod,
                     ULONG ordinal,
                     const PSZ  pszName,
                     void  **ppfn)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("hmod=%x", hmod);
  LOG("ordinal=%u", ordinal);
  LOG("pszName=%s", pszName);
  rc = os2exec_query_procaddr_call(&execsrv, hmod, ordinal,
                                   pszName, (void **)ppfn, &env);
  LOG("*ppfn=%x", *ppfn);
  STKOUT
  return rc;
}

APIRET CDECL
KalQueryModuleHandle(const char *pszModname,
                     unsigned long *phmod)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("pszModname=%s", pszModname);
  rc = os2exec_query_modhandle_call(&execsrv, pszModname,
                                    phmod, &env);
  LOG("*phmod=%x", *phmod);
  STKOUT
  return rc;
}

APIRET CDECL
KalQueryModuleName(unsigned long hmod, unsigned long cbBuf, char *pBuf)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("hmod=%x", hmod);
  LOG("cbBuf=%u", cbBuf);
  rc = os2exec_query_modname_call(&execsrv, hmod,
                                  cbBuf, &pBuf, &env);
  LOG("pBuf=%s", pBuf);
  STKOUT
  return rc;
}

/** attach dataspace to our address space. (any free address) */
int
attach_ds(l4dm_dataspace_t *ds, l4_uint32_t flags, l4_addr_t *addr)
{
  int error;
  l4_size_t size;
 
  if ((error = l4dm_mem_size(ds, &size)))
    {
      printf("Error %d (%s) getting size of dataspace\n",
	  error, l4env_errstr(error));
      return error;
    }
  
  if ((error = l4rm_attach(ds, size, 0, flags, addr)))
    {
      printf("Error %d (%s) attaching dataspace\n",
	  error, l4env_errstr(error));
      return error;
    }
  return 0;
}

/** attach dataspace to our address space. (concrete address) */
int
attach_ds_reg(l4dm_dataspace_t ds, l4_uint32_t flags, l4_addr_t addr)
{
  int error;
  l4_size_t size;
  l4_addr_t a = addr;

  /* get dataspace size */
  if ((error = l4dm_mem_size(&ds, &size)))
    {
      printf("Error %d (%s) getting size of dataspace\n",
	  error, l4env_errstr(error));
      return error;
    }

  /* attach it to a given region */  
  if ((error = l4rm_attach_to_region(&ds, a, size, 0, flags)))
    {
      printf("Error %d (%s) attaching dataspace\n",
	  error, l4env_errstr(error));
      return error;
    }

  return 0;
}

/*  Attaches all sections
 *  for a given module
 */
int
attach_module (ULONG hmod)
{
  CORBA_Environment env = dice_default_environment;
  l4exec_section_t sect;
  l4dm_dataspace_t ds, area_ds;
  l4_uint32_t flags;
  l4_addr_t   addr, map_addr;
  l4_size_t   map_size;
  l4_offs_t   offset;
  l4_threadid_t pager;
  unsigned type;
  int index;
  ULONG rc;

  index = 0; rc = 0;
  while (!os2exec_getsect_call (&execsrv, hmod, &index, &sect, &env) && !rc)
  {
    ds    = sect.ds; 
    addr  = sect.addr;
    type  = sect.info.type;
    flags = 0;  

    LOG("dataspace %x", ds); 
    LOG("addr %x", addr);
    
    if (type & L4_DSTYPE_READ)
      flags |= L4DM_READ;

    if (type & L4_DSTYPE_WRITE)
      flags |= L4DM_WRITE;

    if ((rc = l4rm_lookup(addr, &map_addr, &map_size,
                    &area_ds, &offset, &pager)) != L4RM_REGION_DATASPACE)
    {
      rc = attach_ds_reg (ds, flags, addr);
      if (!rc) 
        LOG("attached");
      else
      {
        LOG("attach_ds_reg returned %u", rc);
        break;
      }
    }
    else
    {
      LOG("map_addr=%x, map_size=%u, area_ds=%x",
          map_addr, map_size, area_ds);
      break;
    }
  }


  return 0; //rc;
}


/*  Attaches recursively a module and
 *  all its dependencies
 */
int
attach_all (ULONG hmod)
{
  CORBA_Environment env = dice_default_environment;
  ULONG imp_hmod, rc = 0;
  int index = 0;

  LOG("attach_all called");
  rc = attach_module(hmod);
  
  if (rc)
    return rc;
    
  while (!os2exec_getimp_call (&execsrv, hmod, &index, &imp_hmod, &env) && !rc)
  {
    if (!imp_hmod) // KAL fake module: no need to attach sections
      continue;
    //  return 0;
  
    rc = attach_all(imp_hmod);
  }

  LOG("attach_all returned: %u", rc);
  
  return rc;
}


unsigned long
PvtLoadModule(char *pszName,
              unsigned long cbName,
              char const *pszModname,
              os2exec_module_t *s,
              unsigned long *phmod)
{
  CORBA_Environment env = dice_default_environment;
  l4dm_dataspace_t ds = L4DM_INVALID_DATASPACE;
  ULONG hmod, rc;

  LOG("PvtLoadModule called");
  rc = os2exec_open_call (&execsrv, pszModname, &ds,
                          1, &hmod, &env);
  if (rc)
    return rc;

  *phmod = hmod;

  LOG("os2exec_open_call() called, rc=%d", rc);
  rc = os2exec_load_call (&execsrv, hmod, s, &env);

  if (rc)
    return rc;

  LOG("os2exec_load_call() called, rc=%d", rc);
  rc = os2exec_share_call (&execsrv, hmod, &env);

  if (rc)
    return rc;

  LOG("os2exec_share_call() called, rc=%d", rc);
  rc = attach_all(hmod);

  if (rc)
    return rc;

  LOG("attach_all() called, rc=%d", rc);

  return 0;
}


APIRET CDECL
KalLoadModule(PSZ pszName,
                  ULONG cbName,
                  char const *pszModname,
                  PULONG phmod)
{
  os2exec_module_t s;
  int rc;
  STKIN
  LOG("pszName=%s");
  LOG("cbName=%u", cbName);
  rc = PvtLoadModule(pszName, cbName, pszModname,
                       &s, phmod);
  LOG("pszModname=%s", pszModname);
  LOG("*phmod=%x", *phmod);
  STKOUT
  return rc;
}

APIRET CDECL
KalExecPgm(char * pObjname,
               long cbObjname,
               unsigned long execFlag,
               char * pArg,
               char * pEnv,
               struct _RESULTCODES *pRes,
               char * pName)
{
  CORBA_Environment env = dice_default_environment;
  APIRET rc;
  int i, j, k, l;
  char *p;

  STKIN
  LOG("started");
  LOG("cbObjname=%x", cbObjname);
  LOG("pName=%s", pName);
  LOG("execFlag=%x", execFlag);
  
  if (pArg == NULL)
  {
    pArg = "\0\0";
    i = 2;
  }
  else
    i = strlstlen(pArg);

  if (pEnv == NULL)
  {
    pEnv = "\0\0";
    j = 2;
  }
  else
    j = strlstlen(pEnv);
  
  l = strlstlen(pArg);
  LOG("pArg len=%d", l);
  LOG("pEnv len=%d", strlstlen(pEnv));

  LOG("pArg=%x", pArg);
  
  LOG("len of pArg=%d", l);
  for (k = 0, p = pArg; k < l; k++)
    if (p[k])
      LOG("%c", p[k]);
    else
      LOG("\\0");
  rc =  os2server_dos_ExecPgm_call (&os2srv, &pObjname,
                        &cbObjname, execFlag, pArg, i,
			pEnv, j,
                        pRes, pName, &env);
  LOG("pRes=%x", pRes);
  LOG("pObjname=%x",  pObjname);
  LOG("ended");
  STKOUT
  return rc;
}

APIRET CDECL
KalError(ULONG error)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("error=%x", error);
  rc = os2server_dos_Error_call (&os2srv, error, &env);
  STKOUT
  return rc;
}


//#define PAG_COMMIT   0x00000010
//#define OBJ_TILE     0x00000040

//#define PAG_EXECUTE  0x00000004
//#define PAG_READ     0x00000001
//#define PAG_WRITE    0x00000002
//#define PAG_GUARD    0x00000008

APIRET CDECL
KalAllocMem(PVOID *ppb,
            ULONG cb,
	    ULONG flags)
{
  l4_uint32_t rights = 0;
  l4_uint32_t area;
  l4dm_dataspace_t ds;
  l4_addr_t addr;
  int rc;

  //enter_kdebug(">");
  STKIN
  LOG("cb=%d", cb);
  LOG("flags=%x", flags);

  if (flags & PAG_READ)
    rights |= L4DM_READ;

  if (flags & PAG_WRITE)
    rights |= L4DM_WRITE;

  if (flags & PAG_COMMIT)
  {
    //enter_kdebug(">");
    /* Create a dataspace of a given size */
    rc = l4dm_mem_open(L4DM_DEFAULT_DSM, cb,
               4096, rights, "DosAllocMem dataspace", &ds);

    //enter_kdebug(">");
    if (rc < 0)
    {
      STKOUT
      return 8; /* ERROR_NOT_ENOUGH_MEMORY */
    }

    /* attach the created dataspace to our address space */
    rc = attach_ds(&ds, rights, &addr);

    //enter_kdebug(">");
    if (rc)
    {
      STKOUT
      return 8; /* What to return? */
    }
  }
  else
  {
    rc = l4rm_area_reserve(cb, 0, &addr, &area);

    if (rc < 0)
    {
      STKOUT
      return 8; /* ERROR_NOT_ENOUGH_MEMORY */
    }
  }
  
  *ppb = (void *)addr;

  LOG("*ppb=%x", addr);
  //enter_kdebug(">");
  STKOUT
  return 0; /* NO_ERROR */
}

APIRET CDECL
KalFreeMem(PVOID pb)
{
  int rc;
  l4_addr_t addr;
  l4_size_t size;
  l4_offs_t offset;
  l4_threadid_t pager;
  l4dm_dataspace_t ds;

  STKIN
  LOG("pb=%x", pb);
  
  rc = l4rm_lookup_region((l4_addr_t)pb, &addr, &size, &ds,
                     &offset, &pager);
  
  if (rc)
  {
    STKOUT
    return 487; /* ERROR_INVALID_ADDRESS */
  }
    
  rc = l4rm_detach(addr);
  
  if (rc)
  {
    STKOUT
    return 5; /* ERROR_ACCESS_DENIED */
  }
    
  rc = l4dm_close(&ds);
  
  if (rc)
  {
    STKOUT
    return 5; /* ERROR_ACCESS_DENIED */      
  }

  STKOUT
  return 0; /* NO_ERROR */
}

APIRET CDECL
KalResetBuffer(HFILE handle)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("handle=%x", handle);
  rc = os2fs_dos_ResetBuffer_call (&fs, handle, &env);
  STKOUT
  return rc;
}

APIRET CDECL
KalSetFilePtrL(HFILE handle,
               LONGLONG ib,
	       ULONG method,
	       PLONGLONG ibActual)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("handle=%x", handle);
  LOG("ib=%d", ib);
  LOG("method=%x", method);
  rc = os2fs_dos_SetFilePtrL_call (&fs, handle, ib,
                                  method, ibActual, &env);
  LOG("ibActual=%u", *ibActual);
  STKOUT
  return rc;
}

APIRET CDECL
KalClose(HFILE handle)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("handle=%x", handle);
  rc = os2fs_dos_Close_call (&fs, handle, &env);
  STKOUT
  return rc;
}

APIRET CDECL
KalQueryHType(HFILE handle,
              PULONG pType,
	      PULONG pAttr)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("handle=%x", handle);
  rc = os2fs_dos_QueryHType_call(&fs, handle, pType, pAttr, &env);
  LOG("Type=%x", *pType);
  LOG("Attr=%x", *pAttr);
  STKOUT
  return rc;
}

APIRET CDECL
KalQueryDBCSEnv(ULONG cb,
                COUNTRYCODE *pcc,
		PBYTE pBuf)
{
  CORBA_Environment env = dice_default_environment;
  int rc;
  STKIN
  LOG("cb=%u", cb);
  LOG("pcc=%x", pcc);
  rc = os2server_dos_QueryDBCSEnv_call (&os2srv, &cb, pcc, &pBuf, &env);
  LOG("pBuf=%x", pBuf);
  STKOUT
  return rc;
}

APIRET CDECL
KalQueryCp(ULONG cb,
           PULONG arCP,
	   PULONG pcCP)
{
  CORBA_Environment env = dice_default_environment;
  APIRET rc;
  //enter_kdebug(">");
  STKIN
  LOG("cb=%u", cb);
  rc = os2server_dos_QueryCp_call (&os2srv, &cb, &arCP, &env);
  //enter_kdebug(">");
  *pcCP = cb;
  LOG("arCP=%x", arCP);
  LOG("pcCP=%x", pcCP);
  LOG("*arCP=%u", *arCP);
  LOG("*pcCP=%u", *pcCP);
  STKOUT
  return rc;
}

APIRET CDECL
KalGetInfoBlocks(PTIB *pptib, PPIB *pppib)
{
  CORBA_Environment env = dice_default_environment;
  l4dm_dataspace_t ds;
  l4_addr_t addr;
  l4_offs_t tib_offs, pib_offs;
  APIRET rc;

  STKIN
  /* get the dataspace with info blocks */
  rc = os2server_dos_GetInfoBlocks_call (&os2srv, &ds,
                           &tib_offs, &pib_offs, &env);

  if (rc)
  {
    STKOUT
    return rc;
  }
 
  /* attach it */ 
  rc = attach_ds(&ds, L4DM_RW, &addr);
  if (rc)
  {
    LOG("error attaching ds!");
    STKOUT
    return rc;
  }
  else
    LOG("ds attached");

  *pptib = (PTIB)((char *)addr + tib_offs);
  *pppib = (PPIB)((char *)addr + pib_offs);

  (*pptib)->tib_ptib2 = (PTIB2)((char *)(*pptib)->tib_ptib2 + addr);
  
  /* fixup fields */
  (*pppib)->pib_pchcmd = (char *)((*pppib)->pib_pchcmd) + addr;
  (*pppib)->pib_pchenv = (char *)((*pppib)->pib_pchenv) + addr;

  /* detach dataspace */
  rc = l4rm_detach(addr);
  
  if (rc)
  {
    LOG("error detaching ds!");
    STKOUT
    return rc;
  }
  else
    LOG("ds detached");

  /* then attach it readonly */
  rc = attach_ds_reg(ds, L4DM_RO, addr);
  
  if (rc)
  {
    LOG("error attaching ds ro!");
    STKOUT
    return rc;
  }
  else
    LOG("ds attached again ro");

  LOG("*pptib=%x", *pptib);
  LOG("*pppib=%x", *pppib);
  STKOUT
  return 0; /* NO_ERROR */
}


APIRET CDECL
KalScanEnv(PSZ pszName,
           PPSZ ppszValue)
{
  CORBA_Environment env = dice_default_environment;
  APIRET rc;

  STKIN
  LOG("pszName=%s", pszName);
  rc = os2server_dos_ScanEnv_call(&os2srv, pszName, ppszValue, &env);
  LOG("*ppszValue=%s", *ppszValue);
  STKOUT
  return rc; /* NO_ERROR */
}

APIRET CDECL
KalSetMaxFH(ULONG cFH)
{
  CurMaxFH = cFH;
  LOG("cFH=%u", cFH);

  return 0; /* NO_ERROR */
}

APIRET CDECL
KalSetRelMaxFH(PLONG pcbReqCount, PULONG pcbCurMaxFH)
{
  CurMaxFH += *pcbReqCount;
  *pcbCurMaxFH = CurMaxFH;
  LOG("*pcbReqCount=%d", *pcbReqCount);
  LOG("CurMaxFH=%u", CurMaxFH);

  return 0; /* NO_ERROR */
}

APIRET CDECL
KalSleep(ULONG ms)
{
  STKIN
  LOG("ms=%u", ms);
  l4_sleep(ms);
  STKOUT

  return 0; /* NO_ERROR */
}

APIRET CDECL
KalDupHandle(HFILE hFile, HFILE *phFile2)
{
  CORBA_Environment env = dice_default_environment;
  APIRET rc;

  STKIN
  LOG("hFile=%x", hFile);
  rc = os2server_dos_DupHandle_call(&os2srv, hFile, phFile2, &env);
  LOG("*phFile2=%x", *phFile2);
  STKOUT
  return rc; /* NO_ERROR */
}

APIRET CDECL
KalDelete(PSZ pszFileName)
{
  CORBA_Environment env = dice_default_environment;
  APIRET rc;

  STKIN
  LOG("pszFileName=%s", pszFileName);
  rc = os2fs_dos_Delete_call (&fs, pszFileName, &env);
  STKOUT
  return rc; /* NO_ERROR */
}

APIRET CDECL
KalForceDelete(PSZ pszFileName)
{
  CORBA_Environment env = dice_default_environment;
  APIRET rc;

  STKIN
  LOG("pszFileName=%s", pszFileName);
  rc = os2fs_dos_ForceDelete_call (&fs, pszFileName, &env);
  STKOUT
  return rc; /* NO_ERROR */
}

APIRET CDECL
KalDeleteDir(PSZ pszDirName)
{
  CORBA_Environment env = dice_default_environment;
  APIRET rc;

  STKIN
  LOG("pszDirName=%s", pszDirName);
  rc = os2fs_dos_DeleteDir_call (&fs, pszDirName, &env);
  STKOUT
  return rc; /* NO_ERROR */
}

APIRET CDECL
KalCreateDir(PSZ pszDirName, PEAOP2 peaop2)
{
  CORBA_Environment env = dice_default_environment;
  APIRET rc;

  STKIN
  LOG("pszDirName=%s", pszDirName);
  LOG("peaop2=%x", peaop2);
  rc = os2fs_dos_CreateDir_call (&fs, pszDirName, peaop2, &env);
  STKOUT
  return rc; /* NO_ERROR */
}
