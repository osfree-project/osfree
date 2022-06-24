/* OD.CMD: this script re-enables the Object Desktop class
   "TSEnhFolder" which might have been deregistered by the
   install script.

   (W) (C) Ulrich M”ller. All rights reserved.
 */

"@echo off"
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

say "This script will re-install the TSEnhFolder class on your system, which"
say "may have been deregistered by the XFolder install script."
say "This only functions properly if Object Desktop is currently installed."
call charout ,"Are you sure you wish to re-install this class? (Y/N) >"
key = ''
do until (pos(key,"YN") > 0)
   key = translate(SysGetKey("NOECHO"))
end;
say key;
say "";
if (key = "Y") then do
    if (SysRegisterObjectClass("TSEnhFolder", "OBJDEFLD") = 1) then do
        say "Registration successful. You will have to restart the WPS or "
        say "reboot to make the changes active. Press any key to exit now."
        "pause >NUL"
    end
    else do
        say "Registration failed. Either you do not have Object Desktop "
        say "installed, or the file OBJDEFLD.DLL is not on your LIBPATH. "
        say "You may try to re-install Object Desktop completely."
        say "Press any key to exit now."
        "pause >NUL"
    end;
end;

