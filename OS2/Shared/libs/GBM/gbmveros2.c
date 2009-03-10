/*

gbmveros2.c - Shows the library version info of GBM (Generalized Bitmap Module).
                         OS/2 version

Author: Heiko Nitzsche

History
-------
29-Apr-2006: On OS/2 the DLL filename can now also be specified
             to be tested. This saves copying around gbmver.exe.

06-May-2006: Check for all functions rather than just for some.
             Give the user some hints about backward compatibility
             and incompatible versions.

16-Aug-2006: Make extension .DLL lower case so that the dynamic
             lookup also works with LIBPATHSTRICT enabled.

12-Nov-2006: Sync to GBMDLG.DLL 1.31 API changes

27-Jan-2008: Sync to GBMRX.DLL 1.10 API changes

26-Aug-2008: Integrate new GBM types
                          Get rid of generic code (moved to separate file)
*/

#define INCL_DOS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmtool.h"

/* ------------------------------ */

/**
 * Maximum filename length that will be parsed.
 * This does not yet include the \0 character !
 */
#if (defined(_MAX_DRIVE) && defined(_MAX_DIR) && defined(_MAX_FNAME) && defined(_MAX_EXT))
  #define GBMVER_FILENAME_MAX (_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT)
#else
  #define GBMVER_FILENAME_MAX  800
#endif

/* ------------------------------ */

typedef struct
{
   gbm_boolean valid;
   int         version;
} VERSION;

typedef struct FUNCTION_TABLE_DEF
{
  PSZ callingConvention; /* calling convention of the function */
  PSZ functionName;      /* name of the function */
  PFN functionAddress;   /* the address of the function from the loaded module */
  int baseVersion;       /* base version since the function is available */
};

#define GET_GBM_VERSION(version)  ((float)version/100.0)

#define GBM_MIN_VERSION        100
#define GBM_UPPER_MIN_VERSION  135  /* version with latest API extensions */

/** GBM.DLL exports */
static struct FUNCTION_TABLE_DEF GBM_FUNCTION_TABLE [] =
{
   /* as _System exported functions (old) */
   "_System ", "Gbm_version"          , NULL, 100,
   "_System ", "Gbm_init"             , NULL, 100,
   "_System ", "Gbm_deinit"           , NULL, 100,
   "_System ", "Gbm_io_open"          , NULL, 109,
   "_System ", "Gbm_io_create"        , NULL, 109,
   "_System ", "Gbm_io_close"         , NULL, 109,
   "_System ", "Gbm_io_lseek"         , NULL, 109,
   "_System ", "Gbm_io_read"          , NULL, 109,
   "_System ", "Gbm_io_write"         , NULL, 109,
   "_System ", "Gbm_query_n_filetypes", NULL, 100,
   "_System ", "Gbm_guess_filetype"   , NULL, 100,
   "_System ", "Gbm_query_filetype"   , NULL, 100,
   "_System ", "Gbm_read_header"      , NULL, 100,
   "_System ", "Gbm_read_palette"     , NULL, 100,
   "_System ", "Gbm_read_data"        , NULL, 100,
   "_System ", "Gbm_write"            , NULL, 100,
   "_System ", "Gbm_err"              , NULL, 100,

   /* as _Optlink exported functions */
   "_Optlink", "gbm_version"          , NULL, 100,
   "_Optlink", "gbm_init"             , NULL, 100,
   "_Optlink", "gbm_deinit"           , NULL, 100,
   "_Optlink", "gbm_io_setup"         , NULL, 107,
   "_Optlink", "gbm_io_open"          , NULL, 107,
   "_Optlink", "gbm_io_create"        , NULL, 107,
   "_Optlink", "gbm_io_close"         , NULL, 107,
   "_Optlink", "gbm_io_lseek"         , NULL, 107,
   "_Optlink", "gbm_io_read"          , NULL, 107,
   "_Optlink", "gbm_io_write"         , NULL, 107,
   "_Optlink", "gbm_query_n_filetypes", NULL, 100,
   "_Optlink", "gbm_guess_filetype"   , NULL, 100,
   "_Optlink", "gbm_query_filetype"   , NULL, 100,
   "_Optlink", "gbm_read_header"      , NULL, 100,
   "_Optlink", "gbm_read_palette"     , NULL, 100,
   "_Optlink", "gbm_read_data"        , NULL, 100,
   "_Optlink", "gbm_write"            , NULL, 100,
   "_Optlink", "gbm_err"              , NULL, 100,

   /* as _System exported functions (new) */
   "_System ", "gbm_restore_io_setup" , NULL, 135,
   "_System ", "gbm_read_imgcount"    , NULL, 135
};
const int GBM_FUNCTION_TABLE_LENGTH = sizeof(GBM_FUNCTION_TABLE) /
                                      sizeof(GBM_FUNCTION_TABLE[0]);
const int GBM_VERSION_ID = 0;

/* ------------------------------ */

/** GBMDLG.DLL exports */
static struct FUNCTION_TABLE_DEF GBMDLG_FUNCTION_TABLE [] =
{
   "_System", "GbmFileDlg"       , NULL, 100,
   "_System", "GbmDefFileDlgProc", NULL, 131
};
const int GBMDLG_FUNCTION_TABLE_LENGTH = sizeof(GBMDLG_FUNCTION_TABLE) /
                                         sizeof(GBMDLG_FUNCTION_TABLE[0]);

/* ------------------------------ */
/* ------------------------------ */

/** GBMRX.DLL exports */
static struct FUNCTION_TABLE_DEF GBMRX_FUNCTION_TABLE [] =
{
   "_System", "GBM_LOADFUNCS"         , NULL, 100,
   "_System", "GBM_DROPFUNCS"         , NULL, 100,
   "_System", "GBM_VERSION"           , NULL, 100,
   "_System", "GBM_VERSIONREXX"       , NULL, 100,
   "_System", "GBM_TYPES"             , NULL, 100,
   "_System", "GBM_ISBPPSUPPORTED"    , NULL, 100,
   "_System", "GBM_FILETYPE"          , NULL, 100,
   "_System", "GBM_FILEPAGES"         , NULL, 100,
   "_System", "GBM_FILEHEADER"        , NULL, 100,
   "_System", "GBM_FILEPALETTE"       , NULL, 100,
   "_System", "GBM_FILEDATA"          , NULL, 100,
   "_System", "GBM_FILEWRITE"         , NULL, 100,

   "_System", "GBM_SCALEALGORITHMS"   , NULL, 110,
   "_System", "GBM_SCALEISSUPPORTED"  , NULL, 110,
   "_System", "GBM_SCALE"             , NULL, 110,

   "_System", "GBM_REFLECT"           , NULL, 110,
   "_System", "GBM_ROTATE"            , NULL, 110,

   "_System", "GBM_PALETTEDATATO24BPP", NULL, 110
};
const int GBMRX_FUNCTION_TABLE_LENGTH = sizeof(GBMRX_FUNCTION_TABLE) /
                                        sizeof(GBMRX_FUNCTION_TABLE[0]);

/* ------------------------------ */

/** GBMDLGRX.DLL exports */
static struct FUNCTION_TABLE_DEF GBMDLGRX_FUNCTION_TABLE [] =
{
   "_System", "GBMDLG_LOADFUNCS"     , NULL, 100,
   "_System", "GBMDLG_DROPFUNCS"     , NULL, 100,
   "_System", "GBMDLG_OPENFILEDLG"   , NULL, 100,
   "_System", "GBMDLG_SAVEASFILEDLG" , NULL, 100,
   "_System", "GBMDLG_VERSIONREXX"   , NULL, 101
};
const int GBMDLGRX_FUNCTION_TABLE_LENGTH = sizeof(GBMDLGRX_FUNCTION_TABLE) /
                                           sizeof(GBMDLGRX_FUNCTION_TABLE[0]);

/* ------------------------------ */

