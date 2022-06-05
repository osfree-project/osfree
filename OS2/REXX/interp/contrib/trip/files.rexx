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

/* ==== config ==== */

   conf.truncate = 1   /* truncates when writing line to middle of file */

trace o
parse source os .
call time 'R'
written = 0
signal on notready

   simple = 'simple'
   if os = 'UNIX' | os = 'BEOS' | os = 'QNX' | os = 'AMIGA' | os = 'LINUX' then
     do
       eol = 'LF'
       oscopy = 'cp'
       osdel1 = 'rm -f'
       oslsl_root = 'ls -l /'
       tonul = ' > /dev/null 2>&1'
       file = '/tmp/rexx.test'
       'echo "First line" > simple'
       'echo "Second line" >> simple'
       'echo "Third line" >> simple'
       num_eol_chars = 1
       eol_chars = '0a'x
     end
   else
     do
       eol = 'CRLF'
       oscopy = 'copy'
       osdel1 = 'del'
       oslsl_root = 'dir \'
       tonul = '> nul 2>&1'
       file = 'c:\temp\rexx.tst'
       'echo First line> simple'
       'echo Second line>> simple'
       'echo Third line>> simple'
       num_eol_chars = 2
       eol_chars = '0d'x || '0a'x
     end
   osdel1 file tonul


   Parse Version Ver .
   If Left( ver, 11 ) = 'REXX-Regina' Then regina = 1
   Else regina = 0
/*signal test022*/

test001:
/* === 001 : simple write and read  ============================ */
   call notify 'simple'
   file001 = file || 001
   if (0\==lineout(file,'This is the first line')) then
      call complain 'Could not write first line'
   if (0\==lineout(file,'of a very little file')) then
      call complain 'Could not write second line'
   if (0\==lineout(file,'that will be used')) then
      call complain 'Could not write third line'
   if (0\==lineout(file,'in this trip test')) then
      call complain 'Could not write fourth line'

   call lineout file

   signal test002


test002:
/* === 002 : simple reading ======================================== */
   call notify 'read'
   call stream file, 'c', 'close'
   res = linein( file )
   if (res\=='This is the first line') then
      call complain 'didnt position at start after reopening after close'

   res = linein( file )
   res = linein( file )
   if (res \== "that will be used") then
      call complain 'Didnt position properly after several reads'

   call lineout file
   signal test003


test003:
/* === 003 : Test behavior at EOF ================================== */
   call notify 'eof'

   res = linein( file, 3 )
   res = linein( file )
   if (res\=='in this trip test') then
      call complain 'did not manage to read last line'

   signal off notready
   res = linein( file )
   if res\=='' then
      call complain 'nullstring not returned at EOF'

   call stream file, 'c', 'close'
   signal on notready

signal test004

test004:
/* === 004 : are we able to read zero lines ======================== */
   call notify 'zero_line'

   res = linein( file, 3, 0 )
   if (res\=='') then
      call complain 'Could not position without reading anything'

   res = linein( file )
   if (res\=='that will be used') then
      call complain 'Did not position correctly in the file'

   res = linein( file, 2 )
   if (res\=='of a very little file') then
      call complain 'Could not position and read simultaneously'

   call lineout file
   signal test005


test005:
/* === 005 : Will it truncate when writing new lines ===================== */
   call notify 'truncate'

   oscopy file file'.tmp' tonul
   res = lineout( file'.tmp', '<<<foobar>>>', 2 )
   if (res\=='0') then
      call complain 'non-zero result from when just positioning'

   res = linein( file'.tmp', 1 )
   if (res\=='This is the first line') then
      call complain 'first line was corrupted'

   res = linein( file'.tmp' )
   if (res\=='<<<foobar>>>') then
      call complain 'second line not properly written'

   signal off notready
   res = linein( file'.tmp' )
   if (res\=="") then
      call complain 'file was not truncated'
   signal on notready

   call lineout file'.tmp'
   signal test006



