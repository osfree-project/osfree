/*! @file ixfmgr.c

    @brief Instalable eXecutable Format Manager. Provides support for
           different executable file formats.

*/


#define INCL_ERRORS
//#include <os2.h>

#include <l4/os3/io.h>
#include <l4/os3/cfgparser.h>
#include <l4/os3/ixfmgr.h>
#include <l4/os3/lx.h>
#include <l4/os3/ne.h>
#include <l4/os3/os2errcodes.h>

IXFHandler *IXFHandlers;
//  {
//    {NEIdentify,  NELoad,  NEFixup},
//    {LXIdentify,  LXLoad,  LXFixup}
//  };

//#define maxFormats (sizeof(IXFHandlers)/sizeof(IXFHandler))

unsigned long IXFIdentifyModule(void * addr, unsigned long size, IXFModule * ixfModule)
{
  //int i;
  IXFHandler *p;
  unsigned int rc = 0;

  for(p = IXFHandlers; p; p = p->next)
  {
    rc = p->Identify(addr, size);
    if (rc == 0/*NO_ERROR*/)
    {
      ixfModule->Load = p->Load;
      ixfModule->Fixup = p->Fixup;
      if (options.debugixfmgr) io_log("IXFIdentifyModule: Format of module identified\n");
      break;
    }
  }
  return rc;
}

unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ixfModule)
{
  if (options.debugixfmgr) io_log("IXFLoadModule: Loading module.\n");
  return ixfModule->Load(addr, size, ixfModule);
}

unsigned long IXFFixupModule(IXFModule * ixfModule)
{
  unsigned long rc;
  if (options.debugixfmgr) io_log("IXFFixupModule: Fixing up module\n");
  rc=ixfModule->Fixup(ixfModule->FormatStruct);
  return rc;
}

slist_t *
lastelem (slist_t *e)
{
  slist_t *p, *q = NULL;
  for (p = e; p; p = p->next) q = p;
  return q;
}