/** Load functions from a DLL. */
static HMODULE load_functions(PSZ                         moduleName,
                              struct FUNCTION_TABLE_DEF * table,
                              const int                   tableLength,
                              char                      * foundModuleName,
                              const int                   foundModuleNameLength)
{
  int      i;
  HMODULE  moduleHandle                     = NULLHANDLE;
  UCHAR    loadError[GBMVER_FILENAME_MAX+1] = { 0 };
  APIRET   rc                               = 0;

  /* load the module */
  rc = DosLoadModule(loadError,
                     sizeof(loadError)-1,
                     moduleName,
                     &moduleHandle);
  if (rc)
  {
    return NULLHANDLE;
  }

  /* get the full path name */
  rc = DosQueryModuleName(moduleHandle,
                          foundModuleNameLength,
                          foundModuleName);
  if (rc)
  {
    DosFreeModule(moduleHandle);
    return NULLHANDLE;
  }

  /* get all function addresses */
  for (i = 0; i < tableLength; i++)
  {
    rc = DosQueryProcAddr(moduleHandle,
                          0L,
                           table[i].functionName,
                          &table[i].functionAddress);
    if (rc)
    {
      table[i].functionAddress = NULL;
    }
  }

  return moduleHandle;
}

/* ------------------------------ */

/** Load functions from GBM.DLL. */
static void unload_functions(HMODULE                     hModule,
                             struct FUNCTION_TABLE_DEF * table,
                             const int                   tableLength)
{
  int i;

  for (i = 0; i < tableLength; i++)
  {
    table[i].functionAddress = NULL;
  }
  if (hModule != NULLHANDLE)
  {
    DosFreeModule(hModule);
  }
}

/* ------------------------------ */

/** qsort compare function for int */
int version_compare(const void * num1, const void * num2)
{
  const int n1 = ((VERSION *) num1)->version;
  const int n2 = ((VERSION *) num2)->version;

  if (n1 < n2)
  {
     return -1;
  }
  if (n1 > n2)
  {
     return 1;
  }
  return 0;
}

/** Show version info about found DLL functions */
static gbm_boolean show_info(struct FUNCTION_TABLE_DEF * table,
                             const  int                  tableLength,
                             const  char               * filename,
                             const  int                  version)
{
  int i;
  int found = 0;
  int foundMinVersion = 9999;

  VERSION * versionTable = (VERSION *) malloc(tableLength * sizeof(VERSION));

  if (versionTable == NULL)
  {
     printf("Out of memory\n");
     return GBM_FALSE;
  }

  #define MIN(a,b)  (a < b ? a : b)

  printf("-> Found \"%s\"\n", filename);

  printf("-----------------------------------------------\n");
  printf("%-9s %-23s %-7s Since\n", "Exported", "Function name", "Found");
  printf("-----------------------------------------------\n");

  for (i = 0; i < tableLength; i++)
  {
    foundMinVersion = MIN(foundMinVersion, table[i].baseVersion);

    versionTable[i].valid   = GBM_FALSE;
    versionTable[i].version = table[i].baseVersion;

    if (table[i].baseVersion > 0)
    {
      if (table[i].functionAddress != NULL)
      {
        versionTable[i].valid = GBM_TRUE;
        found++;
        printf("%-9s %-23s   %-4s  %.2f\n", table[i].callingConvention,
                                            table[i].functionName,
                                            "*",
                                            GET_GBM_VERSION(table[i].baseVersion));
      }
      else
      {
        printf("%-9s %-23s   %-4s  %.2f\n", table[i].callingConvention,
                                            table[i].functionName,
                                            " ",
                                            GET_GBM_VERSION(table[i].baseVersion));
      }
    }
    else
    {
      if (table[i].functionAddress != NULL)
      {
        versionTable[i].valid = GBM_TRUE;
        found++;
        printf("%-9s %-23s   %-4s\n", table[i].callingConvention,
                                      table[i].functionName, "*");
      }
      else
      {
        printf("%-9s %-23s   %-4s\n", table[i].callingConvention,
                                      table[i].functionName, " ");
      }
    }
  }

  printf("\n");

  if (table == GBM_FUNCTION_TABLE)
  {
    if (GBM_MIN_VERSION > version)
    {
      printf("Unsupported version. Minimum version is %f.\n\n", GET_GBM_VERSION(GBM_MIN_VERSION));
    }
    else
    {
      int minCompatVersion   = foundMinVersion;
      int minIncompatVersion = 999;

      qsort(versionTable, tableLength, sizeof(VERSION), version_compare);

      for (i = 0; i < tableLength; i++)
      {
        if (versionTable[i].valid)
        {
          if (minCompatVersion < versionTable[i].version)
          {
            minCompatVersion = versionTable[i].version;
          }
        }
        else
        {
          if (minIncompatVersion > versionTable[i].version)
          {
            minIncompatVersion = versionTable[i].version;
          }
        }
      }

      if ((found != tableLength) && (found > 1))
      {
        if (foundMinVersion == minCompatVersion)
        {
          printf("This GBM.DLL is compatible to applications\n" \
                 "developed for GBM.DLL version %.2f only.\n\n", GET_GBM_VERSION(minCompatVersion));
        }
        else
        {
          printf("This GBM.DLL is backward compatible to applications\n" \
                 "developed for GBM.DLL versions >=%.2f and <%.2f.\n\n", GET_GBM_VERSION(foundMinVersion),
                                                                         GET_GBM_VERSION(minIncompatVersion));
        }
      }
    }
  }

  free(versionTable);

  return ((found == tableLength) && (version >= foundMinVersion));
}

/********************************************/
/********************************************/

int main(int argc, char * argv[])
{
  int retCode = 0;
  int versionGbm = 0;

  /* On OS/2 the user can provide a fully qualified
       * name of the GBM.DLL which is checked alternatively
       * to the automatic lookup.
       */
  char filename_gbm     [GBMVER_FILENAME_MAX+1];
  char filename_gbmdlg  [GBMVER_FILENAME_MAX+1];
  char filename_gbmrx   [GBMVER_FILENAME_MAX+1];
  char filename_gbmdlgrx[GBMVER_FILENAME_MAX+1];

  char foundModuleNameGbm     [GBMVER_FILENAME_MAX+1] = "";
  char foundModuleNameGbmdlg  [GBMVER_FILENAME_MAX+1] = "";
  char foundModuleNameGbmrx   [GBMVER_FILENAME_MAX+1] = "";
  char foundModuleNameGbmdlgrx[GBMVER_FILENAME_MAX+1] = "";

  HMODULE hModuleGbm      = NULLHANDLE;
  HMODULE hModuleGbmdlg   = NULLHANDLE;
  HMODULE hModuleGbmrx    = NULLHANDLE;
  HMODULE hModuleGbmdlgrx = NULLHANDLE;

  if (argc == 2)
  {
    /* check if the user specified a trailing \ */
    if (strlen(argv[1]) > GBMVER_FILENAME_MAX)
    {
       printf("Provided pathname is too long.\n");
       return 1;
    }

    if (argv[1][strlen(argv[1])-1] == '\\')
    {
      sprintf(filename_gbm     , "%sgbm.dll"     , argv[1]);
      sprintf(filename_gbmdlg  , "%sgbmdlg.dll"  , argv[1]);
      sprintf(filename_gbmrx   , "%sgbmrx.dll"   , argv[1]);
      sprintf(filename_gbmdlgrx, "%sgbmdlgrx.dll", argv[1]);
    }
    else
    {
      sprintf(filename_gbm     , "%s\\gbm.dll"     , argv[1]);
      sprintf(filename_gbmdlg  , "%s\\gbmdlg.dll"  , argv[1]);
      sprintf(filename_gbmrx   , "%s\\gbmrx.dll"   , argv[1]);
      sprintf(filename_gbmdlgrx, "%s\\gbmdlgrx.dll", argv[1]);
    }
  }
  else
  {
    strcpy(filename_gbm     , "gbm");
    strcpy(filename_gbmdlg  , "gbmdlg");
    strcpy(filename_gbmrx   , "gbmrx");
    strcpy(filename_gbmdlgrx, "gbmdlgrx");
  }

  printf("===============================================\n");
  printf("Checking for \"%s\"...\n", filename_gbm);

  /* load GBM.DLL */
  hModuleGbm = load_functions(filename_gbm,
                              GBM_FUNCTION_TABLE,
                              GBM_FUNCTION_TABLE_LENGTH,
                              foundModuleNameGbm,
                              GBMVER_FILENAME_MAX);
  if (hModuleGbm == NULLHANDLE)
  {
    printf("Not found or unresolved dependencies.\n\n");
    return 1;
  }

  /* get version from GBM.DLL */
  if (GBM_FUNCTION_TABLE[GBM_VERSION_ID].functionAddress == NULL)
  {
    printf("Is not valid.\n");
    unload_functions(hModuleGbm, GBM_FUNCTION_TABLE, GBM_FUNCTION_TABLE_LENGTH);
    hModuleGbm    = NULLHANDLE;
    return 1;
  }

  versionGbm = GBM_FUNCTION_TABLE[GBM_VERSION_ID].functionAddress();
  printf("-> Found version %.2f\n", GET_GBM_VERSION(versionGbm));

  /* show GBM.DLL info */
  if (! show_info(GBM_FUNCTION_TABLE,
                  GBM_FUNCTION_TABLE_LENGTH,
                  foundModuleNameGbm,
                  versionGbm))
  {
    retCode = 1;
    unload_functions(hModuleGbm, GBM_FUNCTION_TABLE, GBM_FUNCTION_TABLE_LENGTH);
    hModuleGbm = NULLHANDLE;
  }
  else
  {
    printf("This GBM.DLL can be used with all applications\n" \
           "developed for GBM.DLL version >=%.2f.\n\n", GET_GBM_VERSION(GBM_MIN_VERSION));
  }

  printf("===============================================\n");
  printf("Checking for \"%s\"...\n", filename_gbmdlg);

  /* load GBMDLG.DLL */
  hModuleGbmdlg = load_functions(filename_gbmdlg,
                                 GBMDLG_FUNCTION_TABLE,
                                 GBMDLG_FUNCTION_TABLE_LENGTH,
                                 foundModuleNameGbmdlg,
                                 GBMVER_FILENAME_MAX);
  if (hModuleGbmdlg == NULLHANDLE)
  {
    printf("Not found or unresolved dependencies.\n");
    unload_functions(hModuleGbm, GBM_FUNCTION_TABLE, GBM_FUNCTION_TABLE_LENGTH);
    hModuleGbm = NULLHANDLE;
    return 1;
  }

  /* show GBMDLG.DLL info */
  if (! show_info(GBMDLG_FUNCTION_TABLE,
                  GBMDLG_FUNCTION_TABLE_LENGTH,
                  foundModuleNameGbmdlg,
                  0 /* no version info */))
  {
    retCode = 1;
    unload_functions(hModuleGbmdlg, GBMDLG_FUNCTION_TABLE, GBMDLG_FUNCTION_TABLE_LENGTH);
    unload_functions(hModuleGbm   , GBM_FUNCTION_TABLE   , GBM_FUNCTION_TABLE_LENGTH);
    hModuleGbmdlg = NULLHANDLE;
    hModuleGbm    = NULLHANDLE;
  }

  printf("===============================================\n");
  printf("Checking for \"%s\"...\n", filename_gbmrx);

  /* load GBMRX.DLL */
  hModuleGbmrx = load_functions(filename_gbmrx,
                                GBMRX_FUNCTION_TABLE,
                                GBMRX_FUNCTION_TABLE_LENGTH,
                                foundModuleNameGbmrx,
                                GBMVER_FILENAME_MAX);
  if (hModuleGbmrx == NULLHANDLE)
  {
    printf("Not found or unresolved dependencies.\n");
    unload_functions(hModuleGbmrx , GBMRX_FUNCTION_TABLE , GBMRX_FUNCTION_TABLE_LENGTH);
    unload_functions(hModuleGbmdlg, GBMDLG_FUNCTION_TABLE, GBMDLG_FUNCTION_TABLE_LENGTH);
    unload_functions(hModuleGbm   , GBM_FUNCTION_TABLE   , GBM_FUNCTION_TABLE_LENGTH);
    hModuleGbmrx  = NULLHANDLE;
    hModuleGbmdlg = NULLHANDLE;
    hModuleGbm    = NULLHANDLE;
    return 1;
  }

  /* show GBMRX.DLL info */
  if (! show_info(GBMRX_FUNCTION_TABLE,
                  GBMRX_FUNCTION_TABLE_LENGTH,
                  foundModuleNameGbmrx,
                  101 /* no public version info */))
  {
    retCode = 1;
    unload_functions(hModuleGbmrx , GBMRX_FUNCTION_TABLE , GBMRX_FUNCTION_TABLE_LENGTH);
    unload_functions(hModuleGbmdlg, GBMDLG_FUNCTION_TABLE, GBMDLG_FUNCTION_TABLE_LENGTH);
    unload_functions(hModuleGbm   , GBM_FUNCTION_TABLE   , GBM_FUNCTION_TABLE_LENGTH);
    hModuleGbmrx  = NULLHANDLE;
    hModuleGbmdlg = NULLHANDLE;
    hModuleGbm    = NULLHANDLE;
  }

  printf("===============================================\n");
  printf("Checking for \"%s\"...\n", filename_gbmdlgrx);

  /* load GBMDLGRX.DLL */
  hModuleGbmdlgrx = load_functions(filename_gbmdlgrx,
                                   GBMDLGRX_FUNCTION_TABLE,
                                   GBMDLGRX_FUNCTION_TABLE_LENGTH,
                                   foundModuleNameGbmdlgrx,
                                   GBMVER_FILENAME_MAX);
  if (hModuleGbmdlgrx == NULLHANDLE)
  {
    printf("Not found or unresolved dependencies.\n");
    unload_functions(hModuleGbmdlgrx, GBMDLGRX_FUNCTION_TABLE, GBMDLGRX_FUNCTION_TABLE_LENGTH);
    unload_functions(hModuleGbmrx   , GBMRX_FUNCTION_TABLE , GBMRX_FUNCTION_TABLE_LENGTH);
    unload_functions(hModuleGbmdlg  , GBMDLG_FUNCTION_TABLE, GBMDLG_FUNCTION_TABLE_LENGTH);
    unload_functions(hModuleGbm     , GBM_FUNCTION_TABLE   , GBM_FUNCTION_TABLE_LENGTH);
    hModuleGbmdlgrx = NULLHANDLE;
    hModuleGbmrx    = NULLHANDLE;
    hModuleGbmdlg   = NULLHANDLE;
    hModuleGbm      = NULLHANDLE;
    return 1;
  }

  /* show GBMDLGRX.DLL info */
  if (! show_info(GBMDLGRX_FUNCTION_TABLE,
                  GBMDLGRX_FUNCTION_TABLE_LENGTH,
                  foundModuleNameGbmdlgrx,
                  101 /* no public version info */))
  {
    retCode = 1;
  }

  unload_functions(hModuleGbmdlgrx, GBMDLGRX_FUNCTION_TABLE, GBMDLGRX_FUNCTION_TABLE_LENGTH);
  unload_functions(hModuleGbmrx   , GBMRX_FUNCTION_TABLE , GBMRX_FUNCTION_TABLE_LENGTH);
  unload_functions(hModuleGbmdlg  , GBMDLG_FUNCTION_TABLE, GBMDLG_FUNCTION_TABLE_LENGTH);
  unload_functions(hModuleGbm     , GBM_FUNCTION_TABLE   , GBM_FUNCTION_TABLE_LENGTH);
  hModuleGbmdlgrx = NULLHANDLE;
  hModuleGbmrx    = NULLHANDLE;
  hModuleGbmdlg   = NULLHANDLE;
  hModuleGbm      = NULLHANDLE;

  return retCode;
}