test006:
/* === 006 : Can we read characters =================================== */
   call notify 'charin'

   if eol = "LF" then
     do
       numcol1 = 15
       numcol2 = 17
       resstr1 = 'rst line'||'0a'x||'of a v'
       resstr2 = 'ery little file'||'0a'x||'t'
     end
   else
     do
       resstr1 = 'rst line'||'0d'x||'0a'x||'of a v'
       resstr2 = 'ery little file'||'0d'x||'0a'x||'t'
       numcol1 = 16
       numcol2 = 18
     end
   res = charin( file, 15, numcol1 )
   if (res \== resstr1) then
      call complain 'could not read characterbased from file'

   res = charin( file,, numcol2 )
   if (res \== resstr2) then
      call complain 'charin cant position right'

   res = linein( file )
   if (res\=='hat will be used') then
      call complain 'character and line based io does not share read pointer'

   call lineout file
   signal test007



test007:
/* === 007 : Is I/O faked when a file is in error? ==================== */

   if regina = 0 then signal test009 /* following tests only valid for Regina */

   call notify 'faking'

   exist = 'existent'
   nonexist = 'nonexistent'
   oslsl_root '>' exist
   foo = 0

   osdel1 nonexist tonul

   call on notready name trap007
   res = length(charin(nonexist,,10)) + length('cp'(exist,nonexist)) + foo +,
         length(charin(nonexist,,10)) + foo + length(linein(nonexist)) + foo +,
         length(charin(nonexist,,10)) + foo + length(linein(nonexist)) + foo
   if res\==0 then
      call complain "Condition NOTREADY don't fake further I/O properly"

   if foo\==1 then
      call complain "Help, I don't think condition handler was called"

   call lineout nonexist
   call lineout exist

   signal on notready
   signal test008



trap007:
   foo = 1
   address system
   'touch' nonexist
   'echo "Here is the contents of nonexistent" >>' nonexist
   'echo "Here is the second line of nonexistent" >>' nonexist
   'echo "Here is the third line of nonexistent" >>' nonexist
   return




test008:
/* === 008 : But I/O to other files is _not_ to be disturbed =========== */
   call notify 'nofake'

   call on notready name trap008
   exist = 'existent'
   nonexist = 'nonexistent'
   oslsl_root '>' exist
   foo = 0

   sum = 20 + length(linein(exist)) + length(linein(exist))
   call stream exist,    'c', 'close'
   call stream nonexist, 'c', 'close'

   'rm -f' nonexist
   res = length(charin(nonexist,,10)) + length('cp'(exist,nonexist))   + foo +,
         length(charin(exist,,10))    + foo + length(linein(exist,1))  + foo +,
         length(charin(nonexist,,10)) + foo + length(linein(nonexist)) + foo +,
         length(charin(exist,,10))    + foo + length(linein(exist,2))  + foo +,
         length(charin(nonexist,,10)) + foo + length(linein(nonexist)) + foo

   if res\==sum then
      call complain "Condition NOTREADY fake other files too"

   if foo\==1 then
      call complain "Help, I don't think condition handler was called"

   call lineout exist
   call lineout nonexist

   signal on notready
   signal test009

trap008:
   foo = 1
   return



test009:
/* === 009 : If rest of line is a EOL, what does linein return? ====== */
   call notify 'halfline'

   osdel1 file tonul
   if eol = 'CRLF' then do
      'echo First line>'file
      'echo Second line>>'file
      'echo Third line>>'file
   end
   else do
      'echo "First line" >'file
      'echo "Second line" >>'file
      'echo "Third line" >>'file
   end

   call stream file, 'c', 'close'

   res = charin( file,, 10 )   /* now at EOL */
   if res\=='First line' then
      call complain "Simple use of charin() failed: got:["res"] expected [First line]"

   res = linein( file )   /* must be rest of line, i.e. nullstring */
   if res\=='' then
      call complain "Linein() didn't return '' when next char was EOL; got["res"]"

   res = charin( file, 1, 10 )
   if res\=='First line' then
      call complain "Simple use of charin() does not work; got["res"] expected [First line]"

   res = linein( file, 1 )
   if res\=='First line' then
      call complain "Linein() didn't read whole line after positioning; got["res"] expected [First line]"

   call lineout file
   signal test010


test010:
/* === 010 : Are read and write positions independent? ================ */
   call notify 'crp/cwp'

   file = 'testfile'
   oscopy simple file tonul

   res = charin(file,,5)
   if res\=='First' then
      call complain "Simple use of charin() failed"

   res = charout(file,"IRST FIL",2)
   if res\==0 then
      call complain "Simple use of charout() failed"

   res = charin(file,,5)
   if res\==' FILe' then
      call complain "Charout() didn't overwrite data in file"

   res = charin(file,1,10)
   if res\=='FIRST FILe' then
      call complain "Charout() write incorrect data to file"

   res = charout(file, 'xyzzy' )
   if res\==0 then
      call complain "Continuing charout() didn't work"

   res = charin(file,, 10)
   if eol = 'CRLF' then str = "yzzycond l"
   else str = "yzzyond li"
   if res\==str then
      call complain "Charin() or charout() doesn't work; got["res"] expecting ["str"]"

   call lineout file
   signal test011




test011:
/* === : 011 I/O should only be faked when NOTREADY is trapped ======== */
   call notify 'notready'

   if eol = 'CRLF' then none = '\no\such\file'
   else none = '/no/such/file'
   foo = 0

   call stream none, 'c', 'close'
   osdel1 none tonul

   call on notready name trap011
   res = lineout(none,"Foobar") + lineout(none,"HeppHepp")
   if res\==0 then
      call complain "Don't fake line output after error output"

   res = lineout(none,"Foobar") + lineout(none,"HeppHepp")
   if res\==0 then
      call complain "Doesn't fakes line output in next line after error output"

   if foo\==2 then
      call complain "Condition trap was not called"

   signal off notready
   res = lineout(none,"Foobar") + lineout(none,"HeppHepp")
   if res\==2 then
      call complain "Fakes line output in next line after error output"

   if foo\==2 then
      call complain "Condition trap called too many times"


   call on notready name trap011
   call stream none, 'c', 'close'
   foo = 0
   res = charout(none,"Foobar") + charout(none,"HeppHepp")
   if res\==0 then
      call complain "Don't fake char output after error output"

   res = charout(none,"Foobar") + charout(none,"HeppHepp")
   if res\==0 then
      call complain "Doesn't fake char output in next line after error output"

   if foo\==2 then
      call complain "Condition trap was not called"

   signal off notready
   res = charout(none,"Foobar") + charout(none,"HeppHepp")
   if res\==14 then
      call complain "Fakes char output in next line after error output"



   call stream none, 'c', 'close'
   call off notready

   res = lineout(none,"Foobar") + lineout(none,"HeppHepp")
   if res\==2 then
      call complain "Did fake line output when not trapping NOTREADY"

   res = charout(none,"Foobar") + charout(none,"HeppHepp")
   if res\==14 then
      call complain "Did fake char output when not trapping NOTREADY"

   if foo\==2 then
      call complain "Did call trap, when not enabled"

   call lineout none
   signal on notready
   signal test012


trap011:
   foo = foo + 1
   return


test012:
/* === 012 : Illegal to position more than one after EOF ============ */
/*           in writeable files, and after EOF in readable files      */

   if regina = 0 then signal test013 /* test only valid for Regina */

   call notify 'holes'

   file = 'testfile'
   oscopy simple file tonul

   call stream file, 'c', 'close'
   lines = lines( file )
   chars = 0
   if (lines>1) then
      do lines
         chars = chars + length(linein(file)) + num_eol_chars
         end
   else
      do while lines( file )>0
         chars = chars + length(linein(file)) + num_eol_chars
         end

   call stream file, 'c', 'close'


   signal on notready name trap012a
   signal on syntax name trap012a
   foo = 0
   call charin file, chars, 0   /* at last char */
   foo = 1
