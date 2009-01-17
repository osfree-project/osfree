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

/*--------------------------------------------------------------------*/
/* The purpose of this program is to try some strange constructions   */
/*   in REXX, and to fool the interpreter to make a mistake           */
/*                                                                    */
/* Created ... ... : April 1989                                       */
/* Last modified . : 14th March 2003                                  */
/*                                                                    */
/* Current version : 2.00                                             */
/*                                                                    */
/* Written by Anders Christensen, Norwegian Institute of Technology   */
/*   E-mail addr:   Bitnet    : achriste@norunit.earn                 */
/*                  Internet  : anders@solan.unit.no                  */
/*                  Uninett   : christensen@vax.runit.unit.uninett    */
/*--------------------------------------------------------------------*/

written = 0

/*--------------------------------------------------------------------*/
/* First check comments                                               */
/*--------------------------------------------------------------------*/
call notify 'comments'

/*----- Comments can take any character ------------------------------*/
/*   !"#$%&'  *(*+,-./  01234567  89:;<=>?
 *  @ABCDEFG  HIJKLMNO  PQRSTUVW  XYZ[\]^_
 *  `abcdefg  hijklomo  pqrstuvw  xyz{|}~
 *
 *  bel     bs      cr
     esc 
 *  tab 	     ff  
 */


/*----- Let's see if we can nest comments ----------------------------*/
test = /* /* */ ' */ ' ' /* ' /* */ */
if test == ' */   /* ' then
   call complain "Comments can't be nested."
else if test \== ' ' then
   call complain "Strange results from nesting-of-comments test"


/*----- Check if comments act as token separators --------------------*/
abc = 'First '
def = 'Second'
test = abc/*   */def
if test \== 'First Second' then
   call complain 'Comments are not a token separator.'


/*----- Check if comments get confused with whitespace ---------------*/
test1 /* */ = /* */ 'foo' /* */ 'bar'
test2/*   */=/*   */'foo'/*   */'bar'

if (test1 \== 'foo bar') * (test2 \== 'foobar') then
   call complain 'Comments get mixed with whitespace'


/*----- Comments can follow continuation char ------------------------*/
test = 'foo', /*comment*/
   'bar'
if (test\=='foo bar') then
   call complain 'Comment after continutation mark confuses interpreter'

/*----- Comments can come in front of left paran in functions --------*/
if words/* */('foo bar') words/* *//* */('foo bar') \== '2 2' then
   call complain 'Comments not allows in front of left paran in function calls'

if 'WORDS'/* */('foo bar') 'WORDS'/* *//* */('foo bar') \== '2 2' then
   call complain 'Comments not allows in front of left paran in function calls'



/*--------------------------------------------------------------------*/
/* Check the operation of strings                                     */
/*--------------------------------------------------------------------*/
call notify 'strings'

/*----- Single and double quotes are eqivalent -----------------------*/
if " foo  bar " \== ' foo  bar ' then
   call complain 'Single and double quotes are different'


/*----- Quotes might be quoted by doubled ----------------------------*/
if " "" ' """" '' ""'""' " \== ' " '' "" '''' "''"'' ' then
   call complain 'Problems with quoting quotes in strings'


/*----- Let's verify that all characters are allowed in strings ------*/
/*
string = xrange()
interpret 'strin2 = "' || translate(xrange(,'"'),'  ','0a00'x)||xrange('"',      'fe'x) || '"'
/*
 * Unfortunately, there are several things we need to eliminate here.
 * The most evident thing is newlines which are not allowed in literal
 * strings. Regina is based on lex, which don't like 'ff'x or '00'x in
 * the input string. Thus, in 'real life', we would set the limits to
 * zero and ff, and not translate away newline.
 */
do i=1 to 254
   if substr(string,i+1,1) \== d2c(i) then
      call complain 'Character' i 'is invalid in result strings',
             'char='c2x(d2c(i)) 'string='c2x(substr(string,i+1,1))
   if substr(strin2,i+1,1) \== d2c(i) & i\=10 then
      call complain 'Character' i 'is invalid in source code',
             'char='c2x(d2c(i)) 'string='c2x(substr(strin2,i+1,1) )
   end

*/



/*--------------------------------------------------------------------*/
/* Check that hex-strings behave properly                             */
/*--------------------------------------------------------------------*/
call notify 'hexstrings'

/*----- Check for simple use -----------------------------------------*/
if '52 65 78 78'x \== 'Rexx' then
   call complain 'Big problems with hex-strings, simple use failed'


/*----- Both upper and lower case 'x' can be used --------------------*/
if '1234567890'X \== '1234567890'x then
   call complain 'Upper and lower case "x" not equivalent'


/*----- Both upper and lower case hex digits may be used -------------*/
if 'deadbeef'X \== 'DEADBEEF'X then
   call complain 'Upper and lower hex-digits not equivalent'


/*----- Spaces at byte boundaries, else error ------------------------*/
if 'de  adbe  ef'X \== 'dead  beef'X then
   call complain 'Does not handle spaces at byte boundaries'


/*----- First byte might be single hex-digit -------------------------*/
if 'abc'X \== '0abc'X | 'a bc'X \== '0a bc'X then
   call complain 'Does not handle an missing initial zero'


/*----- Initial or terminating space not allowed ---------------------*/
signal on syntax name syntax_hexstring_001
interpret ,
   "if ' 12'x \== '12'x then",
      "call complain 'Leading space makes difference'"
call complain 'Initial space should not really be allowed'

syntax_hexstring_001:
signal on syntax name syntax_hexstring_002
interpret,
   "if '12 'x \== '12'x then ",
      "call complain 'Trailing space makes difference'"
call complain 'Trailing space should not really be allowed'

syntax_hexstring_002:


/*----- Spaces must be at byte boundaries ----------------------------*/
signal on syntax name syntax_hexstring_003
interpret,
   "if '12 3 456 78'X \== '12 03 0456 78'X then",
      "call complain 'Byte groups not padded out to byte boundaries'"
call complain 'Non-even numbers of hex-digits allowed, not standard'
syntax_hexstring_003:


/*----- The "X" must not be part of another token --------------------*/
drop X
drop XYZZY
if 'deedbeef'X||'YZZY' == 'deadbeef'XYZZY then
   call complain 'Read as hexstring when it should not'




/*--------------------------------------------------------------------*/
/* Check that bin-strings behave properly                             */
/*--------------------------------------------------------------------*/
call notify 'binstrings'

/*----- Check for simple use -----------------------------------------*/
if '0101 0010 0110 0101 0111 1000 0111 1000'b \== 'Rexx' then
   call complain 'Big problems with bin-strings, simple use failed'


/*----- Both upper and lower case 'x' can be used --------------------*/
if '1010010111110000'B \== '1010010111110000'b then
   call complain 'Upper and lower case "b" not equivalent'


/*----- Spaces at nibble boundaries, else error ----------------------*/
if '0101  10101111  0000'B \== '01011010  11110000'B then
   call complain 'Does not handle spaces at nibble boundaries'


/*----- First nibble might be less than four digits ------------------*/
if '0 0011'b \== '0000 0011'b | '111 0101'b \== '0111 0101'b then
   call complain 'Does not handle an missing initial zero'


/*----- If needed, pad with extra nibble first -----------------------*/
if '11 0000 1111 0101 1010'B \== '0000 0011 0000 1111 0101 1010'b then
   call complain 'Does not append extra zero nibble when needed'


/*----- Initial or terminating space not allowed ---------------------*/
signal on syntax name syntax_binstring_001
interpret ,
   "if ' 11110000'b \== '11110000'b then",
      "call complain 'Leading space makes difference'"
call complain 'Initial space should not really be allowed'

syntax_binstring_001:
signal on syntax name syntax_binstring_002
interpret,
   "if '11110000 'b \== '11110000'b then ",
      "call complain 'Trailing space makes difference'"
call complain 'Trailing space should not really be allowed'

syntax_binstring_002:


/*----- Spaces must be at nibble boundaries --------------------------*/
signal on syntax name syntax_binstring_003
interpret,
   "if '1111 000 11 0'b \== '11110000 00110000'b then",
      "call complain 'Nibble groups not padded out to nibble boundaries'"
call complain 'Non-even numbers of bin-digits in group allowed, not standard'
syntax_binstring_003:


/*----- The "B" must not be part of another token --------------------*/
drop B
drop BAR
if '111'B||'AR' == '111'BAR then
   call complain 'Read as binstring when it should not'




/*--------------------------------------------------------------------*/
/* Is dataformats ok?                                                 */
/*--------------------------------------------------------------------*/
call notify 'dataformats'






/*--------------------------------------------------------------------*/
/* Then check if line continuations are OK                            */
/*--------------------------------------------------------------------*/
call notify 'continuations'

/*----- A line break should be interpreted as a space 'operator' -----*/
test = 'abc',
'def'
if test \== 'abc def' then
   call complain 'Line continuations is not translated to space'


/*----- Does a comment after the comma confuse it? -------------------*/
test = 'abc', /* */
nop
if test \== 'abc NOP' then
   call complain 'Comments confuse line continuations'

/*----- Let's see if it takes some heavy line continuation -----------*/
if,
   6,
     ==,
        7,
          then,
   call complain "Don't take heavy line continuation"



/*--------------------------------------------------------------------*/
/* Let's test expressions                                             */
/*--------------------------------------------------------------------*/
call notify 'expr'

/*----- First, test all compariastion operators ----------------------*/
str = \('abc'='abc')    \('abc'=='abc')    ('abc'\='abc')    ('abc'\=='abc'),
       ('abc'='xyz')     ('abc'=='xyz')   \('abc'\='xyz')   \('abc'\=='xyz'),
      \('abc'=' abc ')   ('abc'==' abc ')  ('abc'\=' abc ') \('abc'\==' abc '),
       ('abc'\='abc')    ('abc'\=='abc')  \('abc'\='xyz')   \('abc'\=='xyz'),
       ('abc'\=' abc ') \('abc'\==' abc ')

if (str\='0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0') then
   call complain 'Comparison operators do not work properly'


/*---- Then test all the logic operators -----------------------------*/
str = (1|1) (1|0) (0|1) (0|0)
if str\== '1 1 1 0' then
   call complain 'Logical operator | does not seem to work'

str = (1&1) (1&0) (0&1) (0&0)
if str\=='1 0 0 0' then
   call complain 'Logical operator & does not seem to work'

str = (1&&1) (1&&0) (0&&1) (0&&0)
if str\=='0 1 1 0' then
   call complain 'Logical operator && does not seem to work'


/*----- Test logical NOT ---------------------------------------------*/
parse upper version ver
if pos('REGINA',ver)>0 then do
   ipret = 'junk = (((1 \= 0) \= 0) ~= 0)'
   signal on syntax name logical_nots
   interpret ipret
   if junk\=1 then
      call complain "Didn't handle logical nots correctly"
   end

signal after_log_not

logical_nots:
   call complain "Didn't parse all logical nots correctly"

after_log_not:


/*----- Expressions are evaluated from left to right -----------------*/
theargs = '1 2 3'
answer = echoargs() echoargs() echoargs()
if (answer\=='1 2 3') then
   call complain 'Expression not evaluated in correct order'
signal afterargs

echoargs:
   parse var theargs thearg theargs
   return thearg

afterargs:


/*----- Is associativity correct -------------------------------------*/
signal on syntax name syntax_oper1
if (\2**0) \== 1 then
   call complain 'Something is really wrong with prefix \ and **'
call complain 'Incorrect associativelty between prefix \ and **'
syntax_oper1:

signal on syntax name syntax_oper2
if (\2**0) \== 7 then
   call complain 'Something is really wrong with prefix \ and **'
call complain 'Incorrect associativelty between prefix \ and **'
syntax_oper2:

if (-2**2) \== 4 then
   call complain 'Incorrect assiciativety between prefix - and **'

/* prefix + does not change the value of anything .... don't test     */
/* if (+(-2)**2) \== 4 then                                           */
/*    call complain 'Incorrect assiciativety between prefix + and **' */


if (3//2**2) \== 3 then
   call complain 'Incorrect assiciativety between // and **'

if (8%2**2) \== 2 then
   call complain 'Incorrect assiciativety between % and **'

if (3*2**2) \== 12 then
   call complain 'Incorrect assiciativety between * and **'

if (8/2**2) \== 2 then
   call complain 'Incorrect assiciativety between / and **'


if (1+2*3) \== 7 then
   call complain 'Incorrect assiciativety between + and *'

if (7-2*3) \== 1 then
   call complain 'Incorrect assiciativety between - and *'

if (1+6/2) \== 4 then
   call complain 'Incorrect assiciativety between + and /'

if (7-6/2) \== 4 then
   call complain 'Incorrect assiciativety between - and /'

if (2+7//3) \== 3 then
   call complain 'Incorrect assiciativety between + and //'

if (7-7//3) \== 6 then
   call complain 'Incorrect assiciativety between - and //'

if (1+7%3) \== 3 then
   call complain 'Incorrect assiciativety between + and %'

if (7-7%2) \== 4 then
   call complain 'Incorrect assiciativety between - and %'



/*----- Is expressions evalueated completely -------------------------*/
count = 1
junk = ( 1 | subproc() )
if count \== 2 then
   call complain 'OR expressions not completely evaluated'

count = 2
junk = ( 0 & subproc() )
if count \== 3 then
   call complain 'AND expressions not completely evaluated'

signal afterexpr

subproc:
   count = count + 1
   return 1

afterexpr:


/*----- String compare if not both args are numeric ------------------*/
first = '12'
second = '2.3'

if (second > first) then
   call complain 'simple numeric comparisation did not work'

first = '12E1.2'
if (second < first) then
   call complain 'numeric comparisation when not both args are numbers'


/*----- Comparisation is case sensitive ------------------------------*/
first = 'FooBar'
second = 'FOObar'
if (first = second) then
   call complain 'the = operator is case insensitive'

if (first == second) then
   call complain 'the == operator is case insensitive'


/*----- Strings can not be assigned a value --------------------------*/

signal on syntax name strings_1
drop name
rc = -1000
interpret '123 = "hallo"'
rcsave = rc
call complain 'Did not trap setting of string constant'
if (rcsave \== '-1000') then
   call complain 'suspect that command was performed instead of assignment'

strings_1:
signal on syntax name strings_2
drop name
rc = -1000
interpret '123 == "hallo"'
rcsave = rc
call complain 'Did not trap assignment with =='
if (rcsave \== '-1000') then
   call complain 'suspect that command was performed instead of =='

strings_2:
parse source os .
If os = 'UNIX' | os = 'BEOS' | os = 'QNX' | os = 'SKYOS' Then
   Do
      'rm -f ./1'
      'ln -s /bin/true ./1'
   End

rc = -1000
123 <= "hallo"
if (rc \== '0') then
   call complain 'Command was not executed, is "." in PATH ?'

If os = 'UNIX' | os = 'BEOS' | os = 'QNX' | os = 'SKYOS' Then
   Do
      'rm -f ./1'
   End

/*--------------------------------------------------------------------*/
/* Then we test the random generator                                  */
/*--------------------------------------------------------------------*/
call notify 'random'

/*----- Test if we can really set the seed ---------------------------*/
temp1 = ''
temp2 = ''
junk = random(0,999,666)
do 10
   temp1 = temp1 random() ; end

junk = random(0,999,666)
do 10
   temp2 = temp2 random() ; end

if (random(10,10)\==random(10,10)) then
    call complain "Random does not respect upper and lower limits"

if \(temp1==temp2) then
   call complain 'Can not set seed in random generator'

/*--------------------------------------------------------------------*/
/* Test asossiative arrays                                            */
/*--------------------------------------------------------------------*/
call notify 'arrays'

/*----- Test if they work correctly ----------------------------------*/

One = 1
Two = 2
Three = 3
Alfa = 'A'
Beta = 'B'

Fish.1 = 'Hi'
Fish.Beta = 'Hello'
Fish. = 'Bye'
Fish.1.2 = '...'

if (Fish.Alfa \= Fish.Three) then
   call complain "default mechanism do not work"

TwoThree = '1.2'
if (Fish.TwoThree \= Fish.One.Two) then
   call complain "assosiative arrays work incorrectly"

/*-----------------
 * Lexical analysis
 *-----------------*/
call notify 'lexical'
i = 1
do while=3
   i = i + 1
   if i>5 then
      leave
   end

if (i\=6) then
   call complain 'didn''t handle WHILE as ctrl variable correctly'

signal on syntax name lexsynt
interpret 'do (while=3); nop; end'
   call complain "Incorrect handle of WHILE in exprf in DO"
lexsynt:





/*--------------------------------------------------------------------*/
/* Test labels                                                        */
/*--------------------------------------------------------------------*/
call notify 'labels'


/*----- Always use the first occurrence of a label -------------------*/
test = 'before'
signal thelabel
test = 'after'

thelabel:
if test\=='before' then
   call complain 'Something is really wrong with labels'

signal jumpover
thelabel:
call complain 'didnt find first label, but second'
jumpover:


/*----- One statement can have more than one label ------------------*/
foobar = 1
signal first

first: second: third: foobar = foobar + 1
if (foobar=2) then signal second
if (foobar=3) then signal third

if (foobar\=4) then
   call complain 'Multiple labels for one statement does not work'


/*----- Reserved words can be used as labels -------------------------*/
address: value: arg: call: on: off: name: do: end: to: by: for: forever:
while: until: drop: exit: if: then: else: interpret: iterate: leave: nop:
numeric: digits: form: scientific: engineering: fuzz: options: parse:
upper: linein: pull: source: with: var: version: procedure: expose: push:
queue: return: say: select: when: otherwise: signal: trace: nop



/*--------------------------------------------------------------------*/
/* Numbers                                                            */
/*--------------------------------------------------------------------*/
call notify 'numbers'

/*----- possible to assign values to numbers ? -----------------------*/
signal on syntax name ass_numbers_1
istring = '.34 = "hallo"'
interpret istring
call complain 'Assignment of string to number not trapped'

ass_numbers_1:
   if .34 \== '.34' then
      call complain 'The representation of number changed'

   if .34 + 0 \== '0.34' then
      call complain 'The value of number changed'


/*----- integer and fraction part zero simultaneously ? --------------*/

badnumbers = '. 1.23.4 .E1 1.2E2E1 1.2E+-2 12C4'
start_do_loop:
   if badnumbers='' then
      signal after_the_loop

   parse var badnumbers badnumber badnumbers
   signal on syntax name numbers_2
   istring = 'sum = 1. +' badnumber '+ .1'
   interpret istring
   call complain 'The parsing did not choke on the number "'badnumber'".'

   numbers_2:
   signal on syntax name numbers_3
   istring = 'sum = 1. + "'badnumber'" + .1'
   interpret istring
   call complain 'The pluss operator performed pluss on number "'badnumber'".'

   numbers_3:
   signal start_do_loop

after_the_loop:


/*----- Does it interpret multiple embedded signs as unary? ----------*/
signal on syntax name after_mult_emb
istring = 'bar = 1 + foo + 3'
foo = '+-2.0'
interpret istring
call complain 'Does not separate between embedded signs and unary operators'

after_mult_emb:


/*--------------------------------------------------------------------*/
/* Functions                                                          */
/*--------------------------------------------------------------------*/
call notify 'functions'


/*--------------------------------------------------------------------*/
call testing
if result \== '17' then
   call complain 'RESULT not correctly set after call'
signal aftertesting

testing:
   variable = 2 + 3 * 5
return variable

aftertesting:


/*--------------------------------------------------------------------*/
call nextlabel
signal afternextlabel

nextlabel:

if sourceline(sigl) \== 'call nextlabel' then
   call complain 'SIGL not correctly set on signal statement'
return

afternextlabel:


/*--------------------------------------------------------------------*/
/* Test the codes ability to recurse                                  */
/*--------------------------------------------------------------------*/
call notify 'recursing'

/*----- First a simple test ------------------------------------------*/
signal afterrecurse

recurse: procedure
   parse arg parameter
   if parameter>0 then do
/*      say parameter */
      return recurse(parameter-1) parameter
      end
   else
      return 0

afterrecurse:
if recurse(3)\=='0 1 2 3' then
   call complain "didn't recurse properly"

parse value recurse(100) with . '99' result .
if result \== '100' then
   call complain "can't do 100 recursions"




say ''

exit 0



complain:
   say ' ...'
   say 'Tripped in line' sigl':' 'ARG'(1)'.'
   length = charout(,'   (')
return


tell:
   str = arg(1)
   if length+length(str)>70 then do
      say ''
      length = charout(,'   ' || str || ', ') ; end
   else
      length = length + charout(,str || ', ')
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


