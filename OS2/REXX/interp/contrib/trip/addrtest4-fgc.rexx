/*
 * Another ADDRESS...WITH test program
 * Ideally the argument should be a large text file
 * Requires a diff and wc executable and filter.c compiled to an executable
 */
Parse Arg infn
If Stream( infn, 'C', 'QUERY EXISTS' ) = '' Then
   Do
      Say infn 'does not exist.'
      Exit 1
   End
infnsize = Stream( infn, 'C', 'QUERY SIZE' ) + 10
outfn = infn'.out'
Parse Source os .
Select
   When Left( os, 3 ) = 'WIN' Then
      Do
         _diff = 'qdiff'
         _cat = 'type'
         _rm = 'del'
         _filter = '.\filter'
         _wc = 'wc'
      End
   When os = 'OS/2' Then
      Do
         _diff = 'diff'
         _cat = 'type'
         _rm = 'del'
         _filter = '.\filter'
         _wc = 'wc'
      End
   Otherwise
      Do
         _diff = 'diff'
         _cat = '/bin/cat'
         _rm = 'rm'
         _filter = './filter'
         _wc = 'wc'
      End
End
_rm 'filter.log'
Call Mix
Call RoundTrip
Call Blob
Return 0

Mix:
Say 'Running Mix: ************************************'
-- split input file into stdout and stderr half/half
Address System _filter infnsize 'line stderr' with input stream infn output stem out. error stem err.
Say 'Have split' infn 'in half. Lines sent to stdout:' out.0'; lines sent to stderr:' err.0'.'
Say 'The number of lines should be within 1 of each other.'
diffs = Abs( out.0 - err.0 )
If  diffs > 1 Then Say '=== Error: Too many differences in line count:' diffs
-- put 'stdout' half into queue
Address System _filter infnsize 'char' with input stem out. output FIFO ''
step2 = queued()
Parse Value 'wc'( '-l' infn ) With count .
If (out.0 + err.0)  \= count Then Say '=== Error: Lines in input file ('infn'):' count 'stdout stem lines:' out.0 'stderr stem lines:' err.0
-- put 'stdout' half into another file
Address System _filter infnsize 'char' with input FIFO '' output stream outfn
-- put 'stderr' half into another file
outerr = outfn'.err'
Address System _filter infnsize 'char' with input stem err. output stream outerr
return /* FGC */
-- sort original input file
Address System 'sort' infn '>' infn'.sorted'
-- cat both output files into sort...
cmd = _cat outfn outerr' | sort >' outfn'.sorted'
Address System cmd
-- ...and compare them
_diff '-q' infn'.sorted' outfn'.sorted'
If rc \= 0 Then
   Do
      Say '=== Error: Differences between' infn'.sorted' 'and' outfn'.sorted'
   End
Call Stream infn, 'C', 'CLOSE'
Return

RoundTrip:
Say 'Running RoundTrip: ************************************'
-- read input file into stem
Address System _filter infnsize 'char' with input stream infn output stem out.
step1 = out.0
-- then the stem into a queue
Address System _filter infnsize 'char' with input stem out. output FIFO ''
step2 = queued()
-- and back out to another file
Address System _filter infnsize 'char' with input FIFO '' output stream outfn
-- now compare them without sorting
If step1 \= step2 Then Say '*** An error occured. stem output:' step1 'FIFO output:' step2'. See filter.log for possible reasons.'
Call Stream infn, 'C', 'CLOSE'
Return

Blob:
Say 'Running Blob: ************************************'
-- create our blob file
outfn = 'out.blob'
count = 0
lines = 1000
columns = 1000
Do i = 1 To lines
   line = ''
   Do j = 1 To columns
      line = line || D2C( Random( 32, 128 ) )
      count = count + 1
   End
   line.i = line
   Queue line
End
line.0 = i - 1
outfnsize = count + 10 + (lines * 2 ) -- we need enough space to handle CRLF at the end of each line
-- read input from queue into file
outfnqueue = outfn'.queue'
Address System _filter outfnsize 'char' with input FIFO '' output stream outfnqueue
outfnstem = outfn'.stem'
-- then the stem into a stream via stderr
Address System _filter outfnsize 'char stderr' with input stem line. error stream outfnstem
return /* FGC */
-- now compare them without sorting
_diff '-q' outfnqueue outfnstem
_rm outfnqueue
_rm outfnstem
Return
