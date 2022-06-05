Parse Arg infile outfile
Call Stream outfile, 'C', 'OPEN WRITE REPLACE'
first = 1
Do While Lines( infile ) > 0
   line = Linein( infile )
   Select
      When Word( line, 1 ) = 'LIBRARY' Then Call Lineout outfile, 'LIBRARY' Word( line, 2 )
      When Word( line, 1 ) = 'EXPORTS' Then
         Do
            If first Then
               Do
                  Call Lineout outfile, 'EXPORTS'
                  Call Lineout outfile, Word( line, 2 )
                  first = 0
               End
            Else Call Lineout outfile, Word( line, 2 )
         End
      Otherwise Nop
   End
End
Call Lineout outfile, 'SECTION'
Call Lineout outfile, 'DATA SHARED'
Call Stream outfile, 'C', 'CLOSE'
