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

slist_t *
lastelem (slist_t *e)
{
  slist_t *p, *q = 0;
  for (p = e; p; p = p->next) q = p;
  return q;
}
