/***********************************/
/* lxLite installation script file */
/***********************************/
 '@echo off'
 'cls'
 signal ON Halt name BreakHandler;
 call rxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 call SysLoadFuncs

 call SetColor Cyan
 say Center("ÍÍÍÍÍÍÍ FRIENDS software presents ÍÍÍÍÍÍÍ", 78)
 call SetColor White
 say Center("·  \//        An  OS/2 executables packer", 78)
 say Center("½Ä½//\  ·     Lots  of  features, maximal", 78)
 call SetColor Gray
 say Center("  //·  ·×ÄÖÄ· compression, nice interface", 78)
 call SetColor DGray
 say Center(" // ½Ä½ÐÓÄÓÄÄ úFúRúEúEú úSúO FúTúWúAúRúEú", 78)
 call SetColor Cyan
 say Center("ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ", 78)
 version = SysOS2Ver()
 parse value version with hiver "." lover
 if (hiver = 2) & (lover > 11)
  then do
        hiver = lover %  10;
        lover = lover // 10;
       end
 call SetColor LBlue
 say Center("OS/2 version "hiver"."lover" detected", 78)

 call SetColor White
 say copies('*', 78)
 call SetColor Yellow
 say "Please note that documentation in HTML format requires a filesystem"
 say "supporting long filenames. If you try to install documentation on a FAT"
 say "drive, you will hear some beeps during installation (XCOPY beeps)."
 say "However, the INF version of documentation will be readable."
 call SetColor Green
 say "Please enter the destination directory for program, ex: C:\OS2\APPS\LXLITE:"
 call SetColor LGreen
 do until (destDir <> '')
  pull destDir
 end;
 if (length(destDir) > 3) & (pos(right(destDir, 1), "\/") > 0)
  then destDir = left(destDir, length(DestDir) - 1);
 do while stream(destDir,"c","query datetime") = ""
  QueryCreate = Ask("The directory you specified does not exist. Create it? (Y/N)", "YN")
  select
   when QueryCreate = "Y"
    then 'mkDir 'destDir' 1>nul 2>nul'
   otherwise
    say "Aborting..."
    exit
  end
 end
 call SetColor Yellow
 say "lxLite utility pack is a set of tiny utilites handy for command-line"
 say "usage. It contains: noEA, a utility for removing extended attributes"
 say "from a file; unLock, a utility that can unlock executable modules"
 say "that are in use (loaded); sysIcons, a utility that allows"
 say "changing/editing of mouse and system pointers and chCase, which"
 say "allow batch case converts of filename/directory names"
 QueryUtility = (Ask("Install lxLite utility pack? (Y/N)", "YN") = "Y");
 call SetColor Yellow
 say "lxLite resources is a set of files that contain all text resources"
 say "of lxLite. It contains all messages and all API functions known"
 say "by name by lxLite. If you want to change lxLite`s messages or to"
 say "add your own APIs, you can use provided sources."
 QueryTextRes = (Ask("Install lxLite text resources? (Y/N)", "YN") = "Y");
 call SetColor Yellow
 say "lxLite documentation is provided in two formats: .HTML and"
 say "in standard OS/2 .INF book format"
 QueryDoc = (Ask("Install lxLite documentation? (Y/N)", "YN") = "Y");
 call SetColor Yellow
 say "Installation program can create a folder with all installed"
 say "stuff onto your OS/2 desktop"
 QueryFolder = (Ask("Create lxLite folder? (Y/N)", "YN") = "Y");

 call SetColor White
 say copies('*', 78)

 call SetColor Yellow
 if (hiver >= 3)                  /* Repack using Lempel-Ziv for OS/2 v>=2.20 */
  then do
        say "Please wait, repacking lxLite using advanced compression method"
        say "Supported by OS/2 2.99 and above (Warp)"
        'lxLite /yur lxLite.exe'
        'del lxLite2x.cfg 1>nul 2>nul'
       end
  else do
        say "The installed version of OS/2 does not support Lempel-Ziv compression"
	'lxLite /x unLock.exe noEA.exe SysIcons.exe chCase.exe'
        'del lxLite.cfg 1>nul 2>nul'
        'ren lxLite2x.cfg lxLite.cfg 1>nul 2>nul'
       end

 call SetColor White
 say copies('*', 78)

 if QueryFolder
  then do
        rc = SysCreateObject('WPFolder', 'lxLite', '<WP_DESKTOP>', ,
             'ICONFILE='directory()'\lxlite_f.ico;OBJECTID=<LXLITE_FOLDER>;'||,
             'ICONVIEWPOS=30,70,40,18;ICONVIEW=FLOWED;'||,
             'BACKGROUND='directory()'\doc\img\texture6.gif,T;', 'R');
        rc = SysSetObjectData('<LXLITE_FOLDER>', 'OPEN=DEFAULT');
       end;

 call CopyFile "lxLite.exe"
 call CopyFile "lxLite.cfg"
 call CopyFile "stub_min.bin"
 call CopyFile "stub_vdm.bin"
 if QueryDoc
  then do
        call CopyDir  "doc"
        call wpsCreate destDir'\doc\lxLite.inf';
       end;
 if QueryUtility
  then do
	call CopyFile "noEA.exe"
	call CopyFile "chCase.exe"
	call CopyFile "unLock.exe"
	call CopyFile "SysIcons.exe"
	call CopyDir  "AndyPtrs"
	call CopyDir  "AndyB&W"
	call CopyDir  "WGloves"
       end;
 if QueryTextRes
  then do
	call CopyDir  "API"
        call wpsCreate destDir'\api';
       end;
 rc = SysSetObjectData('<LXLITE_FOLDER>', 'BACKGROUND='destDir||'\doc\img\texture6.gif,T');

 call SetColor White
 say copies('*', 78)
 call SetColor LRed
 say "-- CAUTION ---- CAUTION ---- CAUTION ---- CAUTION ---- CAUTION ---- CAUTION --"
 call SetColor Yellow
 say Center("If you`re a previous lxLite user please note that in versions above 1.1.5", 78);
 say Center("command-line switches has been changed. I STRONGLY suggest you to browse", 78);
 say Center("through WHATSNEW file (especially the section regarding version 1.1.7),", 78);
 say Center("or at least to run lxLite /? and see what`s changed", 78);

 call SetColor White
 say Center("***", 78);
 call SetColor Yellow
 say Center("Note also a new lxLite feature: The ability to convert NE files into LX", 78);
 say Center("then to compress them. This is a tricky technology that does not work in", 78);
 say Center("all cases: for executables and dynamic-link libraries it works in 99% cases,", 78);
 say Center("but not for the device drivers (.ADD, .FLT, .DMD etc). During testing I`ve", 78);
 say Center("found that some Warp 4.0 drivers does not convert correctly, namely", 78);
 say Center("RESOURCE.SYS, OS2CDROM.DMD, ESS1688.SYS and some others. For this reason", 78);
 say Center("by default NE conversion is disabled. To enable it, please uncomment the", 78);
 say Center("[pdd] configuration section in lxLite.cfg file and add an appropiate /N", 78);
 say Center("switch (I recommend /N+BLR-) to the [default] section.", 78);
 if QueryUtility
  then do
        call SetColor White
        say Center("***", 78);
        call SetColor Yellow
        say Center("Note that in some conditions chCase can damage file names. Please test", 78);
        say Center("first chCase in a separate directory: especially on non-English characters", 78);
        say Center("This can happen because of incorrect case-conversion tables in COUNTRY.SYS", 78);
        call SetColor LRed
       end;
 call SetColor LRed
 say "-- CAUTION ---- CAUTION ---- CAUTION ---- CAUTION ---- CAUTION ---- CAUTION --"
 if Ask("Installation complete. Do you want to read the WHATSNEW file now?", "YN") = "Y"
  then do
        call SetColor Cyan
        'cls'
        'call cmd /c type doc\whatsnew.txt | more'
        Ask(d2c(13)"Press Space to continue", " ")
	'cls'
       end
 if Ask("Do you wish to clean the install source directory? (Y/N)", "YN") = "Y"
  then do
        call SetColor Yellow
        say "Please wait, cleaning up ..."
        'del lxLite??.* 1>nul 2>nul'
        'del noEA*.* 1>nul 2>nul'
        'del chCase*.* 1>nul 2>nul'
        'del unLock*.* 1>nul 2>nul'
        'del SysIcons*.* 1>nul 2>nul'
        'del lxUtil*.* 1>nul 2>nul'
        'del stub_*.* 1>nul 2>nul'
        'del whatsnew 1>nul 2>nul'
        'del file_id.diz 1>nul 2>nul'
        'echo Y | del "doc\img" 1>nul 2>nul'
        'rmdir "doc\img"'
        'echo Y | del "doc" 1>nul 2>nul'
        'rmdir "doc"'
        'echo Y | del "AndyB&W" 1>nul 2>nul'
        'rmdir "AndyB&W"'
        'echo Y | del "AndyPtrs" 1>nul 2>nul'
        'rmdir "AndyPtrs"'
        'echo Y | del "WGloves" 1>nul 2>nul'
        'rmdir "WGloves"'
        'echo Y | del "API" 1>nul 2>nul'
        'rmdir "API"'
        'attrib -r install.cmd 1>nul 2>nul'
        'del install.cmd 1>nul 2>nul'
       end
 'cls'
 call SetColor White
 say Center("Just another fine product from", 78)
 call SetColor Yellow
 say Center("·ÄÄÄÄÄÄÄÄÄÄÄÄÄÄ·ÄÄ·", 78)
 say Center("ºÄ··Ä··ÖÄ··Ä·ÖÄ¶ÖÄÄ", 78)
 say Center("½  ½  ÐÓÄÄ½ ÓÓÄÓÄÄ½", 78)
 call SetColor Red
 say Center("súoúfútúwúaúrúe", 78)
 call SetColor Green
 say Center("To contact me, write to: Andrew Zabolotny, 2:5030/84.5@FIDOnet", 78)
 say Center("                 e-mail: bit@freya.etu.ru                     ", 78)
 call SetColor Gray
