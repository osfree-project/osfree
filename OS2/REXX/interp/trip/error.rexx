/*
 * This is a test program for Regina error messages.
 */
Options NOEXT_COMMANDS_AS_FUNCS
Trace o
arg_errors = ''
arg_langs = ''
Do i = 1 To Words( Arg(1) )
   If Datatype( Word( Arg(1), i ) ) = 'NUM' Then arg_errors = arg_errors Word( Arg(1), i )
   Else arg_langs = arg_langs Word( Arg(1), i )
End
If arg_langs = '' Then langs = 'en'
Else langs = arg_langs
If arg_errors = '' Then errors = '3.1 4.0 5.0 6.0 6.1 6.2 6.3 7.0 8.1 8.2 9.0 10.1 12.0',
         '13.1 14.3 14.4 15.1 15.2 15.3 16.1 17.0 18.1 18.2 19.1',
         '19.3 20.1 21.1 24.1 25.1 25.2 25.3 25.4 25.5 25.6 25.7 25.8',
         '25.9 25.11 25.12 25.14 25.15 25.17 26.2 26.3 26.4 26.5 26.6 26.7 27.1',
         '28.1 28.2 28.3 28.4 30.0 31.2 31.3 32.0 33.1 34.0 35.1 38.1 38.3',
         '40.3 40.4 40.5 40.11 40.13 40.14 40.17 40.18 40.19 40.21',
         '40.23 40.27 40.28 40.31 40.32 40.33 40.34 40.37 40.38 40.39 40.42 40.914',
         '40.921 40.922 40.923 40.924 41.0 42.0 43.1 44.1 48.1 49.1 51.1',
         '53.1 53.2 53.3 64.1 90.1 90.3 93.1 93.3'
Else errors = arg_errors
Do i = 1 To Words( errors )
   head = 'Testing for error:' Word( errors, i )
   Say Head
   Say Copies( '-', Length( head ) )
   sub = Translate( Word( errors, i ), '_', '.' )
   Interpret 'Call' sub
End
Say 'End of error testing'
Address System 'rm -f err.rex'
Return 0

3_1: Procedure Expose langs
Call doit 'this_should_not_exist.rexx'
Return

4_0: Procedure Expose langs
Say 'Difficult to test!'
Return

5_0: Procedure Expose langs
Say 'Difficult to test!'
Return

6_0: Procedure Expose langs
Say 'Difficult to test!'
Return

6_1: Procedure Expose langs
src.0 = 1
src.1 = '/*' /**/
Call runner
Return

6_2: Procedure Expose langs
src.0 = 1
src.1 = "'"
Call runner
Return

6_3: Procedure Expose langs
src.0 = 1
src.1 = '"'
Call runner
Return

7_0: Procedure Expose langs
src.0 = 1
src.1 = "select; fred end"
Call runner
Return

8_1: Procedure Expose langs
src.0 = 1
src.1 = "then a=b"
Call runner
Return

8_2: Procedure Expose langs
src.0 = 1
src.1 = "else b=d"
Call runner
Return

9_0: Procedure Expose langs
src.0 = 1
src.1 = "when b=d"
Call runner
Return

10_1: Procedure Expose langs
src.0 = 1
src.1 = "end"
Call runner
Return

12_0: Procedure Expose langs
src.0 = 1
src.1 = 'say "'Copies( 'x', 1024 )'"'
Call runner
Return

13_1: Procedure Expose langs
src.0 = 1
src.1 = d2c( 26 )
Call runner
Return

14_3: Procedure Expose langs
src.0 = 1
src.1 = 'if a=b then'
Call runner
Return

14_4: Procedure Expose langs
src.0 = 1
src.1 = 'if a=b then say a;else'
Call runner
Return

15_1: Procedure Expose langs
src.0 = 1
src.1 = 'say x2c("1 234")'
Call runner
Return

15_2: Procedure Expose langs
src.0 = 1
src.1 = 'say b2x(" 1  111111")'
Call runner
Return

15_3: Procedure Expose langs
src.0 = 1
src.1 = 'say "1 23x4"x'
Call runner
Return

15_4: Procedure Expose langs
src.0 = 1
src.1 = 'say b2x("1  211111")'
Call runner
Return

16_1: Procedure Expose langs
src.0 = 1
src.1 = 'signal fred'
Call runner
Return

17_0: Procedure Expose langs
src.0 = 1
src.1 = 'procedure'
Call runner
Return

18_1: Procedure Expose langs
src.0 = 2
src.1 = 'trace o'
src.2 = 'if a=b say x'
Call runner
Return

18_2: Procedure Expose langs
src.0 = 4
src.1 = 'trace o'
src.2 = 'select'
src.3 = 'when a=b say x'
src.4 = 'end'
Call runner
Return

19_1: Procedure Expose langs
src.0 = 1
src.1 = 'address date()'
Call runner
Return

19_3: Procedure Expose langs
src.0 = 1
src.1 = 'call on error name'
Call runner
Return

20_1: Procedure Expose langs
src.0 = 1
src.1 = 'a: procedure expose '
Call runner
Return

21_1: Procedure Expose langs
src.0 = 1
src.1 = 'call on error say'
Call runner
Return

24_1: Procedure Expose langs
src.0 = 1
src.1 = 'trace x'
Call runner
Return

25_1: Procedure Expose langs
src.0 = 1
src.1 = 'call on syntax name'
Call runner
Return

25_2: Procedure Expose langs
src.0 = 1
src.1 = 'call off say'
Call runner
Return

25_3: Procedure Expose langs
src.0 = 1
src.1 = 'signal on fred name'
Call runner
Return

25_4: Procedure Expose langs
src.0 = 1
src.1 = 'signal off say'
Call runner
Return

25_5: Procedure Expose langs
src.0 = 1
src.1 = 'Address system 'ls' with fred'
Call runner
Return

25_6: Procedure Expose langs
src.0 = 1
src.1 = 'Address system 'ls' with input fred'
Call runner
Return

25_7: Procedure Expose langs
src.0 = 1
src.1 = 'Address system 'ls' with output fred'
Call runner
Return

25_8: Procedure Expose langs
src.0 = 1
src.1 = 'Address system 'ls' with output append fred'
Call runner
Return

25_9: Procedure Expose langs
src.0 = 1
src.1 = 'Address system 'ls' with output replace fred'
Call runner
Return

25_11: Procedure Expose langs
src.0 = 1
src.1 = 'numeric form fred'
Call runner
Return

25_12: Procedure Expose langs
src.0 = 1
src.1 = 'parse fred'
Call runner
Return

25_13: Procedure Expose langs
src.0 = 1
src.1 = 'parse upper fred'
Call runner
Return

25_14: Procedure Expose langs
src.0 = 1
src.1 = 'Address system 'ls' with error fred'
Call runner
Return

25_15: Procedure Expose langs
src.0 = 1
src.1 = 'numeric frm fred'
Call runner
Return

25_17: Procedure Expose langs
src.0 = 1
src.1 = 'procedure  fred'
Call runner
Return

26_2: Procedure Expose langs
src.0 = 3
src.1 = 'do a'
src.2 = 'say i'
src.3 = 'end'
Call runner
Return

26_3: Procedure Expose langs
src.0 = 3
src.1 = 'do a=1 for x'
src.2 = 'say i'
src.3 = 'end'
Call runner
Return

26_4: Procedure Expose langs
src.0 = 1
src.1 = 'parse var a 3.5 d 4.6'
Call runner
Return

26_5: Procedure Expose langs
src.0 = 1
src.1 = 'numeric digits -3'
Call runner
Return

26_6: Procedure Expose langs
src.0 = 1
src.1 = 'numeric fuzz -3'
Call runner
Return

26_7: Procedure Expose langs
src.0 = 2
src.1 = 'trace 3.5'
src.2 = 'say hello'
Call runner
Return

27_1: Procedure Expose langs
src.0 = 3
src.1 = 'do i = 1 to 5 to 6'
src.2 = 'say i'
src.3 = 'end'
Call runner
Return

28_1: Procedure Expose langs
src.0 = 1
src.1 = 'leave j'
Call runner
Return

28_2: Procedure Expose langs
src.0 = 1
src.1 = 'iterate'
Call runner
Return