trap012a:
   call stream file, 'c', 'reset'
   if foo\==1 then
      call complain "Trapped when positioning read at last char in file"


   signal on notready name trap012b
   signal on syntax name trap012b
   foo = 0
   call charin file, chars+1, 0  /* char 'after' EOF */
   foo = 1
trap012b:
   call stream file, 'c', 'reset'
   if foo\==0 then
      call complain "Didn't trap positioning read after EOF"

   signal on notready name trap012c
   signal on syntax name trap012c
   foo = 0
   call charout file,, chars+1    /* legal, don't truncate */
   foo = 1
trap012c:
   call stream file, 'c', 'reset'
   if foo\==1 then
      call complain "Trapped when positioning write at EOF+1"

   'cmp' simple file ">/dev/null"
   if rc\==0 then
      call complain "File changed when positioning at EOF+1"

   signal on notready name trap012d
   signal on syntax name trap012d
   foo = 0
   call charout file,, chars+2   /* not legal */
   foo = 1
trap012d:
   call stream file, 'c', 'reset'
   if foo\==0 then
      call complain "Not trapped when positioning write beyond EOF+1"

   'cmp' simple file tonul
   if rc\==0 then
      call complain "File changed when positioning beyond EOF+1"


   signal on notready name trap012e
   signal on syntax name trap012e
   foo = 0
   call charout file,, chars   /* legal, may not truncate */
   foo = 1
trap012e:
   call stream file, 'c', 'reset'
   if foo\==1 then
      call complain "Trapped when positioning at EOF"

   'cmp' simple file tonul
   if rc\==0  then
      call complain "File was changed when char positioning in file"

   call lineout file
   signal off syntax
   signal on notready
   signal test013




test013:
/* === 013 : A line written to the middle of a file gets a ============ */
/*           trailing, but not leading EOF ============================ */
   call notify 'linecnt'

   file = 'testfile'
   oscopy simple file tonul

   signal off notready
   call charout file,, 14+num_eol_chars
   call lineout file, "<<<..--..>>>"
   res = charin( file, 1, 30+(2*num_eol_chars) )

   if left(res,25+(2*num_eol_chars))\=="First line"eol_chars"Sec<<<..--..>>>"eol_chars then
      call complain "Problems when inserting a line into a file"

   if conf.truncate then do
      last_line_len = 25+(2*num_eol_chars)
      if length(res)\==last_line_len then
         call complain "File was not truncated. End of file contents length should be" last_line_len"; got" length(res)
      end
   else
      if length(res)\==30 & right(res,3)\=='d l' then
         call complain "Rest of line was mangled, after inserting line in file"

   call lineout file
   signal on notready
   signal test014




test014:
/* === 014 : When doing system() after fork() all files are closed, === */
/*           Maybe we can trigger flushing twice? ===================== */
   call notify 'flush'

   file = 'testfile'
   call stream file, 'c', 'close'
   call stream file, 'c', 'open write replace'
   call stream file, 'c', 'close'

   junk = charout(file, "<<<--->>>"'0a'x)

   address system 'rc 0'
   call stream file, 'c', 'close'

   if lines(file)\==1 | chars(file)\==10 then
      call complain "Possibly double flushing during file output"

   call lineout file
   signal test015



test015:
/* === 015 : Illegal to position on transient streams ================= */
/*           Default input and output are transient =================== */
   call notify 'repos'

   signal on syntax name trap015a
   signal on notready name trap015a
   res = charin(,42,0)
   call complain "Didn't trap character repositioning on default input"

trap015a:
   signal on syntax name trap015b
   signal on notready name trap015b
   res = charout(,,42)
   call complain "Didn't trap character repositioning on default output"

