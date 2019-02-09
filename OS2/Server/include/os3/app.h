/* os2app client-side API */

#ifndef __OS3_APP_H__
#define __OS3_APP_H__

#ifdef __cplusplus
  extern "C" {
#endif

#include <os3/thread.h>
#include <os3/dataspace.h>

void AppClientNotify(l4_os3_thread_t client_id);

APIRET AppClientAreaAdd(PID pid,
                        //l4_os3_cap_idx_t client_id,
                        PVOID pAddr,
                        ULONG ulSize,
                        ULONG ulFlags);

APIRET AppClientDataspaceAttach(PID pid,
                                //l4_os3_cap_idx_t   client_id,
                                PVOID              pAddr,
                                l4_os3_dataspace_t ds,
                                ULONG              ulRights);

APIRET AppClientDataspaceRelease(PID pid,
                                 //l4_os3_cap_idx_t   client_id,
                                 l4_os3_dataspace_t ds);

#ifdef __cplusplus
  }
#endif

#endif
