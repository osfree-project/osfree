/*
 * This script converts the output from manext to HTML
 *
 * Arguments:
 *           section: one of OVERVIEW,COMMANDS,APPENDIX,GLOSSARY
 *           infile:  file name of .man file
 *           tocfile: file name of Table of Contents output file
 */
Trace o
Parse Arg html_ext ver section infile tocfile .
Select
  When section = 'TOCSTART'  Then Call toc 'START' ver
  When section = 'TOCEND'    Then Call toc 'END' ver
  When section = 'OVERVIEW'  Then Call overview  infile tocfile 'Overview'
  When section = 'COMM'      Then Call commands  infile tocfile 'Commands'
  When section = 'COMMSET'   Then Call commands  infile tocfile 'SET Commands'
  When section = 'COMMSOS'   Then Call commands  infile tocfile 'SOS Commands'
  When section = 'QUERY'     Then Call query     infile tocfile 'Query and Extract'
  When section = 'GLOSSARY'  Then Call glossary  infile tocfile 'Glossary'
  When section = 'APPENDIX1' Then Call appendix1 infile tocfile 'Appendix 1'
  When section = 'APPENDIX2' Then Call appendix2 infile tocfile 'Appendix 2'
  When section = 'APPENDIX3' Then Call appendix2 infile tocfile 'Appendix 3'
  When section = 'APPENDIX4' Then Call appendix2 infile tocfile 'Appendix 4'
  When section = 'APPENDIX5' Then Call appendixx infile tocfile 'Appendix 5'
  When section = 'APPENDIX6' Then Call appendixx infile tocfile 'Appendix 6'
  When section = 'APPENDIX7' Then Call appendix2 infile tocfile 'Appendix 7'
  When section = 'HISTORY'   Then Call history   infile tocfile 'History of THE'
  When section = 'QUICKREF'  Then Call quickref  infile tocfile 'Quick Reference for THE'
  Otherwise
      Do
        Say 'Error: Invalid section:' section 'specified'
      End
End
Return

/********************************************************************/
overview: Procedure Expose html_ext
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
in_code_section = 'N'
already_read = 'N'
in_list_section = 'N'
first_heading = 'N'
Call heading title
Do While(Lines(infile)>0)
   If already_read = 'N' Then line = Linein(infile)
   already_read = 'N'
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DT>'MakeTOC(line,"")'</DT>'
            Say '<HR>'
            Say '<A NAME="' || RemoveSpaces(line) || '"></A>'
            Say '<H2>' line '</H2>'
            line = Linein(infile)
            Say '<HR>'
            first_heading = 'Y'
          End
     When Strip(line) = Copies('-',74) Then Iterate
     When Substr(Strip(line),1,2) = '- 'Then
          Do
            tmp = ''
            If in_list_section = 'Y' Then
              Do
                tmp = Substr(Strip(line),3)
                Do Forever
                   line = Linein(infile)
                   Select
                     When Strip(line) = '' Then 
                          Do
                            Say '<LI>' handle_keywords(tmp,'') '</LI>'
                            Say '</UL>'
                            in_list_section = 'N'
                          End
                     When Substr(Strip(line),1,2) = '- ' Then
                          Do
                             Say '<LI>' handle_keywords(tmp,'') '</LI>'
                             already_read = 'Y'
                          End
                     When Substr(Strip(line),1,2) = 'o ' Then
                          Do
                             in_sublist_section = 'Y'
                             tmp = tmp '<(DD)>'Subword(line,2)
                          End
                     Otherwise
                          Do
                             If in_sublist_section = 'Y' Then
                                Do
                                   tmp = tmp '<(BR)>'line
                                   in_sublist_section = 'N'
                                End
                             Else tmp = tmp line
                          End
                   End
                   If in_list_section = 'N' | already_read = 'Y' Then Leave
                End
              End
            Else
              Do
                Say '<UL>'
                in_list_section = 'Y'
                already_read = 'Y'
              End
          End
     When Strip(line) = '+'||Copies('-',30) Then
          Do
            If in_code_section = 'Y' Then
               Do
                 Say '<IMG SRC="divbot.png" ALT="----------"><BR>'
                 Say '</BLOCKQUOTE><P>'
                 in_code_section = 'N'
               End
            Else
               Do
                 Say '<BLOCKQUOTE>'
                 Say '<IMG SRC="divtop.png" ALT="----------"><BR>'
                 in_code_section = 'Y'
               End
          End
      Otherwise
          Do
            If first_heading = 'N' Then Iterate
            If in_code_section = 'Y' Then
               Do
                 Say Strip(line) '<BR>'
               End
            Else
               Do
                 tmp = ''
                 If Strip(line) = '' Then Iterate
                 Do Forever
                    tmp = tmp line
                    line = Linein(infile)
                    If Strip(line) = '' Then
                      Do
                        tmp = handle_keywords(tmp,"")
                        Say tmp '<P>'
                        Leave
                      End
                 End
               End
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Say '<P Align="Center"><HR>'
Say 'The HTML version of this manual was inspired by <A HREF = "mailto:judygs@uic.edu">Judith Grobe Sachs </A>'
Say '<P Align="Center"><HR>'
Call footing
Return

