include <emitlib.h>

FILE * SOMLINK somtopenEmitFileSL (char *file, char *ext)
{
  // Здесь необходимо предварительно добавить или заменить расширение на ext
  return fopen(file, "w");
};

#ifndef SOM_SOMC_NO_BACKCOMPAT
// Under Win32 this is __fastcall
FILE * somtopenEmitFile (char *file, char *ext)
{
  return somtopenEmitFileSL (file, ext);
};
#endif

int SOMLINK somtfcloseSL (FILE *fp)
{
  return fclose(fp);
}

#ifndef SOM_SOMC_NO_BACKCOMPAT
// Under Win32 this is __fastcall
int somtfclose (FILE *fp)
{
  return SOMLINK somtfcloseSL (FILE *fp);
};
#endif
