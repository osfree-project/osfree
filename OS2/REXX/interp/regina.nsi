;
; Regina Install Script, based on Modern Example Script Written by Joost Verburg
; Requires: ${NSISDIR}\Contrib\Path\path.nsi
; Run as:
;  makensis /DVERSION=x.x /DNODOTVER=xx regina.nsi
; Note:
;  regina.nsi MUST be in the current directory!

!define LONGNAME "Regina Rexx"  ;Long Name (for descriptions)
!define SHORTNAME "Regina" ;Short name (no slash) of package
!define SFHOME "http://regina-rexx.sourceforge.net"
!define SFGROUP "28102"
!define UNINSTALLER "uninstall.exe"
!define DISPLAYICON "$INSTDIR\regina.exe,0"
!define KEYFILE "regina.exe"

!define MUI_ICON "${SRCDIR}\reginaw32.ico"
!define MUI_UNICON "uninstall.ico"

Name "${LONGNAME} ${VERSION}"

!include "MUI.nsh"

!define MUI_CUSTOMPAGECOMMANDS

!define MUI_LICENSEPAGE
!define MUI_COMPONENTSPAGE
!define MUI_DIRECTORYPAGE
!define MUI_FINISHPAGE
!define MUI_FINISHPAGE_NOAUTOCLOSE

!define MUI_ABORTWARNING

!define MUI_UNINSTALLER
!define MUI_UNCONFIRMPAGE

!addplugindir ${SRCDIR}\nsis

; VERSION  ;Must be supplied on compile command line
; NODOTVER ;Must be supplied on compile command line

;--------------------------------
;Configuration

  ;General
  OutFile "${SHORTNAME}${NODOTVER}${ARCH}.exe"
  ShowInstdetails show
  SetOverwrite on


  ;License dialog
  LicenseData "${SRCDIR}\COPYING-LIB"


  ;Folder-select dialog
  InstallDir "c:\${SHORTNAME}"

  LangString TEXT_IO_PAGETITLE_ASSOC ${LANG_ENGLISH} "File Associations"
  LangString TEXT_IO_SUBTITLE_ASSOC ${LANG_ENGLISH} "Associate file extensions with Regina"
  LangString TEXT_IO_PAGETITLE_LANGUAGE ${LANG_ENGLISH} "Language for Error Messages"
  LangString TEXT_IO_SUBTITLE_LANGUAGE ${LANG_ENGLISH} "Select the language for Regina error messages"
  LangString TEXT_IO_PAGETITLE_STACK ${LANG_ENGLISH} "Regina Stack Service"
  LangString TEXT_IO_SUBTITLE_STACK ${LANG_ENGLISH} "Install Regina Stack Service"
;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "${SRCDIR}\COPYING-LIB"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  Page custom SetCustomAssoc
  Page custom SetCustomLanguage
  Page custom SetCustomStack
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

;--------------------------------
;Language
!insertmacro MUI_LANGUAGE "English"

;================
;Variables
  Var IsAdminUser

;--------------------------------
;Reserved files

  ReserveFile "regina_fa.ini"
  ReserveFile "regina_ss.ini"
  ReserveFile "regina_mt.ini"
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;========================================================================
;Installer Sections

;------------------------------------------------------------------------
; Core

Section "${LONGNAME} Core (required)" SecMain
  SectionIn 1 RO
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Distribution files...
  File rexx.exe
  File regina.exe
  File regina.dll
  File rxstack.exe
  File rxqueue.exe
  File de.mtb
  File en.mtb
  File es.mtb
  File no.mtb
  File pt.mtb
  File pl.mtb
  File tr.mtb
  File /oname=README${NODOTVER}.txt ${SRCDIR}\README.${NODOTVER}
  File /oname=READMEW32.txt ${SRCDIR}\README.W32
  File /oname=LICENSE.txt ${SRCDIR}\COPYING-LIB
  Push $INSTDIR
  Push $IsAdminUser ; "true" or "false" per admin user
  Push "PATH"
  Call AddToPath
  CreateDirectory "$SMPROGRAMS\${LONGNAME}"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Uninstall ${SHORTNAME}.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Release Notes.lnk" "$INSTDIR\README${NODOTVER}.txt" "" "$INSTDIR\README${NODOTVER}.txt" 0
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\README.lnk" "$INSTDIR\READMEW32.txt" "" "$INSTDIR\READMEW32.txt" 0
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\LICENSE.lnk" "$INSTDIR\LICENSE.txt" "" "$INSTDIR\LICENSE.txt" 0
  ; Can't use CreateShortcut for URLs
  WriteINIStr "$SMPROGRAMS\${LONGNAME}\Regina Rexx Home Page.url" "InternetShortcut" "URL" "http://regina-rexx.sourceforge.net"
  ; Write the installation path into the registry
  ; Write the uninstall keys
