/*
 * This is an example for using GCI. Enjoy, copy and paste!
 * We have three different execution paths for Unix, Win32 and OS/2.
 * For some interpreters, most notable the builtin interpreter of OS/2, you
 * have to rename this file to gci-try.cmd
 */
parse version version
say "Your REXX interpreter is" version

/******************************************************************************
 * Try to detect the system to show several things.
 */
parse var version "REXX-"ipret"_".
IsRegina = 0
if ipret = "Regina" then do
   IsRegina = 1
   uname = uname('S')
   options NOEXT_COMMANDS_AS_FUNCS
   end

parse source system . source

system = translate(system)
if left(system, 3) = "WIN" then do
   library = "GCI.DLL"
   CLib = "MSVCRT"
   system = "WIN"
   end
else if system = "OS/2" then do
   library = "GCI.DLL"
   system = "OS/2"
   end
else do
   library = "libgci.so"
   tr = trace()
   trace o
   "test `uname -s` = 'SunOS'"
   if RC = 0 then do
      CLib = "libc.so.1"
      MathLib = "m"
      end
   else do
      if IsRegina & Left(uname, 6) = 'CYGWIN' then do
         CLib = "cygwin1.dll"
         MathLib = "cygwin1.dll"
         end
      else do
         CLib = "libc.so.6"
         MathLib = "m"
         end
      end
   trace value tr
   end

/*
 * Check for a builtin RxFuncDefine. Call it with errorneous arguments and
 * expect a simple error if installed and a syntax signal if unknown.
 */
InternalGCI = 0
signal on syntax name NotInstalled
x = "X"
x. = "X"
h = RxFuncDefine(x,x,x,x)
if h \= 0 & h \= 10005 & DataType(h, "NUM") then
   InternalGCI = 1
NotInstalled:
drop GCI_RC

signal on syntax
if \InternalGCI then do
   /*
    * The DLL may have been loaded already. Prevent bugs at this stage.
    */
   call RxFuncDrop RxFuncDefine
   call RxFuncDrop GciPrefixChar
   call RxFuncDrop GciFuncDrop
   if RxFuncadd(RxFuncDefine, "gci", "RxFuncDefine") \= 0 then do
      msg = "Can't load RxFuncDefine of" library
      if IsRegina then
         msg = msg || ":" RxFuncErrMsg()
      say msg
      return 1
      end
   if RxFuncadd(GciFuncDrop, "gci", "GciFuncDrop") \= 0 then do
      msg = "Can't load GciFuncDrop of" library
      if IsRegina then
         msg = msg || ":" RxFuncErrMsg()
      say msg
      return 1
      end
   call RxFuncadd GciPrefixChar, "gci", "GciPrefixChar"
   say "Your interpreter has no internal support of GCI"
   end
else
   say "Your interpreter has internal support of GCI"

say ""

/******************************************************************************
 * After the prelimnaries try to use the GCI.
 */
if system = "OS/2" then signal useOS2

say "Trying to copy string 'hello' to a string 'world' using the C library"
stem.calltype = cdecl
stem.0 = 2
stem.1.name = "Target"
stem.1.type = indirect string80
stem.2.name = "Source"
stem.2.type = indirect string80
stem.return.type = ""           /* We are not interested in the return value */

call funcDefine strcpy, CLib, "strcpy", stem

stem.1.name = "Target"
stem.1.value = "world"
stem.2.name = "Source"
stem.2.value = "hello"
call strcpy stem
say stem.1.name"="stem.1.value stem.2.name"="stem.2.value

call funcDrop strcpy
say ""
/******************************************************************************
 * Check if v1.1 GciPrefixChar works.
 */

version = 1.0
signal on syntax name v1.0
say "Trying to detect and use v1.1 features (same result as above expected)"
oldChar = GciPrefixChar('!')
signal on syntax
stem.!calltype = cdecl
stem.0 = 2
stem.1.!name = "Target"
stem.1.!type = indirect string80
stem.2.!name = "Source"
stem.2.!type = indirect string80
stem.!return.!type = ""

call funcDefine strcpy, CLib, "strcpy", stem

stem.1.!name = "Target"
stem.1.!value = "world"
stem.2.!name = "Source"
stem.2.!value = "hello"
call strcpy stem
say stem.1.!name"="stem.1.!value stem.2.name"="stem.2.!value

call funcDrop strcpy
say ""
call GciPrefixChar oldChar
drop oldChar c1 c2
version = 1.1
v1.0:
/******************************************************************************
 * Use the "as function" feature
 */
