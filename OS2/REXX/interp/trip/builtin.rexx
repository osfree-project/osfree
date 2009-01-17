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
 * Tests all builtin functions of rexx, except from:
 *
 *   linein, linout, lines, charin, charout, chars
 *     --> tested in fileio.rexx
 *
 *   queued
 *     --> tested in stack.rexx
 *
 *   random
 *     --> tested in trip.rexx
 */


/* trace ?int */
written = 0
options FIND_BIF
parse version intr ver .
parse source os .
If Substr(intr,1,11) = 'REXX-Regina' Then intr = 'regina'
If Substr(intr,1,14) = 'REXX:Open-REXX' Then intr = 'unirexx'
If Substr(intr,1,8) = 'REXX/imc' Then intr = 'rexximc'

/* ======================= ABBREV ============================== */
call notify 'abbrev'
   call ch abbrev('information','info',4) 		,1
   call ch abbrev('information','',0)			,1
   call ch abbrev('information','Info',4)		,0
   call ch abbrev('information','info',5)		,0
   call ch abbrev('information','info ')		,0
   call ch abbrev('information','info',3)		,1
   call ch abbrev('info','information',3)		,0
   call ch abbrev('info','info',5)			,0


/* ========================== ABS ============================== */
call notify 'abs'
   call ch abs(-12.345)	,12.345
   call ch abs(12.345) 	,12.345
   call ch abs(-0.0) 	,0
   call ch abs(0.0) 	,0

/* ======================== ADDRESS ============================== */
call notify 'address'
   select
      when intr = 'regina' then
         do
            call ch address(), 'SYSTEM'
            call ch address('I'), 'INPUT NORMAL'
            call ch address('O'), 'REPLACE NORMAL'
            call ch address('E'), 'REPLACE NORMAL'
            address system with input fifo 'MYQUEUE' output stem mystem. error append stream 'my.err'
            call ch address('I'), 'INPUT FIFO MYQUEUE'
            call ch address('O'), 'REPLACE STEM MYSTEM.'
            call ch address('E'), 'APPEND STREAM my.err'
            /*
             * Reset address to default
             */
            Address System WITH INPUT NORMAL OUTPUT NORMAL ERROR NORMAL
         end
      when intr = 'unirexx' then call ch address(), 'CMD'
      when intr = 'rexximc' then call ch address(), 'UNIX'
      otherwise nop
   end



/* ========================== ARG ============================== */
call notify 'arg'
   call testarg2 1,,2
   call testarg1
   signal afterarg

testarg1:
   call ch arg()       ,'0'
   call ch arg(1)      ,''
   call ch arg(2)      ,''
   call ch arg(1,'e')  ,'0'
   call ch arg(1,'O')  ,'1'
   return

testarg2:
   call ch arg()      ,'3'
   call ch arg(1)     ,'1'
   call ch arg(2)     ,''
   call ch arg(3)     ,'2'
   call ch arg(4)     ,''
   call ch arg(1,'e') ,'1'
   call ch arg(2,'E') ,'0'
   call ch arg(2,'O') ,'1'
   call ch arg(3,'o') ,'0'
   call ch arg(4,'o') ,'1'
   return

afterarg:



/* ========================= B2X =============================== */
call notify 'b2x'
   call ch b2x(''), ''
   call ch b2x('0'), '0'
   call ch b2x('1'), '1'
   call ch b2x('10'), '2'
   call ch b2x('010'), '2'
   call ch b2x('1010'), 'A'

   call ch b2x('1 0101'), '15'
   call ch b2x('1 01010101'), '155'
   call ch b2x('1 0101 0101'), '155'
   call ch b2x('10101 0101'), '155'
   call ch b2x('0000 00000000 0000'), '0000'
   call ch b2x('11111111 11111111'), 'FFFF'


/* ======================= BITAND ============================== */
call notify 'bitand'
   call ch bitand( '123456'x, '3456'x ),         '101456'x
   call ch bitand( '3456'x, '123456'x, '99'x ),  '101410'x
   call ch bitand( '123456'x,, '55'x),           '101454'x
   call ch bitand( 'foobar' ),                   'foobar'
   call ch bitand( 'FooBar' ,, 'df'x),           'FOOBAR'  /* for ASCII */


/* ======================== BITOR ============================== */
call notify 'bitor'
   call ch bitor( '123456'x, '3456'x ),         '367656'x
   call ch bitor( '3456'x, '123456'x, '99'x ),  '3676df'x
   call ch bitor( '123456'x,, '55'x),           '577557'x
   call ch bitor( 'foobar' ),                   'foobar'
   call ch bitor( 'FooBar' ,, '20'x),           'foobar'  /* for ASCII */


/* ======================= BITXOR ============================== */
call notify 'bitxor'
   call ch bitxor( '123456'x, '3456'x ),         '266256'x
   call ch bitxor( '3456'x, '123456'x, '99'x ),  '2662cf'x
   call ch bitxor( '123456'x,, '55'x),           '476103'x
   call ch bitxor( 'foobar' ),                   'foobar'
   call ch bitxor( 'FooBar' ,, '20'x),           'fOObAR'  /* for ASCII */


/* ======================== C2D ============================== */
call notify 'c2d'
   call ch c2d( 'ff80'x, 1), '-128'
   call ch c2d( 'foo' ) ,  '6713199'
   call ch c2d( 'bar' ),   '6447474'
   call ch c2d( '' ),      '0'
   call ch c2d( '101'x ),  '257'
   call ch c2d( 'ff'x ),   '255'
   call ch c2d( 'ffff'x),  '65535'
   call ch c2d( 'ffff'x, 2), '-1'
   call ch c2d( 'ffff'x, 1), '-1'
   call ch c2d( 'fffe'x, 2), '-2'
   call ch c2d( 'fffe'x, 1), '-2'
   call ch c2d( 'ffff'x, 3), '65535'
   call ch c2d( 'ff7f'x, 1), '127'
   call ch c2d( 'ff7f'x, 2), '-129'
   call ch c2d( 'ff7f'x, 3), '65407'
   call ch c2d( 'ff80'x, 1), '-128'
   call ch c2d( 'ff80'x, 2), '-128'
   call ch c2d( 'ff80'x, 3), '65408'
   call ch c2d( 'ff81'x, 1), '-127'
   call ch c2d( 'ff81'x, 2), '-127'
   call ch c2d( 'ff81'x, 3), '65409'
   call ch c2d( 'ffffffffff'x, 5), '-1'


