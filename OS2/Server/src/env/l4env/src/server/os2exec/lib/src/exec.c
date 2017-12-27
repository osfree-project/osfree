/* os2exec client-side RPC API */

/* osFree internal */
#include <os3/io.h>
#include <os3/types.h>
#include <os3/exec.h>

/* l4env includes */
#include <l4/names/libnames.h>

/* os2exec RPC includes */
#include <l4/os2exec/os2exec-client.h>

/* os2exec client interface cap */
extern l4_os3_cap_idx_t execsrv;

APIRET ExcClientInit(void)
{
    if (! names_waitfor_name("os2exec", &execsrv, 30000) )
    {
        io_log("os2exec not found on the name server!\n");
        return ERROR_FILE_NOT_FOUND; // @todo more appropriate error code
    }

    return NO_ERROR;
}

APIRET ExcClientOpen(PSZ                pszFileName,
                     ULONG              ulFlags,
                     PBYTE              pbLoadError,
                     PULONG             pcbLoadError,
                     PHMODULE           phmod)
{
    CORBA_Environment env = dice_default_environment;
    l4dm_dataspace_t ds;
    long ret;

    io_log("--- in\n");
    ret = os2exec_open_call(&execsrv, (char *)pszFileName, &ds,
                            ulFlags, (char **)&pbLoadError, pcbLoadError, phmod, &env);
    io_log("--- out\n");
    return (APIRET)ret;
}

APIRET ExcClientLoad(HMODULE          hmod,
                     PBYTE            pbLoadError,
                     PULONG           pcbLoadError,
                     os2exec_module_t *s)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    io_log("+++ in\n");
    //enter_kdebug("dbg");
    ret = os2exec_load_call(&execsrv, hmod, (char **)&pbLoadError,
                            pcbLoadError, s, &env);
    io_log("+++ out\n");
    return (APIRET)ret;
}

APIRET ExcClientShare(HMODULE hmod)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_share_call(&execsrv, hmod, &env);
    return (APIRET)ret;
}
