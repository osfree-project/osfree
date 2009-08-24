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

/* ======================= DATE ============================== */
/* Not a Nice way to do it, but probably the only way ...      */

/* Sun (ofcourse) managed to define their own set of formats, argh! */
Week   = 'Sunday Monday Tuesday Wednesday Thursday Friday Saturday'
Mons   = 'Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec'
Months = 'January February March April May June July August',
         'September October November December'

parse source os .
If os = 'UNIX' | os = 'BEOS' | os = 'QNX' | os = 'SKYOS' Then Signal beforedate

call notify 'date, time NOT TESTABLE'
exit 0

beforedate:
call notify "date"
/* call ch date('C'),   'date'("+%j")+trunc(('date'("+%y")-1)*365.25)+365 */
   call ch date('D'),   strip('date'("+%j"),'L','0')
   call ch date('E'),   'date'("+%d/%m/%y")
   call ch date('M'),   word(Months,'date'("+%m"))
   call ch date('N'),   strip('date'("+%d"),'L','0') word(Mons,'date'("+%m")),
                        "20" || 'date'('+%y')
   call ch date('O'),   'date'("+%y/%m/%d")
   call ch date('S'),   "20" || 'date'("+%y%m%d")
   call ch date('U'),   'date'("+%D")
   call ch date('W'),   word(Week,'date'("+%w")+1)
   call ch date(),      date('N')
afterdate:



/* ======================== TIME ============================== */
call notify 'time'
   /* this will not work at midnight or noon  */
   parse value time(c) time(h) time(l) time(m) time(n) time(s),
          with timec timeh timel timem timen times

   parse value 'date'('"+%H %M %S"') with hour minute second

   if ( abs((hour*60*60 + minute*60 + second)-times) > 3) then
      say 'uppps, time is really not that accurate ....'

   /* time(c) */

   if (times > 12*3600) then do
      times2 = times - 12*3600
      suff = 'pm'
      end
   else do
      times2 = times
      suff = 'am'
      end

   if ((times2%3600)=0) then
      times2 = 12*3600

   hr = times2%3600
   if (hr==0) then
      hr = '12'
   call ch timec, hr':'right((times%60)//60,2,'0')suff

   /* time(h) */
   call ch timeh, times%3600

   /* time(m) */
   call ch timem, times%60

   /* time(n) */
   call ch timen, right(times%3600,2,"0") || ":" ||,
             right((times%60)//60,2,"0") || ":" ||,
             right((times//60),2,"0")

   /* time(l) */
   parse var timel hour ':' minutes ":" secs '.' after
   if (after > 499999) then
      secs = secs + 1
   if (secs == 60) then do
      secs = 0
      minutes = minutes + 1
      end
   if (minutes == 60) then do
      minutes = 0
      hour = hour + 1
      end    /* ok ... let's just assume it is not midnight .... */

   timel = right(hour,2,'0')':'right(minutes,2,'0')':'right(secs,2,'0')
   call ch timel, timen

   /* time(s) ... should be ok :-) */
   call time 'r'
   'sleep 3'
   parse value time(e) time(r) time(e) with one two three .

   call ch three, '.000000'
   call ch one, two
   call ch one%1, 3
aftertime:



   say ' '
exit 0


ch: procedure expose sigl
   parse arg first, second
   if first ^== second then do
      say
      say 'first  = /'first'/'
      say 'second = /'second'/'
      say "FuncTrip: error in line" sigl":" sourceline(sigl) ; end
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