!include "${SRCDIR}\common\uninstaller.nsh"
; WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "DisplayName" "${LONGNAME}"
; WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "DisplayIcon" "${DISPLAYICON}"
; WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "HelpLink" "http://www.rexxla.org/support.html"
; WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "URLUpdateInfo" "http://sourceforge.net/project/showfiles.php?group_id=28102"
; WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "URLInfoAbout" "http://www.rexx.org/"
; WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "DisplayVersion" "${VERSION}"
; WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "Publisher" "Mark Hessling"
; WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UninstallString" '"$INSTDIR\${UNINSTALLER}"'
; WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "InstallLocation" '"$INSTDIR"'
; WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UnInstallLocation" "$INSTDIR" ; dont quote it
; WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "NoModify" 0x00000001
; WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "NoRepair" 0x00000001
; WriteUninstaller "$INSTDIR\${UNINSTALLER}"
  ; Stack Service
  ReadIniStr $R0 "$PLUGINSDIR\regina_ss.ini" "Field 4" State
  ReadIniStr $R1 "$PLUGINSDIR\regina_ss.ini" "Field 5" State
  Push $R0
  Push $R1
  Call InstallRxstack
  StrCpy $5 0
  ReadIniStr $R0 "$PLUGINSDIR\regina_fa.ini" "Field 2" State
  ReadIniStr $R1 "$PLUGINSDIR\regina_fa.ini" "Field 2" Text
  Push $R0
  Push $R1
  Call DoFileAssociation
  ReadIniStr $R0 "$PLUGINSDIR\regina_fa.ini" "Field 3" State
  ReadIniStr $R1 "$PLUGINSDIR\regina_fa.ini" "Field 3" Text
  Push $R0
  Push $R1
  Call DoFileAssociation
  ReadIniStr $R0 "$PLUGINSDIR\regina_fa.ini" "Field 4" State
  ReadIniStr $R1 "$PLUGINSDIR\regina_fa.ini" "Field 4" Text
  Push $R0
  Push $R1
  Call DoFileAssociation
  ReadIniStr $R0 "$PLUGINSDIR\regina_fa.ini" "Field 5" State
  ReadIniStr $R1 "$PLUGINSDIR\regina_fa.ini" "Field 5" Text
  Push $R0
  Push $R1
  Call DoFileAssociation
  ReadIniStr $R0 "$PLUGINSDIR\regina_fa.ini" "Field 6" State
  ReadIniStr $R1 "$PLUGINSDIR\regina_fa.ini" "Field 6" Text
  Push $R0
  Push $R1
  Call DoFileAssociation
  ReadIniStr $R0 "$PLUGINSDIR\regina_fa.ini" "Field 7" State
  ReadIniStr $R1 "$PLUGINSDIR\regina_fa.ini" "Field 7" Text
  Push $R0
  Push $R1
  Call DoFileAssociation
  ReadIniStr $R0 "$PLUGINSDIR\regina_fa.ini" "Field 8" State
  ReadIniStr $R1 "$PLUGINSDIR\regina_fa.ini" "Field 8" Text
  Push $R0
  Push $R1
  Call DoFileAssociation
  Call DoFileAssociationDetails
  Call DoLanguageDefault
SectionEnd

;------------------------------------------------------------------------
; Demos

