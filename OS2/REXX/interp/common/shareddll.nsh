; AddSharedDLL
;
; Increments a shared DLLs reference count.
; Use by passing one item on the stack (the full path of the DLL).
;
; Usage:
;   Push $SYSDIR\myDll.dll
;   Call AddSharedDLL
;

Function AddSharedDLL
  Exch $R1
  Push $R0
  ReadRegDword $R0 HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1
  IntOp $R0 $R0 + 1
  WriteRegDWORD HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1 $R0
  Pop $R0
  Pop $R1
FunctionEnd

; un.RemoveSharedDLL
;
; Decrements a shared DLLs reference count, and removes if necessary.
; Use by passing one item on the stack (the full path of the DLL).
; Note: for use in the main installer (not the uninstaller), rename the
; function to RemoveSharedDLL.
;
; Usage:
;   Push $SYSDIR\myDll.dll
;   Call un.RemoveSharedDLL
;

Function un.RemoveSharedDLL
  Exch $R1
  Push $R0
  ReadRegDword $R0 HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1
  StrCmp $R0 "" remove
    IntOp $R0 $R0 - 1
    IntCmp $R0 0 rk rk uk
    rk:
      DeleteRegValue HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1
    goto Remove
    uk:
      WriteRegDWORD HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1 $R0
    Goto noremove
  remove:
    Delete /REBOOTOK $R1
  noremove:
  Pop $R0
  Pop $R1
FunctionEnd
