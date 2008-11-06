/* SOM Compiler main wrapper */

BLDLEVEL='@#osFree.org:1.0#@SOM Compiler#@'

emitters=value("SMEMIT",,"ENVIRONMENT")
if emitters='' then emitters='h;ih'
emitappend=value("SMEMITAPPEND",,"ENVIRONMENT")
if emitappend='' then emitappend=0
ir=value("SOMIR",,"ENVIRONMENT")
include=value("SMINCLUDE",,"ENVIRONMENT")
if include='' then include=value("SMINCLUDE",,"ENVIRONMENT")
if include='' then include='.'
knownexts=value("SMKNOWEXTS",,"ENVIRONMENT")
addstar=value("SMADDSTAR",,"ENVIRONMENT")
if addstar='' then addstar=1
sombase=value("SOMBASE",,"ENVIRONMENT")
somruntime=value("SOMRUNTIME",,"ENVIRONMENT")
smclasses=value("SMCLASSES",,"ENVIRONMENT")

sompp='sompp.exe'

if arg() then
  do i=0 to arg()
    say arg(i)
    if pos('-', arg(i))>0 then
    do
      if arg(i)='-V' then
        'bldlevel sc.exe'
      else
      if arg(i)='-h' then
        call usage;
    end
    else say arg(i)
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
