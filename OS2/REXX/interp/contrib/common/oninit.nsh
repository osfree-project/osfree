;===============================================
; Requires following !defines in the calling *.nsi file
; Examples:
; REGISTRYNAME   "TheHesslingEditor"
; LONGNAME       "The Hessling Editor"
; SHORTNAME      "THE"
; KEYFILE     "the.exe"
;             Name of key installed file in $INSTALLDIR
; ARCH        "w64"
;             32bit (w32) or 64bit (w64) package architecture
;===============================================
;
;
; set Registry view
!if ${ARCH} == "w64"
SetRegView 64
!else
SetRegView 32
!endif

UninstallDefault:
;
; Uninstall previous version if present
;
ReadRegStr $R1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "UninstallString"
StrCmp $R1 "" NoUninstall
  ;
  ; ask the user to run the uninstaller
  ;
  ReadRegStr $R2 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "UnInstallLocation"
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
