#   File:       rexx.make
#   Target:     rexx
#   Created:    Sunday, March 18, 2001 07:43:32 AM


MAKEFILE        = rexx.make
¥MondoBuild¥    = {MAKEFILE}  # Make blank to avoid rebuilds when makefile is modified

ObjDir          = :
Includes        = 

Sym-68K         = -sym on

COptions        = {Includes} {Sym-68K} -d MAC -d FGC -d NO_EXTERNAL_QUEUES -model far


### Source Files ###

SrcFiles        =  ¶
				  alloca.c ¶
				  builtin.c ¶
				  cmath.c ¶
				  cmsfuncs.c ¶
				  convert.c ¶
				  dbgfuncs.c ¶
				  debug.c ¶
				  doscmd.c ¶
				  envir.c ¶
				  error.c ¶
				  expr.c ¶
				  extlib.c ¶
				  extstack.c ¶
				  files.c ¶
				  funcs.c ¶
				  getopt.c ¶
				  instore.c ¶
				  interp.c ¶
				  interprt.c ¶
				  lexsrc.c ¶
				  library.c ¶
				  macros.c ¶
				  mac.c ¶
				  memory.c ¶
				  misc.c ¶
				  mt_notmt.c ¶
				  nosaa.c ¶
				  options.c ¶
				  os2funcs.c ¶
				  parsing.c ¶
				  rexx.c ¶
				  rexxbif.c ¶
				  rexxext.c ¶
				  shell.c ¶
				  signals.c ¶
				  stack.c ¶
				  strings.c ¶
				  strmath.c ¶
				  tracing.c ¶
				  unxfuncs.c ¶
				  variable.c ¶
				  wrappers.c ¶
				  yaccsrc.c


### Object Files ###

ObjFiles-68K    =  ¶
				  "{ObjDir}alloca.c.o" ¶
				  "{ObjDir}builtin.c.o" ¶
				  "{ObjDir}cmath.c.o" ¶
				  "{ObjDir}cmsfuncs.c.o" ¶
				  "{ObjDir}convert.c.o" ¶
				  "{ObjDir}dbgfuncs.c.o" ¶
				  "{ObjDir}debug.c.o" ¶
				  "{ObjDir}doscmd.c.o" ¶
				  "{ObjDir}envir.c.o" ¶
				  "{ObjDir}error.c.o" ¶
				  "{ObjDir}expr.c.o" ¶
				  "{ObjDir}extlib.c.o" ¶
				  "{ObjDir}extstack.c.o" ¶
				  "{ObjDir}files.c.o" ¶
				  "{ObjDir}funcs.c.o" ¶
				  "{ObjDir}getopt.c.o" ¶
				  "{ObjDir}instore.c.o" ¶
				  "{ObjDir}interp.c.o" ¶
				  "{ObjDir}interprt.c.o" ¶
				  "{ObjDir}lexsrc.c.o" ¶
				  "{ObjDir}library.c.o" ¶
				  "{ObjDir}mac.c.o" ¶
				  "{ObjDir}macros.c.o" ¶
				  "{ObjDir}memory.c.o" ¶
				  "{ObjDir}misc.c.o" ¶
				  "{ObjDir}mt_notmt.c.o" ¶
				  "{ObjDir}nosaa.c.o" ¶
				  "{ObjDir}options.c.o" ¶
				  "{ObjDir}os2funcs.c.o" ¶
				  "{ObjDir}parsing.c.o" ¶
				  "{ObjDir}rexx.c.o" ¶
				  "{ObjDir}rexxbif.c.o" ¶
				  "{ObjDir}rexxext.c.o" ¶
				  "{ObjDir}shell.c.o" ¶
				  "{ObjDir}signals.c.o" ¶
				  "{ObjDir}stack.c.o" ¶
				  "{ObjDir}strings.c.o" ¶
				  "{ObjDir}strmath.c.o" ¶
				  "{ObjDir}tracing.c.o" ¶
				  "{ObjDir}unxfuncs.c.o" ¶
				  "{ObjDir}variable.c.o" ¶
				  "{ObjDir}wrappers.c.o" ¶
				  "{ObjDir}yaccsrc.c.o"

### Libraries ###

LibFiles-68K    =  ¶
				  "{Libraries}MathLib.far.o" ¶
				  "{CLibraries}StdCLib.far.o" ¶
				  "{Libraries}SIOW.far.o" ¶
				  "{Libraries}MacRuntime.o" ¶
				  "{Libraries}IntEnv.far.o" ¶
				  "{Libraries}ToolLibs.far.o" ¶
				  "{Libraries}Interface.o"