/********************************************************************/
commands: Procedure Expose html_ext
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
already_read_line = 'N'
once = 'Y'
Call heading title
Do While(Lines(infile)>0)
   If already_read_line = 'N' Then line = Linein(infile)
   already_read_line = 'N'
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DT>'MakeTOC(line,"")'</DT>'
            Say '<P><HR><A NAME="' || RemoveSpaces(line) || '"></A>'
            Say '<H2>' line '</H2>'
            line = Linein(infile)
          End
     When Strip(line) = Copies('-',74) Then
          Say '<HR>'
     When line = '     COMMAND' Then
          Do
            line = Linein(infile)
            Parse var line keyword '-' desc
            keyword = Strip(keyword)
            keyword_words = Words(keyword)
            Say '<A NAME="'||RemoveSpaces(SpecialTranslate(keyword))||'"></A>'
            Say '<DL><DT>'
            Say Bold(Translate(keyword)) '-' desc '<BR><BR></DT>'
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DD>'MakeTOC(keyword,desc)'</DD>'
          End
     When line = '     SEE ALSO' Then
          Do
            Say '<DT>' Bold('See Also:') '<BR></DT>'
            line = Linein(infile)
            tmp = ''
            Do Forever
               Parse Var line '<' keyword '>' line
               If keyword = '' Then Leave
               tmp = tmp MakeRef('UPPER',',',keyword)
               If line = '' Then Leave
            End
            tmp = Strip(tmp,'T',',')
            Say '<DD>' tmp '<P></DD>'
          End
     When line = '     STATUS' Then 
          Do
            Say '<DT>' Bold('Status:') '<BR></DT>'
            Do Forever
               line = Linein(infile)
               If line = '' Then Leave
               Say '<DD>' Strip(line) '</DD>'
            End
            Say '</DL><P>'
          End
     When line = '     SYNTAX' Then
          Do
            Say '<DT>' Bold('Syntax:') '<BR></DT>'
            syntax_words = ''
            Do Forever
               line = Linein(infile)
               If line = '' Then Leave
               pre = Subword(line,1,keyword_words)
               post = Subword(line,keyword_words+1)
               Say '<DD>' Strip(pre) Italic(strip(post)) '<P></DD>'
               syntax_words = syntax_words Translate(post,"     ","/|[].")
            End
          End
     When line = '     COMPATIBILITY' Then
          Do
            Say '<DT>' Bold('Compatibility:') '<BR></DT>'
            first = 'Y'
            Do Forever
               line = Linein(infile)
               If line = '' Then Leave
               If first = 'Y' Then
                 Do
                   Say '<DD>' handle_keywords(Strip(line),'')
                   first = 'N'
                 End
               Else
                   Say '</DD><DD>' handle_keywords(Strip(line),'')
            End
            Say '<P></DD>'
          End
     When line = '     DEFAULT' Then
          Do
            Say '<DT>' Bold('Default:') '<BR></DT>'
            Do Forever
               line = Linein(infile)
               If line = '' Then Leave
               Parse Var line pre '<' keyword '>' post
               If keyword \= '' Then line = pre MakeRef('UPPER',,keyword) post
               Say '<DD>' Strip(line) '<P></DD>'
            End
          End
     When line = '     DESCRIPTION' Then
          Do
            Say '<DT>' Bold('Description:') '<BR></DT>'
            tmp = ''
            inblock = 0
            Do Forever
               line = Linein(infile)
               If Substr(Strip(line),1,31) = '+'||Copies('-',30) Then
                 Do
                   Say '<BLOCKQUOTE>'
                   Say '<IMG SRC="divtop.png" ALT="----------"><BR>'
                   Do Forever
                     line = Linein(infile)
                     If Substr(Strip(line),1,31) = '+'||Copies('-',30) Then Leave
                     Say line '<BR>'
                   End
                   Say '<IMG SRC="divbot.png" ALT="----------"><BR>'
                   Say '</BLOCKQUOTE><P>'
                   Iterate
                 End
               If Substr(Strip(line),1,31) = '*'||Copies('-',30) Then
                 Do
                   Say '<PRE>'
                   Do Forever
                     line = Linein(infile)
                     If Substr(Strip(line),1,31) = '*'||Copies('-',30) Then Leave
                     Say line
                   End
                   Say '</PRE><P>'
                   Iterate
                 End
               If Strip(line) = '' Then
                 Do
                   tmp = handle_keywords(tmp,syntax_words)
                   Say '<DD>' Strip(tmp) '<P></DD>'
                   If Substr(tmp,Length(tmp),1) = ':' Then 
                      Do
                        first = 'Y'
                        Do Forever
                           line = Strip(Linein(infile))
                           If line = '' Then Leave
                           If first = 'Y' Then
                             Do
                              Say '<DD>' handle_keywords(line,syntax_keywords)
                              first = 'N'
                             End
                           Else
                              Say '</DD><DD>' handle_keywords(line,syntax_keywords)
                        End
                        Say '<P></DD>'
                      End
                   tmp = ''
                   Iterate
                 End
               If line = '     COMPATIBILITY' Then 
                 Do
                   already_read_line = 'Y'
                   Leave
                 End
               If Pos('<>',line) \= 0 Then
                 Do
                   Parse Var line pre '<>' post
                   line = pre||'&lt;&gt;'||post
                 End
               tmp = tmp line
            End
          End
     Otherwise Nop
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing
Return

/********************************************************************/
glossary: Procedure Expose html_ext
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
already_read_line = 'N'
first_time = 'Y'
Call heading title
Do While(Lines(infile)>0)
   If already_read_line = 'N' Then line = Linein(infile)
   already_read_line = 'N'
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DT>'MakeTOC(line,"")'</DT>'
            Say '<HR><A NAME="' || RemoveSpaces(line) || '"></A>'
            Say '<H2>' line '</H2>'
            Say '<HR><P>'
            Say '<DL>'
            line = Linein(infile)
          End
     When Strip(line) = '' Then Nop
     When Strip(line) = Copies('-',74) Then Say '</DL><HR>'
     When Substr(line,1,15) = Copies(' ',15) Then
          Do
            tmp = Strip(line)
            Do Forever
               line = Strip(Linein(infile))
               If line = '' Then Leave
               tmp = tmp line
            End
            tmp = handle_keywords(tmp,'')
            tmp = ConvertSpecialChars(tmp)
            Say '<DD>' Strip(tmp) '<P></DD>'
          End
     Otherwise
          Do
            line = Strip(line)
            Say '<DT><A NAME="' || RemoveSpaces(Translate(line)) || '">' Bold(line) '</A></DT>'
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DD>'MakeTOC(line,"")'</DD>'
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing
Return

/********************************************************************/
query: Procedure Expose html_ext
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
already_read_line = 'N'
once = 'Y'
in_table = 'N'
first_time = 'Y'
Call heading title
Do While(Lines(infile)>0)
   If already_read_line = 'N' Then line = Linein(infile)
   already_read_line = 'N'
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DT>'MakeTOC(line,"")'</DT>'
            If first_time = 'N' Then Say '</DL>'
            first_time = 'N'
            Say '<A NAME="' || RemoveSpaces(Translate(line)) || '"></A>'
            Say '<HR><H2>' line '</H2>'
            Say '<HR><DL>'
            line = Linein(infile)
          End
     When Strip(line) = '' & in_table = 'Y' Then
          Do
            in_table = 'N'
            Say '</TABLE><P></DD>'
          End
     When Strip(line) = '' Then Nop
     When Strip(line) = Copies('-',74) Then
          Do
            Say '</DL>'
            Say '<HR>'
          End
     When Substr(line,26,3) = ' - ' Then
          Do
            If in_table = 'N' Then
              Do
                Say '<DD><TABLE BORDER=0 CELLSPACING=1 CELLPADDING=1>'
                in_table = 'Y'
              End
            Parse Var line col1 '-' col2
            Say '<TR><TD>'||Strip(col1)||'</TD><TD>-</TD><TD>'||handle_keywords(Strip(col2),'')||'</TD></TR>'
          End
     When Substr(line,1,15) = Copies(' ',15) Then
          Do
            Say '<DD>' Strip(line) '</DD>'
          End
     When Substr(line,1,14) = Copies(' ',14) Then
          Do
            Say '<DD>'
            Say '<IMG SRC="divtop.png" ALT="----------"><BR>'
            Say Strip(line) '<BR>'
            Do Forever
              line = Linein(infile)
              If Strip(line) = '' Then Leave
              Say Strip(line) '<BR>'
            End
            Say '<IMG SRC="divbot.png" ALT="----------"><BR>'
            Say '<P></DD>'
          End
     When Substr(line,1,10) = Copies(' ',10) Then
          Do
            tmp = line
            Do Forever
               line = Strip(Linein(infile))
               Select
                 When Substr(line,1,1) = '(' Then
                      Do
                        tmp = handle_keywords(tmp,'')
                        Say '<DD>' Strip(tmp) '</DD>'
                        Say '<DD>' line '<P></DD>'
                        Leave
                      End
                 When line = '' Then 
                      Do
                        tmp = handle_keywords(tmp,'')
                        Say '<DD>' Strip(tmp) '<P></DD>'
                        Leave
                      End
                 Otherwise
                      Do
                        tmp = tmp line
                      End
               End
            End
          End
     Otherwise
          Do
            pre = Subword(line,1,1)
            post = Subword(line,2)
            Say '<A NAME="' || RemoveSpaces(Translate(Strip(pre))) || '"></A>'
            Say '<DT>'
            If post = '' Then Say Bold(Strip(pre)) '<BR>'
            Else Say Bold(Strip(pre)) Italic(strip(post)) '<BR>'
            Say '</DT>'
            syntax_words = Translate(post,"     ","/|[].")
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing
Return

/********************************************************************/
history: Procedure Expose html_ext
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
in_env = 'N'
Call heading title
inlist = 0
ul = ''
toc_idx = toc_idx + 1
toc.toc_idx = '<DT>'MakeTOC(title,"")'</DT>'
Say '<A NAME="' || RemoveSpaces(Translate(title)) || '"></A>'
Say '<H2>' title '</H2>'
Do While(Lines(infile)>0)
   line = Linein(infile)
   Select
     When Strip(line) = '' Then Nop
     When Substr(line,1,7) = 'Version' Then
          Do
            If inlist Then
              Do
                If ul \= '' Then Say ul
                inlist = 0
                Say '</UL>'
              End
            Parse Var line v ver dt tail
            head = v ver
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DD>'MakeTOC(Strip(head),dt tail)'</DD>'
            Say '<A NAME="' || RemoveSpaces(Translate(Strip(head))) || '"></A>'
            Say '<HR>'
            Say '<H3>' line '</H3>'
            Say '<HR>'
            ul = ''
          End
     When Substr(line,1,4) = " ---" Then
          Do
            If inlist Then
              Do
                If ul \= '' Then Say ul
                inlist = 0
                Say '</UL>'
              End
            Say '<H4>'||Substr(line,5)||'</H4><UL>'
            ul = ''
          End
     When Substr(line,1,6) = "     o" Then
          Do
            If ul \= '' Then Say ul
            ul = '<LI>' Strip(Substr(line,7))
            inlist = 1
          End
     When line = "         +---------------" Then
          Do
            If ul \= '' Then Say ul
            Say '<PRE>'
            inblock = 0
            Do Forever
               line = Linein(infile)
               If line = "         +---------------" Then Leave
               Say line
            End
            Say '</PRE><P>'
            ul = ''
          End
     Otherwise
          Do
            ul = ul Strip(line)
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
If ul \= '' Then Say '<LI>' ul
Say '</UL><HR>'
Call footing
Return

/********************************************************************/
appendix1: Procedure Expose html_ext
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
in_env = 'N'
Call heading title
Do While(Lines(infile)>0)
   line = Linein(infile)
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            Parse Var line head '-' tail
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DT>'MakeTOC(Strip(head),tail)'</DT>'
            Say '<A NAME="' || RemoveSpaces(Strip(head)) || '"></A>'
            Say '<H2>' line '</H2>'
            Say '<HR>'
            line = Linein(infile)
          End
     When Strip(line) = Copies('-',74) Then Say '<HR>'
     When Strip(line) = Copies('*',9) Then Say '<B><H3><CENTER>'line'</CENTER></H3></B>'
     When Substr(Strip(line),1,15) = Copies('-',15) Then
          Do
            line = Strip(Linein(infile))
            Say '<HR>'
            Say '<CENTER><B>'||line||'</B></CENTER>'
            line = Linein(infile)
            Say '<HR>'
          End
     When Substr(line,1,10) = '      THE_' Then
          Do
            Parse Var line env '-' rem
            If in_env = 'N' Then Say '<DL>'
            in_env = 'Y'
            rem = handle_keywords(rem,'')
            Say '<DT><B>'env'</B></DT>'
            Say '<DD>' rem '<P></DD>'
          End
     When Strip(line) = '' & in_env = 'Y' Then 
          Do
            Say '</DL>'
            in_env = 'N'
          End
     When Strip(line) = '' Then Nop
     Otherwise
          Do
            tmp = Strip(line)
            Do Forever
               line = Strip(Linein(infile))
               If line = '' Then Leave
               tmp = tmp line
            End
          tmp = handle_keywords(tmp,'')
          Say tmp '<P>'
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing
Return

