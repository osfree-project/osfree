/*
 * Generic test for CALL trapping of the conditions FAILURE and HALT.
 * You need the program "rc" to run this script.
 * The test gives some lines of debugging output.
 *
 * The test returns 1 and prints some lines to standard error in case of
 * an error.
 * The test returns 0 without any line written to stdardard error on success.
 *
 * A good testing environment does the following test; remove the redirection
 * element in case of errors and check the output:
 * "rexx addrtest2 >/dev/null"
 */
call on failure name MustBeNoFail
call on error name MustBeNoErr

address path 'rc 0'

signal on error name MustBeError
address path 'rc 1'
call oops "not trapped in condition ERROR, but it should"
exit 1

MustBeError:
if .RS \= 1 then do
   call oops ".RS not set correctly"
   exit 1
   end

call on error name MustBeNoErr
signal on failure name MustBeFailure
address path 'unknown_prg'
call oops "not trapped in condition FAILURE, but it should"
exit 1

MustBeFailure:
if .RS \= -1 then do
   call oops ".RS not set correctly"
   exit 1
   end

say ""
say "test passed"
return 0

MustBeNoFail:
   call oops "trapped in condition FAILURE without expectation"
   exit 1

MustBeNoErr:
   call oops "trapped in condition ERROR without expectation"
   exit 1

oops: procedure
   call lineout "<stderr>", "----------------------------------------"
   call lineout "<stderr>", arg(1)
   call lineout "<stderr>", "----------------------------------------"
   call lineout "<stderr>", ""
   exit 1
   return
