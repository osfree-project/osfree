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
Parse Version ?port .
Parse Source ?os .
If ?os = 'OS/2' Then ?slash = '\'
Else ?slash = '/'
Select
  When section = 'TOCSTART'  Then Call toc 'START' ver infile tocfile
  When section = 'TOCEND'    Then Call toc 'END' ver infile tocfile
  When section = 'OVERVIEW'  Then Call overview  infile tocfile 'Overview'
  When section = 'COMM'      Then Call commands  'comm' infile tocfile 'Commands'
  When section = 'COMMSET'   Then Call commands  'commset' infile tocfile 'SET Commands'
  When section = 'COMMSOS'   Then Call commands  'commsos' infile tocfile 'SOS Commands'
  When section = 'QUERY'     Then Call query     infile tocfile 'Query and Extract'
  When section = 'GLOSSARY'  Then Call glossary  infile tocfile 'Glossary'
  When section = 'APPENDIX1' Then Call appendix1 infile tocfile 'Appendix 1'
  When section = 'APPENDIX2' Then Call appendix2 'app2' infile tocfile 'Appendix 2'
  When section = 'APPENDIX3' Then Call appendix2 'app3' infile tocfile 'Appendix 3'
  When section = 'APPENDIX4' Then Call appendix2 'app4' infile tocfile 'Appendix 4'
  When section = 'APPENDIX5' Then Call appendixx infile tocfile 'Appendix 5'
  When section = 'APPENDIX6' Then Call appendixx infile tocfile 'Appendix 6'
  When section = 'APPENDIX7' Then Call appendix2 'app7' infile tocfile 'Appendix 7'
  When section = 'HISTORY'   Then Call history   infile tocfile 'History of THE'
  When section = 'QUICKREF'  Then Call quickref  infile tocfile 'Quick Reference for THE'
  Otherwise
      Do
        Say 'Error: Invalid section:' section 'specified'
      End
End
Return

/********************************************************************/
overview: Procedure Expose html_ext ?port ?slash ?os
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
in_code_section = 'N'
already_read = 'N'
in_list_section = 'N'
first_heading = 'N'
If ?os = 'OS/2' Then
   Do
      here = Directory()
      If Directory('misc') = '' Then Address Command 'mkdir misc'
      Call Directory here
   End
Else
   Do
      If Stream('misc','C','QUERY EXISTS') = '' Then Address System 'mkdir misc'
   End
thisfile = 'misc'?slash'overview' || html_ext
Call heading thisfile,,title
Do While(Lines(infile)>0)
   If already_read = 'N' Then line = Linein(infile)
   already_read = 'N'
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            Call Lineout tocfile, '<DT>'MakeTOC('#',thisfile,line,"")'</DT>'
            Call Lineout thisfile, '<HR>'
            Call Lineout thisfile, '<A NAME="' || RemoveSpaces(line) || '"></A>'
            Call Lineout thisfile, '<H2>' line '</H2>'
            line = Linein(infile)
            Call Lineout thisfile, '<HR>'
            first_heading = 'Y'
          End
     When Strip(line) = Copies('-',74) Then Iterate
     When Substr(Strip(line),1,2) = '- ' Then
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
                            Call Lineout thisfile, '<LI>' handle_keywords(tmp,'') '</LI>'
                            Call Lineout thisfile, '</UL>'
                            in_list_section = 'N'
                          End
                     When Substr(Strip(line),1,2) = '- ' Then
                          Do
                            Call Lineout thisfile, '<LI>' handle_keywords(tmp,'') '</LI>'
                            already_read = 'Y'
                          End
                     Otherwise
                          Do
                            tmp = tmp line
                          End
                   End
                   If in_list_section = 'N' | already_read = 'Y' Then Leave
                End
              End
            Else
              Do
                Call Lineout thisfile, '<UL>'
                in_list_section = 'Y'
                already_read = 'Y'
              End
          End
     When Strip(line) = '+'||Copies('-',30) Then
          Do
            If in_code_section = 'Y' Then
               Do
                 Call Lineout thisfile, '<IMG SRC="../images/divbot.png" ALT="----------"><BR>'
                 Call Lineout thisfile, '</BLOCKQUOTE><P>'
                 in_code_section = 'N'
               End
            Else
               Do
                 Call Lineout thisfile, '<BLOCKQUOTE>'
                 Call Lineout thisfile, '<IMG SRC="../images/divtop.png" ALT="----------"><BR>'
                 in_code_section = 'Y'
               End
          End
      Otherwise
          Do
            If first_heading = 'N' Then Iterate
            If in_code_section = 'Y' Then
               Do
                 Call Lineout thisfile, Strip(line) '<BR>'
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
                        Call Lineout thisfile, tmp '<P>'
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
Call Lineout thisfile, '<P Align="Center"><HR>'
Call Lineout thisfile, 'The HTML version of this manual was inspired by <A HREF = "mailto:judygs@uic.edu">Judith Grobe Sachs </A>'
Call Lineout thisfile, '<P Align="Center"><HR>'
Call footing thisfile
Return

/********************************************************************/
commands: Procedure Expose html_ext ?port ?slash ?os
Parse arg fn infile tocfile title
toc. = ''
toc_idx = 0
already_read_line = 'N'
once = 'Y'
If ?os = 'OS/2' Then
   Do
      here = Directory()
      If Directory( fn ) = '' Then Address Command 'mkdir' fn
      Call Directory here
   End
Else
   Do
      If Stream(fn, 'C', 'QUERY EXISTS') = '' Then Address System 'mkdir' fn
   End
thisfile = ''
Do While(Lines(infile)>0)
   If already_read_line = 'N' Then line = Linein(infile)
   already_read_line = 'N'
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
/*            Say '<DT>'MakeTOC('#','../' || fn || '/index' || html_ext,line,"")'</DT>' */
            Call Lineout tocfile,'<B>'Translate(title)'</B>'
            line = Linein(infile)
          End
     When Strip(line) = Copies('-',74) & thisfile \= '' Then
          Call Lineout thisfile, '<HR>'
     When line = '     COMMAND' Then
          Do
            line = Linein(infile)
            Parse var line keyword '-' desc
            keyword = Strip(keyword)
            keyword_words = Words(keyword)
            If thisfile \= '' Then Call footing thisfile
            thisfile = fn || ?slash || RemoveSpaces(SpecialTranslate(keyword)) || '.html'
            If ?port = 'REXXSAA' & ?os = 'OS/2' Then
               Do
                  Call Stream thisfile, 'C', 'OPEN WRITE'
                  Call Stream thisfile, 'C', 'CLOSE'
               End
            Else
               Call Stream thisfile, 'C', 'OPEN WRITE REPLACE'
            Call heading thisfile,,title '-' keyword
            Call Lineout thisfile, '<HR>'
            Call Lineout thisfile, '<DL><DT>'
            Call Lineout thisfile, Bold(Translate(keyword)) '-' desc '<BR><BR></DT>'
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DD>'MakeTOC('/',thisfile,keyword,desc)'</DD>'
          End
     When line = '     SEE ALSO' Then
          Do
            Call Lineout thisfile, '<DT>' Bold('See Also:') '<BR></DT>'
            line = Linein(infile)
            tmp = ''
            Do Forever
               Parse Var line '<' keyword '>' line
               If keyword = '' Then Leave
               tmp = tmp MakeRef('UPPER',',',keyword)
               If line = '' Then Leave
            End
            tmp = Strip(tmp,'T',',')
            Call Lineout thisfile, '<DD>' tmp '<P></DD>'
          End
     When line = '     STATUS' Then 
          Do
            Call Lineout thisfile, '<DT>' Bold('Status:') '<BR></DT>'
            Do Forever
               line = Linein(infile)
               If line = '' Then Leave
               Call Lineout thisfile, '<DD>' Strip(line) '</DD>'
            End
            Call Lineout thisfile, '</DL><P>'
          End
     When line = '     SYNTAX' Then
          Do
            Call Lineout thisfile, '<DT>' Bold('Syntax:') '<BR></DT>'
            syntax_words = ''
            Do Forever
               line = Linein(infile)
               If line = '' Then Leave
               pre = Subword(line,1,keyword_words)
               post = Subword(line,keyword_words+1)
               Call Lineout thisfile, '<DD>' Strip(pre) Italic(strip(post)) '<P></DD>'
               syntax_words = syntax_words Translate(post,"     ","/|[].")
            End
          End
     When line = '     COMPATIBILITY' Then
          Do
            Call Lineout thisfile, '<DT>' Bold('Compatibility:') '<BR></DT>'
            first = 'Y'
            Do Forever
               line = Linein(infile)
               If line = '' Then Leave
               If first = 'Y' Then
                 Do
                   Call Lineout thisfile, '<DD>' handle_keywords(Strip(line),'')
                   first = 'N'
                 End
               Else
                   Call Lineout thisfile, '</DD><DD>' handle_keywords(Strip(line),'')
            End
            Call Lineout thisfile, '<P></DD>'
          End
     When line = '     DEFAULT' Then
          Do
            Call Lineout thisfile, '<DT>' Bold('Default:') '<BR></DT>'
            Do Forever
               line = Linein(infile)
               If line = '' Then Leave
               Parse Var line pre '<' keyword '>' post
               If keyword \= '' Then line = pre MakeRef('UPPER',,keyword) post
               Call Lineout thisfile, '<DD>' Strip(line) '<P></DD>'
            End
          End
     When line = '     DESCRIPTION' Then
          Do
            Call Lineout thisfile, '<DT>' Bold('Description:') '<BR></DT>'
            tmp = ''
            inblock = 0
            Do Forever
               line = Linein(infile)
               If Substr(Strip(line),1,31) = '+'||Copies('-',30) Then
                 Do
                   Call Lineout thisfile, '<BLOCKQUOTE>'
                   Call Lineout thisfile, '<IMG SRC="../images/divtop.png" ALT="----------"><BR>'
                   Do Forever
                     line = Linein(infile)
                     If Substr(Strip(line),1,31) = '+'||Copies('-',30) Then Leave
                     Call Lineout thisfile, line '<BR>'
                   End
                   Call Lineout thisfile, '<IMG SRC="../images/divbot.png" ALT="----------"><BR>'
                   Call Lineout thisfile, '</BLOCKQUOTE><P>'
                   Iterate
                 End
               If Substr(Strip(line),1,31) = '*'||Copies('-',30) Then
                 Do
                   Call Lineout thisfile, '<PRE>'
                   Do Forever
                     line = Linein(infile)
                     If Substr(Strip(line),1,31) = '*'||Copies('-',30) Then Leave
                     Call Lineout thisfile, line
                   End
                   Call Lineout thisfile, '</PRE><P>'
                   Iterate
                 End
               If Strip(line) = '' Then
                 Do
                   tmp = handle_keywords(tmp,syntax_words)
                   Call Lineout thisfile, '<DD>' Strip(tmp) '<P></DD>'
                   If Substr(tmp,Length(tmp),1) = ':' Then 
                      Do
                        first = 'Y'
                        Do Forever
                           line = Strip(Linein(infile))
                           If line = '' Then Leave
                           If first = 'Y' Then
                             Do
                              Call Lineout thisfile, '<DD>' handle_keywords(line,syntax_keywords)
                              first = 'N'
                             End
                           Else
                              Call Lineout thisfile, '</DD><DD>' handle_keywords(line,syntax_keywords)
                        End
                        Call Lineout thisfile, '<P></DD>'
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
Call footing thisfile
Return

