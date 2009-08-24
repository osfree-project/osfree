  ;
  ; UnInstall as All Users if an admin
  ;
  Call un.IsUserAdmin
  Pop $IsAdminUser
  StrCmp $IsAdminUser "false" DefaultUser
  SetShellVarContext all
  Goto DefaultUser
  DefaultUser:
