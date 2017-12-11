/* osFree OS/2 personality internal */
#include <os3/io.h>
#include <os3/modmgr.h>

/* libc includes */
#include <string.h>

/* local includes */
#include "api.h"

long ExcOpen(char *szLoadError,
             unsigned long cbLoadError,
             const char *pszModname,
             unsigned long flags,
             unsigned long *hmod)
{
    char exeflag = flags & OPENFLAG_EXEC;
    return OpenModule(szLoadError, cbLoadError, pszModname, exeflag, hmod);
}

long ExcLoad(unsigned long *hmod,
             char *szLoadError,
             unsigned long cbLoadError,
             os2exec_module_t *s)
{
    unsigned long rc;
    IXFModule *ixf;
    IXFSYSDEP *sysdep;

    rc = LoadModule(szLoadError, cbLoadError, hmod);

    ixf = (IXFModule *)*hmod;
    s->ip = ixf->EntryPoint;

    if (ixf->area == 0)
        s->exeflag = 1;
    else
        s->exeflag = 0;

    sysdep = (IXFSYSDEP *)(ixf->hdlSysDep);
    s->sp = sysdep->stack_high;
    s->sp_limit = sysdep->stack_low;
    s->hmod = *hmod;

    io_log("load_component exited\n");

    return rc;
}

long ExcShare(unsigned long hmod)
{
    /* does nothing on Genode */
    return 0;
}

long ExcGetImp(unsigned long hmod,
               unsigned long *index,
               unsigned long *imp_hmod)
{
    /* Error info from ModLoadModule */
    char chLoadError[CCHMAXPATH];
    unsigned long ind = 0;
    unsigned long hmod2;
    IXFModule *ixf;
    char **mod;
    int rc;

    if (!hmod || !index || !imp_hmod)
      return ERROR_INVALID_PARAMETER;

    ixf = (IXFModule *)hmod;

    if (ixf->cbModules <= *index)
      return ERROR_MOD_NOT_FOUND;

    for (ind = 0, mod = ixf->Modules; ind < ixf->cbModules; ind++, mod++)
    {
      if (ind == *index)
        break;
    }

    /* open a module to get its handle. If it is already
       opened, it just returned module handle, not opens
       it for the next time */
    rc = OpenModule(chLoadError, sizeof(chLoadError), *mod, 0, &hmod2);

    if (rc)
      return ERROR_MOD_NOT_FOUND;
#if 0
    ixf = (IXFModule *)hmod2;

    /* skip fake DL module */
    if (!strcasecmp(ixf->name, "DL"))
      hmod2 = 0;
#endif
    *imp_hmod = hmod2;
    ++ *index;

    return rc;
}

long ExcGetSect(unsigned long hmod,
                unsigned long *index,
                l4exec_section_t *sect)
{
    IXFModule *ixf;
    IXFSYSDEP *sysdep;
    slist_t   *s, *r = NULL;
    unsigned long i;

    ixf = (IXFModule *)hmod;
    sysdep = (IXFSYSDEP *)(ixf->hdlSysDep);
    s = sysdep->seclist;

    if (! s)
    {
      // section list is empty (i.e., forwarder)
      sect = NULL;
      return 1;
    }

    for (i = 0; s && i < *index + 1; i++, s = s->next)
    {
      r = s;
    }

    if (i < *index)
    {
      sect = NULL;
      return 1; // set more real error
    }

    memcpy((char *)sect, (char *)r->section, sizeof(l4exec_section_t));
    ++ *index;

    return 0;
}

long ExcQueryProcAddr(unsigned long hmod,
                      unsigned long ordinal,
                      const char *pszName,
                      void **addr)
{
    return ModQueryProcAddr(hmod, ordinal, pszName, addr);
}

long ExcQueryModHandle(const char *pszModname,
                       unsigned long *hmod)
{
    return ModQueryModuleHandle(pszModname, hmod);
}

long ExcQueryModName(unsigned long hmod,
                     unsigned long cbBuf,
                     char *pszBuf)
{
    return ModQueryModuleName(hmod, cbBuf, pszBuf);
}