trap015b:
   signal on syntax name trap015c
   signal on notready name trap015c
   res = linein(,42,0)
   call complain "Didn't trap line repositioning on default input"

trap015c:
   signal on syntax name trap015d
   signal on notready name trap015d
   res = lineout(,,42)
   call complain "Didn't trap line repositioning on default output"

trap015d:

   signal on notready
   signal off syntax
   signal test016



test016:
/* === 016 : Reading EOF for persistent file raises NOTREADY ========== */
   call notify 'persist/eof'

   file = 'testfile'
   oscopy simple file tonul

   call stream file, 'c', 'close'
   call on notready name trap016
   foo = 0
   res = charin(file,28+(num_eol_chars*2),10)
   if (foo\==1) then
      call complain "Didn't raise NOTREADY when reading beyond EOF"

   call lineout file
   signal on notready
   signal test017

trap016:
   foo = 1
   if res\=='line'eol_chars then
      call complain "Data not correct when reading beyond EOF"
   return






test017:
/* === 017 : Let's try to write to <stdin> and vice versa ... ========== */
   call notify 'stdio'

   signal on notready name trap017a
   call lineout 'stdin', 'foobar'
   call complain "Didn't trap writing to default input stream"

trap017a:

   signal on notready name trap017b
   call linein 'stdout'
   call complain "Didn't trap reading from default output stream"

trap017b:

   signal on notready
   signal test018


test018:
/* === 018 : Let's read some really long lines ======================== */
   call notify 'longlines'

   maxlen = 1024 * 256
   len = 256

   do while len<maxlen
      call stream file, 'c', 'open write replace'
      hipp = copies('x',len)
      written = lineout( file, copies('x', len))
      if written\=0 then
         call complain "Didn't manage to write out all the data"
      hopp = copies('x',len)
      line = linein( file, 1 )
      if line\==copies('x',len) | len\=length(line) then
         call complain "Data read don't equal data written"
      len = trunc(len * 1.5, 0) - 10
      end

   call lineout file
   osdel1 file tonul
   drop hopp hipp line
   signal test019


test019:
/* === 019 : Are we able to open 200 files? =========================== */
   call notify 'many'

   osdel1 'a*.tmp' tonul

   mf = 130
   do i=1 to mf
      res1 = lineout( "a" || i || ".tmp", "line 1 in file" i "of test")
      res2 = lineout( "a" || i || ".tmp", "line 2 in file" i "of test")
      if (res1 + res2 > 0) then do
         call complain "Didn't manage to write to all" mf "files (only "i")"
         signal trip019
         end
      end

   do i=1 to 30
      fileno = 1 + (i*5) // (mf-1)
      lineno = random( 1, 2 )

      res = linein( "a" || fileno || ".tmp", lineno ) ;
      if (res\=="line" lineno "in file" fileno "of test") then do
         call complain "Didn't manage to read back the data written"
         leave ;
         end
      end

trip019:
   do i=1 to mf
      call stream "a" || i || ".tmp", 'c', 'close' ;
      end

   osdel1 'a*.tmp' tonul
   signal test020



test020:
/* === 020 : Does chars() or lines() make an unopened file stick ======== */
/*           in the rexx file table? ==================================== */
call notify 'stick'

   time = '1'

   first020:
      signal on notready name notr020a
      call stream 'hepp', 'c', 'close'
      oscopy 'simple hepp' tonul

      if time==2 then
         call linein 'hepp', 1, 0

      signal on syntax name trap020
      if lines('hepp','C')\==3 then
         call complain "Simple use of lines() didn't work"
      signal off syntax

      call stream 'hepp', 'c', 'close' /* must close before delete */
      osdel1 'hepp' tonul
      line = linein('hepp',1)
      if time == 1 then do
         if line='' then
            call complain "NOTREADY not triggered when reading nonex. file"
         else if line=='First line' then
            call complain "File stuck in memory when counting lines()"
         else
            call complain "Something is wrong with lines(), I think"
         end
      else
         if line \== "First line" then
            call complain "File wasn't found in memory"

