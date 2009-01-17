;===============================================
; Requires following !defines in the calling *.nsi file
; Examples:
; SFGROUP     "30846"
;             Group Id on SourceForge
; SFHOME      "http://rexxwrapper.sourceforge.net"
;             SF Home URL
; DISPLAYICON "$INSTDIR\${SHORTNAME}.exe,0"
;             Installed file Icon
; UNINSTALLER "${SHORTNAME}un.exe"
;             Name of uninstaller executable
; KEYFILE     "${SHORTNAME}.exe"
;             Name of key installed file in $INSTALLDIR
;===============================================
;
;
; Uninstall previous version if present
;
ReadRegStr $R1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UninstallString"
StrCmp $R1 "" NoUninstall
  ;
  ; ask the user to run the uninstaller
  ;
  ReadRegStr $R2 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UnInstallLocation"
  StrCmp $R2 "" NoUninstallLocation
  Goto StartUninstall
  NoUninstallLocation:
    StrCpy $R2 $INSTDIR
  StartUninstall:
  MessageBox MB_YESNOCANCEL|MB_ICONEXCLAMATION|MB_TOPMOST "A version of ${LONGNAME} is currently installed.$\nIt is recommended that it be uninstalled before proceeding.$\nUninstall previous version?" /SD IDYES IDNO NoUninstall IDCANCEL DoAbort
  HideWindow
  ClearErrors
  ExecWait '$R1 _?=$R2'
  IfErrors no_remove_uninstaller
  IfFileExists "$INSTDIR\${KEYFILE}" no_remove_uninstaller
    RMDir /r "$R2"
  no_remove_uninstaller:
  BringToFront

  Goto NoUninstall
DoAbort:
  Abort
NoUninstall:
;
; Install as All Users if an admin
;
Call IsUserAdmin
Pop $IsAdminUser
StrCmp $IsAdminUser "false" DefaultUser
SetShellVarContext all
DefaultUser:
