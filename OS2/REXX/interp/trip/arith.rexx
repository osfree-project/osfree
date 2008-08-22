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
towhere = ''
zero = 0

/* ======================= simple arithmetic ======================== */

call notify 'adding'

   call ch 0.00       + 2,          "2"
   call ch 2.00       + 0,          "2.00"
   call ch 0.00       + 0.00,       "0"
   call ch 4.6        + 5.4,        "10.0"
   call ch 0.         + .0,         "0"
   call ch "-1.0"     + 1.00,       "0"
   call ch "-1.1"     + 1,          "-0.1"
   call ch 3          + 4,          "7"
   call ch 0          + 0,          "0"
   call ch 3          + "- 4",      "-1"
   call ch "  -  7  " + "  + 3  ",  "-4"

   call ch "1.000000001"  + "1.000000001",    "2.00000000"
   call ch "1.00"         + "0.0000001",      "1.0000001"
   call ch "1.00"         + "0.00000001",     "1.00000001"
   call ch "1.00"         + "0.000000001",    "1.00000000"
   call ch "1.00"         + "0.0000000001",   "1.00"
   call ch "1.00"         + "0.00000000001",  "1.00"
   call ch "0.003"        + "0.0000004",      "0.0030004"
   
   call ch "1.000000006"  + "1.000000006",    "2.00000001"
   call ch "1.000000043"  + "1.000000003",    "2.00000005"
   call ch "8.999999994"  + "1.000000004",    "10.0000000"
   call ch "1.0000000006" + "1.0000000006",   "2.00000000"
   call ch "1.000000006"  + "1.000000006",    "2.00000001"
   call ch "1.000000006"  + "1.000000006",    "2.00000001"

   call ch "999999" + ".999999", "1000000.00"
   call ch "999999000000" + "999999", "1.00000000E+12"


call notify 'multiplication' 
   call ch '5.00' * '4', '20.00'
   call ch '3.14' * '4', "12.56"
   call ch '1'    * '1',     '1'
   call ch '.1'   * '.1',    '0.01'
   call ch '.11'  * '.11',   '0.0121'
   call ch '0'    * '0',     '0'
   
   call ch '1.00001'       * '1.0001',        '1.00011000'
   call ch '1.00001'       * '1.00001',       '1.00002000'
   call ch '1.000001'      * '1.000001',      '1.00000200'
   call ch '1.0000001'     * '1.0000001',     '1.00000020'
   call ch '1.00000001'    * '1.00000001',    '1.00000002'
   call ch '1.000000001'   * '1.000000001',   '1.00000000'
   call ch '1.0000000001'  * '1.0000000001',  '1.00000000'
   call ch '1.00000000001' * '1.00000000001', '1.00000000'
   
   call ch '0.001'    * '0.001',     '0.000001'
   call ch '0.0001'    * '0.0001',     '0.00000001'
   call ch '0.00001'    * '0.00001',     '0.0000000001'
   call ch '0.000001'    * '0.000001',     '0.000000000001'
   call ch '0.0000001'    * '0.0000001',     '0.00000000000001'
   call ch '0.00000001'    * '0.00000001',     '0.0000000000000001'
   call ch '0.000000001'    * '0.000000001',     '0.000000000000000001'
   call ch '0.0000000001'    * '0.0000000001',     '1E-20'
   call ch '0.00000000001'    * '0.00000000001',     '1E-22'

   call ch '123456' * '789012', '9.74082655E+10'
   call ch '987654321' * '123456789', '1.21932631E+17'


call notify 'division'
   call ch 30 / 5, 6
   call ch 3 / 5, 0.6
   call ch 0.3 / 5, 0.06
   call ch 0.3 / 0.5, 0.6
   call ch 9 / 3, 3
   call ch 4 / 2, 2
   call ch 2 / 4, 0.5
   call ch 10 / 1, 10
   call ch 11 / 1, 11
   call ch 900 / 3, 300
   call ch 963 / 3, 321
   call ch 3 / 2, 1.5

   call ch 10 / 3, 3.33333333
   call ch 10 / 7, 1.42857143
   call ch 123.456 / 7, 17.6365714 
   call ch 7 / 123.456, 0.0567003629

   call ch 4 / 12, 0.333333333
   call ch 3.69 / 1.23, 3
   call ch 4.00 / 2, 2

   signal on syntax 
   towhere = 'after_div'
   call ch 4.00/zero, 123
   call complain 'Didn''t catch division by zero in normal division'
