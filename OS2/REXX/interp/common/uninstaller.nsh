;===============================================
; Requires following !defines in the calling *.nsi file
; Examples:
; REGISTRYNAME "TheHesslingEditor"
; LONGNAME    "The Hessling Editor"
;             Can't have slashes in name
; VERSION     "3.3"
;             Version with dots
; MACH        "x64"
;             Machine achitecture for displaying in Control Panel Uninstall
; KEYFILE     "${SHORTNAME}.exe"
;             Name of key installed file in $INSTALLDIR
; DISPLAYICON "$INSTDIR\${SHORTNAME}.exe,0"
;             Installed file Icon
; UNINSTALLER "${SHORTNAME}un.exe"
;             Name of uninstaller executable
;----------------------------------------------
; These are optional...
; SFGROUP     "30846"
;             Group Id on SourceForge
; SFHOME      "http://rexxwrapper.sourceforge.net"
;             SF Home URL
;===============================================
;
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "DisplayName" "${LONGNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "DisplayIcon" "${DISPLAYICON}"
!ifdef SFHOME
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "HelpLink" "${SFHOME}"
!endif
!ifdef SFGROUP
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "URLUpdateInfo" "http://sourceforge.net/project/showfiles.php?group_id=${SFGROUP}"
!endif
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "URLInfoAbout" "http://www.rexx.org/"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "Publisher" "Mark Hessling"
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "UninstallString" '"$INSTDIR\${UNINSTALLER}"'
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "InstallLocation" '"$INSTDIR"'
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "UnInstallLocation" "$INSTDIR" ; dont quote it
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "NoModify" 0x00000001
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRYNAME}" "NoRepair" 0x00000001
  WriteUninstaller "$INSTDIR\${UNINSTALLER}"