/********************************************************************/
glossary: Procedure Expose html_ext section ?port ?slash ?os
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
already_read_line = 'N'
first_time = 'Y'
thisfile = 'misc'?slash'glossary' || html_ext
Call heading thisfile,,title
Do While(Lines(infile)>0)
   If already_read_line = 'N' Then line = Linein(infile)
   already_read_line = 'N'
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            Call Lineout tocfile, '<DT>'MakeTOC('#',thisfile,line,"")'</DT>'
            Call Lineout thisfile, '<A NAME="' || line || '"></A>'
            Call Lineout thisfile, '<H2>' line '</H2>'
            Call Lineout thisfile, '<HR>'
            line = Linein(infile)
          End
     When Strip(line) = '' Then Nop
     When Strip(line) = Copies('-',74) Then Call Lineout thisfile, '</DL><HR>'
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
            Call Lineout thisfile, '<DD>' Strip(tmp) '<P></DD>'
          End
     Otherwise
          Do
            line = Strip(line)
            Call Lineout thisfile, '<DT><A NAME="' || RemoveSpaces(Translate(line)) || '">' Bold(line) '</A></DT>'
/*
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DD>'MakeTOC('#',thisfile,line,"")'</DD>'
*/
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing thisfile
Return

/********************************************************************/
query: Procedure Expose html_ext ?port ?slash ?os
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
already_read_line = 'N'
once = 'Y'
thisfile = 'misc'?slash'query' || html_ext
Call heading thisfile,,title
in_table = 'N'
first_time = 'Y'
Do While(Lines(infile)>0)
   If already_read_line = 'N' Then line = Linein(infile)
   already_read_line = 'N'
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            Call Lineout tocfile, '<DT>'MakeTOC('#',thisfile,line,"")'</DT>'
            Call Lineout thisfile, '<A NAME="' || RemoveSpaces(line) || '"></A>'
            line = Linein(infile)
          End
     When Strip(line) = '' & in_table = 'Y' Then
          Do
            in_table = 'N'
            Call Lineout thisfile, '</TABLE><P></DD>'
          End
     When Strip(line) = '' Then Nop
     When Strip(line) = Copies('-',74) Then
          Do
            Call Lineout thisfile, '</DL>'
            Call Lineout thisfile, '<HR>'
          End
     When Substr(line,26,3) = ' - ' Then
          Do
            If in_table = 'N' Then
              Do
                Call Lineout thisfile, '<DD><TABLE BORDER=0 CELLSPACING=1 CELLPADDING=1>'
                in_table = 'Y'
              End
            Parse Var line col1 '-' col2
            Call Lineout thisfile, '<TR><TD>'||Strip(col1)||'</TD><TD>-</TD><TD>'||handle_keywords(Strip(col2),'')||'</TD></TR>'
          End
     When Substr(line,1,15) = Copies(' ',15) Then
          Do
            Call Lineout thisfile, '<DD>' Strip(line) '</DD>'
          End
     When Substr(line,1,14) = Copies(' ',14) Then
          Do
            Call Lineout thisfile, '<BLOCKQUOTE>'
            Call Lineout thisfile, '<IMG SRC="../images/divtop.png" ALT="----------"><BR>'
            Call Lineout thisfile, Strip(line) '<BR>'
            Do Forever
              line = Linein(infile)
              If Strip(line) = '' Then Leave
              Call Lineout thisfile, Strip(line) '<BR>'
            End
            Call Lineout thisfile, '<IMG SRC="../images/divbot.png" ALT="----------"><BR>'
            Call Lineout thisfile, '<P></BLOCKQUOTE>'
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
                        Call Lineout thisfile, '<DD>' Strip(tmp) '</DD>'
                        Call Lineout thisfile, '<DD>' line '<P></DD>'
                        Leave
                      End
                 When line = '' Then 
                      Do
                        tmp = handle_keywords(tmp,'')
                        Call Lineout thisfile, '<DD>' Strip(tmp) '<P></DD>'
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
            Call Lineout thisfile, '<A NAME="' || RemoveSpaces(Translate(Strip(pre))) || '"></A>'
            Call Lineout thisfile, '<DT>'
            If post = '' Then Call Lineout thisfile, Bold(Strip(pre)) '<BR>'
            Else Call Lineout thisfile, Bold(Strip(pre)) Italic(strip(post)) '<BR>'
            Call Lineout thisfile, '</DT>'
            syntax_words = Translate(post,"     ","/|[].")
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing thisfile
Return

