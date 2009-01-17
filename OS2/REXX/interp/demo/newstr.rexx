/*
 * Test program for Regina 08a
 *
 * This program tests the new ANSI REXX String functions:
 * CHANGESTR() and COUNTSTR().
 *
 */
Trace o
Say
Say 'This is the output from the COUNTSTR() function'
Say
Call show_countstr '',''
Call show_countstr 'a','abcdef'
Call show_countstr 0,0
Call show_countstr 'a','def'
Call show_countstr 'a',''
Call show_countstr '','def'
Call show_countstr 'abc','abcdef'
Call show_countstr 'abcdefg','abcdef'
Call show_countstr 'abc','abcdefabccdabcd'

Say
Say 'This is the output from the CHANGESTR() function'
Say

Call show_changestr 'a','fred','c'
Call show_changestr '','','x'
Call show_changestr 'a','abcdef','x'
Call show_changestr 0,0,1
Call show_changestr 'a','def','xyz'
Call show_changestr 'a','','x'
Call show_changestr '','def','xyz'
Call show_changestr 'abc','abcdef','xyz'
Call show_changestr 'abcdefg','abcdef','xyz'
Call show_changestr 'abc','abcdefabccdabcd','z'
Return 0

show_countstr: Procedure
Parse Arg arg1,arg2
Say Left("Countstr('"arg1"','"arg2"')",40) "====>" Countstr(arg1,arg2)
Return

show_changestr: Procedure
Parse Arg arg1,arg2,arg3
Say Left("Changestr('"arg1"','"arg2"','"arg3"')",40) "====>" Changestr(arg1,arg2,arg3)
Return
