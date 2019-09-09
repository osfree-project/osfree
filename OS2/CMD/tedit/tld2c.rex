/*
 * This Rexx program converts a TLD file into the suitable C code to be
 * included in default.c
 * First argument is the name of the parser; eg SNOBOL
 * Second argument is filename
 */
trace o
Parse Arg lang fn
If fn = '' Then
   Do
      Say 'No filename specified'
      Exit 1
   End
Say '      { (CHARTYPE *)"*'Translate(lang)'.TLD",'
Say '        (CHARTYPE *)"'Translate(lang)'",'
code = '        (CHARTYPE *)"* 'Translate(lang)
Do While Lines(fn) > 0
   line = Strip(Linein(fn))
   If Substr(line,1,1) = '*' | Length(line) = 0 Then Iterate
   If Substr(line,1,1) = ':' Then
     Do
       Say code || '" \'
       code = Copies(' ',12) || '"'line'\n'
     End
   Else
     code = code || line || '\n'
End
Say code || '\n"'
Say '      },'
Return 0