/********************************************************************/
history: Procedure Expose html_ext ?port ?slash ?os
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
in_env = 'N'
thisfile = 'misc'?slash'history' || html_ext
Call heading thisfile,,title
inlist = 0
ul = ''
Call Lineout tocfile, '<DT>'MakeTOC('#',thisfile,title,"")'</DT>'
Do While(Lines(infile)>0)
   line = Linein(infile)
   Select
     When Strip(line) = '' Then Nop
     When Substr(line,1,7) = 'Version' Then
          Do
            If inlist Then
              Do
                If ul \= '' Then Call Lineout thisfile, ul
                inlist = 0
                Call Lineout thisfile, '</UL>'
              End
            Parse Var line v ver dt tail
            head = v ver
/*
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DD>'MakeTOC('#',thisfile,Strip(head),tail)'</DD>'
*/
            Call Lineout thisfile, '<A NAME="' || RemoveSpaces(Translate(Strip(head))) || '"></A>'
            Call Lineout thisfile, '<HR>'
            Call Lineout thisfile, '<H3>' line '</H3>'
            Call Lineout thisfile, '<HR>'
            ul = ''
          End
     When Substr(line,1,4) = " ---" Then
          Do
            If inlist Then
              Do
                If ul \= '' Then Call Lineout thisfile, ul
                inlist = 0
                Call Lineout thisfile, '</UL>'
              End
            Call Lineout thisfile, '<H4>'||Substr(line,5)||'</H4><UL>'
            ul = ''
          End
     When Substr(line,1,6) = "     o" Then
          Do
            If ul \= '' Then Call Lineout thisfile, ul
            ul = '<LI>' Strip(Substr(line,7))
            inlist = 1
          End
     When line = "         +---------------" Then
          Do
            If ul \= '' Then Call Lineout thisfile, ul
            Call Lineout thisfile, '<PRE>'
            inblock = 0
            Do Forever
               line = Linein(infile)
               If line = "         +---------------" Then Leave
               Call Lineout thisfile, line
            End
            Call Lineout thisfile, '</PRE><P>'
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
If ul \= '' Then Call Lineout thisfile, '<LI>' ul
Call Lineout thisfile, '</UL><HR>'
Call footing thisfile
Return

