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

/*
 * Following statements are tested otherwhere:
 *
 *   PULL PUSH QUEUE
 *     --> tested in stack.rexx
 *
 */

written = 0

/* ============================ GENERAL ============================= */
call notify 'general'

/*----- clauses have precedens before commands ---------------------- */
bar = ''
call doit true 1 'foo'
signal aftertrue

doit:
   rc = -1000
   bar = 'none'
   one = 1
   arg(one) bar
   if (rc^==-1000) then
      call complain "commands have presedens before keywords"
   if (bar^==' FOO') then
      call complain 'ARG didnt work properly wrt the spaces'
   return

aftertrue:

/*----- keywords does not need spaces around them --------------------*/
foo = nospace()
if foo ^== 4 then
   call complain 'Something very wrong with parsing of return'

signal afternospace

nospace:
   nop;return(4)
   call complain "The return clause was not found"

afternospace:




/* ===== ADDRESS ============= */
call notify 'address'

/*----- address with command should not change the env. --------------*/
   address system
   if (address() ^== 'SYSTEM') then
      call complain 'Could not set envionment to system'
   address path 'true'
   if address() ^== 'SYSTEM' then
      call complain 'ADDRESS with command changed default environment'


/*----- Just address toggles environments ----*/

   address command
   if address() ^== 'COMMAND' then
      call complain 'ADDRESS could not set environment to COMMAND'

   address path
   address system
   address
   if (address() ^== 'PATH') then
      call complain 'Just ADDRESS does not toggle environments'


/*----- env names is stored over subroutines -------------------------*/
   address command
   address system
   call checkaddr
   if (address() ^== 'SYSTEM') then
      call complain 'Env. not stored over function calls'

   address
   if (address() ^== 'COMMAND') then
      call complain 'Extra env. name not stored over function calls'

   signal afterchk

checkaddr:
   address command
   return

afterchk:


/*----- does value work at all? -------------------------------------*/

   env = 'SYSTEM'
   address command
   address value env
   if address()^=='SYSTEM' then
      call complain 'Env does not seem to work with VALUE'


/*----- can value be defaulted --------------------------------------*/

   env = 'COMMAND'
   address system
   address (env)
   if address()^== 'COMMAND' then
      call complain 'Env can not be ommitted when it is default'

   address(env)
   if address()^== 'COMMAND' then
      call complain 'Env can not be ommitted when it is default without space'


/*----- the environment name should not need space around it --------*/
signal after_path

path: return 1

after_path:
   rc = -1000
   address path('true')
   if address()=='1' then
      call complain 'env lower priority than value'
   if rc ^== 0 then
      call complain 'Something is very wrong with ADDRESS'

/*----- can we fool it with no space after VALUE? --------------------*/

   env = 'COMMAND'
   address system
   address value(env)
   if (address() ^== 'COMMAND') then
      call complain 'Dont differ between VALUE and value()'

   signal aftervalue

value:
   return 'foobar'

aftervalue:



/* ============================== ARG =============================== */
call notify 'arg'

/*----- can be called without template? ------------------------------*/
   arg



/* ======================= CALL =============================== */
call notify 'call'

/*----- Is RESULT set correctly? -=======-----------------------------*/
result = 'foobar'
call center '*', 7
if result^=='   *   ' then
   call complain 'RESULT not set on CALL to routines that return result'


/*----- Is RESULT dropped when not set? ------------------------------*/
result = 'foobar'
call noret
if symbol('RESULT')^=='LIT' then
   call complain 'RESULT not dropped on CALL that does not return result'

signal after_noret

noret:
   return

after_noret:


/*----- Params to call evaluate from left to right -------------------*/
call incr 1
foo = echoargs( incr(), incr(), incr() incr() )
if foo ^== '2 3 4 5' then
   call complain "Parameters to CALL not evaluate from left to right."


/*----- Empty parameters can be created with extra commas ------------*/
call num_params 'one',, 'three',,, 'six'
signal after_num_params

num_params:
   if arg(1)^=='one' | arg(3)^=='three' | arg(6)^=='six' then
      call complain 'Parameters not correctly set when some are empth'

   if arg(2)^=='' | arg(4)^=='' | arg(5)^=='' | arg(7)^=='' then
      call complain 'Empty parameters in CALL not interpreted as empty'

   return

after_num_params:


/*----- SIGL is set when using CALL ----------------------------------*/
sigl = 'foobar'
call check_sigl
if sigl=='foobar' then
   call complain 'SIGL was set within the routine, not at an outer level'

signal after_check_sigl

check_sigl: procedure
   if sigl^=='SIGL' then
      call complain 'SIGL set within routine where it should be unset.'
   return

after_check_sigl:


/*----- SIGNAL in subroutine does not effect DO-loops in caller ------*/
/*      Same goes for ITERATE and LEAVE                               */
foo = 'one'
do i = 1 to 1
   call loop_routine
   foo = 'two'
   end

if foo^=='two' then
   call complain 'DO loop in caller effected by statements in subroutine'

signal after_loop_routine

loop_routine:
   signal loop_routine_2
   call complain 'Something is very wrong with SIGNAL'

   loop_routine_2:
   signal on syntax name loop_routine_3
   iterate
   call complain 'Something is wrong with plain ITERATE'
   loop_routine_3:
   signal on syntax name loop_routine_4
   iterate i
   call complain 'Something is wrong with symbolic ITERATE'

   loop_routine_4:
   signal on syntax name loop_routine_5
   leave
   call complain 'Something is wrong with plain LEAVE'
   loop_routine_5:
   signal on syntax name loop_routine_6
   leave i
   call complain 'Something is wrong with symbolic LEAVE'
   loop_routine_6:
   return

after_loop_routine:



/*----- NUMERIC settings are stored across subroutines --------------*/
numeric digits 8
numeric fuzz 1
numeric form engineering
call set_numeric

if (fuzz()\=1) then
   call complain 'NUMERIC FUZZ not saved across subroutines'
if (form()\='ENGINEERING') then
   call complain 'NUMERIC FORM not saved across subroutines'
if (digits()\=8) then
   call complain 'NUMERIC DIGITS not saved across subroutines'

numeric digits 8
numeric fuzz 1
numeric form engineering
junk = set_numeric

if (fuzz()\=1) then
   call complain 'NUMERIC FUZZ not saved across subroutines'
if (form()\='ENGINEERING') then
   call complain 'NUMERIC FORM not saved across subroutines'
if (digits()\=8) then
   call complain 'NUMERIC DIGITS not saved across subroutines'

numeric digits 9
numeric fuzz 0
numeric form scientific
signal after_numeric

set_numeric:
   numeric digits 9
   numeric fuzz 0
   numeric form scientific
   return ''

after_numeric:


/*----- ADDRESS setting are stored across subroutines ----------------*/
address Unix
address Command
call set_address_1

if (address()\='COMMAND') then
   call complain 'ADDRESS setting not stored across subroutines'
address
if (address()\='UNIX') then
   call complain 'ADDRESS setting not stored across subroutines'

signal after_address_1

set_address_1:
   address
   return

after_address_1:
address Unix
address Command
junk =  set_address_2()

if (address()\='COMMAND') then
   call complain 'ADDRESS setting not stored across subroutines'
address
if (address()\='UNIX') then
   call complain 'ADDRESS setting not stored across subroutines'

signal after_address_2

set_address_2:
   address system
   return ""

after_address_2:

/*----- Condition traps/information are stored across subroutines ----*/
tag = 0
signal on syntax name addr_1
call change_it
tag = 1
interpret 'a = b = c = d'
signal after_address_3

change_it:
   signal on syntax name addr_2
   return

addr_1:
   tag = 2
   signal after_address_3

addr_2:
   if tag=0 then
      call complain "SYNTAX handler called out of line"
   else if tag = 1 then
      call complain "Condition setting not restored after CALL"

after_address_3:

cond_info = condition('c') condition('i') condition('d') condition('s')
call change_it_2
cond_ifo2 = condition('c') condition('i') condition('d') condition('s')
if cond_info\==cond_ifo2 then
   call complain 'Condition information not properly saved during CALL'

signal after_address_4

change_it_2:
   signal on syntax name ignore_addr
   interpret 'a==b'
   call complain "Didn't manage to trigger a SYNTAX!"
   return

ignore_addr:
   return

after_address_4:

/*----- Check that the elapse time close is saved --------------------*/

call time 'r'
foo = copies('foo ', 200)
do 100
   call words(foo)
   end

recorded = time('e')
call check_time
new = time('e')
if recorded > new then
   call complain 'The elapse time clock seems to have be reset during CALL'

