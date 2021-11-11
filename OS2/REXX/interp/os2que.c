/* External queues implementation using OS/2 quecalls */

#include "rexxsaa.h"


APIRET APIENTRY rexxCreateQueue( PSZ Buffer,
                                 ULONG BuffLen,
                                 PSZ RequestedName,
                                 ULONG* DupFlag )
{
   return 0;
}

APIRET APIENTRY rexxDeleteQueue( PSZ QueueName )
{
   return 0;
}

APIRET APIENTRY rexxQueryQueue( PSZ QueueName,
                                ULONG* Count )
{
   return 0;
}

APIRET APIENTRY rexxAddQueue( PSZ QueueName,
                              PRXSTRING EntryData,
                              ULONG AddFlag )
{
   return 0;
}

APIRET APIENTRY rexxPullQueue( PSZ QueueName,
                               PRXSTRING DataBuf,
                               PDATETIME TimeStamp,
                               ULONG WaitFlag )
{
   return 0;
}
