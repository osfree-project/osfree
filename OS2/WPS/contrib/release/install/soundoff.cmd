/* SOUNDOFF.CMD: this script disables all XFolder sounds in
   case something goes severely wrong
   (W) (C) Ulrich M”ller. All rights reserved.
 */

"@echo off"
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

say "This script will disable all XFolder sounds, while leaving all the "
say "other system sounds intact. Use this script in case you keep getting "
say "errors when XFolder tries to play sounds."
call charout ,"Are you sure you wish to disable XFolder sounds? (Y/N) >"
key = ''
do until (pos(key,"YN") > 0)
   key = translate(SysGetKey("NOECHO"))
end;
say key;
say "";
if (key = "Y") then do
    MMINI = GetBootDrive()||"\MMOS2\MMPM.INI";
    rc = SysINI(MMINI, "MMPM2_AlarmSounds", "555", "DELETE:");
    rc = SysINI(MMINI, "MMPM2_AlarmSounds", "556", "DELETE:");
    rc = SysINI(MMINI, "MMPM2_AlarmSounds", "558", "DELETE:");
    rc = SysINI(MMINI, "MMPM2_AlarmSounds", "559", "DELETE:");
    rc = SysINI(MMINI, "MMPM2_AlarmSounds", "560", "DELETE:");
    rc = SysINI(MMINI, "MMPM2_AlarmSounds", "561", "DELETE:");
end;

exit;

GetBootDrive: procedure
    parse upper value VALUE( "PATH",, "OS2ENVIRONMENT" ) with "\OS2\SYSTEM" -2 boot_drive +2
return boot_drive

