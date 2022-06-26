/*
    CrÇation des objets d'installation de XWorkplace 
    version franáaise (033)
    (C) 1998-2000 Ulrich Mîller.
    Traduction (C) 2003-2005 êquipe de traduction franáaise de XWorkplace - Laurent Catinaud, Aymeric Peyret, RenÇ Louisor-Marchini, Guillaume Gay.
*/

/*
    Ce fichier est invoquÇ par WarpIN apräs que tous les fichier ont ÇtÇ dÇballÇes. 
    CROBJxxx.CMD est appelÇ Ö son tour pour que le dossier de configuration soit crÇÇ. 
*/

/* Modifiez le code du pays ci-dessous en fonction de celui correspondant Ö la langue employÇe. */
LanguageCode = "033";

/* Ici figurent les titres des objets devant àtre crÇÇs.
   Traduisez-les dans votre langue. */

XWorkplace          = "XWorkplace";
OS2                 = "OS/2"

/* Title of the Desktop folder; choose a faily long title,
   or the installation might fail if that object already
   exists */
XFolderMain         = "Installation "||XWorkplace;
/* "Introduction" object (SmartGuide) */
XFolderIntro        = "Introduction";
OnlineReference     = "Guide d'utilisation de "||XWorkplace;
/* other objects */
WorkplaceShell      = "Bureau Çlectronique";
XWPSetup            = "Paramätrage de "||XWorkplace;
XWPClassList        = "Liste des classes WPS";
OS2Kernel           = "Noyau d'"||OS2;
Screen              = "êcran";
Media               = "Multimedia";
String              = "Chaåne de paramätres";
Mouse               = "Souris";
Keyboard            = "Clavier";
Sound               = "Son";
XShutdown           = "Arràt avancÇ...";
/* font folder added with V0.9.7 */
FontFolder          = "Polices";

/* the following three added with V0.9.19 */
Lockup              = "Verrouillage immÇdiat";
FindObjects         = "Recherche d'objets";
Shutdown            = "Arràt";

/*********************************************
 *
 *  Portion indÇpendante de la langue
 *
 *  Remarque : Tout ceci a ÇtÇ rÇÇcrit dans 
 *  la version v0.9.19, mais la partie Ö
 *  traduire ci-dessus n'a pas ÇtÇ modifiÇe. 
 *  Vous n'avez donc qu'Ö copier la partie 
 *  ci-dessous, la coller sous la partie 
 *  prise dans instl001.cmd et placer le 
 *  tout dans votre fichier traduit, et áa 
 *  devrait toujours fonctionner. 
 *
 *  Note du traducteu : plus bas, il y a 
 *  quand màme les rapports de crÇation et
 *  d'erreur de crÇation d'objets que je me
 *  suis permis de traduire. 
 *
 *                   Guillaume Gay 2005-11-01
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

OnlineReferenceFile = "xfldr"LanguageCode".inf";

HelpLibrary = "HELPLIBRARY="pdir||"\help\xfldr"LanguageCode".hlp;"

/* main folder */
class = "WPFolder";
title = XFolderMain;
setup = "DEFAULTVIEW=ICON;SHOWALLINTREEVIEW=YES;ALWAYSSORT=NO;ICONFILE="pdir||"\install\xwp.ico;ICONNFILE=1,"pdir||"\install\xwp_o.ico;"HelpLibrary"HELPPANEL=84;"
id = "<XWP_MAINFLDR>"
target = "<WP_DESKTOP>";
call CreateObject;

/* User Guide */
class = "WPProgram";
title = OnlineReference;
setup = "EXENAME=view.exe;PARAMETERS="pdir||OnlineReferenceFile" %;"
/* added % to parameters list in order not to break the config folder shadow
    V0.9.20 (2002-07-03) [umoeller] */
id = "<XWP_REF>"
target = "<XWP_MAINFLDR>";
call CreateObject;

/* "Readme" shadow (added V0.9.2) */
class = "WPShadow";
title = "readme";
setup = "SHADOWID="pdir||"README;"
id = "<XWP_READMEMAINSHADOW>"
target = "<XWP_MAINFLDR>";
call CreateObject;

/* create "XWorkplace Setup" (added V0.9.0) */
class = "XWPSetup";
title = XWPSetup;
setup = ""
id = "<XWORKPLACE_SETUP>"
target = "<WP_CONFIG>";
idOfShadow = "<XWP_SETUPMAINSHADOW>";
call CreateObjectWithShadow;

/* "Fonts folder" ... added with V0.9.7 */
class = "XWPFontFolder";
title = FontFolder;
setup = "DEFAULTVIEW=DETAILS;DETAILSCLASS=XWPFontObject;SORTCLASS=XWPFontObject;";
id = "<XWP_FONTFOLDER>"
target = "<WP_CONFIG>";
idOfShadow = "<XWP_FONTFOLDERSHADOW>";  /* was missing V0.9.20 */
call CreateObjectWithShadow;