Section "${LONGNAME} Demos" SecDemo
  CreateDirectory "$SMPROGRAMS\${LONGNAME}\${LONGNAME} Demos"
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR\demo
  File rxtest1.dll
  File rxtest2.dll
  ; Distribution files...
  File /oname=rexxcps.rexx ${SRCDIR}\demo\rexxcps.rexx
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${LONGNAME} Demos\Rexxcps.lnk" "$INSTDIR\regina.exe" '-p "$INSTDIR\demo\rexxcps.rexx"' "$INSTDIR\regina.exe"
  File /oname=dynfunc.rexx ${SRCDIR}\demo\dynfunc.rexx
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${LONGNAME} Demos\Dynfunc.lnk" "$INSTDIR\regina.exe" '-p "$INSTDIR\demo\dynfunc.rexx"' "$INSTDIR\regina.exe"
  File ${SRCDIR}\demo\*.rexx
SectionEnd

;------------------------------------------------------------------------
; Development tools

Section "${LONGNAME} Development Kit" SecDev
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR\lib
  ; Distribution files...
  File regina.lib
  File rexx.lib
  SetOutPath $INSTDIR\include
  File /oname=rexxsaa.h ${SRCDIR}\rexxsaa.h
SectionEnd

;------------------------------------------------------------------------
; Doco

Section "${LONGNAME} Documentation" SecDoc
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR\doc
  File ${DOCDIR}\regina${NODOTVER}.pdf
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${LONGNAME} PDF Documentation.lnk" "$INSTDIR\doc\regina${NODOTVER}.pdf" "" "$INSTDIR\doc\regina${NODOTVER}.pdf" 0
SectionEnd

Section ""

  ;Invisible section to display the Finish header
; !insertmacro MUI_FINISHHEADER

SectionEnd

;========================================================================
;Installer Functions

Function .onInit
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "regina_fa.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "regina_ss.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "regina_mt.ini"
  ;
  ; Uninstall previous version if present
  ;
!include "${SRCDIR}\common\oninit.nsh"
FunctionEnd

Function SetCustomAssoc
  !insertmacro MUI_HEADER_TEXT "$(TEXT_IO_PAGETITLE_ASSOC)" "$(TEXT_IO_SUBTITLE_ASSOC)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "regina_fa.ini"
FunctionEnd

Function SetCustomLanguage
  !insertmacro MUI_HEADER_TEXT "$(TEXT_IO_PAGETITLE_LANGUAGE)" "$(TEXT_IO_SUBTITLE_LANGUAGE)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "regina_mt.ini"
FunctionEnd

Function SetCustomStack
  !insertmacro MUI_HEADER_TEXT "$(TEXT_IO_PAGETITLE_STACK)" "$(TEXT_IO_SUBTITLE_STACK)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "regina_ss.ini"
FunctionEnd

Function .onMouseOverSection

  !insertmacro MUI_DESCRIPTION_BEGIN

    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "Installs the core components of ${LONGNAME} to the application folder."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDev} "Installs the files required to embed ${LONGNAME} into you C/C++ application."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDemo} "Install sample ${LONGNAME} programs."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDoc} "Install ${LONGNAME} documentation."

 !insertmacro MUI_DESCRIPTION_END

FunctionEnd

Function DoFileAssociation
  Pop $R1
  Pop $R0
  Strcmp $R0 0 exitfa
  Strcmp $R1 "" exitfa
  ; do the association
  DetailPrint "Registering $R1 extension to run with Regina"
  IntOp $5 $5 + 1
  WriteRegStr HKCR $R1 "" "Regina"
  exitfa:
  Return
FunctionEnd

Function DoFileAssociationDetails
  StrCmp $5 0 exitfad
  ; do the association details
  WriteRegStr HKCR "Regina" "" "Regina Rexx Program"
  WriteRegStr HKCR "Regina\shell" "" "open"
  WriteRegStr HKCR "Regina\DefaultIcon" "" "$INSTDIR\regina.exe,0"
  WriteRegStr HKCR "Regina\shell\open" "" "Run"
  WriteRegStr HKCR "Regina\shell\open\command" "" '"$INSTDIR\regina.exe" -p "%1" %*'
  WriteRegStr HKCR "Regina\shell\edit" "" "Edit"
  WriteRegStr HKCR "Regina\shell\edit\command" "" 'notepad.exe "%1"'
  exitfad:
  Return