say "Trying to find the last occurcance of '.' in 'James F. Cooper' using the C"
say "library using the 'as function' feature"
stem.calltype = cdecl as function
stem.0 = 2
stem.1.name = "String"
stem.1.type = indirect string80
stem.2.name = "Character"
stem.2.type = char
stem.return.type = indirect string80

call funcDefine strrchr, CLib, "strrchr", stem

stem.1.name = "Target"
stem.1.value = "James F. Cooper"
stem.2.name = "Character"
stem.2.value = .
say "The last dot starts at '" || strrchr( stem ) || "'"
say ""
say "Trying to find the last occurcance of '.' in 'James Cooper' using the C lib"
say "We expect a NULL pointer which leads to a dropped value which becomes ''"
say "for a return value."

stem.1.name = "Target"
stem.1.value = "James Cooper"
say "The last dot starts at '" || strrchr( stem ) || "'"

call funcDrop strrchr
say ""
/******************************************************************************
 * Use the "with parameters as function" feature.
 * Note that you must omit "as function" if the return value doesn't exist.
 * We use separate functions for Windows and unix.
 */
if system = "WIN" then signal useWindows

say "Trying to use the math library to compute some natural logarithms"
stem.calltype = cdecl with parameters as function
stem.0 = 1
stem.1.name = "X"
stem.1.type = float128
stem.return.type = float128
call RxFuncDefine logl, MathLib, "logl", stem
if RESULT \= 0 then do
   stem.1.type = float96
   stem.return.type = float96
   call RxFuncDefine logl, MathLib, "logl", stem
   end
if RESULT \= 0 then do
   stem.1.type = float64
   stem.return.type = float64
   call RxFuncDefine logl, MathLib, "log", stem
   end
if RESULT \= 0 then do
   if IsRegina & InternalGCI then
      say "Error, code" RESULT || ":" RxFuncErrMsg()
   else
      say "Error, code" RESULT || ":" GCI_RC
   return 1
   end

say "some logarithms"
do i = 1 to 5
   say "log("i")="logl(i)
   end

call funcDrop logl
say ""
/*******************************/
say "Using a structure and checking the file system's size."
say "You may look into the source."
/*
 * This examples has removed all unnecessary stuff.
 */
stem.calltype = cdecl as function
stem.0 = 2
stem.1.type = indirect string256
stem.2.type = indirect container
stem.2.0 = 10                                /* statvfs64 */
stem.2.1.type    = unsigned                  /* bsize */
stem.2.2.type    = unsigned                  /* frsize */
stem.2.3.type    = unsigned64                /* blocks */
stem.2.4.type    = unsigned64                /* bfree */
stem.2.5.type    = unsigned64                /* bavail */
stem.2.6.type    = unsigned64                /* files */
stem.2.7.type    = unsigned64                /* ffree */
stem.2.8.type    = unsigned64                /* favail */
stem.2.9.type    = unsigned                  /* fsid */
stem.2.10.type   = string256                /* indifferent between unices */
stem.return.type = integer

call funcDefine statvfs, CLib, "statvfs64", stem

args. = 0
args.1.value = source
args.2.value = 10    /* otherwise the argument becomes NULL */
if statvfs( args ) \= -1 then do
   say "statvfs-info of" source
   say "block size =" args.2.1.value "byte"
   size = trunc(args.2.3.value * args.2.1.value / (1024 * 1024))
   avail = trunc(args.2.5.value * args.2.1.value / (1024 * 1024))
   say "file system size =" size"MB, available =" avail"MB"
   say "file nodes =" args.2.6.value "available =" args.2.8.value
   say "sid =" args.2.9.value
   end
else
   say "Sorry, '"source"' not found."

call funcDrop statvfs
say ""
/*******************************/
say "We use qsort of the C library for sorting some strings using arrays."
/*
 * This examples has removed all unnecessary stuff.
 * We need a sorting routine. Without callbacks we have to use one of a
 * library. "strcmp" is a good example. We have to play with the dynamic
 * link loader.
 * The strategy is:
 * Load the loader functions (dlopen, dlsym, dlclose)
 * Load the compare routine (strcmp) using the loader functions
 * Load the sorting routine (qsort) and do the sort
 * Additional sort operations may have to redefine qsort only.
 */
stem.calltype = cdecl with parameters as function
stem.0 = 2
stem.1.type = indirect string256
stem.2.type = integer
stem.return.type = integer       /* handle, but who cares? */
call funcDefine dlopen, "dl", "dlopen", stem

