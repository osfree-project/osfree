
#include <dir.h>
#include <string.h>

#include "io95\io95.h"
#include "io95\find95.h"
#include "lfnapi.h"

static int
IsLFNSupported (char *filename)
{
  if (filename && filename[0] && (filename[1] == ':'))
    return CheckDriveOnLFN (filename[0]);
  else
    return CheckDriveOnLFN (getdisk () + 'A');
}

int LFNConvertToSFN(char* file)
{
    static char buffer[67];

    if (IsLFNSupported(file))
    {
        if (lfn2sfn95(file, buffer) == 0)
        {
            strcpy(file, buffer);
            return 1;
        }
        else
            return 0;
    }

    return 1;
}

static int lfnSupported;
static struct ffblk95 ff95;
static struct ffblk ff;

int LFNFirstFile(char* wildcard, char* file, char* longfile)
{
    int retVal;

    if (IsLFNSupported(wildcard))
    {  
        lfnSupported = 1;

        retVal = findfirst95(wildcard, &ff95, 0);

        if (retVal == 0)
        {
            strcpy(file, ff95.ff_95.ff_shortname);
            strcpy(longfile, ff95.ff_95.ff_longname);
        }
    
        return retVal;
    }
    else
    {
        lfnSupported = 0;

        retVal = findfirst(wildcard, &ff, 0);

        if (retVal == 0)
        {
            strcpy(file, ff.ff_name);
            strcpy(longfile, ff.ff_name);
        }
    
        return retVal;
    }
}

int LFNNextFile(char* file, char* longfile)
{
    int retVal;

    if (lfnSupported)
    {  
        retVal = findnext95(&ff95);

        if (retVal == 0)
        {
            strcpy(file, ff95.ff_95.ff_shortname);
            strcpy(longfile, ff95.ff_95.ff_longname);
        }
    
        return retVal;
    }
    else
    {
        retVal = findnext(&ff);

        if (retVal == 0)
        {
            strcpy(file, ff.ff_name);
            strcpy(longfile, ff.ff_name);
        }
    
        return retVal;
    }
}

void LFNFindStop()
{
   if (lfnSupported)
   {
        findstop95(&ff95);
   }
}

