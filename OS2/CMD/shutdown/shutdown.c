#include <os2.h>

HAB hab;
HMQ hmq;

void main (void)
{
  if (hab = WinInitialize(0))
    if (hmq = WinCreateMsgQueue(hab,0))
      WinShutdownSystem(hab, hmq);
}

