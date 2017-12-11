#ifndef __API_API_H__
#define __API_API_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/loader.h>

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

long ExcShare(unsigned long hmod);

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

long ExcQueryModHandle(const char *pszModname,
                       unsigned long *hmod);

long ExcQueryModName(unsigned long hmod,
                     unsigned long cbBuf,
                     char *pszBuf);

#ifdef __cplusplus
  }
#endif

#endif /* __API_API_H__ */