/********************************************************************/
appendix2: Procedure Expose html_ext section
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
before_table = 1
Call heading title
Do While(Lines(infile)>0)
   line = Linein(infile)
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            Parse Var line head '-' tail
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DT>'MakeTOC(Strip(head),tail)'</DT>'
            Say '<A NAME="' || RemoveSpaces(Strip(head)) || '"></A>'
            Say '<H2>' line '</H2>'
            Say '<HR>'
            line = Linein(infile)
          End
     When Strip(line) = Copies('-',74) Then Say '<HR>'
     When Substr(Strip(line),1,1) = '-' Then
          Do
            line = Linein(infile)
            Say '<B>' Strip(line) '</B><P>'
            line = Linein(infile)
          End
     When Substr(Strip(line),1,2) = '==' Then
          Do
            line = Linein(infile)
            Say '<H2>' Strip(line) '</H2><P>'
            line = Linein(infile)
          End
     When Substr(Strip(line),1,5) = '+----' & Length(Strip(line)) = 31 Then
          Do
            Say '<BLOCKQUOTE>'
            Say '<IMG SRC="divtop.png" ALT="----------"><BR>'
            Do Forever
              line = Strip(Linein(infile))
              If Substr(line,1,5) = '+----' Then Leave
              If Pos('<',line) \= 0 Then
                Do
                  Parse Var line pre '<Key>' rem
                  Say Strip(pre) '&lt;Key&gt;'||rem '<BR>'
                End
              Else
                Say line '<BR>'
            End
            Say '<IMG SRC="divbot.png" ALT="----------"><BR>'
            Say '</BLOCKQUOTE><P>'
          End
     When Substr(Strip(line),1,5) = '+----' Then
          Do
            before_table = 0
            columns = Words(Translate(line,' ','-')) - 1
            If columns = 1 Then 
              Do
                align1 = '<CENTER>'
                align2 = '</CENTER>'
              End
            Else 
              Do
                align1 = ''
                align2 = ''
              End
            Say '<CENTER><TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2>'
            line = Strip(Linein(infile))
            line = Strip(Strip(line,,'|'))
            tmp = '<TR>'
            Do i = 1 To columns
               Parse Var line col '|' line
               If col = '' Then col = '<BR>'
               tmp = tmp'<TH>'Strip(col)'</TH>'
            End
            Say tmp'</TR>'
            line = Linein(infile) /* remove heading underline */
            col. = ''
            Do Forever
               line = Strip(Linein(infile))
               If line = '' Then Leave
               If Translate(line,'  ','+-') = '' Then 
                  Do
                     /* write out the row */
                     tmp = '<TR>'
                     Do i = 1 To columns
                        tmp = tmp'<TD>'||align1||col.i||align2||'</TD>'
                     End
                     Say tmp'</TR>'
                     col. = ''
                     Iterate
                  End
               line = Strip(Strip(line,,'|'))
               If columns = 1 & line = '|' Then col.1 = col.1||'|'||'<BR>'
               Else
                 Do i = 1 To columns
                   Parse Var line col '|' line
                   col.i = col.i||Strip(col)||'<BR>'
                 End
            End
            /* write out the row */
            tmp = '<TR>'
            Do i = 1 To columns
               tmp = tmp'<TD>'||align1||col.i||align2||'</TD>'
            End
            Say tmp'</TR></TABLE></CENTER><P>'
          End
     When Strip(line) = '' Then Nop
     When Substr(line,1,7) = Copies(' ',7) Then 
          Do
            Say '<DD>'Strip(line)'</DD>'
          End
     Otherwise
          Do
            tmp = Strip(line)
            Do Forever
               line = linein(infile)
               If Strip(line) = '' Then Leave
               Select
                  When section = 'APPENDIX4' Then
                     Do
                        Select
                           When Substr(Strip(line),1,7) = 'Syntax:' | Substr(Strip(line),1,19) = 'Meaning of options:' | Substr(Strip(line),1,17) = "ECOLOR Character:" Then
                              Do
                                 If tmp \= '' Then
                                    Do
                                       tmp = handle_keywords(tmp,'')
                                       Say tmp '<BR>'
                                    End
                                 Say '<I><B>'Strip(line)'</B></I><BR>'
                                 tmp = ''
                              End
                           When Substr(line,1,8) = Copies(' ',8) & Substr(line,9,1) \= ' '  Then
                              Do
                                 If tmp \= '' Then
                                    Do
                                       tmp = handle_keywords(tmp,'')
                                       Say tmp '<BR>'
                                    End
                                 Say '<I>'Strip(line)'</I><BR>'
                                 tmp = ''
                              End
                           Otherwise tmp = tmp Strip(line)
                        End
                     End
                  When section = 'APPENDIX7' & Pos(Right( Strip( line ), 1 ), ':;.' ) \= 0 Then
                     Do
                        If before_table Then Say handle_keywords( tmp Strip( line ) )'<BR>'
                        Else Say tmp Strip( line )'<BR>'
                        tmp = ''
                     End
                  Otherwise tmp = tmp Strip(line)
               End
            End
          tmp = handle_keywords(tmp,'')
          Say tmp '<P>'
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing
Return

/********************************************************************/
appendixx: Procedure Expose html_ext section
Parse arg infile tocfile title
If section = 'APPENDIX5' Then head = 'DEFAULT STATUS SETTINGS IN THE'
Else head = 'THE BY TASKS'
Do While Lines(infile) > 0
   line = Linein(infile)
   pos = Pos('.htm',line)
   If pos \= 0 & html_ext \= '.htm' Then
      Do
         Parse Var line start '.htm' end
         line = start || html_ext || end
      End
   Say line
End
If section = 'APPENDIX5' Then infile = 'app5.man'
Else infile = 'app6.man'
Call Lineout tocfile, '<DT>'MakeTOC(Translate(title),head)'</DT>'
Return

/********************************************************************/
quickref: Procedure Expose html_ext
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
table_head. = ''
colval. = ''
in_table = 'N'
in_start = 'Y'
Call heading title
toc_idx = toc_idx + 1
toc.toc_idx = '<DT>'MakeTOC(title,"")'</DT>'
Say '<A NAME="' || RemoveSPaces(Translate(title)) || '"></A>'
Do While(Lines(infile)>0)
   line = Linein(infile)
   If in_start = 'Y' Then
     Do
       line = Strip(line)
       If Length(line) = 0 Then
         Do
           in_start = 'N'
           Say '<CENTER><HR></CENTER>'
           Iterate
         End
       Say '<CENTER><H1>' line '</H1></CENTER>'
       Iterate
     End
   Select
     When Strip(line) = Copies('-',74) Then Say '<HR>'
     When Substr(Strip(line),1,1) = '-' Then
          Do
            line = Linein(infile)
            header = Strip(line)
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DD>'MakeTOC(header,'')'</DD>'
            Say '<A NAME="' || RemoveSpaces(Translate(header)) || '"></A>'
            Say '<H2>' header '</H2>'
            line = Linein(infile)
          End
     When Substr(Strip(line),1,5) = '+----' Then
          Do
            columns = Words(Translate(line,' ','-')) - 1
            If columns = 1 Then 
              Do
                align1 = '<CENTER>'
                align2 = '</CENTER>'
              End
            Else 
              Do
                align1 = ''
                align2 = ''
              End
            Say '<TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2>'
            line = Strip(Linein(infile))
            tmp = '<TR>'
            Do i = 0 To columns
               Parse Var line col '|' line
               If col = '' Then col = '<BR>'
               table_head.i = Strip(col)'<BR>'
            End
            Do Forever
               line = Strip(Linein(infile))
               If Substr(line,1,5) = '+----' Then Leave
               Do i = 0 To columns
                  Parse Var line col '|' line
                  If col = '' Then col = '<BR>'
                  table_head.i = table_head.i||Strip(col)||'<BR>'
               End
            End
            tmp = ''
            Do i = 1 To columns
               tmp = tmp'<TH>'table_head.i'</TH>'
            End
            Say tmp'</TR>'
            col. = ''
            Do Forever
               line = Strip(Linein(infile))
               If Substr(line,1,5) = '+----' Then Leave
               If line = '' | Translate(line,'  ','+-') = '' Then Leave
               line = Strip(Strip(line,,'|'))
               If columns = 1 & line = '|' Then col.1 = col.1||'|'||'<BR>'
               Else
                 Do i = 1 To columns
                   Parse Var line col '|' line
                   col.i = col.i||Strip(col)||'<BR>'
                 End
            End
            tmp = '<TR>'
            Do i = 1 To columns
               tmp = tmp'<TD>'||align1||col.i||align2||'</TD>'
            End
            Say tmp'</TR></TABLE><P>'
          End
     When Strip(line) = '' Then Nop
     When header = 'Command-line invocation' Then
          Do
            Select
              When Word(line,1) = 'the' Then
                   Do
                     Say '<B>' Strip(line) '</B><P>'
                   End
              When Word(line,1) = 'Where:' Then
                   Do
                     Say '<B>' Strip(line) '</B><P>'
                     Say '<TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2><TR>'
                     ar = '<TD>'
                     dr = '<TD>'
                     Do Forever
                        line = Linein(infile)
                        If Strip(line) = '' Then Leave
                        Parse Var line 1 arg 19 desc
                        arg = Strip(arg)
                        desc = Strip(desc)
                        ar = ar arg'<BR>'
                        dr = dr desc'<BR>'
                     End
                     Say ar'</TD>'
                     Say dr'</TD>'
                     Say '</TR></TABLE><P>'
                   End
              When Word(line,1) = 'Option' Then
                   Do
                     Say '<B>' Strip(line) '</B><BR>'
                   End
              Otherwise Nop
            End
          End
     When header = 'Prefix commands' Then
          Do 
            Say '<TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2><TR>'
            ar = '<TD>'
            dr = '<TD>'
            Do Forever
              Parse Var line arg '-' desc
              arg = Strip(arg)
              desc = Strip(desc)
              ar = ar arg'<BR>'
              dr = dr desc'<BR>'
              line = Linein(infile)
              If Strip(line) = '' Then Leave
            End
            Say ar'</TD>'
            Say dr'</TD>'
            Say '</TR></TABLE><P>'
          End
     When header = 'Line Targets' Then
          Do 
            Say '<TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2><TR>'
            ar = '<TD>'
            dr = '<TD>'
            Do Forever
              Parse Var line arg 27 desc
              arg = Strip(arg)
              desc = Strip(desc)
              ar = ar arg'<BR>'
              dr = dr desc'<BR>'
              line = Linein(infile)
              If Strip(line) = '' Then Leave
            End
            Say ar'</TD>'
            Say dr'</TD>'
            Say '</TR></TABLE><P>'
            Do Forever
               line = Strip(Linein(infile))
               If line = '' Then Leave
               Say line '<BR>'
            End
          End
     When Substr(line,2,5) = '[SET]' | Substr(line,2,3) = 'SET' Then
          Do
            If Translate(Word(line,2)) = 'SET_COMMAND' Then
               Say '<A HREF = "comm' || html_ext || '#SET"> SET' Word(line,2) '</A>' Subword(line,3) '<BR>'
            Else
               Say '<A HREF = "commset' || html_ext || '#SET' || Translate(Word(line,2))'"> [SET]' Word(line,2) '</A>' Subword(line,3) '<BR>'
          End
     When Substr(line,2,3) = 'SOS' Then
          Do
            If Translate(Word(line,2)) = 'SOS_COMMAND' Then
               Say '<A HREF = "comm' || html_ext || '#SOS"> SOS' Word(line,2) '</A>' Subword(line,3) '<BR>'
            Else
               Say '<A HREF = "commsos' || html_ext || '#SOS' || Translate(Word(line,2))'"> SOS' Word(line,2) '</A>' Subword(line,3) '<BR>'
          End
     When Substr(line,2,1) \= ' ' Then
          Do
            Select
              When Word(line,1) = '[Locate]' Then word1 = 'LOCATE'
              When Substr(Word(line,1),1,1) = '?' Then word1 = SpecialTranslate('?')
              When Substr(Word(line,1),1,1) = '!' Then word1 = SpecialTranslate('!')
              When Substr(Word(line,1),1,1) = '=' Then word1 = SpecialTranslate('=')
              When Substr(Word(line,1),1,1) = '&' Then word1 = SpecialTranslate('&')
              Otherwise word1 = Translate(Word(line,1))
            End
            Say '<A HREF = "comm' || html_ext || '#'word1'">' Word(line,1) '</A>' Subword(line,2) '<BR>'
          End
     When Substr(line,1,7) = Copies(' ',7) Then 
          Do
            Say Strip(line)'<BR>'
          End
     Otherwise
          Do
            tmp = Strip(line)
            Do Forever
               line = Strip(linein(infile))
               If line = '' Then Leave
               tmp = tmp line
            End
          tmp = handle_keywords(tmp,'')
          Say tmp '<P>'
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing
Return

