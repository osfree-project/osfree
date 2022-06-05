/* compares the result of shortcut assignments to original assignment operators */
/* eg. x+= 1 should be the same as x = x + 1 */
Signal On Syntax

written = 0
towhere = ''

Call notify 'addition assign'
Call ch 0.00       , '+', 2
Call ch 2.00       , '+', 0
Call ch 0.00       , '+', 0.00
Call ch 4.6        , '+', 5.4
Call ch 0.         , '+', .0
Call ch '-1.0'     , '+', 1.00
Call ch '-1.1'     , '+', 1
Call ch 3          , '+', 4
Call ch 0          , '+', 0
Call ch 3          , '+', '- 4'
Call ch '  -  7  ' , '+', '  + 3  '

Call ch '1.000000001'  , '+', '1.000000001'
Call ch '1.00'         , '+', '0.0000001'
Call ch '1.00'         , '+', '0.00000001'
Call ch '1.00'         , '+', '0.000000001'
Call ch '1.00'         , '+', '0.0000000001'
Call ch '1.00'         , '+', '0.00000000001'
Call ch '0.003'        , '+', '0.0000004'

Call ch '1.000000006'  , '+', '1.000000006'
Call ch '1.000000043'  , '+', '1.000000003'
Call ch '8.999999994'  , '+', '1.000000004'
Call ch '1.0000000006' , '+', '1.0000000006'
Call ch '1.000000006'  , '+', '1.000000006'
Call ch '1.000000006'  , '+', '1.000000006'

Call ch '999999' , '+', '.999999'
Call ch '999999000000' , '+', '999999'

Call notify 'subtraction assign'
Call ch 0.00       , '-', 2
Call ch 2.00       , '-', 0
Call ch 0.00       , '-', 0.00
Call ch 4.6        , '-', 5.4
Call ch 0.         , '-', .0
Call ch '-1.0'     , '-', 1.00
Call ch '-1.1'     , '-', 1
Call ch 3          , '-', 4
Call ch 0          , '-', 0
Call ch 3          , '-', '- 4'
Call ch '  -  7  ' , '-', '  + 3  '

Call ch '1.000000001'  , '-', '1.000000001'
Call ch '1.00'         , '-', '0.0000001'
Call ch '1.00'         , '-', '0.00000001'
Call ch '1.00'         , '-', '0.000000001'
Call ch '1.00'         , '-', '0.0000000001'
Call ch '1.00'         , '-', '0.00000000001'
Call ch '0.003'        , '-', '0.0000004'

Call ch '1.000000006'  , '-', '1.000000006'
Call ch '1.000000043'  , '-', '1.000000003'
Call ch '8.999999994'  , '-', '1.000000004'
Call ch '1.0000000006' , '-', '1.0000000006'
Call ch '1.000000006'  , '-', '1.000000006'
Call ch '1.000000006'  , '-', '1.000000006'

Call ch '999999' , '-', '.999999'
Call ch '999999000000' , '-', '999999'

Call notify 'multiplication assign'
Call ch '5.00' , '*', '4'
Call ch '3.14' , '*', '4'
Call ch '1'    , '*', '1'
Call ch '.1'   , '*', '.1'
Call ch '.11'  , '*', '.11'
Call ch '0'    , '*', '0'

Call ch '1.00001'       , '*', '1.0001'
Call ch '1.00001'       , '*', '1.00001'
Call ch '1.000001'      , '*', '1.000001'
Call ch '1.0000001'     , '*', '1.0000001'
Call ch '1.00000001'    , '*', '1.00000001'
Call ch '1.000000001'   , '*', '1.000000001'
Call ch '1.0000000001'  , '*', '1.0000000001'
Call ch '1.00000000001' , '*', '1.00000000001'

Call ch '0.001'    , '*', '0.001'
Call ch '0.0001'    , '*', '0.0001'
Call ch '0.00001'    , '*', '0.00001'
Call ch '0.000001'    , '*', '0.000001'
Call ch '0.0000001'    , '*', '0.0000001'
Call ch '0.00000001'    , '*', '0.00000001'
Call ch '0.000000001'    , '*', '0.000000001'
Call ch '0.0000000001'    , '*', '0.0000000001'
Call ch '0.00000000001'    , '*', '0.00000000001'

Call ch '123456' , '*', '789012'
Call ch '987654321' , '*', '123456789'


