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
 * Tests the functionality of functions
 */

written = 0 

/*========= test recursivity in arguments ==============*/
call notify 'arguments'
   call ch echoargs( echoarg(1), echoarg(2) ),  "1 2"
 

call notify 'comments'
   call ch echoargs( 'foo' ), 'foo'   /* can't take comment before "(" */
 

call notify 'nullargs'
   call ch echoargs( 1, 2, , 4, , , 7 ), '1 2  4   7'
   call ch echoargs( 1, 2, ), '1 2' 
   call ch echoargs( 1, 2,, ), '1 2' 
   call ch echoargs(  , 2,, ), ' 2'
 

call notify 'labels'
   call ch foobar(), 'foo'


call notify 'external'
   signal on notready 
   call stream 'testfile', 'c', 'close'
   'rm -f testfile'
   file = 'testfile'
   call lineout file, 'if (arg(1)=0) then'
   call lineout file, '   return 0'
   call lineout file, 'else'
   call lineout file, '   return 1+"'file'"(arg(1)-1)'
   call lineout file 

   result = 'testfile'( 4,5,6 ) 
   call ch result, 4
 

call notify 'result'
   result = 'marker'
   junk = echoarg( 'foo' )
   call ch result, 'marker'   

   call echoarg 'foo' 
   call ch result, 'foo'


call notify 'builtin'
   call ch 'CENTER'('X',5), '  X  '  /* is builtin uppercase? */
/*   call ch center('X',5), '5-X-5'    /* internal before builtin? */ */



call notify 'whole'
   call ch left( 'foobar', 3.0 ), 'foo'

    

signal afterargs

center:
   return arg(2) || '-' || arg(1) || '-' || arg(2) 


echoargs:
   do i=1 to arg()
      if i=1 then
         result = arg(1)
      else
         result = result arg(i) 
      end
   return result

echoarg:
   return arg(1) 


foobar:
   return 'foo'

foobar:
   return 'bar'

afterargs:




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