/********************************************************************/
appendix1: Procedure Expose html_ext ?port ?slash ?os
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
thisfile = 'misc'?slash'app1' || html_ext
Call heading thisfile,,title
in_env = 'N'
Do While(Lines(infile)>0)
   line = Linein(infile)
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            Parse Var line head '-' tail
            Call Lineout tocfile, '<DT>'MakeTOC('#',thisfile,Strip(head),tail)'</DT>'
            Call Lineout thisfile, '<A NAME="' || RemoveSpaces(Strip(head)) || '"></A>'
            Call Lineout thisfile, '<H2>' line '</H2>'
            Call Lineout thisfile, '<HR>'
            line = Linein(infile)
          End
     When Strip(line) = Copies('-',74) Then Call Lineout thisfile, '<HR>'
     When Strip(line) = Copies('*',9) Then Call Lineout thisfile, '<B><H3><CENTER>'line'</CENTER></H3></B>'
     When Substr(Strip(line),1,15) = Copies('-',15) Then
          Do
            line = Strip(Linein(infile))
            Call Lineout thisfile, '<HR>'
            Call Lineout thisfile, '<CENTER><B>'||line||'</B></CENTER>'
            line = Linein(infile)
            Call Lineout thisfile, '<HR>'
          End
     When Substr(line,1,10) = '      THE_' Then
          Do
            Parse Var line env '-' rem
            If in_env = 'N' Then Call Lineout thisfile, '<DL>'
            in_env = 'Y'
            rem = handle_keywords(rem,'')
            Call Lineout thisfile, '<DT><B>'env'</B></DT>'
            Call Lineout thisfile, '<DD>' rem '<P></DD>'
          End
     When Strip(line) = '' & in_env = 'Y' Then 
          Do
            Call Lineout thisfile, '</DL>'
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
          Call Lineout thisfile, tmp '<P>'
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing thisfile
Return

/********************************************************************/
appendix2: Procedure Expose html_ext section ?port ?slash ?os
Parse arg outfn infile tocfile title
toc. = ''
toc_idx = 0
before_table = 1
in_table = 'N'
thisfile = 'misc' || ?slash || outfn || html_ext
Call heading thisfile,,title
Do While(Lines(infile)>0)
   line = Linein(infile)
   Select
     When Strip(line) = Copies('=',72) Then
          Do
            line = Strip(Linein(infile))
            Parse Var line head '-' tail
            Call Lineout tocfile, '<DT>'MakeTOC('#',thisfile,Strip(head),tail)'</DT>'
            Call Lineout thisfile, '<A NAME="' || RemoveSpaces(Strip(head)) || '"></A>'
            Call Lineout thisfile, '<H2>' line '</H2>'
            Call Lineout thisfile, '<HR>'
            line = Linein(infile)
          End
     When Strip(line) = Copies('-',74) Then Call Lineout thisfile, '<HR>'
     When Substr(Strip(line),1,1) = '-' Then
          Do
            line = Linein(infile)
            Call Lineout thisfile, '<B>' Strip(line) '</B><P>'
            line = Linein(infile)
          End
     When Substr(Strip(line),1,2) = '==' Then
          Do
            line = Linein(infile)
            Call Lineout thisfile,'<H2>' Strip(line) '</H2><P>'
            line = Linein(infile)
          End
     When Substr(Strip(line),1,5) = '+----' & Length(Strip(line)) = 31 Then
          Do
            Call Lineout thisfile, '<BLOCKQUOTE>'
            Call Lineout thisfile, '<IMG SRC="../images/divtop.png" ALT="----------"><BR>'
            Do Forever
              line = Strip(Linein(infile))
              If Substr(line,1,5) = '+----' Then Leave
              If Pos('<',line) \= 0 Then
                Do
                  Parse Var line pre '<Key>' rem
                  Call Lineout thisfile, Strip(pre) '&lt;Key&gt;'||rem '<BR>'
                End
              Else
                Call Lineout thisfile, line '<BR>'
            End
            Call Lineout thisfile, '<IMG SRC="../images/divbot.png" ALT="----------"><BR>'
            Call Lineout thisfile, '</BLOCKQUOTE><P>'
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
            Call Lineout thisfile, '<CENTER><TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2>'
            line = Strip(Linein(infile))
            line = Strip(Strip(line,,'|'))
            tmp = '<TR>'
            Do i = 1 To columns
               Parse Var line col '|' line
               If col = '' Then col = '<BR>'
               tmp = tmp'<TH>'Strip(col)'</TH>'
            End
            Call Lineout thisfile, tmp'</TR>'
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
                     Call Lineout thisfile, tmp'</TR>'
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
            Call Lineout thisfile, tmp'</TR></TABLE></CENTER><P>'
          End
     When Strip(line) = '' Then Nop
     When Substr(line,1,7) = Copies(' ',7) Then 
          Do
            Call Lineout thisfile, '<DD>'Strip(line)'</DD>'
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
                                       Call Lineout thisfile, tmp '<BR>'
                                    End
                                 Call Lineout thisfile,'<I><B>'Strip(line)'</B></I><BR>'
                                 tmp = ''
                              End
                           When Substr(line,1,8) = Copies(' ',8) & Substr(line,9,1) \= ' '  Then
                              Do
                                 If tmp \= '' Then
                                    Do
                                       tmp = handle_keywords(tmp,'')
                                       Call Lineout thisfile,tmp '<BR>'
                                    End
                                 Call Lineout thisfile,'<I>'Strip(line)'</I><BR>'
                                 tmp = ''
                              End
                           Otherwise tmp = tmp Strip(line)
                        End
                     End
                  When section = 'APPENDIX7' & Pos(Right( Strip( line ), 1 ), ':;.' ) \= 0 Then
                     Do
                        If before_table Then Call Lineout thisfile,handle_keywords( tmp Strip( line ) )'<BR>'
                        Else Call Lineout thisfile,tmp Strip( line )'<BR>'
                        tmp = ''
                     End
                  Otherwise tmp = tmp Strip(line)
               End
            End
          tmp = handle_keywords(tmp,'')
          Call Lineout thisfile, tmp '<P>'
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing thisfile
Return