call time 'r'
new = time('e')
if (new > recorded) then
   call complain 'The elapse time clock doesn''t seem to operate correctly'

signal after_address_5

check_time:
   call time 'r'
   return

after_address_5:


/*----- Trace setting saved across CALL -------------------------------*/

normal = 'Normal'
trace normal

call change_trace

if trace()\=='N' then
   call complain 'The trace setting was not saved across CALL'

signal after_address_6

change_trace:
   label = 'Label'
   trace label
   return

after_address_6:


/*
 * Note: tests for limits (number of arguments and depth of nesting
 * is performed in limits.rexx
 */


/* ================================================================== */
/* ============================== DO ================================ */
/* ================================================================== */

call notify 'do-loops'
/*----- First, try a rather simple do-loop ---------------------------*/
signal = ""
do = 4
end = 5
then = 1
forever = 6
iterate = 1
do for = forever to do + end by then while iterate
   signal = signal for
   end

/*----- Check value of control variable ------------------------------*/
if for ^= 10 then
   call complain 'Control variable have incorrect value on exit'

/*----- Did it loop correct? -----------------------------------------*/
if signal^==' 6 7 8 9' then
   call complain 'Did not perform the loop correct'


j = 1
do i=-234 to 234 for 500
  j = j + 1
  end

if j\=470 | i\=235 then
   call complain "Loop starting on negative number fails"


j = 1
do i=-23.4 to 23.5 for 50
   j = j + 1
   end

if j\=48 | i\=23.6 then
   call complain "Fractal loop starting on negative number fails"


/*----- Can we change the value of the ctrl var? ---------------------*/
array = ''
do i=1 to 10
   array = array i
   if i=3 then
      i = 8
   end

if array\==' 1 2 3 9 10' then
   call complain 'Changing the ctrl var in loop didn''t work'

/*----- Can we drop and set the control variable ---------------------*/
result = ''
do i = 1 to 10
   result = result i
   j = i
   drop i
   i = j + 2
   end
if result ^== ' 1 4 7 10' then
   call complain 'Cannot drop and set control variable during looping'


/*----- don't loop if end condition is fullfilled initially ----------*/
do i=10 to 9
   call complain 'Ran loop, even when end condition was fullfilled initially'
   leave
   end



/*----- Is phrases calculated in correct order? ----------------------*/
str = ''
do i=1 by func(1) for func(0) to func(2)
   call complain 'Does not handle for-phrase correct'
   end

if str ^==' 1 0 2' then
   call complain 'Incorrect evaluation order for phrases in do-loops'


str = ''
do i=func(1) to func(4) by func(2) for func(5)
   nop
   end

if str^==' 1 4 2 5' then
   call complain 'Expressions in DO-loops are re-evaluated'


signal afterfunc
func:
   str = str arg(1)
   return arg(1)
afterfunc:


/*----- control var should be set before any other evaluation --------*/
i = 1
array = ''
do i = fff(2) to fff(4)
   leave
   end

if array \== ' 1 2' then
   call complain 'Ctrl var in DO loop assigned incorrectly'
signal after_do_9

fff:
   array = array i
   i = arg(1)
   return arg(1)+1

after_do_9:


/*----- Check value of i on first invocation -------------------------*/
do i='-01' to 1 until 1
   if (i \== '-1') then
      call complain 'loop variable not normalized on first iteration'
   end


/*----- Can we use while (and until) as control variable? ------------*/
do while = 1 for 0
   end

if while ^== '1' then
   call complain 'Could not use while as control variable'


/*----- Can we manipulate the control variable? ---------------------*/
j = 0
do i=1 to 10
   if i=4 then i=7
   j = j + 1
   end

if j^==7 then
   call complain 'Can not change value of control variable'


/*----- Can we change to a new control variable during looping? -----*/
i = 0
j = 0
i. = 0
do i.i=i.i to 10
   i = (i+1)//10
   j = j + 1
   end

if j^==101 then
   call complain 'Can not swap control variable during looping'

count = 4
do i=1 to 10 while count>0
   count = count - 1
   end

if i^=='5' then
   call complain 'Incorrect sequence in processing DO-loops (while)'


count = 4
do i=1 to 10 until count==0
   count = count - 1
   end

