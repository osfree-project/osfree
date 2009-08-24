/*
 * Testing file for the extended syntax of the CALL instruction introduced
 * in Regina 3.3.
 *
 * Usage: rexx lexical2 [>output]
 * If an error occurs one message with one line is written to standard error
 * and the exit code is 1. The redirected output should be send to the
 * maintainers of Regina to fix the behaviour.
 * On success "All tests passed successfully." is written to standard output
 * and the exit code is 0.
 */

/*
 * Check the version. Starting with 3.3RC1c we support the extended call
 * syntax again.
 */
parse version "REXX-"regina"_"v .
if regina \= "Regina" then do
   call lineout ,"Your REXX interpreter is not smart enough for this test."
   exit 1
   end

sub = ""
do len = 3 to length(v)
   if \datatype( left( v, len ), "N" ) then do
      sub = substr( v, len )
      v = left( v, len - 1 )
      leave
      end
   end

if v < 3.3 | ( v == "3.3" & ( sub < "RC1c" | left( sub, 4 ) = "beta" ) ) then do
   call lineout ,"Your Regina interpreter is not smart enough for this test."
   exit 1
   end
drop len v sub

/*
 * Test the OS to determine the way we can remove a file.
 */
parse source os .
if os = 'UNIX' | os = 'BEOS' | os = 'QNX' | os = 'AMIGA' | os = 'LINUX' then
   osdel1 = 'rm -f'
else
   osdel1 = 'del'

/*
 * Test for a debugging version of Regina.
 */
QUEUE "options NOEXT_COMMANDS_AS_FUNCS; call dumptree; exit 0"
t = trace()
trace o
ADDRESS REXX "" WITH INPUT FIFO "" OUTPUT FIFO "" ERROR FIFO ""
trace value t
call desbuf
if rc = 0 then
   dumptree = "call dumptree"
else
   dumptree = ""

/*
 * Be sure our generated scripts will be invoked first.
 */
OLD_MACRO_PATH = value( "REGINA_MACROS", , "SYSTEM")
call value "REGINA_MACRO_PATH", directory(), "SYSTEM"

call stream  "A.rexx", "C", "OPEN WRITE REPLACE"
call lineout "A.rexx", 'say "arguments: " arg()'
call lineout "A.rexx", 'retval = 10'
call lineout "A.rexx", 'do i = 1 to arg()'
call lineout "A.rexx", '   if arg( i,'O' ) then '
call lineout "A.rexx", '      iterate'
call lineout "A.rexx", '   say "argument" i || ":" arg( i )'
call lineout "A.rexx", '   retval = retval + 2**i + arg( i ) * i'
call lineout "A.rexx", '   end'
call lineout "A.rexx", 'say "returning" retval'
call lineout "A.rexx", 'exit retval'
call stream  "A.rexx", "C", "CLOSE"