28_3: Procedure Expose langs
src.0 = 3
src.1 = 'do i = 1'
src.2 = 'leave j'
src.3 = 'end'
Call runner
Return

28_4: Procedure Expose langs
src.0 = 3
src.1 = 'do i = 1'
src.2 = 'iterate j'
src.3 = 'end'
Call runner
Return

30_0: Procedure Expose langs
src.0 = 7
src.1 = 'a = "X1234567890123456789012345678901234567890123456789012345678901234567890"'
src.2 = 'X1234567890123456789012345678901234567890123456789012345678901234567890 = 3'
src.3 = 'call X'
src.4 = 'Return'
src.5 = 'X: Procedure Expose (A) X1234567890123456789012345678901234567890123456789012345678901234567890'
src.6 = 'say hello a'
src.7 = 'Return'
Call runner
Return

31_2: Procedure Expose langs
src.0 = 1
src.1 = '3a="X"'
Call runner
Return

31_3: Procedure Expose langs
src.0 = 1
src.1 = '.a="X"'
Call runner
Return

32_0: Procedure Expose langs
src.0 = 1
src.1 = 'upper fred.'
Call runner
Return

33_1: Procedure Expose langs
src.0 = 2
src.1 = 'numeric fuzz 10'
src.2 = 'numeric digits 11'
Call runner
Return

34_0: Procedure Expose langs
src.0 = 2
src.1 = 'a=2'
src.2 = 'if a then say a'
Call runner
Return

35_1: Procedure Expose langs
src.0 = 1
src.1 = 'if > '
Call runner
Return

38_1: Procedure Expose langs
src.0 = 1
src.1 = 'parse var a * > '
Call runner
Return

38_3: Procedure Expose langs
src.0 = 1
src.1 = 'parse value date() ar a '
Call runner
Return

40_3: Procedure Expose langs
src.0 = 1
src.1 = 'say substr("abc")'
Call runner
Return

40_4: Procedure Expose langs
src.0 = 1
src.1 = 'say substr("abc", 1, 1, 1, 1)'
Call runner
Return

40_5: Procedure Expose langs
src.0 = 1
src.1 = 'say stream( ,"C", "CLOSE")'
Call runner
Return

40_11: Procedure Expose langs
src.0 = 1
src.1 = 'say format("abc")'
Call runner
Return

40_13: Procedure Expose langs
src.0 = 1
src.1 = 'say d2x(-1)'
Call runner
Return

40_14: Procedure Expose langs
src.0 = 1
src.1 = 'say word("abc", -1)'
Call runner
Return

40_17: Procedure Expose langs
src.0 = 1
src.1 = 'say errortext(150)'
Call runner
Return

40_18: Procedure Expose langs
src.0 = 2
src.1 = 'trace o'
src.2 = 'say date("n","9999992","b")'
Call runner
Return

40_19: Procedure Expose langs
src.0 = 1
src.1 = 'say date("n","ddd")'
Call runner
Return

40_21: Procedure Expose langs
src.0 = 1
src.1 = 'say date("", 2344,"B")'
Call runner
Return

40_23: Procedure Expose langs
src.0 = 1
src.1 = 'say left("abc", 2, "abc")'
Call runner
Return

40_27: Procedure Expose langs
src.0 = 1
src.1 = 'say stream(":::", "C", "QUERY EXISTS")'
Call runner
Return

40_28: Procedure Expose langs
src.0 = 1
src.1 = 'say date(x)'
Call runner
Return

40_31: Procedure Expose langs
src.0 = 1
src.1 = 'say random("9999992")'
Call runner
Return

40_32: Procedure Expose langs
src.0 = 1
src.1 = 'say random(1,100000002)'
Call runner
Return

40_33: Procedure Expose langs
src.0 = 1
src.1 = 'say random(100,99)'
Call runner
Return

40_34: Procedure Expose langs
src.0 = 1
src.1 = 'say sourceline(100)'
Call runner
Return

40_37: Procedure Expose langs
src.0 = 1
src.1 = 'say value("FRED",,"JUNK")'
Call runner
Return

40_38: Procedure Expose langs
src.0 = 1
src.1 = 'say FORMAT causes this error)'
Call runner
Return

