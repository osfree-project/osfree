/*
 * rexxwrap.cmd
 * This program builds an executable program from a Rexx source program.
 *
 * Copyright (C) 1999-2006 Mark Hessling <mark@rexx.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Mark Hessling  mark@rexx.org  http://www.rexx.org/
 *
 */
ver = 'This is Rexx/Wrapper 2.4 - 21 September 2006'
Signal on Halt Name HaltProgram
Trace o
Parse Source os src .
win32_os = 'WIN32 WIN NT WindowsNT Windows95 Windows98'
os2_os = 'OS2 OS/2'
Select
   When Wordpos(os,win32_os) \= 0 Then
      Do
         os = 'WIN32'
         null = '2> nul:'
      End
   When Wordpos(os,os2_os) \= 0 Then
      Do
         os = 'OS2'
         '@echo off'
         null = '2> nul:'
      End
   When os = 'DOS' Then
      Do
         null = '2> nul:'
      End
   When os = 'QNX' Then
      Do
         null = '2> /dev/null'
      End
   When os = 'BeOS' Then
      Do
         os = 'BEOS'
         null = '2> /dev/null'
      End
   Otherwise
      Do
         os = 'UNIX'
         null = '2> /dev/null'
      End
End
_debug = Value( 'REXXWRAPPER_DEBUG', , 'ENVIRONMENT' )
if _debug = '' Then _debug = 0

!zlib_modules = "compress uncompr deflate inflate adler32 zutil trees inftrees inffast crc32"
!des_modules = "rand_key set_key str2key ecb_enc cbc_cksm des_enc"
!my_modules = "rand_key set_key str2key ecb_enc cbc_cksm des_enc"
defines. = ''
defines.regina = '-DUSE_REGINA'
defines.rexximc = '-DUSE_REXXIMC'
defines.rexxtrans = '-DUSE_REXXTRANS'
defines.os2rexx = '-DUSE_OS2REXX'
defines.orexx = '-DUSE_OREXX'
defines.oorexx = '-DUSE_OOREXX'
defines.winrexx = '-DUSE_WINREXX'
defines.quercus = '-DUSE_QUERCUS'
defines.unirexx = '-DUSE_UNIREXX'

implib.regina = 'regina'
implib.rexxtrans = 'rexxtrans'
implib.orexx = 'rexx'
implib.oorexx = 'rexx'
implib.winrexx = 'rxrexx'
implib.quercus = 'wrexx'
implib.unirexx = 'rxx'

compiler.win32 = 'cygwin ming vc wcc borland lcc'
compiler.unix = 'gcc cc c89 egcs xlc wcc'
compiler.qnx = 'cc'
compiler.os2 = 'gcc emxos2 emxdos wcc'
compiler.dos = 'djgpp'
compiler.beos = 'gcc'

ossep.win32 = '\'
ossep.unix = '/'
ossep.qnx = '/'
ossep.os2 = '\'
ossep.dos = '\'
ossep.beos = '/'

ossep = ossep.os

promptfile = 'rexxwrap.prm'

prompts.1 = 'Program Name'
switches.1 = '-program'
variables.1 = '!program'
prompts.2 = 'Rexx filename'
switches.2 = '-rexxfiles'
variables.2 = '!rexxfiles'
prompts.3 = 'Rexx interpreter'
switches.3 = '-interpreter'
variables.3 = '!interpreter'
prompts.4 = 'Compiler'
switches.4 = '-compiler'
variables.4 = '!compiler'
prompts.5 = 'Rexx Interpreter Library'
switches.5 = '-intlib'
variables.5 = '!intlib'
prompts.6 = 'Rexx Interpreter Header File Directory'
switches.6 = '-intincdir'
variables.6 = '!intincdir'
prompts.7 = 'Rexx/Wrapper Source Directory'
switches.7 = '-srcdir'
variables.7 = '!srcdir'
prompts.8 = 'Encrypt source: yes/no'
switches.8 = '-encrypt'
variables.8 = '_encrypt'
prompts.9 = 'Compress source: yes/no'
switches.9 = '-compress'
variables.9 = '_compress'
prompts.10 = 'Use tokenised code rather than source: yes/no'
switches.10 = '-token'
variables.10 = '_token'
prompts.11 = 'Call Rexx program as subroutine: yes/no'
switches.11 = '-subroutine'
variables.11 = '_subroutine'
Select
   When os = 'WIN32' Then
      Do
         prompts.12 = 'Do you want to build a GUI version: yes/no'
         switches.12 = '-win32gui'
         variables.12 = '_win32gui'
         prompts.13 = 'Do you want a console window if required: yes/no'
         switches.13 = '-guiconsole'
         dep.13 = 12
         variables.13 = '_guiconsole'
         prompts.14 = 'Do you want to specify versioning information and add an icon to the executable: yes/no'
         switches.14 = '-resource'
         variables.14 = '_resource'
         prompts.15 = 'Program version. eg 2.0.1'
         switches.15 = '-version'
         variables.15 = '_version'
         dep.15 = 14
         prompts.16 = 'Program description'
         switches.16 = '-desc'
         variables.16 = '_desc'
         dep.16 = 14
         prompts.17 = 'License'
         switches.17 = '-license'
         variables.17 = '_license'
         dep.17 = 14
         prompts.18 = 'Icon filename'
         switches.18 = '-icon'
         variables.18 = '_icon'
         dep.18 = 14
         vars = 18
      End
   When os = 'OS2' Then
      Do
         prompts.12 = 'Do you want to build a GUI version: yes/no'
         switches.12 = '-win32gui'
         variables.12 = '_win32gui'
         prompts.13 = 'Do you want to add an icon to the executable: yes/no'
         switches.13 = '-resource'
         variables.13 = '_resource'
         dep.14 = 13
         prompts.14 = 'Icon filename'
         switches.14 = '-icon'
         variables.14 = '_icon'
         vars = 14
      End
   Otherwise
      Do
         vars = 11
      End
End

prompts.0 = vars
switches.0 = vars
variables.0 = vars

_encrypt = 'no'
_compress = 'no'
_token = 'no'
_subroutine = 'no'
_win32gui = 'no'
_guiconsole = 'no'
!defines = ''
!sys_defs=''
!mh_extra_libs=''

/*
 * Check if we have rexxwrapper-config script. This saves us the bother
 * of asking the user where Rexx/Wrapper source code is
 * Can't use '...with output...' as only Regina support this :-(
 */
tmpfile = 'rexxwrap.tmp'
Call Stream tmpfile, 'C', 'CLOSE'
'rexxwrapper-config --sourcedir >' tmpfile null
If rc = 0 Then
   Do
      rr_srcdir = Linein( tmpfile )
      have_rexxwrapper_config = 1
   End
Else
   /*
    * Check if REXXWRAPPER_HOME is set.  This allows us to know
    * the location of RexxWrapper files are.
    */
   Do
      rexxwrapper_home = Value( 'REXXWRAPPER_HOME', ,'ENVIRONMENT' )
      If rexxwrapper_home \= '' Then
         Do
            rr_srcdir = rexxwrapper_home
            have_rexxwrapper_config = 1
         End
      Else have_rexxwrapper_config = 0
   End

/*
 * Check if we have rexxtrans-config script. This saves us the bother
 * of asking the user where Rexx/Trans library and header files are.
 * These values are used for -intlib and -intincdir switches respectively.
 * Can't use '...with output...' as only Regina support this :-(
 */
Call Stream tmpfile, 'C', 'CLOSE'
'rexxtrans-config --libs --prefix >' tmpfile null
If rc = 0 Then
   Do
      rexxtrans_intlib = Linein( tmpfile )
      rexxtrans_intincdir = Linein( tmpfile )'/include'
      have_rexxtrans_config = 1
   End
Else
   /*
    * Check if REXXTRANS_HOME is set.  This allows us to setup
    * the location of RexxTrans files in case RexxTrans is selected.
    */
   Do
      rexx_home = Value( 'REXXTRANS_HOME', ,'ENVIRONMENT' )
      If rexx_home \= '' Then
         Do
            rexxtrans_intlib = rexx_home'\rexxtrans.lib'
            rexxtrans_intincdir = rexx_home
            have_rexxtrans_config = 1
         End
      Else have_rexxtrans_config = 0
   End

/*
 * Check if we have regina-config script. This saves us the bother
 * of asking the user where Regina library and header files are.
 * These values are used for -intlib and -intincdir switches respectively.
 * Can't use '...with output...' as only Regina support this :-(
 */
Call Stream tmpfile, 'C', 'CLOSE'
'regina-config --libs --prefix >' tmpfile null
If rc = 0 Then
   Do
      regina_intlib = Linein( tmpfile )
      regina_intincdir = Linein( tmpfile )'/include'
      have_regina_config = 1
   End
Else have_regina_config = 0

/*
 * Check if we have oorexx-config script. This saves us the bother
 * of asking the user where ooRexx library and header files are.
 * These values are used for -intlib and -intincdir switches respectively.
 * Can't use '...with output...' as only Regina support this :-(
 */
Call Stream tmpfile, 'C', 'CLOSE'
'oorexx-config --libs --prefix >' tmpfile null
If rc = 0 Then
   Do
      oorexx_intlib = Linein( tmpfile )
      oorexx_intincdir = Linein( tmpfile )'/include'
      have_oorexx_config = 1
   End
Else
   /*
    * Check if on WIN32 and REXX_HOME is set.  This allows us to setup
    * the location of ooRexx files in case ooRexx is selected.
    */
   Do
      rexx_home = Value( 'REXX_HOME', ,'ENVIRONMENT' )
      If rexx_home \= '' Then
         Do
            oorexx_intlib = rexx_home'\api\rexx.lib'
            oorexx_intincdir = rexx_home'\api'
            have_oorexx_config = 1
         End
      Else have_oorexx_config = 0
   End

Say ''
Say ver
Say Copies( '-', Length( ver ) )
Select
   When Words(Arg(1)) = 0 Then
      /*
       * Run with interactive prompting...
       */
      Do
         /*
          * Read the saved prompt file (if there is one)
          * and set the default values to those saved in the file.
          */
         default. = ''
         If Stream(promptfile,'C','QUERY EXISTS') \= '' Then
            Do While Lines(promptfile) > 0
               line = Linein(promptfile)
               Interpret line
            End
         Call Stream promptfile, 'C', 'CLOSE'
         /*
          * Now ask the user for the new values or confirm the old ones...
          */
         Do i = 1 To prompts.0
            prompt = prompts.i
            switch = switches.i
            variable = variables.i
            ask = 'yes'
            line = ''
            If Datatype( dep.i ) = 'NUM' Then
               Do
                  dep_idx = dep.i
                  dep_var = variables.dep_idx
                  ask = Value( dep_var )
               End
            /*
             * Check for some exclusions
             */
            If variables.i = '_resource' Then
               Do
                  Select
                     When !compiler = 'vc' | !compiler = 'emxos2' Then Nop
                     When !compiler = 'gcc' & os = 'OS2' Then Nop
                     Otherwise
                        Do
                           ask = 'no'
                           _resource = 'no'
                        End
                  End
               End
            If Translate(!interpreter) = 'OS2REXX' & (Translate(!compiler) = 'EMXOS2' | Translate(!compiler) = 'WCC' | Translate(!compiler) = 'GCC') & ( variables.i = '!intlib' | variables.i = '!intincdir' ) Then
               Do
                  !intlib = ''
                  !intincdir = ''
                  ask = 'no'
               End
            If (Translate(!compiler) = 'MING' | Translate(!compiler) = 'CYGWIN' | Translate(!compiler) = 'LCC') & variables.i = '!intlib' Then ask = 'no'
            /*
             * Don't ask the user for -srcdir if rexxwrapper-config is found
             */
            If switches.i = '-srcdir' & have_rexxwrapper_config Then
               Do
                  !srcdir = rr_srcdir
                  line = !srcdir
                  ask = 'no'
               End
            /*
             * If the interpreter is Rexx/Trans or Regina or ooRexx and the
             * associated *-config file exists, use the values supplied
             * by the *-config script
              Also if REXX_HOME is set on WIN32 for ooRexx
             */
            Select
               When switches.i = '-intlib' & have_regina_config & !interpreter = 'regina' Then
                  Do
                     !intlib = regina_intlib
                     line = !intlib
                     ask = 'no'
                  End
               When switches.i = '-intlib' & have_rexxtrans_config & !interpreter = 'rexxtrans' Then
                  Do
                     !intlib = rexxtrans_intlib
                     line = !intlib
                     ask = 'no'
                  End
               When switches.i = '-intlib' & have_oorexx_config & !interpreter = 'oorexx' Then
                  Do
                     !intlib = oorexx_intlib
                     line = !intlib
                     ask = 'no'
                  End
               When switches.i = '-intincdir' & have_regina_config & !interpreter = 'regina' Then
                  Do
                     !intincdir = regina_intincdir
                     line = !intincdir
                     ask = 'no'
                  End
               When switches.i = '-intincdir' & have_rexxtrans_config & !interpreter = 'rexxtrans' Then
                  Do
                     !intincdir = rexxtrans_intincdir
                     line = !intincdir
                     ask = 'no'
                  End
               When switches.i = '-intincdir' & have_oorexx_config & !interpreter = 'oorexx' Then
                  Do
                     !intincdir = oorexx_intincdir
                     line = !intincdir
                     ask = 'no'
                  End
               Otherwise Nop
            End
            /*
             * We need to ask the user for the value now
             */
            If ask = 'yes' Then
               Do
                  Call Charout ,prompt || '(' || switch || ') [' || default.i || '] '
                  Parse Pull line
                  If line = '' Then line = default.i
                  If line = '-' Then line = ''
                  Interpret variable '= line'
               End
            default.i = 'default.' || i '=' "'"line"'"
         End
      End
   When Left(Arg(1),2) = '-d' Then
      /*
       * Re-run with existing default values or from specified parameter file
       */
      Do
         If Length( Arg(1) ) > 2 Then promptfile = Substr( Word( Arg(1), 1 ), 3 )
         If Stream(promptfile,'C','QUERY EXISTS') \= '' Then
            Do
               Do While Lines(promptfile) > 0
                  line = Linein(promptfile)
                  Interpret line
               End
               Call Stream promptfile, 'C', 'CLOSE'
               Do i = 1 To prompts.0
                  save = 'yes'
                  If Datatype( dep.i ) = 'NUM' Then
                     Do
                        dep_idx = dep.i
                        dep_var = variables.dep_idx
                        save = Value( dep_var )
                     End
                  /*
                   * Check for some exclusions
                   */
                  If variables.i = '_resource' Then
                     Do
                        Select
                           When !compiler = 'vc' | !compiler = 'emxos2' Then Nop
                           When !compiler = 'gcc' & os = 'OS2' Then Nop
                           Otherwise
                              Do
                                 _resource = 'no'
                                 save = 'no'
                              End
                        End
                     End
                  If Translate(!interpreter) = 'OS2REXX' & (Translate(!compiler) = 'EMXOS2' | Translate(!compiler) = 'WCC' | Translate(!compiler) = 'GCC') & ( variables.i = '!intlib' | variables.i = '!intincdir' ) Then save = 'no'
                  If ( Translate(!compiler) = 'MING' | Translate(!compiler) = 'CYGWIN' | Translate(!compiler) = 'LCC') & variables.i = '!intlib' Then save = 'no'
                  variable = variables.i
                  line = default.i
                  Interpret variable '= line'
                  If save = 'yes' Then
                     Do
                        Say prompts.i || '(' || switches.i || ') [' || line || '] '
                     End
               End
            End
         Else
            Do
               Call Abort 'Cannot use -d option as' promptfile 'cannot be found'
            End
      End
   When Arg(1) = '-help' | Arg(1) = '--help' | Arg(1) = '-h' | Arg(1) = '-?' Then
      Do
         Call Usage
         Exit 1
      End
   When Arg(1) = '-syntax' | Arg(1) = '--syntax' Then
      Do
         Call ShowSyntax
         Exit 1
      End
   Otherwise
      /*
       * All parameters expected on command line
       */
      If src = 'SUBROUTINE' Then
         Do
            Do i = 1 To Arg()
               argv = Arg(i)
               Call ProcessArg argv
            End
         End
      Else
         Do
            Do i = 1 To Words(Arg(1))
               argv = Word(Arg(1),i)
               Call ProcessArg argv
            End
         End

      /*
       * If we have rexxwrapper-config use !srcdir from it
       */
      If !srcdir = '!SRCDIR' & have_rexxwrapper_config Then !srcdir = rr_srcdir
      /*
       * If the interpreter is Rexx/Trans or Regina or ooRexx and the
       * associated *-config file exists, use the values supplied
       * by the *-config script
       */
      If !intlib = '!INTLIB' & have_regina_config & !interpreter = 'regina' Then !intlib = regina_intlib
      If !intlib = '!INTLIB' & have_rexxtrans_config & !interpreter = 'rexxtrans' Then !intlib = rexxtrans_intlib
      If !intlib = '!INTLIB' & have_oorexx_config & !interpreter = 'oorexx' Then !intlib = oorexx_intlib
      If !intincdir = '!INTINCDIR' & have_regina_config & !interpreter = 'regina' Then !intincdir = regina_intincdir
      If !intincdir = '!INTINCDIR' & have_rexxtrans_config & !interpreter = 'rexxtrans' Then !intincdir = rexxtrans_intincdir
      If !intincdir = '!INTINCDIR' & have_oorexx_config & !interpreter = 'oorexx' Then !intincdir = oorexx_intincdir
   End

!compiler = Translate(!compiler)
!lowerinterpreter = !interpreter
!interpreter = Translate(!interpreter)

extralink_intbuild. = ''
extralink_rexxtok. = ''
extralink_program. = ''
dos_rsp_file = 0
need_getopt. = 0
getopt_obj = ''
borland_rsp_file = 0
Select
   When os = 'WIN32' Then
      Do
         If _debug > 0 Then opt = '-g -DREXXWRAPPER_DEBUG='_debug
         Else opt = ''
         cflags. = '-DWIN32 -c' opt
         ldflags_intbuild. = opt '-ointbuild'
         ldflags_rexxtok. = opt '-orexxtok'
         ldflags_program. = opt '-o' || !program
         exe_ext = '.exe'
         /* cygwin */
         cc.cygwin = 'gcc'
         obj.cygwin = '.o'
         dirsep.cygwin = '/'
         link.cygwin = cc.cygwin
         cflags.cygwin = '-c -DUNIX' opt
         /* ming */
         cc.ming = 'gcc'
         obj.ming = '.o'
         dirsep.ming = '\'
         link.ming = cc.ming
         /* lcc */
         cc.lcc = 'lcc'
         obj.lcc = '.obj'
         dirsep.lcc = '\'
         link.lcc = 'lcclnk'
         If _win32gui = 'yes' Then subsys =  'windows'
         Else subsys = 'console'
         ldflags_intbuild.lcc = '-o intbuild'exe_ext
         ldflags_rexxtok.lcc = '-o rexxtok'exe_ext
         ldflags_program.lcc = '-o' !program||exe_ext '-subsystem' subsys
         /* vc */
         cc.vc = 'cl'
         obj.vc = '.obj'
         dirsep.vc = '\'
         If _debug > 0 Then
            Do
               vcopt = '-Od -Zi -DREXXWRAPPER_DEBUG='_debug
               vclink = '-debug'
            End
         Else
            Do
               vcopt = '-Ox'
               vclink = ''
            End
         If _win32gui = 'yes' Then subsys =  '-subsystem:windows'
         Else subsys = ''
         cflags.vc = '-DWIN32 -c' vcopt '-nologo'
         link.vc = 'link'
         ldflags_intbuild.vc = vclink '-nologo -out:intbuild'exe_ext
         ldflags_rexxtok.vc = vclink '-nologo -out:rexxtok'exe_ext
         ldflags_program.vc = vclink '-nologo -out:' || !program || exe_ext 'user32.lib advapi32.lib wsock32.lib' subsys
         /* borland */
         cc.borland = 'bcc32'
         obj.borland = '.obj'
         dirsep.borland = '\'
         cflags.borland = '-DWIN32 -q -w-aus -w-use -w-pro -w-use -c' opt
         link.borland = 'ilink32'
         ldflags_intbuild.borland = '-q -out:intbuild'exe_ext
         ldflags_rexxtok.borland = '-q -out:rexxtok'exe_ext
         ldflags_program.borland = '-q'
         /* wcc */
         cc.wcc = 'wcc386'
         obj.wcc = '.obj'
         dirsep.wcc = '\'
         If _debug > 0 Then wccopt = '-g -DREXXWRAPPER_DEBUG='_debug Else wccopt = '-oneatx'
         cflags.wcc = '-DWIN32 -zz -ei -zq -mf' opt
         link.wcc = 'wlink'
         ldflags_intbuild.wcc = 'option quiet option stack=64k system nt name intbuild'exe_ext 'file'
         ldflags_rexxtok.wcc = 'option quiet option stack=64k system nt name rexxtok'exe_ext 'file'
         ldflags_program.wcc = 'option quiet option stack=64k option heapsize=40960 system nt name' !program || exe_ext
         run_configure = 0
         need_getopt. = 1
         file_delete = 'del /q'
         directory_delete = 'rd'
         file_copy = 'copy'
      End
   When os = 'OS2' Then
      Do
         If _debug > 0 Then opt = '-g -DREXXWRAPPER_DEBUG='_debug
         Else opt = ''  /* opt = '-O' -- incorrect optimization option (valerius) */
         /* gcc */
         cc.gcc = 'gcc'
         obj.gcc = '.o'
         exe_ext = '.exe'
         dirsep.gcc = '\'
         cflags.gcc = '-Zcrtdll -Zomf -Zmt -mprobe -D__OS2__ -DOS2 -c -Wall' opt
         ldflags_intbuild.gcc = opt '-Zcrtdll -Zomf -Zmt -mprobe -o intbuild'exe_ext
         ldflags_rexxtok.gcc = opt '-Zcrtdll -Zomf -Zmt -mprobe -o rexxtok'exe_ext
         ldflags_program.gcc = opt '-Zcrtdll -Zomf -Zmt -mprobe -o' !program||exe_ext
         extralink_intbuild.gcc = '' /*'emxbind -q intbuild'*/
         extralink_rexxtok.gcc = '' /*'emxbind -q rexxtok'*/
         extralink_program.gcc = ''
         link.gcc = cc.!compiler
         /* emxos2 */
         cc.emxos2 = 'gcc'
         obj.emxos2 = '.obj'
         exe_ext = '.exe'
         dirsep.emxos2 = '\'
         cflags.emxos2 = '-Zcrtdll -Zomf -Zmt -mprobe -D__OS2__ -DOS2 -c -Wall' opt
         ldflags_intbuild.emxos2 = opt '-Zcrtdll -Zomf -Zmt -mprobe -o intbuild'exe_ext
         ldflags_rexxtok.emxos2 = opt '-Zcrtdll -Zomf -Zmt -mprobe -o rexxtok'exe_ext
         ldflags_program.emxos2 = opt '-Zcrtdll -Zomf -Zmt -mprobe -o' !program||exe_ext
         extralink_intbuild.emxos2 = '' /*'emxbind -q intbuild'*/
         extralink_rexxtok.emxos2 = '' /*'emxbind -q rexxtok'*/
         extralink_program.emxos2 = ''
         link.emxos2 = cc.!compiler
         /* emxdos */
         cc.emxdos = 'gcc'
         obj.emxdos = '.o'
         dirsep.emxdos = '\'
         cflags.emxdos = ' -DDOS -c -Wall' opt
         ldflags_intbuild.emxdos = opt '-o intbuild'
         ldflags_rexxtok.emxdos = opt '-o rexxtok'
         ldflags_program.emxdos = opt '-o' !program
         extralink_intbuild.emxdos = 'emxbind -q intbuild -acm'
         extralink_rexxtok.emxdos = 'emxbind -q rexxtok -acm'
         extralink_program.emxdos = 'emxbind -q' !program '-acm'
         link.emxdos = cc.!compiler
         /* wcc */
         cc.wcc = 'wcc386'
         obj.wcc = '.obj'
         dirsep.wcc = '\'
         If _debug > 0 Then wccopt = '-g -DREXXWRAPPER_DEBUG='_debug Else wccopt = '-oneatx'
         cflags.wcc = '-DOS2 -D__OS2__ /4s /wx /zq /mf' opt
         link.wcc = 'wlink'
         If _win32gui = 'yes' Then subsys = 'os2v2_pm'
         Else subsys = 'os2v2'
         ldflags_intbuild.wcc = 'option quiet option stack=64k system os2v2 name intbuild'exe_ext 'file'
         ldflags_rexxtok.wcc = 'option quiet option stack=64k system' subsys 'name rexxtok' exe_ext 'file'
         ldflags_program.wcc = 'option quiet option stack=64k option heapsize=40960 system' subsys 'name' !program || exe_ext
         run_configure = 0
         need_getopt. = 1
         link.wcc = 'wlink'
         /* common */
         directory_delete = 'rd'
         file_delete = 'del /n '
         file_copy = 'copy'
      End
   When os = 'DOS' Then
      Do
         If _debug > 0 Then opt = '-g -DREXXWRAPPER_DEBUG='_debug
         Else opt = '-O'
         cc.djgpp = 'gcc'
         obj.djgpp = '.o'
         exe_ext = '.exe'
         dirsep.djgpp = '\'
         cflags. = '-DDOS -c' opt
         link. = cc.!compiler
         ldflags_intbuild.djgpp = opt '-o intbuild'
         ldflags_rexxtok.djgpp = opt '-o rexxtok'
         ldflags_program.djgpp = opt '-o' !program
         extralink_intbuild.djgpp = 'coff2exe intbuild'
         extralink_rexxtok.djgpp = 'coff2exe rexxtok'
         extralink_program.djgpp = 'coff2exe' !program
         run_configure = 0
         dos_rsp_file = 1
         need_getopt. = 1
         directory_delete = 'rd'
         file_delete = 'del'
         file_copy = 'copy'
      End
   When os = 'QNX' Then
      Do
         If _debug > 0 Then opt = '-g -DREXXWRAPPER_DEBUG='_debug
         Else opt = '-Otax'
         cc.cc = 'cc'
         obj.cc = '.o'
         exe_ext = ''
         dirsep.cc = '/'
         cflags.cc = '-c' opt
         link.cc = cc.!compiler
         ldflags_intbuild. = '-N0x20000 -Q -ointbuild' opt
         ldflags_rexxtok. = '-N0x20000 -Q -orexxtok' opt
         ldflags_program. = '-N0x20000  -Q -o' || !program opt
         run_configure = 1
         directory_delete = 'rm -f'
         file_delete = 'rm -f'
         file_copy = 'cp'
      End
   When os = 'UNIX' Then
      Do
         If _debug > 0 Then opt = '-g -DREXXWRAPPER_DEBUG='_debug
         Else opt = '-Otax'
         If _debug > 0 Then wccopt = '-g -DREXXWRAPPER_DEBUG='_debug Else wccopt = '-oneatx'
         /* wcc */
         cc.wcc = 'wcc386'
         obj.wcc = '.o'
         exe_ext = '.exe'
         dirsep.wcc = '/'
         watcom = value('WATCOM',, 'ENVIRONMENT')
         cflags.wcc = '-dUNIX -dLINUX -D__LINUX__ -4s -wx -zq -mf -i='watcom'/lh' opt
         link.wcc = 'wlink'
         subsys = 'linux'
         ldflags_intbuild.wcc = 'option quiet option stack=64k system linux name intbuild'exe_ext 'file'
         ldflags_rexxtok.wcc = 'option quiet option stack=64k system' subsys 'name rexxtok' exe_ext 'file'
         ldflags_program.wcc = 'option quiet option stack=64k system' subsys 'name' !program || exe_ext
         run_configure = 0
         need_getopt. = 1
         link.wcc = 'wlink'
         directory_delete = 'rm -f'
         file_delete = 'rm -f'
         file_copy = 'cp'
      End
   Otherwise
      Do
         If _debug > 0 Then opt = '-g -DREXXWRAPPER_DEBUG='_debug
         Else opt = '-O'
         cc.gcc = 'gcc'
         cc.egcs = 'egcs'
         cc.cc = 'cc'
         cc.xlc = 'xlc'
         cc.c89 = 'c89'
         obj. = '.o'
         exe_ext = ''
         dirsep. = '/'
         cflags. = '-c -I.' opt
         link. = cc.!compiler
         ldflags_intbuild. = opt '-ointbuild'
         ldflags_rexxtok. = opt '-orexxtok'
         ldflags_program. = opt '-o' || !program
         run_configure = 1
         directory_delete = 'rm -f'
         file_delete = 'rm -f'
         file_copy = 'cp'
      End
