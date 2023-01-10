#!/usr/bin/env regina
/*
 * External queues are identified by '@' in the queue name. MYQ@ is a queue available on the local
 * machine with rxstack listening on the default port: 5757
 *
 */
Parse Arg action
If action = 'send' Then Call RunSender
Else Call RunReceiver
Return 0

RunSender: Procedure
Call RxQueue 'S', 'MYQ@' -- set our queue to MYQ@
Do i = 1 To 10
   line = 'line'i
   Queue line
   Say 'Queued:' line
End
Queue 'quit'
Say 'Queued:' 'quit'
Return

RunReceiver: Procedure
Call RxQueue 'C', 'MYQ@' -- create MYQ on the local machine
Call RxQueue 'S', 'MYQ@' -- set our queue to MYQ@
Call RxQueue 'T', 0 -- set the timeout on the current queue to 0 (wait indefinitely)
Do Forever
   Parse Pull line
   Say 'Received:' line
   If line = 'quit' Then Leave
End
Call RxQueue 'D', 'MYQ@' -- delete MYQ
Return