exit

BreakHandler:
 call SetColor Red
 say d2c(13)Center("ú-= Installation procedure aborted =-ú", 78)
 call SetColor Gray
exit 1

CopyFile:
 parse arg fName dPath
 call SetColor Cyan
 say "Copying "fName" -> "destDir||dPath"\"fName
 'copy 'fName destDir||dPath' 1>nul 2>nul'
 call wpsCreate destDir||dPath'\'||fName;
return;

wpsCreate:
 parse arg fName
 if \QueryFolder then return;
 select
  when (Pos('.EXE', Translate(fName)) > 0)
   then do
         progType = 'WPProgram';
         progSetup = 'EXENAME='fName';PARAMETERS="%*"';
         progTitle = getTitle(fName);
         progIcon = progTitle||'.ico';
        end;
  when (Pos('.INF', Translate(fName)) > 0)
   then do
         progType = 'WPProgram';
         progSetup = 'EXENAME=VIEW.EXE;PARAMETERS='fName;
         progTitle = getINFtitle(fName);
         progIcon = '';
        end;
  when (Pos('.CFG', Translate(fName)) > 0)
   then do
         progType = 'WPShadow';
         progSetup = 'SHADOWID='fName;
         progTitle = getTitle(fName)||'^configuration file';
         progIcon = '';
        end;
  when (Pos('\API', Translate(fName)) > 0)
   then do
         progType = 'WPShadow';
         progSetup = 'SHADOWID='fName;
         progIcon = '';
        end;
  otherwise return;
 end;

 if length(progIcon) > 0 then progIcon = 'ICONFILE='progIcon;
 rc = SysCreateObject(progType, progTitle, '<LXLITE_FOLDER>', progSetup';'progIcon, 'R');
