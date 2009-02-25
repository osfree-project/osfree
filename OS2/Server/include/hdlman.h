#ifndef HDLMAN_H
#define HDLMAN_H

#include <os2.h>

VOID hdlInit();
VOID hdlDone();

ULONG hdlNew(ULONG ulTypeId, PVOID pObject);
PVOID hdlGet(ULONG ulTypeId, ULONG ulHandle);
BOOL hdlRemove(ULONG ulTypeId, ULONG ulHandle);

#endif // HDLMAN_H
