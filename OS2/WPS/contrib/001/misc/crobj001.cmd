/*
    Create default XWorkplace configuration folder
    English version (001)
    (C) 1998-2007 Ulrich M”ller.
 */

/*
    This file is executed both by INSTLxxx.CMD
    and by XWorkplace directly if the config folder
    is to be (re)created.
*/

/* Here come the titles of the objects to be created.
   Translate these to your language. The "~" character
   determines the character for objects in the XWorkplace
   Configuration Folder which will then be underlined
   in the respective menu items. */

XWorkplace          = "XWorkplace";
OS2                 = "OS/2"

/* Config Folder title */
ConfigFolder        = XWorkplace||" Configuration Folder";
/* here come the objects in the config folder */
CommandLines        = "~Command prompts";
OS2Win              = "~"||OS2||" window";
OS2Fullscreen       = OS2||" ~fullscreen";
DosWin              = "~DOS window";
DOSFullscreen       = "DOS f~ullscreen";
CreateAnother       = "Create ~new";
Folder              = "Folder";
    /* the following three were added with V0.9.16 */
URLFolder           = "URL Folder";
DataFile            = "Data file";
ProgramObject       = "Program object";
QuickSettings       = "~Quick Settings";
DefaultView         = "~Default view for this folder";
IconView            = "Icon view";
TreeView            = "Tree view";
DetailsView         = "Details view";
SmallIcons          = "Small icons for Icon and Tree views";
NormalIcons         = "Normal icons for Icon and Tree views";
ShowAllInTreeView   = "Show all objects in Tree view";
WorkplaceShell      = "Workplace Shell";
XWPSetup            = XWorkplace||" Setup";
OS2Kernel           = OS2||" Kernel";
Screen              = "Screen";
Mouse               = "Mouse";
Keyboard            = "Keyboard";
/* the following one is new with V0.9.19 */
OnlineReference     = XWorkplace||" User Guide";
Treesize            = "Treesize";
PackTree            = "Pack this tree";

/* where to create the config folder: */
TargetLocation      = "<XWP_MAINFLDR>"

/*********************************************
 *
 *  NLS-independent portion
 *
 *  Note: All of this was rewritten with V0.9.19,
 *  but the NLS part above is unchanged. Just copy
 *  the entire chunk below from crobj001.cmd to your
 *  translated file, and it should still work.
 *
 ********************************************/

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

parse source dir;
parse var dir x1 x2 dir;
dir = filespec("D", dir)||filespec("P", dir);
pdir = left(dir, length(dir)-8);
idir = dir;
dir = pdir||"bin\";

/* main config folder */
class = "WPFolder";
title = ConfigFolder;
setup = "ICONVIEW=NONFLOWED,MINI;DEFAULTVIEW=ICON;SHOWALLINTREEVIEW=YES;ALWAYSSORT=NO;";
id = "<XWP_CONFIG>"
target = TargetLocation;
call CreateObject;

/* command prompts subfolder */
class = "WPFolder";
title = CommandLines;
setup = "DEFAULTVIEW=ICON;ICONVIEW=NONFLOWED,MINI;SHOWALLINTREEVIEW=YES;ALWAYSSORT=NO;";
id = "<XWP_CFG1>"
target = "<XWP_CONFIG>";
call CreateObject;

class = "WPProgram";

if (SysSearchPath("PATH", "CMDSHL.CMD") \= "") then do
    title = OS2Win||" (CmdShl)";
    target = "<XWP_CFG1>";
    setup = "EXENAME=cmdshl.cmd;PARAMETERS=%;PROGTYPE=WINDOWABLEVIO;CCVIEW=YES;";
    id = "<XWP_CMDSHL>";
    call CreateObject;
end
if (SysSearchPath("PATH", "BASH.EXE") \= "") then do
    title = OS2Win||" (bash)";
    target = "<XWP_CFG1>";
    setup = "EXENAME=bash.exe;PARAMETERS=%;PROGTYPE=WINDOWABLEVIO;CCVIEW=YES;";
    id = "<XWP_BASH>";
    call CreateObject;
end
if (SysSearchPath("PATH", "KSH.EXE") \= "") then do
    title = OS2Win||" (ksh)";
    target = "<XWP_CFG1>";
    setup = "EXENAME=ksh.exe;PARAMETERS=%;PROGTYPE=WINDOWABLEVIO;CCVIEW=YES;";
    id = "<XWP_KSH>";
    call CreateObject;
end

title = OS2Win;
target = "<XWP_CFG1>";
setup = "EXENAME=*;PARAMETERS=%;PROGTYPE=WINDOWABLEVIO;CCVIEW=YES;";
id = "<XWP_OS2WIN>";
call CreateObject;

title = OS2Fullscreen;
setup = "EXENAME=*;PARAMETERS=%;PROGTYPE=FULLSCREEN;CCVIEW=YES;";
id = "<XWP_OS2FULL>";
call CreateObject;

title = DosWin;
setup = "EXENAME=*;PARAMETERS=%;PROGTYPE=WINDOWEDVDM;CCVIEW=YES;";
id = "<XWP_DOSWIN>";
call CreateObject;

title = DosFullscreen;
setup = "EXENAME=*;PARAMETERS=%;PROGTYPE=VDM;CCVIEW=YES;";
id = "<XWP_DOSFULL>";
call CreateObject;

