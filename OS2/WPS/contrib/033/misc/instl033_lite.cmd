/*
    CrÇation des objets d'installation de eWorkplace
    version franáaise (033)
    (C) 1998-2002 Ulrich Mîller.
    Traduction (C) 2003-2005 êquipe de traduction franáaise de XWorkplace - Laurent Catinaud, Aymeric Peyret, RenÇ Louisor-Marchini, Guillaume Gay.
*/

/* Modifiez le code du pays ci-dessous en fonction de celui correspondant Ö la langue employÇe. */
LanguageCode = "033";

/* Ici figurent les titres des objets devant àtre crÇÇs.
   Traduisez-les dans votre langue. */

XWorkplace          = "eCS";
OS2                 = "eCS"

WorkplaceShell      = "Bureau Çlectronique";
Screen              = "êcran";
String              = "Chaåne de paramätres";
FontFolder          = "Polices";
TrashCan            = "Poubelle";

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

HelpLibrary = "HELPLIBRARY="pdir||"\help\xfldr"LanguageCode".hlp;"

/* "Fonts" folder  */
class = "XWPFontFolder";
title = FontFolder;
setup = "DEFAULTVIEW=DETAILS;DETAILSCLASS=XWPFontObject;SORTCLASS=XWPFontObject;"
id = "<XWP_FONTFOLDER>"
target = "<WP_CONFIG>";
call CreateObject;

/* create "Workplace Shell" */
class = "XFldWPS";
title = WorkplaceShell;
setup = "";
id = "<XWP_WPS>";
target = "<WP_CONFIG>";
call CreateObject;

/* create "Screen" */
class = "XWPScreen";
title = Screen;
setup = "";
id = "<XWP_SCREEN>";
target = "<WP_CONFIG>";
call CreateObject;

/* create "Setup String" template in Templates folder */
class = "XWPString";
title = String;
setup = "TEMPLATE=YES;"
id = "<XWP_STRINGTPL>";
target = "<WP_TEMPS>";
call CreateObject;

/* create trash can on desktop */
class = "XWPTrashCan";
title = TrashCan;
setup = "DEFAULTVIEW=DETAILS;ALWAYSSORT=YES;DETAILSCLASS=XWPTrashObject;SORTCLASS=XWPTrashObject;"
id = "<XWP_TRASHCAN>";
target = "<WP_DESKTOP>";
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

/* create eCenter in eCS "Utilities" folder */
class = "XCenter";
title = "eCenter";
setup = "";
id = "<XWP_XCENTER>"
target = "<WP_TOOLS>";
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