/* ======================== C2X ============================== */
call notify 'c2x'
   call ch c2x( 'foobar'),             '666F6F626172'
   call ch c2x( '' ),                  ''
   call ch c2x( '101'x ),              '0101'
   call ch c2x( '0123456789abcdef'x ), '0123456789ABCDEF'
   call ch c2x( 'ffff'x ),             'FFFF'
   call ch c2x( 'ffffffff'x ),         'FFFFFFFF'


/* ======================= CENTER ============================== */
call notify 'center'
   call ch center('****',8,'-')      ,'--****--'
   call ch center('****',7,'-')      ,'-****--'
   call ch center('*****',8,'-')     ,'-*****--'
   call ch center('*****',7,'-')     ,'-*****-'
   call ch center('12345678',4,'-')  ,'3456'
   call ch center('12345678',5,'-')  ,'23456'
   call ch center('1234567',4,'-')   ,'2345'
   call ch center('1234567',5,'-')   ,'23456'


/* ===================== CHANGESTR ============================= */
call notify 'changestr'
if ver > 4 Then
  do
    Call ch changestr('a','fred','c') , 'fred'
    Call ch changestr('','','x') , ''
    Call ch changestr('a','abcdef','x') , 'xbcdef'
    Call ch changestr('0','0','1') , '1'
    Call ch changestr('a','def','xyz') , 'def'
    Call ch changestr('a','','x') , ''
    Call ch changestr('','def','xyz') , 'def'
    Call ch changestr('abc','abcdef','xyz') , 'xyzdef'
    Call ch changestr('abcdefg','abcdef','xyz') , 'abcdef'
    Call ch changestr('abc','abcdefabccdabcd','z') , 'zdefzcdzd'
  end

/* ====================== COMPARE ============================== */
call notify 'compare'
   call ch compare('foo', 'bar')      , 1
   call ch compare('foo', 'foo')      , 0
   call ch compare('   ', '' )        , 0
   call ch compare('foo', 'f', 'o')   , 0
   call ch compare('foobar', 'foobag'), 6



/* ======================= COPIES ============================== */
call notify 'copies'
   call ch copies('foo',3)     , 'foofoofoo'
   call ch copies('x', 10)     , 'xxxxxxxxxx'
   call ch copies('', 50)      , ''
   call ch copies('', 0)       , ''
   call ch copies('foobar',0 ) , ''

/* ===================== COUNTSTR ============================== */
call notify 'countstr'
if ver > 4 Then
  do
    Call ch countstr('','') , 0
    Call ch countstr('a','abcdef') , 1
    Call ch countstr(0,0) , 1
    Call ch countstr('a','def') , 0
    Call ch countstr('a','') , 0
    Call ch countstr('','def') , 0
    Call ch countstr('abc','abcdef') , 1
    Call ch countstr('abcdefg','abcdef') , 0
    Call ch countstr('abc','abcdefabccdabcd') , 3
  end

/* ===================== DATATYPE ============================== */
call notify 'datatype'
   call ch datatype('foobar'), 'CHAR'
   call ch datatype('foo bar'), 'CHAR'
   call ch datatype('123.456.789'), 'CHAR'
   call ch datatype('123.456'), 'NUM'
   call ch datatype(''), 'CHAR'
   call ch datatype('DeadBeef','A'), '1'
   call ch datatype('Dead Beef','A'), '0'
   call ch datatype('1234ABCD','A'), '1'
   call ch datatype('','A'), '0'
   call ch datatype('foobar','B'), '0'
   call ch datatype('01001101','B'), '1'
   call ch datatype('0110 1101','B'), '1'
   call ch datatype('0110 110','B'), '0'
   call ch datatype('','B'), '1'
   call ch datatype('foobar','L'), '1'
   call ch datatype('FooBar','L'), '0'
   call ch datatype('foo bar','L'), '0'
   call ch datatype('','L'), '0'
   call ch datatype('foobar','M'), '1'
   call ch datatype('FooBar','M'), '1'
   call ch datatype('foo bar','M'), '0'
   call ch datatype('FOOBAR','M'), '1'
   call ch datatype('','M'), '0'
   call ch datatype('foo bar','N'), '0'
   call ch datatype('1324.1234','N'), '1'
   call ch datatype('123.456.789','N'), '0'
   call ch datatype('','N'), '0'
   call ch datatype('foo bar','S'), '0'
   call ch datatype('??@##_Foo$Bar!!!','S'), '1'
   call ch datatype('999e+9','S'), '1'
   call ch datatype('','S'), '0'
   call ch datatype('foo bar','U'), '0'
   call ch datatype('Foo Bar','U'), '0'
   call ch datatype('FOOBAR','U'), '1'
   call ch datatype('','U'), '0'

   numeric digits 9
   call ch datatype('Foobar','W'), '0'
   call ch datatype('123','W'), '1'
   call ch datatype('12.3','W'), '0'
   call ch datatype('','W'), '0'
   call ch datatype('123.123','W'), '0'
   call ch datatype('123.123E3','W'), '1'
   call ch datatype('123.0000003','W'), '0'
   call ch datatype('123.0000004','W'), '0'
   call ch datatype('123.0000005','W'), '0'
   call ch datatype('123.0000006','W'), '0'
   call ch datatype(' 23','W'), '1'
   call ch datatype(' 23 ','W'), '1'
   call ch datatype('23 ','W'), '1'
   call ch datatype('123.00','W'), '1'
   call ch datatype('123000E-2','W'), '1'
   call ch datatype('123000E+2','W'), '1'

   call ch datatype('Foobar','X'), '0'
   call ch datatype('DeadBeef','X'), '1'
   call ch datatype('A B C','X'), '0'
   call ch datatype('A BC DF','X'), '1'
   call ch datatype('123ABC','X'), '1'
   call ch datatype('123AHC','X'), '0'
   call ch datatype('','X'), '1'

   call ch datatype('0.000E-2','w'), '1'
   call ch datatype('0.000E-1','w'), '1'
   call ch datatype('0.000E0','w'), '1'
   call ch datatype('0.000E1','w'), '1'
   call ch datatype('0.000E2','w'), '1'
   call ch datatype('0.000E3','w'), '1'
   call ch datatype('0.000E4','w'), '1'
   call ch datatype('0.000E5','w'), '1'
   call ch datatype('0.000E6','w'), '1'

   call ch datatype('0E-1','w'), '1'
   call ch datatype('0E0','w'), '1'
   call ch datatype('0E1','w'), '1'
   call ch datatype('0E2','w'), '1'



/* ====================== DELSTR ============================== */
call notify 'delstr'
   call ch delstr('Med lov skal land bygges', 6), 'Med l'
   call ch delstr('Med lov skal land bygges', 6,10), 'Med lnd bygges'
   call ch delstr('Med lov skal land bygges', 1), ''
   call ch delstr('Med lov skal', 30), 'Med lov skal'
   call ch delstr('Med lov skal', 8,8), 'Med lov'
   call ch delstr('Med lov skal', 12), 'Med lov ska'
   call ch delstr('Med lov skal', 13), 'Med lov skal'
   call ch delstr('Med lov skal', 14), 'Med lov skal'
   call ch delstr('', 30), ''

/* ====================== DELWORD ============================== */
call notify 'delword'
   call ch delword('Med lov skal land bygges', 3), 'Med lov '
   call ch delword('Med lov skal land bygges', 1), ''
   call ch delword('Med lov skal land bygges', 1,1), 'lov skal land bygges'
   call ch delword('Med lov skal land bygges', 2,3), 'Med bygges'
   call ch delword('Med lov skal land bygges', 2,10), 'Med '
   call ch delword('Med lov   skal land bygges', 3,2), 'Med lov   bygges'
   call ch delword('Med lov   skal land   bygges', 3,2), 'Med lov   bygges'
   call ch delword('Med lov skal land   bygges', 3,2), 'Med lov bygges'
   call ch delword('Med lov skal land bygges', 3,0), 'Med lov skal land bygges'
   call ch delword('Med lov skal land bygges', 10), 'Med lov skal land bygges'
   call ch delword('Med lov skal land bygges', 9,9), 'Med lov skal land bygges'
   call ch delword('Med lov skal land bygges', 1,0), 'Med lov skal land bygges'
   call ch delword('  Med lov skal', 1,0), '  Med lov skal'
   call ch delword('  Med lov skal   ', 4), '  Med lov skal   '
   call ch delword('', 1), ''



/* ====================== DIGITS ============================== */
call notify 'digits'
   call ch digits(), '9'


/* ======================= D2C ============================== */
call notify 'd2c'
   call ch d2c(127), '7f'x
   call ch d2c(128), '80'x
   call ch d2c(129), '81'x
   call ch d2c(1), '01'x
   call ch d2c(-1,1), 'FF'x
   call ch d2c(-127,1), '81'x
   call ch d2c(-128,1), '80'x
   call ch d2c(-129,1), '7F'x
   call ch d2c(-1,2), 'FFFF'x
   call ch d2c(-127,2), 'FF81'x
   call ch d2c(-128,2), 'FF80'x
   call ch d2c(-129,2), 'FF7F'x
   call ch d2c(129,0), ''
   call ch d2c(129,1), '81'x
   call ch d2c(256+129,2), '0181'x
   call ch d2c(256*256+256+129,3), '010181'x


/* ======================= D2X ============================== */
call notify 'd2x'
/*   call ch d2x(0), '0' */
   call ch d2x(127), '7F'
   call ch d2x(128), '80'
   call ch d2x(129), '81'
   call ch d2x(1), '1'
   call ch d2x(-1,2), 'FF'
   call ch d2x(-127,2), '81'
   call ch d2x(-128,2), '80'
   call ch d2x(-129,2), '7F'
   call ch d2x(-1,3), 'FFF'
   call ch d2x(-127,3), 'F81'
   call ch d2x(-128,4), 'FF80'
   call ch d2x(-129,5), 'FFF7F'
   call ch d2x(129,0), ''
   call ch d2x(129,2), '81'
   call ch d2x(256+129,4), '0181'
   call ch d2x(256*256+256+129,6), '010181'



/* ===================== ERRORTEXT ============================== */
call notify 'errortext'
   call ch errortext(10), 'Unexpected or unmatched END'
   call ch errortext(40), 'Incorrect call to routine'
   call ch errortext(50), 'Unrecognized reserved symbol'
   call ch errortext( 1), ''



/* ======================== FORM ============================== */
call notify 'form'
   call ch form(), 'SCIENTIFIC'


/* ======================= FORMAT ============================== */
call notify 'format'
   call ch format(12.34), '12.34'
   call ch format(12.34,4), '  12.34'
   call ch format(12.34,4,4), '  12.3400'
   call ch format(12.34,4,1), '  12.3'
   call ch format(12.35,4,1), '  12.4'
   call ch format(12.34,,4), '12.3400'
   call ch format(12.34,4,0), '  12'

   call ch format(99.995,3,2), '100.00'
   call ch format(0.111,,4), '0.1110'
   call ch format(0.0111,,4), '0.0111'
   call ch format(0.00111,,4), '0.0011'
   call ch format(0.000111,,4), '0.0001'
   call ch format(0.0000111,,4), '0.0000'
   call ch format(0.00000111,,4), '0.0000'
   call ch format(0.555,,4), '0.5550'
   call ch format(0.0555,,4), '0.0555'
   call ch format(0.00555,,4), '0.0056'
   call ch format(0.000555,,4), '0.0006'
   call ch format(0.0000555,,4), '0.0001'
   call ch format(0.00000555,,4), '0.0000'
   call ch format(0.999,,4), '0.9990'
   call ch format(0.0999,,4), '0.0999'
   call ch format(0.00999,,4), '0.0100'
   call ch format(0.000999,,4), '0.0010'
   call ch format(0.0000999,,4), '0.0001'
   call ch format(0.00000999,,4), '0.0000'
   call ch format(0.455,,4), '0.4550'
   call ch format(0.0455,,4), '0.0455'
   call ch format(0.00455,,4), '0.0046'
   call ch format(0.000455,,4), '0.0005'
   call ch format(0.0000455,,4), '0.0000'
   call ch format(0.00000455,,4), '0.0000'

   call ch format(1.00000045,,6), '1.000000'
   call ch format(1.000000045,,7), '1.0000001'  /* this is an error in TRL */
   call ch format(1.0000000045,,8), '1.00000000'

   call ch format(12.34,,,,0), '1.234E+1'
   call ch format(12.34,,,3,0), '1.234E+001'
   call ch format(12.34,,,3,),  '12.34'
   call ch format(1.234,,,3,0), '1.234     '
   call ch format(12.34,3,,,0), '  1.234E+1'
   call ch format(12.34,,2,,0), '1.23E+1'
   call ch format(12.34,,3,,0), '1.234E+1'
   call ch format(12.34,,4,,0), '1.2340E+1'
   call ch format(12.345,,3,,0), '1.235E+1'

   call ch format(99.999,,,,), '99.999'
   call ch format(99.999,,2,,), '100.00'
   call ch format(99.999,,2,,2), '1.00E+2'
   call ch format(.999999,,4,2,2), '1.0000'
   call ch format(.999999,,5,2,2),  '9.99999E-01'
   call ch format(.9999999,,5,2,2), '1.00000    '
   call ch format(.999999,,6,2,2), '9.999990E-01'
   call ch format(90.999,,0), '91'
   call ch format(0099.999,5,3,,), '   99.999'

   call ch format(0.0000000000000000001,4), '   1E-19'
   call ch format(0.0000000000000000001,4,4), '   1.0000E-19'
   call ch format(0.0000001,4,,,3), '   1E-7'
   call ch format(0.0000001,4,4,,3), '   1.0000E-7'
   call ch format(0.000001,4,4,,3), '   0.0000'
   call ch format(0.0000001,4,5,,2), '   1.00000E-7'
   call ch format(0.0000001,4,4,4,3), '   1.0000E-0007'
   call ch format(1000,4,4,,3), '   1.0000E+3'

   call ch format(0.0000000000000000000001), '1E-22'
   call ch format(0.0000000000000000000001,,,0,), '0.0000000000000000000001'
   call ch format(0.0000001,,,0,3), '0.0000001'



/* ======================== FUZZ ============================== */
call notify 'fuzz'
   call ch fuzz(), '0'


/* ======================= INSERT ============================== */
call notify 'insert'
   call ch insert('abc','def'), 'abcdef'
   call ch insert('abc','def',2), 'deabcf'
   call ch insert('abc','def',3), 'defabc'
   call ch insert('abc','def',5), 'def  abc'
   call ch insert('abc','def',5,,'*'), 'def**abc'
   call ch insert('abc','def',5,4,'*'), 'def**abc*'
   call ch insert('abc','def',,0), 'def'
   call ch insert('abc','def',2,1), 'deaf'


/* ====================== LASTPOS ============================== */
call notify 'lastpos'
   call ch lastpos('b', 'abc abc'), 6
   call ch lastpos('b', 'abc abc',5), 2
   call ch lastpos('b', 'abc abc',6), 6
   call ch lastpos('b', 'abc abc',7), 6
   call ch lastpos('x', 'abc abc'), 0
   call ch lastpos('b', 'abc abc',20), 6
   call ch lastpos('b', ''), 0
   call ch lastpos('', 'c'), 0
   call ch lastpos('', ''), 0
   call ch lastpos('b', 'abc abc',20), 6
   call ch lastpos('bc', 'abc abc'), 6
   call ch lastpos('bc ', 'abc abc',20), 2
   call ch lastpos('abc', 'abc abc',6), 1
   call ch lastpos('abc', 'abc abc'), 5
   call ch lastpos('abc', 'abc abc',7), 5


/* ======================== LEFT ============================== */
call notify 'left'
   call ch left('foobar',1),     'f'
   call ch left('foobar',0),     ''
   call ch left('foobar',6),     'foobar'
   call ch left('foobar',8),     'foobar  '
   call ch left('foobar',8,'*'), 'foobar**'
   call ch left('foobar',1,'*'), 'f'



/* ======================= LENGTH ============================== */
call notify 'length'
   call ch length(''),            0
   call ch length('a'),           1
   call ch length('abc'),         3
   call ch length('abcdefghij'), 10


/* ======================= LOWER =============================== */
call notify 'lower'
   call ch lower('FOOBAR',2,3),  'FoobAR'
   call ch lower('FOOBAR',3),  'FOobar'
   call ch lower('FOOBAR',3,8),  'FOobar  '
   call ch lower('FOOBAR',3,8,'*'),  'FOobar**'
   call ch lower('FOOBAR',6,3),  'FOOBAr'
   call ch lower('FOOBAR',8,3),  'FOOBAR'


/* ======================== MAX ============================== */
call notify 'max'
   call ch max( 10.1 ),               '10.1'
   call ch max( -10.1, 3.8 ),         '3.8'
   call ch max( 10.1, 10.2, 10.3 ),   '10.3'
   call ch max( 10.3, 10.2, 10.3 ),   '10.3'
   call ch max( 10.1, 10.2, 10.3 ),   '10.3'
   call ch max( 10.1, 10.4, 10.3 ),   '10.4'
   call ch max( 10.3, 10.2, 10.1 ),   '10.3'
   call ch max( 1, 2, 4, 5 ),         '5'
   call ch max( -0, 0 ),              '0'
   call ch max( 1,2,3,4,5,6,7,8,7,6,5,4,3,2 ),   '8'


/* ======================== MIN ============================== */
call notify 'min'
   call ch min( 10.1 ),               '10.1'
   call ch min( -10.1, 3.8 ),         '-10.1'
   call ch min( 10.1, 10.2, 10.3 ),   '10.1'
   call ch min( 10.1, 10.2, 10.1 ),   '10.1'
   call ch min( 10.1, 10.2, 10.3 ),   '10.1'
   call ch min( 10.4, 10.1, 10.3 ),   '10.1'
   call ch min( 10.3, 10.2, 10.1 ),   '10.1'
   call ch min( 5, 2, 4, 1 ),         '1'
   call ch min( -0, 0 ),              '0'
   call ch min( 8,2,3,4,5,6,7,1,7,6,5,4,3,2 ),   '1'

/* ====================== OVERLAY ============================== */
call notify 'overlay'
   call ch overlay('foo', 'abcdefghi',3,4,'*'), 'abfoo*ghi'
   call ch overlay('foo', 'abcdefghi',3,2,'*'), 'abfoefghi'
   call ch overlay('foo', 'abcdefghi',3,4,), 'abfoo ghi'
   call ch overlay('foo', 'abcdefghi',3), 'abfoofghi'
   call ch overlay('foo', 'abcdefghi',,4,'*'), 'foo*efghi'
   call ch overlay('foo', 'abcdefghi',9,4,'*'), 'abcdefghfoo*'
   call ch overlay('foo', 'abcdefghi',10,4,'*'), 'abcdefghifoo*'
   call ch overlay('foo', 'abcdefghi',11,4,'*'), 'abcdefghi*foo*'
   call ch overlay('', 'abcdefghi',3), 'abcdefghi'
   call ch overlay('foo', '',3), '  foo'
   call ch overlay('', '',3,4,'*'), '******'
   call ch overlay('', ''), ''

/* ======================== POS ============================== */
call notify 'pos'
   call ch pos('foo','a foo foo b'), 3
   call ch pos('foo','a foo foo',3), 3
   call ch pos('foo','a foo foo',4), 7
   call ch pos('foo','a foo foo b',30), 0
   call ch pos('foo','a foo foo b',1), 3
   call ch pos('','a foo foo b'), 0
   call ch pos('foo',''), 0
   call ch pos('',''), 0
   call ch pos('b','a'), 0
   call ch pos('b','b'), 1
   call ch pos('b','abc'), 2
   call ch pos('b','def'), 0
   call ch pos('foo','foo foo b'), 1

   if intr = 'regina' then do
      call ch index('a foo foo b','foo'), 3
      call ch index('a foo foo','foo',3), 3
      call ch index('a foo foo','foo',4), 7
      call ch index('a foo foo b','foo',30), 0
      call ch index('a foo foo b','foo',1), 3
      call ch index('a foo foo b',''), 0
      call ch index('','foo'), 0
      call ch index('',''), 0
      call ch index('a','b'), 0
      call ch index('b','b'), 1
      call ch index('abc','b'), 2
      call ch index('def','b'), 0
      call ch index('foo foo b','foo'), 1
   end

/* ====================== REVERSE ============================== */
call notify 'reverse'
   call ch reverse('foobar'),    'raboof'
   call ch reverse(''),          ''
   call ch reverse('fubar'),     'rabuf'
   call ch reverse('f'),         'f'
   call ch reverse('  foobar '), ' raboof  '



/* ======================= RIGHT ============================== */
call notify 'right'
   call ch right('',4),           '    '
   call ch right('foobar',0),     ''
   call ch right('foobar',3),     'bar'
   call ch right('foobar',6),     'foobar'
   call ch right('foobar',8),     '  foobar'
   call ch right('foobar',8,'*'), '**foobar'
   call ch right('foobar',4,'*'), 'obar'



/* ======================== SIGN ============================== */
call notify 'sign'
   call ch sign('0'),    0
   call ch sign('-0'),   0
   call ch sign('0.4'),  1
   call ch sign('-10'), -1
   call ch sign('15'),   1



/* ===================== SOURCELINE ============================== */
call notify 'sourceline'
   parse source . . srcfile .
   call ch sourceline(),   lines(srcfile,'C')   /* don't work for stdin */
   call ch sourceline(whatline()-2), '   parse source . . srcfile .'
   call ch sourceline(whatline()-4), "call notify 'sourceline'"

   signal aftersrc
   whatline: return sigl
aftersrc:


/* ======================== SPACE ============================== */
call notify 'space'
   call ch space(' foo ')               ,'foo'
   call ch space('  foo')               ,'foo'
   call ch space('foo  ')               ,'foo'
   call ch space('  foo  ')             ,'foo'
   call ch space(' foo bar ')           ,'foo bar'
   call ch space('  foo  bar  ')        ,'foo bar'
   call ch space(' foo bar ',2)         ,'foo  bar'
   call ch space(' foo bar ',,'-')      ,'foo-bar'
   call ch space('  foo  bar  ',2,'-')  ,'foo--bar'
   call ch space(' f-- b-- ',2,'-')     ,'f----b--'
   call ch space(' f o o   b a r ',0)   ,'foobar'


/* ======================= STRIP ============================== */
call notify 'strip'
   call ch strip('  foo   bar   '),          'foo   bar'
   call ch strip('  foo   bar   ','L'),      'foo   bar   '
   call ch strip('  foo   bar   ','T'),      '  foo   bar'
   call ch strip('  foo   bar   ','B'),      'foo   bar'
   call ch strip('  foo   bar   ','B','*'),  '  foo   bar   '
   call ch strip('  foo   bar',,'r'),        '  foo   ba'
   call ch strip('  foo' || '0a'x || '09'x || ' '),         'foo' /* this is not an error */



/* ======================= SUBSTR ============================== */
call notify 'substr'
   call ch substr('foobar',2,3),  'oob'
   call ch substr('foobar',3),  'obar'
   call ch substr('foobar',3,6),  'obar  '
   call ch substr('foobar',3,6,'*'),  'obar**'
   call ch substr('foobar',6,3),  'r  '
   call ch substr('foobar',8,3),  '   '



/* ======================= SUBWORD ============================== */
call notify 'subword'
   call ch subword(' to be or not to be ',5),  'to be'
   call ch subword(' to be or not to be ',6),  'be'
   call ch subword(' to be or not to be ',7),  ''
   call ch subword(' to be or not to be ',8,7),  ''
   call ch subword(' to be or not to be ',3,2),  'or not'
   call ch subword(' to be or not to be ',1,2),  'to be'
   call ch subword(' to be or not   to be ',4,2),  'not   to'
   call ch subword('abc de f', 3), 'f'



/* ======================= SYMBOL ============================== */

