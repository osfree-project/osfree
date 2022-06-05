/*
 * Generic test for ADDRESS WITH options.
 * You need the program "cat" to run this script.
 * The test gives some lines of debugging output.
 *
 * The test returns 1 and prints some lines to standard error in case of
 * an error.
 * The test returns 0 without any line written to stdardard error on success.
 *
 * An argument may be supplied for a queue's name.
 *
 * A good testing environment does the following tests, remove the redirection
 * elements in case of errors and check the output:
 * 1) "rexx addrtest1 >/dev/null"
 * 2) "rexx addrtest1 SESSION >/dev/null"
 * 3) "rexx addrtest1 local_queue >/dev/null"
 * 4) "rexx addrtest1 SESSION@somewhere >/dev/null"
 * 5) "rexx addrtest1 remote_queue@somewhere >/dev/null"
 *
 * If you supply ALL as the argument, each of the above tests is done.
 */

parse upper arg q
if q = "ALL" then do
   trace o
   parse source . . script
   call extcall script
   call extcall script "SESSION"
   call extcall script "local_queue"
   call extcall script "SESSION@"
   call extcall script "remote_queue@localhost"
   exit 0
   end

internal = 1

resultFIFO = "first line,second line,third line"
resultLIFO = "third line,second line,first line"
resultSTEMLIFO = "stem line 3,stem line 2,stem line 1"
resultSTEMFIFO = "stem line 1,stem line 2,stem line 3"
resultSTEMLIFOFIFO = resultSTEMLIFO || "," || resultSTEMFIFO

qn = q
if qn = "" then
   qn = "(empty for default queue SESSION)"
say "--------------------------------------------"
say "Queue's name q is" qn
say "--------------------------------------------"

if q \= "" & q \= "SESSION" then do
   /* sigh, no chance to force a special queue without deletion and without
    * having a false queue.
    */
   if left( q, 8 ) \= "SESSION@" then do
      call RxQueue 'D', q
      call RxQueue 'C', q     /* doesn't return a value for the first */
   end
   call RxQueue 'S', q
   nq = RxQueue( 'G' )
   say NNG nq
   posat = pos( '@', q )
   if posat = 0 then
      posat = length( q ) + 1 /* if lengths are different */
   else
      internal = 0
   if left( q, posat ) \= left( nq, posat ) then do
      call oops "(setting the queue name to " || qn || ")"
      exit 1
      end
   end

call info "Just QUEUE, then PULL"
call fillq
call showq "DELETE", resultFIFO

if internal then do
   call info "LIFO> cat >LIFO"
   call fillq
   "LIFO> cat >LIFO"
   call showq "DELETE", resultLIFO

   call info "LIFO> cat >FIFO"
   call fillq
   "LIFO> cat >FIFO"
   call showq "DELETE", resultFIFO
   end

if q \= "" & q \= "SESSION" then
   call RxQueue 'D', q

call info 'ADDRESS SYSTEM "cat" WITH INPUT LIFO "" OUTPUT FIFO q'
call fillq
ADDRESS SYSTEM "cat" WITH INPUT LIFO "" OUTPUT FIFO q
call showq "DELETE", resultFIFO, q

call info 'ADDRESS SYSTEM "cat" WITH INPUT LIFO "" OUTPUT APPEND FIFO q'
call fillq
ADDRESS SYSTEM "cat" WITH INPUT LIFO "" OUTPUT APPEND FIFO q
call showq "DELETE", resultFIFO, q

s.0 = 3
s.1 = "stem line 1"
s.2 = "stem line 2"
s.3 = "stem line 3"

call info 'ADDRESS SYSTEM "cat" WITH INPUT STEM s. OUTPUT LIFO q'
ADDRESS SYSTEM "cat" WITH INPUT STEM s. OUTPUT LIFO q
call showq "PERSIST", resultSTEMLIFO, q

call info 'ADDRESS SYSTEM "cat" WITH INPUT STEM s. OUTPUT APPEND FIFO q'
ADDRESS SYSTEM "cat" WITH INPUT STEM s. OUTPUT APPEND FIFO q
call showq "DELETE", resultSTEMLIFOFIFO, q

call info 'ADDRESS SYSTEM "echo Hello World" WITH OUTPUT FIFO q'
ADDRESS SYSTEM "echo Hello World" WITH OUTPUT FIFO q
call showq "DELETE", "Hello World", q

l1 = 'Line to stderr'
l2 = 'Line to stdout'
call info 'ADDRESS SYSTEM WITH OUTPUT APPEND STREAM "stdout" ERROR APPEND STREAM "stdout"'
ADDRESS SYSTEM WITH OUTPUT APPEND STREAM 'stdout' ERROR APPEND STREAM 'stdout'
call info 'ADDRESS SYSTEM "std"'
ADDRESS SYSTEM "std"
Say "If you don't see '"l1"' followed by '"l2"', there is a problem!"

call info 'ADDRESS SYSTEM WITH OUTPUT STREAM "junk" ERROR STREAM "junk"'
ADDRESS SYSTEM WITH OUTPUT STREAM 'junk' ERROR STREAM 'junk'
call info 'ADDRESS SYSTEM "std"'
ADDRESS SYSTEM "std"
line1 = Linein( 'junk' )
line2 = Linein( 'junk' )
Say "Line 1 from 'junk' is: '"line1"', should be: '"l1"'"
Say "Line 2 from 'junk' is: '"line2"', should be: '"l2"'"
ADDRESS SYSTEM "rm -f junk"
if line1 \= l1 | line2 \= l2 then
   call oops lastaction

exit 0

info:
   say ""
   lastaction = arg(1)
   say "Action:" lastaction
   return

fillq: procedure
   say ""
   say "Action:" arg(1)
   queue "first line"
   queue "second line"
   queue "third line"
   return

showq: procedure expose lastaction
   saved = ""
   error = 0

   /* chop the result string into single lines */
   resStem = arg(2)
   resStem.0 = 0
   do while resStem \= ''
      resStem.0 = resStem.0 + 1
      h = resStem.0
      parse var resStem resStem.h "," resStem
      end

   if arg(3) \= "" then do
      saved = rxqueue('G')
      call rxqueue 'S', arg(3)
      say lastaction "(Queue" arg(3) || ", current is" saved || ")"
      end
   gotten = queued()
   if gotten \= resStem.0 then
      error = 1
   do i = 1 to gotten
      parse pull line
      if line \= resStem.i then
         error = 1
      if arg(1) \= "DELETE" then
         queue line
      say line
      end
   if saved \= "" then do
      call rxqueue 'S', saved
      say "(current set back to" rxqueue('G') || ")"
      end
   if error then
      call oops lastaction
   return

oops: procedure
   call lineout "<stderr>", "----------------------------------------"
   call lineout "<stderr>", "Something's wrong in test" arg(1)
   call lineout "<stderr>", "----------------------------------------"
   call lineout "<stderr>", ""
   exit 1
   return

extcall: procedure
   if arg() > 1 then
      address REXX arg(1)        WITH error stem err.
   else
      address REXX arg(1) arg(2) WITH error stem err.
   if err.0 = 0 then
      return
   do i = 1 to err.0
      call lineout "<stderr>", err.i
      end
   exit 1
   return
