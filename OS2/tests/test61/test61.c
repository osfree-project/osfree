 #define INCL_DOSDEVICES   /* Device values */

 #define INCL_DOSERRORS    /* Error values */

 #include <os2.h>

 #include <stdio.h>

 #include <string.h>



 HFILE   DevHandle        = NULLHANDLE;   /* Handle for device */

 ULONG   ulCategory       = 0x83;         /* Device category */

 ULONG   ulFunction       = 0x1D;         /* Device-specific function */

 UCHAR   uchParms[120]    = {0};          /* Input and output for function */

 ULONG   ulParmLen        = 0;            /* Input and output parameter size */

 UCHAR   uchDataArea[200] = {0};          /* Input and output data area */

 ULONG   ulDataLen        = 0;            /* Input and output data size */

 APIRET  rc               = NO_ERROR;     /* Return code */



  strcpy(uchParms,"/X /Y /Z");    /* Input parameters */

  ulParmLen = strlen(uchParms);   /* Length of input parameters */



  strcpy(uchDataArea,"DF=123;NP=BCR;UN=1993;MAX=328");  /* Input data */

  ulDataLen = strlen(uchDataArea);                      /* Length of data  */



  rc = DosDevIOCtl(DevHandle,           /* Handle to device */

                   ulCategory,          /* Category of request */

                   ulFunction,          /* Function being requested */

                   uchParms,            /* Input/Output parameter list */

                   sizeof(uchParms),    /* Maximum output parameter size */

                   &ulParmLen,          /* Input:  size of parameter list */

                                        /* Output: size of parameters returned */

                   uchDataArea,         /* Input/Output data area */

                   sizeof(uchDataArea), /* Maximum output data size */

                   &ulDataLen);         /* Input:  size of input data area */

                                        /* Output: size of data returned   */



  if (rc != NO_ERROR) {

      printf("DosDevIOCtl error: return code = %u\n", rc);

      return 1;

  }
