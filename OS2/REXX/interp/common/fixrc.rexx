/*
 * This program converts a generic Win32 resource file into a version-specific
 * one.
 * Parameters:
 *   Input:   input rc file
 *   Output:  output rc file with version numbers
 *   Version: in x.x format
 * Assumptions: input file exists and in correct format
 *              output file is writeable
 */
Parse Arg input output version verdate
verdate = Strip( verdate, 'B', '"' )
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
Call Stream output, 'C', 'OPEN WRITE REPLACE'
Do While Lines( input) > 0
   line = Linein( input )
   line = Changestr( qm_ver, line, verdot )
   line = Changestr( qm_vercomma, line, vercomma )
   line = Changestr( '????????', line, verdate )
   line = Changestr( '????', line, veryear )
   Call Lineout output, line
End
Call Stream input, 'C', 'CLOSE'
Call Stream output, 'C', 'CLOSE'
Return 0
