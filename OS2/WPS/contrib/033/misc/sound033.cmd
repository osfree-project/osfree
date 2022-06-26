/* Activation des nouveaux sons systämes XFolder
   version franáaise (033)
   (W) (C) Ulrich Mîller. All rights reserved.
   Traduction (C) 2003-2005 êquipe de traduction franáaise de XWorkplace - Laurent Catinaud, Aymeric Peyret, RenÇ Louisor-Marchini, Guillaume Gay.
*/

/* ce sont les titres des Çvänements sonores Ö crÇer  */
/* V0.9.16: retrait des chaånes prÇfixes "XWorkplace" */
ShutdownSound       = "Arràt"
RestartWPSSound     = "Relance du WPS"
ContextOpenSound    = "Ouverture de menu"
ContextSelectSound  = "SÇlection de menu"
CnrSelectSound      = "Double-clic sur dossier"
HotkeyPrsSound      = "Appui sur raccourcis"
            /* added V0.9.3 (2000-04-20) [umoeller] */

/* ne pas modifier ce qui suit */
call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

parse source dir;
parse var dir x1 x2 dir;
dir = filespec("D", dir)||filespec("P", dir);
dir = left(dir, lastpos("\INSTALL", translate(dir)))||"wav\";

MMINI = GetBootDrive()||"\MMOS2\MMPM.INI";
rc = SysINI(MMINI, "MMPM2_AlarmSounds", "555", dir||"autoshut.wav#"||ShutdownSound||"#70");
rc = SysINI(MMINI, "MMPM2_AlarmSounds", "556", dir||"autoshut.wav#"||RestartWPSSound||"#70");
rc = SysINI(MMINI, "MMPM2_AlarmSounds", "558", dir||"ckckk.wav#"||ContextOpenSound||"#100");
rc = SysINI(MMINI, "MMPM2_AlarmSounds", "559", dir||"dudubeep.wav#"||ContextSelectSound||"#100");
rc = SysINI(MMINI, "MMPM2_AlarmSounds", "560", dir||"dudubeep.wav#"||CnrSelectSound||"#100");
rc = SysINI(MMINI, "MMPM2_AlarmSounds", "561", dir||"open.wav#"||HotkeyPrsSound||"#100");
            /* added V0.9.3 (2000-04-20) [umoeller] */

exit;

GetBootDrive: procedure
    parse upper value VALUE( "PATH",, "OS2ENVIRONMENT" ) with "\OS2\SYSTEM" -2 boot_drive +2
return boot_drive

