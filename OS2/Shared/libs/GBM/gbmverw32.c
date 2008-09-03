/*

gbmverw32.c - Shows the library version info of GBM (Generalized Bitmap Module).
                           Windows NT/2000/XP version
						   
Author: Heiko Nitzsche

History
-------
26-Aug-2008: Inital version for Windows
*/

#include <windows.h>

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
  const char * callingConvention; /* calling convention of the function */
  const char * functionName;      /* name of the function */
  FARPROC      functionAddress;   /* the address of the function from the loaded module */
  int baseVersion;       /* base version since the function is available */
};

#define GET_GBM_VERSION(version)  ((float)version/100.0)

#define GBM_MIN_VERSION        100
#define GBM_UPPER_MIN_VERSION  135  /* version with latest API extensions */

/** GBM.DLL exports */
static struct FUNCTION_TABLE_DEF GBM_FUNCTION_TABLE [] =
{
   /* as _System exported functions (old) */
   "__stdcall ", "_gbm_version@0"           , NULL, 100,
   "__stdcall ", "_gbm_init@0"              , NULL, 100,
   "__stdcall ", "_gbm_deinit@0"            , NULL, 100,
   "__stdcall ", "_gbm_io_open@8"           , NULL, 109,
   "__stdcall ", "_gbm_io_create@8"         , NULL, 109,
   "__stdcall ", "_gbm_io_close@4"          , NULL, 109,
   "__stdcall ", "_gbm_io_lseek@12"         , NULL, 109,
   "__stdcall ", "_gbm_io_read@12"          , NULL, 109,
   "__stdcall ", "_gbm_io_write@12"         , NULL, 109,
   "__stdcall ", "_gbm_query_n_filetypes@4" , NULL, 100,
   "__stdcall ", "_gbm_guess_filetype@8"    , NULL, 100,
   "__stdcall ", "_gbm_query_filetype@8"    , NULL, 100,
   "__stdcall ", "_gbm_read_header@20"      , NULL, 100,
   "__stdcall ", "_gbm_read_palette@16"     , NULL, 100,
   "__stdcall ", "_gbm_read_data@16"        , NULL, 100,
   "__stdcall ", "_gbm_write@28"            , NULL, 100,
   "__stdcall ", "_gbm_err@4"               , NULL, 100,
   "__stdcall ", "_gbm_restore_io_setup@0"  , NULL, 135,
   "__stdcall ", "_gbm_read_imgcount@16"    , NULL, 135
};
const int GBM_FUNCTION_TABLE_LENGTH = sizeof(GBM_FUNCTION_TABLE) /
                                      sizeof(GBM_FUNCTION_TABLE[0]);
const int GBM_VERSION_ID = 0;

/* ------------------------------ */
/* ------------------------------ */

/** Load functions from a DLL. */
static HMODULE load_functions(const char                * moduleName,
                              struct FUNCTION_TABLE_DEF * table,
                              const int                   tableLength,
                              char                      * foundModuleName,
                              const int                   foundModuleNameLength)
{
  int      i;
  HMODULE  moduleHandle = NULL;
  DWORD    rc;
  
  /* load the module */
  moduleHandle = LoadLibrary(moduleName);
  if (moduleHandle == NULL)
  {
    return NULL;
  }

  /* get the full path name */
  rc = GetModuleFileName(moduleHandle,
                         foundModuleName,
                         foundModuleNameLength-1);
  if ((rc >= foundModuleNameLength-1) &&
      (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
  {
    FreeLibrary(moduleHandle);
    return NULL;
  }
  foundModuleName[rc] = 0;

  /* get all function addresses */
  for (i = 0; i < tableLength; i++)
  {
    table[i].functionAddress = GetProcAddress(moduleHandle,
                                              table[i].functionName);
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
  if (hModule != NULL)
  {
    FreeLibrary(hModule);
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

/** Show version info about found DLL functions. */
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

  printf("------------------------------------------------\n");
  printf("%-9s %-24s %-7s Since\n", "Exported", "Function name", "Found");
  printf("------------------------------------------------\n");

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
        printf("%-9s %-24s   %-4s  %.2f\n", table[i].callingConvention,
                                            table[i].functionName,
                                            "*",
                                            GET_GBM_VERSION(table[i].baseVersion));
      }
      else
      {
        printf("%-9s %-24s   %-4s  %.2f\n", table[i].callingConvention,
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
        printf("%-9s %-24s   %-4s\n", table[i].callingConvention,
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

int main(int argc, char * argv[])
{
  int retCode = 0;
  int versionGbm = 0;

  /* On Windows the user can provide a fully qualified
       * name of the GBM.DLL which is checked alternatively
       * to the automatic lookup.
       */
  char filename_gbm      [GBMVER_FILENAME_MAX+1] = { 0 };
  char foundModuleNameGbm[GBMVER_FILENAME_MAX+1] = { 0 };

  HMODULE hModuleGbm = NULL;

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
      sprintf(filename_gbm, "%sgbm.dll", argv[1]);
    }
    else
    {
      sprintf(filename_gbm, "%s\\gbm.dll", argv[1]);
    }
  }
  else
  {
    strcpy(filename_gbm, "gbm");
  }

  printf("================================================\n");
  printf("Checking for \"%s\"...\n", filename_gbm);

  /* load GBM.DLL */
  hModuleGbm = load_functions(filename_gbm,
                              GBM_FUNCTION_TABLE,
                              GBM_FUNCTION_TABLE_LENGTH,
                              foundModuleNameGbm,
                              GBMVER_FILENAME_MAX);
  if (hModuleGbm == NULL)
  {
    printf("Not found or unresolved dependencies.\n\n");
    return 1;
  }

  /* get version from GBM.DLL */
  if (GBM_FUNCTION_TABLE[GBM_VERSION_ID].functionAddress == NULL)
  {
    printf("Is not valid.\n");
    unload_functions(hModuleGbm, GBM_FUNCTION_TABLE, GBM_FUNCTION_TABLE_LENGTH);
    hModuleGbm = NULL;
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
    hModuleGbm = NULL;
  }
  else
  {
    printf("This GBM.DLL can be used with all applications\n" \
           "developed for GBM.DLL version >=%.2f.\n\n", GET_GBM_VERSION(GBM_MIN_VERSION));
  }

  unload_functions(hModuleGbm, GBM_FUNCTION_TABLE, GBM_FUNCTION_TABLE_LENGTH);
  hModuleGbm = NULL;

  return retCode;
}


