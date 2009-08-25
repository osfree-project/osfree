/*! @file ixfmgr.c

    @brief Instalable eXecutable Format Manager. Provides support for
           different executable file formats.

*/

#define INCL_ERRORS
//#include <os2.h>

#include <io.h>
#include <cfgparser.h>
#include <ixfmgr.h>
#include <lx.h>
#include <ne.h>
#include <os2errcodes.h>

typedef
  struct
  {
    fnIdentify * Identify;
    fnLoad * Load;
    fnFixup * Fixup;
  } IXFHandler;

IXFHandler IXFHandlers[2] =
  {
    NEIdentify,  NELoad,  NEFixup,
    LXIdentify,  LXLoad,  LXFixup
  };

#define maxFormats (sizeof(IXFHandlers)/sizeof(IXFHandler))

unsigned long IXFIdentifyModule(void * addr, unsigned long size, IXFModule * ixfModule)
{
  int i;
  unsigned int rc;

  for(i=0; i<maxFormats; i++)
  {
    rc=IXFHandlers[i].Identify(addr, size);
    if (rc==0/*NO_ERROR*/)
    {
      ixfModule->Load=IXFHandlers[i].Load;
      ixfModule->Fixup=IXFHandlers[i].Fixup;
      if (options.debugixfmgr) io_printf("IXFIdentifyModule: Format of module identified\n");
      break;
    }
  }
  return rc;
}

unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ixfModule)
{
  if (options.debugixfmgr) io_printf("IXFLoadModule: Loading module.\n");
  return ixfModule->Load(addr, size, ixfModule);
}

unsigned long IXFFixupModule(IXFModule * ixfModule)
{
  unsigned long rc;
  if (options.debugixfmgr) io_printf("IXFFixupModule: Fixing up module\n");
  rc=ixfModule->Fixup(ixfModule->FormatStruct);
  return rc;
}
