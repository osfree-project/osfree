/*--------------------------------------------------------------------*/
/* REXX script to convert a bunch of .html files into os/2 .ipf files */
/*  which can be converted later into .inf files using ipfc compiler  */
/*                                                                    */
/*               Copyright (c) 1997 by FRIENDS software               */
/*                         All Rights Reserved                        */
/*                                                                    */
/* FidoNet: 2:5030/84.5                                               */
/* e-mail:  Andrew Zabolotny <bit@freya.etu.ru>                       */
/*                                                                    */
/* Updated 1998 by Ulrich M”ller. These changes are (mostly) marked   */
/* with UM in this file. Documentation also updated.                  */
/* Modifications     (C) 1998 Ulrich M”ller                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/* user-customisable section start */

/* A command to convert any image file into os/2 bmp format           */
/* This script requires Image Alchemy for os/2, at least demo version */
/* If someone knows of a free proggy which provides at least partial  */
/* or equivalent functionality, please mail me                        */
/* Global.ImageConvert = 'alchemy.exe -o -O -8 <input> <output> >nul'; */
 Global.ImageConvert = 'gbmsize <input> <output>,1.1 >nul';
/* Executable/description of an external WWW browser to launch when   */
/* user selects an URL link. Normally, you shouldn`t change it (even  */
/* if you have Netscape) since WebEx is found on almost every OS/2    */
/* system, and Navigator is not.                                      */
 Global.WWWbrowser = 'netscape.exe*Netscape';
/* Text to be included on the "URL" pages */
 Global.ExternalLinksTitle = 'Resources on the Internet';
 Global.ExternalLinksText = 'This chapter contains all external links referenced in this book.'||'0d0a'x,
    'Each link contained herein is an Unified Resource Locator (URL) to a certain location'||'0d0a'x,
    'on the Internet. Simply double-click on one of them to launch Netscape'||'0d0a'x,
    'with the respective URL.';
 Global.LaunchNetscapeText = 'Click below to launch '||,
    substr(Global.WWWbrowser, pos('*', Global.WWWbrowser) + 1) 'with this URL:'
/* default book font; use warpsans bold for a nicer-looking books     */
Global.DefaultFont = ':font facename=default size=0x0.';
/* Global.DefaultFont = ':font facename=''WarpSans'' size=9x9.'; */
/* fonts for headings (1 through 6)                                   */
 Global.HeaderFont.1 = ':font facename=''Helv'' size=32x20.';
 Global.HeaderFont.2 = ':font facename=''Helv'' size=20x12.';
 Global.HeaderFont.3 = ':font facename=''Tms Rmn'' size=18x10.'
 Global.HeaderFont.4 = ':font facename=''Tms Rmn'' size=16x8.'
 Global.HeaderFont.5 = ':font facename=''Courier'' size=14x8.'
 Global.HeaderFont.6 = ':font facename=''Helv'' size=14x10.'
/* font for url links (which launches WebExplorer)                    */
 Global.URLinkFont  =  ':font facename=''System VIO'' size=14x8.';
/* proportional font (for <tt>...</tt>                                */
 Global.ProportFont =  ':font facename=''System VIO'' size=14x8.';
/* non-proportional font for CITE, CODE, TT tags */
 Global.CITEFont  =    ':font facename=''Courier'' size=18x12.';
/*  Global.CITEFont =  ':font facename=''System VIO'' size=14x8.'; */

/* end of user-customisable section                                   */
/*--------------------------------------------------------------------*/
'@echo off'
 call rxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 call SysLoadFuncs

 parse arg _cmdLine

/* Verify user-customizations V1.0.18 (2009-02-19) [shl] */

 s = word(Global.ImageConvert, 1)

 if s == '' then do
  say 'Global.ImageConvert must be set to a useful value'
  exit 1
 end
 else do
  if SysSearchPath('PATH', s) = '' then do
   if SysSearchPath('PATH', s'.exe') = '' then do
    say 'Required external utility' s 'not found in PATH'
    if 0 then exit 1
   end
  end
 end

/***************** hard-coded variables **********************/
/* maximal line length for ipfc :-( */
 Global.maxLineLength = 256;
/* unix end-of-line constant */
 Global.EOL = d2c(10);
/* file extensions and name of handler procedures for these; */
/* all other file extensions will be ignored */
 Global.TypeHandler = '*.HTML doParseHTML *.SHTML doParseHTML *.HTM doParseHTML',
                      '*.HT3 doParseHTML *.HTM3 doParseHTML *.TXT doParseText',
                      '*.TEXT doParseText *.CMD doParseText *.BAT doParseText',
                      '*.GIF doParseImage *.JPG doParseImage *.PNG doParseImage';
/* Set up some global variables */
 Global.Picture.0 = 0;                     /* keep track of embedded Pictures */
 Global.LinkID = 0;                         /* total number of external links */
 Global.URLinks = 0;                               /* keep track of url links */
 Global.Title = '';                                             /* book Title */
 Global.HREF = '';   /* Speedup: keep all encountered HREFs and IMG_SRCs in a */
 Global.IMGSRC = '';    /* string so we can use Pos() and WordPos() functions */
 Global.SubLinks = 0;           /* This stem keeps track of the SUBLINKS tags */
 Global.NoSubLinks = 0;       /* This stem keeps track of the NOSUBLINKS tags */