if i^=='4' then
   call complain 'Incorrect sequence in processing DO-loops (until)'


do i=1 to 10 for 4
   nop
   end

if i^=='5' then
   call complain 'Incorrect sequence in processing DO-loops (for)'


signal on syntax name there_do_1
times = '-2'
do times
   call complain 'Allowed negative number for exprr in DO'
   signal there_do_1
   end

there_do_1:
signal on syntax name there_do_2
do i=1 to 10 for times
   call complain 'Allowed negative number for exprf in DO'
   signal there_do_2
   end

there_do_2:


count = 1
do i=1 to 10 until 1
   count = count + 1
   end

if count\==2 then
   call complain 'Loop never executed in an UNTIL loop'


/*----- WHILE resevered within an UNTIL, and vice versa --------------*/
signal on syntax name there_do_3
until='foo'
interpret "do forever while until='foo';",
          "   call complain 'Allowed UNTIL within a WHILE in DO';",
          "   leave; end"

there_do_3:
signal on syntax name there_do_4
while = 'foo'
interpret "do forever until while\='foo';",
          "   call complain 'Allowed WHILE within an UNTIL in DO';",
          "   leave; end;"

there_do_4:


/*----- TO FOR and BY reserved only in the first part ----------------*/
signal on syntax name there_do_5
to = 1
for = 2
by = 3
do i=1 while to + for + by > 0
   signal after_do_5
   end

there_do_5:
   call complain "TO FOR and/or BY unnecessarily reserved in DO"
after_do_5:



/*----- a symbol can follow END --------------------------------------*/
a = 1
signal on syntax name check_end_1
string = 'do i=1 to 4 ; a=a+1 ; end i'
interpret string

check_end_1:
if a\=5 then
   call complain 'Didn''t allow symbol after ''END'' '

signal on syntax name check_end_2
string = 'do i=1 to 4 ; a=a+1 ; end j'
interpret string

call complain "Didn't catch incorrect symbol after END"
check_end_2:
signal on syntax name check_end_3
sigl = ''
j = 1
a=5 /* just to be sure */
i.2 = 3
string = 'do i.j=1 to 4; a=a+1; if i.j=2 then j=2; end i.j'
interpret string

check_end_3:
if sigl\='' then
   call complain "Didn't like compound symbol after END"
else if a\=8 then
   call complain "Didn't iterate correct number of times"


signal on syntax name check_end_4
sigl = ''
a = 8
j = 1
i.2 = 3
string = 'do i.j=1 to 4; a=a+1; end i.1'
interpret string

check_end_4:
if sigl=='' then
   call complain "Allowed substitution in compound symbol after END"
else if a\=8 then
   call complain "Didn't iterate correct number of times"



/*----- some extra testing -------------------------------------------*/
tag = 0
signal on syntax name do_while
do while=5
   tag = tag + 1
   leave
   end

if tag=0 then
   call complain "WHILE incorrectly allowed in exprr"

signal after_while_1

do_while:
   call complain "WHILE not allowed as a name (correct?)"

after_while_1:
signal off syntax



/* ============================= DROP =============================== */
call notify 'drop'

foo = 1
foo. = 2
bar.bar = 3
drop foo foo. bar.bar

if symbol('foo')\=='LIT' then
   call complain 'DROP didn''t work with simple variables'

if symbol('foo.')\=='LIT' then
   call complain 'DROP didn''t work with stem variables'

if symbol('foo.123')\=='LIT' then
   call complain 'DROP didn''t work with compound variables'

if symbol('bar.bar')\=='LIT' then
   call complain 'DROP didn''t work with compound variables'

aaa = 1
bbb = 2
ccc = 3
ddd = 4
vars = '  aaa bbb  ccc   ddd'
drop foo (vars) bar
if symbol('aaa')symbol('aaa')symbol('aaa')symbol('aaa')\=='LITLITLITLIT' then
   call complain 'Didn''t manage to drop variables indirectly'

if symbol('vars')\=='VAR' then
   call complain 'Dropped indirect variable as well, should not'


bar = 'asdf'
foo = 'asdf'
vars1 = 'foo'
vars2 = 'bar'
drop vars1 (vars2) (vars1) vars2
if symbol('foo')\=='VAR' | symbol('bar')\=='LIT' then
   call complain 'Variables not dropped strictly in order from left to right'

foo = 'asdf'
drop bar
drop bar bar foo foo


foo = 'bar'
call test_drop
signal after_drop_1

test_drop:
   procedure expose foo
   drop foo
   return

if symbol('foo')\=='LIT' then
   call complain 'EXPOSED variable not dropped in caller'

after_drop_1:


/*----- drop on a stem drops all its compounds -----------------------*/
bar = 'BAR'
foo.bar = 'barf'
drop foo.
if foo.bar == 'barf' then
   call complain 'Drop on stem does not drop compounds of that stem'
else if foo.bar ^== 'FOO.BAR' then
   call complain 'Something is very wrong about dropping of stems'



/* ============================= EXIT =============================== */


/* ================================================================== */
/* ============================== IF ================================ */
/* ================================================================== */
call notify 'if'

/*----- the ';' can not replace a statement after 'then' ------------ */
do i=0 to 1
   str = ''
   if i then ;
      str = 'hepp'

   if str=='hepp' && i  then
      call complain 'Null-clause in THEN-part of IF-statement'

   str = ''
   if i then
      a=b
   else ;
      str = hepp

   if str=='' && i then
      call complain 'Null-clause in ELSE-part of IF-statement'

   str = ''
   if i then nop
      str = 'hepp'

   if str^=='hepp' then
      call complain 'NOP is ignored in IF statement'

   str = ''
   if i then
      a=b
   else
      nop
      str = 'hepp'

   if str^=='hepp' then
      call complain 'NOP is ignored in IF statement'
end


/*----- does 'else' bind correctly? --------------------------------- */
if 0 then
   if 1 then
      nop
   else
      call complain "ELSE does not bind correctly in IF-statements"
nop


/*----- can then be used as variable in extr -------------------------*/
then = '2'
if '1 2' = 1 then
   call complain "then may be used in IF-expr as a variable"

/*----- whitespace handling in non-exact comparisons -----------------*/
a = '0a'x
b = ' '
if a \= b then
   call complain "non-exact comparison doesn't treat whitespace correctly"
OPTIONS STRICT_WHITE_SPACE_COMPARISONS
if a = b then
   call complain "non-exact comparison doesn't treat whitespace correctly"
OPTIONS NOSTRICT_WHITE_SPACE_COMPARISONS


/* ===================== INTERPRET ============================ */

interpret 'interpret "a=1";a=a+1'
if a\=2 then
   call complain "couldn't nest INTERPRET"

/* ================================================================== */
/* ============================ ITERATE ============================= */
/* ================================================================== */
call notify 'iterate'

/*----- does iterate work at all ? -------------------------------------*/
k = 0
do i=1 to 20
   if i>7 then
      iterate
   k = k + 1
   end

if k^=='7' then
   call complain 'ITERATE does not work correctly'


/*----- does it iterate the right level --------------------------------*/
k=0
do l=1 to 2
   do i=1 to 10
      do j=1 to 10
         if (i>4) & (j>5) then
            iterate i
         k = k + 1
      end
   end
end

if k^=='140' then
   call complain 'ITERATE "var" does not leave right level'


/*----- can we fool the interpreter to iterate another loop -----------*/

k = 0
i = 1
j = 2
do i.i=1 to 10
   temp = i.i
   do i.j=1 to 10
      if (i.i>3) & (i.j>6) then do
         i.i = temp
         i = 2
         iterate i.i
         end
      k = k + 1
      end
   i.i = temp
   end

if k^==54 then
   call complain 'Substitution suspected in ITERATE variable'


/*----- can we terminate inactive loop ------------------------------*/
/*
 * k = 0
 * do i=1 to 10
 *    if i=3 then
 *       interpret 'iterate'
 *    k = k + 1
 *    end
 *
 * if i^==11 then
 *    call complain 'INTERPRET ITERATE can terminate inactive loop'
 */


/*----- if two loops use same control variable -----------------------*/

l = 0
do i=1 to 10
   k = i
   do i=1 to 10
      l = l + 1
      if k>6 then
         iterate i
      end
   i = k
   end

if l^==100 then
   call complain "ITERATE doesn't leave innermost loop when identical ctrlvars"


/*----- can we fool iterate with a non-repetitive loop ----------------*/
k = ''
do i=1 to 3
   if i=2 then do
      nop
      iterate
      end
   k = k i
   end

