/*
 * This program converts a generic Win32 resource file into a version-specific
 * one.
 * Parameters:
 *   Input:   input rc file
 *   Output:  output rc file with version numbers
 *   Version: in x.x or x.x.x format
 *   Architecture (32bit/64bit): 32 or 64
 *   Interpreter: Regina, ooRexx, etc
 *   Version Date: DD MON YYYY - MUST be last in argument list!
 * Assumptions: input file exists and in correct format
 *              output file is writeable
 */
Parse Arg input output version arch interpreter verdate
If Words( verdate ) \= 3 Then
   Do
      Say 'Invalid value of "'verdate'" for version date.'
      Exit 1
   End
verdate = Strip( verdate, 'B', '"' )
interpreter = Strip( interpreter, 'B', '"' )
verdot = version
num_dots = Countstr( '.', verdot )
Select
   When num_dots = 1 Then
      Do
         qm_ver = '?.?'
         qm_vercomma = '?,?'
      End
   When num_dots = 2 Then
      Do
         qm_ver = '?.?.?'
         qm_vercomma = '?,?,?'
      End
   Otherwise Say 'Invalid verdot format:' verdot'. Must be x.x or x.x.x'
End
vercomma = Changestr( '.', version, ',' )
veryear = Left( Date( 'S' ), 4 )
Call Stream input, 'C', 'OPEN READ'
Do While Lines( input) > 0
   line = Linein( input )
   If num_dots = 2 & Countstr( '?.?.0', line ) = 1 Then line = Changestr( '?.?.0', line, verdot )
   If num_dots = 2 & Countstr( '?,?,0', line ) = 1 Then line = Changestr( '?,?,0', line, vercomma )
   line = Changestr( qm_ver, line, verdot )
   line = Changestr( qm_vercomma, line, vercomma )
   line = Changestr( '????????', line, verdate )
   line = Changestr( '????', line, veryear )
   line = Changestr( '??bits??', line, arch )
   line = Changestr( '??interpreter??', line, interpreter )
   Queue line
End
Call Stream input, 'C', 'CLOSE'
-- input can be same as output, so need to close input first
Call Stream output, 'C', 'OPEN WRITE REPLACE'
Do Queued()
   Parse Pull line
   Call Lineout output, line
End
Call Stream output, 'C', 'CLOSE'
Return 0