Call notify 'division assign'
Call ch 30 , '/', 5
Call ch 3 , '/', 5
Call ch 0.3 , '/', 5
Call ch 0.3 , '/', 0.5
Call ch 9 , '/', 3
Call ch 4 , '/', 2
Call ch 2 , '/', 4
Call ch 10 , '/', 1
Call ch 11 , '/', 1
Call ch 900 , '/', 3
Call ch 963 , '/', 3
Call ch 3 , '/', 2

Call ch 10 , '/', 3
Call ch 10 , '/', 7
Call ch 123.456 , '/', 7
Call ch 7 , '/', 123.456

Call ch 4 , '/', 12
Call ch 3.69 , '/', 1.23
Call ch 4.00 , '/', 2


Call notify 'modulus assign'
Call ch 7 , '%', 3
Call ch 8 , '%', 3
Call ch 9 , '%', 3
Call ch 10 , '%', 3
Call ch 11 , '%', 3
Call ch 12 , '%', 3

Call ch 4 , '%', 3
Call ch 3 , '%', 3
Call ch 2 , '%', 3
Call ch 1 , '%', 3
Call ch 0 , '%', 3
Call ch -1 , '%', 3
Call ch -2 , '%', 3
Call ch -3 , '%', 3
Call ch -4 , '%', 3

Call ch 818 , '%', 9

Call notify 'remainder assign'
Call ch 7 , '//', 3
Call ch 8 , '//', 3
Call ch 9 , '//', 3
Call ch 10 , '//', 3
Call ch 11 , '//', 3
Call ch 12 , '//', 3

Call ch 4 , '//', 3
Call ch 3 , '//', 3
Call ch 2 , '//', 3
Call ch 1 , '//', 3
Call ch 0 , '//', 3
Call ch -1 , '//', 3
Call ch -2 , '//', 3
Call ch -3 , '//', 3
Call ch -4 , '//', 3

Call ch -2.57 , '//', 1.2
Call ch 12.34 , '//', 1
Call ch 12.34 , '//', 2
Call ch 12.34 , '//', 3
Call ch 12.34 , '//', 4
Call ch 12.34 , '//', 5
Call ch 12.34 , '//', 6
Call ch 12.34 , '//', 7
Call ch 12.34 , '//', 8
Call ch 12.34 , '//', 9
Call ch 12.34 , '//', 10
Call ch 12.34 , '//', 11
Call ch 12.34 , '//', 12
Call ch 12.34 , '//', 13
Call ch 12.34 , '//', 14
Call ch 12.34 , '//', 15
Call ch 12.34 , '//', 16

Call notify 'or assign'
Call ch 1 , '|', 1
Call ch 1 , '|', 0
Call ch 0 , '|', 0
Call ch 0 , '|', 1

Call notify 'and assign'
Call ch 1 , '&', 1
Call ch 1 , '&', 0
Call ch 0 , '&', 0
Call ch 0 , '&', 1

Call notify 'xor assign'
Call ch 1 , '&&', 1
Call ch 1 , '&&', 0
Call ch 0 , '&&', 0
Call ch 0 , '&&', 1

Call notify 'concat assign'
Call ch 'abc' , '||', 'def'
Call ch "''" , '||', 'def'
Call ch 'abc' , '||', "''"
Call ch 'abc' , '||', ' spaces '

Exit

/* compares the result of shortcut assignments to original assignment operators */
ch: Procedure
Parse Arg initial, op, second
x.1 = initial
x.2 = initial
x1 = initial
x2 = initial
str1 = 'x.1' op'=' second
str2 = 'x.2 = x.2' op second
str3 = 'x1' op'=' second
str4 = 'x2 = x2' op second
Interpret str1
Interpret str2
If x.1 \== x.2 Then
   Do
      Say 'Error in assignment between: "'str1'" and "'str2'". Results: "'x.1'" NOT "'x.2'"'
      Say 'Initial values of x.1 and x.2 are:' initial
   End
Interpret str3
Interpret str4
If x1 \== x2 Then
   Do
      Say 'Error in assignment between: "'str3'" and "'str3'". Results: "'x1'" NOT "'x2'"'
      Say 'Initial values of x1 and x2 are:' initial
   End
Return

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


syntax:

   say 'Syntax error' rc 'in line' sigl
   say sourceline( sigl )
   exit
