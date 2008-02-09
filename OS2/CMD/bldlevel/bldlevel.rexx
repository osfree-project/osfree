/* $Id: bldlevel.cmd,v 1.2 2004/08/21 14:48:44 prokushev Exp $ */

Say 'Build Level Display Facility Version 1.1'
Say '(C) Copyright 2003-2004 by Yuri Prokushev'

call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
call SysLoadFuncs

if (arg(1)='')|(pos('/?', arg(1))=1) then
do
  say SysGetMessage(8067) /* Help message */
  return
end

BuildDate=''
HostName=''
Package=''
ASDFeatureID=''
LanguageCode=''
CountryCode=''
FixPackVer=''
Build=''

filename=arg(1)
rc=stream(filename, 'c', 'QUERY EXIST');
if rc='' then
do
  Say SysGetMessage(1533, '', filename)
  return
end

f=charin(filename,1, chars(filename))

as:
interpret "parse var f with Skip '@#' Vendor ':' Revision '#@' Description '"||d2c(0)||"' Skip"
if length(Vendor)>31 then
do
  f=Vendor||':'||Revision||'#'||'@'||Description||d2c(0)||Skip
  signal as
end

Signature='@'||'#'Vendor||':'||Revision||'#'||'@'||Description

/* This modification to original is to allow use not only asciiz strings */
Parse var Description Description '#@'

/* Following is extensions to original bldlevel. */
/* Used, for example, by TCP/IP tools */
Parse var Description Description '##' BuildInfo '@@' Description2

If Description='' then
do
  Description=Description2

  /* Another type extensions (Odin, Innotek)*/
  Parse var BuildInfo BuildInfo '1##' BuildInfo2

  If BuildInfo<>'' then
  do
    Parse var BuildInfo 'built ' BuildDate ' -- On ' HostName ';' Unknown
    Parse var Description Package ':' Description
  end

  /* Following is another type extensions (found in Odin) */

  If BuildInfo='' then
  do
    BuildDate=SubStr(BuildInfo2, 1, 26);
    BuildInfo2=DelStr(BuildInfo2, 1, 25);
    parse var BuildInfo2 HostName ':' ASDFeatureID ':' LanguageCode ':' CountryCode ':' Build ':' Unknown ':' FixPackVer
  end

end

/* And show time! */

Say SysGetMessage(8068)||Signature
Say SysGetMessage(8069)||Vendor
Say SysGetMessage(8070)||Revision
If BuildDate<>'' then Say SysGetMessage(8071)||Space(BuildDate)
If HostName<>'' then Say SysGetMessage(8072)||Space(HostName)
If Build<>'' then Say SysGetMessage(8073)||Revision'.'Build
else Say SysGetMessage(8073)||Revision
If Package<>'' then Say SysGetMessage(8074)||Space(Package)
Say SysGetMessage(8075)||Space(Description)
If ASDFeatureID<>'' then Say SysGetMessage(8076)||ASDFeatureID
If LanguageCode<>'' then Say SysGetMessage(8077)||LanguageCode
If CountryCode<>'' then Say SysGetMessage(8078)||CountryCode
If FixPackVer<>'' then Say SysGetMessage(8079)||FixPackVer

