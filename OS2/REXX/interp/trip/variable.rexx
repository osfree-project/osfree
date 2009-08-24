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

call notify 'drop'
   foo = 'bar'
   drop foo

   if foo^=='FOO' then
      call complain 'Plain DROP does not work'


/* === is a stem-symbol's value different from the value of the 
       compound-symbol that has the same stem and a null tail? ====== */

   foo. = 'here'
   bar = ""
   foo.bar = 'there'

   if (foo.=='there') then
      call complain 'Stem symbol set when compound with null tail is set'


/* === is assignment to a stem destructive for *all* 
       compound variables with that stem? ========================= */

   foo.bar = 'here'
   foo. = 'there'
   if (foo.bar^=='there') then
      call complain 'Old compound symbols not destroyed when stem set'


/* === default value of simple variables === */

drop bar foo.bar foo foobar

if foobar^=='FOOBAR' then
   call complain 'Simple variables have incorrect default value'

if foo.bar ^=='FOO.BAR' then
   call complain 'Something is wrong with the dropping of variables.'

drop foo.
bar = 'hepp'
if foo.bar == 'FOO.HEPP' then
   call complain 'Tail part of uninitialized compound vars are upcased.'

else if foo.bar ^== 'FOO.hepp' then
   call complain 'Compound vars with lower case tails have wrong def value.'



/* === HEAD.tail is different from HEAD.TAIL === */
upper = 'TAIL'
lower = 'tail'
space = ' tail '

head.upper = 'upper'
head.lower = 'lower'
head.space = 'space'

if head.lower == head.upper then
   call complain "Compound vars don't differ between upper and lower case."

if head.lower == head.space then
   call complain 'Tail of compound var is spaced before interpretation.'


/* === stems and collections only on first level === */
drop foo. bar barf 
foo.bar. = 'hepp'
if foo.bar.barf == 'hepp' then
   call complain 'Stems seems to multidimentional'


/* === nothing magic about dots === */
foo.1.2 = hei
drop foo. bar barf baz
bar = 1
baz = 2
barf = 1.2

foo.barf = 'hepp'
if foo.bar.baz ^== 'hepp' then
   call complain 'Multidimentional compound must be retrieved in same dim.'

barf = 2.1
foo.baz.bar = 'hopp'
if foo.barf ^== 'hopp' then
   call complain 'Multidim. compounds must be retrived in same dimention.'


/* === tails can contain any char === */
/* 
 * Sigh, this does not check that all the chacters are actually 
 * used, since (let's say) one of the characters might be deleted 
 * from the tail. 
 */
bar = xrange("00"x, "23"x)
drop foo.
foo.bar = 'hepp'
if (foo.bar ^== 'hepp') then
   call complain "Tail in compound vars don't work with stange chars"

/* 
 * But at least specifically check for the null character, since some
 * programmers might be attempted to use that as string terminator
 */
drop foo.
bar = 'hepp' || "00"x || 'hopp'
baz = 'hepp' || "00"x || 'hipp'
foo.bar = 'first'
foo.baz = 'second'

if (foo.bar == foo.baz) then
   call complain 'Nul char seems to terminate name string'


/* === 'reserved' words can be used in assignments === */
/* 
 * To get a better testing of this, it might be best to run the 
 * program block.rexx, which use 'reserved' words rather extensively
 */
address = 1       ; value = 2     ; arg = 3       ; call = 4
on = 5            ; off = 6       ; name = 7      ; do = 8
end = 9           ; to = 10       ; by = 11       ; for = 12
forever = 13      ; while = 14    ; until = 15    ; drop = 16
exit = 17         ; if = 18       ; then = 19     ; else = 20
interpret = 21    ; iterate = 22  ; leave = 23    ; nop = 24
numeric = 25      ; digits = 26   ; form = 27     ; scientific = 28
engineering = 29  ; fuzz = 30     ; options = 31  ; parse = 32
upper = 33        ; linein = 34   ; pull = 35     ; source = 36 
with = 37         ; var = 38      ; version = 39  ; procedure = 40
expose = 41       ; push = 42     ; queue = 43    ; return = 44
say = 45          ; select = 46   ; when = 47     ; otherwise = 48
signal = 49       ; trace = 50

all = ' 'address value arg call on off name do end to by for forever,
         while until drop exit if then else interpret iterate leave,
         nop numeric digits form scientific engineering fuzz options,
         parse upper linein pull source with var version procedure,
         expose push queue return say select when otherwise signal,
         trace

sum = '' 
do i=1 to 50
   sum = sum i
   end

if sum ^== all then
   call complain 'Using "reserved" words as var names does not work'



/* === when stem is set, all compound with that stem is dropped === */
drop foo. bar baz barf
foo.bar = 'hepp'
foo. = 'hopp'
if (foo.bar == 'hepp') then
   call complain 'Compound not dropped when stem was set'
else if foo.bar ^== 'hopp' then
   call complain 'Compound has wrong value after stem was set'



/* Check that drop works correctly */

   drop foo. bar
   foo. = 'first'
   bar = 'baz'
   foo.bar = 'second'

   drop bar
   if (foo.bar ^== 'first') then
      call complain 'Dropped simple var is not properly dropped'



/*----- more checking of drop ----------------------------------------*/

   vars = 'baz foo.bar foo'
   drop foo (vars) barf (vars) vars

   if foo barf foo.bar baz vars ^== 'FOO BARF FOO.BAR BAZ VARS' then
      call complain 'indirect drop does not seem to work'



/*----- */

   foo = ''
   call recursive 4, 1, 2, 3
 
   facit = ' 4 1 1 2 2 3 3 1 1 2 2 3 3 -2 -2 -4 -4 -6 -6 4 -2 -2 -4 -4 -6 -6',
            '3 -2 -2 -4 -4 -6 -6 -2 -2 -4 -4 -6 -6 4 4 8 8 12 12',
            '3 4 4 8 8 12 12 2 4 4 8 8 12 12 4 4 8 8 12 12 -8 -8 -16 -16',
            '-24 -24 2 -8 -8 -16 -16 -24 -24 1 -8 -8 -16 -16 -24 -24 -8 -8',
            '-16 -16 -24 -24 16 16 32 32 48 48 1 16 16 32 32 48 48'

   if foo\==facit then 
      call complain 'Recurive treatment of variables did not work'
 
   signal after_recursive

recursive: procedure expose foo
   parse arg num, st, st., ts.1
   if num=0 then return

   foo = foo num st value('st') st. value('st.') ts.1 value('ts.1')
   call check_rec 
   foo = foo num st value('st') st. value('st.') ts.1 value('ts.1')

   call recursive num-1, st, st., ts.1
   return 

check_rec: procedure expose foo st st. ts.1
   foo = foo st value('st') st. value('st.') ts.1 value('ts.1')
   st = st * -2
   st. = st. * -2
   ts.1 = ts.1 * -2
   foo = foo st value('st') st. value('st.') ts.1 value('ts.1')
   return

after_recursive:


   say ' '
exit 0


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


 