End

If Words(Arg(1)) = 0 Then
   /*
    * If running in interactive mode, save the unvalidated default values...
    */
   Do
      Call OpenWriteReplace promptfile
      Do i = 1 To prompts.0
         If Datatype( dep.i ) = 'NUM' Then
            Do
               dep_idx = dep.i
               dep_var = variables.dep_idx
            End
         Call Lineout promptfile, default.i '/* ['switches.i']' prompts.i '*/'
      End
      Call Stream promptfile, 'C', 'CLOSE'
   End

/*
 * Validate the parameters...
 */
If !program = '!PROGRAM' | !program = '' Then Call Abort 'no output program name specified with -program= switch'
If !rexxfiles = '!REXXFILES' | !rexxfiles = '' Then Call Abort 'no Rexx programs specified with -rexxfiles= switch'
/* get absolute file name */
!rexxfiles = Stream(!rexxfiles,'C','QUERY EXISTS')
If !rexxfiles = '' Then Call Abort 'could not find Rexx program specified with -rexxfiles= switch'
If !compiler = '!COMPILER' | !compiler = '' Then Call Abort 'no compiler specified with -compiler= switch'
If Wordpos(!compiler,Translate(compiler.os)) = 0 Then Call Abort 'unsupported compiler specified with -compiler= switch; must be one of:' compiler.os
If !interpreter = '!INTERPRETER' | !interpreter = '' Then Call Abort 'no Rexx interpreter specified with -interpreter= switch'
If defines.!interpreter = '' Then Call Abort 'invalid Rexx interpreter specified:' !interpreter
/*
  Set !dirsep here before its used...
  */
!dirsep = dirsep.!compiler
/*
 * Check if !intlib and/or !intincdir need to be validated...
 */
check_intlib = 'yes'
check_intincdir = 'yes'
If !interpreter = 'OS2REXX' & (!compiler = 'EMXOS2' | !compiler = 'WCC' | !compiler = 'GCC') Then
   Do
      check_intlib = 'no'
      check_intincdir = 'no'
   End
If !compiler = 'MING' | !compiler = 'CYGWIN' | !compiler = 'LCC' Then check_intlib = 'no'
If have_rexxtrans_config | have_regina_config | have_oorexx_config Then
   Do
      check_intlib = 'no'
      check_intincdir = 'no'
   End
/*
 * Check !intlib if required
 */
If check_intlib = 'yes' Then
   Do
      If !intlib = '!INTLIB' | !intlib = '' Then Call Abort 'no Rexx interpreter library specified with -intlib= switch'
      If Stream(!intlib,'C','QUERY EXISTS') = '' Then Call Abort 'could not find Rexx interpreter library specified with -intlib= switch'
      /* surround !intlib with double quotes */
      if os<>'WIN32' then
      do
        !intlib = quote( !intlib )
      end
   End
/*
 * Check !intincdir if required
 */
