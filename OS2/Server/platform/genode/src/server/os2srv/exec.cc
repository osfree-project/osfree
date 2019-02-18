#include <os3/thread.h>
#include <os3/processmgr.h>

#include <stdio.h>

extern "C" int
LoaderExec(char *cmd, char *params, char *vc, l4_os3_task_t *taskid)
{
    return 0;
}

extern "C" int
LoaderExecOS2(char *pName, int consoleno, struct t_os2process *proc)
{
    l4_os3_task_t taskid;
    char str[16];
    int rc;

    sprintf(str, "/dev/vc%d", consoleno);
    rc = LoaderExec((char *)"os2app", pName, str, &taskid);
    proc->task = taskid;
    return rc;
}
