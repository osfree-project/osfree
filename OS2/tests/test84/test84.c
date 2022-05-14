 #define INCL_DOSFILEMGR   /* File Manager values */

 #define INCL_DOSERRORS    /* Error values */

 #include <os2.h>

 #include <stdio.h>

 #include <string.h>

int main(VOID)
{

 UCHAR   uchDataArea[200] = {0};          /* Input and output data area */

 ULONG   ulDataLen        = 0;            /* Input and output data size */

 UCHAR   uchParms[120]    = {0};          /* Input and output for function */

 ULONG   ulParmLen        = 0;            /* Input and output parameter size */

 ULONG   ulFunction       = 0x81DE;       /* Device-specific function */

 HFILE   hfFile           = NULLHANDLE;   /* Handle for file */

 APIRET  rc               = NO_ERROR;     /* Return code */



  strcpy(uchDataArea,"34 22 37");   /* Data to pass to file system */

  ulDataLen = strlen(uchDataArea);  /* Length of input data */



  strcpy(uchParms,"PARM1: 98");     /* Input parameters */

  ulParmLen = strlen(uchParms);     /* Length of input parameters */



  rc = DosFSCtl(uchDataArea,         /* Input/output data area */

                sizeof(uchDataArea), /* Maximum output data size */

                &ulDataLen,          /* Input:  size of input data area */

                                     /* Output: size of data returned   */

                uchParms,            /* Input/Output parameter list */

                sizeof(uchParms),    /* Maximum output parameter size */

                &ulParmLen,          /* Input:  size of parameter list */

                                     /* Output: size of parameters returned */

                ulFunction,          /* Function being requested */

                "MY_FSD",            /* File System Driver (FSD) name */

                hfFile,              /* Handle for file */

                FSCTL_FSDNAME);      /* Indicate FSD name is the route */



  if (rc != NO_ERROR) {

      printf("DosFSCtl error: return code = %u\n", rc);

      return 1;

  }

return 0;
}