return;

CopyDir:
 parse arg dName
 call SetColor Cyan
 say "Copying "dName"\ -> "destDir"\"dName"\"
 'xcopy /s /e "'dName'" "'destDir'\'dName'"\ 1>nul 2>nul'
return

Ask:
 parse arg Question,Reply;
 call SetColor Green
 rc = charOut(, Question)
 call SetColor LGreen
 do until Pos(Answer, Reply) \= 0
  KeyIn = SysGetKey("noecho")
  parse upper var KeyIn Answer
 end
 say Answer;
return Answer;

lowStr: procedure
 arg S;
return translate(S, xRange('a','z'), xRange('A','Z'));

getTitle: procedure
 arg fName
 __S = fileSpec('name', fName);
 if pos('.', __S) > 0
  then __S = subStr(__S, 1, pos('.', __S)-1);
return substr(__S, 1, 1)||lowStr(substr(__S, 2))

getINFtitle: procedure;
 arg fName;
 Title = strip(charIn(fName, 108, 48), 'Trailing', d2c(0));
 if Title = '' then Title = getTitle(fName);
return Title;

SetColor:
 procedure expose break;
 arg Col;
 Col = ColorNo(Col);

 if Col = -1 then return -1;
 if Col > 7
  then Col = '1;3'Col-8;
  else Col = '0;3'Col;
 call charOut ,d2c(27)'['Col'm';
return 0;
 
ColorNo:
 procedure expose break;
 arg ColName;
 if Substr(ColName, 1, 1) = 'L'
  then do
        ColName = right(ColName, length(ColName) - 1);
        Light = 8;
       end
  else Light = 0;
 select
  when Abbrev('BLACK', ColName, 3)
   then return Light + 0;
  when Abbrev('BLUE', ColName, 3)
   then return Light + 4;
  when Abbrev('GREEN', ColName, 3)
   then return Light + 2;
  when Abbrev('CYAN', ColName, 3)
   then return Light + 6;
  when Abbrev('RED', ColName, 3)
   then return Light + 1;
  when Abbrev('MAGENTA', ColName, 3)
   then return Light + 5;
  when Abbrev('BROWN', ColName, 3)
   then return Light + 3;
  when Abbrev('GRAY', ColName, 3)
   then return Light + 7;
  when Abbrev('DGRAY', ColName, 3)
   then return 8;
  when Abbrev('YELLOW', ColName, 3)
   then return 11;
  when Abbrev('WHITE', ColName, 3)
   then return 15;
 end;
return -1;
