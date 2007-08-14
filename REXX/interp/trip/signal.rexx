/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992  Anders Christensen <anders@solan.unit.no>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version. 
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

written = 0 
trace off


signal test009

test001:
/* === 001 : Simple use of Signal on syntax  ============================ */
   call notify 'simple'
   signal on syntax name syntax001
   if ( 1 * 'asdfgsgf' ) then
      say 'minus one'
   call complain 'SIGNAL ON SYNTAX does not trap runtime errors'
   signal test002

   syntax001:
      signal test002


test002:
/* === 002 : Does CALL ON terminate statement =========================== */
/*   call notify 'call_on' */
   call random ,,1000
   num = random() 
   call on halt name halt002
   call substr 'kill'("-15" getpid()),1,random()

   if ( num == random() ) then
      call complain "CALL ON HALT does not complete statement"

   if ( magic ^== '002' ) then
      call complain "CALL ON HALT not performed"

   signal test003

   halt002:
      magic = '002'
      return 


test003:
/* === 003 : Check that ERROR condition works ================== */
   call notify 'error'
   signal on error name error003
   'rc' 10
   call complain 'condition ERROR does not appear to work'
   signal test004

   error003:
      signal test004


test004:
/* === 004 : Check that ERROR traps non-trapped FAILURE */
   call notify 'failure'
   signal off failure
   signal on error name error004
   'rc -10'
   call complain 'condition ERROR does not trap FAILURES'
   signal test005

   error004:
      signal test005


test005:
/* === 005 : Check that FAILURE does not trap non-trapped ERROR ====== */
   call notify 'nofailure'
   signal off error
   signal on failure name fail005
   'rc 10'
   signal test006

   fail005:
      call complain 'FAILURE does also trap ERROR conditions'
      signal test006


test006:
/* === 006 : CALL ON can be performed between two statement in an
             INTERPRET statment ======================================= */
   call notify 'interpret'
   call on halt name halt006
   first = 'none'
   second = 'none'
   third = 'none'
   stmnt = 'first = "one" ;second = "kill"("-2" getpid()) ; third = "three"'
   interpret stmnt
   if first^=="one" then
      call complain "Strange effects with INTERPRET and CALL ON"

   if second^=="" then
      call complain "CALL ON seems to interrupt the statement"

   if third^=='three' then
      call complain "CALL ON interrupts INTERPRET statement"

   signal test007

   halt006:
      if third=='three' then
         call complain "CALL ON triggered after INTERPRET was finished"
      else if (third^=='none') then
         call complain "Strange effects with CALL ON"

      if second=='none' then
         call complain "CALL ON triggered before statement was finished"
      else if (second^=='') then
         call complain "Some strange effects with CALL ON, second=<"second">"
      return


test007:
/* === 007 : That SIGNAL/CALL OFF XXX NAME YYY is illegal ========= */
   call notify 'off_name'
   signal on syntax name syntax007
   where = 'one'
   interpret 'signal off halt name foobar'
   where = 'junk'
   signal next007

   syntax007:
      if where ^== 'one' then
         call complain "SIGNAL OFF XXX NAME YYY is (illegally) allowed"
      
   next007:
   signal on syntax name syntax007_2
   where = 'two'
   interpret 'call off halt name foobar'
   where = 'junk'
   signal test008

   syntax007_2:
      if where ^== 'two' then
         call complain "CALL OFF XXX NAME YYY is (illegally) allowed"
      
   signal test008 


test008:
/* === 008 : Check that DELAY mode works for the conditions it is 
             supposed to work for (ERROR/FAILURE/NOTREADY) =========== */
call notify 'delay'

   call on error name error008 
   current = condition('d') 
   'rc 10'
   if (rc^==11) then
      call complain 'Variable RC set in errorhandler does not survive return'
   if (condition('d')^==current) then
      call complain 'New ERROR condition was not ignore in ERROR handler'

   signal test009

   error008:
      if (rc^=='10') then
         call complain 'Variable RC not properly set after ERROR condition'
      'rc' 11
      if (rc^=='11') then 
         call complain 'Variable RC seems to be locked in delayed mode'
      if (condition('d')^='rc 10') then
         call complain 'ERROR condition does not delay further ERRORS'
      return 


test009:
/* === 009 : If HALT delayed? It should not be ==================== */
   call notify 'halt' 
   signal off syntax
   call on halt name halt009
   count = 0
   junk = 'kill'('-15' getpid()) 
   if ( ref == ref2 ) then
      call complain 'CALL ON HALT appears to be ingored while delayed'

   if (sourceline(sigl) ^== "         junk = 'kill'('-15', getpid())") then
      call complain 'CALL ON HALT offsets SIGL by non-zero value'

   if ( count ^== 2 ) then
      call complain 'Second HALT seems to have been ignored'

   signal test010

   halt009:
      if (count = 0) then do
         ref = sigl
         count = 1 
         junk = 'kill'('-15', getpid())
         junk = junk || junk
         call on halt name halt009
         end
      else do
         ref2 = sigl
         count = 2
         if (ref ^== ref2-16) then
            call complain 'Troubles in line counting ...'
         end
      return


test010:
/* === 010 : NOVALUE must trigger on both unset and dropped 
             variables ============================================== */
   call notify 'novalue'
   signal on novalue name noval010
   junk = 'just junk'
   message = ''
   junk = has_not_been_set 
   call complain "NOVALUE does not trap unset variables"
   
   noval010:
   if junk^=='just junk' then
      call complain 'SIGNAL ON NOVALUE was not interrupted'

   signal on novalue name noval010_2
   junk = 'bar'
   has_been_dropped = 'foo'
   drop has_been_dropped 
   message = 'just before'
   junk = has_been_dropped
   call complain 'SIGNAL ON NOVALUE does not seem to have been triggered'
   signal test011

   noval010_2:
      if message ^== 'just before' then

         call complain 'Triggered by DROP, not assignment'

      if junk^=='bar' then
         call complain 'SIGNAL ON NOVALUE finished statement'

   signal test011



test011:
/* === 011 : SIGNAL stops execution, CALL continues it =========== */
   call notify 'execution'
   call random ,,1000
   first = random() 
   second = random()
   third = random()
   call random ,,1000
   signal on halt name halt011 
   junk = '.' 'kill'('-15' getpid()) '.' random()
   call complain 'SIGNAL ON HALT does not seem to work'
 
   halt011:
      if (first ^== random()) then
         call complain 'SIGNAL ON HALT does not seem to break statement'

      call on halt name halt011_2
      call random ,,1000
      junk = '.' "kill"('-15' getpid()) '.' random()
      if (third ^== random()) then
         call complain 'CALL ON HALT evalueate expression twice?'

      signal test012

   halt011_2:
      if (second ^== random()) then
         call complain 'CALL ON HALT does not continue expression'
      return


test012:
/* === 012 : Make sure that CALL ON triggered in the expression
             of an IF, DO or SELECT, is effectuated at the next
             statement boundary, not after the (structured) 
             statement is finished ==================================== */
   call notify 'triggered'
   count = 0
   signal on syntax name syntax012_1
   if ( 1 + 1 || 'a' ) then
      call complain "SYNTAX ON executes IF's statement after syntax error"
   call complain "SYNTAX ON continued execution"

   syntax012_1:
      signal on syntax name syntax012_2
      do i=4 to left('a','b')
         call complain "SYNTAX ON executes contents of DO after error"
         end
      call complain "SYNTAX ON continued execution"

   syntax012_2:
      signal on syntax name syntax012_3
      drop two
      select
         when 0 then nop
         when 'two' then 
            call complain 'SYNTAX ON exectues contents after error in WHEN'
         when 1 then 
            call complain 'SYNTAX ON executes rest of WHEN after error'
         otherwise
            nop
         end
      call complain 'SYNTAX ON continued execution'

   syntax012_3:
      signal test013



test013:
/* === 013 : HALT in a CALL statement is trigger before return ========= */
   call notify 'return'
   call on halt name halt013
   count = 0
   call sub013 'kill'('-15' getpid())
   if count^==1 then
      call complain 'HALT handler does not seem to have been called'

   signal test014

   sub013:
      count = 1 
      return 

   halt013:
      if count^==0 then
         call complain 'CALL ON HALT executes subroutine before trigging'
      return


test014:
/* === 014 : RC is set when SYNTAX is triggered ==================== */
   call notify 'rc/syntax'
   signal on syntax name syntax014
   rc = 0
   old_envir = address()
   address 'COMMAND'

   junk = no_such_function('foobar')
   call complain 'SYNTAX condition was not triggered'
   signal after014

   syntax014:
      if rc^==43 then
         call complain 'RC not set when SIGNAL ON SYNTAX triggered'

after014:
   address (old_envir)
      signal test015


test015: trace off
/* === 015 : Get the lineno right in INTERPRET ====================== */
   call notify 'lineno1'
   signal on syntax name syntax015
   rc = 0 
   statement = "junk = 'foobar' ; call left() ; morejunk = 'xyzzy'"
   thisline = syntax015() 
   interpret statement
   call complain 'SIGNAL SYNTAX did not trigger'
   signal test016

   syntax015:
      if rc=0 then 
         return sigl
      if thisline ^== sigl-1 then
         call complain 'SIGL not properly set when SYNTAX in INTERPRET'
      signal test016      



test016:
/* === 015 : Get the lineno right in INTERPRET ====================== */
   call notify 'lineno2'
   call on halt name halt016
   count = 0 
   statement = "junk='foobar'; junk='kill'('-15' getpid()); morejunk='xyzzy'"
   thisline = halt016() 
   interpret statement
   signal test017

   halt016:
      if count=0 then do
         count = 1 ;
         return sigl
         end
      if thisline ^== sigl-1 then
         call complain 'SIGL not properly set when CALL ON HALT in INTERPRET'
      return      



test017:
/* === 017 : When in delayed mode, ERRORs are ignored, and only 
             the last is remembered ================================ */
   call notify 'in_delay'
   call on halt name halt017
   count = 1 
   junk = 'kill'( '-2' getpid()) 
   if count^=='4' then
      call complain 'Something is wrong in the order ERRORs are handled'

   signal test018

   halt017:
      if count=1 then do
         if (condition('D')^=='SIGINT') then
            call complain 'Result from condition() is incorrect'

         junk = 'kill'( '-1' getpid())
         if (condition('D')^=='SIGINT') then 
            call complain 'ERRORs not ignored while in handler'

         junk = 'kill'( '-15' getpid())
         count = 2 
         call on halt name halt017
         return
         end

      if count=2 then do
         count = 3
         if condition('D')^=='SIGTERM' then
            call complain 'Does not reset DELAY mode with CALL ON'
         junk = 'kill'( '-1' getpid() )
         return
         end

      count = 4 
      if condition('D')^=='SIGHUP' then
         call complain 'Wrong order in which ERRORs are handled'

      return



test018:
/* === 018 : Is different conditions properly stacked ============== */
   call notify 'stacking'
   call on halt name halt018
   call on error name error018
   
   count = 0 
   junk = 'kill'( '-15' getpid())
   if count ^== '7' then
      call complain 'did not trigger both ERROR and HALT'

   signal test019

   halt018:
      if count^=='0' then
         call complain 'HALT not called initially'
      
      count = count + 1
      'rc 10'
      if count^=='3' then
         call complain 'ERROR couldnt trigger when HALT was DELAYed'

      count = count + 4
      return      

   error018:
      if count^=='1' then
         call complain 'ERROR not properly called'

      count = count + 2
      return
      


test019:
/* === 019 : Is RESULT set at return from CALL ON? =============== */
   call notify 'result'
   call on failure name fail019
   result = 'foobar'
   'rc -10'
   if result^=='foobar' then
      call complain 'RESULT was set at return from CALL ON'

   signal test020

   fail019:
      return 'xyzzy'




test020:
/* === 020 : That CALL ON HALT in DELAYed mode is processed 
             afterwards, when the DELAY mode is removed ============== */
   call notify 'after_delay'
   call on halt name halt020
   count = 0
   junk = 'kill'( '-2' getpid())
   if (count^==15) then
      call complain 'CALL ON HALT does not seem to work'
   
   signal test021

   halt020:
      if count=0 then do
         count = count + 1 
         junk = 'kill'( '-15' getpid())
         if (count^==1) then
            call complain 'CALL ON HALT not in DELAYed mode'

         call on halt name halt020
         if (count^==15) then
            call complain 'CALL ON HALT did not remove DELAYed mode'

         return 
         end
  
      if count=1 then do
         count = count + 2 
         call on halt name halt020
         if (count^==3) then
            call complain 'CALL ON HALT not in DELAYed mode'

         junk = 'kill'( '-15' getpid())
         if (count^==15) then
            call complain 'CALL ON HALT did not clear DELAYED mode'
         return
         end
   
      if (count=3) then do
         count = count + 4
         junk = 'kill'( '-2' getpid())
         if (count^==7) then
            call complain 'CALL ON HALT triggered in DELAYed mode'
         return
         end

      if (count=7) then do
         count = count + 8 
         return
         end

      call complain 'Something is wrong with CALL ON HALT'
      return



test021:
/* === 021 : condition label name is not changed in subroutines ========== */
   call notify 'subrout'
   count = 0
   call on error name err021_1
   'rc 10'
   if (count^==1) then 
      call complain 'ERROR not triggered'

   call sub021
   count = count + 2
   'rc 10'
   if count^==4 then
      call complain 'Trap information set by handler'
      
   signal test022

   err021_2:
      count = 100
      return

   sub021:
      call on error name err022_2
      return      
      
   err021_1:
      if count^==0 & count^==3 then
         call complain 'Wrong error handler called'

      count = count + 1
      call on error name err022_2
      return 



test022:
/* === 022 : If delayed mode is cleared in a subroutine, then
             it should not be cleared at the level above ============= */
   call notify 'delay/sub'
   count = 0
   call on error name err022
   'rc 10'
   if (count^==1) then
      call complain 'ERROR condition was not properly handled'

   signal test023

   err022:
      count = count + 1
      if (count^==1) then
         call complain 'ERROR handler called incorrectly'
  
      call sub022
      'rc 10' 
      return 

   sub022:
      call off error
      return



test023:
/* === 023 : Information about the current trapped condition is to 
             be saved across routine calls =========================== */
   call notify 'ctc/sub'
   signal on error name err023_1
   'rc 10'
   call complain 'ERROR condition was not triggered'

   err023_1:
   if (condition('D')^=='rc 10') then
      call complain 'Current trapped condition has incorrect information'

   call sub023
   if (condition('D')^=='rc 10') then
      call complain 'Current trapped condition info not saved across subs'
 
   signal test024

   sub023:
      signal on error name err023_2
      if (condition('D')^=='rc 10') then 
         call complain 'CTC not "exposed" to subroutines'

      'rc 11'
      call complain 'ERROR was not properly triggered'
      
      err023_2:
      if (condition('D')^=='rc 11') then 
         call complain 'CTC was not properly set in sub routine'
      
      return



test024:
/* === 024 : does SIGNAL OFF remove CALL ON? ====================== */
   call notify 'on/off'
   call on error name err024
   signal off error
   'rc 10'
   signal test025
   
   err024:
      call complain 'SIGNAL OFF does not remove a CALL ON'
      return 



test025:
/* === 025 : does CALL ON HALT automagically change SIGL? ============= */
   call notify 'sigl/halt'
   call on halt name halt025
   sigl = 'junk'
   junk = 'kill'( '-15' getpid())
   if (sigl=='junk') then
      call complain 'SIGL not set during CALL ON HALT'
  
   signal test026

   halt025:
      return



test026:
/* === 026 : NOVALUE triggered at all types of symbols ================= */
   call notify 'symbols'
   signal on novalue name noval026_1
   junk = no_such_var_026
   call complain 'NOVALUE not triggered at simple value'
   
   noval026_1:
   signal on novalue name noval026_2
   junk = no_such_stem_026.
   call complain 'NOVALUE not triggered on stem symbols'

   noval026_2:
   signal on novalue name noval026_3
   junk = no_such_026.compound_026
   call complain 'NOVALUE not triggered on compound'

   noval026_3:
   signal on novalue name noval026_4
   stem_defined_026. = 'foobar'
   count = 1
   xyzzy = stem_defined_026.baz
   count = count + 1
   if (xyzzy^=='foobar') then
      call complain 'Something is wrong in the use of stems'

   count = count + 1
   xyzzy = stem_undefined_026.baz
   call complain 'NOVALUE not triggered on compound'
   signal test027

   noval026_4:
   if count^==3 then
      call complain 'NOVALUE triggered on unset compound when stem is set'
   signal test027



test027:
/* === 027 : Check that drop is triggered at NOVALUE ============= */
   call notify 'drop'
   signal on novalue name noval027_1
   drop no_such_027
   signal ok027_1

   noval027_1:
   call complain 'NOVALUE triggered for simple symbol in DROP'

   ok027_1:
   signal on novalue name noval027_2
   drop no_such_stem_027.
   signal ok027_2

   noval027_2:
   call complain 'NOVALUE triggered for stem symbol in DROP'

   ok027_2:
   signal on novalue name noval027_3
   drop no_stem_027.foobar
   signal ok027_3

   noval027_3:
   call complain 'NOVALUE triggered for compound symbol in DROP'

   ok027_3:
   signal test028



test028:

/* need to check novalue for expose and parse */


   say ' '
exit 0


ch: procedure expose sigl 
   parse arg first, second
   if first ^== second then do
      say
      say 'first  = /'first'/'
      say 'second = /'second'/'
      say "FuncTrip: error in " sigl":" sourceline(sigl) ; end
   return


complain:
   say ' ...'
   say 'Tripped in line' sigl':' arg(1)'.'
   length = charout(,'   (')
return


notify:
   parse arg word .
   written = written + length(word) + 2
   if written>75 then do
      written = length(word)
      say ' '
   end
   call charout , word || ', '
   return


error:
   say 'Error discovered in function insert()'
   return
