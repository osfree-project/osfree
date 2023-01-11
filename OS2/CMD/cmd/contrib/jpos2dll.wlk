# WLINK control file for JPOS2DLL

name      jpos2dll.dll
system    os2v2 dll initglobal
option    map
option    quiet
option    description 'JPOS2DLL.DLL  Copyright 2002, Rex Conn and JP Software Inc.  All Rights Reserved'
option    manyautodata
segment   type DATA shared
lib       clib3r, os2386

export    QueryExtLIBPATH.1, SetExtLIBPATH.2, SendKeys.3, QuitSendKeys.4
export    StartHook.5, PauseKeys.6, KeystackHookProc.7