stem.calltype = cdecl with parameters as function
stem.0 = 2
stem.1.type = integer            /* handle */
stem.2.type = indirect string256
stem.return.type = integer       /* entry point address, but who cares? */
call funcDefine dlsym, "dl", "dlsym", stem

stem.calltype = cdecl with parameters as function
stem.0 = 1
stem.1.type = integer            /* handle */
stem.return.type = integer
call funcDefine dlclose, "dl", "dlclose", stem

CLibHandle = dlopen( CLib, 1 /* RTLD_LAZY */ )
if CLibHandle = 0 then do
   say "dlopen() can't load" CLib
   return 1
   end

strcmp = dlsym( CLibHandle, "strcmp" )
if strcmp = 0 then do
   say "dlsym() can't relocate strcmp()"
   return 1
   end

stem.calltype = cdecl
stem.0 = 4
stem.1.type = indirect array
stem.1.0 = 3
stem.1.1.type = string95
stem.2.type = integer
stem.3.type = integer
stem.4.type = integer
stem.return.type = ""
call funcDefine qsort10, CLib, "qsort", stem

args.0 = 4
args.1.value = 3
args.1.1.value = "Ann"
args.1.2.value = "Charles"
args.1.3.value = "Betty"
args.2.value = 3
args.3.value = 96
args.4.value = strcmp
say "Sorting (" args.1.1.value args.1.2.value args.1.3.value ") ..."
call qsort10 args
say "Sorted values are (" args.1.1.value args.1.2.value args.1.3.value ")"

call dlclose CLibHandle
call funcDrop qsort
call funcDrop dlclose
call funcDrop dlsym
call funcDrop dlopen
say ""
call accessStructTm
return 0


/***************************************************/
useWindows:
stem.calltype = stdcall with parameters as function
stem.0 = 4
stem.1.name = "HWND"
stem.1.type = unsigned
stem.2.name = "Text"
stem.2.type = indirect string1024
stem.3.name = "Caption"
stem.3.type = indirect string1024
stem.4.name = "Type"
stem.4.type = unsigned
stem.return.type = integer

call funcDefine messagebox, "user32", "MessageBoxA", stem

MB_YESNO_INFO = x2d(44)
if messagebox( 0, "Do you love this rocking GCI?", "GCI", MB_YESNO_INFO ) = 6 then
   say "Yes, you're right, GCI is cool."
else
   say "No, you're kidding! GCI is cool."

call funcDrop messagebox
say ""
/*******************************/
say "We operate on containers and check this file's date."
say "You may look into the source."
/*
 * This examples has removed all unnecessary stuff.
 */
stem.calltype = stdcall as function
stem.0 = 2
stem.1.type = indirect string256
stem.2.type = indirect container
stem.2.0 = 8                                 /* WIN32_FIND_DATA */
stem.2.1.type = unsigned                     /* FileAttributes */
stem.2.2.type = unsigned64                   /* Creation */
stem.2.3.type = unsigned64                   /* Access */
stem.2.4.type = unsigned64                   /* Write */
stem.2.5.type = unsigned64                   /* Size */
stem.2.6.type = unsigned64                   /* Reserved */
stem.2.7.type = string259                    /* FileName */
stem.2.8.type = string13                     /* AlternateFileName */
stem.return.type = integer

stem2.calltype = stdcall with parameters
stem2.0 = 1
stem2.1.type = integer
stem2.return.type = ""

call funcDefine findfirstfile, "kernel32", "FindFirstFileA", stem

call funcDefine findclose, "kernel32", "FindClose", stem2

args. = 0
args.1.value = source
args.2.value = 8    /* otherwise the argument becomes NULL */
handle = findfirstfile( args )
if handle \= -1 then do
   say "argument's name="source
   say "filename="args.2.7.value
   say "8.3-name="args.2.8.value
   numeric digits 40
   filetime = args.2.4.value
   d = /*second*/ 1000*1000*10   *   /*seconds per day*/ 60*60*24
   daypart = trunc(filetime / d)
   date = date( 'N', daypart + date('B', 16010101, 'S'), 'B')
   ns = filetime - daypart * d
   secs = ns % (10*1000*1000)
   fract = ns // (10*1000*1000)
   time = time('N', secs, 'S') || "." || right(fract, 7, '0')
   say "ns from 1.1.1601="filetime "= GMT," date || "," time
   numeric digits 9
   call findclose handle
   end