call notify 'symbol'
   parse value 'foobar' with alpha 1 beta 1 omega 1 gamma.foobar
   omega = 'FOOBAR'
   call ch symbol('HEPP'),         'LIT'
   call ch symbol('ALPHA'),        'VAR'
   call ch symbol('Un*x'),         'BAD'
   call ch symbol('gamma.delta'),  'LIT'
   call ch symbol('gamma.FOOBAR'), 'VAR'
   call ch symbol('gamma.alpha'),  'LIT'
   call ch symbol('gamma.omega'),  'VAR'
   call ch symbol('gamma.Un*x'),   'LIT'
   call ch symbol('Un*x.gamma'),   'BAD'
   call ch symbol('!!'),           'LIT'
   call ch symbol(''),             'BAD'
   call ch symbol('00'x),          'BAD'
   call ch symbol('foo-bar'),      'BAD'


/* ======================= TRACE ============================== */


/* ====================== TRANSLATE ============================== */
call notify 'translate'
   call ch translate('Foo Bar'), 'FOO BAR'
   call ch translate('Foo Bar',,''), 'Foo Bar'
   call ch translate('Foo Bar','',), '       '
   call ch translate('Foo Bar','',,'*'), '*******'
   call ch translate('Foo Bar',xrange('01'x,'ff'x)), 'Gpp!Cbs'
   call ch translate('','klasjdf','woieruw'), ''
   call ch translate('foobar','abcdef','fedcba'), 'aooefr'



/* ======================= TRUNC ============================== */
call notify 'trunc'
   call ch trunc(1234.5678, 2), '1234.56'
   call ch trunc(-1234.5678), '-1234'
   call ch trunc(.5678), '0'
   call ch trunc(.00123), '0'
   call ch trunc(.00123,4), '0.0012'
   call ch trunc(.00127,4), '0.0012'
   call ch trunc(.1678), '0'
   call ch trunc(1234.5678), '1234'
   call ch trunc(4.5678, 7), '4.5678000'

   call ch trunc(10000005.0,2), 10000005.00
   call ch trunc(10000000.5,2), 10000000.50
   call ch trunc(10000000.05,2), 10000000.10
   call ch trunc(10000000.005,2), 10000000.00

   call ch trunc(10000005.5,2), 10000005.50
   call ch trunc(10000000.55,2), 10000000.60
   call ch trunc(10000000.055,2), 10000000.10
   call ch trunc(10000000.0055,2), 10000000.00

   call ch trunc(10000000.04,2), 10000000.00
   call ch trunc(10000000.045,2), 10000000.00
   call ch trunc(10000000.45,2), 10000000.50

   call ch trunc(10000000.05,2), 10000000.10
   call ch trunc(10000000.05,2), 10000000.10
   call ch trunc(10000000.05,2), 10000000.10

   call ch trunc(99999999.,2), 99999999.00
   call ch trunc(99999999.9,2), 99999999.90
   call ch trunc(99999999.99,2), 100000000.00

   call ch trunc(1E2,0), 100
   call ch trunc(12E1,0), 120
   call ch trunc(123.,0), 123
   call ch trunc(123.1,0), 123
   call ch trunc(123.12,0), 123
   call ch trunc(123.123,0), 123
   call ch trunc(123.1234,0), 123
   call ch trunc(123.12345,0), 123

   call ch trunc(1E2,1), 100.0
   call ch trunc(12E1,1), 120.0
   call ch trunc(123.,1), 123.0
   call ch trunc(123.1,1), 123.1
   call ch trunc(123.12,1), 123.1
   call ch trunc(123.123,1), 123.1
   call ch trunc(123.1234,1), 123.1
   call ch trunc(123.12345,1), 123.1

   call ch trunc(1E2,2), 100.00
   call ch trunc(12E1,2), 120.00
   call ch trunc(123.,2), 123.00
   call ch trunc(123.1,2), 123.10
   call ch trunc(123.12,2), 123.12
   call ch trunc(123.123,2), 123.12
   call ch trunc(123.1234,2), 123.12
   call ch trunc(123.12345,2), 123.12

   call ch trunc(1E2,3), 100.000
   call ch trunc(12E1,3), 120.000
   call ch trunc(123.,3), 123.000
   call ch trunc(123.1,3), 123.100
   call ch trunc(123.12,3), 123.120
   call ch trunc(123.123,3), 123.123
   call ch trunc(123.1234,3), 123.123
   call ch trunc(123.12345,3), 123.123

   call ch trunc(1E2,4), 100.0000
   call ch trunc(12E1,4), 120.0000
   call ch trunc(123.,4), 123.0000
   call ch trunc(123.1,4), 123.1000
   call ch trunc(123.12,4), 123.1200
   call ch trunc(123.123,4), 123.1230
   call ch trunc(123.1234,4), 123.1234
   call ch trunc(123.12345,4), 123.1234

   call ch trunc(1E2,5), 100.00000
   call ch trunc(12E1,5), 120.00000
   call ch trunc(123.,5), 123.00000
   call ch trunc(123.1,5), 123.10000
   call ch trunc(123.12,5), 123.12000
   call ch trunc(123.123,5), 123.12300
   call ch trunc(123.1234,5), 123.12340
   call ch trunc(123.12345,5), 123.12345


/* ======================= UPPER =============================== */
call notify 'upper'
   call ch upper('foobar',2,3),  'fOOBar'
   call ch upper('foobar',3),  'foOBAR'
   call ch upper('foobar',3,8),  'foOBAR  '
   call ch upper('foobar',3,8,'*'),  'foOBAR**'
   call ch upper('foobar',6,3),  'foobaR'
   call ch upper('foobar',8,3),  'foobar'
   call ch upper('foobar',1,1),  'Foobar'