40_39: Procedure Expose langs
src.0 = 1
src.1 = 'say linein(fn, 6, 6)'
Call runner
Return

40_42: Procedure Expose langs
src.0 = 1
src.1 = 'say stream("stdin","C","SEEK +10")'
Call runner
Return

40_914: Procedure Expose langs
src.0 = 1
src.1 = 'say crypt("a line","??")'
Call runner
Return

40_920: Procedure Expose langs
src.0 = 1
src.1 = 'say "STREAM-this should be a low-level I/O error - how to force one ?")'
Call runner
Return

40_921: Procedure Expose langs
src.0 = 1
src.1 = 'say "STREAM-positioning type error ?")'
Call runner
Return

40_922: Procedure Expose langs
src.0 = 1
src.1 = 'say stream("aa", "C","SEEK")'
Call runner
Return

40_923: Procedure Expose langs
src.0 = 1
src.1 = 'say stream("aa", "C","SEEK +10 READ CHAR APPEND WRITE j j j j j j j j j j j ")'
Call runner
Return

40_924: Procedure Expose langs
src.0 = 1
src.1 = 'say stream("aa", "C","SEEK +10 READ WRITE")'
Call runner
Return

41_0: Procedure Expose langs
src.0 = 3
src.1 = 'do a = r to 10'
src.2 = 'say a'
src.3 = 'end'
Call runner
Return

42_0: Procedure Expose langs
src.0 = 1
src.1 = 'say "cant work out a test for this error"'
Call runner
Return

43_1: Procedure Expose langs
src.0 = 2
src.1 = 'options NOEXT_COMMANDS_AS_FUNCS'
src.2 = 'say fred()'
Call runner
Return

44_1: Procedure Expose langs
src.0 = 4
src.1 = 'say fred()'
src.2 = 'return'
src.3 = 'fred:'
src.4 = 'return'
Call runner
Return

48_1: Procedure Expose langs
src.0 = 1
src.1 = 'say "cant work out a test for this error"'
Call runner
Return

49_1: Procedure Expose langs
src.0 = 1
src.1 = 'say "cant work out a test for this error"'
Call runner
Return

51_1: Procedure Expose langs
src.0 = 2
src.1 = 'options STRICT_ANSI'
src.2 = 'say fred.()'
Call runner
Return

53_1: Procedure Expose langs
src.0 = 1
src.1 = 'address system "ls" with output stream 3'
Call runner
Return

53_2: Procedure Expose langs
src.0 = 1
src.1 = 'address system "ls" with output stem 3'
Call runner
Return

53_3: Procedure Expose langs
src.0 = 1
src.1 = 'address system "ls" with output stem ls'
Call runner
Return

64_1: Procedure Expose langs
src.0 = 1
src.1 = 'call a,'
Call runner
Return

90_1: Procedure Expose langs
src.0 = 2
src.1 = 'options STRICT_ANSI'
src.2 = 'say crypt()'
Call runner
Return

90_2: Procedure Expose langs
src.0 = 2
src.1 = 'options STRICT_ANSI'
src.2 = 'upper a'
Call runner
Return

90_3: Procedure Expose langs
src.0 = 2
src.1 = 'options STRICT_ANSI'
src.2 = 'say time("T")'
Call runner
Return

93_1: Procedure Expose langs
src.0 = 1
src.1 = 'say stream(fn, "C","QUERY JUNK")'
Call runner
Return

93_3: Procedure Expose langs
src.0 = 1
src.1 = 'say stream(fn, "C","FRED")'
Call runner
Return

runner: Procedure Expose langs src.
Call Stream  'err.rex', 'C', 'OPEN WRITE REPLACE'
Do i = 1 To src.0
   Call Lineout 'err.rex', src.i
   Say '=='i'==>' src.i
End
Call Stream 'err.rex', 'C', 'CLOSE'
Call doit 'err.rex'
Return

doit: Procedure Expose langs
Parse Arg fn
Say
Do i = 1 To Words( langs )
   Call Value 'REGINA_LANG', Word( langs, i ), 'ENVIRONMENT'
   Address Command 'rexx' fn
   Say
End
Return
