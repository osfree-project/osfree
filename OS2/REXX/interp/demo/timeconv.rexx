/*
 * Test program for Regina 08a
 *
 * This program tests the new ANSI REXX Time conversion functionality.
 *
 */
Trace o
Say
Say 'This is the output from the "normal" Time() function'
Say
normal_times = "N C O S L M H"
Do i = 1 To Words(normal_times)
  Call show_times 1, Word(normal_times,i)
End
Say
Say 'This is the output from the Time() conversion function'
Say
Call show_times 3,'L','12:45:04','N'
Call show_times 3,'S','12:45:04','N'
Call show_times 3,'N','45904','S'
Call show_times 3,'L','12:50am','C'
Call show_times 3,'L','12:50pm','C'
Call show_times 3,'L','00:50am','C'
Call show_times 3,'L','02:50pm','C'
Call show_times 3,'L','10','H'
Call show_times 3,'L','000','H'
Call show_times 3,'L','12','M'
Call show_times 3,'L','121','M'
Call show_times 3,'L','12','S'
Call show_times 3,'L','8192','S'
Call show_times 3,'E','8192','S'
Call show_times 3,'L','12:45:04.345673','L'
Call show_times 3,'N','12:45:04','N'
Call show_times 3,'S','45904','S'
Call show_times 3,'H','10','H'
Call show_times 3,'M','10','M'
Call show_times 3,'C','12:50am','C'
Call show_times 3,'C','12:50pm','C'
Call show_times 3,'C','02:50am','C'
Call show_times 3,'C','02:50pm','C'
Return 0

show_times: Procedure
Parse Arg num,arg1,arg2,arg3
If num = 1 Then
  Do
    Say Left("Time('"arg1"')",30) "===>" Time(arg1)
  End
Else
  Do
    Say Left("Time('"arg1"','"arg2"','"arg3"')",30) "===>" Time(arg1,arg2,arg3)
  End
Return
/**/