/********************************************************************/
toc: Procedure Expose html_ext
Parse arg action ver
If action = 'START' Then 
  Do
    Call heading 'Table of Contents'
    Say '<HR><CENTER><H1>The Hessling Editor</H1> </CENTER>'
    Say '<HR><CENTER><H1>Version' ver '</H1> </CENTER><P>'
    Say '<A NAME="TOC"></A>'
    Say '<HR><CENTER><H2>TABLE OF CONTENTS</H2> </CENTER><HR><DL>'
  End
Else
  Do
    Say '</DL><HR>'
    Call footing 'TOC'
  End
Return

/********************************************************************/
Bold: Procedure Expose html_ext
Parse Arg line
Return '<B>'line'</B>'

/********************************************************************/
Italic: Procedure Expose html_ext
Parse Arg line
Return '<I>'line'</I>'

/********************************************************************/
MakeTOC: Procedure Expose infile html_ext
Parse Arg keyword,desc
Parse Var infile base '.' .
name = RemoveSpaces(SpecialTranslate(keyword))
Return '<A HREF = "'base || html_ext || '#'name'">' keyword '</A>' desc '<BR>'

/********************************************************************/
MakeRef: Procedure Expose html_ext
Parse Arg trans,extra,keyword
Select
  When Words(keyword) > 1 & Translate(Word(keyword,1)) = 'SET' Then filename = 'commset' || html_ext
  When Words(keyword) > 1 & Translate(Word(keyword,1)) = 'SOS' Then filename = 'commsos' || html_ext
  When Words(keyword) > 1 & Translate(Word(keyword,1)) = 'QUERY,' Then filename = 'query' || html_ext
  When keyword \= Translate(keyword) Then filename = 'glossary' || html_ext
  Otherwise 
       Do
         If keyword = 'REXX' | keyword = 'Rexx' Then filename = 'glossary' || html_ext
         else filename = 'comm' || html_ext
       End
End
If trans = 'UPPER' Then name = Strip(SpecialTranslate(keyword),,"'")
Else name = Strip(keyword,,"'")
name = RemoveSpaces(name)
Return '<A HREF = "'filename'#'name'">'||keyword||'</A>'extra

/********************************************************************/
heading: Procedure Expose html_ext
Parse Arg title
Say '<HTML>'
Say '<HEAD><TITLE>THE Reference -' title '</TITLE></HEAD>'
Say '<BODY BGCOLOR="#F1EDD1" LINK = "#0000FF" VLINK = "#FF0022" ALINK = "#808000">'
Say '<CENTER> <img WIDTH="64" HEIGHT="64" HSPACE="20" SRC="the64.png" ALT="THE"> </CENTER>'
Return

/********************************************************************/
footing: Procedure Expose html_ext
Parse Arg src .
Say '<ADDRESS>'
Say 'The Hessling Editor is Copyright &copy; <A HREF = "http://www.lightlink.com/hessling/">Mark Hessling</A>, 1990-'Left( Date('S'), 4 )
Say '&lt;<A HREF = "mailto:M.Hessling@qut.edu.au">M.Hessling@qut.edu.au</A>&gt;'
Say '<BR>Generated on:' Date()
Say '</ADDRESS><HR>'
If src \= 'TOC' Then Say 'Return to <A HREF = "index' || html_ext || '#TOC"> Table of Contents </A><BR>'
Say '</BODY> </HTML>'
Return

/********************************************************************/
handle_keywords: Procedure Expose html_ext
Parse Arg line,syntax_words
tmp = ''
Do Forever    /* handle links */
   Parse Var line pre '<' keyword '>' line
   Select
     When Left(keyword,1) = '(' & Right(keyword,1) = ')' Then
       Do
         tmp = tmp pre '<'Strip(Strip(keyword,'L','('),'T',')')'>'
       End
     When keyword = '' Then tmp = tmp pre
     When Left( keyword, 5 ) = 'http:' Then
        Do
           tmp = tmp pre '<a href="'keyword'">'keyword'</a>'
        End
     Otherwise tmp = tmp Strip(pre) MakeRef('UPPER',,keyword)
   End
   If line = '' Then Leave
End
line = tmp
tmp = ''
Do Forever              /* handle arguments */
   Parse Var line pre "'" keyword "'" line
   Select
     When keyword = '' Then tmp = tmp pre
     Otherwise 
       Do
         If Words(syntax_words) \= 0 & Wordpos(Translate(keyword),Translate(syntax_words)) = 0 Then
            tmp = tmp pre "'"keyword"'"
         Else
            Do
              If pre = '' Then
                 tmp = tmp pre Italic(keyword)
              Else
                 Do
                   strip_pre = Strip(pre)
                   If Substr(strip_pre,Length(strip_pre)) = "#" Then
                      tmp = tmp pre || keyword
                   Else
                      tmp = tmp pre Italic(keyword)
                 End
            End
       End
   End
   If line = '' Then Leave
End
Return tmp

/********************************************************************/
SpecialTranslate: Procedure
Parse Arg keyword
Select
  When keyword = '?' Then Return('QUESTIONMARK')
  When keyword = '=' Then Return('EQUALSIGN')
  When keyword = '!' Then Return('EXCLAIMARK')
  When keyword = '&' Then Return('AMPERSAND')
  Otherwise Return Translate(keyword)
End
Return 'WRONG!!'

/********************************************************************/
RemoveSpaces: Procedure 
Parse Arg val
retval = ''
Do i = 1 To Words(val)
   retval = retval || Word(val,i)
End
Return retval

/********************************************************************/
ConvertSpecialChars: Procedure
Parse Arg val
newval = ''
intag = 0
Do i = 1 To Length(val)
   char = Substr(val,i,1)
   Select
      When Substr(val,i,2) = '<A' Then
         Do
           intag = 1
           newval = newval || Substr(val,i,2)
           i = i + 1
         End
      When Substr(val,i,4) = '</A>' Then
         Do
           intag = 0
           newval = newval || Substr(val,i,4)
           i = i + 3
         End
      When char = '"' & intag = 0 Then newval = newval || '&quot;'
      When char = '>' & intag = 0 Then newval = newval || '&gt;'
      When char = '<' & intag = 0 Then newval = newval || '&lt;'
      Otherwise newval = newval || char
   End
End
Return newval
