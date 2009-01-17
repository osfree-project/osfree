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

/* testing the implementation of the stack */
trace o
written = 0
options MAKEBUF_BIF DROPBUF_BIF DESBUF_BIF

/* first check PUSH */
call notify 'push'
   push 'first'
   push 'second'
   pull one
   pull two

   if (one^=="SECOND") | (two^=="FIRST") then
      call complain 'The PUSH statement does not work'

call notify 'queue'
   queue 'first'
   queue 'second'
   pull one
   pull two

   if (one^=="FIRST") | (two^=="SECOND") then
      call complain 'The QUEUE statement does not work'

   queue
   push
   pull one
   pull two

   if (one^=='') | (two^=="") then
      call complain 'Either QUEUE or PUSH do not stack empty lines'

/* the queued() function */
call notify 'queued'
   call ch queued(), 0
   push 'one'
   call ch queued(), 1
   push 'two'
   queue 'three'
   call ch queued(), 3

   parse pull first
   call ch first, 'two'
   parse pull second
   call ch second, 'one'
   parse pull third
   call ch third, 'three'
   call ch queued(), 0

   push 'one'
   push 'two'
   call makebuf
   push 'three'
   call ch queued(), 3
   push 'four'
   call makebuf
   call ch queued(), 4
   push 'five'

   call dropbuf
   call ch queued(), 4
   call dropbuf
   call ch queued(), 2
   call dropbuf
   call ch queued(), 0
   call dropbuf
   call ch queued(), 0



call notify 'commands'

   address system

/*----- Let's see if it can pipe the stack ----------------------------*/

   queue 'To be or not to be'
   'LIFO> tr abcdefghijkl ABCDEFGHIJKL >LIFO'

   parse pull line
   if line^=='To BE or not to BE' then
      call complain 'Can not pipe the stack-contents through a command'

   queue 'To be or not to be'
   Address System 'tr abcdefghijkl ABCDEFGHIJKL' With Input LIFO '' Output LIFO ''

   parse pull line
   if line^=='To BE or not to BE' then
      call complain 'Can not pipe the stack-contents through a command using ANSI extensions'

/*----- does it clear the stack when command just read a few lines ----*/
   queue 'asdf asdf'
   queue 'zxcv zxcv'
   queue 'tyui tyui'
   queue 'ghjk ghjk'

   'LIFO> head -2 | wc >LIFO'
   if queued()^=='1' then
      call complain 'Commands does not clear stack'

   parse pull lines words chars .
   if lines words chars^=='2 4 20' then
      call complain 'Something is incorrect in the stack'

   queue 'asdf asdf'
   queue 'zxcv zxcv'
   queue 'tyui tyui'
   queue 'ghjk ghjk'

   Address System 'head -2 | wc' With Input LIFO '' Output LIFO ''
   if queued()^=='1' then
      call complain 'Commands does not clear stack using ANSI extensions'

   parse pull lines words chars .
   if lines words chars^=='2 4 20' then
      call complain 'Something is incorrect in the stack using ANSI extensions'

/*----- let's see if it really send it through system(3) ---------------*/
/* sorry, too many machines don't set the USER environment variable. */
/*
   address system

   'whoami >LIFO'
   'echo $USER >LIFO'
   parse pull myself .
   parse pull alsome .
   if myself^==alsome then
      call complain 'Command did not resolve environment variable'
*/

/*----- let's take it for some heavy piping ----------------------------*/
signal next_test
   dict_words = '/usr/dict/words'
   if ^stream(dict_words, 'c', 'readable') then do
      'man sh > /tmp/dict 2>/dev/null'
      'man csh >> /tmp/dict 2>/dev/null'
      'man vi >> /tmp/dict 2>/dev/null'
      'cat /tmp/dict | tr -cs ''a-zA-z0-9'' '' '' | sort -u > /tmp/dict_words'
      dict_words = '/tmp/dict_words'
   end

   words = 'wc'('-l dict_words')
   'cat' dict_words '>LIFO'
   if words != queued() then
      call complain 'Could not get all short lines to the stack'

   'LIFO> cat >LIFO'
   if words != queued() then
      call complain 'Could not correctly run short lines on the stack'

   'LIFO> sort -0.3 >LIFO'
   if words != queued() then
      call complain 'Could not correctly "rotate" short lines on the stack'

   'LIFO> wc -l >LIFO'
   if queued() != 1 then
      call complain 'Could not flush short lines into the stack'

   parse pull lines .
   if words != lines then
      call complain 'Not all short lines flushed got through'

/*----- then, let's try the same thing with some longer lines ----------*/
   oldtrace = trace()
   trace off
   'awk "BEGIN{while(1){print \"yes\"};}"' copies('X',10) '| head -10000 >LIFO'
   trace oldtrace
   if queued()!=10000 then
      call complain 'Could not put 10000 lines to the stack'

   'LIFO> cat >LIFO'
   if  queued() != 10000 then
      call complain 'Could not run long lines through cat'

   'LIFO> sort >FIFO'
   if queued() != 10000 then
      call complain 'Could not rotate long lines on the stack'

   'LIFO> wc -l >LIFO'
   if queued() != 1 then
      call complain 'Could not flush long stack to program'

next_test:
/*----- The nullstring is a valid command ------------------------------*/
/*
 * This is kind of hard to check, but at least we can try to send a
 * nullstring as a command, and try to catch any errors that might
 * occur from it.
 */
call notify 'empty command'

   ""

/* The setting of RC variable checked in signal.rexx */



call notify 'buffers'
/*
 * try to check the performance of buffers. However, this is
 * specific to some implementations
 */
   do queued() ; pull ; end   /* just in case ... */

   res = makebuf() makebuf() makebuf()
   if res \== '1 2 3' then
      call complain 'MAKEBUF does not return correct return value'


   res = dropbuf() dropbuf() dropbuf() dropbuf() dropbuf()
   if res \= '2 1 0 0 0' then
      call complain 'DROPBUF does not return correct return value'


   push 'second'
   push 'third'
   call makebuf
   push 'fourth'
   call dropbuf
   parse pull line1
   parse pull line2
   if line1\='third' | line2\='second' then
      call complain 'DROPBUF kills line below the buffer'

   call makebuf
   push 'first'
   call makebuf
   push 'second'
   parse pull line1
   parse pull line2
   brc = makebuf()
   if line1\='second' | line2\='first' | brc\='2' then
      call complain 'Reading lines doesn''t remove buffers'

   call desbuf
   push 'first'
   push 'second'
   res= dropbuf()
   if queued()>0 | res\=0 then
      call complain 'DROPBUF does not remove ''zeroth'' buffer'


   call desbuf
   call makebuf
   call makebuf
   push 'first'
   call makebuf
   push 'second'
   call makebuf
   push 'third'
   call makebuf
   call makebuf
   res = dropbuf(-3)
   parse pull line
   if res\=3 | line\='second' then
      call complain 'DROPBUF with negative argument did not work'

   call desbuf
   push 'first'
   call makebuf
   push 'second'
   res = dropbuf(0)
   if queued()\=0 | res\=0 then
      call complain 'DROPBUF with zero argument didn''t clear stack'



say ''
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
   parse arg word
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