### Default Rules ###

.c.o  Ä  .c  {¥MondoBuild¥}
	{C} {depDir}{default}.c -o {targDir}{default}.c.o {COptions}


### Build Rules ###

rexx  ÄÄ  {ObjFiles-68K} {LibFiles-68K} {¥MondoBuild¥}
	ILink ¶
		-o {Targ} ¶
		{ObjFiles-68K} ¶
		{LibFiles-68K} ¶
		{Sym-68K} ¶
		-mf -d ¶
		-t 'APPL' ¶
		-c 'siow' ¶
		-model far ¶
		-state rewrite ¶
		-compact -pad 0
	If "{Sym-68K}" =~ /-sym Å[nNuU]Å/
		ILinkToSYM {Targ}.NJ -mf -sym 3.2 -c 'sade'
	End

rexx  ÄÄ  "{RIncludes}"SIOW.r {¥MondoBuild¥}
	Rez "{RIncludes}"SIOW.r -o {Targ} -append


### Required Dependencies ###

"{ObjDir}alloca.c.o"  Ä  alloca.c
"{ObjDir}builtin.c.o"  Ä  builtin.c
"{ObjDir}cmath.c.o"  Ä  cmath.c
"{ObjDir}cmsfuncs.c.o"  Ä  cmsfuncs.c
"{ObjDir}convert.c.o"  Ä  convert.c
"{ObjDir}dbgfuncs.c.o"  Ä  dbgfuncs.c
"{ObjDir}debug.c.o"  Ä  debug.c
"{ObjDir}doscmd.c.o"  Ä  doscmd.c
"{ObjDir}envir.c.o"  Ä  envir.c
"{ObjDir}error.c.o"  Ä  error.c
"{ObjDir}expr.c.o"  Ä  expr.c
"{ObjDir}extlib.c.o"  Ä  extlib.c
"{ObjDir}extstack.c.o"  Ä  extstack.c
"{ObjDir}files.c.o"  Ä  files.c
"{ObjDir}funcs.c.o"  Ä  funcs.c
"{ObjDir}getopt.c.o"  Ä  getopt.c
"{ObjDir}instore.c.o"  Ä  instore.c
"{ObjDir}interp.c.o"  Ä  interp.c
"{ObjDir}interprt.c.o"  Ä  interprt.c
"{ObjDir}lexsrc.c.o"  Ä  lexsrc.c
"{ObjDir}library.c.o"  Ä  library.c
"{ObjDir}mac.c.o"  Ä  mac.c
"{ObjDir}macros.c.o"  Ä  macros.c
"{ObjDir}memory.c.o"  Ä  memory.c
"{ObjDir}misc.c.o"  Ä  misc.c
"{ObjDir}mt_notmt.c.o"  Ä  mt_notmt.c
"{ObjDir}nosaa.c.o"  Ä  nosaa.c
"{ObjDir}options.c.o"  Ä  options.c
"{ObjDir}os2funcs.c.o"  Ä  os2funcs.c
"{ObjDir}parsing.c.o"  Ä  parsing.c
"{ObjDir}rexx.c.o"  Ä  rexx.c
"{ObjDir}rexxbif.c.o"  Ä  rexxbif.c
"{ObjDir}rexxext.c.o"  Ä  rexxext.c
"{ObjDir}shell.c.o"  Ä  shell.c
"{ObjDir}signals.c.o"  Ä  signals.c
"{ObjDir}stack.c.o"  Ä  stack.c
"{ObjDir}strings.c.o"  Ä  strings.c
"{ObjDir}strmath.c.o"  Ä  strmath.c
"{ObjDir}tracing.c.o"  Ä  tracing.c
"{ObjDir}unxfuncs.c.o"  Ä  unxfuncs.c
"{ObjDir}variable.c.o"  Ä  variable.c
"{ObjDir}wrappers.c.o"  Ä  wrappers.c
"{ObjDir}yaccsrc.c.o"  Ä  yaccsrc.c


### Optional Dependencies ###
### Build this target to generate "include file" dependencies. ###

Dependencies  Ä  $OutOfDate
	MakeDepend ¶
		-append {MAKEFILE} ¶
		-ignore "{CIncludes}" ¶
		-objdir "{ObjDir}" ¶
		-objext .o ¶
		{Includes} ¶
		{SrcFiles}