errs = 0
errs = errs + check( "a=1,2                           ", "64.1", "64.1", ""   )
errs = errs + check( "a=(                             ", "36"  , "36"  , ""   )
errs = errs + check( "a=()                            ", "37"  , "37"  , ""   )
errs = errs + check( "a=(,3)                          ", "37.1", "37.1", ""   )
errs = errs + check( "a=(2,)                          ", "37.1", "37.1", ""   )
errs = errs + check( "a=(2,3)                         ", "37.1", "37.1", ""   )
errs = errs + check( "a=(())                          ", "37"  , "37"  , ""   )
errs = errs + check( "a=((,3))                        ", "37.1", "37.1", ""   )
errs = errs + check( "a=((2,))                        ", "37.1", "37.1", ""   )
errs = errs + check( "a=((2,3))                       ", "37.1", "37.1", ""   )
errs = errs + check( "a=(2+(2,3))                     ", "37.1", "37.1", ""   )
errs = errs + check( "a=(2,3+2)                       ", "37.1", "37.1", ""   )
errs = errs + check( "a=((2,3+2))                     ", "37.1", "37.1", ""   )
errs = errs + check( "a=((2,3+2)+2)                   ", "37.1", "37.1", ""   )
errs = errs + check( "a=(1,,,4)                       ", "37.1", "37.1", ""   )
errs = errs + check( "a=(1,,,(3,4))                   ", "37.1", "37.1", ""   )
errs = errs + check( "a=((1,2),,,4)                   ", "37.1", "37.1", ""   )
errs = errs + check( "result=a(                       ", "36"  , "36"  , ""   )
errs = errs + check( "result=a()                      ", ""    , ""    , "10" )
errs = errs + check( "result=a(,3)                    ", ""    , ""    , "20" )
errs = errs + check( "result=a(2,)                    ", ""    , ""    , "14" )
errs = errs + check( "result=a(2,3)                   ", ""    , ""    , "24" )
errs = errs + check( "result=a(())                    ", "37"  , "37"  , ""   )
errs = errs + check( "result=a((,3))                  ", "37.1", "37.1", ""   )
errs = errs + check( "result=a((2,))                  ", "37.1", "37.1", ""   )
errs = errs + check( "result=a((2,3))                 ", "37.1", "37.1", ""   )
errs = errs + check( "result=a(2+(2,3))               ", "37.1", "37.1", ""   )
errs = errs + check( "result=a(2,3+2)                 ", ""    , ""    , "28" )
errs = errs + check( "result=a((2,3+2))               ", "37.1", "37.1", ""   )
errs = errs + check( "result=a((2,3+2)+2)             ", "37.1", "37.1", ""   )
errs = errs + check( "result=a(1,,,4)                 ", ""    , ""    , "45" )
errs = errs + check( "result=a(1,,,(3,4))             ", "37.1", "37.1", ""   )
errs = errs + check( "result=a((1,2),,,4)             ", "37.1", "37.1", ""   )
errs = errs + check( "call a(                         ", "36"  , "36"  , ""   )
errs = errs + check( "call a()                        ", "37"  , ""    , "10" )
errs = errs + check( "call a ()                       ", "37"  , ""    , "10" )
errs = errs + check( "call a() 1                      ", "37"  , "35"  , ""   )
errs = errs + check( "call a()+1                      ", "37"  , "35"  , ""   )
errs = errs + check( "call a 1 ()                     ", "37"  , "37"  , ""   )
errs = errs + check( "call a 1 (),2                   ", "37"  , "37"  , ""   )
errs = errs + check( "call a() 1,2                    ", "37"  , "35"  , ""   )
errs = errs + check( "call a ()1                      ", "37"  , "35"  , ""   )
errs = errs + check( "call a ()1,2                    ", "37"  , "35"  , ""   )
errs = errs + check( "call a (1)1                     ", ""    , ""    , "23" )
errs = errs + check( "call a (1)1,2                   ", ""    , ""    , "31" )
errs = errs + check( "call a(,3)                      ", "37.1", ""    , "20" )
errs = errs + check( "call a(2,)                      ", "37.1", ""    , "14" )
errs = errs + check( "call a(1 2)                     ", ""    , ""    , "0"  )
errs = errs + check( "call a(1,2) (3)                 ", "37.1", "35"  , ""   )
errs = errs + check( "call a(1) (2) (3)               ", ""    , ""    , "0"  )
errs = errs + check( "call a(1) (2), (3)              ", ""    , ""    , "0"  )
errs = errs + check( "call a(2,3)                     ", "37.1", ""    , "24" )
errs = errs + check( "call a(())                      ", "37"  , "37"  , ""   )
errs = errs + check( "call a((,3))                    ", "37.1", "37.1", ""   )
errs = errs + check( "call a((2,))                    ", "37.1", "37.1", ""   )
errs = errs + check( "call a((2,3))                   ", "37.1", "37.1", ""   )
errs = errs + check( "call a(2+(2,3))                 ", "37.1", "37.1", ""   )
errs = errs + check( "call a(2,3+2)                   ", "37.1", ""    , "28" )
errs = errs + check( "call a((2,3+2))                 ", "37.1", "37.1", ""   )
errs = errs + check( "call a((2,3+2)+2)               ", "37.1", "37.1", ""   )
errs = errs + check( "call a(1,,,4)                   ", "37.1", ""    , "45" )
errs = errs + check( "call a(1,,,(3,4))               ", "37.1", "37.1", ""   )
errs = errs + check( "call a((1,2),,,4)               ", "37.1", "37.1", ""   )
errs = errs + check( "call a((()))                    ", "37"  , "37"  , ""   )
errs = errs + check( "call a((2,()))                  ", "37.1", "37.1", ""   )
errs = errs + check( "call a(((2,3)))                 ", "37.1", "37.1", ""   )
errs = errs + check( "call a((2,(3)))                 ", "37.1", "37.1", ""   )
errs = errs + check( "call a((2,(3+2))+2)             ", "37.1", "37.1", ""   )
errs = errs + check( "call a(((2,3+2))+2)             ", "37.1", "37.1", ""   )
errs = errs + check( "call a((1),,,(4))               ", "37.1", ""    , "45" )
errs = errs + check( "call a((1)),,,(4))              ", "37.2", "37.2", ""   )
errs = errs + check( "call a((1,2),,,4)               ", "37.1", "37.1", ""   )
errs = errs + check( "call a(((1,2)),,,4)             ", "37.1", "37.1", ""   )
errs = errs + check( "call a((1,2),,,(4))             ", "37.1", "37.1", ""   )
errs = errs + check( "call a()))                      ", "37"  , "37.2", ""   )
errs = errs + check( "call a 2,()                     ", "37"  , "37"  , ""   )
errs = errs + check( "call a (2,3)                    ", "37.1", ""    , "24" )
errs = errs + check( "call a 2,(3)                    ", ""    , ""    , "24" )
errs = errs + check( "call a 2,(3+2)+2                ", ""    , ""    , "32" )
errs = errs + check( "call a (2,3+2)+2                ", "37.1", "35"  , ""   )
errs = errs + check( "call a 1,,,(4)                  ", ""    , ""    , "45" )
errs = errs + check( "call a (1),,,(4)                ", ""    , ""    , "45" )
errs = errs + check( "call a (1+2),,,(3+4)            ", ""    , ""    , "59" )
errs = errs + check( "call a 1+(2,3)                  ", "37.1", "37.1", ""   )
errs = errs + check( "call a 1,,,4                    ", ""    , ""    , "45" )
errs = errs + check( "call a 1 + 2,,,3 + 4            ", ""    , ""    , "59" )
errs = errs + check( "call a (1,2),,,4                ", "37.1", "37.1", ""   )
errs = errs + check( "call a 1,,,(3,4)                ", "37.1", "37.1", ""   )
errs = errs + check( "call a a()                      ", ""    , ""    , "22" )
errs = errs + check( "call a ,a() + 1,                ", ""    , ""    , "36" )
errs = errs + check( "call a ,trunc((1+2)),3          ", ""    , ""    , "37" )
errs = errs + check( "call a ,trunc((1+2),1),3        ", ""    , ""    , "37" )
errs = errs + check( "call a(a())                     ", ""    , ""    , "22" )
errs = errs + check( "call a((a()))                   ", ""    , ""    , "22" )
errs = errs + check( "call a(,a() + 1,)               ", "37.1", ""    , "36" )
errs = errs + check( "call a(trunc((1+2)))            ", ""    , ""    , "15" )
errs = errs + check( "call a(trunc((1+2),1))          ", ""    , ""    , "15" )
errs = errs + check( "call a(trunc((1+2)),1)          ", "37.1", ""    , "21" )
osdel1 "TST.rexx"
osdel1 "A.rexx"

