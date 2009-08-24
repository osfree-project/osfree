/*
 * This program tests the external queue interface of Regina
 * It runs num_clients instances of itself, each instance will create a
 * named queued. For even numbered instances, lines will be queued
 * FIFO; odd numbered instances will be queued LIFO.
 * Once each instance queues its lines, it will read lines off
 * another stack; created by its opposite instance.
 * Opposite instances are eg. instance 1 and num_clients, 2 and 48, etc
 */
Trace o
parse source sys env prog
num_clients = 50 /* 50 */
num_lines = 10   /* 100 */
If Arg() = 0 Then
   Do
      cmd = "rexx"
      if stream( "./rexx", "C", "FSTAT" ) \= "" then
         cmd = "./rexx"
      if stream( "../rexx", "C", "FSTAT" ) \= "" then
         cmd = "../rexx"
      Do i = 1 To num_clients
         Address System cmd prog i '&'
      End
   End
Else
   Do
      /*
       * Set up the list of opposites...
       */
      Do i = 1 To num_clients
         opposite.i = 1+(num_clients-i)
      End
      Parse Arg instance
      say instance
      If instance // 2 = 0 Then order = 'fifo'
      Else order = 'lifo'
      call RxQueue 'Set', Rxqueue('Create', 'QUEUE'instance'@')
      /* Say '***Creating queue' Rxqueue('Create', 'QUEUE'instance'@'  ) */
      /*
       * push or queue num_lines lines onto our queue
       */
      Do i = 1 To num_lines
         line = 'line'i 'from instance' instance
         /* Say '***Going to put <'line'> on queue:' order */
         If order = 'fifo' Then Queue line
         Else Push line
      End
      /*
       * Sleep for 10 seconds, then find our opposite, and
       * set our default queue to its queue, and read off
       * the the lines, checking they are in the correct
       * order and the correct contents
       */
      Call Sleep(10)
/*
      Say '***Setting queue. Previous was:' rxqueue('Set', 'QUEUE'opposite.instance'@' )
      Say '***Getting queue. Now:' rxqueue('Get')
*/
      Call rxqueue 'Set', 'QUEUE'opposite.instance'@'
      /*
       * The order of the lines is the opposite to
       * the order in which we put our lines on our queue
       */
      If order = 'lifo' Then
         Do
            /*
             * PULL the lines off the stack, and check them
             * they should be in FIFO order,
             */
            Do i = 1 To num_lines
               Parse Pull line
               exp = 'line'i 'from instance' opposite.instance
               If line \=  exp Then Call Abort line, exp
            End
         End
      Else
         Do
            /*
             * PULL the lines off the stack, and check them
             * they should be in LIFO order,
             */
            Do i = num_lines To 1 By -1
               Parse Pull line
               exp = 'line'i 'from instance' opposite.instance
               If line \= exp Then Call Abort line, exp
            End
         End
      /*
       * Cleanup up our opposite's queue
       */
      Call Rxqueue 'Delete', 'QUEUE'opposite.instance'@'
      Say 'Instance' instance 'finished.'
   End
Return 0

Abort: Procedure Expose instance
Parse Arg line, exp
say '--------------------------------------------------------------------'
Say 'Error validating instance' instance'. Got <'line'> Expecting <'exp'>'
Call Rxqueue 'Delete', 'QUEUE'opposite.instance'@'
say '--------------------------------------------------------------------'
Exit 1