If check_intincdir = 'yes' Then
   Do
      If !intincdir = '!INTINCDIR' | !intincdir = '' Then Call Abort 'no Rexx interpreter include directory specified with -intincdir= switch'
      /* surround !intincdir with double quotes */
      !intincdir = quote( !intincdir )
   End
If !srcdir = '!SRCDIR' | !srcdir = '' Then Call Abort 'no source directory specified with -srcdir= switch'
If Stream(!srcdir||!dirsep||'rexxtok.c','C','QUERY EXISTS') = '' Then Call Abort "source directory" !srcdir "does not exist; could not find:'"!srcdir||!dirsep||"rexxtok.c'"
If _encrypt = '' | Abbrev("yes",_encrypt,1) | Abbrev("no",_encrypt,1) Then Nop
Else Call Abort "invalid response for -encrypt switch; must be a valid abbreviation of 'yes' or 'no'"
If _compress = '' | Abbrev("yes",_compress,1) | Abbrev("no",_compress,1) Then Nop
Else Call Abort "invalid response for -compress switch; must be a valid abbreviation of 'yes' or 'no'"
If _token = '' | Abbrev("yes",_token,1) | Abbrev("no",_token,1) Then Nop
Else Call Abort "invalid response for -token switch; must be a valid abbreviation of 'yes' or 'no'"
If _subroutine = '' | Abbrev("yes",_subroutine,1) | Abbrev("no",_subroutine,1) Then Nop
Else Call Abort "invalid response for -subroutine switch; must be a valid abbreviation of 'yes' or 'no'"
If _win32gui = '' | Abbrev("yes",_win32gui,1) | Abbrev("no",_win32gui,1) Then Nop
Else Call Abort "invalid response for -win32gui switch; must be a valid abbreviation of 'yes' or 'no'"
If _guiconsole = '' | Abbrev("yes",_guiconsole,1) | Abbrev("no",_guiconsole,1) Then Nop
Else Call Abort "invalid response for -guiconsole switch; must be a valid abbreviation of 'yes' or 'no'"

If !interpreter = 'rexxtrans' & Abbrev("yes",_token,1) Then Call Abort "invalid combination of switches; -token, -interpreter=rexxtrans"
If need_getopt.!compiler Then getopt_obj = 'getopt'obj.!compiler

If Right(!srcdir,1) = !dirsep Then !srcdir = Truncate(!srcdir,'T',!dirsep) /* jap */

/*
 * Determine absolute directory for !srcdir
 */
here = Directory()
src = Directory( !srcdir )
If src \= '' Then !srcdir = src
Call Directory here
/*
 * We start generating temporary files from here on, so make/change to the temporary directory
 * so all intermediate files are generated in a separate directory
 */
/*
 * Change to and or make the temporary directory...
 */
_dir = /* '_' */ !program'.tmp'
If Directory( _dir ) = '' Then
   Do
      'mkdir' _dir
      If rc \= 0 Then Call Abort 'Unable to make temporary directory:' _dir
      If Directory( _dir ) = '' Then Call Abort 'Unable to change directory to:' _dir
   End

If run_configure Then
   Do
      /*
       * Run the configure script to create config.h and
       * other system specifics in the file 'configur'
       */
      Say 'Running configure script...'
      Call Value 'CC',cc.!compiler,'ENVIRONMENT'
      !srcdir || '/' || 'configur --with-rexx=' || !lowerinterpreter '--quiet' /* can we quote( srcdir || configur ) ? */
      If rc \= 0 Then Call Abort "error running configure script"
      !defines = !defines '-DHAVE_CONFIG_H'
      Do While Lines('./defined_values') > 0
         line = Linein('./defined_values')
         Interpret line
      End
   End

!includes = '-I'quote( !srcdir ) '-I'quote( !srcdir||!dirsep'common' )
If !intincdir \= '' Then !includes = !includes '-I'!intincdir

!defines = !defines !sys_defs defines.!interpreter
!cc = cc.!compiler
!obj = obj.!compiler
!link = link.!compiler
!cflags = cflags.!compiler
!ldflags_intbuild = ldflags_intbuild.!compiler
!ldflags_rexxtok = ldflags_rexxtok.!compiler
!ldflags_program = ldflags_program.!compiler
!extralink_intbuild = extralink_intbuild.!compiler
!extralink_rexxtok = extralink_rexxtok.!compiler
!extralink_program = extralink_program.!compiler

!template = !srcdir || !dirsep || 'rexxwrap.c'
/*
 * For those compilers (on WIN32) that have to have their import libraries
 * generated, generate them here.
 */
Select
   When !compiler = 'MING' | !compiler = 'CYGWIN' Then
      Do
         Call Stream 'int.def', 'C', 'OPEN WRITE REPLACE'
         If !interpreter = 'REGINA' Then
            Do
               ats_rexxstart = ''
               ats_rexxfreememory = ''
               ats_rexxregisterexitexe = ''
            End
         Else
            Do
               ats_rexxstart = '@36'
               ats_rexxfreememory = '@4'
               ats_rexxregisterexitexe = '@12'
            End
         Call Lineout 'int.def','EXPORTS RexxStart'ats_rexxstart
         Call Lineout 'int.def','EXPORTS RexxFreeMemory'ats_rexxfreememory
         Call Lineout 'int.def','EXPORTS RexxRegisterExitExe'ats_rexxregisterexitexe
         Call Stream 'int.def', 'C', 'CLOSE'
         cmd = 'dlltool -A -k --input-def int.def --dllname' implib.!interpreter'.dll --output-lib libmyrexx.a'
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
         !intlib = 'libmyrexx.a'
      End
   When !compiler = 'LCC' Then
      Do
         Call Stream 'int.def', 'C', 'OPEN WRITE REPLACE'
         If !interpreter = 'REGINA' Then
            Do
               ats_rexxstart = ''
               ats_rexxfreememory = ''
               ats_rexxregisterexitexe = ''
            End
         Else
            Do
               ats_rexxstart = '@36'
               ats_rexxfreememory = '@4'
               ats_rexxregisterexitexe = '@12'
            End
         Call Lineout 'int.def',implib.!interpreter'.dll'
         Call Lineout 'int.def','_RexxStart'ats_rexxstart
         Call Lineout 'int.def','_RexxFreeMemory'ats_rexxfreememory
         Call Lineout 'int.def','_RexxRegisterExitExe'ats_rexxregisterexitexe
         Call Stream 'int.def', 'C', 'CLOSE'
         cmd = 'buildlib int.def myrexx.lib'
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
         !intlib = 'myrexx.lib'
      End
   When !compiler = 'BORLAND' Then
      Do
         borland_rsp_file = 1
         !mh_extra_libs = 'import32.lib cw32.lib'
         /*
          * If we are using the Borland compiler AND Object Rexx, then we need
          * to convert the Object Rexx import library to Borland format
          */
         If !interpreter = 'OREXX' | !interpreter = 'OOREXX' Then
            Do
               'coff2omf -q -lib:st' !intlib 'rexx_bor.lib' /* .\ */
               If rc \= 0 Then Call Abort 'Failure when converting' !intlib 'to Borland format'
               !intlib = 'rexx_bor.lib'
            End
      End
   Otherwise Nop
End

If Abbrev("yes",_token,1) Then !defines = !defines '-DUSE_TOKENIZED'
If Abbrev("yes",_subroutine,1) Then !defines = !defines '-DREXXWRAPPER_SUBROUTINE'

If Abbrev( "yes", _win32gui, 1 ) Then
   Do
      !defines = !defines '-DRUN_AS_GUI'
      If Abbrev("yes", _guiconsole, 1 ) Then !defines = !defines '-DWITH_GUI_CONSOLE'
      !includes = !includes '-I'quote( !srcdir || !dirsep || 'common' )
   End

!des_objs=''
!wcc_des_objs = ''
If Abbrev("yes",_encrypt,1) Then
   Do
      !defines = !defines '-DUSE_DES'
      !includes = !includes '-I'quote( !srcdir || !dirsep || 'des' )
      Call BuildDesModules
      Do i = 1 To Words(!des_modules)
         !des_objs = !des_objs Word(!des_modules,i) || !obj
         !wcc_des_objs = !wcc_des_objs 'file' Word(!des_modules,i) || !obj
      End
   End
