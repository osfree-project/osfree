#define INCL_DOSNLS       /* National Language Support values */

#define INCL_DOSERRORS    /* DOS Error values */

#include <os2.h>

#include <stdio.h>





int main(VOID) {

APIRET rc      = NO_ERROR;



rc = DosSetProcessCp(850);



if (rc != NO_ERROR) {

  printf("DosSetProcessCp error: return code = %u\n", rc);

  return 1;

}



return NO_ERROR;

}
