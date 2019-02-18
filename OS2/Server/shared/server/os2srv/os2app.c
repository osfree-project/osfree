/* os2app instances communication */

#include <os3/app.h>
#include <os3/cpi.h>

#include <stdlib.h>
#include <string.h>

APIRET CPAppAddData(app_data_t *data);
APIRET CPAppGetData(PID pid, app_data_t *data);

list_t *list = NULL;

list_t *list_query(int key)
{
    list_t *item;

    for (item = list; item; item = item->next)
    {
        if (item->key == key)
        {
            return item;
        }
    }

    return NULL;
}

int list_add(int key, void *data)
{
    list_t *item;

    item = (list_t *)malloc(sizeof (list_t));

    if (! item)
    {
        return 1;
    }

    memset(item, 0, sizeof(list_t));
    item->next = item->prev = NULL;
    item->key  = key;
    item->data = data;

    if (list)
    {
        item->next = list;
        list->prev = item;
    }

    list = item;

    return 0;
}

void list_del(int key)
{
    list_t *item;

    for (item = list; item; item = item->next)
    {
        if (item->key == key )
        {
            if (item->prev)
            {
                item->prev->next = item->next;
            }

            if (item->next)
            {
                item->next->prev = item->prev;
            }

            if (list == item)
            {
                list = item->next;
            }

            free(item);
            break;
        }
    }
}

APIRET CPAppAddData(app_data_t *data)
{
    int key = data->pid;
    int ret = list_add(key,  data);
    struct t_os2process *proc = PrcGetProc(data->pid);
    l4_os3_thread_t thread;

    if (! proc)
    {
        return ERROR_PROC_NOT_FOUND;
    }

    thread = proc->task;
    AppClientNotify(thread);
    return ret;
}

APIRET CPAppGetData(PID pid, app_data_t *data)
{
    //struct t_os2process *proc = PrcGetProcNative(thread);
    int key = pid;
    list_t *item;

    if ( (item = list_query(key)) )
    {
        memcpy(data, item->data, sizeof(app_data_t));
        list_del(key);
        return NO_ERROR;
    }

    return ERROR_FILE_NOT_FOUND;
}
