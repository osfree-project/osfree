/* os2app client-side API */

#ifndef __OS3_APP_H__
#define __OS3_APP_H__

#ifdef __cplusplus
  extern "C" {
#endif

APIRET AppClientGetLoadError(ULONG  ulClient,
                             PBYTE  szLoadError,
                             PULONG pcbLoadError);

APIRET AppClientTerminate(ULONG ulClient);

APIRET AppClientAreaAdd(ULONG ulClient,
                        PVOID pAddr,
                        ULONG ulSize,
                        ULONG ulFlags);

APIRET AppClientDataspaceAttach(ULONG              ulClient,
                                PVOID              pAddr,
                                l4_os3_dataspace_t ds,
                                ULONG              ulRights);

APIRET AppClientDataspaceRelease(ULONG              ulClient,
                                 l4_os3_dataspace_t ds);

#ifdef __cplusplus
  }
#endif

#endif
