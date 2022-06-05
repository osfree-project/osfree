#!/usr/bin/env regina
/*
 * Test program for Regina 08a
 *
 * This program tests the new ANSI REXX Date conversion functionality.
 *
 */
Trace o
Say
Say 'This is the output from the "normal" Date() function'
Say
normal_dates = "B D E M N O S U W"
Do i = 1 To Words(normal_dates)
  Call show_dates Word(normal_dates,i)
End
Say
Say 'This is the output from the Date() conversion function'
Say
Call show_dates 'S','60','D'
Call show_dates 'N','03/06/96','E'
Call show_dates 'N','06/03/96','U'
Call show_dates 'N','96/06/03','O'
Call show_dates 'N','19960603','S'
Call show_dates 'E','12 Jun 1996','N'
Call show_dates 'E','3 Jun 1996','N'
Call show_dates 'B','19960603','S'
Call show_dates 'B','19960704','S'
Call show_dates 'B','00010101','S'
Call show_dates 'B','19000101','S'
Call show_dates 'B','19890827','S'
Call show_dates 'S','726340','B'
Call show_dates 'S','0','B'
Call show_dates 'S','000','B'
Call show_dates 'S','187','D'
Call show_dates 'S','1','D'
Call show_dates 'S','61','D'
Call show_dates 'S','60','D'
Call show_dates 'S','59','D'
Call show_dates 'S','58','D'
Call show_dates 'S','728715','B'
Call show_dates 'S','728716','B'
Call show_dates 'S','728717','B'
Call show_dates 'S','728718','B'
Call show_dates 'S','728719','B'
Call show_dates 'E','728685','B'
Call show_dates 'E','728686','B'
Call show_dates 'E','728687','B'
Call show_dates 'E','728688','B'
Call show_dates 'E','728689','B'
Call show_dates 'B','19960228','S'
Call show_dates 'B','19960229','S'
Call show_dates 'B','19960301','S'
Call show_dates 'B','19960302','S'
Call show_dates 'E','728905','B'
Call show_dates 'W','728905','B'
Call show_dates 'W','728906','B'
Parse Version ver
If Left( ver, 11 ) = 'REXX-Regina' Then
   Do
      Say
      Say 'Regina extensions'
      Say
      Call show_dates 'I'
      Call show_dates 'I', '3 Feb 1996'
      Call show_dates 'I', '3 Feb 1996', 'N'
      Call show_dates 'I', '3 Feb 1996', 'N', '/'
      Say
      Say 'Regina extensions to allow values for "D" option outside of current year'
      Say
      Call show_dates 'S','-3','D'
      Call show_dates 'S','-2','D'
      Call show_dates 'S','-1','D'
      Call show_dates 'S','0','D'
      Call show_dates 'S','456','D'
   End
Say
Say 'Year 2000 (and beyond) specific examples'
Say
Call show_dates 'N','03/06/99','E'
Call show_dates 'N','06/03/99','U'
Call show_dates 'N','99/06/03','O'
Call show_dates 'N','03/06/00','E'
Call show_dates 'N','06/03/00','U'
Call show_dates 'N','00/06/03','O'
Call show_dates 'N','03/06/01','E'
Call show_dates 'N','06/03/01','U'
Call show_dates 'N','01/06/03','O'
Call show_dates 'S','750000','B'
Call show_dates 'B','20540607','S'
Call show_dates 'N','25250101','S'
Call show_dates 'W','25250101','S'

Say
Say 'Date separator conversion examples'
Say

--Say Left("Date('S','13 Nov 1996','','-')",40) "===>" Date('S','13 Nov 1996','','-')
--Say Left("Date('S','13 Nov 1996',,'-')",40) "===>" Date('S','13 Nov 1996',,'-')

Call show_dates 'S','13 Feb 1996','N', '-'
Call show_dates 'N','13 Feb 1996','N', ''
Call show_dates 'N','13 Feb 1996','N', '-'
Call show_dates 'O','06/01/50','U', ''
Call show_dates 'E','02/13/96','U', '.'

Say Left("Date('N',,,'_')",40) "===>" Date('N',,,'_')
Say Left("Date('N','3 Feb 2012',,'_')",40) "===>" Date('N','3 Feb 2012',,'_')
Say Left("Date('N',,,'')",40) "===>" Date('N',,,'')
Say Left("Date('S','1996-11-13','S','','-')",40) "===>" Date('S','1996-11-13','S','','-')
Say Left("Date('S','13-Nov-1996','N','','-')",40) "===>" Date('S','13-Nov-1996','N','','-')
Say Left("Date('S','3-Nov-1996','N','','-')",40) "===>" Date('S','3-Nov-1996','N','','-')
Say Left("Date('O','06*01*50','U','','*')",40) "===>" Date('O','06*01*50','U','','*')
Say Left("Date('U','13.Feb.1996','N',,'.')",40) "===>" Date('U','13.Feb.1996','N',,'.')
Say Left("Date('U','13.Feb.1996','N','','.')",40) "===>" Date('U','13.Feb.1996','N','','.')
Say Left("Date('U','3.Feb.1996','N','+','.')",40) "===>" Date('U','3.Feb.1996','N','+','.')

Return 0

show_dates: Procedure
Select
   When Arg() = 1 Then
      Say Left("Date('"arg(1)"')",40) "===>" Date(arg(1))
   When Arg() = 2 Then
      Say Left("Date('"arg(1)"','"arg(2)"')",40) "===>" Date(arg(1),arg(2))
   When Arg() = 3 Then
      Say Left("Date('"arg(1)"','"arg(2)"','"arg(3)"')",40) "===>" Date(arg(1),arg(2),arg(3))
   When Arg() = 4 Then
      Say Left("Date('"arg(1)"','"arg(2)"','"arg(3)"','"arg(4)"')",40) "===>" Date(arg(1),arg(2),arg(3),arg(4))
   When Arg() = 5 Then
      Say Left("Date('"arg(1)"','"arg(2)"','"arg(3)"','"arg(4)"','"arg(5)"')",40) "===>" Date(arg(1),arg(2),arg(3),arg(4),arg(5))
   Otherwise Nop
End
Return
