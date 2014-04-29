#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOSDEVIOCTL
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#include <os2.h>

#include "portable.h"

#include "Fat32def.h"


int main(int iArgc, PSZ rgArgv[])
{
HFILE hDisk;
APIRET rc;
ULONG  ulAction;
ULONG  ulDataSize;
ULONG  ulParmSize;
USHORT rgfClean[2];
INT iArg;

   if (iArgc < 2)
      {
      printf("USAGE: F32STAT Drive: options\n");
      printf("Valid options are:\n");
      printf("/FCLEAN - forces volume clean (set volume boot state).\n");
      printf("/CLEAN  - sets the current disk state to clean.\n");
      printf("/DIRTY  - forces volume boot state dirty.\n");
      exit(1);
      }

   strupr(rgArgv[1]);

   rc = DosOpen(rgArgv[1],
      &hDisk,
      &ulAction,                          /* action taken */
      0L,                                 /* new size     */
      0L,                                 /* attributes   */
      OPEN_ACTION_OPEN_IF_EXISTS,         /* open flags   */
      OPEN_ACCESS_READONLY |              /* open mode    */
        OPEN_SHARE_DENYNONE |
        OPEN_FLAGS_DASD,
      NULL);                              /* ea data      */
   if (rc)
      {
      printf("Cannot open disk %s, rc = %d\n",
         rgArgv[1], rc);
      exit(1);
      }

   ulDataSize = sizeof rgfClean;
   rgfClean[0] = FALSE;
   rgfClean[1] = FALSE;

   rc = DosDevIOCtl(hDisk,
      IOCTL_FAT32,
      FAT32_GETVOLCLEAN,
      NULL, 0, NULL,
      (PVOID)rgfClean, ulDataSize, &ulDataSize);
   if (rc)
      {
      printf("FAT_GETVOLCLEAN failed, rc = %d\n", rc);
      exit(1);
      }

   if (rgfClean[0])
      {
      printf("Clean shutdown flag for drive %s was SET at boot.\n", rgArgv[1]);
      printf("The data on the drive is fully accessable.\n");
      printf("Shutdown will update clean shutdown flag.\n");
      }
   else
      {
      printf("Clean shutdown flag for drive %s was NOT set at boot.\n", rgArgv[1]);
      printf("The data on the drive cannot be changed now.\n");
      printf("CHKDSK or SCANDISK needs to be run on the drive.\n");
      printf("Shutdown will NOT set clean shutdown flag.\n");
      }
   printf("\n");

   if (rgfClean[1])
      printf("Current status of drive %s is NOT dirty.\n", rgArgv[1]);
   else
      printf("Current status of drive %s is DIRTY.\n", rgArgv[1]);


   for (iArg = 2; iArg < iArgc ; iArg++)
      {
      if (!stricmp(rgArgv[iArg], "/CLEAN"))
         {
         ulParmSize = sizeof rgfClean;
         rgfClean[0] = TRUE;
         rc = DosDevIOCtl(hDisk,
            IOCTL_FAT32,
            FAT32_MARKVOLCLEAN,
            (PVOID)rgfClean, ulParmSize, &ulParmSize,
            NULL, 0, NULL);
         if (rc)
            printf("Marking volume clean returned %d\n", rc);
         else
            printf("Volume marked clean!\n");
         }

      if (!stricmp(rgArgv[iArg], "/FCLEAN"))
         {
         ulParmSize = sizeof rgfClean;
         rgfClean[0] = TRUE;
         rc = DosDevIOCtl(hDisk,
            IOCTL_FAT32,
            FAT32_FORCEVOLCLEAN,
            (PVOID)rgfClean, ulParmSize, &ulParmSize,
            NULL, 0, NULL);
         if (rc)
            printf("Forcing volume clean returned %d\n", rc);
         else
            printf("Volume forced clean!\n");
         }

      if (!stricmp(rgArgv[iArg], "/DIRTY"))
         {
         ulParmSize = sizeof rgfClean;
         rgfClean[0] = FALSE;
         rc = DosDevIOCtl(hDisk,
            IOCTL_FAT32,
            FAT32_FORCEVOLCLEAN,
            (PVOID)rgfClean, ulParmSize, &ulParmSize,
            NULL, 0, NULL);
         if (rc)
            printf("Marking volume dirty returned %d\n", rc);
         else
            printf("Volume marked dirty!\n");
         }


      }


   DosClose(hDisk);
   return 0;
}
