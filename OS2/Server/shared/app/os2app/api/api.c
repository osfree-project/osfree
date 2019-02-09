/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/types.h>
#include <os3/cpi.h>
#include <os3/kal.h>

/* libc includes */
#include <stdlib.h>

/* local includes */
#include "api.h"

extern unsigned long long shared_memory_area;
extern char pszLoadError[260];
extern ULONG rcCode;

extern l4_os3_cap_idx_t execsrv;

extern vmdata_t *areas_list;

void AppNotify(void)
{
  l4_os3_dataspace_t ds;
  app_data_t data;
  APIRET rc;

  while (! (rc = CPClientAppGetData(&data)) )
  {
    switch (data.opcode)
    {
      case OPCODE_ADD_AREA:
        AppAddArea(data.u.aa.addr,
                   data.u.aa.size,
                   data.u.aa.flags);
        break;

      case OPCODE_ATTACH_DATASPACE:
        ds = data.u.ad.ds;
        AppAttachDataspace(data.u.ad.addr,
                           ds,
                           data.u.ad.rights);
        break;

      case OPCODE_RELEASE_DATASPACE:
        ds = data.u.rd.ds;
        AppReleaseDataspace(ds);
    }
  }
}

long AppAddArea(void *addr,
                unsigned long size,
                unsigned long flags)
{
  vmdata_t *ptr;

  ptr = malloc(sizeof(vmdata_t));

  if (! ptr)
    return ERROR_INVALID_ADDRESS;

  ptr->is_shared = 1;
  ptr->owner = 0;
  //ptr->owner.thread = L4_INVALID_ID;
  ptr->area = shared_memory_area;
  ptr->rights = flags;
  ptr->addr = addr;
  ptr->size = size;
  *(ptr->name) = '\0';
  if (areas_list) (areas_list)->prev = ptr;
  ptr->next = areas_list;
  ptr->prev = 0;
  areas_list = ptr;
  return 0;
}

long AppAttachDataspace(void *addr,
                        const l4_os3_dataspace_t ds,
                        unsigned long rights)
{
  unsigned long long area = shared_memory_area;
  return attach_ds_area(ds, area, rights, addr);
}

long AppReleaseDataspace(const l4_os3_dataspace_t ds)
{
  return DataspaceFree(ds);
}
