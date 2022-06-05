/*
 * Additional test for ADDRESS WITH options for checking the STREAM resource.
 * You need the programs "std" and "cat" to run this script.
 * The test gives some lines of debugging output.
 *
 * The test returns 1 and prints some lines to standard error in case of
 * an error.
 * The test returns 0 without any line written to stdardard error on success.
 *
 * A good testing environment does the following test; remove the redirection
 * element in case of errors and check the output:
 * "rexx addrtest3 >/dev/null"
 *
 * Don't pass arguments to the script. Arguments are used internally for
 * recursive calls.
 */

if countstr( "/", stream( ".", "C", "Query exists" ) ) > 0 then
   Std = "./std"
else
   Std = ".\std"

if arg(1) = "stdout" then do
   ADDRESS SYSTEM Std WITH OUTPUT STREAM 'stdout' ERROR STREAM 'stdout'
   exit 0
   end

if arg(1) = "stderr" then do
   ADDRESS SYSTEM Std WITH OUTPUT STREAM 'stderr' ERROR STREAM '<stderr>'
   exit 0
   end

if arg(1) = "mixed" then do
   ADDRESS SYSTEM Std WITH OUTPUT STREAM 'stdout' ERROR STREAM 'stderr'
   exit 0
   end

/*
 * Normal main routine
 *
 * First, initialize some useful values and files.
 */
parse source . . myself
Junk = "junk.tst"  /* Never change without changing below too */
if stream( Junk, "C", "Query exists" ) \= "" then
   call stream Junk, "C", "Open Write Replace" /* Auto-close on ADDRESS */

expected1 = "Line to stdout"
expected2 = "Line to stderr"

/*
 * Next, do a basic test.
 */
call info 'ADDRESS SYSTEM Std WITH OUTPUT STEM o. ERROR STEM e.'
ADDRESS SYSTEM Std WITH OUTPUT STEM o. ERROR STEM e.
say "o.0="o.0
say "o.1="o.1
say "e.0="e.0
say "e.1="e.1
if o.0 \= 1 | o.1 \= expected1 | e.0 \= 1 | e.1 \= expected2 then
   call oops lastaction
drop e. o.

/*
 * Next, check the default streams for proper running.
 */
call info 'ADDRESS REGINA myself "mixed" WITH OUTPUT STEM o. ERROR STEM e.'
ADDRESS REGINA myself "mixed" WITH OUTPUT STEM o. ERROR STEM e.
say "o.0="o.0
say "o.1="o.1
say "e.0="e.0
say "e.1="e.1
if o.0 \= 1 | o.1 \= expected1 | e.0 \= 1 | e.1 \= expected2 then
   call oops lastaction
drop e. o.

call info 'ADDRESS REGINA myself "stdout" WITH OUTPUT STEM o. ERROR STEM e.'
ADDRESS REGINA myself "stdout" WITH OUTPUT STEM o. ERROR STEM e.
say "o.0="o.0
say "o.1="o.1
say "o.2="o.2
say "e.0="e.0
if o.0 \= 2 | o.1 \= expected2 | o.2 \= expected1 | e.0 \= 0 then
   call oops lastaction
drop e. o.

call info 'ADDRESS REGINA myself "stderr" WITH OUTPUT STEM o. ERROR STEM e.'
ADDRESS REGINA myself "stderr" WITH OUTPUT STEM o. ERROR STEM e.
say "o.0="o.0
say "e.0="e.0
say "e.1="e.1
say "e.2="e.2
if o.0 \= 0 | e.0 \= 2 | e.1 \= expected2 | e.2 \= expected1 then
   call oops lastaction
drop e. o.

call info 'ADDRESS COMMAND WITH OUTPUT STREAM "junk.tst" ERROR STREAM Junk'
ADDRESS COMMAND WITH OUTPUT STREAM 'junk.tst' ERROR STREAM Junk
call info 'ADDRESS COMMAND "'Std'" (with some implicite redirections)'
ADDRESS COMMAND Std
line1 = Linein( Junk )
line2 = Linein( Junk )
Say "Line 1 from '"Junk"' is: '"line1"'"
Say "Line 2 from '"Junk"' is: '"line2"'"
If line1 \= expected2 | line2 \= expected1 then
   call oops lastaction
ADDRESS COMMAND WITH OUTPUT NORMAL ERROR NORMAL

/*
 * junk.tst currently contains the two expected lines.
 */
call info 'ADDRESS SYSTEM "cat" WITH INPUT STREAM Junk OUTPUT APPEND STREAM Junk'
ADDRESS SYSTEM "cat" WITH INPUT STREAM Junk OUTPUT APPEND STREAM Junk
line1 = Linein( Junk )
line2 = Linein( Junk )
line3 = Linein( Junk )
line4 = Linein( Junk )
Say "Line 1 from '"Junk"' is: '"line1"'"
Say "Line 2 from '"Junk"' is: '"line2"'"
Say "Line 3 from '"Junk"' is: '"line3"'"
Say "Line 4 from '"Junk"' is: '"line4"'"
If line1 \= expected2 | line2 \= expected1 then
   call oops lastaction
If line3 \= expected2 | line4 \= expected1 then
   call oops lastaction

/*
 * Reset address to default
 */
Address System WITH INPUT NORMAL OUTPUT NORMAL ERROR NORMAL

exit 0

info:
   say ""
   lastaction = arg(1)
   say "Action:" lastaction
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

DoExit0:
   exit 0
