
#include <sm.h>

#include <scentry.xh>
#include <scclass.xh>
#include <emitlib.h>

SOMTEntryC * SOMLINK somtGetObjectWrapper(Entry * ep)
{
  SOMTClassEntryC * oCls;
  oCls=new SOMTClassEntryC();
  
  oCls->somtSetEntryStruct(ep);

  return oCls;
}

FILE * SOMLINK somtopenEmitFile (char *file, char *ext)
{
  // Здесь необходимо предварительно добавить или заменить расширение на ext
  return fopen(file, "w");
};

int SOMLINK somtfclose (FILE *fp)
{
  return fclose(fp);
}

