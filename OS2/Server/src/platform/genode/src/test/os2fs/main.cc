/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/fs.h>

/* Genode includes */
#include <libc/component.h>
#include <base/heap.h>

/* local includes */
#include "genode_env.h"

void Libc::Component::construct(Libc::Env &env)
{
    const char *fn = "c:\\config.sys";
    char Buf[1024];
    APIRET rc;
    HFILE hf;
    ULONG ulAction;
    ULONGLONG fSize = 0;
    FILESTATUS3L Info;

    Genode::Heap heap(env.ram(), env.rm());

    init_genode_env(env, heap);

    FSClientInit();

    rc = FSClientOpenL((PSZ)fn,
                       &hf,
                       &ulAction,
                       0,
                       0,
                       OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                       OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                       OPEN_ACCESS_READONLY,
                       NULL);

    io_log("DosOpenL rc=%u\n", rc);

    if (rc)
    {
        return;
    }

    fSize = sizeof(Info);

    rc = FSClientQueryFileInfo(hf,
                               FIL_STANDARDL,
                               (char *)&Info,
                               (PULONG)&fSize);

    io_log("DosQueryFileInfo rc=%u\n", rc);

    if (rc)
    {
        return;
    }

    fSize = Info.cbFile;

    io_log("fSize=%u\n", fSize);

    rc = FSClientRead(hf, Buf, (ULONG)fSize, (PULONG)&fSize);

    io_log("read %u bytes\n", fSize);
    io_log("%s\n", Buf);

    FSClientClose(hf);

    fSize = sizeof(Info);

    rc = FSClientQueryPathInfo((PSZ)fn,
                               FIL_STANDARDL,
                               (char *)&Info,
                               (PULONG)&fSize);

    io_log("DosQueryPathInfo rc=%u\n", rc);

    if (rc)
    {
        return;
    }

    fSize = Info.cbFile;

    io_log("fSize=%u\n", fSize);

    FSClientDone();

    io_log("success\n");
}