/* create "Workplace Shell" */
class = "XFldWPS";
title = WorkplaceShell;
setup = "";
id = "<XWP_WPS>";
target = "<WP_CONFIG>";
idOfShadow = "<XWP_WPSMAINSHADOW>";
call CreateObjectWithShadow;

/* create "OS/2 Kernel" */
class = "XFldSystem";
title = OS2Kernel;
setup = "";
id = "<XWP_KERNEL>"
target = "<WP_CONFIG>";
idOfShadow = "<XWP_KERNELMAINSHADOW>";
call CreateObjectWithShadow;

/* create "Screen" (added V0.9.2) */
class = "XWPScreen";
title = Screen;
setup = "";
id = "<XWP_SCREEN>"
target = "<WP_CONFIG>";
idOfShadow = "<XWP_SCREENMAINSHADOW>";
call CreateObjectWithShadow;

/* create "Media" (added V0.9.5) */
class = "XWPMedia";
title = Media;
setup = "";
id = "<XWP_MEDIA>"
target = "<WP_CONFIG>";
idOfShadow = "<XWP_MEDIAMAINSHADOW>";
call CreateObjectWithShadow;

/* "Mouse" shadow (added V0.9.2) */
class = "WPShadow";
title = Mouse;
setup = "SHADOWID=<WP_MOUSE>;";
id = "<XWP_MOUSEMAINSHADOW>"
target = "<XWP_MAINFLDR>";
call CreateObject;
setup = "SHADOWID=<WPSTK_MOUSE>;";
id = "<XWP_MOUSEMAINSHADOW>"
call CreateObject;

/* "Keyboard" shadow (added V0.9.2) */
class = "WPShadow";
title = Keyboard;
setup = "SHADOWID=<WP_KEYB>;";
id = "<XWP_KEYBMAINSHADOW>";
target = "<XWP_MAINFLDR>";
call CreateObject;

/* "Sound" shadow (added V0.9.2) */
class = "WPShadow";
title = Sound;
setup = "SHADOWID=<WP_SOUND>;";
id = "<XWP_SOUNDMAINSHADOW>"
target = "<XWP_MAINFLDR>";
call CreateObject;

/* create "WPS Class List" (added V0.9.2) */
class = "XWPClassList";
title = XWPClassList;
setup = "";
id = "<XWP_CLASSLIST>"
target = "<WP_CONFIG>";
idOfShadow = "<XWP_CLASSLISTMAINSHADOW>";
call CreateObjectWithShadow;

/* create "Setup String" template (added V0.9.5) */
class = "XWPString";
title = String;
setup = "TEMPLATE=YES;";
id = "<XWP_STRINGTPL>"
target = "<XWP_MAINFLDR>";
call CreateObject;

/* XShutdown... changed with V0.9.7 */
class = "WPProgram";
title = XShutdown;
setup = "EXENAME="dir"xshutdwn.cmd;MINIMIZED=YES;"HelpLibrary"HELPPANEL=17;";
id = "<XWP_XSHUTDOWN>"
target = "<XWP_MAINFLDR>";
call CreateObject;

/* the following three added with V0.9.19
   and redone with V0.9.20 */
commonXWPString = "DEFAULTOBJECT=<WP_DESKTOP>;CONFIRMINVOCATION=NO;NOSTRINGPAGE=YES;HELPLIBRARY=WPHELP.HLP;HELPPANEL"

/* create "Lockup" setup string object */
class = "XWPString";
title = Lockup;
setup = "SETUPSTRING=MENUITEMSELECTED%3D705%3B;"commonXWPString"=8004;ICONRESOURCE=78,PMWP;"
id = "<XWP_LOCKUPSTR>"
target = "<WP_NOWHERE>";
call CreateObject;

/* create "Find objects" setup string object */
class = "XWPString";
title = FindObjects;
setup = "SETUPSTRING=MENUITEMSELECTED%3D8%3B;"commonXWPString"=1205;ICONRESOURCE=79,PMWP;";
id = "<XWP_FINDSTR>";
target = "<WP_NOWHERE>";
call CreateObject;

/* create "Shutdown" setup string object */
/* setup string modified to use POSTSHUTDOWN=YES V0.9.20 */
class = "XWPString";
title = Shutdown;
setup = "SETUPSTRING=POSTSHUTDOWN%3DYES%3B;"commonXWPString"=4001;ICONRESOURCE=80,PMWP;"
id = "<XWP_SHUTDOWNSTR>";
target = "<WP_NOWHERE>";
call CreateObject;

/* create XCenter (V0.9.7)
   moved this down V0.9.19 (2002-04-25) [umoeller]
   */
class = "XCenter";
title = "XCenter";
setup = "";
id = "<XWP_XCENTER>"
target = "<XWP_MAINFLDR>";
call CreateObject;

"@call "idir"crobj"LanguageCode

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
    call charout , 'CrÇation de "'title'" de classe "'class'", avec les paramätres "'setup'"... '
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