else
   say "Sorry, '"source"' not found."

call funcDrop findfirstfile
call funcDrop findclose
say ""
call accessStructTm

return 0

/***************************************************/
accessStructTm: procedure expose IsRegina InternalGCI CLib version
if version < 1.1 then
   return
say "Finally, we use the LIKE keyword and check the number of the week."
say "You may look into the source, we use a PROCEDURE and v1.1 specific code."
/*
 * This examples has removed all unnecessary stuff.
 */
tm.0 = 10
tm.1.type = integer /* tm_sec */
tm.2.type = integer /* tm_min */
tm.3.type = integer /* tm_hour */
tm.4.type = integer /* tm_mday */
tm.5.type = integer /* tm_mon */
tm.6.type = integer /* tm_year */
tm.7.type = integer /* tm_wday */
tm.8.type = integer /* tm_yday */
tm.9.type = integer /* tm_isdst */
tm.10.type = string 32 /* reserved stuff sometimes used by the OS */

time_t.0 = 1
time_t.1.type = integer64 /* SURPRISE! some systems may use 64 bit data types
                           * already. We don't have problems with this,
                           * because we use double buffering.
                           */

stem.calltype = cdecl
stem.0 = 1
stem.1.type = indirect container like time_t
stem.return.type = ""
call funcDefine _time, CLib, "time", stem

stem.calltype = cdecl
stem.0 = 1
stem.1.type = indirect container like time_t
stem.return.type = indirect container like tm
call funcDefine localtime, CLib, "localtime", stem

stem.calltype = cdecl
stem.0 = 4
stem.1.type = indirect string256  /* dest */
stem.2.type = unsigned            /* size(dest) */
stem.3.type = indirect string256  /* template */
stem.4.type = indirect container like tm
stem.return.type = unsigned
call funcDefine strftime, CLib, "strftime", stem

time_val.1.value = 1
time_val.1.1.value = 1
call _time time_val

lct.1.value = 1
lct.1.1.value = time_val.1.1.value
call localtime lct

strf.1.value = ""
strf.2.value = 256
strf.3.value = "%A"
strf.4.value = lct.return.value
do i = 1 to 10
   strf.4.i.value = lct.return.i.value
   end
call strftime strf
dayname = strf.1.value

strf.3.value = "%B"
call strftime strf
monthname = strf.1.value

strf.3.value = "%U"
call strftime strf
week!Sun = strf.1.value

strf.3.value = "%W"
call strftime strf
week!Mon = strf.1.value

if week!Mon = week!Sun then
   add = "."
else
   add = " if you count Monday as the first day of the week. Otherwise it " ||,
         "is the" week!Sun || ". week."
say "Today is a" dayname "in" monthname || ". We have the" week!Mon || ". week" || add
say
call funcDrop strftime
call funcDrop localtime
call funcDrop _time

return

/***************************************************/
useOS2:
say "Checking the high precision system timer."
stem.calltype = stdcall
stem.0 = 1
stem.1.name = "Frequency"
stem.1.type = indirect unsigned
stem.return.type = "unsigned"

call funcDefine DosTmrQueryFreq, "doscalls", "#362", stem

stem.1.name = "Frequency"
stem.1.value = 0        /* don't raise NOVALUE */

call DosTmrQueryFreq stem
if stem.return.value \= 0 then
   say "Error" stem.return.value "while using DosTmrQueryFreq."
else
   say "The timer has a frequency of" stem.1.value "Hz"

call funcDrop DosTmrQueryFreq
say ""
/*******************************/
say "You should hear your beeper."
/*
 * Use the "with parameters" feature.
 */
stem.calltype = stdcall with parameters
stem.0 = 2
stem.1.name = "Frequency"
stem.1.type = unsigned
stem.2.name = "Duration"
stem.2.type = unsigned
stem.return.type = ""           /* We are not interested in the return value */

call funcDefine DosBeep, "doscalls", "#286", stem

do i = 500 to 3000 by 100
   call DosBeep i, 10
   end

call funcDrop DosBeep
say ""
/*******************************/
say "Checking the installed codepages."
/*
 * Use the "as function" feature.
 */
stem.calltype = stdcall as function
stem.0 = 3
stem.1.name = "cb"
stem.1.type = unsigned
stem.2.name = "arCP"
stem.2.type = indirect array
stem.2.0 = 25
stem.2.1.type = unsigned
stem.3.name = "pcCP"
stem.3.type = indirect unsigned
stem.return.type = "unsigned"

