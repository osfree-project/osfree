/*
 *
 */
Trace O
validargs = '-s -d -a -A -p -S -l -L -u -e -b -B -P -t'
vars = '_srcdir _tmpdir _abiminor _abimajor _shlpre _shlpst _libpre _libpst _useabi _exe _shlfile _libfile _binprefix _target'
mandatory = '1 1 1 1 1 1 1 1 1 1 1 1 1 1'
valuereq  = '1 1 1 1 0 0 0 0 1 0 1 1 0 1'
If ValidOpts( Arg(1), validargs, mandatory, valuereq ) = 0 Then
exes = 'rexx'_binprefix||_exe 'regina'_binprefix||_exe 'rxqueue'_binprefix||_exe 'rxstack'_binprefix||_exe 'regina-config'
Select
   When Countstr( 'beos', _target ) \= 0 Then
      Do
         from = 'COPYING-LIB BUGS       TODO       README.BeOS README.'_abimajor'?  demo/*.rexx rexxsaa.h' /**/
         to =   'doc/regina  doc/regina doc/regina doc/regina  doc/regina           bin         include'
         extn_dir = 'add-ons/regina'
         shldir = 'lib'
         share_dir = _tmpdir'/'extn_dir
      End
   When Countstr( 'cygwin', _target ) \= 0 Then
      Do
         from = 'COPYING-LIB BUGS       TODO       README.Unix README.'_abimajor'?  regina.1 demo/*.rexx  rexxsaa.h' /**/
         to =   'doc/regina  doc/regina doc/regina doc/regina  doc/regina           man/man1 share/regina include'
         extn_dir = 'bin'
         shldir = 'bin'
         share_dir = _tmpdir'/share/regina'
      End
   Otherwise
      Do
         from = 'COPYING-LIB BUGS       TODO       README.Unix README.'_abimajor'?  regina.1 demo/*.rexx  rexxsaa.h' /**/
         to =   'doc/regina  doc/regina doc/regina doc/regina  doc/regina           man/man1 share/regina include'
         extn_dir = 'lib'
         shldir = 'lib'
         share_dir = _tmpdir'/share/regina'
      End
End

Do i = 1 To Words(exes)
   fn = Word(exes,i)
   If Stream(fn, 'C', 'QUERY EXISTS') \= '' Then Call copy fn,_tmpdir'/bin'
End

/*
 * Copy test libraries if they exist
 */
fn = _shlpre || 'test1' || _shlpst
If Stream( fn, 'C', 'QUERY EXISTS') \= '' Then Call copy fn,_tmpdir'/'extn_dir
fn = _shlpre || 'test2' || _shlpst
If Stream( fn, 'C', 'QUERY EXISTS') \= '' Then Call copy fn,_tmpdir'/'extn_dir

/*
 * Copy message binary files
 */
Call copy '*.mtb',share_dir

/*
 * Copy static library
 */
fn = _libpre||_libfile||_libpst
If Stream( fn, 'C', 'QUERY EXISTS') \= '' Then Call copy fn,_tmpdir'/lib'
fn = _libpre||_shlfile||_libpst
If Stream( fn, 'C', 'QUERY EXISTS') \= '' Then Call copy fn,_tmpdir'/lib'

/*
 * Copy shared library
 */
fn = _shlpre||_shlfile||_shlpst
If _useabi = 'yes' Then
   Do
      fn_abi_major = fn'.'_abimajor
      fn_abi = fn_abi_major'.'_abiminor
      If Stream( fn_abi, 'C', 'QUERY EXISTS') \= '' Then Call copy fn_abi,_tmpdir'/'shldir
      here = Directory()
      Call Directory _tmpdir'/'shldir
      Address System 'ln -s' fn_abi fn_abi_major
      Address System 'ln -s' fn_abi fn
      Call Directory here
   End
Else
   Do
      If Stream( fn, 'C', 'QUERY EXISTS') \= '' Then Call copy fn,_tmpdir'/'shldir
   End
/*
 * Copy all files in 'from' variable to their appropriate
 * destination in 'to'.
 */
Do i = 1 To Words(from)
  Call copy _srcdir'/'Word(from,i),_tmpdir'/'Word(to,i)
End
Return 0

Copy: Procedure
Parse Arg from, to
Address System 'cp' from to
Return 0

/*
 * Validate the input arguments
 */
ValidOpts: Procedure Expose (vars)
Parse Arg args, validargs, mandatory, valuereq
Do i = 1 To Words(args)
   optarg = Word(args,i)
   Do j = 1 To Words(validargs)
      If Substr(optarg,1,Length(Word(validargs,j))) = Word(validargs,j) Then
         Do
            optval = Strip(Substr(optarg,1+Length(Word(validargs,j))))
            optvar = Word(vars,j)
            Interpret optvar "=optval"
            Leave j
         End
   End
End
error = 0
Do i = 1 To Words(vars)
   Select
      When Translate(Value(Word(vars,i))) = Translate(Word(vars,i)) & Word(mandatory,i) = 1 Then
         Do
            Say 'Mandatory parameter' Word(validargs,i) 'not supplied'
            error = 1
         End
      When Word(valuereq,i) = 1 & Value(Word(vars,i)) = '' Then
         Do
            Say 'No value supplied for parameter' Word(validargs,i)
            error = 1
         End
      Otherwise Nop
   End
End

Return error
