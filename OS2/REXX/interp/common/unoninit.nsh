;===============================================
; Requires following !defines in the calling *.nsi file
; Examples:
; ARCH   "w64"
;        32bit (w32) or 64bit (w64) package architecture
;===============================================
;
;
; set Registry view
!if ${ARCH} == "w64"
SetRegView 64
!else
SetRegView 32
!endif
  ;
  ; UnInstall as All Users if an admin
  ;
  Call un.IsUserAdmin
  Pop $IsAdminUser
  StrCmp $IsAdminUser "false" DefaultUser
  SetShellVarContext all
  Goto DefaultUser
  DefaultUser:
