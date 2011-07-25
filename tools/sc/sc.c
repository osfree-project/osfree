/* SOM Compiler */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Some macros for easy code reading
#define VERBOSE (!strcmp(verbose, "-v"))

void usage(void)
{
  printf("usage: sc [-C:D:E:I:S:VU:cd:hi:m:prsvw] f1 f2 ...\n");
  printf("Where:\n");
  printf("        -C <n>            - size of comment buffer (default: 200000)\n");
  printf("        -D <DEFINE>       - same as -D option for cpp.\n");
  printf("        -E <var>=<value>  - set environment variable.\n");
  printf("        -I <INCLUDE>      - same as -I option for cpp.\n");
  printf("        -S <n>            - size of string buffer (default: 200000)\n");
  printf("        -U <UNDEFINE>     - same as -U option for cpp.\n");
  printf("        -V                - show version number of compiler.\n");
  printf("        -c                - ignore all comments.\n");
  printf("        -d <dir>          - output directory for each emitted file.\n");
  printf("        -h                - this message.\n");
  printf("        -i <file>         - use this file name as supplied.\n");
  printf("        -m <name[=value]> - add global modifier.\n");
  printf("        -p                - shorthand for -D__PRIVATE__.\n");
  printf("        -r                - check releaseorder entries exist (default: FALSE).\n");
  printf("        -s <string>       - replace SMEMIT variable with <string>\n");
  printf("        -u                - update interface repository.\n");
  printf("        -v                - verbose debugging mode (default: FALSE).\n");
  printf("        -w                - don't display warnings (default: FALSE).\n");
  printf("\n");
  printf("Modifiers:\n");
  printf("        addprefixes : adds 'functionprefix' to method names in template file\n");
  printf("        [no]addstar : [no]add '*' to C bindings for interface references.\n");
  printf("              corba : check the source for CORBA compliance.\n");
  printf("                csc : force running of OIDL compiler.\n");
  printf("         emitappend : append the emitted files at the end of existing file.\n");
  printf("           noheader : don't add a header to the emitted file.\n");
  printf("              noint : don't warn about 'int' causing portability problems.\n");
  printf("             nolock : don't lock the IR during update.\n");
  printf("               nopp : don't run the source through the pre-processor.\n");
  printf("               notc : don't use typecodes for emit information.\n");
  printf("         nouseshort : don't generate short names for types.\n");
  printf("          pp=<path> : specify a local pre-processor to use.\n");
  printf("           tcconsts : generate CORBA TypeCode constants.\n");
  printf("\n");
  printf("Note: All command-line modifiers can be set in the environment\n");
  printf("by changing them to UPPERCASE and preappending 'SM' to them.\n");
  printf("\n");
  printf("Environment Variables (current state):\n");
  printf("        SMEMIT='||emitters\n");
  printf("                : emitters to run (default : h;ih).\n");
  printf("        SMINCLUDE='||include\n");
  printf("                : where to search for .idl and .efw files.\n");
  printf("        SMKNOWNEXTS='||knownext\n");
  printf("                : add headers to user written emitters.\n");
  printf("        SMADDSTAR='||addstar\n");
  printf("                : add or no '*' to C bindings for interface references.\n");
  printf("        SMEMITAPPEND='||emitappend\n");
  printf("                : add or no '*' to C bindings for interface references.\n");
}

int main(int argc, char **argv)
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
  char * verbose;    // be verbose or not
  char * somcpp;     // name of preprocessor
  char * somipc;     // name of abstrach and object graph builder
  char * sourcefile; // filename to compile
  long addstar;


  int i;             // parameter counter
  char str[280];


  ////////////////////////////
  // prepare working variables
  ////////////////////////////

  // initial source file
  sourcefile="";

  // list of emitters from environment or set default
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
  if (!tmpstr) addstar=1;
  else
  {
    if (!strcmp(tmpstr, "1")) addstar=1;
    if (!strcmp(tmpstr, "0")) addstar=0;
  }
  sombase=getenv("SOMBASE");
  somruntime=getenv("SOMRUNTIME");
  smclasses=getenv("SMCLASSES");
  tmpdir=getenv("TEMP");
  comment="-C";
  verbose="";
  somcpp="somcpp.exe";
  somipc="somipc.exe";

  // parse arguments
  if(argc <= 1) {
    printf("fatal error: No source file specified.\n");
  }

  for (i=1; i<argc; i++)
  {
    if ((!strcmp(argv[i], "-h"))||(!strcmp(argv[i], "-?")))
    {
      usage();
      return(0);
    } else
    if (!strcmp(argv[i], "-v"))
    {
      verbose="-v";
    } else
    if (!strcmp(argv[i], "-c"))
    {
      comment="";
    }
  }

  if VERBOSE printf("Running shell command:\n");

  sprintf(str, "%s -D__OS2__  -I. -IC:\os2tk45\h -IC:\os2tk45\idl -IC:\os2tk45\som\include -D__SOMIDL_VERSION_1__  -D__SOMIDL__  %s %s> %s\\e8100000.CTN\n", somcpp, comment, sourcefile, tmpdir);
  if VERBOSE printf(str);
  system(str);

  sprintf(str, "%s -mppfile=%s\\e8100000.CTN %s -e emith -e emitih -e emitctm -e emitc  -o somcls %s\n", somipc, tmpdir, verbose, sourcefile);
  if VERBOSE printf(str);
  system(str);

  sprintf(str, "del /q %s\\e8100000.CTN\n", tmpdir);
  if VERBOSE printf(str);
  system(str);

  if VERBOSE printf("Removed \"%s\\e8100000.CTN\"\n", tmpdir);
}