/********************************************************************/
appendixx: Procedure Expose html_ext section ?port ?slash ?os
Parse arg infile tocfile title
If section = 'APPENDIX5' Then
   Do
      head = 'DEFAULT STATUS SETTINGS IN THE'
      thisfile = 'misc'?slash'app5' || html_ext
   End
Else 
   Do
      head = 'THE BY TASKS'
      thisfile = 'misc'?slash'app6' || html_ext
   End
Call heading thisfile,,title
Call Lineout tocfile, '<DT>'MakeTOC('#',thisfile,Translate(title),head)'</DT>'
Do While(Lines(infile)>0)
   line = Linein(infile)
   If ?port = 'REXXSAA' & ?os = 'OS/2' Then num = OS2Countstr("A HREF",line)
   Else num = Countstr("A HREF",line)
   outline = ''
   If num = 0 Then Call Lineout thisfile, line
   Else
      Do
         Do num
            Parse Var line start "A HREF" . '=' . '"' loc '"' . '>' rem
            Select
               When Substr(loc,1,6) = 'mailto' Then
                  Do
                     anchor = 'A HREF="' || loc || '">'
                     outline = outline || start || anchor
                     line = rem
                  End
               When Substr(loc,1,7) = 'commsos' Then
                  Do
                     Parse Var loc . '#' cmd .
                     anchor = 'A HREF="../commsos/' || cmd || html_ext || '">'
                     outline = outline || start || anchor
                     line = rem
                  End
               When Substr(loc,1,7) = 'commset' Then
                  Do
                     Parse Var loc . '#' cmd .
                     anchor = 'A HREF="../commset/' || cmd || html_ext || '">'
                     outline = outline || start || anchor
                     line = rem
                  End
               When Substr(loc,1,4) = 'comm' Then
                  Do
                     Parse Var loc . '#' cmd .
                     anchor = 'A HREF="../comm/' || cmd || html_ext || '">'
                     outline = outline || start || anchor
                     line = rem
                  End
               Otherwise
                  Do
                     Parse Var loc pre '.' . '#' cmd .
                     anchor = 'A HREF="../misc/' || pre || html_ext || '#' || cmd || '">'
                     outline = outline || start || anchor
                     line = rem
                  End
            End
         End
         Call Charout thisfile, outline
         Call Lineout thisfile, line
      End
End
Return

/********************************************************************/
quickref: Procedure Expose html_ext ?port ?slash ?os
Parse arg infile tocfile title
toc. = ''
toc_idx = 0
table_head. = ''
colval. = ''
in_table = 'N'
in_start = 'Y'
thisfile = 'misc'?slash'quickref' || html_ext
Call heading thisfile,,title
Call Lineout tocfile,'<DT>'MakeTOC('#',thisfile,title,"")'</DT>'
Call Lineout thisfile, '<A NAME="' || RemoveSPaces(Translate(title)) || '"></A>'
Do While(Lines(infile)>0)
   line = Linein(infile)
   If in_start = 'Y' Then
     Do
       line = Strip(line)
       If Length(line) = 0 Then
         Do
           in_start = 'N'
           Call Lineout thisfile, '<CENTER><HR></CENTER>'
           Iterate
         End
       Call Lineout thisfile, '<CENTER><H1>' line '</H1></CENTER>'
       Iterate
     End
   Select
     When Strip(line) = Copies('-',74) Then Call Lineout thisfile, '<HR>'
     When Substr(Strip(line),1,1) = '-' Then
          Do
            line = Linein(infile)
            header = Strip(line)
/*
            toc_idx = toc_idx + 1
            toc.toc_idx = '<DD>'MakeTOC('#',thisfile,header,'')'</DD>'
*/
            Call Lineout thisfile, '<A NAME="' || RemoveSpaces(Translate(header)) || '"></A>'
            Call Lineout thisfile, '<H2>' header '</H2>'
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
            Call Lineout thisfile, '<TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2>'
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
            Call Lineout thisfile, tmp'</TR>'
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
            Call Lineout thisfile, tmp'</TR></TABLE><P>'
          End
     When Strip(line) = '' Then Nop
     When header = 'Command-line invocation' Then
          Do
            Select
              When Word(line,1) = 'the' Then
                   Do
                     Call Lineout thisfile, '<B>' Strip(line) '</B><P>'
                   End
              When Word(line,1) = 'Where:' Then
                   Do
                     Call Lineout thisfile, '<B>' Strip(line) '</B><P>'
                     Call Lineout thisfile, '<TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2><TR>'
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
                     Call Lineout thisfile, ar'</TD>'
                     Call Lineout thisfile, dr'</TD>'
                     Call Lineout thisfile, '</TR></TABLE><P>'
                   End
              When Word(line,1) = 'Option' Then
                   Do
                     Call Lineout thisfile, '<B>' Strip(line) '</B><BR>'
                   End
              Otherwise Nop
            End
          End
     When header = 'Prefix commands' Then
          Do 
            Call Lineout thisfile, '<TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2><TR>'
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
            Call Lineout thisfile, ar'</TD>'
            Call Lineout thisfile, dr'</TD>'
            Call Lineout thisfile, '</TR></TABLE><P>'
          End
     When header = 'Line Targets' Then
          Do 
            Call Lineout thisfile, '<TABLE BORDER=1 CELLSPACING=1 CELLPADDING=2><TR>'
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
            Call Lineout thisfile, ar'</TD>'
            Call Lineout thisfile, dr'</TD>'
            Call Lineout thisfile, '</TR></TABLE><P>'
            Do Forever
               line = Strip(Linein(infile))
               If line = '' Then Leave
               Call Lineout thisfile, line '<BR>'
            End
          End
     When Substr(line,2,5) = '[SET]' | Substr(line,2,3) = 'SET' Then
          Do
            If Translate(Word(line,2)) = 'SET_COMMAND' Then
               Call Lineout thisfile, '<A HREF = "../comm/SET' || html_ext || '"> SET' Word(line,2) '</A>' Subword(line,3) '<BR>'
            Else
               Call Lineout thisfile, '<A HREF = "../commset/SET' || Translate(Word(line,2)) || html_ext'"> [SET]' Word(line,2) '</A>' Subword(line,3) '<BR>'
          End
     When Substr(line,2,3) = 'SOS' Then
          Do
            If Translate(Word(line,2)) = 'SOS_COMMAND' Then
               Call Lineout thisfile, '<A HREF = "../comm/SOS' || html_ext'"> SOS' Word(line,2) '</A>' Subword(line,3) '<BR>'
            Else
               Call Lineout thisfile, '<A HREF = "../commsos/SOS' || Translate(Word(line,2)) || html_ext'"> SOS' Word(line,2) '</A>' Subword(line,3) '<BR>'
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
            Call Lineout thisfile, '<A HREF = "../comm/' || word1 || html_ext'">' Word(line,1) '</A>' Subword(line,2) '<BR>'
          End
     When Substr(line,1,7) = Copies(' ',7) Then 
          Do
            Call Lineout thisfile, Strip(line)'<BR>'
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
          Call Lineout thisfile, tmp '<P>'
          End
   End
End
Do i = 1 To toc_idx
   Call Lineout tocfile, toc.i
End
Call footing thisfile
Return

/********************************************************************/
toc: Procedure Expose html_ext ?port ?slash ?os
Parse arg action ver section tocfile
If action = 'START' Then 
  Do
    Call heading tocfile,'INDEX','Table of Contents'
    Call Lineout tocfile, '<HR><CENTER><H1>The Hessling Editor</H1> </CENTER>'
    Call Lineout tocfile, '<HR><CENTER><H1>Version' ver '</H1> </CENTER><P>'
    Call Lineout tocfile, '<A NAME="TOC"></A>'
    Select
       When section = 'COMM' Then Call Lineout tocfile, '<HR><CENTER><H2>Commands</H2> </CENTER><HR><DL>'
       When section = 'COMMSET' Then Call Lineout tocfile, '<HR><CENTER><H2>SET Commands</H2> </CENTER><HR><DL>'
       When section = 'COMMSOS' Then Call Lineout tocfile, '<HR><CENTER><H2>SOS Commands</H2> </CENTER><HR><DL>'
       When section = 'GLOSSARY' Then Call Lineout tocfile, '<HR><CENTER><H2>GLOSSARY</H2> </CENTER><HR><DL>'
       When section = 'HISTORY' Then Call Lineout tocfile, '<HR><CENTER><H2>History of THE</H2> </CENTER><HR><DL>'
       When section = 'QUERY' Then Call Lineout tocfile, '<HR><CENTER><H2>Query and Extract</H2> </CENTER><HR><DL>'
       When section = 'APPENDIX1' Then Call Lineout tocfile, '<HR><DL>'
       When section = 'APPENDIX2' Then Call Lineout tocfile, '<HR><DL>'
       When section = 'APPENDIX3' Then Call Lineout tocfile, '<HR><DL>'
       When section = 'APPENDIX4' Then Call Lineout tocfile, '<HR><DL>'
       When section = 'APPENDIX5' Then Call Lineout tocfile, '<HR><DL>'
       Otherwise Call Lineout tocfile, '<HR><CENTER><H2>TABLE OF CONTENTS</H2> </CENTER><HR><DL>'
    End
  End
