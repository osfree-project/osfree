#define INCL_DOSMISC       /* DOS Miscellaneous values */

#define INCL_DOSERRORS     /* DOS Error values         */

#include <os2.h>

#include <stdio.h>



int main(VOID)  {



  ULONG   aulSysInfo[QSV_MAX] = {0};       /* System Information Data Buffer */

  APIRET  rc                  = NO_ERROR;  /* Return code                    */



  rc = DosQuerySysInfo(1L,                 /* Request all available system   */

                       QSV_MAX,            /* information                    */

                       (PVOID)aulSysInfo,

                       sizeof(ULONG)*QSV_MAX);



  if (rc != NO_ERROR) {

     printf("DosQuerySysInfo error: return code = %u\n", rc);

     return 1;

  } else {

     printf("Maximum length for a path name is %u characters.\n",

             aulSysInfo[QSV_MAX_PATH_LENGTH-1]);  /* Max length of path name */



     printf("Total physical memory is %u bytes.\n",

             aulSysInfo[QSV_TOTPHYSMEM-1]);       /* Total physical memory   */

  } /* endif */



   return NO_ERROR;

}