/* ======================= VALUE ============================== */
call notify 'value'
   x.a = 'asdf'
   x.b = 'foo'
   x.c = 'A'
   a = 'B'
   b = 'C'
   c = 'A'

   call ch value('a'), 'B'
   call ch value(a), 'C'
   call ch value(c), 'B'
   call ch value('c'), 'A'
   call ch value('x.A'), 'foo'
   call ch value(x.B), 'B'
   call ch value('x.B'), 'A'
   call ch value('x.'||a), 'A'
   call ch value(value(x.b)), 'C'

   xyzzy = 'foo'
   call ch value('xyzzy'),       'foo'
   call ch value('xyzzy','bar'), 'foo'
   call ch value('xyzzy'),       'bar'
   call ch value('xyzzy','bar'), 'bar'
   call ch value('xyzzy'),       'bar'
   call ch value('xyzzy','foo'), 'bar'
   call ch value('xyzzy'),       'foo'

   xyzzy = 'void'
   if os = 'UNIX' | os = 'AMIGA' Then
      envvar = '$xyzzy'
   else
      envvar = '%xyzzy%'
   call value 'xyzzy', 'bar', 'ENVIRONMENT'
   call ch value('xyzzy', 'bar', 'ENVIRONMENT'), 'bar'
   call ch value('xyzzy',,       'ENVIRONMENT'), 'bar'
   call ch value('xyzzy',      , 'ENVIRONMENT'), 'echo'(envvar)
   call ch value('xyzzy', 'foo', 'ENVIRONMENT'), 'bar'
   call ch value('xyzzy', 'bar', 'ENVIRONMENT'), 'foo'
   call ch value('xyzzy',      , 'ENVIRONMENT'), 'echo'(envvar)
   call ch value('xyzzy',      , 'ENVIRONMENT'), 'bar'
   call ch value('xyzzy', 'foo', 'ENVIRONMENT'), 'bar'
   call ch value('xyzzy',      , 'ENVIRONMENT'), 'echo'(envvar)


/* ======================= VERIFY ============================== */
call notify 'verify'
   call ch verify('foobar', 'barfo', N, 1), 0
   call ch verify('foobar', 'barfo', M, 1), 1
   call ch verify('', 'barfo'), 0
   call ch verify('foobar', ''), 1
   call ch verify('foobar', 'barf', N, 3), 3
   call ch verify('foobar', 'barf', N, 4), 0
   call ch verify('', ''), 0



/* ======================== WORD ============================== */
call notify 'word'
   call ch word('This is certainly a test',1), 'This'
   call ch word('   This is certainly a test',1), 'This'
   call ch word('This    is certainly a test',1), 'This'
   call ch word('This    is certainly a test',2), 'is'
   call ch word('This is    certainly a test',2), 'is'
   call ch word('This is certainly a    test',5), 'test'
   call ch word('This is certainly a test   ',5), 'test'
   call ch word('This is certainly a test',6), ''
   call ch word('',1), ''
   call ch word('',10), ''
   call ch word('test ',2), ''



/* ====================== WORDINDEX ============================== */
call notify 'wordindex'
   call ch wordindex('This is certainly a test',1), '1'
   call ch wordindex('  This is certainly a test',1), '3'
   call ch wordindex('This   is certainly a test',1), '1'
   call ch wordindex('  This   is certainly a test',1), '3'
   call ch wordindex('This is certainly a test',2), '6'
   call ch wordindex('This   is certainly a test',2), '8'
   call ch wordindex('This is   certainly a test',2), '6'
   call ch wordindex('This   is   certainly a test',2), '8'
   call ch wordindex('This is certainly a test',5), '21'
   call ch wordindex('This is certainly a   test',5), '23'
   call ch wordindex('This is certainly a test  ',5), '21'
   call ch wordindex('This is certainly a test  ',6), '0'
   call ch wordindex('This is certainly a test',6), '0'
   call ch wordindex('This is certainly a test',7), '0'
   call ch wordindex('This is certainly a test  ',7), '0'


/* ===================== WORDLENGTH ============================== */
call notify 'wordlength'
   call ch wordlength('This is certainly a test',1), '4'
   call ch wordlength('This   is   certainly a test',2), '2'
   call ch wordlength('This is certainly a test',5), '4'
   call ch wordlength('This is certainly a test ',5), '4'
   call ch wordlength('This is certainly a test',6), '0'
   call ch wordlength('',1), '0'
   call ch wordlength('',10), '0'


/* ====================== WORDPOS ============================== */
call notify 'wordpos'
   call ch wordpos('This','This is a small test'), 1
   call ch wordpos('test','This is a small test'), 5
   call ch wordpos('foo','This is a small test'), 0
   call ch wordpos('  This  ','This is a small test'), 1
   call ch wordpos('This','  This is a small test'), 1
   call ch wordpos('This','This   is a small test'), 1
   call ch wordpos('This','this is a small This'), 5
   call ch wordpos('This','This is a small This'), 1
   call ch wordpos('This','This is a small This',2), 5
   call ch wordpos('is a ','This  is a small test'), 2
   call ch wordpos('is   a ','This  is a small test'), 2
   call ch wordpos('  is a ','This  is  a small test'), 2
   call ch wordpos('is a ','This  is a small test',2), 2
   call ch wordpos('is a ','This  is a small test',3), 0
   call ch wordpos('is a ','This  is a small test',4), 0
   call ch wordpos('test  ','This  is a small test'), 5
   call ch wordpos('test  ','This  is a small test',5), 5
   call ch wordpos('test  ','This  is a small test',6), 0
   call ch wordpos('test  ','This  is a small test   '), 5
   call ch wordpos('  test','This  is a small test   ',6), 0
   call ch wordpos('test  ','This  is a small test   ',5), 5
   call ch wordpos('      ','This  is a small test'), 0
   call ch wordpos('      ','This  is a small test',3), 0
   call ch wordpos('','This  is a small test',4), 0
   call ch wordpos('test  ',''), 0
   call ch wordpos('',''), 0
   call ch wordpos('','  '), 0
   call ch wordpos('  ',''), 0
   call ch wordpos('  ','',3), 0
   call ch wordpos(' a ',''), 0
   call ch wordpos(' a ','a'), 1

   call ch find('This is a small test','This'), 1
   call ch find('This is a small test','test'), 5
   call ch find('This is a small test','foo'), 0
   call ch find('This is a small test','  This  '), 1
   call ch find('  This is a small test','This'), 1
   call ch find('This   is a small test','This'), 1
   call ch find('this is a small This','This'), 5
   call ch find('This is a small This','This'), 1
   call ch find('This is a small This','This',2), 5
   call ch find('This  is a small test','is a '), 2
   call ch find('This  is a small test','is   a '), 2
   call ch find('This  is  a small test','  is a '), 2
   call ch find('This  is a small test','is a ',2), 2
   call ch find('This  is a small test','is a ',3), 0
   call ch find('This  is a small test','is a ',4), 0
   call ch find('This  is a small test','test  '), 5
   call ch find('This  is a small test','test  ',5), 5
   call ch find('This  is a small test','test  ',6), 0
   call ch find('This  is a small test   ','test  '), 5
   call ch find('This  is a small test   ','  test',6), 0
   call ch find('This  is a small test   ','test  ',5), 5
   call ch find('This  is a small test','      '), 0
   call ch find('This  is a small test','      ',3), 0
   call ch find('This  is a small test','',4), 0
   call ch find('','test  '), 0
   call ch find('',''), 0
   call ch find('  ',''), 0
   call ch find('','  '), 0
   call ch find('','  ',3), 0
   call ch find('',' a '), 0
   call ch find('a',' a '), 1


