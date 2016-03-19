/* def2exp.rexx
 * Write all lines beginning with EXPORTS to the outfile.
 * This is used for AIX 3.x and AIX 4.1
 */
Parse Arg infile outfile .
Call Stream outfile, 'C', 'OPEN WRITE REPLACE'
Do While( Lines( infile ) > 0 )
   Parse Value Linein( infile ) With . 'EXPORTS' func
   If func \= '' Then Call Lineout outfile, Strip( func )
End
Return 0
