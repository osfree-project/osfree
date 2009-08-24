written = 0


data.1.caption = 'do'
data.1.1 = 14 'do 3; say hi'
data.1.2 = 14 'do 3;'
data.1.3 = 25 'do forever 5; say hallo; end'
data.1.4 = ''

data.2.caption = 'end'
data.2.1 = 10 'do 1; say hi; end; end'
data.2.2 = 10 'say hi; end'
data.2.3 = ''

data.3.caption = 'if'
data.3.1 = 18 'if 1'
data.3.2 = 14 'if 1 then;;'
data.3.3 = 18 'select; when 1; else nop; end'
data.3.4 = 08 'nop; then nop'
data.3.5 = 08 'nop; else nop'
data.3.6 = 08 'if 1 then nop; else nop; else nop'
data.3.7 = 08 'select; then nop; when 1 then nop; end'
data.3.8 = 18 'if 0; else nop'
data.3.9 = 14 'if 1 then nop; else'
data.3.10= 10 'if 1 then nop; else end'
data.3.11= 10 'do 3; if 1 then nop; else end'
data.3.12= 10 'do 3; if 1 then nop; else end; end'
data.3.13= 14 'if'
data.3.14= 08 'select; when 0 then nop; then nop; end'
data.3.15= ''

data.4.caption = 'select'
data.4.1 = 09 'when 1 then nop;'
data.4.2 = 07 'select; say hallo; end'
data.4.3 = 07 'select; when 0 then nop; say hallo; end'
data.4.4 = 07 'select; when 1 then nop; nop'
data.4.5 = 14 'select; when 1 then nop'
data.4.6 = 21 'select nop; when 1 then nop; end'
data.4.7 = 07 'select; end'
data.4.8 = 07 'select; otherwise'
data.4.9 = 14 'select; when 1 then nop'
data.4.10= 14 'select; when 1 then'
data.4.11= 14 'select; when 1'
data.4.12= 35 'select; when'
data.4.13= 21 'select foo'
data.4.14= 07 'select; otherwise; end'
data.4.15= 35 'select; when then nop; end'
data.4.16= 35 'select; when; then nop; end'
data.4.17= 07 'select; when 1 then nop; nop'
data.4.18= 07 'select; end junk'
data.4.19= 07 'select; end'
data.4.20= ''

data.5.caption = 'assignment'
data.5.1 = 31 '4=5'
data.5.2 = 31 '4==5'
data.5.3 = 31 '.4=5' /* Regina wrong */
data.5.4 = 31 '.4==5' /* Regina wrong */
data.5.5 = ''

data.6.caption = 'parse'
data.6.1 = 38 'parse source . *(a) .'
data.6.2 = 38 'parse source .2e'
data.6.3 = ''

data.7.caption = 'labels'
data.7.1 = 20 "'foo': bar"
data.7.2 = ''

data.8.caption = 'strings'
data.8.1 = 6  "foo = 'bar"
data.8.2 = 6  'foo = "bar'
data.8.3 = 6  "foo = bar /* " /**/
data.8.4 = ''

data.9.caption = 'when'
data.9.1 = 09 'when 1 then nop'
data.9.2 = 09 'otherwise nop'
data.9.3 = 09 'select; when 0 then nop; otherwise; nop; otherwise; nop; end'
data.9.4 = 09 'select; when 0 then nop; otherwise; nop; when 1 then nop; end'
data.9.5 = ''

data.10.caption = 'end'
data.10.1 = 10 'end'
data.10.2 = 10 'do i=3 to 5; nop; end j'
data.10.3 = 10 "i=1; do a.i=1 to 3; j=1; end a.j"
data.10.4 = 10 'i=1; do a.i=1 to 3; end a.1'
data.10.5 = 10 'do 3; nop; if 1 then end; end'
data.10.6 = 09 'if 1 then otherwise'
data.10.7 = 09 'if 1 then when'
data.10.8 = 10 'if 1 then end'
data.10.8 = ''

data.11.caption = 'data'
data.11.1 = 13 'say `'
data.11.2 = 13 'say ['
data.11.3 = 13 'say ]'
data.11.4 = 13 'say {'
data.11.5 = 13 'say }'
data.11.6 = ''

