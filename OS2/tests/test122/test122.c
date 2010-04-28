#define INCL_DOSFILEMGR /* File Manager values */

#define INCL_DOSMISC    /* Miscellaneous values */

#define INCL_DOSERRORS  /* DOS Error values */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {



HFILE   FileHandle       = NULLHANDLE;      /* File handle */

ULONG   ulAction         = 0;               /* Action taken by DosOpen */

UCHAR   uchDataArea[160] = "";              /* Message buffer */

APIRET  rc               = 0;               /* Return code */



rc = DosOpen ("MYMSG.DAT", &FileHandle, &ulAction, 120L, FILE_ARCHIVED,

              OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,

              OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYREADWRITE |

              OPEN_ACCESS_READWRITE, 0L);



strcpy (uchDataArea, "This is a sample message that is going to be written ");

strcat (uchDataArea, "to the message file.  It is longer than 80");

strcat (uchDataArea, " characters, so it should wrap.              ");



rc = DosPutMessage(FileHandle, strlen(uchDataArea), uchDataArea);

if (rc != NO_ERROR) {

   printf("DosPutMessage error: return code = %u\n", rc);

   return 1;

}



rc = DosClose(FileHandle);



return NO_ERROR;

}