if !compiler = 'WCC' Then !des_objs = !wcc_des_objs

!zlib_objs = ''
!wcc_zlib_objs = ''
If Abbrev("yes",_compress,1) Then
   Do
      !defines = !defines '-DUSE_ZLIB'
      !includes = !includes '-I'quote( !srcdir || !dirsep || 'zlib' )
      Call BuildZlibModules
      Do i = 1 To Words(!zlib_modules)
         !zlib_objs = !zlib_objs Word(!zlib_modules,i) || !obj
         !wcc_zlib_objs = !wcc_zlib_objs 'file' Word(!zlib_modules,i) || !obj
      End
   End

If !compiler = 'WCC' Then !zlib_objs = !wcc_zlib_objs
If !compiler = 'WCC' Then !intlib = 'library' !intlib

If Abbrev("yes",_token,1) Then
   Do
      Call BuildRexxtok
      Say 'Running rexxtok to produce' !program || '.tok...'
      cmd = '.'ossep'rexxtok' '-t'!program'.tok' '-s'!rexxfiles
      If _debug > 0 Then Say '   <<debug>>' cmd
      cmd
      If rc \= 0 Then Call Abort 'error running rexxtok'
   End

Call BuildIntBuild
Say 'Running intbuild to produce' !program || '.c...'
If Abbrev("yes",_token,1) Then
   cmd = '.'ossep'intbuild'exe_ext quote( !template ) !program !program'.tok'
Else
   cmd = '.'ossep'intbuild'exe_ext quote( !template ) !program !rexxfiles
If _debug > 0 Then Say '   <<debug>>' cmd
cmd
If rc \= 0 Then Call Abort 'error running intbuild'
Say 'Compiling' !program || '.c...'
if os \= 'UNIX' then
   cmd = !cc !cflags !defines !includes '.' || !dirsep || !program || '.c'
else
   cmd = !cc !cflags !defines !includes !program || '.c'
If _debug > 0 Then Say '   <<debug>>' cmd
cmd
If rc \= 0 Then Call Abort 'error compiling' !program || '.c'
Call BuildGetargv0
Say 'Linking' !program
Select
   When dos_rsp_file Then
      Do
         Call OpenWriteReplace 'tmp.rsp'
         Call Lineout 'tmp.rsp',!program || !obj !zlib_objs !des_objs 'getargv0' || !obj
         Call Stream 'tmp.rsp', 'C', 'CLOSE'
         cmd = !link !ldflags_program '@tmp.rsp' !mh_extra_libs !intlib
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
   When borland_rsp_file Then
      Do
         Call OpenWriteReplace 'tmp.rsp'
         Call Lineout 'tmp.rsp','c0x32.obj' !program || !obj !zlib_objs !des_objs 'getargv0' || !obj
         Call Lineout 'tmp.rsp',!program
         Call Lineout 'tmp.rsp',''
         Call Lineout 'tmp.rsp',!mh_extra_libs !intlib
         Call Stream 'tmp.rsp', 'C', 'CLOSE'
         cmd = !link !ldflags_program '@tmp.rsp'
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
   When !compiler = 'VC' & _resource = 'yes' Then
      Do
         Call BuildResourceFile
         cmd = !link !ldflags_program !program || !obj !des_objs !zlib_objs '_myres.obj' 'getargv0' || !obj !intlib !mh_extra_libs
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
   When (!compiler = 'EMXOS2'| !compiler = 'GCC') & _win32gui = 'yes' Then
      Do
         If _resource = 'yes' Then
            Do
              Call BuildResourceFile
              res = '_myres.res'
            End
         Else res = ''
         deffile = !program'.def'
         Call Stream deffile, 'C', 'OPEN WRITE REPLACE'
         Call Lineout deffile, 'NAME WINDOWAPI'
         Call Stream deffile, 'C', 'CLOSE'
         cmd = !link !ldflags_program !program || !obj !des_objs !zlib_objs 'getargv0' || !obj deffile res !intlib !mh_extra_libs
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
   When !compiler = 'WCC' Then
      Do
         if os \= 'UNIX' then
            cmd = !link !ldflags_program 'file' !program || !obj !des_objs !zlib_objs 'file getargv0' || !obj !intlib !mh_extra_libs
         else
            cmd = !link !ldflags_program 'file' _dir || '/rexxwrapper' || !obj !des_objs !zlib_objs 'file getargv0' || !obj !intlib !mh_extra_libs
         if os='WIN32' then cmd=cmd||"alias _RexxStart='_RexxStart@36' alias _GetModuleFileNameA='_GetModuleFileNameA@12' lib kernel32.lib"
         If _debug > 0 Then Say '   <<debug>>' cmd
      say cmd
         cmd
      End
   Otherwise
      Do
         cmd = !link !ldflags_program !program || !obj !des_objs !zlib_objs 'getargv0' || !obj !intlib !mh_extra_libs
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
End
If rc \= 0 Then Call Abort 'error linking' !program
If !extralink_program \= '' Then
   Do
      If _debug > 0 Then Say '   <<debug>>' !extraline_program
      !extralink_program
      If rc \= 0 Then Call Abort 'error running' !extralink_program 'for' !program
   End
/*
 * Some cleanup and finalization
 */
Call Directory here
file_delete 'rexxwrap.tmp' null
file_copy _dir || !dirsep || !program || exe_ext '.'
If rc \= 0 Then Call Abort 'error copying' !program || exe_ext 'from' _dir
file_delete _dir || !dirsep || '*' null
directory_delete _dir null
Say 'Done!' !program || exe_ext 'built successfully'
Return

BuildGetargv0:
Say 'Compiling getargv0.c...'
cmd = !cc !cflags !defines !includes quote( !srcdir || !dirsep || 'getargv0.c' )
If _debug > 0 Then Say '   <<debug>>' cmd
cmd
If rc \= 0 Then Call Abort 'error compiling getargv0.c'
Return

BuildResourceFile:
Call Stream '_myres.rc', 'C', 'OPEN WRITE REPLACE'
If !compiler = 'VC' Then
   Do
      my_ver = Translate( _version, ',', '.' )
      Call Lineout '_myres.rc','#include "winver.h"'
      Call Lineout '_myres.rc',''
      Call Lineout '_myres.rc', 'VS_VERSION_INFO VERSIONINFO'
      Call Lineout '_myres.rc', ' FILEVERSION' my_ver
      Call Lineout '_myres.rc', ' PRODUCTVERSION' my_ver
      Call Lineout '_myres.rc', ' FILEFLAGSMASK 0x0L'
      Call Lineout '_myres.rc', ' FILEFLAGS 0x0L'
      Call Lineout '_myres.rc', ' FILEOS 0x40004L'
      Call Lineout '_myres.rc', ' FILETYPE VFT_APP'
      Call Lineout '_myres.rc', ' FILESUBTYPE 0x0L'
      Call Lineout '_myres.rc', 'BEGIN'
      Call Lineout '_myres.rc', '    BLOCK "StringFileInfo"'
      Call Lineout '_myres.rc', '    BEGIN'
      Call Lineout '_myres.rc', '        BLOCK "040904b0"'
      Call Lineout '_myres.rc', '        BEGIN'
      Call Lineout '_myres.rc', '            VALUE "FileDescription", "'_desc'\0"'
      Call Lineout '_myres.rc', '            VALUE "FileVersion", "'_version'\0"'
      Call Lineout '_myres.rc', '            VALUE "InternalName", "'Translate( !program )'\0"'
      Call Lineout '_myres.rc', '            VALUE "LegalCopyright", "Copyright ?'Substr( Date( 'S' ), 1, 4 )'\0"'
      Call Lineout '_myres.rc', '            VALUE "License", "'_license'\0"'
      Call Lineout '_myres.rc', '            VALUE "OriginalFilename", "'!program'.exe\0"'
      Call Lineout '_myres.rc', '            VALUE "ProductName", "'Translate( !program )'\0"'
      Call Lineout '_myres.rc', '            VALUE "ProductVersion", "'_version'\0"'
      Call Lineout '_myres.rc', '        END'
      Call Lineout '_myres.rc', '    END'
      Call Lineout '_myres.rc',''
      Call Lineout '_myres.rc', '    BLOCK "VarFileInfo"'
      Call Lineout '_myres.rc', '    BEGIN'
      Call Lineout '_myres.rc', '        VALUE "Translation", 0x409, 1200'
      Call Lineout '_myres.rc', '    END'
      Call Lineout '_myres.rc', 'END'
      Call Lineout '_myres.rc',''
