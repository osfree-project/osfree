include "const.oph"
include "sysram1.oxh"

APP Regina, &101F401E
  caption "Regina", 3
  mime KDataTypePriorityHigh%, "text/rexx"
  icon "rexx48.mbm"
  icon "mask48.mbm"
  icon "rexx32.mbm"
  icon "mask32.mbm"
  icon "rexx24.mbm"
  icon "mask24.mbm"
  icon "rexx16.mbm"
  icon "mask16.mbm"
ENDA      

include "system.oxh"
include "toolbar.oph"

PROC main:
  global winid%   
  global file$(255)
  global args$(50)
  global rexxstartini$(50)
  global rexxstartdir$(50)
  global rexxexe$(50)
  global trace_chk%
  
  file$="c:\documents\*.rexx"
  trace_chk%=0
  rexxstartdir$="C:\System\Apps\Regina"
  rexxstartini$=rexxstartdir$ +"\Regina.ini"
  rexxexe$="?:\System\Apps\Regina\rexx.exe"
  searchrexx%:
  loadm "z:\system\opl\Toolbar"
  tbarlink:("continue")
endp

proc continue:  
  local a&, choice%
  local ev&(32), key%
  local cmdl$(255)

  if (cmd$(3) = "O")
    file$ = cmd$(2)
    cmdr%:
    cmde%:
  endif
  wininit:
  while (1)
      getevent32 ev&()
      if (ev&( KEvAType%) = KKeyMenu32%) 
        menu:
      elseif (ev&(1) = KKeySidebarMenu32%)
        menu:
      elseif ((ev&(1) and &400) = 0)
        key% = ev&(1) + (%A - 1)
        onerr nix::
        @%("cmd"+chr$(key%)):
        onerr off
      elseif (ev&(KEvAType%) =  KEvPtr&)
        TBarOffer%:(ev&(KEvAPtrOplWindowId%),ev&(KEvAPtrType%),ev&(KEvAPtrPositionX%),ev&(KEvAPtrPositionY%))
      elseif (ev&(KEvAType%) = KEvCommand&)
        cmdl$ = getcmd$
        key% = asc( cmdl$)
        if (key% = %X)
           cmde%:
        elseif (key% = %O)
           file$=right$(cmdl$,len(cmdl$)-1)
           cmdr%:
        endif
      endif
      nix::
    endwh
ENDP

proc cmdtbdownc%:
  cmdc%:
endp

proc cmdc%:
  LOCAL rc%
  
  file$=OPENFILEDIALOG$:(file$,0,0,0)
  if ( file$ = "" )
    file$="c:\documents\*.rexx"
  else  
    dinit "Command Line"
    dcheckbox trace_chk%,"Trace"  
    dedit file$, "Program", 25
    dedit args$, "Arguments", 25
    dbuttons "Ok", KDButtonEnter%, "Cancel", -KDButtonEsc%
    rc%=dialog
    if ( rc% = 0 )
      file$="c:\documents\*.rexx"
    endif
  endif  
endp

proc WinInit:
  local id%, icon%

  gupdate off
  gfont KFontCourierNormal11&
  tbarinit:("Regina", gwidth, gheight)

  Icon%=gCreateBit(0,0) rem create it once and for all
	gCls
  tbarbutt:("r", 1, "Run", 0, icon%, icon%, KTbFlgCmdOnPtrDown%)
  tbarbutt:("c", 2, "Command" +chr$(10) + "line", 0, icon%, icon%, KTbFlgCmdOnPtrDown%)
  tbarbutt:("e", 3, "Exit", 0, icon%, icon%, KTbFlgCmdOnPtrDown%)
  rem tbarbutt:("a", 4, "About", 0, icon%, icon%, KTbFlgCmdOnPtrDown%)
  tbarshow:
  return id%
endp


proc menu:
  local val%
  local comd$(4)
  
  minit
  mcard "File", "Run", %r, "Command Line", %c, "Exit", %e
  mcard "Help",  "About", %a,	"Contents", %h
  
  val% = menu
  if (val% > 0)
    @%("cmd"+chr$(val%)):
  endif
endp

proc cmdtbdowne%:
  cmde%:
endp

proc cmdtbdowna%:
  cmda%:
endp

proc cmde%:
	stop
endp

proc cmda%:
  dinit "RexxStart v3.4"
  dtext "by Mark Hessling",""
  dtext "  http://www.rexx.org/",""
  dtext "based on PerlStart by Olaf Flebbe",""
  dtext "  http://www.science-computing.de/o.flebbe",""
  dbuttons "Continue", KDButtonEnter%
  dialog
endp

proc checkrexx%:( location$)
   rexxexe$ = location$ + right$(rexxexe$, len(rexxexe$)-1) 
   return exist( rexxexe$)
endp

proc searchrexx%:  
  if (checkrexx%:("C"))
    return
  elseif (checkrexx%:("D"))
    return
  else
    alert("Rexx executable not found on C: or D:")
    stop
  endif
endp

proc cmdtbdownd%:
  cmdd%:
endp

proc cmdtbdownr%:
  cmdr%:
endp

proc cmdr%:
   local tid&, statusword&
   local runargs$(50)
   local trace_flag$(3)

   if file$ = "c:\documents\*.rexx"
		  cmdc%:
   endif
   if file$ = "c:\documents\*.rexx"
		  return
   endif
        
   onerr nix::  
   if ( trace_chk% = 0 )
      trace_flag$ = ""
   else
      trace_flag$ = "-t "
   endif      
   if args$ <> ""
      runargs$ = " " + args$
   else
      runargs$ = args$   
   endif
   busy "[" + trace_flag$ + file$ + args$ + "]"
   tid& = RunExeWithCmd&:( rexxexe$, trace_flag$ + file$ + args$)
   busy off
   onerr off
   LOGONTOTHREAD:(tid&,statusWord&)
   IOWAITSTAT32 statusWord&
   setforeground:
   return
   nix::
   busy off
   alert(rexxexe$ + " unable to be run")
endp

proc cmdh%:
   RUNAPP&:("Data","c:\system\apps\regina\regina.hlp","",0)
endp
