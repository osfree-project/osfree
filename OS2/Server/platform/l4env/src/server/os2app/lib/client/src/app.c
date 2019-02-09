/* os2app client-side API */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/dataspace.h>
#include <os3/cpi.h>
#include <os3/app.h>
#include <os3/io.h>

/* l4env includes */
#include <l4/sys/types.h>

/* os2app RPC */
#include <l4/os2app/os2app-client.h>

void AppClientNotify(l4_os3_thread_t client_id)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = client_id.thread;

    os2app_app_notify_send(&os2app, &env);
}

APIRET AppClientAreaAdd(PID pid,
                        //l4_os3_cap_idx_t client_id,
                        PVOID pAddr,
                        ULONG ulSize,
                        ULONG ulFlags)
{
    //CORBA_Environment env = dice_default_environment;
    //l4_threadid_t os2app = client_id.thread;
    //l4_addr_t addr = (l4_addr_t)pAddr;
    //l4_size_t size = (l4_size_t)ulSize;
    //l4_uint32_t flags = (l4_uint32_t)ulFlags;
    app_data_t data;
    long   ret;

    //ret = os2app_app_AddArea_call(&os2app, addr, size, flags, &env);
    data.pid = pid;
    data.u.aa.addr = pAddr;
    data.u.aa.size = ulSize;
    data.u.aa.flags = ulFlags;
    data.opcode = OPCODE_ADD_AREA;
    ret = CPClientAppAddData(&data);
    return (APIRET)ret;
}

APIRET AppClientDataspaceAttach(PID pid,
                                //l4_os3_cap_idx_t   client_id,
                                PVOID              pAddr,
                                l4_os3_dataspace_t ds,
                                ULONG              ulRights)
{
    //CORBA_Environment env = dice_default_environment;
    //l4_threadid_t os2app = client_id.thread;
    //l4_addr_t addr = (l4_addr_t)pAddr;
    //l4_uint32_t rights = (l4_uint32_t)ulRights;
    app_data_t data;
    long ret;

    //ret = os2app_app_AttachDataspace_call(&os2app, addr, &ds.ds, rights, &env);
    data.pid = pid;
    data.u.ad.addr = pAddr;
    data.u.ad.ds   = ds;
    data.u.ad.rights = ulRights;
    data.opcode = OPCODE_ATTACH_DATASPACE;
    ret = CPClientAppAddData(&data);
    return (APIRET)ret;
}

APIRET AppClientDataspaceRelease(PID pid,
                                 //l4_os3_cap_idx_t   client_id,
                                 l4_os3_dataspace_t ds)
{
    //CORBA_Environment env = dice_default_environment;
    //l4_threadid_t os2app = client_id.thread;
    app_data_t data;
    long ret;

    //ret = os2app_app_ReleaseDataspace_call(&os2app, &ds.ds, &env);
    data.pid = pid;
    data.u.rd.ds = ds;
    data.opcode = OPCODE_RELEASE_DATASPACE;
    ret = CPClientAppAddData(&data);
    return (APIRET)ret;
}