if k^==' 1 3' then
   call complain 'ITERATE is fooled by a non-repetitive loop'





/* ================================================================== */
/* ============================= LEAVE ============================== */
/* ================================================================== */
call notify 'leave'

/*----- does leave work at all ? -------------------------------------*/
do i=0 to 20
   if i==7 then
      leave
   end

if i^=='7' then
   call complain 'LEAVE does not work correctly'


/*----- does it leave the right level --------------------------------*/
k=0
do l=1 to 2
   do i=1 to 10
      do j=1 to 10
         if (i>4) & (j>5) then
            leave i
         k = k + 1
      end
   end
end

if k^=='90' then
   call complain 'LEAVE "var" does not leave right level'


/*----- can we fool the interpreter to leave another loop -----------*/
k = 0
i = 1
j = 2
do i.i=1 to 10
   do i.j=1 to 10
      k = k + 1
      if (i.i=3) & (i.j=6) then do
         i = 2
         leave i.i
         end
   end
end

if k^==26 then
   call complain 'Substitution suspected in LEAVE variable'


/*----- can we terminate inactive loop ------------------------------*/
/*
 * do i=1 to 10
 *    if i=3 then
 *       interpret leave
 *    end
 *
 * if i^==11 then
 *    call complain 'INTERPRET LEAVE can terminate inactive loop'
 */


/*----- if two loops use same control variable -----------------------*/
l = 0
do i=1 to 10
   k = i
   do i=1 to 10
      if i=6 then
         leave i
      l = l + 1
      end
   i = k
   end

if l^==50 then
   call complain 'LEAVE does not leave innermost loop when identical ctrlvars'


/*----- can we fool leave with a non-repetitive loop ----------------*/
do i=1 to 10
   if i=4 then do
      nop
      leave
      nop
      end
   end

if i^=='4' then
   call complain 'LEAVE is fooled by a non-repetitive loop'



/* ======================== NOP =============================== */
/* ====================== NUMERIC ============================= */
/* ====================== OPTIONS ============================= */


/* ================================================================== */
/* ============================= PARSE ============================== */
/* ================================================================== */
call notify 'parsing'

string1 = 'Det er sikkert mange utlendinger som ikke kan lese dette'

/*----- Test if they work correctly ----------------------------------*/
parse var string1 before 'sikkert' between 9 .
if (before^=='Det er ') then
   call complain "litteral patterns can not overlap positional patterns"

parse var string1 -10 all 100
if (all^==string1) then
   call complain "confused when offsets are outside string boundaries"

parse value 'hello' with 2 foo +3
if (foo\=='ell') then
   call complain "confused by absolute and relative patterns"

parse var string1 . 'sikkert' word 'utlendinger' .
if (word^==' mange ') then
   call complain "does not handle spaces between words correctly"

parse var string1 . 'sikkert' word . 'utlendinger' .
if (word^=='mange') then
   call complain "does not handle spaces between words correctly"


/*----- can assignment to one var be assigned later in same templ. ---*/
parse var string1 . 'si' tmp 'ert' . (tmp) testing +7 .
if testing^=='kke kan' then
   call complain 'Variable assigned in a PARSE can not be references later'

parse var string1 . 'si' tmp 'ert' . (tmp) +0 testing +5 .
if testing^=='kke k' then
   call complain 'The pattern +0 does not work in PARSE'

parse value "hepp" with 1 var1 1 var2 1 var3 1
if (var1^=="hepp" | var2^=="hepp" | var3^=="hepp") then
   call complain "can't handle multiple assignement"

parse value "hest" with var1 "s" -1 var2 +2 -1 var3
if (var1^=="he" | var2^=="es" | var3^=="st") then
   call complain "Something is wrong with relative offsets"


/*----- Stems can be set during parse --------------------------------*/
foo.bar = 'hallo'
foo = 'nope'
parse value 'hepp' with foo. .

if foo. ^== 'hepp' then
   call complain 'Parse did not manage to set stem variable.'


/*----- Nullstrings matches End-Of-String ----------------------------*/
foo = "Foo bar"
parse var foo pre '' post
if (foo^=='Foo bar') | (post^=='') then
   call complain 'Nullstrings as patterns do not match end-of-string'


/*----- Variable positional patterns ---------------------------------*/
a = 3
b = 4
c = 5
foo = 'foo bar baz'
parse var foo x1 =(a) x2 +(b) x3 -(c) x4
if x1\=='fo' then
   call complain 'Variable positional pattern = does not work correctly'