notr020a:
   if time=1 then do
      time = 2
      signal first020
      end



/* Then, repeat for charin() */

   time = '1'
   second020:
      signal on notready name notr020b
      call stream 'hepp', 'c', 'close'
      oscopy 'simple hepp' tonul

      if time==2 then
         call lineout 'hepp', "hepp"   /* opens for read and write */

      hepp_size = 31 + (num_eol_chars * 3)
      line_size = 4 + num_eol_chars
      if chars('hepp')\==hepp_size + (time==2)*line_size then
         call complain "Simple use of chars() didn't work"

      call stream 'hepp', 'c', 'close' /* must close before delete */
      osdel1 'hepp' tonul
      line = linein('hepp',1)
      if time == 1 then do
         if line='' then
            call complain "NOTREADY not triggered when reading nonex. file"
         else if line=='First line' then
            call complain "File stuck in memory when counting chars()"
         else
            call complain "Something is wrong with chars(), I think"
         end
      else
         if line \== "First line" then
            call complain "File wasn't found in memory"

notr020b:
   if time=1 then do
      time = 2
      signal second020
      end

   call lineout hepp
   signal on notready
   signal test021

trap020:
   call complain "Interpreter does not support ANSI. LINES( fn, 'C' ) is valid ANSI."


test021:
/* === 021 : Test that close *always* close a file, even if ============= */
/*           the file in error is in error state ======================== */
call notify 'close'
   call on notready name notr021

   file = simple
   res = charin(file,,5) charin(file,500),
                     stream(file,'c','close') charin(file,,5)
   if res\=='foo' then
      call complain "NOTREADY condition was not triggered"

   call lineout file
   signal on notready
   signal test022

notr021:
   if res="First line" then
      call complain "Something very strange happened, failure not noticed"
   else if res='First' then
      call complain "Statement interrupted during execution"
   else if res\="First  UNKNOWN First" then
      call complain "Didn't manage to read correct data"
   res = 'foo'

   return



test022:
/* === 022 : Does empty output without repositioning close the file? ===== */
call notify 'empty_out(default)'
   signal off notready

   call stream simple, 'c', 'close'
   line = linein( simple ) ;

   if line\=="First line" then
      call complain "STREAM(fn,'C','CLOSE') doesn't seem to have closed file"

   call lineout simple
   line = linein( simple )

   if line\=="First line" then
      call complain "LINEOUT(fn) should be the same as 'CLOSE'."

   call stream simple, 'c', 'close'
   signal on notready
   signal test022a

test022a:
/* === 022 : Does empty output without repositioning close the file? STRICT_ANSI ===== */
call notify 'empty_out(STRICT_ANSI)'
   signal off notready
   OPTIONS STRICT_ANSI

   call stream simple, 'c', 'close'
   line = linein( simple ) ;

   if line\=="First line" then
      call complain "STREAM(fn,'C','CLOSE') doesn't seem to have closed file"

   call lineout simple
   line = linein( simple )

   if line\=="Second line" then
      call complain "LINEOUT(fn) should be different to 'CLOSE' according to STRICT_ANSI."

   call stream simple, 'c', 'close'
   OPTIONS NOSTRICT_ANSI
   signal on notready
   signal test023