End
/*
 * Change all double slashes to single slashes, and then change single slashes to doubles!
 */
_icon = Changestr( '\\', _icon, '\' )
_icon = Changestr( '\', _icon, '\\' )
Select
   When !compiler = 'VC' Then Call Lineout '_myres.rc', '2 ICON    DISCARDABLE     "'_icon'"'
   When !compiler = 'EMXOS2' | (!compiler = 'GCC' & os = 'OS2' ) Then Call Lineout '_myres.rc', 'ICON 1 "'_icon'"'
   Otherwise Nop
End
Call Stream '_myres.rc', 'C', 'CLOSE'
Select
   When !compiler = 'VC' Then
      Do
         cmd = 'rc /r /fo_myres.res _myres.rc'
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
         cmd = 'cvtres /MACHINE:IX86 /NOLOGO /OUT:_myres.obj _myres.res'
         If _debug > 0 Then Say '   <<debug>>' cmd
         Address System cmd
      End
   When !compiler = 'EMXOS2' | (!compiler = 'GCC' & os = 'OS2' ) Then
      Do
         cmd = 'rc -r _myres.rc'
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
   Otherwise Nop
End
Return

BuildIntBuild: Procedure Expose _debug !srcdir !dirsep !cc !cflags !defines !includes !link !ldflags_intbuild !obj !des_objs !zlib_objs !mh_extra_libs !extralink_intbuild dos_rsp_file borland_rsp_file file_delete
Say 'Compiling intbuild.c...'
cmd = !cc !cflags !defines !includes quote( !srcdir || !dirsep || 'intbuild.c' )
If _debug > 0 Then Say '   <<debug>>' cmd
cmd
If rc \= 0 Then Call Abort 'error compiling intbuild.c'
Say 'Linking intbuild...'
Select
   When dos_rsp_file Then
      Do
         Call OpenWriteReplace 'tmp.rsp'
         Call Lineout 'tmp.rsp','intbuild' || !obj !zlib_objs !des_objs
         Call Stream 'tmp.rsp', 'C', 'CLOSE'
         !link !ldflags_intbuild '@tmp.rsp' !mh_extra_libs
      End
   When borland_rsp_file Then
      Do
         Call OpenWriteReplace 'tmp.rsp'
         Call Lineout 'tmp.rsp','c0x32.obj intbuild' || !obj !zlib_objs !des_objs
         Call Lineout 'tmp.rsp','intbuild'
         Call Lineout 'tmp.rsp',''
         Call Lineout 'tmp.rsp',!mh_extra_libs
         Call Stream 'tmp.rsp', 'C', 'CLOSE'
         !link !ldflags_intbuild '@tmp.rsp'
      End
   Otherwise
      Do
         cmd = !link !ldflags_intbuild 'intbuild' || !obj !des_objs !zlib_objs !mh_extra_libs
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
End
If rc \= 0 Then Call Abort 'error linking intbuild'
If !extralink_intbuild \= '' Then
   Do
      !extralink_intbuild
      If rc \= 0 Then Call Abort 'error running' !extralink_intbuild 'for intbuild'
   End
Return 0

BuildRexxtok: Procedure Expose _debug !compiler !srcdir !dirsep !cc !cflags !defines !includes !link !ldflags_rexxtok !obj !mh_extra_libs !extralink_rexxtok dos_rsp_file borland_rsp_file !intlib getopt_obj file_delete
Say 'Compiling rexxtok.c...'
cmd = !cc !cflags !defines !includes quote( !srcdir || !dirsep || 'rexxtok.c' )
If _debug > 0 Then Say '   <<debug>>' cmd
cmd
If rc \= 0 Then Call Abort 'error compiling rexxtok.c'
If getopt_obj \= '' Then
   Do
      Say 'Compiling getopt.c...'
      cmd = !cc !cflags !defines !includes quote( !srcdir || !dirsep || 'common' || !dirsep || 'getopt.c' )
      If _debug > 0 Then Say '   <<debug>>' cmd
      cmd
      If rc \= 0 Then Call Abort 'error compiling getopt.c'
   End
Say 'Linking rexxtok...'
Select
   When dos_rsp_file Then
      Do
         Call OpenWriteReplace 'tmp.rsp'
         If getopt_obj \= '' Then Call Lineout 'tmp.rsp',getopt_obj 'rexxtok' || !obj
         Else Call Lineout 'tmp.rsp','rexxtok' || !obj
         Call Stream 'tmp.rsp', 'C', 'CLOSE'
         cmd = !link !ldflags_rexxtok '@tmp.rsp' !mh_extra_libs !intlib
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
   When borland_rsp_file Then
      Do
         Call OpenWriteReplace 'tmp.rsp'
         If getopt_obj \= '' Then rsp_objs = 'c0x32.obj' getopt_obj 'rexxtok' || !obj
         Else rsp_objs = 'c0x32.obj rexxtok' || !obj
         Call Lineout 'tmp.rsp',rsp_objs
         Call Lineout 'tmp.rsp','rexxtok'
         Call Lineout 'tmp.rsp',''
         Call Lineout 'tmp.rsp',!mh_extra_libs !intlib
         Call Stream 'tmp.rsp', 'C', 'CLOSE'
         cmd = !link !ldflags_rexxtok '@tmp.rsp'
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
   When !compiler = 'WCC' Then
      Do
         cmd = !link !ldflags_rexxtok 'rexxtok' || !obj 'file' getopt_obj !mh_extra_libs !intlib
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
   Otherwise
      Do
         cmd = !link !ldflags_rexxtok 'rexxtok' || !obj getopt_obj !mh_extra_libs !intlib
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
      End
End
If rc \= 0 Then Call Abort 'error linking rexxtok'
If !extralink_rexxtok \= '' Then
   Do
      If _debug > 0 Then Say '   <<debug>>' !extralink_rexxtok
      !extralink_rexxtok
      If rc \= 0 Then Call Abort 'error running' !extralink_rexxtok 'for rexxtok'
   End
Return 0

BuildDesModules: Procedure Expose !srcdir !dirsep !cc !cflags !defines !includes !des_modules !obj _debug
des_dir = !srcdir || !dirsep || 'des' || !dirsep
Call Charout ,'Compiling DES modules...'
Do i = 1 To Words(!des_modules)
   src = des_dir || Word(!des_modules,i) || '.c'
   If rebuild(src,Word(!des_modules,i) || !obj) Then
      Do
         Call Charout ,Word(!des_modules,i) || ' '
         cmd = !cc !cflags !defines !includes quote( src )
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
         If rc \= 0 Then Call Abort 'error compiling DES module' Word(!des_modules,i)
      End
End
Say ''
Return 0

BuildZlibModules: Procedure Expose !srcdir !dirsep !cc !cflags !defines !includes !zlib_modules !obj _debug
zlib_dir = !srcdir || !dirsep || 'zlib' || !dirsep
Call Charout ,'Compiling ZLIB modules...'
Do i = 1 To Words(!zlib_modules)
   src = zlib_dir || Word(!zlib_modules,i) || '.c'
   If rebuild(src,Word(!zlib_modules,i) || !obj) Then
      Do
         Call Charout ,Word(!zlib_modules,i) || ' '
         cmd = !cc !cflags !defines !includes quote( zlib_dir || Word(!zlib_modules,i) || '.c' )
         If _debug > 0 Then Say '   <<debug>>' cmd
         cmd
         If rc \= 0 Then Call Abort 'error compiling ZLIB module' Word(!zlib_modules,i)
      End
End
Say ''
Return 0

quote: Procedure
Parse Arg str
Return '"'str'"'

rebuild: Procedure
Parse Arg src,obj
srcts = Stream(src,'C','QUERY TIMESTAMP')
objts = Stream(obj,'C','QUERY TIMESTAMP')
If objts = '' Then Return 1
If objts > srcts Then Return 0
Return 1

OpenWriteReplace: Procedure Expose file_delete
/*
 * This function tries to do the equivalent of:
 * Stream(fn,'C','OPEN WRITE REPLACE')
 * because not all interpreters understand the above
 * syntax :-(
 */
Parse Arg fn
file_delete fn
Call Stream fn, 'C', 'OPEN WRITE'
Return

ProcessArg:
Parse Arg parm
Do pai = 1 To switches.0
   If switches.pai = parm & Countstr( 'yes/no', prompts.pai ) = 1 Then
      Do
         Interpret variables.pai '= "yes"'
         Return
      End
   Parse Var parm sw '=' value
   If switches.pai = sw Then
      Do
         Interpret variables.pai '= value'
         Return
      End
End
Call Abort 'unknown switch:' argv 'specified'
Return

Usage: Procedure Expose os
Say 'Usage:'
Say '  rexxwrap.cmd [-d[prmfile] | -help | -syntax | -program=prog -rexxfiles=rexxprog -source=srcdir -compiler=compiler -interpreter=interpreter -intlib=intlib -intincdir=intincdir [-compress] [-encrypt] ]'
Say 'where:'
Say '       -d[prmfile] - executes rexxwrap.cmd using default responses in "prmfile" or "rexxwrap.prm" if "prmfile" not supplied'
Say '       -help       - show this information'
Say '       -syntax     - show syntax of PRM file'
Say '       prog        - name of executable file'
Say '       rexxprog    - name of input Rexx program'
Say '       srcdir      - directory containing Rexx/Wrapper source files'
Say '       compiler    - name of compiler. Can be one of:'
int = '       interpreter - name of Rexx interpreter to link with. Can be one of'
Select
   When os = 'WIN32' Then
      Do
         Say '                     cygwin (Cygnus Win32)'
         Say '                     ming (MingW32)'
         Say '                     lcc (lccwin32)'
         Say '                     vc (Microsoft Visual C++)'
         Say '                     wcc (Watcom C++)'
         Say '                     borland (Borland C++ 5.5)'
         Say int
         Say '                     regina - Regina'
         Say '                     rexxtrans - Rexx Translation library'
         Say '                     orexx - IBM Object REXX'
         Say '                     oorexx - RexxLA Open Object Rexx'
         Say '                     winrexx - Enterprise Alternatives WinREXX'
         Say '                     quercus - Quercus Systems Personal Rexx'
         Say '                     unirexx - Workstation Group uni-REXX'
      End
   When os = 'OS2' Then
      Do
         Say '                     gcc (Innotek/gcc) target OS/2'
         Say '                     emxos2 (EMX/gcc) target OS/2'
         Say '                     emxdos (EMX/gcc) target DOS - VCPI'
         Say '                     wcc (OpenWatcom)'
         Say int
         Say '                     regina - Regina'
         Say '                     os2rexx - "classic" Rexx on OS/2'
      End
   When os = 'DOS' Then
      Do
         Say '                     djgpp (DJGPP) - DPMI'
         Say int
         Say '                     regina - Regina'
      End
   When os = 'QNX' Then
      Do
         Say '                     cc (Watcom C++)'
         Say int
         Say '                     regina - Regina'
      End
   When os = 'BEOS' Then
      Do
         Say '                     gcc (GNU Compiler)'
         Say int
         Say '                     regina - Regina'
      End
   Otherwise
      Do
         Say '                     gcc (GNU compiler)'
         Say '                     egcs (Experimental GNU Compiler)'
         Say '                     cc (Native C compiler)'
         Say '                     xlc (IBM compiler)'
         Say '                     c89 (Native C compiler)'
         Say int
         Say '                     regina - Regina'
         Say '                     rexximc - REXX/imc'
         Say '                     rexxtrans - Rexx Translation library'
         Say '                     orexx - IBM Object REXX'
         Say '                     oorexx - RexxLA Open Object Rexx'
         Say '                     unirexx - Workstation Group uni-REXX'
      End
End
Say "       intlib      - the full file name of the Rexx interpreter library to link to"
Say "       intincdir   - the directory containing the Rexx interpreter's header file"
Say "       compress    - indicates that the source code is to be compressed"
Say "       encrypt     - indicates that the source code is to be encrypted"
Say "       token       - indicates that the source code is to be tokenised"
Say "       subroutine  - indicates that the Rexx program is to be called as a subroutine"
Select
When os = 'WIN32' Then
   Do
      Say " The following options are available if you are using Microsoft Visual C++ compiler."
      Say "       win32gui    - indicates that the program is to built as a WIN32 GUI application"
      Say "       guiconsole  - indicates that the program displays any output or gets input from an optional console"
      Say "       resource    - indicates if program resources are to be generated"
      Say "       version     - specify the version information; usually a string like 1.0.2"
      Say "       desc        - a description of the program"
      Say "       icon        - the icon to attach to the program executable"
      Say
      Say " If you plan on building a program with the Microsoft Visual C++ complier,"
      Say " then you don't need to read the following section: it is not relevant to you."
      Say
      Say " If you plan on building a program with a C compiler other than Microsoft"
      Say " Visual C++, then you need to understand the following section."
      Say
      Say " Nearly all C compilers on this platform are incompatible with one another."
      Say " As most Rexx interpreters are built with Microsoft Visual C++, then you"
      Say " cannot simply specify the Rexx API import library that comes with your Rexx"
      Say " interpreter and expect it to work. Instead, you need to use an import library"
      Say " that is compatible with your compiler, generated by the compiler you intend"
      Say " using."
      Say
      Say " Rexx/Wrapper tries to automate the process of building a suitable import library"
      Say " but if the executable fails to link, it may be due to problems with the import"
      Say " library and the DLL supplied with your interpreter."
      Say
      Say " To build import libraries for your complier, and Rexx interpreter follow these"
      Say " steps:"
      Say
      Say " cygwin and MinGW:"
      Say " ================"
      Say " 1) and the following lines to 'myrexx.def'; one line per symbol in the "
      Say "    following format:"
      Say "    EXPORTS RexxStart@36"
      Say "    EXPORTS RexxFreeMemory@4"
      Say " 2) dlltool -k -A --input-def myrexx.def --dllname myrexx.dll --output-lib libmyrexx.a"
      Say
      Say " Borland:"
      Say " ========"
      Say " 1) coff2omf myrexx.lib myrexx_borland.lib"
      Say
   End
When os = 'OS2' Then
   Do
      Say " The following options are available if you are using EMX compiler."
      Say "       win32gui    - indicates that the program is to built as an OS/2 GUI application"
      Say "       icon        - the icon to attach to the program executable"
   End
Otherwise Nop
End
Return

ShowSyntax: Procedure Expose os prompts. switches.
Say 'Syntax:'
Do i = 1 To prompts.0
   Say '  'Left( 'default.'i, 12 ) '- ['switches.i']' prompts.i
End
Exit 1

Abort: Procedure Expose here
Trace o
Parse Arg msg
Say Copies('*',60)
Say msg
Say Copies('*',60)
Call Directory here
Exit 1
Return

HaltProgram:
Say ''
Say ''
Say '...rexxwrap.cmd has been terminated!'
Say ''
Exit 2
Return

/* Count number of times first string includes into second one */
/* valerius, 2007, Aug 23 ------------------------------------ */
Countstr: procedure
c = arg(1)
s = arg(2)

i = 0
p = 0

do until p = 0
  p = pos(c, s, p + 1)
  if p > 0 then
    i = i + 1
end

return i

/* valerius, 2008, Aug 11 ------------------------------------ */
truncate: procedure
name = arg(1)
type = arg(2)
char = arg(3)

return strip(name, type, char)
/* ----------------------------------------------------------- */