FunctionEnd

Function DoLanguageDefault
  StrCpy $1 2
  langStartLoop:
    StrCpy $2 "Field $1"
    ReadIniStr $R0 "$PLUGINSDIR\regina_mt.ini" $2 State
    StrCmp $R0 1 langFound
    IntOp $1 $1 + 1
    StrCmp $1 9 langExit ; the "9" here must be 1 more than the last field number in regina_mt.ini
    Goto langStartLoop
  langFound:
  ReadIniStr $R0 "$PLUGINSDIR\regina_mt.ini" $2 Text
  ; do the language default
  Push "REGINA_LANG_DIR"
  Push "$INSTDIR"
  Push $IsAdminUser ; "true" or "false"
  Call WriteEnvStr
  Push "REGINA_LANG"
  Push "$R0"
  Push $IsAdminUser ; "true" or "false"
  Call WriteEnvStr
  DetailPrint "$R0 set as default language for error messages"
  langExit:
  Return
FunctionEnd

Function InstallRxstack
  Pop $R1
  Pop $R0
  Strcmp $R0 0 exitss
  ; do the install of rxstack
  DetailPrint "Installing Regina Stack Service"
  nsExec::ExecToLog "$INSTDIR\rxstack -install"
  Strcmp $R1 0 exitss
  DetailPrint "Starting Regina Stack Service"
  Services::SendServiceCommand 'start' 'Regina Stack'
  Pop $R0
  StrCmp $R0 'Ok' exitss
     MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST "Failed to start the Regina Stack service:$\n$0" /SD IDOK
  exitss:
FunctionEnd

;========================================================================
;Uninstaller Section

Section "Uninstall"

  ; get rid of file associations
  Push ".rexx"
  Call un.DeleteFileAssociation
  Push ".rex"
  Call un.DeleteFileAssociation
  Push ".rx"
  Call un.DeleteFileAssociation
  Push ".cmd"
  Call un.DeleteFileAssociation
  Push $INSTDIR
  Push $IsAdminUser ; pushes "true" or "false"
  Push "PATH"
  Call un.RemoveFromPath
  DeleteRegKey HKCR "Regina"
  ; remove LANG environment variables
  Push "REGINA_LANG_DIR"
  Push $IsAdminUser ; pushes "true" or "false"
  Call un.DeleteEnvStr
  Push "REGINA_LANG"
  Push $IsAdminUser ; pushes "true" or "false"
  Call un.DeleteEnvStr
  ; stop and remove the rxstack service - ignore if we get errors
  Services::SendServiceCommand 'stop' 'Regina Stack'
  Exec "$INSTDIR\rxstack -remove"

  RMDir /r "$INSTDIR"

  ; Remove the installation stuff
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}"

  ; remove shortcuts directory and everything in it
  RMDir /r "$SMPROGRAMS\${LONGNAME}"

; !insertmacro MUI_UNFINISHHEADER

SectionEnd

;========================================================================
;Uninstaller Functions

Function un.onInit
  ;
  ; UnInstall as All Users if an admin
  ;
!include "${SRCDIR}\common\unoninit.nsh"
FunctionEnd

Function un.DeleteFileAssociation
  Pop $R0
  ReadRegStr $R1 HKCR "$R0" ""
  StrCmp $R1 "Regina" 0 NoOwn ; only delete key if we own it
  DeleteRegKey HKCR "$R0"
  DetailPrint "Deleting file association for $R0"
  NoOwn:
FunctionEnd

!include "${SRCDIR}\common\admin.nsh"
!include "${SRCDIR}\common\isnt.nsh"
!include "${SRCDIR}\common\path.nsh"
!include "${SRCDIR}\common\WriteEnv.nsh"

;eof