after_div: towhere=''

call notify 'modulus'
   call ch 7 % 3, 2
   call ch 8 % 3, 2
   call ch 9 % 3, 3
   call ch 10 % 3, 3
   call ch 11 % 3, 3
   call ch 12 % 3, 4

   call ch 4 % 3, 1
   call ch 3 % 3, 1
   call ch 2 % 3, 0
   call ch 1 % 3, 0
   call ch 0 % 3, 0
   call ch -1 % 3, 0
   call ch -2 % 3, 0
   call ch -3 % 3, -1
   call ch -4 % 3, -1

   signal on syntax 
   towhere = 'after_mod'
   call ch 4.00%zero, 123
   call complain 'Didn''t catch division by zero in integer division'
after_mod: towhere=''


call notify 'reminder'
   call ch 7 // 3, 1
   call ch 8 // 3, 2
   call ch 9 // 3, 0
   call ch 10 // 3, 1
   call ch 11 // 3, 2
   call ch 12 // 3, 0

   call ch 4 // 3, 1
   call ch 3 // 3, 0
   call ch 2 // 3, 2
   call ch 1 // 3, 1
   call ch 0 // 3, 0
   call ch -1 // 3, -1
   call ch -2 // 3, -2
   call ch -3 // 3, 0
   call ch -4 // 3, -1

   call ch -2.57 // 1.2, -0.17
   call ch 12.34 // 1, 0.34
   call ch 12.34 // 2, 0.34
   call ch 12.34 // 3, 0.34
   call ch 12.34 // 4, 0.34
   call ch 12.34 // 5, 2.34
   call ch 12.34 // 6, 0.34
   call ch 12.34 // 7, 5.34
   call ch 12.34 // 8, 4.34
   call ch 12.34 // 9, 3.34
   call ch 12.34 // 10, 2.34
   call ch 12.34 // 11, 1.34
   call ch 12.34 // 12, 0.34
   call ch 12.34 // 13, 12.34
   call ch 12.34 // 14, 12.34
   call ch 12.34 // 15, 12.34
   call ch 12.34 // 16, 12.34

   signal on syntax 
   towhere = 'after_rem'
   call ch 4.00//zero, 123
   call complain 'Didn''t catch division by zero in reminder division'
after_rem: towhere=''


call notify 'power'
   call ch 2 ** (-4), 0.0625
   call ch 2 ** (-3), 0.125
   call ch 2 ** (-2), 0.25
   call ch 2 ** (-1), 0.5
   call ch 2 ** 0, 1
   call ch 2 ** 1, 2
   call ch 2 ** 2, 4
   call ch 2 ** 3, 8
   call ch 2 ** 4, 16
   call ch 2 ** 5, 32

   call ch 2.2 ** 1, 2.2
   call ch 2.2 ** 2, 4.84


call notify 'exponential'
   call ch 1000 * 1000, 1000000
   call ch 1000 * 100000, 100000000
   call ch 1000 * 1000000, 1.00000000E+9
   call ch 1000 * 10000000, 1.00000000E+10


call notify 'numeric'
   numeric digits 1
   a = ''
   do i=8 to 15 for 9 
      a = a i
      end
  
   j = 8 * 8
   numeric digits 
   call ch a, ' 8 9 1E+1 1E+1 1E+1 1E+1 1E+1 1E+1 1E+1'
   call ch i, '1E+1'
   call ch j, '6E+1'

   say ' '


exit 0


ch: procedure expose sigl 
   parse arg first, second
   if first \== second then do
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


syntax:
   if towhere\='' then
      interpret 'signal' towhere
   
   say 'Syntax error' rc 'in line' sigl 
   say sourceline( sigl )
   exit
