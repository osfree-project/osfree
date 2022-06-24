/*
    CrÇation du dossier de configuration eWorkplace par dÇfaut
    English version (001)
    (C) 1998-2002 Ulrich Mîller.
    Traduction (C) 2003-2005 êquipe de traduction franáaise de XWorkplace - Laurent Catinaud, Aymeric Peyret, RenÇ Louisor-Marchini, Guillaume Gay.
*/

/* Ici figurent les titres des objets devant àtre crÇÇs.
   Traduisez-les dans votre langue. Le caractäre "~"
   dÇfinit la lettre devant àtre soulignÇe dans les
   ÇlÇment de menu des objets du dossier de configuration
   eWorkplace. */

XWorkplace          = "eCS"
OS2                 = "eCS"

/* Config Folder title */
ConfigFolder        = "Dossier menu de configuration";
/* here come the objects in the config folder */
CommandLines        = "Invites de ~commandes";
OS2Win              = "~"||OS2||" en fenàtre";
OS2Fullscreen       = OS2||" ~plein Çcran";
DosWin              = "~DOS en fenàtre";
DOSFullscreen       = "DOS p~lein Çcran";
CreateAnother       = "CrÇer ~nouveau";
Folder              = "Dossier";
    /* the following three were added with V0.9.16 */
URLFolder           = "Dossier d'URL";
DataFile            = "Fichier de donnÇes";
ProgramObject       = "Objet Programme";

Treesize            = "Treesize";

/* where to create the config folder: */
TargetLocation      = "<WP_CONFIG>"

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

/* more items in main cfg folder */

class = "WPProgram";
title = "---";
target = "<XWP_CONFIG>";
setup = "";
id = "<XWP_SEP1>";
call CreateObject;

title = Treesize;
setup = "EXENAME="dir"treesize.exe;CCVIEW=YES;";
id = "<XWP_TREESIZE>";
call CreateObject;


exit;


CreateObject:
    len = length(id);
    if (len == 0) then do
        Say 'Erreur sur l''objet "'title'": ID d''objet non fourni.';
        exit;
    end

    if (left(id, 1) \= '<') then do
        Say 'Erreur sur l''objet "'title'": l''ID d''objet ne commence pas par "<".';
        exit;
    end

    if (right(id, 1) \= '>') then do
        Say 'Erreur sur l''objet "'title'": l''ID d''objet ne se termine pas par ">".';
        exit;
    end

    len = length(setup);
    if ((len > 0) & (right(setup, 1) \= ';')) then do
        Say 'Erreur sur l''objet "'title'": la chaåne de paramätres "'setup'" ne se termine pas par un ";".';
        exit;
    end
    call charout , 'CrÇation de "'title'" de classe "'class'", avec les paramätres "'ssetup'"... '
    rc = SysCreateObject(class, title, target, setup"TITLE="title";OBJECTID="id";", "U");
    if (\rc) then do
        rc = SysCreateObject(class, title, "<WP_DESKTOP>", setup"TITLE="title";OBJECTID="id";", "U");
    end;
    if (\rc) then do
        Say 'Avertissement : l''objet "'title'" de classe "'class'" n''a pas pu àtre crÇÇ.'
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
