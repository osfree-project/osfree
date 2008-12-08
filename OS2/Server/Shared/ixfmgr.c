/*! @file ixfmgr.c

    @brief Instalable eXecutable Format Manager. Provides support for
           different executable file formats.

*/

#define INCL_ERRORS
#include <os2.h>

#include <ixfmgr.h>
#include <lx.h>
#include <ne.h>

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
    if (rc==NO_ERROR)
    {
      ixfModule->Load=IXFHandlers[i].Load;
      ixfModule->Fixup=IXFHandlers[i].Fixup;
      break;
    }
  }
  return rc;
}

unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ixfModule)
{
  return ixfModule->Load(addr, size, &ixfModule->FormatStruct);
}

unsigned long IXFFixupModule(IXFModule * ixfModule)
{
  return ixfModule->Fixup(ixfModule->FormatStruct);
}
