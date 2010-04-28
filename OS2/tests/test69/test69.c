 #define INCL_DOSFILEMGR   /* File Manager values */

 #define INCL_DOSERRORS    /* DOS error values    */

 #define INCL_DOSMISC      /* DOS miscellaneous   */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {

 UCHAR        uchFileName[80] = "A:\\NONEXIST.ZQX"; /* File to fail on      */

 HFILE        hfConfig      = 0;             /* Handle for Config file      */

 ULONG        ulOpenAction  = 0,             /* Action taken by DosOpen     */

              ulErrorClass  = 0,             /* Type of error encountered   */

              ulErrorAction = 0,             /* Action warranted by error   */

              ulErrorLoc    = 0;             /* Where error occurred        */

 APIRET       rc            = NO_ERROR,      /* Return code                 */

              DosOpenRc     = NO_ERROR;      /* Return code from DosOpen    */



  rc = DosError(FERR_DISABLEHARDERR); /* Suppress error window.  If this is

                                         omitted, OS/2 will put up an error

                                         window asking the user to intervene */

  if (rc != NO_ERROR) {

     printf("DosError error: return code = %u\n", rc);

     return 1;

  }



  DosOpenRc = DosOpen(uchFileName,          /* File to open (path and name) */

               &hfConfig,                   /* File handle returned         */

               &ulOpenAction,               /* Action taken by DosOpen      */

               0L,0L,        /* Primary allocation and attributes (ignored) */

               OPEN_ACTION_FAIL_IF_NEW |

               OPEN_ACTION_OPEN_IF_EXISTS,  /* Open an existing file only   */

               OPEN_SHARE_DENYNONE   |

               OPEN_ACCESS_READWRITE,       /* Read and write access        */

               0L);                         /* Extended attributes(ignored) */



    if (DosOpenRc == NO_ERROR) {

        printf("DosOpen successful... but it should have failed.\n");

        return 1;

    } else {

      rc = DosErrClass(DosOpenRc,           /* Return code from failing API */

                       &ulErrorClass,       /* Class of error               */

                       &ulErrorAction,      /* Action to take for error     */

                       &ulErrorLoc);        /* Where did the error occur?   */



      if (rc != NO_ERROR) {

          printf("DosErrClass error: return code = %u\n", rc);

          return 1;

      } else {

          printf("rc= %u  ErrorClass= %u  ErrorLoc= %u\n",

                  DosOpenRc, ulErrorClass, ulErrorLoc);

          printf("ErrorAction= %u means: ", ulErrorAction);

          switch (ulErrorAction) {

           case(ERRACT_RETRY):  printf("Retry immediately");    break;

           case(ERRACT_DLYRET): printf("Retry after a delay");  break;

           case(ERRACT_USER):   printf("Incorrect user input"); break;

           case(ERRACT_ABORT):  printf("Terminate in an orderly fashion");

                                                                break;

           case(ERRACT_PANIC):  printf("Terminate NOW");        break;

           case(ERRACT_IGNORE): printf("Ignore this error");    break;

           case(ERRACT_INTRET): printf("Retry after user intervenes");

                                                                break;

           default:             printf("Unknown error action"); break;

          } /* switch */

          printf("\n");

      }

   }  /* DosOpen failure */



  rc = DosError(FERR_ENABLEHARDERR);        /* Re-enable error window */

  if (rc != NO_ERROR) {

     printf("DosError error: return code = %u\n", rc);

     return 1;

  }



  return NO_ERROR;

}
