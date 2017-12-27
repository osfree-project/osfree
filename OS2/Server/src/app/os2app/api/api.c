/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/types.h>
#include <os3/kal.h>

/* l4env includes */
#include <l4/generic_ts/generic_ts.h>

/* libc includes */
#include <stdlib.h>

/* local includes */
#include "api.h"

extern l4_uint32_t shared_memory_area;
extern char pszLoadError[260];
extern ULONG rcCode;

extern l4_os3_cap_idx_t execsrv;

extern vmdata_t *areas_list;

long AppGetLoadError(char *uchLoadError,
                     ULONG *cbLoadError,
                     ULONG *retCode)
{
  strcpy(uchLoadError, pszLoadError);
  *cbLoadError = strlen(uchLoadError) + 1;
  *retCode = rcCode;
}

long AppTerminate(void)
{
  l4ts_exit();
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
  ptr->owner = L4_INVALID_ID;
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
  unsigned long area = shared_memory_area;
  return attach_ds_area(ds, area, rights, addr);
}

long AppReleaseDataspace(const l4_os3_dataspace_t ds)
{
  return DataspaceFree(ds);
}