Else
  Do
    Call Lineout tocfile, '</DL><HR>'
    Call footing tocfile 'TOC'
  End
Return

/********************************************************************/
Bold: Procedure Expose html_ext ?port ?slash ?os
Parse Arg line
Return '<B>'line'</B>'

/********************************************************************/
Italic: Procedure Expose html_ext ?port ?slash ?os
Parse Arg line
Return '<I>'line'</I>'

/********************************************************************/
MakeTOC: Procedure Expose html_ext ?port ?slash ?os
Parse Arg type,fn,keyword,desc
name = RemoveSpaces(SpecialTranslate(keyword))
/* fn = Substr(fn,4) */
If type = '#' Then
   Return '<A HREF = "'fn || '#'name'">' keyword '</A>' desc '<BR>'
Else
   Return '<A HREF = "'fn || '">' keyword '</A>' desc '<BR>'

/********************************************************************/
MakeRef: Procedure Expose html_ext ?port ?slash ?os
Parse Arg trans,extra,keyword
If trans = 'UPPER' Then name = Strip(SpecialTranslate(keyword),,"'")
Else name = Strip(keyword,,"'")
name = RemoveSpaces(name)
Select
  When Words(keyword) > 1 & Translate(Word(keyword,1)) = 'SET' Then 
     Do
        filename = '../commset/'
        name = name || html_ext
     End
  When Words(keyword) > 1 & Translate(Word(keyword,1)) = 'SOS' Then 
     Do
        filename = '../commsos/'
        name = name || html_ext
     End
  When Words(keyword) > 1 & Translate(Word(keyword,1)) = 'QUERY,' Then 
     Do
        filename = '../misc/query' || html_ext || '#'
     End
  When keyword \= Translate(keyword) Then 
     Do
        filename = '../misc/glossary' || html_ext || '#'
     End
  Otherwise 
       Do
         if keyword = 'REXX' Then 
            Do
               filename = '../misc/glossary' || html_ext || '#'
               keyword = 'Rexx'
            End
         else 
            Do
               filename = '../comm/'
               name = name || html_ext
            End
       End
End
Return '<A HREF = "'filename || name'">'||keyword||'</A>'extra

/********************************************************************/
heading: Procedure Expose html_ext ?port ?slash ?os
Parse Arg fn, section, title
Call Lineout fn, '<HTML>'
Call Lineout fn, '<HEAD><TITLE>THE Reference -' title '</TITLE></HEAD>'
Call Lineout fn, '<BODY BGCOLOR="#F1EDD1" LINK = "#0000FF" VLINK = "#FF0022" ALINK = "#808000">'
If section = 'INDEX' Then
   Call Lineout fn, '<CENTER> <img WIDTH="64" HEIGHT="64" HSPACE="20" SRC="images/the64.png"" ALT="THE"> </CENTER>'
Else
   Call Lineout fn, '<CENTER> <img WIDTH="64" HEIGHT="64" HSPACE="20" SRC="../images/the64.png"" ALT="THE"> </CENTER>'
Return

/********************************************************************/
footing: Procedure Expose html_ext ?port ?slash ?os
Parse Arg fn src .
Call Lineout fn, '<ADDRESS>'
Call Lineout fn, 'The Hessling Editor is Copyright &copy; <A HREF = "http://www.lightlink.com/hessling/">Mark Hessling</A>, 1990-'Left( Date('S'), 4 )
Call Lineout fn, '&lt;<A HREF = "mailto:M.Hessling@qut.edu.au">M.Hessling@qut.edu.au</A>&gt;'
Call Lineout fn, '<BR>Generated on:' Date()
Call Lineout fn, '</ADDRESS><HR>'
If src \= 'TOC' Then Call Lineout fn, 'Return to <A HREF = "../index' || html_ext || '#TOC"> Table of Contents </A><BR>'
Call Lineout fn, '</BODY> </HTML>'
Call Stream fn, 'C', 'CLOSE'
Return

/********************************************************************/
handle_keywords: Procedure Expose html_ext ?port ?slash ?os
Parse Arg line,syntax_words
tmp = ''
Do Forever    /* handle links */
   Parse Var line pre '<' keyword '>' line
   Select
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

OS2Countstr: Procedure
Parse Arg needle, haystack
ltest = Length( needle )
Parse Var haystack . '' -(ltest) qtest
If qtest == needle Then haystack = haystack || needle
Do i = 0 Until haystack == ''
   Parse Var haystack . (needle) haystack
End
Return i