/* ======================= WORDS ============================== */
call notify 'words'
   call ch words('This is certainly a test'), 5
   call ch words('   This is certainly a test'), 5
   call ch words('This    is certainly a test'), 5
   call ch words('This is certainly a test   '), 5
   call ch words('  hepp '), 1
   call ch words('  hepp	hepp   '), 2
   call ch words(''), 0
   call ch words('   '), 0


/* ======================= XRANGE ============================== */
call notify 'xrange'
   call ch xrange('f','r'), 'fghijklmnopqr'
   call ch xrange('7d'x,'83'x), '7d7e7f80818283'x
   call ch xrange('a','a'), 'a'



/* ======================== X2B ============================== */
   call ch x2b('416263'), '010000010110001001100011'
   call ch x2b('DeadBeef'), '11011110101011011011111011101111'
   call ch x2b('1 02 03'), '00010000001000000011'
   call ch x2b('102 03'), '00010000001000000011'
   call ch x2b('102'), '000100000010'
   call ch x2b('11 2F'), '0001000100101111'
   call ch x2b(''), ''




/* ======================== X2C ============================== */
call notify 'x2c'
   call ch x2c('416263'), 'Abc'
   call ch x2c('DeadBeef'), 'deadbeef'x
   call ch x2c('1 02 03'), '010203'x
   call ch x2c('11 0222 3333 044444'), '1102223333044444'x
   call ch x2c(''), ''
   call ch x2c('2'), '02'x
   call ch x2c('1   02   03'), '010203'x



/* ======================== X2D ============================== */
call notify 'x2d'
   call ch x2d( 'ff80', 2), '-128'
   call ch x2d( 'ff80', 1), '0'
   call ch x2d( 'ff 80', 1), '0'
   call ch x2d( '' ),      '0'
   call ch x2d( '101' ),  '257'
   call ch x2d( 'ff' ),   '255'
   call ch x2d( 'ffff'),  '65535'

   call ch x2d( 'ffff', 2), '-1'
   call ch x2d( 'ffff', 1), '-1'
   call ch x2d( 'fffe', 2), '-2'
   call ch x2d( 'fffe', 1), '-2'
   call ch x2d( 'ffff', 4), '-1'
   call ch x2d( 'ffff', 2), '-1'
   call ch x2d( 'fffe', 4), '-2'
   call ch x2d( 'fffe', 2), '-2'

   call ch x2d( 'ffff', 3), '-1'
   call ch x2d( '0fff'), '4095'
   call ch x2d( '0fff', 4), '4095'
   call ch x2d( '0fff', 3), '-1'
   call ch x2d( '07ff'), '2047'
   call ch x2d( '07ff', 4), '2047'
   call ch x2d( '07ff', 3), '2047'

   call ch x2d( 'ff7f', 1), '-1'
   call ch x2d( 'ff7f', 2), '127'
   call ch x2d( 'ff7f', 3), '-129'
   call ch x2d( 'ff7f', 4), '-129'
   call ch x2d( 'ff7f', 5), '65407'

   call ch x2d( 'ff80', 1), '0'
   call ch x2d( 'ff80', 2), '-128'
   call ch x2d( 'ff80', 3), '-128'
   call ch x2d( 'ff80', 4), '-128'
   call ch x2d( 'ff80', 5), '65408'

   call ch x2d( 'ff81', 1), '1'
   call ch x2d( 'ff81', 2), '-127'
   call ch x2d( 'ff81', 3), '-127'
   call ch x2d( 'ff81', 4), '-127'
   call ch x2d( 'ff81', 5), '65409'

   call ch x2d( 'ffffffffffff', 12), '-1'

/* ======================= JUSTIFY ============================= */
call notify 'justify'
   if intr = 'regina' Then Do
      call ch justify('Dette er en test',20,'-'), 'Dette--er---en--test'
      call ch justify('Dette er en test',10,'-'), 'Dette-er-e'

      call ch justify('  Dette er en test',25), 'Dette    er    en    test'
      call ch justify('Dette   er en test',24), 'Dette    er    en   test'
      call ch justify('Dette er   en test',23), 'Dette   er    en   test'
      call ch justify('Dette er en   test',22), 'Dette   er   en   test'
      call ch justify('Dette er en test  ',21), 'Dette   er   en  test'
      call ch justify('  Dette er en test',20), 'Dette  er   en  test'
      call ch justify('Dette   er en test',19), 'Dette  er  en  test'
      call ch justify('Dette er   en test',18), 'Dette  er  en test'
      call ch justify('Dette er en   test',17), 'Dette er  en test'
      call ch justify('Dette er en test  ',16), 'Dette er en test'
      call ch justify('  Dette er en test',15), 'Dette er en tes'
      call ch justify('Dette   er en test',14), 'Dette er en te'
      call ch justify('Dette er   en test',13), 'Dette er en t'
      call ch justify('Dette er en   test',12), 'Dette er en '
      call ch justify('Dette er en test  ',11), 'Dette er en'
      call ch justify('Dette er en test',10), 'Dette er e'

      call ch justify('Dette er en test',0), ''
      call ch justify('foo',10), 'foo       '
      call ch justify('',10), '          '
   End

   say ' '

exit 0


ch: procedure expose sigl
   parse arg first, second
   if first \== second then do
      say
      say 'first= /'first'/'
      say 'second=/'second'/'
      say 'FuncTrip: error in ' sigl':' sourceline(sigl) ; end
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

