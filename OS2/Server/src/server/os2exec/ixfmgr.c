/*! @file ixfmgr.c

    @brief Instalable eXecutable Format Manager. Provides support for
           different executable file formats.

*/

/* OS/2 API includes */
#define INCL_ERRORS
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/io.h>
#include <os3/cfgparser.h>
#include <os3/ixfmgr.h>

/* libc includes */
#include <stdlib.h>

unsigned long ixfCopyModule(IXFModule *dst, IXFModule *src);

extern cfg_opts options;

IXFHandler *IXFHandlers;
//  {
//    {NEIdentify,  NELoad,  NEFixup},
//    {LXIdentify,  LXLoad,  LXFixup}
//  };

//#define maxFormats (sizeof(IXFHandlers)/sizeof(IXFHandler))

unsigned long
IXFIdentifyModule(void *addr, unsigned long size, IXFModule *ixfModule)
{
  IXFHandler *p;
  unsigned int rc = 0;

  for (p = IXFHandlers; p; p = p->next)
  {
    rc = p->Identify(addr, size);

    if (rc == NO_ERROR)
    {
      ixfModule->Load = p->Load;
      ixfModule->Fixup = p->Fixup;

      if (options.debugixfmgr) 
         io_log("IXFIdentifyModule: Format of module identified\n");

      break;
    }
  }

  return rc;
}

unsigned long
IXFLoadModule(void *addr, unsigned long size, IXFModule *ixfModule)
{
  if (options.debugixfmgr) 
     io_log("IXFLoadModule: Loading module.\n");

  return ixfModule->Load(addr, size, ixfModule);
}

unsigned long IXFFixupModule(IXFModule *ixfModule)
{
  unsigned long rc;

  if (options.debugixfmgr)
     io_log("IXFFixupModule: Fixing up module\n");

  rc = ixfModule->Fixup(ixfModule->FormatStruct);

  return rc;
}

unsigned long IXFAllocModule(IXFModule **ixf)
{
    IXFSYSDEP *sysdep;

    if (! ixf)
        return ERROR_INVALID_PARAMETER;

    *ixf = (IXFModule *)malloc(sizeof(IXFModule));

    if (! *ixf)
        return ERROR_NOT_ENOUGH_MEMORY;

    sysdep = (IXFSYSDEP *)malloc(sizeof(IXFSYSDEP));

    if (! sysdep)
        return ERROR_NOT_ENOUGH_MEMORY;

    sysdep->secnum  = 0;
    sysdep->seclist = NULL;

    (*ixf)->hdlSysDep = (unsigned long long)sysdep;

    return NO_ERROR;
}

unsigned long IXFFreeModule(IXFModule *ixf)
{
    if (! ixf || ! ixf->hdlSysDep)
        return ERROR_INVALID_PARAMETER;

    free((void *)ixf->hdlSysDep);
    free(ixf);

    return NO_ERROR;
}

unsigned long IXFCopyModule(IXFModule *dst, IXFModule *src)
{
    return ixfCopyModule(dst, src);
}

/* slist_t *
lastelem (slist_t *e)
{
  slist_t *p, *q = 0;
  for (p = e; p; p = p->next) q = p;
  return q;
} */
