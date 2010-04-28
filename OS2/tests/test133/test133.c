 #define INCL_DOSFILEMGR   /* File Manager values */

 #define INCL_DOSERRORS    /* DOS error values    */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {

 UCHAR        uchFileName[80] = "C:\\CONFIG.SYS";  /* File to manipulate    */

 FILESTATUS3  fsts3ConfigInfo = {{0}};       /* Buffer for file information */

 ULONG        ulBufSize     = sizeof(FILESTATUS3);  /* Size of above buffer */

 HFILE        hfConfig      = 0;             /* Handle for Config file      */

 ULONG        ulOpenAction  = 0;             /* Action taken by DosOpen     */

 APIRET       rc            = NO_ERROR;      /* Return code                 */



  rc = DosOpen(uchFileName,                 /* File to open (path and name) */

               &hfConfig,                /* File handle returned         */

               &ulOpenAction,               /* Action taken by DosOpen      */

               0L,0L,        /* Primary allocation and attributes (ignored) */

               OPEN_ACTION_FAIL_IF_NEW |

               OPEN_ACTION_OPEN_IF_EXISTS,  /* Open an existing file only   */

               OPEN_FLAGS_NOINHERIT | OPEN_ACCESS_READONLY |

               OPEN_SHARE_DENYNONE,         /* Read access only             */

               0L);                         /* Extended attributes (ignored)*/



    if (rc != NO_ERROR) {

        printf("DosOpen error: return code = %u\n", rc);

        return 1;

    }



    rc = DosQueryFileInfo(hfConfig,   /* Handle of file                  */

                          FIL_STANDARD,  /* Request standard (Level 1) info */

                          &fsts3ConfigInfo, /* Buffer for file information  */

                          ulBufSize);    /* Size of buffer                  */

    if (rc != NO_ERROR) {

        printf("DosQueryFileInfo error: return code = %u\n", rc);

        return 1;

    }



    rc = DosClose(hfConfig);      /* Close the file  (check RC in real life) */

    printf("%s ---  File size: %u bytes\n",uchFileName, fsts3ConfigInfo.cbFile);

    printf("Last updated: %d/%d/%d; %d:%2.2d\n",

            fsts3ConfigInfo.fdateLastWrite.month,        /* Month            */

            fsts3ConfigInfo.fdateLastWrite.day,          /* Day              */

            (fsts3ConfigInfo.fdateLastWrite.year+1980L), /* Years since 1980 */

            fsts3ConfigInfo.ftimeLastWrite.hours,        /* Hours            */

            fsts3ConfigInfo.ftimeLastWrite.minutes);     /* Minutes          */



  return NO_ERROR;

}
