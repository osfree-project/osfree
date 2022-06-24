/* enable new XFolder system sounds
   English version (001)
   (W) (C) Ulrich M”ller. All rights reserved.
 */

/* these are the titles of the sounds to be created */
/* V0.9.16: removed the leading "XWorkplace" strings */
ShutdownSound       = "Shutdown"
RestartWPSSound     = "Restart WPS"
ContextOpenSound    = "Open menu"
ContextSelectSound  = "Menu selection"
CnrSelectSound      = "Folder double click"
HotkeyPrsSound      = "Hotkey pressed"
            /* added V0.9.3 (2000-04-20) [umoeller] */

/* do not change the following */
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