data.12.caption ='hex/bin'
data.12.1 = 15 'a="f f f"x'
data.12.2 = 15 "a='f f f'x"
data.12.3 = 15 'a=" f f"x'
data.12.4 = 15 "a=' f f'x"
data.12.5 = 15 'a="f f "x'
data.12.6 = 15 "a='f f 'x"
data.12.7 = 15 'a="1 1 1"b'
data.12.8 = 15 "a='1 1 1'b"
data.12.9 = 15 'a=" 1 1"b'
data.12.10= 15 "a=' 1 1'b"
data.12.11= 15 'a="1 1 "b'
data.12.12= 15 "a='1 1 'b"
data.12.13= 15 "a='fgf'x"
data.12.14= 15 "a='f00l'x"
data.12.15= 15 "a='1120'b"
data.12.16= ''

data.13.caption = 'signal'
data.13.1 = 16 'signal nonexitent'
data.13.2 = 16 'signal on novalue name nonexistent; drop xxx; say xxx'
data.13.3 = 16 'signal on notready name nonexistent; say lines("/nope1")'
data.13.4 = 16 'call on notready name nonexistent; xxx=lines("/nope2")'
data.13.5 = ''

/*
 * Everything related to procedure must be checked in a separate
 * script, since it is very sensitive to the setup in this file.
 */

data.14.caption = 'strings'
data.14.1 = 19 'signal +34'
data.14.2 = 19 'signal'
data.14.3 = 19 'call'
data.14.4 = 19 'call +34'
data.14.5 = ''

data.15.caption = 'symbols'
data.15.1 = 25 'call on foobar'
data.15.2 = 25 'call on "name"'
data.15.3 = 19 'call on failure name "foobar"'
data.15.4 = 25 'call on'
data.15.5 = 25 'call on name foo'
data.15.6 = 19 'call on failure name'
data.15.7 = 20 'do i=3 to 5; nop; end "i"'
data.15.8 = 20 'do i=3 to 5; nop; end +i'
data.15.9 = 20 'do i=3 to 5; iterate "i"; end'
data.15.10= 20 'do i=3 to 5; iterate +i; end'
data.15.11= 20 'do i=3 to 5; leave "i"; end'
data.15.12= 20 'do i=3 to 5; leave +i; end'
data.15.13= 25 'signal on foobar'
data.15.14= 25 'signal on "name"'
data.15.15= 19 'signal on failure name "foobar"'
data.15.16= 25 'signal on'
data.15.17= 25 'signal on name foo'
data.15.18= 19 'signal on failure name'
data.15.19= 25 'numeric foobar'
data.15.20= 25 'numeric form foobar'
data.15.21= 25 'foobar='fff';numeric form (foobar)'
data.15.22= 35 'numeric fuzz **i'
data.15.23= 35 'numeric digits **i'
data.15.24= 21 'numeric form scientific foo'
data.15.25= 21 'numeric form engineering foo'
data.15.26= ''

data.16.caption = 'trailers'
data.16.1 = 25 'call on failure foo' /* Regina wrong */
data.16.2 = 19 'call on failure name testing foo'
data.16.3 = 25 'call off failure foo' /* Regina wrong */
data.16.4 = 25 'do forever foo; leave; end'
data.16.5 = 10 'do 3; leave; end foo'
data.16.6 = 21 'do i=1 to 3; leave; end i foo'
data.16.7 = 21 'do i=1 to 3; leave i foo; end'
data.16.8 = 21 'do i=1 to 3; iterate i foo; end'
data.16.9 = 21 'nop foo'
data.16.10= 21 'numeric form scientific foo'
data.16.11= 21 'numeric form engineering foo'
data.16.12= 21 'select foo; when 1 then nop; end'
data.16.13= 10 'select; when 1 then nop; end foo'
data.16.14= 21 'signal foo bar'
data.16.15= 19 'signal on novalue name foo bar'
data.16.16= 21 'signal off novalue name foo'
data.16.17= 21 'signal off novalue foo'
data.16.18= 21 'trace off forever'
data.16.19= 21 'trace off now'
data.16.20= ''

data.17.caption = 'trace'
data.17.1 = 24 'trace boffset'
data.17.2 = ''

data.18.caption = 'subkwd'
data.18.1 = 25 'call on failure foobar there'
data.18.2 = 25 'numeric foobar'
data.18.3 = 25 'numeric form foobar'
data.18.4 = 25 'parse foobar .'
data.18.5 = 25 'procedure foobar'
data.18.6 = 25 'signal on failure foobar there'
data.18.7 = ''

