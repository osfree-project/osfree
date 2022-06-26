/*
    Create XWorkplace installation objects
    Japanese version (081)
    (C) 1998-2000 Ulrich M罵ler.
 */

options EXMODE

/* added for Japanese */
SysIni('USER', 'XWorkplace', 'SB_NoneSelected', '合計 $C オブジェクト$x(50%)$SA、合計 $zM MB、空き $fM MB')
SysIni('USER', 'XWorkplace', 'SB_WPObject', '$t (クラス: $W)')
SysIni('USER', 'XWorkplace', 'SB_MultiSelected', '$c / $C オブジェクト選択$x(50%)$sA 選択')

/*
    This file gets called by WarpIN after all files have been unpacked.
    This calls CROBJxxx.CMD in turn to have the config folder created.
*/

/* change the following language code to your language. */
LanguageCode = "081";

/* Here come the titles of the objects to be created.
   Translate these to your language. */

XWorkplace          = "XWorkplace";
OS2                 = "OS/2"

/* Title of the Desktop folder; choose a faily long title,
   or the installation might fail if that object already
   exists */
XFolderMain         = XWorkplace||" インストール";
/* "Introduction" object (SmartGuide) */
XFolderIntro        = "概要";
OnlineReference     = XWorkplace||" ユーザーの手引き";
/* other objects */
WorkplaceShell      = "ワークプレース・シェル";
XWPSetup            = XWorkplace||" 設定";
XWPClassList        = "WPS クラス・リスト";
OS2Kernel           = OS2||" カーネル";
Screen              = "画面";
Media               = "マルチメディア";
String              = "セットアップ・ストリング";
Mouse               = "マウス";
Keyboard            = "キーボード";
Sound               = "サウンド";
XShutdown           = "eXtended Shutdown...";
/* font folder added with V0.9.7 */
FontFolder          = "フォント";

/* the following three added with V0.9.19 */
Lockup              = "すぐにロック";
FindObjects         = "オブジェクトの検索";
Shutdown            = "終了";

/*********************************************
 *
 *  NLS-independent portion
 *
 *  Note: All of this was rewritten with V0.9.19,
 *  but the NLS part above is unchanged. Just copy
 *  the entire chunk below from instl001.cmd to your
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
