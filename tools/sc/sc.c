/* SOM Compiler */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void main(void)
{
  char * emitters;
  char * tmpstr;
  long emitappend;
  char * ir;
  char * include;
  char * knownexts;
  char * sombase;
  char * somruntime;
  char * smclasses;
  char * tmpdir;
  char * comment;
  char * verbose;
  char * somcpp;
  char * somipc;
  long addstar;

  emitters=getenv("SMEMIT");
  if (!emitters) emitters="h;ih";

  tmpstr=getenv("SMEMITAPPEND");
  if (!tmpstr) emitappend=0;
  else
  {
    if (!strcmp(tmpstr, "1")) emitappend=1;
    if (!strcmp(tmpstr, "0")) emitappend=0;
  }
  ir=getenv("SOMIR");
  include=getenv("SMINCLUDE");
  if (!include) include=getenv("INCLUDE");
  if (!include) include=".;\som\include";
  knownexts=getenv("SMKNOWEXTS");
  tmpstr=getenv("SMADDSTAR");
  if (!tmpstr) addstar=1; else addstar=0;
  sombase=getenv("SOMBASE");
  somruntime=getenv("SOMRUNTIME");
  smclasses=getenv("SMCLASSES");
  tmpdir=getenv("TEMP");
  comment="-C";
  verbose="";
  somcpp="somcpp.exe";
  somipc="somipc.exe";
}

/*

if arg() then
  do i=1 to arg()
    if pos('-', arg(i))>0 then
    do
      if arg(i)='-V' then
        'bldlevel sc.exe'
      else
      if arg(i)='-c' then
        comment=''
      else
      if arg(i)='-v' then
        verbose='-v'
      else
      if arg(i)='-h' then
        call usage;
    end
    else
    do
      if verbose='-v' then say 'Running shell command:'
      '@'||somcpp||' -D__OS2__  -I. -IC:\os2tk45\h -IC:\os2tk45\idl -IC:\os2tk45\som\include -D__SOMIDL_VERSION_1__  -D__SOMIDL__  '||comment||' '||arg(i)||' > '||tmpdir||'e8100000.CTN'
      '@'||somipc||' -mppfile='||tmpdir||'e8100000.CTN    '||verbose||' -e emith -e emitih -e emitctm -e emitc  -o somcls '||arg(i)
      if verbose='-v' then say 'Removed "'||tmpdir||'e8100000.CTN".'
      '@del /q '||tmpdir||'e8100000.CTN'
    end
  end
else
  say 'fatal error: No source file specified.'

return

usage:
say 'usage: sc [-C:D:E:I:S:VU:cd:hi:m:prsvw] f1 f2 ...'
say 'Where:'
say '        -C <n>            - size of comment buffer (default: 200000)'
say '        -D <DEFINE>       - same as -D option for cpp.'
say '        -E <var>=<value>  - set environment variable.'
say '        -I <INCLUDE>      - same as -I option for cpp.'
say '        -S <n>            - size of string buffer (default: 200000)'
say '        -U <UNDEFINE>     - same as -U option for cpp.'
say '        -V                - show version number of compiler.'
say '        -c                - ignore all comments.'
say '        -d <dir>          - output directory for each emitted file.'
say '        -h                - this message.'
say '        -i <file>         - use this file name as supplied.'
say '        -m <name[=value]> - add global modifier.'
say '        -p                - shorthand for -D__PRIVATE__.'
say '        -r                - check releaseorder entries exist (default: FALSE).'
say '        -s <string>       - replace SMEMIT variable with <string>'
say '        -u                - update interface repository.'
say '        -v                - verbose debugging mode (default: FALSE).'
say "        -w                - don't display warnings (default: FALSE)."
say ''
say 'Modifiers:'
say '        addprefixes : adds "functionprefix" to method names in template file'
say '        [no]addstar : [no]add "*" to C bindings for interface references.'
say '              corba : check the source for CORBA compliance.'
say '                csc : force running of OIDL compiler.'
say '         emitappend : append the emitted files at the end of existing file.'
say "           noheader : don't add a header to the emitted file."
say "              noint : don't warn about "int" causing portability problems."
say "             nolock : don't lock the IR during update."
say "               nopp : don't run the source through the pre-processor."
say "               notc : don't use typecodes for emit information."
say "         nouseshort : don't generate short names for types."
say '          pp=<path> : specify a local pre-processor to use.'
say '           tcconsts : generate CORBA TypeCode constants.'
say ''
say 'Note: All command-line modifiers can be set in the environment'
say 'by changing them to UPPERCASE and preappending "SM" to them.'
say ''
say 'Environment Variables (current state):'
say '        SMEMIT='||emitters
say '                : emitters to run (default : h;ih).'
say '        SMINCLUDE='||include
say '                : where to search for .idl and .efw files.'
say '        SMKNOWNEXTS='||knownext
say '                : add headers to user written emitters.'
say '        SMADDSTAR='||addstar
say '                : add or no "*" to C bindings for interface references.'
say '        SMEMITAPPEND='||emitappend
say '                : add or no "*" to C bindings for interface references.'
return
*/