/* Default state for all switches */
 Global.optCO = 1;  /* COlored output */
 Global.optCE = 1;  /* enable CEntering */
 Global.optCH = 0;  /* disable CHecking */
 Global.optP = 1;   /* embed Pictures */
 Global.optS = 1;   /* Sort links */
 Global.optD = 0;   /* Debug log */
 call AnalyseOptions;
 call DefineQuotes;

 call ShowHeader;
 if length(_fName) = 0
  then call ShowHelp;

 Global.oName = _oName;
 if length(Global.oName) = 0
  then do
        i = lastPos('.', _fName);
        if i > 0
         then Global.oName = left(_fName, i)||'ipf';
         else Global.oName = _fName||'.ipf';
       end;
 call SetColor lCyan;
 if Global.OptCH
  then say 'Checking the integrity of links for '_fName;
  else do
        say 'Output goes into 'Global.oName;
        call SysFileDelete(Global.oName);
       end;

 DateTime = Date(n)', 'Time(c);
 call logError ''
 call logError '--- ['DateTime'] conversion started: index file '_fName;

 call putline '.*'copies('-', 76)'*';
 call putline '.*'center('Converted by HTML2IPF from '_fName' at 'DateTime, 76)'*';
 call putline '.*'copies('-', 76)'*';
 call putline ':userdoc.';
 call putline ':docprof toc=12345.';

 call time 'R'
 call ParseFile _fName, 1;
 do until ResolveLinks(1) = 0;
  Global.Sublinks = 0;
  Global.NoSublinks = 0;/* Include all unresolved sublinks */
 end;
 call ConvertPictures;
 call OutputURLs;

 call putline ':euserdoc.';

 call SetColor lCyan;
 elapsed = time('E');
 say 'finished; elapsed time = 'elapsed%3600':'elapsed%60':'trunc(elapsed//60,1);
 DateTime = Date(n)', 'Time(c);
 call logError '--- ['DateTime'] conversion finished';
 call Charout ,d2c(27)'[0m';
exit;

AnalyseOptions:
 _fName = ''; _oName = '';
 do i = 1 to words(_cmdLine)
  nw = word(_cmdLine, i);
  if left(nw, 1) = '-'
   then do
         nw = translate(substr(nw, 2));
         OptState = pos(right(nw, 1), '-+');
         if OptState > 0
          then nw = left(nw, length(nw) - 1);
          else OptState = 2;
     OptState = OptState - 1;
     select
          when abbrev('COLORS', nw, 2)
           then Global.OptCO = OptState;
          when abbrev('CENTER', nw, 2)
           then Global.OptCE = OptState;
          when abbrev('CHECK', nw, 2)
           then Global.OptCH = OptState;
          when abbrev('SORT', nw, 1)
           then Global.OptS = OptState;
          when abbrev('PICTURES', nw, 1)
           then Global.OptP = OptState;
          when abbrev('DEBUG', nw, 1)
           then Global.OptD = OptState;
          otherwise
           do
            call ShowHeader;
            call SetColor lRed;
            say 'Invalid option in command line: 'word(_cmdLine, i);
            call ShowHelp;
           end;
         end;
        end
   else if length(_fName) = 0
         then _fName = nw
         else
        if length(_oName) = 0
         then _oName = nw
         else do
               call ShowHeader;
               call SetColor lRed;
               say 'Extra filename in command line: 'word(_cmdLine, i);
               call ShowHelp;
              end;
 end;
return;

ShowHeader:
 call SetColor white
 say 'ÄÅÄ HTML2IPF ÄÅÄ Version 0.1.0 ÄÅÄ Copyright (c) 1997 by FRIENDS software ÄÅÄ'
return;

ShowHelp:
 call SetColor Yellow
 say 'Usage: HTML2IPF [IndexFilename.HTML] {OutputFilename.IPF} {conversion options}'
 call SetColor lGreen;
 say '[IndexFilename.HTML]'
 call SetColor Green;
 say 'ÀÄ´is the "root" .HTML file to start with'
 call SetColor lGreen;
 say '{OutputFilename.IPF}'
 call SetColor Green;
 say 'ÀÄ´is the output filename (usually with the .IPF extension)'
 call SetColor lGreen;
 say '{conversion options}'
 call SetColor Green;
 say 'ÀÄÂ´are one or more of the following:'
 say '  ÀÂÂ´-CO{LORS}{+|-}'
 say '   ³ÀÙuse (+) or don`t use (-) ansi [c]olors in output'
 say '   ÃÂ´-CE{NTER}{+|-}'
 say '   ³ÀÙenable (+) or disable (-) processing <CENTER> tags'
 say '   ÃÂ´-CH{ECK}{+|-}'
 say '   ³ÀÙenable (+) or disable (-) checking files only'
 say '   ÃÂ´-S{ORT}{+|-}'
 say '   ³ÀÙsort (+) or don`t sort (-) links alphabetically'
 say '   ÃÂ´-P{ICTURES}{+|-}'
 say '   ³ÀÙinclude (+) or don`t include (-) [p]ictures in .IPF file'
 say '   ÀÂ´-D{EBUG}{+|-}'
 say '    ÀÙenable (+) or disable (-) [d]ebug logging into HTML2IPF.LOG'
 call SetColor lCyan;
 say 'default HTML2IPF options:'
 call SetColor Cyan;
 say 'ÀÄ´-COLORS+ -CENTER+ -CHECK- -SORT+ -PICTURES+ -DEBUG-'
exit(1);

ConvertPictures:
 procedure expose Global.;
 if (\Global.optP) | (Global.OptCH) then return;
 do i = 1 to Global.Picture.0
  if stream(Global.Picture.i.dst, 'c', 'Query Exists') = ''
   then call RunCmd Global.ImageConvert, Global.Picture.i.src, Global.Picture.i.dst;
 end;
return;

RunCmd:
 parse arg cmd, in, out;

 call SetColor lGreen
 ip = pos('<input>', cmd);
 if ip <> 0 then cmd = left(cmd, ip - 1)||in||substr(cmd, ip + 7);
 op = pos('<output>', cmd);
 if op <> 0 then cmd = left(cmd, op - 1)||out||substr(cmd, op + 8);
 cmd;
return;

OutputURLs:
/* make a chapter with links to internet locations */
 if Global.URLinks = 0
  then return;
 call putline ':h1 group=99 x=right width=30%.'Global.ExternalLinksTitle;
 call putline Global.DefaultFont;
 call putline ':p.'Global.ExternalLinksText;
/* Sort URLs alphabetically */
 if Global.OptS
  then do i = 1 to Global.URLinks;
        ii = Global.URLinks.i;
        do j = i + 1 to Global.URLinks;
         ji = Global.URLinks.j;
         if Global.LinkID.ji < Global.LinkID.ii
          then do
                tmp = Global.URLinks.i;
                Global.URLinks.i = Global.URLinks.j;
                Global.URLinks.j = tmp;
                ii = ji;
               end;
        end;
       end;
 if Global.OptCH
  then do
        call SetColor LGreen;
        do i = 1 to Global.URLinks
         j = Global.URLinks.i;
         say 'Unresolved link: 'Global.LinkID.j.RealName;
         call logError '--- Unresolved link: 'Global.LinkID.j.RealName;
        end;
        return;
       end;
 Global.CurrentDir = '';
 do i = 1 to Global.URLinks
  j = Global.URLinks.i;
  call putline ':h2 res='GetLinkID(Global.LinkID.j)' group=98 x=right y=bottom width=60% height=40%.'IPFstring(Global.LinkID.j.RealName);
  call putline Global.DefaultFont;
  call putline ':p.:lines align=center.';
  call putline IPFstring(Global.LaunchNetscapeText);
  call putline Global.URLinkFont;
  call putline ':p.:link reftype=launch object='''left(Global.WWWbrowser, pos('*', Global.WWWbrowser) - 1),
                             ''' data='''Global.LinkID.j.RealName'''.';
  call putline IPFstring(Global.LinkID.j.RealName);
  call putline ':elink.:elines.';
 end;
return;

/*
 * ParseFile:
 *      parse a HTML file; called recursively if needed
 */

ParseFile:
    procedure expose Global.;
    parse arg fName, DeepLevel;
    call SetColor Cyan;
    call charout ,'Parsing 'fName' ...';

    Global.CurrentDir = '';
    id = GetLinkID(fName);
    if id > 0 then Global.LinkID.id.Resolved = 1;

    tmp = translate(stream(fName, 'c', 'Query Exists'), '/', '\');
    if length(tmp) = 0
     then do
           call SetColor lRed;
           say ' not found';
           call logError '--- file 'fName' not found';
           return;
          end;
    fName = Shorten(tmp);
    Global.CurrentDir = fileSpec('P', translate(fName, '\', '/'));
    Global.CurrentFile = fName;
    call logError '--- Parsing file "'fName'" ...';

    Global.Article.linkID = '';                   /* --UM: ID for online help linking */
    Global.Article.Group = 0; /* --UM: IPF group to link to for multiple windows */
    Global.Article.XPos = '';                  /* --UM: IPF window display width */
    Global.Article.Width = '';                 /* --UM: IPF window display width */

    Global.Article.Title = '';                                  /* Article Title */
    Global.Article.line.0 = 0;                      /* count of lines in Article */
    Global.Article.Hidden = 0;  /* Is current article hidden from book contents? */
    Global.OpenTag.0 = 0;  /* keep track of open tags to close at end of chapter */
    Global.RefEndTag = '';       /* end tag to put at next end-of-reference <\a> */
    Global.IsParagraph = 0;                   /* We`re inside a <P>...</P> pair? */
    Global.IsGraphic = 0;            /* (UM) We`re inside a <PRE>...</PRE> pair? */
    Global.LastWasATag = 0;         /* (UM) last block was not a text, but a tag */
    Global.IsTable = 0;               /* We`re inside a <TABLE>...</TABLE> pair? */
    Global.IsCentered = 0;          /* We`re inside a <CENTER>...</CENTER> pair? */
    Global.IsOutputEnabled = 1; /* A global switch to enable/disable text output */
    Global.SkipSpaces = 0;                   /* set to 1 in lists to skip spaces */
    Global.AfterBreak = 0;            /* set to 1 after .br to avoid empty lines */
    call PutToken Global.EOL;                     /* initialize output subsystem */
    Global.AfterBreak = 1;              /* avoid empty lines at start of Article */
    Global.EOF = 0;
    Global.CurFont = Global.DefaultFont;
   /* Remember the count of SUBLINKS and NOSUBLINKS to restore it later */
    locSublinks = Global.Sublinks;
    locNoSublinks = Global.NoSublinks;

    fExt = max(lastPos('/', fName), lastPos('\', fName));
    if lastPos('.', fName) > fExt
     then fExt = translate(substr(fName, lastPos('.', fName) + 1))
     else fExt = '';
    fExt = wordpos('*.'fExt, Global.TypeHandler);
    if fExt > 0
     then fExt = word(Global.TypeHandler, fExt + 1)
     else do
           call SetColor lRed;
           say ' unknown file type';
           call logError '--- File 'fName': unknown type - ignored';
           return;
          end;

    select
     when fExt = 'doParseHTML'  then call doParseHTML;
     when fExt = 'doParseImage' then call doParseImage;
     when fExt = 'doParseText'  then call doParseText;
     otherwise call logError 'Unknown file type handler: 'fExt;
    end;
    call ProgressBar;
    call stream Global.CurrentFile, 'c', 'close';            /* close input file */

    if length(Global.Article.Title) = 0
     then Global.Article.Title = IPFstring(filespec('N', translate(fName, '\', '/')));
    if (length(Global.Title) = 0)
     then do
           Global.Title = ':title.'Global.Article.Title;
           call putline Global.Title; IndexFile = 'Y';
          end;

    /* finally write text to output stream */
    call putline '.* Source filename: 'fName;
    if id > 0
     then do
           if (Global.Article.Hidden) & (IndexFile \= 'Y')
            then do
                  i = max(1, DeepLevel - 1);
                  j = ' hide';
                  Global.SubLinks = 1; Global.Sublinks.1 = '*';
                 end;
            else do
                  i = DeepLevel;
                  j = '';
                 end;
           if (Global.Article.Group > 0) then
               j = j' group='Global.Article.Group;
           if (Global.Article.XPos \= '') then
               j = j' x='Global.Article.XPos;
           if (Global.Article.Width \= '') then
               j = j' width='Global.Article.Width;
           if (Global.Article.linkID \= '') then
               j = j' res='Global.Article.linkID
           else j = j' res='id;
           call putline ':h'i j'.'Global.Article.Title;
          end;
    call putline Global.DefaultFont;
    call putline ':p.';
    /* write all the lines */
    do i = 1 to Global.Article.line.0
        /* avoid empty lines, because this will lead
           to spaces in the IPF viewer (UM) */
        i___ = i+1;
        if (length(Global.Article.line.i) > 0) then
        do
            if (left(Global.Article.line.i, 3) = ":p.") & (left(Global.Article.line.i___, 4) = ":li.")
            then
                call putline ".br";
            else
                call putline Global.Article.line.i;
        end
    end;

    drop Global.Article.;

    call SetColor Blue;
    call charout ,' done';
    call CRLF;

    call ResolveLinks DeepLevel+1;

    /* Restore the SUBLINKS and NOSUBLINKS counter */
    Global.Sublinks = locSublinks;
    Global.NoSublinks = locNoSublinks;
return;

ResolveLinks:
 procedure expose Global.;
 arg DeepLevel;
 LinkCount = 0;
 Links.0 = 0;

 do i = 1 to Global.LinkID
  if (\Global.LinkID.i.Resolved)
   then do
         if Global.SubLinks > 0
          then do
                do j = 1 to Global.SubLinks
                 if Pos(Global.SubLinks.j, translate(Global.LinkID.i.InitialName)) = 1
                  then do; j = -1; leave; end;
                end;
                if j \= -1 then Iterate;
               end;
         do j = 1 to Global.NoSubLinks
          if Pos(Global.NoSubLinks.j, translate(Global.LinkID.i.InitialName)) = 1
           then do; j = -1; leave; end;
         end;
         if j = -1 then Iterate;
         Links.0 = Links.0 + 1; j = Links.0;
         Links.j = Global.LinkID.i.RealName;
         Global.LinkID.i.Resolved = 1;
        end;
 end;
 if Global.OptS
  then call SortLinks 1, Links.0;
 if DeepLevel > 6 then DeepLevel = 6;
 do i = 1 to Links.0
  call ParseFile translate(Links.i, '/', '\'), DeepLevel;
  LinkCount = LinkCount + 1;
 end;
 drop Global.SubLinks.;
 drop Global.NoSubLinks.;
return LinkCount;

SortLinks:
 procedure expose Links.;
 arg iLeft, iRight;

 Left = iLeft; Right = iRight;
 Middle = (Left + Right) % 2;
 MidVar = Links.Middle;
 do until Left > Right
  do while Links.Left < MidVar;
   Left = Left + 1;
  end;
  do while Links.Right > MidVar;
   Right = Right - 1;
  end;

  if Left <= Right
   then do
         tmp = Links.Left;
         Links.Left = Links.Right;
         Links.Right = tmp;
         Left = Left + 1;
         Right = Right - 1;
        end;
 end;
 if iLeft < Right
  then call SortLinks iLeft, Right;
 if Left < iRight
  then call SortLinks Left, iRight;
return;

doParseHTML:
 Global.FileContents = '';
 Global.FileSize = chars(fName);                                 /* file size */
 call ParseContents 'EMPTY';
return;

doParseText:
 Global.SubLinks = 1;
 Global.SubLinks.1 = '*';           /* A plain text file cannot have sublinks */
 Global.FileSize = chars(fName);                                 /* file size */
 call PutToken ':lines align=left.';
 call SetFont Global.ProportFont; /* draw text using proportional font */
 do while chars(fName) > 0;
  call ProgressBar;
  Global.FileContents = charin(fName,,4096);
/* remove all \0x0d Characters from output stream */
  do until i = 0
   i = pos(d2c(13), Global.FileContents);
   if i > 0 then Global.FileContents = delstr(Global.FileContents, i, 1);
  end;
  call PutText Global.FileContents;
 end;
 call PutToken ':elines.';
return;

doParseImage:
 _imgBitmap = GetPictureID(fName);
 if (\Global.optP) | (length(_imgBitmap) <= 1)
  then do
        if Global.optP
         then do
               call SetColor Yellow;
               parse value SysCurPos() with row col;
               if col > 0 then call CRLF;
               say 'Warning: Picture "'Global._imgname'" missing';
               call logError 'Picture "'Global._imgname'" missing';
              end;
        call PutText ':lines align=center.';
        call PutText fName;
        call PutText ':elines.';
       end
  else do
        Global.Picture.0 = Global.Picture.0 + 1;
        i = Global.Picture.0;
        Global.Picture.i.dst = left(_imgBitmap, pos('*', _imgBitmap) - 1);
        Global.Picture.i.src = substr(_imgBitmap, pos('*', _imgBitmap) + 1);
        Global.Picture.i.alt = fName;
        call PutToken ':artwork name='''Global.Picture.i.dst''' align=center.';
       end;
return;

/*
 * ParseContents:
 *      called for each HTML file.
 *      Arguments: either EMPTY, HEAD, or BODY.
 */

ParseContents:
    procedure expose Global.;
    arg TextHandler;

    /* loop thru file contents */
    do until (length(Global.FileContents) = 0) & (Global.EOF)

        /* read next text chunk */
        Token = GetToken();

        if left(Token, 1) = d2c(0) then
        do
            /* ** tag found */
            Token = strip(substr(Token, 2));
            /* assume everything starting with <! is not important */
            if left(Token, 1) = '!'
                then iterate;
            /* find out the tag */
            Tag = strip(translate(Token, xrange('A','Z')'_!', xrange('a','z')'-/'));
            TagBreakPos = pos(' ', Tag);
            if (TagBreakPos > 0)
                then Tag = left(Tag, TagBreakPos - 1);
            TagBreakPos = 0;
            select
                when Tag = 'HTML' then TagBreakPos = doTagHTML();
                when Tag = '!HTML'    then TagBreakPos = doTag!HTML();
                when Tag = 'HEAD' then TagBreakPos = doTagHEAD();
                when Tag = '!HEAD'    then TagBreakPos = doTag!HEAD();
                when Tag = 'BODY' then TagBreakPos = doTagBODY();
                when Tag = '!BODY'    then TagBreakPos = doTag!BODY();
                when Tag = 'META' then TagBreakPos = doTagMETA();
                when Tag = 'TITLE'    then TagBreakPos = doTagTITLE();
                when Tag = '!TITLE'   then TagBreakPos = doTag!TITLE();
                when Tag = 'META' then TagBreakPos = doTagMETA();
                when Tag = 'A'    then TagBreakPos = doTagA();
                when Tag = '!A'   then TagBreakPos = doTag!A();
                when Tag = 'IMG'  then TagBreakPos = doTagIMG();
                when Tag = 'I'    then TagBreakPos = doTagI();
                when Tag = '!I'   then TagBreakPos = doTag!I();
                when Tag = 'B'    then TagBreakPos = doTagB();
                when Tag = '!B'   then TagBreakPos = doTag!B();
                when Tag = 'U'    then TagBreakPos = doTagU();
                when Tag = '!U'   then TagBreakPos = doTag!U();
                when Tag = 'EM'   then TagBreakPos = doTagEM();
                when Tag = '!EM'  then TagBreakPos = doTag!EM();
                when Tag = 'TT'   then TagBreakPos = doTagTT();
                when Tag = '!TT'  then TagBreakPos = doTag!TT();
                when Tag = 'P'    then TagBreakPos = doTagP();
                when Tag = '!P'   then TagBreakPos = doTag!P();
                when Tag = 'H1'   then TagBreakPos = doTagH1();
                when Tag = '!H1'  then TagBreakPos = doTag!H1();
                when Tag = 'H2'   then TagBreakPos = doTagH2();
                when Tag = '!H2'  then TagBreakPos = doTag!H2();
                when Tag = 'H3'   then TagBreakPos = doTagH3();
                when Tag = '!H3'  then TagBreakPos = doTag!H3();
                when Tag = 'H4'   then TagBreakPos = doTagH4();
                when Tag = '!H4'  then TagBreakPos = doTag!H4();
                when Tag = 'H5'   then TagBreakPos = doTagH5();
                when Tag = '!H5'  then TagBreakPos = doTag!H5();
                when Tag = 'H6'   then TagBreakPos = doTagH6();
                when Tag = '!H6'  then TagBreakPos = doTag!H6();
                when Tag = 'OL'   then TagBreakPos = doTagOL();
                when Tag = '!OL'  then TagBreakPos = doTag!OL();
                when Tag = 'UL'   then TagBreakPos = doTagUL();
                when Tag = '!UL'  then TagBreakPos = doTag!UL();
                when Tag = 'LI'   then TagBreakPos = doTagLI();
                when Tag = 'DL'   then TagBreakPos = doTagDL();
                when Tag = '!DL'  then TagBreakPos = doTag!DL();
                when Tag = 'DT'   then TagBreakPos = doTagDT();
                when Tag = 'DD'   then TagBreakPos = doTagDD();
                when Tag = 'BR'   then TagBreakPos = doTagBR();
                when Tag = 'CITE' then TagBreakPos = doTagCITE();
                when Tag = '!CITE'    then TagBreakPos = doTag!CITE();
                when Tag = 'CENTER'   then TagBreakPos = doTagCENTER();
                when Tag = '!CENTER'  then TagBreakPos = doTag!CENTER();
                when Tag = 'PRE'  then TagBreakPos = doTagPRE();
                when Tag = '!PRE' then TagBreakPos = doTag!PRE();
                when Tag = 'META' then TagBreakPos = doTagMETA();
                when Tag = 'MENU' then TagBreakPos = doTagMENU();
                when Tag = '!MENU'    then TagBreakPos = doTag!MENU();
                when Tag = 'CODE' then TagBreakPos = doTagCODE();
                when Tag = '!CODE'    then TagBreakPos = doTag!CODE();
                when Tag = 'STRONG'   then TagBreakPos = doTagSTRONG();
                when Tag = '!STRONG'  then TagBreakPos = doTag!STRONG();
                when Tag = 'ADDRESS'  then TagBreakPos = doTagADDRESS();
                when Tag = '!ADDRESS' then TagBreakPos = doTag!ADDRESS();
                when Tag = 'HR'   then TagBreakPos = doTagHR();
                when Tag = 'TABLE'    then TagBreakPos = doTagTABLE();
                when Tag = '!TABLE'   then TagBreakPos = doTag!TABLE();
                when Tag = 'TR'   then TagBreakPos = doTagTR();
                when Tag = '!TR'  then TagBreakPos = doTag!TR();
                when Tag = 'TH'   then TagBreakPos = doTagTH();
                when Tag = '!TH'  then TagBreakPos = doTag!TH();
                when Tag = 'TD'   then TagBreakPos = doTagTD();
                when Tag = '!TD'  then TagBreakPos = doTag!TD();
                when Tag = 'BLOCKQUOTE'then TagBreakPos = doTagBLOCKQUOTE();
                when Tag = '!BLOCKQUOTE'then TagBreakPos = doTag!BLOCKQUOTE();
                otherwise call logError 'Unexpected tag <'Token'>';
            end; /* select */
            if TagBreakPos then leave;
              end;
        else /* no token, but text: */
            select
               when TextHandler = 'EMPTY' then call doTextEMPTY;
               when TextHandler = 'HEAD'  then call doTextHEAD;
               when TextHandler = 'BODY'  then call doTextBODY;
            end;
    end;
return;

ParseTag:
 procedure expose Global.;
 parse arg Tag;
 parse var Tag Prefix Tag
 Prefix = translate(Prefix);
 do while length(Tag) > 0
  parse value translate(Tag, ' ', Global.EOL) with subTag '=' Tag;
  Tag = strip(Tag, 'leading');
  if left(Tag, 1) = '"'
   then parse var Tag '"' subTagValue '"' Tag
   else parse var Tag subTagValue Tag;
  subTag = translate(strip(subTag));
  subTagValue = strip(subTagValue);
  select
   when Prefix = 'A'
    then select
          when subTag = 'AUTO'      then call doTagA_AUTO;
          when subTag = 'HREF'      then call doTagA_HREF;
          when subTag = 'NAME'      then call doTagA_NAME;
          otherwise call logError 'Unexpected subTag 'subTag'="'subTagValue'"';
         end;
   when Prefix = 'IMG'
    then select
      when subTag = 'SRC'       then call doTagIMG_SRC;
      when subTag = 'ALT'       then call doTagIMG_ALT;
      when subTag = 'ALIGN'     then call doTagIMG_ALIGN;
      when subTag = 'WIDTH'     then call doTagIMG_WIDTH;
      when subTag = 'HEIGHT'    then call doTagIMG_HEIGHT;
          otherwise call logError 'Unexpected subTag 'subTag'="'subTagValue'"';
         end;
   when Prefix = 'HTML'
    then select
      when subTag = 'HIDDEN'    then call doTagHTML_HIDDEN;
      when subTag = 'SUBLINKS'  then call doTagHTML_SUBLINKS;
      when subTag = 'NOSUBLINKS'    then call doTagHTML_NOSUBLINKS;

      when subTag = 'ID'        then call doTagHTML_ID;         /* --UM */
      when subTag = 'GROUP'     then call doTagHTML_GROUP;      /* --UM */
      when subTag = 'WIDTH'     then call doTagHTML_WIDTH;      /* --UM*/
      when subTag = 'XPOS'      then call doTagHTML_XPOS;       /* --UM*/
          otherwise call logError 'Unexpected subTag 'subTag'="'subTagValue'"';
         end;
  end;
 end;
return;

doTagHTML:
 call ParseTag Token;
 call ParseContents 'EMPTY';
return 0;

doTag!HTML:
return 1;

doTagHTML_ID:                           /* --UM */
 Global.Article.linkID = SubtagValue;
return 0;

doTagHTML_GROUP:                        /* --UM */
 Global.Article.Group = SubtagValue;
return 0;

doTagHTML_WIDTH:                        /* --UM */
 Global.Article.Width = SubtagValue;
return 0;

doTagHTML_XPOS:                         /* --UM */
 Global.Article.XPos = SubtagValue;
return 0;

doTagHTML_HIDDEN:
 Global.Article.Hidden = 1;
return 0;

doTagHTML_SUBLINKS:
 Global.SubLinks = Global.SubLinks + 1;
 i = Global.SubLinks;
 Global.SubLinks.i = translate(SubTagValue);
return 0;

doTagHTML_NOSUBLINKS:
 Global.NoSubLinks = Global.NoSubLinks + 1;
 i = Global.NoSubLinks;
 Global.NoSubLinks.i = translate(SubTagValue);
return 0;

doTagHEAD:
 Global.grabTitle = 0;
 call ParseContents 'HEAD';
return 0;

doTag!HEAD:
 Global.grabTitle = 0;
return 1;

doTagBODY:
 Global.grabTitle = 0;
 call ParseContents 'BODY';
return 0;

doTag!BODY:
return 1;

doTagTITLE:
 Global.grabTitle = 1;
 Global.Article.Title = '';
return 0;

doTag!TITLE:
 Global.grabTitle = 0;
return 0;

doTagEM:
doTagI:
 call PutToken ":hp1.";
return 0;

doTag!EM:
doTag!I:
 call PutToken ":ehp1.";
return 0;

doTagB:
doTagSTRONG:
 call PutToken ':hp2.';
return 0;

doTag!B:
doTag!STRONG:
 call PutToken ':ehp2.';
return 0;

doTagU:
 call PutToken ':hp5.';
return 0;

doTag!U:
 call PutToken ':ehp5.';
return 0;

/* doTagEM:
 call PutToken ':hp3.';
return 0;

doTag!EM:
 call PutToken ':ehp3.';
return 0; */

doTagCITE:
doTagCODE:
doTagTT:
 call SetFont Global.CITEFont;
return 0;

doTag!CITE:
doTag!CODE:
doTag!TT:
 call SetFont Global.DefaultFont;
return 0;

doTagBLOCKQUOTE:
doTagP:
 call NewLine;
 call PutToken ':p.';
 Global.IsParagraph = 1;
return 0;

doTag!BLOCKQUOTE:
doTag!P:
 call NewLine;
 Global.IsParagraph = 0;
return 0;

doTagBR:
    if Global.IsTable then
        return 0; /* IPFC does not allow .br`s in tables */
    Global.AfterBreak = 0;
    call NewLine;
    call PutToken '.br';
    call NewLine;
    Global.AfterBreak = 1;
    if doCheckTag(':eul.') | doCheckTag(':edl.') | doCheckTag(':eol.') then
        Global.SkipSpaces = 1;
return 0;

doTagPRE:
    call NewLine;
    call PutToken ':cgraphic.';
    Global.IsGraphic = 1;   /* UM */
return 0;

doTag!PRE:
    call PutToken ':ecgraphic.';
    Global.IsGraphic = 0;   /* UM */
    /* HTML compatibility: add extra line break (UM)*/
    Global.AfterBreak = 0;
    call NewLine;
    call PutToken ':p.';
    call NewLine;
    Global.AfterBreak = 1;
    if doCheckTag(':eul.') | doCheckTag(':edl.') | doCheckTag(':eol.') then
        Global.SkipSpaces = 1;
    /* call PutToken ':p.'; */
return 0;

doTagH_begin:
    arg i;
    call NewLine;
    if \Global.IsTable
     then do; call PutToken '.br'; call NewLine; end;
    call SetFont Global.HeaderFont.i;
    if \Global.IsTable
     then do; call NewLine; call PutToken '.br'; end;
    call NewLine;
    Global.AfterBreak = 1;
return;

doTagH1:
 call doTagH_begin 1;
return 0;

doTagH2:
 call doTagH_begin 2;
return 0;

doTagH3:
 call doTagH_begin 3;
return 0;

doTagH4:
 call doTagH_begin 4;
return 0;

doTagH5:
 call doTagH_begin 5;
return 0;

doTagH6:
 call doTagH_begin 6;
return 0;

doTag!H1:
doTag!H2:
doTag!H3:
doTag!H4:
doTag!H5:
doTag!H6:
 call SetFont Global.DefaultFont;
 if \Global.IsTable
  then do
        call NewLine; call PutToken '.br'; call NewLine;
        call NewLine; call PutToken '.br';
       end;
 call NewLine;
 Global.AfterBreak = 1;
return 0;

doTagHR:
 call NewLine;
 call PutToken ':cgraphic.'copies('Ä', 80)':ecgraphic.';
 call doTagBR;
return 0;

doTagOL:
 if Global.IsTable
  then return 0;
 call doOpenOL;
return 0;

doTag!OL:
 if Global.IsTable
  then return 0;
 call NewLine;
 call doCloseTag ':eol.';
 call doTagBR;              /* UM */
return 0;

doTagMENU:
doTagUL:
 if Global.IsTable
    then return 0;
 call doOpenUL;
return 0;

doTag!MENU:
doTag!UL:
 if Global.IsTable
    then return 0;
 call NewLine;
 call doCloseTag ':eul.';
 call doTagBR;              /* UM */
return 0;

doTagLI:
    if Global.IsTable
        then return 0;
    if (doCheckTag(':eul.') = 0) & (doCheckTag(':eol.') = 0)
        then call doOpenUL;
    call NewLine;
    call PutToken ':li.';
    call NewLine;
    Global.SkipSpaces = 1;
return 0;

doTagDL:
 if Global.IsTable
    then return 0;
 call doOpenDL;
return 0;

doTag!DL:
 if Global.IsTable
  then return 0;
 call NewLine;
 if \Global.DLDescDefined
  then call doTagDD;
 call doCloseTag ':edl.';
return 0;

doTagDT:
 if Global.IsTable
  then return 0;
 if doCheckTag(':edl.') = 0
  then call doOpenDL;
 call NewLine; call PutToken ':dt.';
 Global.SkipSpaces = 1;
 Global.DLTermDefined = 1;
 Global.DLDescDefined = 0;
return 0;

doTagDD:
 if Global.IsTable
  then return 0;
 if doCheckTag(':edl.') = 0
  then call doOpenDL;
 call NewLine;
 if \Global.DLTermDefined
  then call doTagDT;
 call PutToken ':dd.';
 Global.SkipSpaces = 1;
 Global.DLTermDefined = 0;
 Global.DLDescDefined = 1;
return 0;

doTagA:
 call CloseRef;
 call ParseTag Token;
return 0;

doTag!A:
 call CloseRef;
return 0;

doTagA_HREF:
 i = GetLinkID(subTagValue);
 if i > 0 then do /* changed, --UM */
    _data = translate(Global.LinkID.i)
    if (pos('.INF', _data) > 0) then do
        if (pos('#', _data) > 0) then
            _data = translate(Global.LinkID.i, " ", "#")
        else _data = Global.LinkID.i
        call PutToken ":link reftype=launch object='view.exe' data='"_data"'.";
        Global.CurLink = i;
        Global.RefEndTag = ':elink.'||Global.RefEndTag;
    end
    else do
        call PutToken ':link reftype=hd res='i'.';
        Global.CurLink = i;
        Global.RefEndTag = ':elink.'||Global.RefEndTag;
    end;
  end
return 0;

doTagA_AUTO: /* new, --UM */
 i = GetLinkID(subTagValue);
 if i > 0 then do /* changed, UM */
    call PutToken ':link reftype=hd res='i' auto dependent.';
    Global.CurLink = i;
  end
return 0;

doTagA_NAME:
 /* ignore */
return 0;

doTagIMG:
 Global._altName = 'missing Picture';
 Global._imgName = '';
 if Global.IsCentered /* Choose default picture alignment */
  then Global._imgAlign = 'center';
  else Global._imgAlign = 'left';
 call ParseTag Token;
 _imgBitmap = GetPictureID(Global._imgName);
 if (\Global.optP) | (length(_imgBitmap) <= 1),
    | Global.IsTable       /* Since IPF does not allow pictures in tables :-( */
  then do
        if Global.optP & \Global.IsTable
         then do
               call SetColor Yellow;
               parse value SysCurPos() with row col;
               if col > 0 then call CRLF;
               say 'Warning: Picture "'Global._imgName'" missing';
               call logError 'Picture "'Global._imgName'" missing';
              end;
        call PutText ' 'Global._altName' ';
       end
  else do
        if pos(':elink.', Global.RefEndTag) > 0
         then do /* image is a link */
               call PutToken ':elink.';
              end;
        if Global.IsParagraph
         then call PutToken Global.EOL;
        Global.Picture.0 = Global.Picture.0 + 1;
        i = Global.Picture.0;
        Global.Picture.i.dst = left(_imgBitmap, pos('*', _imgBitmap) - 1);
        Global.Picture.i.src = substr(_imgBitmap, pos('*', _imgBitmap) + 1);
        Global.Picture.i.alt = Global._altName;
        call PutToken ':artwork name='''Global.Picture.i.dst''' align='Global._imgAlign;
        if Global.IsParagraph
         then call PutToken ' runin.';
         else call PutToken '.';
        if pos(':elink.', Global.RefEndTag) > 0
         then do /* image is a link */
               call PutToken ':artlink.:link reftype=hd res='Global.CurLink'.:eartlink.';
               call PutToken ':link reftype=hd res='Global.CurLink'.';
              end;
       end;
return 0;

doTagIMG_ALIGN:
 if pos('<'translate(subTagValue)'>', '<LEFT><RIGHT><CENTER>') > 0
  then Global._imgAlign = subTagValue;
return 0;

doTagIMG_SRC:
 Global._imgName = subTagValue;
return 0;

doTagIMG_ALT:
 Global._altName = subTagValue;
return 0;

doTagIMG_WIDTH:
doTagIMG_HEIGHT:
/* nop */
return 0;

doTagADDRESS:
/* nop */
return 0;

doTag!ADDRESS:
/* nop */
return 0;

doTagMETA:
/* nop */
return 0;

doTagCENTER:
 if \Global.OptCE
  then return 0;
 Global.IsCentered = 1;
 call PutToken ':lines align=center.';
return 0;

doTag!CENTER:
 if \Global.OptCE
  then return 0;
 if Global.IsCentered
  then do
        Global.IsCentered = 0;
        call NewLine;
        call PutToken ':elines.';
       end;
return 0;

doTagTABLE:
 Global.Table.WasCentered = Global.IsCentered;
 if Global.IsCentered
  then call doTag!CENTER;
 call NewLine;
 Global.AfterBreak = 0;
 call PutToken '.* table';
 Global.Table.Begin = Global.Article.Line.0;
 call NewLine;
 Global.Table.Width = 0;
 Global.Table.MaxWidth = 0;
 Global.AfterBreak = 1;
 Global.IsTable = 1;
 Global.IsOutputEnabled = 0;
return 0;

doTag!TABLE:
 call NewLine;
 if (Global.IsTable)
  then do
        i = Global.Table.Begin;
        if Global.Table.MaxWidth > 0
         then ColWidth = (79 - Global.Table.MaxWidth) % Global.Table.MaxWidth
         else tableCols = 78;
        tableCols = '';
        do j = 1 to Global.Table.MaxWidth
         tableCols = tableCols' 'ColWidth;
        end;
        if \Global.OptCH
         then Global.Article.Line.i = ':table cols='''substr(tableCols, 2)'''.';
        call PutToken ':etable.';
       end;
 Global.Table.Begin = 0;
 Global.IsTable = 0;
 Global.IsOutputEnabled = 1;
 if Global.Table.WasCentered
  then call doTagCENTER;
return 0;

doTagTR:
 call PutToken ':row.';
 call PutToken Global.EOL;
 Global.IsOutputEnabled = 0;
return 0;

doTag!TR:
 call CloseRef;
 if Global.Table.Width > Global.Table.MaxWidth
  then Global.Table.MaxWidth = Global.Table.Width;
 Global.Table.Width = 0;
return 0;

doTagTH:
 Global.IsOutputEnabled = 1;
 Global.Table.Width = Global.Table.Width + 1;
 call PutToken ':c.'; call doTagU;
return 0;

doTag!TH:
 call CloseRef;
 call doTag!U;
return 0;

doTagTD:
 Global.IsOutputEnabled = 1;
 Global.Table.Width = Global.Table.Width + 1;
 call PutToken ':c.';
return 0;

doTag!TD:
 call CloseRef;
return 0;

doTextEMPTY:
 Token = translate(Token, ' ', xrange(d2c(0),d2c(31)));
 if length(strip(Token)) > 0
  then call logError 'Unexpected text 'Token;
return;

doTextHEAD:
 if Global.grabTitle = 1
  then Global.Article.Title = Global.Article.Title||IPFstring(translate(Token, '  ', d2c(9)d2c(10)))
  else call dotextempty;
return;

/*
 * doTextBODY:
 *      called by ParseContents for plain text.
 *      Token normally contains a whole paragraph (<P>Token</P>.
 */

doTextBODY:
    call PutText Token;
return;

doOpenOL:
 call NewLine;
 call doOpenTag ':ol compact.',':eol.';
return;

doOpenUL:
 call NewLine;
 call doOpenTag ':ul compact.',':eul.';
return;

doOpenDL:
 call NewLine;
 call doOpenTag ':dl compact break=all.', ':edl.';
 Global.DLTermDefined = 0;
 Global.DLDescDefined = 0;
return;

CloseRef:
 call PutToken Global.RefEndTag;
 Global.RefEndTag = '';
return;

/* recursive Tags management */
doOpenTag:
 parse arg ot, ct;
 call PutToken ot;
 Global.OpenTag.0 = Global.OpenTag.0 + 1;
 i = Global.OpenTag.0;
 Global.OpenTag.i = ct;
 Global.OpenTag.i.open = ot;
return;

/*
 * doCloseTag:
 *      puts out closing tags (</UL> etc.).
 *      Arguments: IPF closing tag (e.g. ":eul.")
 */

doCloseTag:
    parse arg bottom;
    if (length(bottom) = 0) then
        i = 1
    else do i = Global.OpenTag.0 to 0 by -1
        if bottom = Global.OpenTag.i
            then leave;
        end;

    if (i > 0)
    then do
        call NewLine;
        do j = Global.OpenTag.0 to i by -1
            call PutToken Global.OpenTag.j;
            call PutToken Global.EOL;
        end;
        Global.OpenTag.0 = i - 1;
        return 1;
    end;
return 0;

doCheckTag:
 parse arg SearchArg;
 do i = Global.OpenTag.0 to 1 by -1
  if pos(SearchArg, Global.OpenTag.i) > 0
   then return 1;
 end;
return 0;

/* Set the current font in output stream */
SetFont:
 parse arg Font;
 if Global.IsTable
  then return;
 if Global.CurFont = Font
  then return;
 Global.CurFont = Font;
 call PutToken Font;
return;

/* Get id number depending of link value (<A HREF=...>) */
/* Returns 0 if link belongs to same page (alas, IPF doesn`t permit this...) */
GetLinkID:
 procedure expose Global.;
 parse arg link;

 InitialLink = link;
 if (pos('#', link) > 0) & (pos('.INF', translate(link)) = 0)
  then link = left(link, pos('#', link) - 1);
 if length(link) = 0
  then return 0;
 link = FindFile(link);
 ulink = translate(link);
 i = wordpos(ulink, Global.HREF);
 if i > 0 then return i;
 Global.LinkID = Global.LinkID + 1;
 i = Global.LinkID;
 Global.LinkID.i = ulink;
 Global.LinkID.i.RealName = link;
 Global.LinkID.i.InitialName = InitialLink;
 Global.HREF = Global.HREF||ulink||' ';
 if (pos(".INF", translate(link)) > 0 ) then do /* inserted, UM */
        Global.LinkID.i.Resolved = 1;
 end
 else if (length(stream(link, 'c', 'query exists')) = 0)
  then do
        Global.LinkID.i.Resolved = 1;
        Global.URLinks = Global.URLinks + 1;
        j = Global.URLinks;
        Global.URLinks.j = i;
        parse var link prot ':' location;
        if (length(location) = 0) | (pos('/', prot) > 0)
         then Global.LinkID.i.RealName = filespec('N', translate(link, '\', '/'))
       end;
  else Global.LinkID.i.Resolved = 0;
return i;

/* transform image extension into .bmp */
GetPictureID:
 procedure expose Global.;
 parse arg PictName;

 PictName = FindFile(PictName);
 if length(stream(PictName, 'c', 'query exists')) > 0
  then do
        tmp = PictName;
        i = lastPos('.', tmp);
        if i > 0
         then PictName = left(tmp, i)||'bmp';
         else PictName = tmp||'.bmp';
       end
  else do
        tmp = '';
        PictName = '';
       end;
return PictName||'*'||tmp;

/* Actively search for file on all possible paths */
FindFile:
 parse arg fName;

 ifName = fName;
 parse var fName prot ':' location;
 if (length(location) > 0) & (pos('/', prot) = 0)
  then fName = location;
 tmp = '';
 do while length(fName) > 0
  do while pos(left(fName, 1), '/\') > 0
   fName = substr(fName, 2);
  end;
  if length(fName) = 0
   then leave;
  tmp = stream(fName, 'c', 'query exists');
  if length(tmp) > 0 then return Shorten(tmp);
  tmp = stream(Global.CurrentDir||fName, 'c', 'query exists');
  if length(tmp) > 0 then return Shorten(tmp);
  tmp1 = Pos('/', fName);
  tmp2 = Pos('\', fName);
  if (tmp2 < tmp1) & (tmp2 > 0)
   then tmp = tmp2
   else tmp = tmp1;
  if tmp > 0
   then fName = substr(fName, tmp)
   else fName = '';
 end;
return ifName;

/*
 * GetToken:
 *      return next Token (a Tag or a text string) from input stream
 */

GetToken:
    procedure expose Global.;
    if (length(Global.FileContents) < 512) & (\Global.EOF) then
GetData:
        do
            /* read next chunk of file */
            Global.FileContents = Global.FileContents||charin(Global.CurrentFile,,1024);
            call ProgressBar;
            /* remove all \0x0d Characters from input stream */
            do until i = 0
                i = pos('0D'x, Global.FileContents);
                if i > 0 then Global.FileContents = delstr(Global.FileContents, i, 1);
            end;
            Global.EOF = (chars(Global.CurrentFile) = 0);
        end;

    i = pos('<', Global.FileContents);
    if (i = 0) then
        if (\Global.EOF)
            then signal GetData;
        else do
              i = length(Global.FileContents) + 1;
              if i = 1 then return '';
        end;
    if (i = 1)
        then do
            j = pos('>', Global.FileContents);
            if (j = 0) then
                if \Global.EOF then
                    signal GetData;
                else
                    j = length(Global.FileContents) + 1;
            Token = '00'x||substr(Global.FileContents, 2, j - 2);
            Global.FileContents = substr(Global.FileContents, j + 1);
        end
        else do
            Token = NoQuotes(left(Global.FileContents, i - 1));
            Global.FileContents = substr(Global.FileContents, i);
        end;
return Token;

/*
 * PutToken:
 *      put an IPF Token into Global.Article.Line. stem
 */

PutToken:
    procedure expose Global.;
    parse arg Output;

    if Global.OptCH then return;

    if (Output = Global.EOL) then
        if (Global.AfterBreak)
            then Global.AfterBreak = 0;
            else do
                 Global.Article.line.0 = Global.Article.line.0 + 1;
                 i = Global.Article.line.0;
                 Global.Article.line.i = '';
            end;
        else do
            Global.AfterBreak = 0;
            i = Global.Article.line.0;
            if length(Global.Article.line.i) + length(Output) > Global.maxLineLength
            then do
                call PutToken Global.EOL;
                i = Global.Article.line.0;
            end;
            Global.Article.line.i = Global.Article.line.i||Output;
        end;
return;

/*
 * PutText:
 *      add a text line to the Global.Article.Line. stem;
 *      if EOLs are present, string is subdivided
 */

PutText:
    procedure expose Global.;
    parse arg Output;  /* text to output */

    if (Global.OptCH) then
        return;

    if (Global.IsTable) & (\Global.IsOutputEnabled)
        then return; /* Skip everything out of :c. ... :c. or :row. tags */

    if (Global.SkipSpaces)
        then Output = strip(strip(Output, 'leading'), 'leading', d2c(9));

    do while (length(Output) > 0)
        EOLpos = pos(Global.EOL, Output);
        if (EOLpos > 0) then
        do  /* EOL in this block of text: */
            if (EOLpos > 1) then
                _text_ = left(Output, EOLpos - 1);
            Output = substr(Output, EOLpos + 1);
            if EOLpos > 1 then
                call PutText _text_;
            call PutToken Global.EOL;
        end;
        else do
            /* no EOL in this block of text: */
            Global.SkipSpaces = 0;
            /* replace tab Characters with needed number of spaces */
            curpos = -1;
            do forever
                tabpos = pos(d2c(9), Output);
                if (tabpos = 0) then
                    /* no tabs found */
                    leave; /* end do */
                /* else tab found: */
                if curpos = -1 /* effective position not yet computed? */
                then do
                    i = Global.Article.line.0;
                    tmpS = Global.Article.line.i;
                    curpos = 0;
                    do while length(tmpS) > 0
                        if pos(left(tmpS, 1), '&:.') > 0
                        then do
                            EOLpos = pos('.', tmpS, 2);
                            if EOLpos = 0 then leave;
                            if left(tmpS, 1) = '&' then
                                tmpS = substr(tmpS, EOLpos);
                            else do; tmpS = substr(tmpS, EOLpos + 1); iterate; end;
                        end;
                        curpos = curpos + 1; tmpS = substr(tmpS, 2);
                    end;
                end;
                Output = left(Output, tabpos - 1)||copies(' ',
                                ,8 - (curpos + tabpos - 1)//8),
                                ||substr(Output, tabpos + 1);
            end;
            /* make text IPF-compatible */
            Output = IPFstring(Output);

            /* subdivide Output string if it is too long */
            i = Global.Article.line.0;
            do while (length(Global.Article.line.i) + length(Output) > Global.maxLineLength)
                EOLpos = Global.maxLineLength - length(Global.Article.line.i);
                j = EOLpos;
                do while (EOLpos > 0)
                    if (c2d(substr(Output, EOLpos, 1)) <= 32) then Leave;
                    EOLpos = EOLpos - 1;
                end;
                if (EOLpos = 0) & (length(Global.Article.line.i) = 0)
                then do
                    /* Line cannot be split on word-bound :-( */
                    EOLpos = j;
                    _text_ = left(Output, EOLpos - 1);
                    Output = substr(Output, EOLpos);
                end
                else do
                    if EOLpos > 1 then
                        _text_ = left(Output, EOLpos - 1)
                    else _text_ = '';
                    Output = substr(Output, EOLpos + 1);
                end;
                Global.Article.line.i = Global.Article.line.i||_text_;

                call PutToken Global.EOL;

                i = Global.Article.line.0;
            end;
            Global.Article.line.i = Global.Article.line.i||Output;

            Output = '';
        end;
    end;

    /* remove leading spaces, but only if we're not in <PRE> mode (UM) */
    if (\Global.IsGraphic) then
            Global.Article.line.i = strip(Global.Article.line.i, "leading");
    Global.AfterBreak = 0;
return;

PutLine:
 parse arg str;
 if Global.OptCH then return;
 call lineout Global.oName, str;
return;

NewLine:
 nli = Global.Article.line.0;
 if length(Global.Article.line.nli) > 0
  then do; call PutToken Global.EOL; return 1; end;
return 0;

/*
 * IPFstring:
 *      this makes a block of regular HTML text
 *      IPF-compatible
 */

IPFstring:
 parse arg ins;
 return ChangeStr(d2c(0), ,
         ChangeStr(':', ,
          ChangeStr('&', ,
           ChangeStr('.', ins, d2c(0)'per.'), ,
          '&amp.'), ,
         '&colon.'), ,
        '&');
/*
 ins = StrReplace('.', d2c(0)'per.', ins);
 ins = StrReplace('&', '&amp.', ins);
 ins = StrReplace(':', '&colon.', ins);
return StrReplace(d2c(0), '&', ins);
*/
ChangeStr:
 procedure expose Global.;
 parse arg src,var,trg;
 curpos = 1;
 do forever
  curpos = pos(src, var, curpos);
  if curpos = 0 then leave;
  var = left(var, curpos - 1)||trg||substr(var, curpos + 1);
  curpos = curpos + length(trg);
 end;
return var;

Shorten:
 parse arg fName;
 fName = translate(stream(fName, 'c', 'query exists'), '/', '\');
 tmp = translate(Directory(), '/', '\');
 if Pos(tmp, fName) = 1
  then return substr(fName, length(tmp) + 2);
 if substr(fName, 2, 1) = ':'
  then return substr(fName, 3);
return fName;

logError:
 procedure expose Global.;
 if Global.optD
  then do
        parse arg line;
        call lineout 'HTML2IPF.log', line;
       end;
return;

CRLF:
 parse value SysTextScreenSize() with maxrow maxcol;
 parse value SysCurPos() with row col;
 call charout ,copies(' ', maxcol-col);
return;

ProgressBar:
 parse value SysCurPos() with row col;
 if col > 79 - 18 then say '';
 Rest = ((Global.FileSize - chars(Global.CurrentFile)) * 16) % Global.FileSize;
 call setcolor lcyan; call charOut ,'[';
 call setcolor white; call charOut ,copies('Û', Rest)copies('±', 16-Rest);
 call setcolor lcyan; call charOut ,']'copies('08'x, 18);
return;

SetColor:
 arg col;
 col = ColorNo(col);
 if \Global.optCO then return;

 if col = -1 then return -1;
 if col > 7
  then col = '1;3'col-8;
  else col = '0;3'col;
 call Charout ,d2c(27)'['col'm';
return 0;

ColorNo:
 arg colname;
 if substr(colname, 1, 1) = 'L'
  then do
        colname = right(colname, length(colname) - 1);
        light = 8;
       end
  else light = 0;
 select
  when abbrev('BLACK', colname, 3)
   then return light + 0;
  when abbrev('BLUE', colname, 3)
   then return light + 4;
  when abbrev('GREEN', colname, 3)
   then return light + 2;
  when abbrev('CYAN', colname, 3)
   then return light + 6;
  when abbrev('RED', colname, 3)
   then return light + 1;
  when abbrev('MAGENTA', colname, 3)
   then return light + 5;
  when abbrev('BROWN', colname, 3)
   then return light + 3;
  when abbrev('GRAY', colname, 3)
   then return light + 7;
  when abbrev('DGRAY', colname, 3)
   then return 8;
  when abbrev('YELLOW', colname, 3)
   then return 11;
  when abbrev('WHITE', colname, 3)
   then return 15;
 end;
 return -1;

/* these constants have been ripped from    */
/* HTM2txt v 1.0, mar.11,1997 by otto r„der */
DefineQuotes:
/* --------------------------------------------- */
/* constants contributed by tremro@digicom.qc.ca */
/* --------------------------------------------- */
 Global.Quotes = ,
  "COPY   (C)",
  "SPACE  0x20",
  "QUOT   0x22",
  "AMP    0x00",
  "LT     <",
  "GT     >",
  "NBSP   0x20",
  "#160   0x20",
  "IEXCL  0xA1",
  "CENT   0xA2",
  "POUND  0xA3",
  "CURREN 0xA4",
  "YEN    0xA5",
  "BRVBAR 0xA6",
  "SECT   0xA7",
  "UML    0xA8",
  "COPY   0xA9",
  "ORDF   0xAA",
  "LAQNO  0xAB",
  "NOT    0xAC",
  "SHY    0xAD",
  "REG    0xAE",
  "HIBAR  0xAF",
  "DEG    0xB0",
  "PLUSMN 0xB1",
  "SUP2   0xB2",
  "SUP3   0xB3",
  "ACUTE  0xB4",
  "MICRO  0xB4",
  "PARA   0xB6",
  "MIDDOT 0xB7",
  "CEDIL  0xB8",
  "SUP1   0xB9",
  "ORDM   0xBA",
  "RAQUO  0xBB",
  "FRAC14 0xBC",
  "FRAC12 0xBD",
  "FRAC34 0xBE",
  "IQUEST 0xBF",
  "AGRAVE 0xC0",
  "AACUTE 0xC1",
  "ACIRC  0xC2",
  "ATILDE 0xC3",
  "AUML   0xC4",
  "ARING  0xC5",
  "AELIG  0xC6",
  "CCEDIL 0xC7",
  "EGRAVE 0xC8",
  "EACUTE 0xC9",
  "ECIRC  0xCA",
  "EUML   0xCB",
  "IGRAVE 0xCC",
  "IACUTE 0xCD",
  "ICIRC  0xCE",
  "IUML   0xCF",
  "ETH    0xD0",
  "NTILDE 0xD1",
  "OGRAVE 0xD2",
  "OACUTE 0xD3",
  "OCIRC  0xD4",
  "OTILDE 0xD5",
  "OUML   0xD6",
  "TIMES  0xD7",
  "OSLASH 0xD8",
  "UGRAVE 0xD9",
  "UACUTE 0xDA",
  "UCIRC  0xDB",
  "UUML   0xDC",
  "YACUTE 0xDD",
  "THORN  0xDE",
  "SZLIG  0xDF",
  "AGRAVE 0xE0",
  "AACUTE 0xE1",
  "ACIRC  0xE2",
  "ATILDE 0xE3",
  "AUML   0xE4",
  "ARING  0xE5",
  "AELIG  0xE6",
  "CCEDIL 0xE7",
  "EGRAVE 0xE8",
  "EACUTE 0xE9",
  "ECIRC  0xEA",
  "EUML   0xEB",
  "IGRAVE 0xEC",
  "IACUTE 0xED",
  "ICIRC  0xEE",
  "IUML   0xEF",
  "ETH    0xF0",
  "NTILDE 0xF1",
  "OGRAVE 0xF2",
  "OACUTE 0xF3",
  "OCIRC  0xF4",
  "OTILDE 0xF5",
  "OUML   0xF6",
  "DIVIDE 0xF7",
  "OSLASH 0xF8",
  "UGRAVE 0xF9",
  "UACUTE 0xFA",
  "UCIRC  0xFB",
  "UUML   0xFC",
  "YACUTE 0xFD",
  "THORN  0xFE",
  "YUML   0xFF";
return;

/* substitute quoted Characters */
NoQuotes:
    parse arg text;

    sPos = 1;
    do forever
        qPos = pos('&', text, sPos);
        if qPos = 0 then
            leave;
        parse var text _head '&' Token ';' _tail

        wordN = wordpos(translate(Token), Global.Quotes);
        if wordN = 0
         then do
               if (left(Token, 1)='#') & (datatype(substr(Token, 2), 'num'))
                then do
                      Token = substr(Token,2);
                      Token = d2c(Token);
                     end
                else do
                      Token=d2c(0)||Token||';'
                     end
              end
         else do
               Token = word(Global.Quotes, wordN + 1);
               if left(Token, 2)="0x" then Token=x2c(substr(Token, 3));
              end
        sPos = length(_head) + length(Token) + 1;
        text = _head||Token||_tail;
    end;
return translate(text, '&', d2c(0));
