/*
 * This program reads the en.mts file and inserts the entries into error.c
 */
fn = 'en.mts'
en. = ''
idx = 0
Do While Lines( fn ) > 0
   line = Linein( fn )
   If Strip( line ) = '' | Left( line, 1 ) = '#' Then Iterate
   Parse Var line err ',' suberr ',' text
   If Datatype( err ) \= 'NUM' | DataType( suberr ) \= 'NUM' Then Call Abort fn, lineno, 'Invalid line format'
   idx = idx + 1
   en.idx = '   {' Right(err,3)','Right(suberr,3)',"'Changestr('"', text, '\"' )'" },'
End
en.0 = idx

fn = 'error.c'
Call Stream 'error.c', 'C', 'OPEN READ'
idx = 0
Do While Lines( fn ) > 0
   idx = idx + 1
   error.idx = Linein( fn )
End
error.0 = idx
Call Stream 'error.c', 'C', 'CLOSE'

Call Stream fn, 'C', 'OPEN WRITE REPLACE'
Do i = 1 To error.0
   If error.i = '/* PLACEMARKER */' Then
      Do j = 1 To en.0
         Call Lineout fn,en.j
      End
   Else Call Lineout fn,error.i
End
Call Stream 'error.c', 'C', 'CLOSE'
Return 0

Abort: Procedure
Parse Arg fn, lineno, msg
Say 'Error processing' fn 'at line:' lineno':' msg
Exit 1
