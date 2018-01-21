/*
 *
 *
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/dataspace.h>
#include <os3/thread.h>
#include <os3/types.h>

/* servers RPC includes */
#include "os2exec-server.h"

/* local includes */
#include "api.h"

long DICE_CV
os2exec_open_component (CORBA_Object _dice_corba_obj,
                        const char* fname /* in */,
                        const l4dm_dataspace_t *img_ds /* in */,
                        unsigned long flags /* in */,
                        char **chLoadError /* in, out */,
                        unsigned long *pcbLoadError /* out */,
                        unsigned long *phmod /* out */,
                        CORBA_Server_Environment *_dice_corba_env)
{
  return ExcOpen(*chLoadError, *pcbLoadError, fname, flags, phmod);
}


long DICE_CV
os2exec_load_component (CORBA_Object _dice_corba_obj,
                        unsigned long hmod /* in */,
                        char **chLoadError /* in, out */,
                        unsigned long *pcbLoadError /* out */,
                        os2exec_module_t *s /* out */,
                        CORBA_Server_Environment *_dice_corba_env)
{
  return ExcLoad(&hmod, *chLoadError, *pcbLoadError, s);
}

long DICE_CV
os2exec_share_component (CORBA_Object _dice_corba_obj,
                         unsigned long hmod /* in */,
                         CORBA_Server_Environment *_dice_corba_env)
{
  l4_os3_thread_t thread;

  thread.thread = *_dice_corba_obj;
  return ExcShare(hmod, thread);
}

long DICE_CV
os2exec_getimp_component (CORBA_Object _dice_corba_obj,
                          unsigned long hmod /* in */,
                          unsigned long *index /* in, out */,
                          unsigned long *imp_hmod /* out */,
                          CORBA_Server_Environment *_dice_corba_env)
{
  return ExcGetImp(hmod, index, imp_hmod);
}


long DICE_CV
os2exec_getsect_component (CORBA_Object _dice_corba_obj,
                           unsigned long hmod /* in */,
                           unsigned long *index /* in, out */,
                           l4_os3_section_t *sect /* out */,
                           CORBA_Server_Environment *_dice_corba_env)
{
  return ExcGetSect(hmod, index, sect);
}


long DICE_CV
os2exec_query_procaddr_component (CORBA_Object _dice_corba_obj,
                                  unsigned long hmod /* in */,
                                  unsigned long ordinal /* in */,
                                  const char* modname /* in */,
                                  l4_addr_t *addr /* out */,
                                  CORBA_Server_Environment *_dice_corba_env)
{
  return ExcQueryProcAddr(hmod, ordinal, modname, (void **)addr);
}


long DICE_CV
os2exec_query_modhandle_component (CORBA_Object _dice_corba_obj,
                                   const char* pszModname /* in */,
                                   unsigned long *phmod /* out */,
                                   CORBA_Server_Environment *_dice_corba_env)
{
  return ExcQueryModuleHandle(pszModname, phmod);
}


long DICE_CV
os2exec_query_modname_component (CORBA_Object _dice_corba_obj,
                                 unsigned long hmod /* in */,
                                 unsigned long cbBuf /* in */,
                                 char* *pBuf /* out */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
  return ExcQueryModuleName(hmod, cbBuf, *pBuf);
}

long DICE_CV
os2exec_alloc_sharemem_component (CORBA_Object _dice_corba_obj,
                                  l4_uint32_t size /* in */,
                                  const char *name /* in */,
                                  unsigned long rights /* in */,
                                  l4_addr_t *addr /* out */,
                                  unsigned long long *area /* out */,
                                  CORBA_Server_Environment *_dice_corba_env)
{
  return ExcAllocSharedMem(size, name, rights, (void **)addr, area);
}

long DICE_CV
os2exec_map_dataspace_component (CORBA_Object _dice_corba_obj,
                                 l4_addr_t   addr /* in */,
                                 l4_uint32_t rights /* in */,
                                 const l4dm_dataspace_t *ds /* in */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
  l4_os3_cap_idx_t temp_ds;
  temp_ds.ds = *ds;

  return ExcMapDataspace((void *)addr, rights, temp_ds);
}

long DICE_CV
os2exec_unmap_dataspace_component (CORBA_Object _dice_corba_obj,
                                   l4_addr_t addr /* in */,
                                   const l4dm_dataspace_t *ds /* in */,
                                   CORBA_Server_Environment *_dice_corba_env)
{
  l4_os3_cap_idx_t temp_ds;
  temp_ds.ds = *ds;

  return ExcUnmapDataspace((void *)addr, temp_ds);
}

long DICE_CV
os2exec_get_dataspace_component (CORBA_Object _dice_corba_obj,
                                 l4_addr_t *addr /* in */,
                                 l4_size_t *size /* in */,
                                 l4dm_dataspace_t *ds /* out */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
  l4_os3_dataspace_t temp_ds;
  l4_os3_thread_t client_id;
  temp_ds.ds = *ds;
  client_id.thread = *_dice_corba_obj;

  return ExcGetDataspace((void **)addr, (unsigned long *)size,
                         &temp_ds, client_id);
}

long DICE_CV
os2exec_get_sharemem_component (CORBA_Object _dice_corba_obj,
                                l4_addr_t pb /* in */,
                                l4_addr_t *addr /* out */,
                                l4_uint32_t *size /* out */,
                                l4_threadid_t *owner /* out */,
                                CORBA_Server_Environment *_dice_corba_env)
{
  l4_os3_thread_t thread;
  thread.thread = *owner;

  return ExcGetSharedMem((void *)pb, (void **)addr,
                         (unsigned long *)size, &thread);
}


long DICE_CV
os2exec_get_namedsharemem_component (CORBA_Object _dice_corba_obj,
                                     const char* name /* in */,
                                     l4_addr_t *addr /* out */,
                                     l4_size_t *size /* out */,
                                     l4_threadid_t *owner /* out */,
                                     CORBA_Server_Environment *_dice_corba_env)
{
  l4_os3_cap_idx_t thread;
  thread.thread = *owner;

  return ExcGetNamedSharedMem(name, (void **)addr,
                              (unsigned long *)size, &thread);
}

/*  increment the refcnt for a sharemem area
 */
long DICE_CV
os2exec_increment_sharemem_refcnt_component (CORBA_Object _dice_corba_obj,
                                    l4_addr_t addr /* in */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  return ExcIncrementSharedMemRefcnt((void *)addr);
}

/*  release the reserved sharemem area
 */
long DICE_CV
os2exec_release_sharemem_component (CORBA_Object _dice_corba_obj,
                                    l4_addr_t addr /* in */,
                                    l4_uint32_t *count /* out */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  return ExcReleaseSharedMem((void *)addr, (unsigned long *)count);
}