call value "REGINA_MACRO_PATH", OLD_MACRO_PATH, "SYSTEM"
if errs > 0 then do
   call lineout "stderr", ""
   call lineout "stderr", "Some tests didn't pass successfully."
   call lineout "stderr", "Please, check the standard output log."
   return 1
   end

say "All tests passed successfully."
return 0

check: procedure expose dumptree
   a = dumptree || ";" strip( arg( 1 ) ) || "; return RESULT"
   t = trace()
   trace o
   call stream  "TST.rexx", "C", "OPEN WRITE REPLACE"
   call lineout "TST.rexx", a
   call stream  "TST.rexx", "C", "CLOSE"
   call value "REGINA_OPTIONS", "", "SYSTEM"
   address REXX "TST.rexx" with output stem o0. error stem e0.
   rs0 = .RS; rc0 = .RC
   call value "REGINA_OPTIONS", "CALLS_AS_FUNCS", "SYSTEM"
   address REXX "TST.rexx" with output stem o1. error stem e1.
   rs1 = .RS; rc1 = .RC
   trace value t

   v0 = 0; v1 = 0
   if rs0 >= 0 & rc0 > 100 & arg( 2 ) \= "" then do
      match = "Error" arg( 2 )
      ml = length( match )
      do i = 1 to e0.0
         if left( e0.i, ml ) = match then do
            v0 = 1
            leave
            end
         end
      end
   if rs0 >= 0 & rc0 = arg( 4 ) then
      v0 = 1

   if rs1 >= 0 & rc1 > 100 & arg( 3 ) \= "" then do
      match = "Error" arg( 3 )
      ml = length( match )
      do i = 1 to e1.0
         if left( e1.i, ml ) = match then do
            v1 = 1
            leave
            end
         end
      end
   if rs1 >= 0 & rc1 = arg( 4 ) then
      v1 = 1

   if v0 & v1 then
      return 0

   say ""
   say "=========================================================================="
   say a

   say "---"
   if \v0 then do
      do i = 1 to o0.0
         say "0O" || right( i, 2, "0" ) || ":" o0.i
         end
      do i = 1 to e0.0
         say "0E" || right( i, 2, "0" ) || ":" e0.i
         end
      end
   if \v0 & \v1 then
      say "----------"
   if \v1 then do
      do i = 1 to o1.0
         say "1O" || right( i, 2, "0" ) || ":" o1.i
         end
      do i = 1 to e1.0
         say "1E" || right( i, 2, "0" ) || ":" e1.i
         end
      end
   return 1

