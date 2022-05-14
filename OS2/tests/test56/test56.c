 #define INCL_DOSPROCESS   /* Process and thread values */

 #define INCL_DOSERRORS    /* Error values */

 #include <os2.h>

 #include <stdio.h>

int main(VOID)
{

 uDB_t   DebugBuf   = {0};        /* Debug buffer */

 ULONG   TargetPID  = 0;          /* Process ID of controlled process */

 ULONG   TargetAddr = 0;          /* Address within the controlled process */

 LONG    NewValue   = 0;          /* Value to be substituted */

 APIRET  rc         =  NO_ERROR;  /* Return code */



    DebugBuf.Cmd = DBG_C_WriteMem;  /* Perform WRITE WORD command */



    DebugBuf.Pid = TargetPID;       /* Target process to control */



    DebugBuf.Addr = TargetAddr;     /* Target address for command */



    DebugBuf.Value = NewValue;      /* Value to change in other process */



    rc = DosDebug ( &DebugBuf );

    if (rc != NO_ERROR) {

        printf("DosDebug error: return code = %u\n", rc);

        return 1;

    }
return 0;
}
