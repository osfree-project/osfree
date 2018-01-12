#ifndef __API_API_H__
#define __API_API_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/loader.h>
#include <os3/dataspace.h>

#define OPENFLAG_EXEC 1

long ExcOpen(char *szLoadError,
             unsigned long cbLoadError,
             const char *pszModname,
             unsigned long flags,
             unsigned long *hmod);

long ExcLoad(unsigned long *hmod,
             char *szLoadError,
             unsigned long cbLoadError,
             os2exec_module_t *s);

long ExcShare(unsigned long hmod,
              void *client_id);

long ExcGetImp(unsigned long hmod,
               unsigned long *index,
               unsigned long *imp_hmod);

long ExcGetSect(unsigned long hmod,
                unsigned long *index,
                l4exec_section_t *sect);

long ExcQueryProcAddr(unsigned long hmod,
                      unsigned long ordinal,
                      const char *pszName,
                      void **addr);

long ExcQueryModuleHandle(const char *pszModname,
                          unsigned long *hmod);

long ExcQueryModuleName(unsigned long hmod,
                        unsigned long cbBuf,
                        char *pszBuf);

long ExcAllocSharedMem(unsigned long size,
                       const char    *name,
                       unsigned long rights,
                       void          **addr,
                       unsigned long long *area);

long ExcMapDataspace(void               *addr,
                     unsigned long      rights,
                     l4_os3_dataspace_t ds);

long ExcUnmapDataspace(void               *addr,
                       l4_os3_dataspace_t ds);

long ExcGetDataspace(void               **addr,
                     unsigned long      *size,
                     l4_os3_dataspace_t *ds,
                     l4_os3_cap_idx_t   client_id);

long ExcGetSharedMem(void *pb,
                     void **addr,
                     unsigned long *size,
                     l4_os3_cap_idx_t *owner);

long ExcGetNamedSharedMem(const char       *name,
                          void             **addr,
                          unsigned long    *size,
                          l4_os3_cap_idx_t *owner);

long ExcIncrementSharedMemRefcnt(void *addr);

long ExcReleaseSharedMem(void          *addr,
                         unsigned long *count);

int load_ixfs(void);

#ifdef __cplusplus
  }
#endif

#endif /* __API_API_H__ */