/* "create new" folder */
class = "WPFolder";
title = CreateAnother;
target = "<XWP_CONFIG>";
setup = "DEFAULTVIEW=ICON;ICONVIEW=NONFLOWED,MINI;SHOWALLINTREEVIEW=YES;ALWAYSSORT=NO;";
id = "<XWP_CFG2>";
call CreateObject;

class = "WPFolder";
title = Folder;
target = "<XWP_CFG2>";
setup = "TEMPLATE=YES;";
id = "<XWP_FOLDERTEMPLATE>";
call CreateObject;

class = "WPUrlFolder";
title = URLFolder;
id = "<XWP_URLFOLDERTEMPLATE>";
call CreateObject;

class = "WPDataFile";
title =  DataFile;
id = "<XWP_DATAFILETEMPLATE>";
call CreateObject;

class = "WPProgram";
title = ProgramObject;
id = "<XWP_PROGRAMTEMPLATE>";
call CreateObject;

/* XWorkplace subfolder */
class = "WPFolder";
title = "~"||XWorkplace;
target = "<XWP_CONFIG>";
setup = "DEFAULTVIEW=ICON;ICONVIEW=NONFLOWED,MINI;SHOWALLINTREEVIEW=YES;ALWAYSSORT=NO;";
id = "<XWP_CFG4>";
call CreateObject;

class = "WPShadow";
title = XWPSetup;
target = "<XWP_CFG4>";
setup = "SHADOWID=<XWORKPLACE_SETUP>;";
id = "<XWORKPLACE_SETUPCFGSHADOW>";
call CreateObject;

title = WorkplaceShell;
setup = "SHADOWID=<XWP_WPS>;";
id = "<XWP_WPSCFGSHADOW>";
call CreateObject;

title = Mouse;
setup = "SHADOWID=<WP_MOUSE>;";
id = "<XWP_WPMOUSECFGSHADOW>";
call CreateObject;
setup = "SHADOWID=<WPSTK_MOUSE>;";
id = "<XWP_WPMOUSECFGSHADOW>";
call CreateObject;

title = Keyboard;
setup = "SHADOWID=<WP_KEYB>;";
id = "<XWP_WPKEYBCFGSHADOW>";
call CreateObject;

title = Screen;
setup = "SHADOWID=<XWP_SCREEN>;";
id = "<XWP_SCREENCFGSHADOW>";
call CreateObject;

title = OS2Kernel;
setup = "SHADOWID=<XWP_KERNEL>;";
id = "<XWP_KERNELCFGSHADOW>";
call CreateObject;

class = "WPProgram";
title = "---";
setup = "";
id = "<XWP_SEP41>";
call CreateObject;

class = "WPShadow";
title = OnlineReference;
setup = "SHADOWID=<XWP_REF>;";
id = "<XWP_REFCFGSHADOW>";
call CreateObject;

class = "WPProgram";
title = "--- ";
setup = "";
id = "<XWP_SEP42>";
call CreateObject;

class = "WPShadow";
title = ConfigFolder;
setup = "SHADOWID=<XWP_CONFIG>;";
id = "<XWP_CONFIGCFGSHADOW>";
call CreateObject;

/* more items in main cfg folder */

class = "WPProgram";
title = "---";
target = "<XWP_CONFIG>";
setup = "";
id = "<XWP_SEP1>";
call CreateObject;

if (SysSearchPath("PATH", "NETSCAPE.EXE") \= "") then do
    title = "Netscape (DDE)";
    setup = "EXENAME="dir"netscdde.exe;";
    id = "<XWP_NETSCAPE>";
    call CreateObject;
end

title = Treesize;
setup = "EXENAME="dir"treesize.exe;CCVIEW=YES;";
id = "<XWP_TREESIZE>";
call CreateObject;

if (SysSearchPath("PATH", "ZIP.EXE") \= "") then do
    title = PackTree;
    setup = "EXENAME="dir"packtree.cmd;CCVIEW=YES;";
    id = "<XWP_PACKTREE>";
    call CreateObject;
end

exit;


CreateObject:
    len = length(id);
    if (len == 0) then do
        Say 'Error with object "'title'": object ID not given.';
        exit;
    end

    if (left(id, 1) \= '<') then do
        Say 'Error with object "'title'": object ID does not start with "<".';
        exit;
    end

    if (right(id, 1) \= '>') then do
        Say 'Error with object "'title'": object ID does not end with ">".';
        exit;
    end

    len = length(setup);
    if ((len > 0) & (right(setup, 1) \= ';')) then do
        Say 'Error with object "'title'": Setup string "'setup'" does not end in semicolon.';
        exit;
    end
    call charout , 'Creating "'title'" of class "'class'", setup "'setup'"... '
    rc = SysCreateObject(class, title, target, setup"TITLE="title";OBJECTID="id";", "U");
    if (\rc) then do
        rc = SysCreateObject(class, title, "<WP_DESKTOP>", setup"TITLE="title";OBJECTID="id";", "U");
    end;
    if (\rc) then do
        Say 'Warning: object "'title'" of class "'class'" could not be created.'
    end
    else do
        Say "OK"
    end

    id = "";

    return;

CreateObjectWithShadow:
    idOld = id;
    call CreateObject;

    class = "WPShadow";
    setup = "SHADOWID="idOld";"
    id = idOfShadow;
    target = "<XWP_MAINFLDR>";

    call CreateObject;

    return;
