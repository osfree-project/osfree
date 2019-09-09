; AutoHotkey Version: 1.x
; Language:       English
; Platform:       Win9x/NT/Wine
; Copyright 2007 Google, Jennifer Lai <jenlai@google.com>
;
; Script Function:
; installing Open Watcom 1.7 to C:\WATCOM

Run, C:\Program Files\open-watcom-c-win32-1.7.exe
WinWait, Open Watcom C/C++, I &Agree
IfWinNotActive, Open Watcom C/C++, I &Agree, WinActivate, Open Watcom C/C++, I &Agree
WinWaitActive, Open Watcom C/C++, I &Agree
Send, {ALTDOWN}a{ALTUP}
WinWait, Open Watcom C/C++, Install Open Watcom
IfWinNotActive, Open Watcom C/C++, Install Open Watcom , WinActivate, Open Watcom C/C++, Install Open Watcom
WinWaitActive, Open Watcom C/C++, Install Open Watcom
Send, {ALTDOWN}n{ALTUP}
Sleep, 1000
Send, {ALTDOWN}fn{ALTUP}
Sleep, 1000
Send, {ALTDOWN}n{ALTUP}
WinWait, Open Watcom C/C++, Setup needs to modif
IfWinNotActive, Open Watcom C/C++, Setup needs to modif, WinActivate, Open Watcom C/C++, Setup needs to modif
WinWaitActive, Open Watcom C/C++, Setup needs to modif
Send, {ALTDOWN}o{ALTUP}
WinWait, Setup, Setup has completed
IfWinNotActive, Setup, Setup has completed , WinActivate, Setup, Setup has completed
WinWaitActive, Setup, Setup has completed
Send, {ALTDOWN}o{ALTUP}