test023:
/* === 023 : Line positioning of file using STREAM ===== */
call notify 'line_pos'
   signal off notready

   call stream simple, 'c', 'close'
   call stream simple, 'c', 'open write replace'
   do i = 1 to 100
      call lineout simple, Left('line 'i, 10, '*')
   end
   if eol = 'LF' then line_length = 11
   else line_length = 12
   call stream simple, 'c', 'close'

   cmd = 'OPEN BOTH'
   ret = stream( simple, 'c', cmd ); correct = 'READY:'
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') didn't work: got:" ret 'instead of:' correct
   call checkseekpos 1, 1, 1+(100*line_length), 101

   /*
    * Move read pos to line 30, write pos to line 70
    */
   cmd = 'SEEK =30 READ LINE'
   ret = stream( simple, 'c', cmd ); correct = 30
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') didn't position correctly got:" ret 'instead of:' correct
   call checkseekpos 1+(29*line_length) , 30, 1+(100*line_length), 101

   cmd = 'SEEK =70 WRITE LINE'
   ret = stream( simple, 'c', cmd ); correct = 70
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') didn't position correctly got:" ret 'instead of:' correct
   call checkseekpos 1+(29*line_length) , 30, 1+(69*line_length), 70

   /*
    * Move read pos to line 60, write pos to line 50
    */
   cmd = 'SEEK +30 READ LINE'
   ret = stream( simple, 'c', cmd ); correct = 60
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') didn't position correctly got:" ret 'instead of:' correct
   call checkseekpos 1+(59*line_length) , 60, 1+(69*line_length), 70

   cmd = 'SEEK -20 WRITE LINE'
   ret = stream( simple, 'c', cmd ); correct = 50
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') didn't position correctly got:" ret 'instead of:' correct
   call checkseekpos 1+(59*line_length) , 60, 1+(49*line_length), 50

   /*
    * Move read pos to line 50, write pos to line 40
    */
   cmd = 'SEEK -10 READ LINE'
   ret = stream( simple, 'c', cmd ); correct = 50
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') didn't position correctly got:" ret 'instead of:' correct
   call checkseekpos 1+(49*line_length) , 50, 1+(49*line_length), 50

   cmd = 'SEEK -10 WRITE LINE'
   ret = stream( simple, 'c', cmd ); correct = 40
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') didn't position correctly got:" ret 'instead of:' correct
   call checkseekpos 1+(49*line_length) , 50, 1+(39*line_length), 40

   /*
    * Should now be able to read line 50
    */
   ret = linein( simple ); correct = 'line 50***'
   if ret \= correct then
      call complain "LINEIN(fn) returns the wrong value got:" '['ret']' 'instead of:' '['correct']'
   /* read pos should now be at line 51 */
   call checkseekpos 1+(50*line_length) , 51, 1+(39*line_length), 40

   /*
    * Move read pos to line 91, write pos to line 81
    */
   cmd = 'SEEK <10 READ LINE'
   ret = stream( simple, 'c', cmd ); correct = 91
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') didn't position correctly got:" ret 'instead of:' correct
   call checkseekpos 1+(90*line_length) , 91, 1+(39*line_length), 40

   cmd = 'SEEK <20 WRITE LINE'
   ret = stream( simple, 'c', cmd ); correct = 81
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') didn't position correctly got:" ret 'instead of:' correct
   call checkseekpos 1+(90*line_length) , 91, 1+(80*line_length), 81

   call stream simple, 'c', 'close'
   signal on notready


/* CHARIN():

   * If name is omitted, I/O skjer til standard input/output
   * Lesing starter normalt paa current read position
   * Error naar start er etter EOF
   * Dersom lengde er null, skjer bare positionering
   * Dersom det er sjanse for at flere tegn blir tilgjengelig
     (transiente) er I/O blokket.
   * EOF paa persistent filer gir NOTREADY

   CHAROUT():

   * Returnerer remaining chars
   * Hvis string=nullstring, => rc=0
   * Default write=\= default read position
   * Initial read position and write position
   * Start for write kan spesifisere size(file)+1, mens read bare
     kan spesifisere size(file)
   * Dersom string=tom og start=tom, saa settes cur write pos til
     EOF, og evt filen lukkes
   * Utskrift er blokket, eller gir feil paa persistent files

   CHARS()

   * Antallet inkluderer lkinjeskift
   * Returnerer 1 dersom ukjent

   LINEIN():

   * Kan returnere delvise linjer i kombinasjon med charing
   * Dersom start er oppgitt, starter read alltid paa starten av en
     linje, selv om start er current linje, og current er paa
     midten av linja.
   * Blokkerer I/O til fullstendig linje er tilgjenglig (transient)
   * Retuernerer ufullstendig linje og trigger NOTREADY for EOF
     paa persistent files

   LINEOUT():

   * String='' skriver ut ebn EOL
   * Ved skriving til en file, terminerer lionjer *etter* nu linje,
     ikke foran

   LINES():

   * Antall hele linjher som er igjen: a) halv linje paa slutten teller
     ikke, mens b) halv linje paa starten telles

   * Pull/linein og say/lineout bruker samme interface.

   * Status for en stream lagres ikke over subroutine kall

   Vanskelige ....

   * Skrive til <stdin> og lese fra <stdout>
   * Aapne filer en masse, og se om noe skjaerer seg.
   * Les lange (svaert lange) linjer
   * Proev aa forvirre med filer der siste linjer ikke har EOL
   * Posisjoner til en linjer som ikke finnes
   * Skrive inn, eller fjern EOL fra fil, og se om det forvirrer
     linjetellingen. Gjoer skrivingen baade innefra og utenfra Rexx
   * Apne tom filer for lesing
   * La lesepos vaere langt ute i fila, trunker paa et tidligere punkt
   * Forsoek posisjonering paa standard input og output
   * Test at lesing/skriving avc ASCII NUL fungerer baade for
     character og linje basert I/O
   * Klarer vi aa lang kroell med /dev/null?  Vi skal ikke kunne lese,
     og ved skriving skal det bli kroell i linjetellingen
   * Har leseaccess; proever aa skrive, mistes leseakksess?
   * Omvendt, dersom vi starter med leseaccess.
   * Har leseaccess, denne fjernes (Av eksternt program), forsoeker aa
     faa skriveaksess, mistes leseacckess
   * samme for skriveaccess
   * Aapner fil for leseakses, flytter fila, aapner for skriveakksess
   * Kan vi lure den til aa flush'e to ganger: ved fork() og close()

   * Kan vi bruke cd() til aa faa den til aa operere paa en annen fil?

   * Naa trigger syntax dersom vi proever aa positionere oss til et
     punkt etter eof.

*/



   test999:
   say ' '
say time('E') "seconds used"
exit 0

notready:
   say ' ...'
   say 'Got an unwanted NOTREADY ... Ouch!'
   say 'cond("d")='condition("d")
   length = charout(,'   (')
   return


ch: procedure expose sigl
   trace off
   parse arg first, second
   if first \== second then do
      say
      say 'first  = /'first'/'
      say 'second = /'second'/'
      say "FuncTrip: error in " sigl":" sourceline(sigl) ; end
   return


complain:
   trace off
   say ' ...'
   if arg() = 2 then lineno = arg(2)
   else lineno = sigl
   say 'Tripped in line' lineno':' arg(1)'.'
   length = charout(,'   (')
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

checkseekpos: Procedure Expose simple sigl
   parse arg readchar, readline, writechar, writeline

   lineno = sigl
   cmd = 'QUERY SEEK READ CHAR'
   ret = stream( simple, 'c', cmd ); correct = readchar
   if ret \= correct then
      call complain "STREAM( fn,'C','"cmd"') returns the wrong value got:" ret 'instead of:' correct, lineno

   cmd = 'QUERY SEEK READ LINE'
   ret = stream( simple, 'c', cmd ); correct = readline
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') returns the wrong value got:" ret 'instead of:' correct, lineno

   cmd = 'QUERY SEEK WRITE CHAR'
   ret = stream( simple, 'c', cmd ); correct = writechar
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') returns the wrong value got:" ret 'instead of:' correct, lineno

   cmd = 'QUERY SEEK WRITE LINE'
   ret = stream( simple, 'c', cmd ); correct = writeline
   if ret \= correct then
      call complain "STREAM(fn,'C','"cmd"') returns the wrong value got:" ret 'instead of:' correct, lineno

   return