data.19.caption = 'wholenum'
data.19.1 = 26 'parse value x with . +9999999999 .'
data.19.2 = 26 'parse value x with . +9.5 .'
data.19.3 = 26 'var=1112223334; parse arg . +(var) .'
data.19.4 = 26 'var=9.5; parse value x with . +(var) .'
data.19.5 = 26 'a = 3**4.5'
data.19.6 = 26 'do 3.5; nop; end'
data.19.7 = 26 'do i=1 to 5 for 3.5; nop; end'
data.19.8 = 26 'numeric digits 5.5'
data.19.9 = 26 'numeric fuzz 1.5'
data.19.10= 26 'trace 3.5'
data.19.11= ''

data.20.caption = 'do'
data.20.1 = 27 'do 4 by 4; nop; end'
data.20.2 = 27 'do 4 to 4; nop; end'
data.20.3 = 27 'do 4 for 4; nop; end'
data.20.4 = 27 'do i=1 by 4 by 4; nop; end'
data.20.5 = 27 'do i=1 to 4 to 4; nop; end'
data.20.6 = 27 'do i=1 for 4 for 4; nop; end'
data.20.7 = 27 'do 4 until 0 while 1; nop; end'
data.20.8 = 27 'do 4 while 0 until 1; nop; end'
data.20.9 = 25 'do forever to 4; nop; end'
data.20.10= 34 'do while 0 for 5; nop; end'
data.20.11= 27 'to=1; do to+1; nop; end'
data.20.12= 27 'by=1; do by+1; nop; end'
data.20.13= 27 'for=1; do for+1; nop; end'
data.20.14= ''

data.21.caption = 'loops'
data.21.1 = 28 'iterate'
data.21.2 = 28 'iterate a'
data.21.3 = 28 'leave'
data.21.4 = 28 'leave a'
data.21.5 = 28 'do 3; iterate a; end'
data.21.6 = 28 'do 3; leave a; end'
data.21.7 = 28 'do foo=1 to 3; iterate bar; end'
data.21.8 = 28 'do foo=1 to 3; leave bar; end'
data.21.9 = 28 'do 1; interpret iterate; end'
data.21.10= 28 'do 1; interpret leave; end'
data.21.11= ''

data.22.caption = 'express'
data.22.1 = 33 'numeric digits; numeric fuzz 10'
data.22.2 = 33 'numeric fuzz 4; numeric digits 2'
data.22.3 = 33 'numeric digits 0'
data.22.4 = 26 'numeric digits -1'
data.22.5 = 26 'numeric fuzz -1'
data.22.6 = 25 'numeric form ("foobar")'
data.22.7 = 33 'do -1; nop; end'
data.22.8 = 33 'do i=1 to 3 for -1; nop; end'
data.22.9 = ''


Data.30.1 = 20 'drop "xxx"'


data.a.1 = 17 'call DoubleProc'

j = 1
   startoflooop:
   if symbol('data.j.caption')\='VAR' then signal endoftesst

   call notify data.j.caption
   i = 1
   startofloop:
      if symbol('data.j.i')\='VAR' then do
         say 'Inconsistent data for DATA.'j'.'i
         exit
         end

      if data.j.i=='' then do
         do k=i+1 for 10
            if symbol('data.j.k')=='VAR' then do
               say 'Extra data for DATA.'j'.'k
               exit
               end
            end
         signal endoftest
         end

      signal on syntax
      parse var data.j.i num string
      num = num + 0
      interpret string
         say
         say 'Didn''t catch "'string'" as syntax error' num
         signal endofloop

      syntax:
         if rc\=num then do
            say
            say 'Reports "'string'" as syntax error' rc', not' num
            say condition('e')
            end

   endofloop:
      i = i + 1
      signal startofloop

   endoftest:
endoflooop:
   j = j + 1
   signal startoflooop

endoftesst:
   say
   exit 0


notify:
   parse arg word .
   written = written + length(word) + 2
   if written>75 then do
      written = length(word)
      say ' '
   end
   call charout , word || ', '
   return


oops:
  say 'Naa har du driti paa draget igjen, Anders!'
  exit 1



