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

call notify 'args'
/*----- At least 10 arguments to functions and procedures -------------*/
   call ch echoargs( 1,2,3,4,5,6,7,8,9,0 ), '1 2 3 4 5 6 7 8 9 0'


call notify 'levels'
/*
 * Note: the limit 100 refers to the nesting of control structures, 
 * which _includes_ function calls. Consequently, other things, like 
 * loops and SELECTs may also count. Thus, make sure this is outside
 * any such loops. Maybe even external functions count (sigh!)
 */
tag = 1
call nesting
signal after_nesting

nesting:
   if tag = 100 then 
      return

   tag = tag + 1
   call nesting
   return

after_nesting:


call notify 'comments'
/*----- At least 10 levels of nesting in comments ---------------------*/
/*1 /*2 /*3 /*4 /*5 /*6 /*7 /*8 /*9 /*  10  */ */ */ */ */ */ */ */ */ */


call notify 'strings'
/*----- At least 100 in source code -----------------------------------*/
string = '.........1.........2.........3.........4.........5.........6.........7.........8.........9.........!'

if length(string)^=100 then
   call complain 'String seems to have been truncated in src code'

d = '.........'
s = d'1'd'2'd'3'd'4'd'5'd'6'd'7'd'8'd'9'd'!'
if s^==string then
   call complain 'Something is wrong with large strings in src code'


call notify 'hexstrings'
/*----- At least 100 chars in resulting string ------------------------*/
istring = 'hstring = "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f',
                     '10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f',
                     '20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f',
                     '30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f',
                     '40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f',
                     '50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f',
                     '60 61 62 63"X'

signal on syntax name syntax_hex_1

interpret istring

syntax_hex_1:
if hstring^==xrange('00'x, '63'x) then
   call complain 'Hexstring seems to mismatch real value'

if length(hstring)^=100 then
   call complain 'Hexstring seems to have been truncated' 


istring = 'bstring = "00000000 00000001 00000010 00000011',
                     '00000100 00000101 00000110 00000111',
                     '00001000 00001001 00001010 00001011',
                     '00001100 00001101 00001110 00001111',
                     '00010000 00010001 00010010 00010011',
                     '00010100 00010101 00010110 00010111',
                     '00011000 00011001 00011010 00011011',
                     '00011100 00011101 00011110 00011111',
                     '00100000 00100001 00100010 00100011',
                     '00100100 00100101 00100110 00100111',
                     '00101000 00101001 00101010 00101011',
                     '00101100 00101101 00101110 00101111',
                     '00110000 00110001"b'
signal on syntax name syntax_bin_1
interpret istring

syntax_bin_1:
if bstring^==xrange('00'x, '31'x) then
   call complain 'Binstring seems to mismatch read value'

if length(bstring)^==50 then
   call complain 'Binstring seems to have been truncated'



/*--------------------------------------------------------------------*/
/* Let's check some of the inplementation minimums                    */
/*--------------------------------------------------------------------*/
call notify 'symbols'

/*----- The Standard requires 50 significant chars in variablenames --*/
FiftyCharactersInVariableNamesAreAnAbsoluteMinimum = 'foo'
FiftyCharactersInVariableNamesAreAnAbsoluteMinimun = 'bar'
if FiftyCharactersInVariableNamesAreAnAbsoluteMinimum = ,
   FiftyCharactersInVariableNamesAreAnAbsoluteMinimun then
   call complain 'Less than 50 significant chars in symbolnames'

/*----- And in compound symbols too ----------------------------------*/
drop characters in variable names is a minimum ! ?
Fifty.Characters.In.Variable.Names.Is.A.Minimum.?! = 'foo'
Fifty.Characters.In.Variable.Names.Is.A.Minimum.?? = 'bar'
if Fifty.Characters.In.Variable.Names.Is.A.Minimum.?? = ,
   Fifty.Characters.In.Variable.Names.Is.A.Minimum.?! then
      call complain 'Less than 50 significant chars in compound symbols'
/*----- And also in numbers ------------------------------------------*/
if 0000000000000000000000000123.4560000000000000E789 ^= 123.456E789 then
   call complain 'Leading and trailing zeros not stripped correctly'

if 123.45600000000000000000000000000000000000003E789 ^= 123.456E789 then
   call complain 'Numbers of length 50 not correctly interpreted'

signal afterargs

echoargs:
   do i=1 to arg()
      if i=1 then
         result = arg(1)
      else
         result = result arg(i)
      end
   return result 


afterargs:


/*----- substituted tail should take at least 50000 -------------------*/

drop foo. 
foo.bar = ''
junk = copies( 'x', 49998 )
do i=1 to 2
  idx = junk'.'i

  if (foo.idx \== 'FOO.'idx) then
     call complain 'Didn''t take long tail in compound variables'
  foo.idx = i 
  end
if (1+foo.junk.1 \== foo.junk.2) then
   call complain 'Didn''t differ between long tail compound variables'

drop foo. junk idx

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
   say 'Tripped in line' sigl':' 'ARG'(1)'.'
   length = charout(,'   (')
return


