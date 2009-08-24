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
  Call show_dates 1, Word(normal_dates,i)
End
Say
Say 'This is the output from the Date() conversion function'
Say
Call show_dates 3,'S','60','D'
Call show_dates 3,'N','03/06/96','E'
Call show_dates 3,'N','06/03/96','U'
Call show_dates 3,'N','96/06/03','O'
Call show_dates 3,'N','19960603','S'
Call show_dates 3,'E','12 Jun 1996','N'
Call show_dates 3,'E','3 Jun 1996','N'
Call show_dates 3,'B','19960603','S'
Call show_dates 3,'B','19960704','S'
Call show_dates 3,'B','00010101','S'
Call show_dates 3,'B','19000101','S'
Call show_dates 3,'B','19890827','S'
Call show_dates 3,'S','726340','B'
Call show_dates 3,'S','0','B'
Call show_dates 3,'S','000','B'
Call show_dates 3,'S','187','D'
Call show_dates 3,'S','1','D'
Call show_dates 3,'S','61','D'
Call show_dates 3,'S','60','D'
Call show_dates 3,'S','59','D'
Call show_dates 3,'S','58','D'
Call show_dates 3,'S','-3','D'
Call show_dates 3,'S','-2','D'
Call show_dates 3,'S','-1','D'
Call show_dates 3,'S','0','D'
Call show_dates 3,'S','456','D'
Call show_dates 3,'S','728715','B'
Call show_dates 3,'S','728716','B'
Call show_dates 3,'S','728717','B'
Call show_dates 3,'S','728718','B'
Call show_dates 3,'S','728719','B'
Call show_dates 3,'E','728685','B'
Call show_dates 3,'E','728686','B'
Call show_dates 3,'E','728687','B'
Call show_dates 3,'E','728688','B'
Call show_dates 3,'E','728689','B'
Call show_dates 3,'B','19960228','S'
Call show_dates 3,'B','19960229','S'
Call show_dates 3,'B','19960301','S'
Call show_dates 3,'B','19960302','S'
Call show_dates 3,'E','728905','B'
Call show_dates 3,'W','728905','B'
Call show_dates 3,'W','728906','B'
Say
Say 'Year 2000 (and beyond) specific examples'
Say
Call show_dates 3,'N','03/06/99','E'
Call show_dates 3,'N','06/03/99','U'
Call show_dates 3,'N','99/06/03','O'
Call show_dates 3,'N','03/06/00','E'
Call show_dates 3,'N','06/03/00','U'
Call show_dates 3,'N','00/06/03','O'
Call show_dates 3,'N','03/06/01','E'
Call show_dates 3,'N','06/03/01','U'
Call show_dates 3,'N','01/06/03','O'
Call show_dates 3,'S','750000','B'
Call show_dates 3,'B','20540607','S'
Call show_dates 3,'N','25250101','S'
Call show_dates 3,'W','25250101','S'
Return 0

show_dates: Procedure
Parse Arg num,arg1,arg2,arg3
If num = 1 Then
  Do
    Say Left("Date('"arg1"')",30) "===>" Date(arg1)
  End
Else
  Do
    Say Left("Date('"arg1"','"arg2"','"arg3"')",30) "===>" Date(arg1,arg2,arg3)
  End
Return