call funcDefine DosQueryCp, "doscalls", "#291", stem

drop stem.
stem. = 0 /* NOVALUE should not happen */
stem.0 = 3
stem.1.name = "cb"
stem.1.value = 100
stem.2.name = "arCP"
stem.2.value = 25
stem.3.name = "pcCP"
if DosQueryCp( stem ) = 0 then do
   say "current codepage:" stem.2.1.value
   do i = 2 to stem.3.value / 4
      say "prepared codepage:" stem.2.i.value
      end
   end
else
   say "Error calling DosQueryCp."

call funcDrop DosQueryCp
say ""
/*******************************/
say "Examining the file system on" left( source, 2 )
/*
 * Use the "as function" feature.
 */
stem.calltype = stdcall as function
stem.0 = 4
stem.1.name = "disknum"
stem.1.type = unsigned
stem.2.name = "infolevel"
stem.2.type = unsigned
stem.3.name = "pBuf"
stem.3.type = indirect container
stem.3.0 = 5
stem.3.1.name = "idFileSystem"
stem.3.1.type = unsigned
stem.3.2.name = "cSectorUnit"
stem.3.2.type = unsigned
stem.3.3.name = "cUnit"
stem.3.3.type = unsigned
stem.3.4.name = "cUnitAvail"
stem.3.4.type = unsigned
stem.3.5.name = "cbSector"
stem.3.5.type = unsigned16
stem.4.name = "cbBuf"
stem.4.type = unsigned
stem.return.type = "unsigned"

call funcDefine DosQueryFSInfo, "doscalls", "#278", stem

drop stem.
stem. = 0 /* NOVALUE should not happen */
stem.0 = 3
stem.1.name = "disknum"
stem.1.value = c2d( translate( left( source, 1 ) ) ) - c2d( 'A' ) + 1
stem.2.name = "infolevel"
stem.2.value = 1
stem.3.name = "pBuf"
stem.3.value = 5
stem.4.name = "cbBuf"
stem.4.value = 18
if DosQueryFSInfo( stem ) = 0 then do
   cluster = stem.3.2.value * stem.3.5.value
   say "Total size:" showFileSize( cluster*stem.3.3.value )
   say "Free  size:" showFileSize( cluster*stem.3.4.value )
   end
else
   say "Error calling DosQueryFSInfo."

call funcDrop DosQueryFSInfo
say ""
return 0

/*****/
showFileSize: procedure
   suffix = "byte"
   size = arg(1)
   suffixes = "KB MB GB TB"
   do i = 1 to words( suffixes )
      if size < 1024 then
         leave
      suffix = word( suffixes, i )
      size = size / 1024
      end
   if size >= 100 then
      size = format( size, , 0 )
   else if size >= 10 then
      size = format( size, , 1 )
   else
      size = format( size, , 2 )
   return size suffix

/*****************************************************************************/
syntax:
   /*
    * Not all interpreters are ANSI compatible.
    */
   code = .MN
   if code = '.MN' then
      code = RC
   if datatype( SIGL_FUNCDEFINE, "W" ) then
      SIGL = SIGL_FUNCDEFINE
   say "Error" code "in line" SIGL || ":" condition('D')
   say "GCI_RC=" || GCI_RC
   exit 0

/*****************************************************************************/
funcDrop:
   /*
    * Drops one defined function depending on whether is is defined in the
    * lightweight library or in the interpreter's kernel.
    */
   if InternalGCI then
      call RxFuncDrop arg(1)
   else
      call GciFuncDrop arg(1)
   return

/*****************************************************************************/
funcDefine:
   /*
    * Defines a new subroutine as RxFuncDefine does, additionally it undefines
    * (drops) the subroutine in front and it shows the error messages.
    * Finally it terminates the process is an error occurs.
    */
   _SIGL_FUNCDEFINE = SIGL
   call funcDrop arg(1)
   drop GCI_RC
   SIGL_FUNCDEFINE = _SIGL_FUNCDEFINE
   call RxFuncDefine arg(1), arg(2), arg(3), arg(4)
   drop SIGL_FUNCDEFINE _SIGL_FUNCDEFINE
   if RESULT = 0 then
      return
   if IsRegina & InternalGCI then
      errAdd = ":" RxFuncErrMsg()
   else do
      if GCI_RC \= "GCI_RC" then
         errAdd = ":" GCI_RC
      else
         errAdd = ""
      end
   say "Error defining '" || arg(1) || "', code" RESULT || errAdd

   exit 1