if x2\=='o ba' then
   call complain 'Variable positional patterns = or + do not work'

if x3\=='r baz' then
   call complain 'Variable positional pattern - does not work'

if x4\=='oo bar baz' then
   call complain 'Variable positional pattern - does still not work'


/* ================================================================== */
/* ============================ PROCDURE ============================ */
/* ================================================================== */
call notify 'procecure'

/*--------------------------------------------------------------------*/
/* Test ability to properly expose variables                          */
/*--------------------------------------------------------------------*/

/*---- check a few things --------------------------------------------*/

a = 'hello'
bar.1 = 'oops'
foo.1 = 'yupp'
test.2 = 'hepp'
test.X.1 = 'foobar'
foo.b = 'nope'
b = 1

call checkit

signal aftercheckit


checkit: procedure expose bar.b b foo.b test. test.1 test.b,
                          test. test.X.B bbb.123 bbb.

   if (foo.1 ^= 'yupp') then
      call complain "Don't substitute indexes during expose"

   if (test.X.1 ^== "foobar") then
      call complain "Don't handle last in a list of exposees"

   if (bar.1 ^== 'BAR.1') then
      call complain "substitutes too much in indexes during expose"

   if (test.2 ^== 'hepp') then
      call complain 'Expose of stems does not expose all compounds'

   test.1 = 'foo'
   test.2 = 'bar'
   bbb. = 'baz'
return

aftercheckit:

   if (test.1 \=='foo') then
      call complain "Didn't properly handle explicit expose for compounds"

   if (test.2 \== 'bar') then
      call complain "Didn't properly handle implicit expose for compounds"

   if bbb.123 \== 'baz' then
      call complain "Didn't properly handle expose of stems"

   if bbb.1 \== 'baz' then
      call complain "Didn't handle var exposed as both stem and compound"

/* ====================== RETURN ============================== */
/* ======================== SAY =============================== */


/* ================================================================== */
/* ============================ SELECT ============================== */
/* ================================================================== */
call notify 'select'

/*----- does it calculate in correct order? --------------------------*/
/*----- last when and the otherwise should be skipped, and -----------*/
/*----- string should have a particular order ------------------------*/
str = ''
select
   when func(0)=2 then str=asdf
   when func(1)=2 then str=asdf
   when func(2)=2 then str=str 'hepp'
   when func(3)=2 then str=asdf
   otherwise
      str=asdf
   end

if str^==' 0 1 2 hepp' then
   call complain 'SELECT does not operate correctly'

/*----- Can we do without an otherwise? ----------------------------*/
select
   when 0 then nop
   when 1 then nop
   when 0 then nop
   end  /* this should not result in a syntax error */


/*----- A semicolon should not be interpreted as nop after then -----*/
str = ''
select
   when 1 then ;
     str = 'hepp'
   when 1 then nop
   end

if str^=='hepp' then
   call complain 'Semi after then in select interpreted as statement'



/* ====================== SIGNAL ============================== */
call notify 'signal'
   signal next_sig
   call complain 'simple SIGNAL statement failed'

   next_sig:
   signal off novalue
   signal value nextsig2
   call complain 'SIGNAL statement with VALUE did not work'

   nextsig2:
   with = 'NEXT_SIG_3'
   signal value with     /* hehe ... */
   call complain 'WITH has special meaning in SIGNAL VALUE'

   next_sig_3:



/* ======================= TRACE ============================== */
call notify 'trace'

/* Only first character counts in trace settings */
trace off
trace offset
trace on
trace o
trace oooooooooooooooooooooooooooooooooooooooooooo


   say ' '
exit 0


echoargs:
   do i=1 to arg()
      if i=1 then
         result = arg(1)
      else
         result = result arg(i)
      end
   return result


incr:
   if arg()>0 then
      increment = arg(1)
   else
      increment = increment + 1
   return increment




ch: procedure expose sigl
   parse arg first, second
   if first ^== second then do
      say
      say "first= /"first"/"
      say "second=/"second"/"
      say "FuncTrip: error in " sigl":" sourceline(sigl) ; end
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

complain:
   say ' ...'
   say 'Tripped in line' sigl':' arg(1)'.'
   length = charout(,'   (')
return



