/*
 * TODO:
 * - support reserved checkout repository - DONE not tested
 * Add syntax/extension mapping to repository settings - DONE
 * Issue with TABS display and invalid UTF8 strings on GTK - DONE
 * - revamp batch jobs. - DONE
 *   Jobs: button to show number of running jobs - DONE
 *   Clicking on Jobs: button displays dialog listing each job (description, time submitted, status (Running, Completed)
 *     and menu to view and delete job details - DONE
 *   Jobs stay in log dir until they are deleted, so old jobs can be viewed in next session - DONE
 *   use "tee" for batch jobs so a permanent record of the job is retained - DONE
 *   have "log" directory for each repository so jobs for different repositories
 *     are listed seperately - DONE
 * LabelModule: make it work with "standard" cvs - can't because CVS doesn't return correct
 *     info for common "linked" directories
 * Add ListLocked command for a module.  List all files that are checked out. - DONE
 * Add Directory/File menu maintenance - DONE
 * Finish Link Directory - DONE
 * Finish GetAll - DONE (problem with dw_messagebox() under Win32 - removed messagebox)
 * Finish Copy Repository; delete label files TODO
 * Remove .label files from checkoutall/undocheckoutall - DONE
 * When checking out (for lock), undo checkout, check in, update file buttons - DONE
 * When adding directories, ignore "files to ignore" - DONE
 * Finish Promote - DONE
   default promote command: zip %V%.zip %D%
      %V% version promoting (NOT dir,ver, but ver)
      %D% !installdirs
   need !installdirs, !promotecommand
 * Make to run in background. Need to run it somehow in a native shell script that captures
   all of the output. - DONE
 * copy repository to work this way for CVS
   - copy all files in labelfile from current repository directory to new repository directory
     cp oldir/file,v newdir/file,v
   - for each file in label file, get rel
     - get labels for file
     - for each label determine if the rel on the label > rel in labelfile
       - if so, delete label: "rtag -d label"
     - now all labels are gone, "admin -orel::"
     - delete label files identified above
 * LABEL command to include admin/build.conf, and this build.conf should be used when
   building or promoting or an option to use original settings, or current settings!
 * Renameinrepository still - DON'T provide rename
 * ReportMods doesn't work across repositories.
   Try being in test: test/working projects/latest - DONE
 * Delete Repository
 * Add bookmarks - DONE
 * "flat" display of files - DONE
 * Add Clear Output on messages pane - DONE
 * Show Differences should not allow "Working" unless a working file exists - DONE
 * Use cvs list only if !global.!!repository.!!enhanced_cvs.!!last_repository = 'Y'
 * Determine if dw_exit() should be called! rexxdw.rexx does not use it.
 * Have a module level exclusion list of directories and files that are NOT to
 * be included in a label or build. Useful when large binaries are put in the
 * repository but are not used in the build
 * Check all callbacks when running "cvs" command in case we have lost access to CVS server; see UpdateCallbak
 *
 */
OPTIONS INTERNAL_QUEUES NOEXT_COMMANDS_AS_FUNCS
!global.!version = '4.1.1'
Numeric Digits 12

Parse Version . vno .
If vno < 5.00 Then Call AbortText "You need a version of Regina (Language Level) >= 5.00"
Trace 'O'
Parse Source os called_as prog
If called_as \= 'SUBROUTINE' Then
   Do
      Say 'This program MUST be called as a subroutine'
      Exit 1
   End

Signal on Halt
/*
signal on novalue
*/
--!global.!container_disallows_icons = 1
/*
 * Parse our arguments...
 */
!global.!batch_mode = 0
If Arg() > 1 | Left( Arg(1), 2 ) = '-h' Then !global.!batch_mode = 1
Do i = 1 To Arg()
   If Arg( i ) = '-O' Then !global.!offline = 1
   Else
      Do
         If !global.!batch_mode Then Queue Arg( i )
      End
End
/*
Parse Arg !global.!_args
Select
   When Words( !global.!_args ) > 1 | Left( !global.!_args, 2 ) = '-h' Then !global.!batch_mode = 1
   When Word( !global.!_args, 1 ) = '-o' Then
      Do
         !global.!offline = 1
         !global.!_args = Delword( !global.!_args, 1, 1 )
      End
   Otherwise Nop
End
*/

Call Initialisation

/*
 * Set things here that are need in batch as well as online
 */
!global.!tempfiles.0 = 0
!global.!mainwindow = '' /* set this here so we know if we have a main window to use */
!global.!color_background = '#eee5de'
!global.!validcvstypes = 'local pserver rsh ssh' /* ensure these stay in this order */
!global.!usermenu_saved_variable_name.0 = 0
/*
 * We can run the administrative functions; label, build and
 * promote in "batch" mode. So parse the command line and see
 * if we are running a batch command.
 */
If !global.!batch_mode Then Call Batch

If RxFuncAdd( 'DW_LoadFuncs', 'rexxdw', 'DW_LoadFuncs' ) \= 0 Then
   Do
      Say 'No GUI environment available - running in batch mode.'
      !global.!batch_mode = 1
      Queue '-H'
      Call Batch
   End
Call dw_loadfuncs

/*
Call RxFuncAdd 'CurlLoadFuncs', 'rexxcurl', 'CurlLoadFuncs'
Call CurlLoadFuncs
*/

Call dw_init
!global.!screen_width = dw_screen_width()
!global.!screen_height = dw_screen_height()
Select
   When !global.!screen_height > 768 Then !global.!toolbox_size = 32
   When !global.!screen_height > 600 Then !global.!toolbox_size = 26
   Otherwise !global.!toolbox_size = 20
End
/*
 * We can set our font size for GTK now...
 */
If !global.!os \= 'OS2' & !global.!os \= 'WIN32' & !global.!os \= 'WIN64' & !global.!os \= 'OSX' Then
   Do
      If !REXXDW.!GTK_MAJOR_VERSION > 1 Then
         Do
            If !global.!toolbox_size = 32 Then font_size = 10
            Else font_size = 8
            !global.!fixedfont      = 'monospace' font_size
            !global.!italicfont     = 'monospace italic' font_size
            !global.!boldfont       = 'monospace bold italic' font_size
            !global.!bolditalicfont = 'monospace italic' font_size
         End
      Else
         Do
            !global.!fixedfont      = '-adobe-courier-medium-r-normal-*-*-120-*-*-m-*-iso8859-1'
            !global.!italicfont     = '-adobe-courier-medium-o-normal-*-*-120-*-*-m-*-iso8859-1'
            !global.!boldfont       = '-adobe-courier-bold-r-normal-*-*-120-*-*-m-*-iso8859-1'
            !global.!bolditalicfont = '-adobe-courier-bold-o-normal-*-*-120-*-*-m-*-iso8859-1'
         End
   End

!global.!windowstyle = dw_or( !REXXDW.!DW_FCF_SYSMENU, !REXXDW.!DW_FCF_TITLEBAR, !REXXDW.!DW_FCF_SHELLPOSITION, !REXXDW.!DW_FCF_TASKLIST, !REXXDW.!DW_FCF_DLGBORDER, !REXXDW.!DW_FCF_SIZEBORDER, !REXXDW.!DW_FCF_MINMAX )

!global.!container_colour = dw_rgb( 255, 245, 225 )

!global.!months = 'January February March April May June July August September October November December'

!global.!maxfilebuttons = 21
!global.!maxfileseparators = 8
!global.!buttonheight = 25
!global.!widgetheight = 25

!global.!running_jobs = ''
/*
 * Many dwindows callbacks fire while setting up the windows.
 * Use this flag to indicate to our callback handlers to only
 * operate when this has been set to 1.
 */
!global.!allowcallbacks = 0
/*
 * Set !!last_repository to zero to allow the user settings file to be written during
 * initial setup
 */
!!last_repository = 0
/*
 * We have to read the users .qocca file to determine how many repositories they have
 * configured, so we know how many notebook pages to create.
 */
If Word( Stream( !global.!home'.qocca', 'C', 'FSTAT' ), 8 ) = 'RegularFile' Then
   Do
      Call ConvertOldUserSettingsFile
   End
Call ReadUserSettingsFile

If Datatype( !global.!!user.!!bookmark.0 ) \= 'NUM' Then
   Do
      !global.!!user.!!bookmark.0 = 0
      !global.!!user.!!bookmark_repository.0 = 0
   End
/*
 * If we don't have a default repository, we have to ask the user to setup one
 */
If !global.!!number_repositories = 0 | Left( !global.!!number_repositories, 1 ) = '!' Then
   Do
      If Stream( !global.!user_settings, 'C', 'QUERY EXISTS' ) = '' Then
         Do
            If UserPreferencesCallback( 'junk', 1 ) = 'close' Then Call CleanupAndExit 1
            Call WriteUserSettingsFile
         End
      If GetNewRepository() = 'close' Then Call CleanupAndExit 1
   End
Else
   Do
      /*
       * If we have 1 parameter passed, then assume it is a repository name
       * to open with
      If Words( !global.!_args ) = 1 Then
       */
      If Arg() = 1 Then
         Do
            repno = FindRepository( Arg( 1 ) )
            If repno = 0 Then
               Do
                  Call dw_messagebox 'Unknown repository', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Nothing known about "'Arg( 1 )'" repository.'
                  Call dw_exit 0
                  Exit 1
               End
            Else
               !!last_repository = repno
         End
   End
!global.!repfiles_flags.0 = 5
If !global.!container_disallows_icons = 1 Then icon_flag = !REXXDW.!DW_CFA_STRING
Else icon_flag = !REXXDW.!DW_CFA_BITMAPORICON
!global.!repfiles_flags.1 = dw_or( icon_flag, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!repfiles_flags.2 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!repfiles_flags.3 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!repfiles_flags.4 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!repfiles_flags.5 = dw_or( icon_flag, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!repfiles_titles.0 = 5
!global.!repfiles_titles.1 = '' /* Sts */
!global.!repfiles_titles.2 = 'Revision Date'
!global.!repfiles_titles.3 = 'Last Change'
!global.!repfiles_titles.4 = 'Revision'
!global.!repfiles_titles.5 = 'Reminder'
!global.!allfiles_flags.0 = 6
!global.!allfiles_flags.1 = dw_or( icon_flag, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!allfiles_flags.2 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!allfiles_flags.3 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!allfiles_flags.4 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!allfiles_flags.5 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!allfiles_flags.6 = dw_or( icon_flag, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!allfiles_titles.0 = 6
!global.!allfiles_titles.1 = '' /* Sts */
!global.!allfiles_titles.2 = 'Revision Date'
!global.!allfiles_titles.3 = 'Last Change'
!global.!allfiles_titles.4 = 'Revision'
!global.!allfiles_titles.5 = 'Directory'
!global.!allfiles_titles.6 = 'Reminder'
!global.!nonrepfiles_flags.0 = 3
!global.!nonrepfiles_flags.1 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!nonrepfiles_flags.2 = dw_or( !REXXDW.!DW_CFA_ULONG, !REXXDW.!DW_CFA_RIGHT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!nonrepfiles_flags.3 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!nonrepfiles_titles.0 = 3
!global.!nonrepfiles_titles.1 = 'File Date'
!global.!nonrepfiles_titles.2 = 'Size'
!global.!nonrepfiles_titles.3 = 'Permissions'
!global.!deletedfiles_flags.0 = 1
!global.!deletedfiles_flags.1 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
!global.!deletedfiles_titles.0 = 1
!global.!deletedfiles_titles.1 = 'Revision'
!global.!filefiltericons.0 = 6
!global.!filefiltericons.1 = 'fileuptodate'      ; !global.!filefiltericonsdesc.1 = 'Up-to-date'
!global.!filefiltericons.2 = 'filemodified'      ; !global.!filefiltericonsdesc.2 = 'Locally Modified'
!global.!filefiltericons.3 = 'fileneedscheckout' ; !global.!filefiltericonsdesc.3 = 'Needs Checkout'
!global.!filefiltericons.4 = 'fileneedspatch'    ; !global.!filefiltericonsdesc.4 = 'Needs Patch'
!global.!filefiltericons.5 = 'fileneedsmerge'    ; !global.!filefiltericonsdesc.5 = 'Needs Merge'
!global.!filefiltericons.6 = 'fileconflict'      ; !global.!filefiltericonsdesc.6 = 'Conflicts'
/*
 * Create the main window now and show it
 */
Call CreateMainWindow
--Call dw_window_set_font !global.!mainwindow, !global.!fixedfont /* MH */
/*
 * We now know the current repository we are working with; set
 * the notebook tab to match
 */
Call dw_notebook_page_set !global.!repnotebook, !global.!repnotebookpage.!!last_repository
/*
 * We can now change the main window title
 */
Call SetWindowTitle

!global.!repfile_sort_order = 0 /* default - name */
!global.!repfile_sort_direction.0 = 'ascending' /* name */
!global.!repfile_sort_direction.1 = 'ascending' /* status */
!global.!repfile_sort_direction.2 = 'ascending' /* date */
!global.!repfile_sort_direction.3 = 'ascending' /* user / locker */
!global.!repfile_sort_direction.4 = 'ascending' /* rev */
!global.!repfile_sort_direction.5 = 'ascending' /* reminder */
!global.!nonrepfile_sort_order = 0 /* default - name */
!global.!nonrepfile_sort_direction.0 = 'ascending' /* name */
!global.!nonrepfile_sort_direction.1 = 'ascending' /* date */
!global.!nonrepfile_sort_direction.2 = 'ascending' /* size */
!global.!nonrepfile_sort_direction.3 = 'ascending' /* perm */
!global.!allfile_sort_order = 0 /* default - name */
!global.!allfile_sort_direction.0 = 'ascending' /* name */
!global.!allfile_sort_direction.1 = 'ascending' /* status */
!global.!allfile_sort_direction.2 = 'ascending' /* date */
!global.!allfile_sort_direction.3 = 'ascending' /* user / locker */
!global.!allfile_sort_direction.4 = 'ascending' /* rev */
!global.!allfile_sort_direction.5 = 'ascending' /* reminder */

/*
 * Catch syntax errors from here on. This allows us to remove temporary
 * files we have created.
 */
Signal on Syntax

/*
 * Now populate our windows with the details based on the repository we
 * have selected
 */
Call DisplayRepository
Call PopulateBookmarks
/*
 * Our main loop
 */
/*
 * The following is the main event loop for this program. We check the
 * variable !REXXDW.!HAVE_REXXCALLBACK to determine which mechanism
 * can be used for callbacks. It is far better to use the mechanism
 * offered by an interpreter that offers RexxCallBack() in its API.
 */
If !REXXDW.!HAVE_REXXCALLBACK Then
   Do
      Call dw_main
   End
Else
   Do Forever
      cmd = dw_main_iteration()
      If cmd \= '' Then Interpret 'Call' cmd
   End

Return 0

Initialisation: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Various globals
 */
!global.!crlf = d2c(13)||d2c(10)
!global.!offline = 0
!global.!remindertext = ''
!global.!select_year = Substr( Date( 'S' ), 1, 4 )
!global.!select_month = Date( 'M' )
!global.!select_mon = Substr( Date( 'S' ), 5, 2 )
!global.!select_day = ''
If Value( 'QOCCA_ADMINISTRATOR', , 'ENVIRONMENT' ) = '' Then !global.!superadministrator = 'N'
Else !global.!superadministrator = 'Y'
Parse Source os . prog
Select
   When os = 'WIN32' | os = 'WIN64' | os = 'OS2' | os = 'OS/2' Then
      Do
         !global.!home = Value( 'QOCCA_HOME', , 'ENVIRONMENT' )
         If !global.!home = '' Then !global.!home = 'c:\'
         !global.!ossep = '\'
         !global.!devnull = '> nul 2>&1'
         !global.!stderrdevnull = '2> nul'
         !global.!background_start_window = 'start'
         !global.!background_start_nowindow = 'start /B'
         !global.!datadir = '' /*'-D c:\bin'*/
         If os = 'WIN32' | os = 'WIN64' Then
            Do
               win = Uname( 'S' )
               If win = 'WIN95' | win = 'WIN98' | win = 'WINME' Then !global.!background_start_nowindow = 'start /M'
               imagedir = 'images\win'
               !global.!os = os
               !global.!fixedfont = '8.Courier'
               !global.!italicfont = '8.Courier Italic'
               !global.!boldfont = '8.Courier Bold'
               !global.!bolditalicfont = '8.Courier Bold Italic'
               !global.!shell_ext = '.bat'
               !global.!shell_rc = '%errorlevel%'
               !global.!cmddelr = 'qrm -fr'
               !global.!cmdrename = 'move'
               !global.!cmdcopy = 'copy'
               !global.!cmddiff = 'qdiff.exe'
               !global.!cmdecho = 'qecho.exe -E'
               !global.!cmdmkdir = 'qmkdir.exe -p'
               !global.!cmdtee = 'qtee.exe'
               rexxutil = 'rexxutil'
            End
         Else
            Do
               !global.!os = 'OS2'
               imagedir = 'images\os2'
               !global.!fixedfont = '5.System VIO'
               !global.!italicfont = '5.System VIO Italic'
               !global.!boldfont = '5.System VIO Bold'
               !global.!bolditalicfont = '5.System VIO Bold Italic'
               !global.!shell_ext = '.cmd'
               !global.!shell_rc = '%errorlevel%'
               !global.!cmddelr = 'rm -fr'
               !global.!cmdrename = 'ren'
               !global.!cmdcopy = 'copy'
               !global.!cmddiff = 'diff.exe'
               !global.!cmdecho = 'echo.exe'
               !global.!cmdmkdir = 'mkdir.exe -p'
               !global.!cmdtee = 'tee.exe'
               rexxutil = 'regutil'
            End
         !global.!background_end = ''
         !global.!shell_command = ''
         !global.!shell_first_line = '@echo off'
         !global.!command_prefix = 'call'
         !global.!symlink = ''
         !global.!rexxdiff = 'rexxdiff.exe' /*regina.exe -a \qocca\diff.rexx*/
         !global.!qocca_command = 'qocca.exe' /*regina \qocca\qocca.rexx*/
         Call Value 'CVS_PASSFILE', !global.!home'.cvspass', 'ENVIRONMENT'
      End
   When Uname( 'S' ) = 'Darwin' Then -- MacOS X
      Do
         !global.!home = Value( 'HOME', , 'ENVIRONMENT' )
         !global.!ossep = '/'
         !global.!devnull = '> /dev/null 2>&1'
         !global.!stderrdevnull = '2> /dev/null'
         !global.!cmddelr = 'rm -fr'
         !global.!cmdrename = 'mv'
         !global.!cmdcopy = 'cp'
         !global.!cmddiff = 'diff'
         !global.!cmdecho = 'echo'
         !global.!cmdmkdir = 'mkdir -p'
         !global.!cmdtee = 'tee'
         !global.!background_start_window = 'xterm -e'   -- ???
         !global.!background_start_nowindow = ''
         !global.!background_end = '&'
         !global.!shell_ext = '.sh'
         !global.!shell_command = 'sh'
         !global.!shell_first_line = '#!/bin/sh'
         !global.!shell_rc = '$?'
         !global.!command_prefix = 'sh'
         !global.!symlink = 'ln -sf'
         -- determine which version of MacOS X we are running;
         -- < 10.6 (Snow Leopard) "open" does not allow args to be passed
         -- to an application
         Address System 'sw_vers -productVersion' With Output Stem osxver.
         If osxver.1 < 10.6 Then
            Do
               _args = ''
               !global.!rexxdiff_by_env_variable = 1
            End
         Else
            Do
               _args = '--args'
               !global.!rexxdiff_by_env_variable = 0
            End
         !global.!rexxdiff = 'open -n -b org.rexx.app.rexxdiff' _args
         !global.!qocca_command = 'qocca' /*regina $HOME/qocca/qocca.rexx*/
         imagedir = 'images/win'                 -- ???
         !global.!datadir = ''
         !global.!os = 'OSX'
         rexxutil = 'regutil'
         If Right( !global.!home, 1 ) \= '/' Then cvspass = !global.!home'/.cvspass'
         Else cvspass = !global.!home'.cvspass'
         Call Value 'CVS_PASSFILE', cvspass, 'ENVIRONMENT'
         !global.!fixedfont = '10.Monaco'
         !global.!italicfont = '10.Monaco Italic'
         !global.!boldfont = '10.Monaco Bold'
         !global.!bolditalicfont = '10.Monaco Bold Italic'
      End
   Otherwise /* Unixy OSes */
      Do
         !global.!home = Value( 'HOME', , 'ENVIRONMENT' )
         !global.!ossep = '/'
         !global.!devnull = '> /dev/null 2>&1'
         !global.!stderrdevnull = '2> /dev/null'
         !global.!cmddelr = 'rm -fr'
         !global.!cmdrename = 'mv'
         !global.!cmdcopy = 'cp'
         !global.!cmddiff = 'diff'
         !global.!cmdecho = 'echo'
         !global.!cmdmkdir = 'mkdir -p'
         !global.!cmdtee = 'tee'
         !global.!background_start_window = 'xterm -e'
         !global.!background_start_nowindow = ''
         !global.!background_end = '&'
         !global.!shell_ext = '.sh'
         !global.!shell_command = 'sh'
         !global.!shell_first_line = '#!/bin/sh'
         !global.!shell_rc = '$?'
         !global.!command_prefix = 'sh'
         !global.!symlink = 'ln -sf'
         !global.!rexxdiff = 'rexxdiff' /*regina -a $HOME/qocca/diff.rexx*/
         !global.!qocca_command = 'qocca' /*regina $HOME/qocca/qocca.rexx*/
         imagedir = 'images/win'
         !global.!datadir = ''
         !global.!os = os
         rexxutil = 'rexxutil'
         If Right( !global.!home, 1 ) \= '/' Then cvspass = !global.!home'/.cvspass'
         Else cvspass = !global.!home'.cvspass'
         Call Value 'CVS_PASSFILE', cvspass, 'ENVIRONMENT'
         /*
          * Setting the font for GTK has to wait until we have
          * loaded Rexx/DW, so we can get the value of
          * !REXXDW.!GTK_MAJOR_VERSION
          */
      End
End
/*
 * Based on the full path to ourselves, find the icons directory
 */
pos = Lastpos( !global.!ossep, prog )
If pos = 0 Then base = '.'
Else base = Substr( prog, 1, pos-1 )
len = Length( base )
If Translate( Right( base, 3 ) ) = 'BIN' Then
   Do
      !global.!icondir = Substr( base, 1, len-3 ) || 'share' || !global.!ossep || 'qocca' || !global.!ossep || imagedir || !global.!ossep
      !global.!helpdir = Substr( base, 1, len-3 ) || 'share' || !global.!ossep || 'qocca' || !global.!ossep || 'help' || !global.!ossep
   End
Else
   Do
      !global.!icondir = base || !global.!ossep || imagedir || !global.!ossep
      !global.!helpdir = base || !global.!ossep || 'help' || !global.!ossep
   End
/*
 * Based on the full path to ourselves, find the base directory for highlight
 * Don't do this for Unix platforms
 */
If os = 'WIN32' | os = 'WIN64' | os = 'OS2' | os = 'OS/2' Then
   Do
      If Translate( Right( base, 3 ) ) = 'BIN' Then !global.!datadir = '-D' quote( Substr( base, 1, len-4 ) )
      Else !global.!datadir = '-D' quote( base )
   End
/*
 * Do we actually have highlight?
 */
Address System 'highlight --version' With Output Stem junk. Error Stem junk.
If rc = 0 Then !global.!have_highlight = 1
Else !global.!have_highlight = 0

!global.!valid_repository_types = 'cvs rcs svn'

/*
 * Load RexxUtil external function package
 */
/*
Call RxFuncAdd 'SysLoadFuncs', rexxutil, 'SysLoadFuncs'
*/
Call RxFuncAdd 'sysloadfuncs', rexxutil, 'sysloadfuncs'
Call SysLoadFuncs

/*
 * Get username and home values
 */
user = Value( 'QOCCA_USERNAME', , 'ENVIRONMENT' )
If user = '' Then !global.!user = Userid()
Else !global.!user = user
If !global.!home = '' Then Call AbortText 'HOME environment value not set. Cannot continue'
!global.!home = Changestr( '/', !global.!home, !global.!ossep )
If Right( !global.!home, 1 ) \= !global.!ossep Then !global.!home = !global.!home||!global.!ossep
!global.!user_settings = !global.!home'.qocca'||!global.!ossep||'user.settings'

/*
 * If on Windows 32bit only, load w32funcs
 */
If !global.!os = 'WIN32' Then
   Do
      Call RxFuncAdd 'w32loadfuncs','w32util','w32loadfuncs'
      If w32loadfuncs() \= 0 Then
         Do
            Say RxFuncErrmsg()
            Exit 1
         End
      !global.!loadedw32util = 1
   End
/*
 * Load the Rexx/SQL functions for MySQL
 */
!global.!havesql = 0
/*
!global.!havesql = 1
If \!global.!batch_mode & !global.!os \= 'OS2' Then
   Do
      If RxFuncAdd( 'SQLLoadFuncs','rexxsql','SQLLoadFuncs' ) \= 0 Then
         Do
            Say RxFuncErrmsg()
            !global.!havesql = 0
         End
      Else
         Do
            If sqlloadfuncs() \= 0 Then Exit 1
         End
   End
*/
/*
 * Defines the known templates for various compilers
 */
Call SetCompilerTemplates

Return

DisplayRepository: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * While setting up our display, don't allow callbacks to be processed
 */
!global.!allowcallbacks = 0
Call SetCursorWait

Call DisplayStatus 'Setting environment...'
Call SetRepositoryEnvs
/*
 * We are now pointing to a configuration (and have logged in if necessary)
 */
Call DisplayStatus  'Reading repository settings...'
Call ConnectToRepository !!last_repository
Call GetConfigurationFile

Call DisplayStatus  'Creating working environment...'
Call SetupEnvironment

Call GetModulesFromRepository
/*
 * Now we have all the repository information, we can create our tree
 */
Call GenerateTree

Call DisplayRepositoryFiles
Call DisplayNonRepositoryFiles
Call DisplayDeletedFiles
/*
 * Don't display "all files"; only generate this list when switching pages
 */
/*
 * Create/update our toolbars...
 */
Call AdjustToolbars
/*
 * Blank out the status line
 */
Call SetCursorNoWait
Call DisplayStatus ''
!global.!allowcallbacks = 1
/*
 * Reshow the main window because buttons could have changed
 */
Call dw_window_show !global.!mainwindow
Return

AdjustToolbars: Procedure Expose !REXXDW. !global. !!last_repository
Call CreateDirToolbarList
Call CreateDirToolbar
/*
 * Now we know the current module (if there is one)
 */
Call AdjustDirToolbarList
Call AdjustDirToolbarButtons
/*
 * Create the file toolbox buttons
 */
Call CreateFileToolbarList
Call CreateFileToolbar
/*
 * Now we know the current files (if there are some)
 */
Call AdjustFileToolbarList
Call AdjustFileToolbarButtons
Return

ConvertOldUserSettingsFile: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Read the old .qocca file into memory variables
 */
fn = !global.!home'.qocca'
Call Stream fn, 'C', 'OPEN READ'
Do While Lines(fn) > 0
   Parse Value Linein(fn) With key '=' value
   If key \= '' Then
      Do
         If Left( value, 8 ) = '"!GLOBAL.' Then value = ''
         If key = '!!last_repository' Then Interpret key '=' value
         Else Interpret '!GLOBAL.'key '=' value
      End
End
Call Stream fn, 'C', 'CLOSE'
!global.!!last_repository_name = !global.!!repository.!!name.!!last_repository
!global.!!number_repositories = !global.!!repository.!!name.0
/*
 * Delete the .qocca file and create a directory with the same name
 */
Call SysFileDelete fn
Call CreateDirectory fn
/*
 * Now write the new config file and repository details
 */
Call WriteUserSettingsFile
Return

ReadUserSettingsFile: Procedure Expose !REXXDW. !global. !!last_repository
fn = !global.!user_settings
Call Stream fn, 'C', 'OPEN READ'
Do While Lines(fn) > 0
   Parse Value Linein(fn) With key '=' value
   If key \= '' Then
      Do
         If Left( value, 8 ) = '"!GLOBAL.' Then value = ''
/*         If key = '!!last_repository' Then Interpret key '=' value*/
         Interpret '!GLOBAL.'key '=' value
      End
End
Call Stream fn, 'C', 'CLOSE'
/*
 * Now read all repository settings files
 */
dir = !global.!home'.qocca'
Call SysFileTree dir||!global.!ossep||'repository.*', 'FILE.', 'FO'
/*
 * If we have repository.* files then rename them to *.repository files
 * and then read these in
 */
Do i = 1 To file.0
   fn = file.i
   Parse Var fn . 'repository.' base
   Address System !global.!cmdrename fn dir||!global.!ossep||base'.repository' With Output Stem junk. Error Stem junk.
End
Call SysFileTree dir||!global.!ossep||'*.repository', 'FILE.', 'FO'
If file.0 > 0 Then
   Do
      /*
       * In case we don't find a match, set the last repository to be 1
       */
      !!last_repository = 1
      If file.0 > 1 Then Call SysStemSort 'FILE.','ascending',!global.!!user.!!preference.!!sort, , , 1, 100
      Do i = 1 To file.0
         fn = file.i
         Call Stream fn, 'C', 'OPEN READ'
         Do While Lines(fn) > 0
            Parse Value Linein(fn) With key '=' value
            If key \= '' Then
               Do
                  If Left( value, 8 ) = '"!GLOBAL.' Then value = ''
                  a = Value( key )'.'Value( i )
                  Interpret '!GLOBAL'.a '=' value
               End
         End
         Call Stream fn, 'C', 'CLOSE'
         /*
          * Check if this repository was the last one to be opened
          */
         If !global.!!repository.!!name.i = !global.!!last_repository_name Then !!last_repository = i
         /*
          * Tidy up new variables and set defaults
          */
         If Left( !global.!!repository.!!filefilters.i, 1 ) = '!' Then !global.!!repository.!!filefilters.i = ''
         If Left( !global.!!repository.!!cvs_compression.i, 1 ) = '!' Then !global.!!repository.!!cvs_compression.i = '-z3'
   End
End
!global.!!number_repositories = file.0
Return

WriteUserSettingsFile: Procedure Expose !REXXDW. !global. !!last_repository
dir = !global.!home'.qocca'
Call CreateDirectory dir
/*
 * Do a quick check that we have access to the variables we are writing;
 * we don't want to stuff up the .qocca file!
 */
If \DirectoryExists( Strip( !global.!!tmpdir, 'T', '/' ) ) | Datatype( !!last_repository ) \= 'NUM' | !global.!!user.!!textpager = '!GLOBAL.!!USER.!!TEXTPAGER' Then Return
If !!last_repository \= 0 Then
   Do
      /*
       * Write the repositories...if there are any
       */
      If Left( !global.!!number_repositories, 1 ) \= '!' Then
         Do i = 1 To !global.!!number_repositories
            fn = dir||!global.!ossep||!global.!!repository.!!name.i'.repository'
            Call Stream fn, 'C', 'OPEN WRITE REPLACE'
            Call Lineout fn,'!!repository.!!name='Quote( !global.!!repository.!!name.i )
            Call Lineout fn,'!!repository.!!path='Quote( !global.!!repository.!!path.i )
            Call Lineout fn,'!!repository.!!type='Quote( !global.!!repository.!!type.i )
            Call Lineout fn,'!!repository.!!shortpath='Quote( !global.!!repository.!!shortpath.i )
            Call Lineout fn,'!!repository.!!server='Quote( !global.!!repository.!!server.i )
            Call Lineout fn,'!!repository.!!cvsport='Quote( !global.!!repository.!!cvsport.i )
            Call Lineout fn,'!!repository.!!username='Quote( !global.!!repository.!!username.i )
            Call Lineout fn,'!!repository.!!contype='Quote( !global.!!repository.!!contype.i )
            Call Lineout fn,'!!repository.!!password='Quote( !global.!!repository.!!password.i )
            Call Lineout fn,'!!repository.!!last_tree_opened='Quote( !global.!!repository.!!last_tree_opened.i )
            Call Lineout fn,'!!repository.!!working_dir='Quote( !global.!!repository.!!working_dir.i )
            Call Lineout fn,'!!repository.!!public_cvs='Quote( !global.!!repository.!!public_cvs.i )
            Call Lineout fn,'!!repository.!!public_cvs_dirs='Quote( !global.!!repository.!!public_cvs_dirs.i )
            Call Lineout fn,'!!repository.!!enhanced_cvs='Quote( !global.!!repository.!!enhanced_cvs.i )
            Call Lineout fn,'!!repository.!!filefilters='Quote( !global.!!repository.!!filefilters.i )
            Call Lineout fn,'!!repository.!!cvs_compression='Quote( !global.!!repository.!!cvs_compression.i )
            Call Stream fn, 'C', 'CLOSE'
         End
      /*
       * Write the last repository, ...
      Call Lineout fn,'!!last_repository='Quote( !!last_repository )
       */
   End
/*
 * Set the user settings configuration file name
 */
fn = !global.!user_settings
Call Stream fn, 'C', 'OPEN WRITE REPLACE'
/*
 * Write the last repository, ...
 */
Call Lineout fn,'!!last_repository_name='Quote( !global.!!repository.!!name.!!last_repository )
/*
 * ... now the user settings...
 */
Call Lineout fn,'!!user.!!style='Quote( !global.!!user.!!style )
Call Lineout fn,'!!user.!!diffprog='Quote( !global.!!user.!!diffprog )
Call Lineout fn,'!!user.!!textpager='Quote( !global.!!user.!!textpager )
Call Lineout fn,'!!user.!!textpager_console='Quote( !global.!!user.!!textpager_console )
Call Lineout fn,'!!user.!!texteditor='Quote( !global.!!user.!!texteditor )
Call Lineout fn,'!!user.!!texteditor_console='Quote( !global.!!user.!!texteditor_console )
Call Lineout fn,'!!user.!!binarypager='Quote( !global.!!user.!!binarypager )
Call Lineout fn,'!!user.!!binarypager_console='Quote( !global.!!user.!!binarypager_console )
Call Lineout fn,'!!user.!!binaryeditor='Quote( !global.!!user.!!binaryeditor )
Call Lineout fn,'!!user.!!binaryeditor_console='Quote( !global.!!user.!!binaryeditor_console )
Call Lineout fn,'!!user.!!preference.!!sort='Quote( !global.!!user.!!preference.!!sort )
Call Lineout fn,'!!user.!!preference.!!confirm_exit='Quote( !global.!!user.!!preference.!!confirm_exit )
Call Lineout fn,'!!user.!!preference.!!save_position='Quote( !global.!!user.!!preference.!!save_position )
Call Lineout fn,'!!user.!!preference.!!debug_commands='Quote( !global.!!user.!!preference.!!debug_commands )
Call Lineout fn,'!!user.!!preference.!!showmodified='Quote( !global.!!user.!!preference.!!showmodified )
Call Lineout fn,'!!user.!!preference.!!windowsfileassociations='Quote( !global.!!user.!!preference.!!windowsfileassociations )
Call Lineout fn,'!!user.!!preference.!!windowsfileassociations_extensions='Quote( !global.!!user.!!preference.!!windowsfileassociations_extensions )
Call Lineout fn,'!!user.!!preference.!!tabspaces='Quote( !global.!!user.!!preference.!!tabspaces )
Call Lineout fn,'!!tmpdir='Quote( !global.!!tmpdir )
If Datatype( !global.!!user.!!filemenu.!!label.0 ) = 'NUM' Then
   Do
      Call Lineout fn,'!!user.!!filemenu.!!label.0='Quote( !global.!!user.!!filemenu.!!label.0 )
      Call Lineout fn,'!!user.!!filemenu.!!command.0='Quote( !global.!!user.!!filemenu.!!command.0 )
      Do i = 1 To !global.!!user.!!filemenu.!!label.0
         Call Lineout fn,'!!user.!!filemenu.!!label.'i'='Quote( !global.!!user.!!filemenu.!!label.i )
         Call Lineout fn,'!!user.!!filemenu.!!command.'i'='Quote( !global.!!user.!!filemenu.!!command.i )
      End
   End
If Datatype( !global.!!user.!!dirmenu.!!label.0 ) = 'NUM' Then
   Do
      Call Lineout fn,'!!user.!!dirmenu.!!label.0='Quote( !global.!!user.!!dirmenu.!!label.0 )
      Call Lineout fn,'!!user.!!dirmenu.!!command.0='Quote( !global.!!user.!!dirmenu.!!command.0 )
      Do i = 1 To !global.!!user.!!dirmenu.!!label.0
         Call Lineout fn,'!!user.!!dirmenu.!!label.'i'='Quote( !global.!!user.!!dirmenu.!!label.i )
         Call Lineout fn,'!!user.!!dirmenu.!!command.'i'='Quote( !global.!!user.!!dirmenu.!!command.i )
      End
   End
/*
 * If not set, then the 0th item is zero.
 * Set the 0th item of !!bookmark_repository to the same value as the base
 */
If Datatype( !global.!!user.!!bookmark.0 ) \= 'NUM' Then !global.!!user.!!bookmark.0 = 0
Call Lineout fn,'!!user.!!bookmark.0='Quote( !global.!!user.!!bookmark.0 )
Call Lineout fn,'!!user.!!bookmark_repository.0='Quote( !global.!!user.!!bookmark.0 )
Do i = 1 To !global.!!user.!!bookmark.0
   Call Lineout fn,'!!user.!!bookmark.'i'='Quote( !global.!!user.!!bookmark.i )
   Call Lineout fn,'!!user.!!bookmark_repository.'i'='Quote( !global.!!user.!!bookmark_repository.i )
End
Call Lineout fn,'!!user.!!window_pos_size='Quote( !global.!!user.!!window_pos_size )
Call Stream fn, 'C', 'CLOSE'
Return

/*
 * Add bookmarks for the current repository into the bookmark combobox
 */
PopulateBookmarks: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Clean out the entries currently in the listbox
 */
count = dw_listbox_count( !global.!bookmarks_cb )
idx = 0
Do i = 1 To count
   Call dw_listbox_delete !global.!bookmarks_cb, idx
End
Call dw_window_set_text !global.!bookmarks_cb, ''
/*
 * Add the bookmarks for this repository
 */
Do i = 1 To !global.!!user.!!bookmark.0
   If !global.!!user.!!bookmark_repository.i = !!last_repository Then Call dw_listbox_append !global.!bookmarks_cb, !global.!!user.!!bookmark.i
End
Return

SelectBookmarkCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, list_idx
If list_idx < 0 Then Return 1
text = dw_listbox_get_text( win, list_idx )
Call dw_window_set_text win, text
Return 1

SelectBookmarkApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * For the selected bookmark, try and open it
 */
text = Strip( dw_window_get_text( !global.!bookmarks_cb ) )
openat = 0
Do i = 1 To !global.!repdir.!!last_repository.0
   If !global.!repdir.!!last_repository.i = '/'text Then
      Do
         openat = i
         Leave
      End
End
If openat \= 0 Then
   Do
      /*
       * Set the current dirname, open the tree and make it current...
       */
      !global.!current_dirname.!!last_repository = Substr( !global.!repdir.!!last_repository.openat, 2 )
      Call OpenTreeAt openat
      Call SetCursorWait
      /*
       * Adjust the directory toolbar
       */
      Call AdjustDirToolbarList
      Call AdjustDirToolbarButtons
      /*
       * Refresh the repository files, non-rep files and deleted files
       * tabs. DO NOT refresh the all files tab; it takes too long unless
       * it is the current tab
       */
      Call DisplayRepositoryFiles
      Call DisplayNonRepositoryFiles
      Call DisplayDeletedFiles
      If !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then Call DisplayAllFiles
      /*
       * If this directory is set for reserved checkout,
       */
      Call SetLockedStatusIcon
      /*
       * Adjust the file toolbar as well!
       */
      Call FileSelectCallback
      Call SetCursorNoWait
   End
Return 0

UserPreferencesCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, initial
/*
 * User preferences...
 *
 * +---------------------------------------------------------------------------------+
 * | Text                                                                            |
 * |                                                                                 |
 * +---------------------------------------------------------------------------------+
 * |  Text Editor:   _______________________X Binary Editor:   ____________________X |
 * |  Text Pager:    _______________________X Binary Viewer:   ____________________X |
 * |  Temp dir:      _______________________X X Use Windows File Associations:       |
 * |  Syntax Theme:  _______________________v X Case Sensitive Sorting               |
 * |  Diff Program:  _______________________X Tab Spaces:    _____________________v  |
 * |  <Directory Menus>                       <File Menus>
 * +---------------------------------------------------------------------------------+
 *
 */
title_width = 100
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Edit User Preferences', !global.!windowstyle )
If initial Then
   Do
      /*
       * We have to create the window icon here because the main window has not
       * been created yet.
       */
      qoccaicon = dw_icon_load_from_file( !global.!icondir'qocca' )
   End
Else qoccaicon = !global.!qoccaicon
Call dw_window_set_icon win, qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, !REXXDW.!DW_SIZE_AUTO, 150, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call UserPreferencesHelpFocusCallback t1, 'INITIAL', t1
/*
 * Add a horiz box for two columns of widgets
 */
data_box = dw_box_new( !REXXDW.!DW_HORZ )
 Call dw_box_pack_start topbox, data_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a vertical box for the left column of widgets
 */
left_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start data_box, left_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 5
/*
 * Add a vertical box for the right column of widgets
 */
right_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start data_box, right_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 5
/*
 * Add the boxes and widgets for the left column...
 */
/* text editor */
tmpbox = dw_groupbox_new( !REXXDW.!DW_VERT, 'Text Editor', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start left_box, tmpbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
text_editor_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start tmpbox, text_editor_box, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If Left( !global.!!user.!!texteditor, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!!user.!!texteditor
text_editor_entry = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start text_editor_box, text_editor_entry, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect text_editor_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'UserPreferencesHelpFocusCallback', 'TEXTEDITOR', t1
abutton = dw_bitmapbutton_new_from_file( 'Browse', 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start text_editor_box, abutton, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericFileSelector', text_editor_entry, 'Locate text editor', '', 1, !REXXDW.!DW_FILE_OPEN
text_editor_cb = dw_checkbox_new( 'Run in console', 0 )
Call dw_box_pack_start tmpbox, text_editor_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!!user.!!texteditor_console = 1 Then Call dw_checkbox_set text_editor_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set text_editor_cb, !REXXDW.!DW_UNCHECKED
/* text pager */
tmpbox = dw_groupbox_new( !REXXDW.!DW_VERT, 'Text Viewer', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start left_box, tmpbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
text_pager_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start tmpbox, text_pager_box, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If Left( !global.!!user.!!textpager, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!!user.!!textpager
text_pager_entry = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start text_pager_box, text_pager_entry, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect text_pager_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'UserPreferencesHelpFocusCallback', 'TEXTPAGER', t1
abutton = dw_bitmapbutton_new_from_file( 'Browse', 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start text_pager_box, abutton, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericFileSelector', text_pager_entry, 'Locate text viewer', '', 1, !REXXDW.!DW_FILE_OPEN
text_pager_cb = dw_checkbox_new( 'Run in console', 0 )
Call dw_box_pack_start tmpbox, text_pager_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!!user.!!textpager_console = 1 Then Call dw_checkbox_set text_pager_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set text_pager_cb, !REXXDW.!DW_UNCHECKED
/* temp dir */
tmp_dir_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, tmp_dir_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
tmp = dw_text_new( 'Temp Directory', 0 )
Call dw_box_pack_start tmp_dir_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
/*
 * If this is the first time through, try and find the user's temp directory...
 */
If initial Then
   Do
      oldvalue = GetUserTempDirectory()
   End
Else
   Do
      If Left( !global.!!tmpdir, 1 ) = '!' Then oldvalue = ''
      Else oldvalue = !global.!!tmpdir
   End
tmp_dir_entry = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmp_dir_box, tmp_dir_entry, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect tmp_dir_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'UserPreferencesHelpFocusCallback', 'TMPDIR', t1
abutton = dw_bitmapbutton_new_from_file( 'Browse', 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start tmp_dir_box, abutton, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericDirectorySelector', tmp_dir_entry, 'Set Temporary Directory to:'
/* diff program */
diff_program_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, diff_program_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
tmp = dw_text_new( 'Diff Program', 0 )
Call dw_box_pack_start diff_program_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!!user.!!diffprog, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!!user.!!diffprog
diff_program_entry = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start diff_program_box, diff_program_entry, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect diff_program_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'UserPreferencesHelpFocusCallback', 'DIFFPROG', t1
abutton = dw_bitmapbutton_new_from_file( 'Browse', 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start diff_program_box, abutton, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericFileSelector', diff_program_entry, 'Locate diff program', '', 1, !REXXDW.!DW_FILE_OPEN
/* empty space */
Call dw_box_pack_start left_box, 0, 0, 10, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/* directory menu */
b2 = dw_button_new( 'Maintain Directory Menus', 0 )
Call dw_box_pack_start left_box, b2, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'ModifyUserMenuCallback', 'DIR'
/*
 * Add the boxes and widgets for the right column...
 */
/* binary editor */
tmpbox = dw_groupbox_new( !REXXDW.!DW_VERT, 'Binary Editor', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start right_box, tmpbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
binary_editor_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start tmpbox, binary_editor_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
If Left( !global.!!user.!!binaryeditor, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!!user.!!binaryeditor
binary_editor_entry = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start binary_editor_box, binary_editor_entry, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect binary_editor_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'UserPreferencesHelpFocusCallback', 'BINARYEDITOR', t1
abutton = dw_bitmapbutton_new_from_file( 'Browse', 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start binary_editor_box, abutton, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericFileSelector', binary_editor_entry, 'Locate binary editor', '', 1, !REXXDW.!DW_FILE_OPEN
binary_editor_cb = dw_checkbox_new( 'Run in console', 0 )
Call dw_box_pack_start tmpbox, binary_editor_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!!user.!!binaryeditor_console = 1 Then Call dw_checkbox_set binary_editor_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set binary_editor_cb, !REXXDW.!DW_UNCHECKED
/* binary pager */
tmpbox = dw_groupbox_new( !REXXDW.!DW_VERT, 'Binary Viewer', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start right_box, tmpbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
binary_pager_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start tmpbox, binary_pager_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
If Left( !global.!!user.!!binarypager, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!!user.!!binarypager
binary_pager_entry = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start binary_pager_box, binary_pager_entry, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect binary_pager_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'UserPreferencesHelpFocusCallback', 'BINARYPAGER', t1
abutton = dw_bitmapbutton_new_from_file( 'Browse', 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start binary_pager_box, abutton, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericFileSelector', binary_pager_entry, 'Locate binary viewer', '', 1, !REXXDW.!DW_FILE_OPEN
binary_pager_cb = dw_checkbox_new( 'Run in console', 0 )
Call dw_box_pack_start tmpbox, binary_pager_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!!user.!!binarypager_console = 1 Then Call dw_checkbox_set binary_pager_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set binary_pager_cb, !REXXDW.!DW_UNCHECKED
/* tab spaces spinbutton */
tab_spaces_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start right_box, tab_spaces_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
tmp = dw_text_new( 'Tab Spaces', 0 )
Call dw_box_pack_start tab_spaces_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
sb = dw_spinbutton_new( '', 0 )
Call dw_box_pack_start tab_spaces_box, sb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_spinbutton_set_limits sb, 20, 0
If Datatype( !global.!!user.!!preference.!!tabspaces ) \= 'NUM' Then !global.!!user.!!preference.!!tabspaces = 4
Call dw_spinbutton_set_pos sb, !global.!!user.!!preference.!!tabspaces
Call dw_signal_connect sb, !REXXDW.!DW_SET_FOCUS_EVENT, 'UserPreferencesHelpFocusCallback', 'TABSPACES', t1
/*
 * Now the user's default syntax highlighter
 */
If initial Then
   Do
      stylebox = ''
      Call dw_box_pack_start right_box, 0, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   End
Else
   Do
      Call DisplayStatus 'Getting syntax highlighting themes...'

      tmp_box = dw_box_new( !REXXDW.!DW_HORZ )
      Call dw_box_pack_start right_box, tmp_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
      tmp = dw_text_new( 'Syntax Theme', 0 )
      Call dw_box_pack_start tmp_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
      If Left( !global.!!user.!!style, 1 ) = '' Then default_style = ''
      Else default_style = !global.!!user.!!style
      stylebox = dw_combobox_new( default_style, 0 )
      Call dw_box_pack_start tmp_box, stylebox, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Address System 'highlight -w' !global.!datadir With Output Stem styles. Error Stem junk.
      Do i = 1 To styles.0
         If Words( styles.i ) = 1 Then Call dw_listbox_append stylebox, Strip( styles.i )
      End
      Call dw_signal_connect stylebox, !REXXDW.!DW_LIST_SELECT_EVENT, 'SelectStyleCallback'
      Call dw_window_set_text stylebox, default_style /* default value doesn't work in dw_combobox_new */

      Call DisplayStatus ''
   End
/* empty space */
Call dw_box_pack_start right_box, 0, 0, 10, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/* file menu */
b1 = dw_button_new( 'Maintain File Menus', 0 )
Call dw_box_pack_start right_box, b1, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'ModifyUserMenuCallback', 'FILE'
/* -- checkboxes in groupbox - full width-- */
groupbox = dw_groupbox_new( !REXXDW.!DW_VERT, 'Options', !REXXDW.!DW_FONT_BOLD )
num_rows = 3 /* number of widgets + 1 for gaps */
Call dw_box_pack_start topbox, groupbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
/* 1st row box */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start groupbox, tmpbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/* left column box */
left_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start tmpbox, left_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/* right column box */
right_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start tmpbox, right_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/* quick exit - left */
confirm_exit_cb = dw_checkbox_new( 'Confirmation on Exit', 0 )
Call dw_box_pack_start left_box, confirm_exit_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!!user.!!preference.!!confirm_exit = 1 Then Call dw_checkbox_set confirm_exit_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set confirm_exit_cb, !REXXDW.!DW_UNCHECKED
/* save position on exit - right */
save_position_cb = dw_checkbox_new( 'Use saved position on start', 0 )
Call dw_box_pack_start right_box, save_position_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!!user.!!preference.!!save_position = '1' Then Call dw_checkbox_set save_position_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set save_position_cb, !REXXDW.!DW_UNCHECKED
/* 2nd row box */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start groupbox, tmpbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/* left column box */
left_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start tmpbox, left_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/* sorting - left */
sort_cb = dw_checkbox_new( 'Case Sensitive Sorting', 0 )
Call dw_box_pack_start left_box, sort_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!!user.!!preference.!!sort = 'sensitive' Then Call dw_checkbox_set sort_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set sort_cb, !REXXDW.!DW_UNCHECKED
/* right column box */
right_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start tmpbox, right_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/* debug commands - right */
debug_commands_cb = dw_checkbox_new( 'Debug Repository Commands', 0 )
Call dw_box_pack_start right_box, debug_commands_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!!user.!!preference.!!debug_commands = 1 Then Call dw_checkbox_set debug_commands_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set debug_commands_cb, !REXXDW.!DW_UNCHECKED
/* win32 file associations - full width */
If !global.!os = 'WIN32' Then
   Do
      tmpbox = dw_groupbox_new( !REXXDW.!DW_VERT, 'Use Windows File Associations', !REXXDW.!DW_FONT_BOLD )
      Call dw_box_pack_start topbox, tmpbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
      fa_cb = dw_checkbox_new( 'For all files', 0 )
      Call dw_box_pack_start tmpbox, fa_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      /*
       * On first start of QOCCA, set Windows File Associations to true under WIN32
       */
      If initial Then
         Do
            Call dw_checkbox_set fa_cb, !REXXDW.!DW_CHECKED
         End
      Else
         Do
            If !global.!!user.!!preference.!!windowsfileassociations = 1 Then Call dw_checkbox_set fa_cb, !REXXDW.!DW_CHECKED
            Else Call dw_checkbox_set fa_cb, !REXXDW.!DW_UNCHECKED
         End
      fa_box = dw_box_new( !REXXDW.!DW_HORZ )
      Call dw_box_pack_start tmpbox, fa_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
      tmp = dw_text_new( 'Only for:', 0 )
      Call dw_box_pack_start fa_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
      If Left( !global.!!user.!!preference.!!windowsfileassociations_extensions, 1 ) = '!' Then oldvalue = ''
      Else oldvalue = !global.!!user.!!preference.!!windowsfileassociations_extensions
      fa_entry = dw_entryfield_new( oldvalue, 0 )
      Call dw_box_pack_start fa_box, fa_entry, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_signal_connect fa_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'UserPreferencesHelpFocusCallback', 'FILEASSOC', t1
   End
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
Call dw_window_set_gravity win, !REXXDW.!DW_GRAV_CENTRE, !REXXDW.!DW_GRAV_CENTRE
Call dw_window_set_pos_size win, 0, 0, 0, 0
Call dw_window_show win
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'UserPreferencesApplyCallback', win, dialog_wait, text_editor_entry, text_pager_entry, tmp_dir_entry, binary_editor_entry, binary_pager_entry, fa_cb, fa_entry, sort_cb, stylebox, sb, diff_program_entry, confirm_exit_cb, text_editor_cb, binary_editor_cb, text_pager_cb, binary_pager_cb, save_position_cb, debug_commands_cb
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End

Return 0

SelectStyleCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, list_idx
If list_idx < 0 Then Return 1
text = dw_listbox_get_text( win, list_idx )
Call dw_window_set_text win, text
Return 1

ModifyUserMenuCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Display the current list of user file menus
 */
/*
 * Create a container box with
 * +---------------------------------------------------+
 * | Specify the ... Use %F% for the full filename     |
 * +---------------------------------------------------+
 * | Label  |  Command                                 |
 * +---------------------------------------------------+
 * | the    |  the %F%                               |^|
 * | xedit  |  xedit %F%                             |v|
 * +---------------------------------------------------+
 * | xyx    |                                  | Save  |
 * +---------------------------------------------------+
 * |               | Change |   | Delete |             |
 * +---------------------------------------------------+
 * |                      Cancel                       |
 * +---------------------------------------------------+
 * |                      Apply                        |
 * +---------------------------------------------------+
 * and provide right-click menu with 'Change' and 'Delete'
 *
 */
Parse Arg ., type .
title_width = 50
flags.0 = 2
flags.1 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
flags.2 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
titles.0 = 2
titles.1 = 'Menu Label'
titles.2 = 'Command'
title.DIR = 'Maintain Directory Menu'
title.FILE = 'Maintain File Menu'
help.DIR.1 = 'This dialog allows you to add your own menus to the directory menu.'
help.DIR.2 = ' Specify the menu label and command to execute. The fully qualified'
help.DIR.3 = ' working directory is available for substitution into your command.'
help.DIR.4 = ' Add %D% where you want the directory name to be substituted, and %W% where you want the working directory substituted.'
help.DIR.5 = ' User variables can also be substituted; use %xx% where xx is a string. A dialog box will popup asking for the values.'
help.DIR.6 = ' Use %%% to specify a percent sign.'
help.DIR.0 = 6
help.FILE.1 = 'This dialog allows you to add your own menus to the file menu.'
help.FILE.2 = ' Specify the menu label and command to execute. The fully qualified'
help.FILE.3 = ' working directory and file name is available for substitution into your command.'
help.FILE.4 = ' Add %D% where you want the directory name to be substituted, %W% where you want the working directory substituted, and'
help.FILE.5 = ' %F% where you want the file name to be substituted.'
help.FILE.6 = ' User variables can also be substituted; use %xx% where xx is a string. A dialog box will popup asking for the values.'
help.FILE.7 = ' Use %%% to specify a percent sign.'
help.FILE.0 = 7
If type = 'DIR' Then
   Do
      If Left( !global.!!user.!!dirmenu.!!label.0, 1 ) = '!' Then
         Do
            !global.!!user.!!dirmenu.!!label.0 = 0
            !global.!!user.!!dirmenu.!!command.0 = 0
         End
      Call SysStemCopy '!global.!!user.!!dirmenu.!!label.', '!global.!current_menu_label.'
      Call SysStemCopy '!global.!!user.!!dirmenu.!!command.', '!global.!current_menu_command.'
   End
Else
   Do
      If Left( !global.!!user.!!filemenu.!!label.0, 1 ) = '!' Then
         Do
            !global.!!user.!!filemenu.!!label.0 = 0
            !global.!!user.!!filemenu.!!command.0 = 0
         End
      Call SysStemCopy '!global.!!user.!!filemenu.!!label.', '!global.!current_menu_label.'
      Call SysStemCopy '!global.!!user.!!filemenu.!!command.', '!global.!current_menu_command.'
   End
win = dw_window_new( !REXXDW.!DW_DESKTOP, title.type, !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, !REXXDW.!DW_SIZE_AUTO, 200, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
res = -1
Do i = 1 To help.type.0
   res = dw_mle_import( t1, help.type.i, res )
End
Call dw_window_disable t1
/*
 * Create our container in the box...
 */
!global.!usermenucontainer = dw_container_new( 1, !REXXDW.!DW_SINGLE_SELECTION )
Call dw_box_pack_start topbox, !global.!usermenucontainer, 400, 300, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
!global.!usermenucontainer.?firsttime = 1
/*
 * Use a normal container.
 */
Call dw_container_setup !global.!usermenucontainer, 'flags.', 'titles.', 0

Call DisplayUserMenu
/*
 * Create edit area
 */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, tmpbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
!global.!usermenu_text = dw_entryfield_new( '', 0 )
Call dw_box_pack_start tmpbox, !global.!usermenu_text, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
!global.!usermenu_entry = dw_entryfield_new( '', 0 )
Call dw_box_pack_start tmpbox, !global.!usermenu_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
ba = dw_button_new( 'Add/Change', 0 )
Call dw_box_pack_start tmpbox, ba, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
bd = dw_button_new( 'Delete', 0 )
Call dw_box_pack_start tmpbox, bd, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

/*
 * Create Cancel and Apply button
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
Call dw_window_set_gravity win, !REXXDW.!DW_GRAV_CENTRE, !REXXDW.!DW_GRAV_CENTRE
Call dw_window_set_pos_size win, 0, 0, 0, 0
Call dw_window_show win
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
    */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'UserMenuApplyCallback', win, dialog_wait, type
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect ba, !REXXDW.!DW_CLICKED_EVENT, 'UserMenuAddChangeButtonCallback'
   Call dw_signal_connect bd, !REXXDW.!DW_CLICKED_EVENT, 'UserMenuDeleteButtonCallback'
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
Return 1

DisplayUserMenu: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Delete everything in the container so we can replace the existing data
 */
If !global.!usermenucontainer.?firsttime = 0 Then
   Do
      Call dw_signal_disconnect !global.!usermenucontainer, !REXXDW.!DW_ITEM_SELECT_EVENT
      Call dw_container_clear !global.!usermenucontainer, !REXXDW.!DW_DONT_REDRAW
   End
!global.!usermenucontainer.?firsttime = 0
/*
 * Add the entries into the container
 */
containermemory = dw_container_alloc( !global.!usermenucontainer, !global.!current_menu_label.0 )
Do i = 1 To !global.!current_menu_label.0
   Call dw_container_set_item !global.!usermenucontainer, containermemory, 0, i-1, !global.!current_menu_label.i
   Call dw_container_set_item !global.!usermenucontainer, containermemory, 1, i-1, !global.!current_menu_command.i
   Call dw_container_set_row_title containermemory, i-1, i
End
Call dw_container_set_stripe !global.!usermenucontainer, !REXXDW.!DW_CLR_WHITE, !global.!container_colour
Call dw_container_insert !global.!usermenucontainer, containermemory, !global.!current_menu_label.0
Call dw_container_optimize !global.!usermenucontainer
/*
 * Only now can we connect our signals, because dw_set_file() causes the signals to fire!!!...
 */
Call dw_signal_connect !global.!usermenucontainer, !REXXDW.!DW_ITEM_SELECT_EVENT, 'UserMenuSelectCallback'

Return

/*
 * Set our edit are with values from the currently selected item
 */
UserMenuSelectCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg window, ., idx
Call dw_window_set_text !global.!usermenu_text, !global.!current_menu_label.idx
Call dw_window_set_text !global.!usermenu_entry, !global.!current_menu_command.idx
Return 1

/*
 * Set our edit are with values from the currently selected item
 */
UserMenuDeleteButtonCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * If we don't have anything in the edit area yet, just return
 */
label = Strip( dw_window_get_text( !global.!usermenu_text ) )
If label = '' Then Return 0
/*
 * If we have the label in the array delete it, otherwise ignore the delete
 */
idx = 0
Do i = 1 To !global.!current_menu_label.0
   If !global.!current_menu_label.i = label Then
      Do
         idx = i
         Leave
      End
End
If idx = 0 Then Return 1
Call SysStemDelete '!global.!current_menu_label.', idx
Call SysStemDelete '!global.!current_menu_command.', idx
/*
 * Redisplay the container
 */
Call DisplayUserMenu
Return 1

UserMenuAddChangeButtonCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * If we don't have anything in the edit area yet, just return
 */
label = Strip( dw_window_get_text( !global.!usermenu_text ) )
If label = '' Then Return 0
/*
 * If we don't have the label in the array already, add it, otherwise
 * update the value
 */
idx = 0
Do i = 1 To !global.!current_menu_label.0
   If !global.!current_menu_label.i = label Then
      Do
         idx = i
         Leave
      End
End
command = Strip( dw_window_get_text( !global.!usermenu_entry ) )
If idx = 0 Then
   Do
      Call SysStemInsert '!global.!current_menu_label.', 1+!global.!current_menu_label.0, label
      Call SysStemInsert '!global.!current_menu_command.', 1+!global.!current_menu_command.0, command
   End
Else
   Do
      !global.!current_menu_label.idx = label
      !global.!current_menu_command.idx = command
   End
/*
 * Redisplay the container
 */
Call DisplayUserMenu
Return 0

/*
 * Apply the changes for the menu permanently
 */
UserMenuApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., win, dialog, type
If type = 'DIR' Then
   Do
      Call SysStemCopy '!global.!current_menu_label.',   '!global.!!user.!!dirmenu.!!label.'
      Call SysStemCopy '!global.!current_menu_command.', '!global.!!user.!!dirmenu.!!command.'
   End
Else
   Do
      Call SysStemCopy '!global.!current_menu_label.',   '!global.!!user.!!filemenu.!!label.'
      Call SysStemCopy '!global.!current_menu_command.', '!global.!!user.!!filemenu.!!command.'
   End
/*
 * Save the user preferences now
 */
Call WriteUserSettingsFile
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Destroy the window
 */
Call dw_window_destroy win
Return 0


UserPreferencesApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., win, dialog, text_editor_entry, text_pager_entry, tmp_dir_entry, binary_editor_entry, binary_pager_entry, fa_cb, fa_entry, sort_cb, stylebox, sb, diff_program_entry, confirm_exit_cb, text_editor_cb, binary_editor_cb, text_pager_cb, binary_pager_cb, save_position_cb, debug_commands_cb
/*
 * Validate that all mandatory fields are supplied
 * If on WIN32 and Windows File Associations checked, then we don't need
 * editors or pagers
 */
t_editor = Strip( dw_window_get_text( text_editor_entry ) )
t_pager = Strip( dw_window_get_text( text_pager_entry ) )
b_editor = Strip( dw_window_get_text( binary_editor_entry ) )
b_pager = Strip( dw_window_get_text( binary_pager_entry ) )
diff_prog = Strip( dw_window_get_text( diff_program_entry ) )
If stylebox \= '' Then style = Strip( dw_window_get_text( stylebox ) )
Else style = ''
strict_validation = 1
t_tmpdir = Strip( Strip( Changestr( !global.!ossep, dw_window_get_text( tmp_dir_entry ), '/' ) ), 'T', '/' )
If !global.!os = 'WIN32' Then
   Do
      fa_ext = Strip( dw_window_get_text( fa_entry ) )
      fa = dw_checkbox_get( fa_cb )
      If fa Then strict_validation = 0
      Else strict_validation = 1
   End
Select
   When t_editor = '' & strict_validation Then
      Do
         Call dw_messagebox 'Validation Error', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Text Editor field is empty. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When b_editor = '' & strict_validation Then
      Do
         Call dw_messagebox 'Validation Error', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Binary Editor field is empty. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When t_pager = '' & strict_validation Then
      Do
         Call dw_messagebox 'Validation Error', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Text Viewer field is empty. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When b_pager = '' & strict_validation Then
      Do
         Call dw_messagebox 'Validation Error', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Binary Viewer field is empty. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When t_tmpdir = '' | DirectoryExists(t_tmpdir) = 0 Then
      Do
         Call dw_messagebox 'Validation Error', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), "Temp Dir is empty or doesn't exist. Please re-enter."
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   Otherwise Nop
End
/*
 * Check text editor has %F%
 */
If Countstr( '%F', t_editor ) \= 1 & strict_validation Then
   Do
      Call dw_messagebox 'Validation Error', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Text Editor field does not have mandatory %F% placemarker. Please re-enter.'
      Call dw_dialog_dismiss dialog, 'retry'
      Return 0
   End
/*
 * Append !global.!ossep to !!tmpdir if it doesn't have a trailing slash
 */
If Right( t_tmpdir, 1 ) \= '/' Then !global.!!tmpdir = t_tmpdir||'/'
Else !global.!!tmpdir = t_tmpdir
!global.!!user.!!texteditor = t_editor
!global.!!user.!!textpager = t_pager
!global.!!user.!!binaryeditor = b_editor
!global.!!user.!!binarypager = b_pager
If dw_checkbox_get( sort_cb ) Then !global.!!user.!!preference.!!sort = 'sensitive'
Else !global.!!user.!!preference.!!sort = 'insensitive'
!global.!!user.!!preference.!!confirm_exit = dw_checkbox_get( confirm_exit_cb )
!global.!!user.!!preference.!!save_position = dw_checkbox_get( save_position_cb )
!global.!!user.!!preference.!!debug_commands = dw_checkbox_get( debug_commands_cb )
If !global.!os = 'WIN32' Then !global.!!user.!!preference.!!windowsfileassociations = fa
Else !global.!!user.!!preference.!!windowsfileassociations = 0
!global.!!user.!!preference.!!tabspaces = dw_spinbutton_get_pos( sb )
!global.!!user.!!preference.!!showmodified = 1
!global.!!user.!!style = style
!global.!!user.!!diffprog = diff_prog
!global.!!user.!!preference.!!windowsfileassociations_extensions = fa_ext
!global.!!user.!!texteditor_console = dw_checkbox_get( text_editor_cb )
!global.!!user.!!textpager_console = dw_checkbox_get( text_pager_cb )
!global.!!user.!!binaryeditor_console = dw_checkbox_get( binary_editor_cb )
!global.!!user.!!binarypager_console = dw_checkbox_get( binary_pager_cb )
/*
 * Save the user preferences now
 */
Call WriteUserSettingsFile
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Destroy the window
 */
Call dw_window_destroy win
Return 0

UserPreferencesHelpFocusCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, which, textwin
help.initial = 'Specify your preferences below.'
help.texteditor = 'Enter the command you wish to execute to Edit a text configuration item, or use the folder icon to select a program.',
                  ' You must also include %F% to specify where filenames are the be included in the command string.  You can optionally add %L% and %C% to specify line and column positions when the editor is called from the Build Results window.'
help.textpager = 'Enter the command you wish to execute to View a text configuration item, or use the folder icon to select a program.'
help.binaryeditor = 'Enter the command you wish to execute to Edit a binary configuration item, or use the folder icon to select a program.'
help.binarypager = 'Enter the command you wish to execute to View a binary configuration item, or use the folder icon to select a program.'
help.tmpdir = 'Enter a directory where temporary files used by QOCCA can be stored, or use the folder icon to select a directory. This temporary directory should not be a directory that other developers are likely to use.'
help.fileassoc = 'Check the button if you want to use the default Windows File Associations for editing and viewing instead of the selected programs.'
help.sorting = 'Check the button if you want the order of the files in the files pane to respect case.'
help.showmodified = "Check the button if you want QOCCA to execute 'Show Modified' automatically each time a new file list is displayed."
help.setcolours = "Click this to specify the foreground and background colours of the 3 major panes."
help.filemenus = "Click this to bring up a dialog that allows you to create your own menu items for the File Menu."
help.dirmenus = "Click this to bring up a dialog that allows you to create your own menu items for the Directory Menu."
help.sytnax = "Select the default theme to use in 'Show Annotations' and 'Show Differences'."
help.tabspaces = "Replace TABS in 'Show Annotations' and 'Show Differences' with this many spaces. 0 indicates no substitution."
help.diffprog = "Specify the 'diff' program to call for 'Show Differences'. If not specifed, 'rexxdiff ' is used, where %F1% and %F2% are the file names to compare, %T1% and %T2% are the file descriptions."
help.fileassoc = "Specify the extensions to apply the default file associations to."
Call dw_window_enable textwin
Call dw_mle_set_cursor textwin, 0
Call dw_mle_delete textwin
Call dw_mle_import textwin, help.which, -1
Call dw_window_disable textwin
Return 0

RepositorySettingsCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Project properties... (from build.conf)
 *
 * +---------------------------------------------------+
 * | Text                                              |
 * |                                                   |
 * +---------------------------------------------------+
 * |  Admin Users: ______________________              | !admin_users
 * |  Admin Email: ______________________              | !admin_email
 * |  Build Users: _______________________             | !build_users
 * |  Update Users: ______________________             | !update_users
 * |  User mappings: _____________________             | !user_mappings
 * |  Runtime Environments: ______________________     | !runtime_envs
 * |  Platform Targets: __________________             | !platforms
 * |  Files to Ignore: ___________________             | !ignorefiles
 * |  Binary files: __ ___________________             | !binaryfiles
 * |  Version Env Variable _______________             | !versionenvvar
 * |  Build Number Env Variable _______________        | !buildenvvar
 * |  Build Directory:    ____________________________ | !builddir
 * |  Log Directory:      ____________________________ | !logdir
 * |  User Can Delete: x                               | !user_can_delete
 * |  Reserved Checkout: x                             | !reservedcheckout
 * +---------------------------------------------------+
 * | Help                                              |
 * |                                                   |
 * +---------------------------------------------------+
 */
this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
num_platforms = Words( !global.!platforms )

title_width = 130
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Edit Repository Settings', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, 400, 75, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call UserPreferencesHelpFocusCallback t1, 'INITIAL', t1
Call dw_window_disable t1

/*
 * Add a vertical box for the left column of widgets
 */
left_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start topbox, left_box, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 5
/*
 * Add the boxes and widgets for the left column...
 */
/* admin users */
admin_users_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, admin_users_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Admin Users', 0 )
Call dw_box_pack_start admin_users_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
admin_users_entry = dw_entryfield_new( !global.!admin_users, 0 )
Call dw_box_pack_start admin_users_box, admin_users_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect admin_users_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'AU', t1
/* admin email */
admin_email_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, admin_email_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Admin Email', 0 )
Call dw_box_pack_start admin_email_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
admin_email_entry = dw_entryfield_new( !global.!admin_email, 0 )
Call dw_box_pack_start admin_email_box, admin_email_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect admin_email_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'AE', t1
/* build users */
build_users_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, build_users_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Build Users', 0 )
Call dw_box_pack_start build_users_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
build_users_entry = dw_entryfield_new( !global.!build_users, 0 )
Call dw_box_pack_start build_users_box, build_users_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect build_users_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'BU', t1
/* update users */
update_users_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, update_users_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Update Users', 0 )
Call dw_box_pack_start update_users_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
update_users_entry = dw_entryfield_new( !global.!update_users, 0 )
Call dw_box_pack_start update_users_box, update_users_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect update_users_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'UU', t1
/* update users */
user_mappings_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, user_mappings_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'User Mappings', 0 )
Call dw_box_pack_start user_mappings_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
user_mappings_entry = dw_entryfield_new( !global.!user_mappings, 0 )
Call dw_box_pack_start user_mappings_box, user_mappings_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect user_mappings_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'UM', t1
/* runtime environments */
runtime_environments_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, runtime_environments_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Runtime Environments', 0 )
Call dw_box_pack_start runtime_environments_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
runtime_environments_entry = dw_entryfield_new( !global.!runtime_envs, 0 )
Call dw_box_pack_start runtime_environments_box, runtime_environments_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect runtime_environments_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'RE', t1
/* platform targets */
platform_targets_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, platform_targets_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Platform Targets', 0 )
Call dw_box_pack_start platform_targets_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
platform_targets_entry = dw_entryfield_new( !global.!platforms, 0 )
Call dw_box_pack_start platform_targets_box, platform_targets_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect platform_targets_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'PT', t1
/* files to ignore */
files_to_ignore_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, files_to_ignore_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Files To Ignore', 0 )
Call dw_box_pack_start files_to_ignore_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
files_to_ignore_entry = dw_entryfield_new( !global.!ignorefiles, 0 )
Call dw_box_pack_start files_to_ignore_box, files_to_ignore_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect files_to_ignore_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'IF', t1
/* binary files */
binary_files_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, binary_files_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Binary Files', 0 )
Call dw_box_pack_start binary_files_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
binary_files_entry = dw_entryfield_new( !global.!binaryfiles, 0 )
Call dw_box_pack_start binary_files_box, binary_files_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect binary_files_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'BF', t1
/* variable env variable */
version_env_var_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, version_env_var_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Version Env Variable', 0 )
Call dw_box_pack_start version_env_var_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
version_env_var_entry = dw_entryfield_new( !global.!versionenvvar, 0 )
Call dw_box_pack_start version_env_var_box, version_env_var_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect version_env_var_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'VE', t1
/* build env variable */
build_env_var_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, build_env_var_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Build Env Variable', 0 )
Call dw_box_pack_start build_env_var_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
build_env_var_entry = dw_entryfield_new( !global.!buildenvvar, 0 )
Call dw_box_pack_start build_env_var_box, build_env_var_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect build_env_var_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'BE', t1
/* build dir variable */
build_dir_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, build_dir_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Build Directory', 0 )
Call dw_box_pack_start build_dir_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
build_dir_entry = dw_entryfield_new( !global.!builddir, 0 )
Call dw_box_pack_start build_dir_box, build_dir_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect build_dir_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'BD', t1
/* log env variable */
log_dir_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, log_dir_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Log Directory', 0 )
Call dw_box_pack_start log_dir_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
log_dir_entry = dw_entryfield_new( !global.!logdir, 0 )
Call dw_box_pack_start log_dir_box, log_dir_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect log_dir_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'RepositorySettingsHelpFocusCallback', 'LD', t1
/* syntax mapping */
b0 = dw_button_new( 'Edit Syntax Mappings', 0 )
Call dw_box_pack_start left_box, b0, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b0, !REXXDW.!DW_CLICKED_EVENT, 'EditSyntaxMappingCallback'
/* user can delete */
user_can_delete_cb = dw_checkbox_new( 'User Can Delete', 0 )
Call dw_box_pack_start left_box, user_can_delete_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!user_can_delete = 'Y' Then Call dw_checkbox_set user_can_delete_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set user_can_delete_cb, !REXXDW.!DW_UNCHECKED
/* reserved checkout */
reserved_checkout_cb = dw_checkbox_new( 'Reserved Checkout for Repository', 0 )
Call dw_box_pack_start left_box, reserved_checkout_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!reservedcheckout = 'Y' Then Call dw_checkbox_set reserved_checkout_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set reserved_checkout_cb, !REXXDW.!DW_UNCHECKED
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
width = 500
height = 590  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
Call dw_main_sleep 10
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'RepositorySettingsApplyCallback', win, dialog_wait, admin_users_entry, admin_email_entry, build_users_entry, update_users_entry, runtime_environments_entry, platform_targets_entry, files_to_ignore_entry, binary_files_entry, version_env_var_entry, build_env_var_entry, build_dir_entry, log_dir_entry, user_can_delete_cb, reserved_checkout_cb, user_mappings_entry
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
Return 0

RepositorySettingsApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., win, dialog_wait, admin_users_entry, admin_email_entry, build_users_entry, update_users_entry, runtime_environments_entry, platform_targets_entry, files_to_ignore_entry, binary_files_entry, version_env_var_entry, build_env_var_entry, build_dir_entry, log_dir_entry, user_can_delete_cb, reserved_checkout_cb, user_mappings_entry

admin_users = Strip( dw_window_get_text( admin_users_entry ) )
admin_email = Strip( dw_window_get_text( admin_email_entry ) )
If Words( admin_users ) \= Words( admin_email ) Then
   Do
      Call dw_messagebox 'Validation Error', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), 'There must be an email address for each Admin User.'
      Call dw_dialog_dismiss dialog_wait, 'retry'
      Return 0
   End
/*
 * No more validation done
 */
!global.!admin_users = admin_users
!global.!admin_email = admin_email
!global.!user_mappings = Strip( dw_window_get_text( user_mappings_entry ) )
!global.!build_users = Strip( dw_window_get_text( build_users_entry ) )
!global.!update_users = Strip( dw_window_get_text( update_users_entry ) )
!global.!runtime_envs = Strip( dw_window_get_text( runtime_environments_entry ) )
!global.!platforms = Strip( dw_window_get_text( platform_targets_entry ) )
!global.!ignorefiles = Strip( dw_window_get_text( files_to_ignore_entry ) )
!global.!binaryfiles = Strip( dw_window_get_text( binary_files_entry ) )
!global.!versionenvvar = Strip( dw_window_get_text( version_env_var_entry ) )
!global.!buildenvvar = Strip( dw_window_get_text( build_env_var_entry ) )
!global.!builddir = Strip( dw_window_get_text( build_dir_entry ) )
!global.!logdir = Strip( dw_window_get_text( log_dir_entry ) )
ucd = dw_checkbox_get( user_can_delete_cb )
If ucd Then !global.!user_can_delete = 'Y'
Else !global.!user_can_delete = 'N'
If dw_checkbox_get( reserved_checkout_cb ) Then !global.!reservedcheckout = 'Y'
Else !global.!reservedcheckout = 'N'

Call dw_dialog_dismiss dialog_wait, 'apply'
Call dw_window_destroy win

Call WriteBuildConf 'Updated repository settings'
/*
 * In case we have changed to/from reserved checkout
 */
Call SetLockedStatusIcon

Return 0

RepositorySettingsHelpFocusCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, which, textwin
help.initial = 'Update the repository details below.'
help.au = 'Enter the names of all users who are allowed to administer this repository separated by spaces.'
help.ae = 'Enter the email address for each admin user. There must be a one-to-one relationship between admin users and their email addresses separated by spaces.'
help.bu = 'Enter the names of all users who are allowed to build modules in this repository separated by spaces.'
help.uu = 'Enter the names of all users who are allowed to update configuration items in this repository separated by spaces.'
help.um = 'Enter the userid mappings in the format: userid=new_userid[,userid=new_userid].'
help.re = 'Enter the list of environments to which a module can be promoted (not currently used).'
help.pt = 'Enter the platform on which builds are required for this repository.'
help.if = 'Enter the list of file extensions that you want to ignore when adding files to the repository.'
help.bf = 'Enter the list of file extensions that you QOCCA to consider to be binary.'
help.ve = 'Enter the environment variable that you want to be set to the version being built.'
help.be = 'Enter the environment variable that you want to be set to the build number being built.'
help.bd = "Enter the directory where a build is to be done from. This directory is a relative directory specification from the repository's working directory; generally something as simple as 'build'."
help.ld = "Enter the directory where log files from batch jobs are written. This directory is a relative directory specification from the repository's working directory; generally something as simple as 'log'."
Call dw_mle_set_cursor textwin, 0
Call dw_mle_delete textwin
Call dw_mle_import textwin, help.which, -1
Return 0

EditSyntaxMappingCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Create a container box with
 * +---------------------------------------------------+
 * | Syntax | Extensions                               |
 * +---------------------------------------------------+
 * | xml    |                                        |^|
 * | rexx   | rexx rexx cmd the                      |v|
 * +---------------------------------------------------+
 * | xml    |                                  | Save  |
 * +---------------------------------------------------+
 * |                    Apply                          |
 * +---------------------------------------------------+
 * |                    Cancel                         |
 * +---------------------------------------------------+
 *
 */
title_width = 50
Address System 'highlight -p' !global.!datadir With Output Stem langs. Error Stem err.
If langs.0 = 0 Then
   Do
      Do i = 1 To err.0
      End
      Return 0
   End
flags.0 = 2
flags.1 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
flags.2 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
titles.0 = 2
titles.1 = 'Syntax'
titles.2 = 'Extensions'
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Edit Syntax Mappings', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Create our container in the box...
 */
!global.!syntaxcontainer = dw_container_new( 1, !REXXDW.!DW_SINGLE_SELECTION )
Call dw_box_pack_start topbox, !global.!syntaxcontainer, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
!global.!syntaxcontainer.?firsttime = 1
!global.!editsyntax_index = 0
/*
 * Use a normal container.
 */
Call dw_container_setup !global.!syntaxcontainer, 'flags.', 'titles.', 0

Call DisplaySyntaxMapping
/*
 * Create edit area
 */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
!global.!editsyntax_text = dw_entryfield_new( '', 0 )
Call dw_box_pack_start tmpbox, !global.!editsyntax_text, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_disable !global.!editsyntax_text
!global.!editsyntax_entry = dw_entryfield_new( '', 0 )
Call dw_box_pack_start tmpbox, !global.!editsyntax_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
b0 = dw_button_new( 'Save', 0 )
Call dw_box_pack_start tmpbox, b0, 50, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Create Apply and Cancel buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
width = 500
height = 480  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
    */
   Call dw_signal_connect b0, !REXXDW.!DW_CLICKED_EVENT, 'EditSyntaxMappingSaveButtonCallback'
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'EditSyntaxMappingApplyCallback', win, dialog_wait
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
Drop !global.!langs. !global.!extensions.
Return 0

DisplaySyntaxMapping: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Determine how many items in the container...
 */
Address System 'highlight -p' !global.!datadir With Output Stem langs. Error Stem err.
idx = 0
Do i = 1 To langs.0
   Parse Var langs.i . ':' ext .
   ext = Strip( ext )
   If ext \= '' Then
      Do
         idx = idx + 1
         !global.!langs.idx = ext
         !global.!langs.0 = idx
      End
End
Drop langs.i
If !global.!langs.0 > 1 Then Call SysStemSort '!GLOBAL.!LANGS.','ascending', , , , 1, 20
/*
 * Delete everything in the container so we can replace the existing files
 */
If !global.!syntaxcontainer.?firsttime = 0 Then
   Do
      Call dw_signal_disconnect !global.!syntaxcontainer, !REXXDW.!DW_ITEM_SELECT_EVENT
      Call dw_container_clear !global.!syntaxcontainer, !REXXDW.!DW_DONT_REDRAW
   End
!global.!syntaxcontainer.?firsttime = 0
/*
 * Add the entries into the container
 */
!global.!syntaxcontainermemory = dw_container_alloc( !global.!syntaxcontainer, !global.!langs.0 )
Do i = 1 To !global.!langs.0
   /*
    * Find our currently set mappings...
    */
   !global.!extensions.i = ''
   If Left( !global.!syntax.0, 1 ) \= '!' Then
      Do j = 1 To !global.!syntax.0
         If !global.!syntax.j = !global.!langs.i Then
            Do
               !global.!extensions.i = !global.!extension.j
               Leave j
            End
      End
   Call dw_container_set_item !global.!syntaxcontainer, !global.!syntaxcontainermemory, 0, i-1, !global.!langs.i
   Call dw_container_set_item !global.!syntaxcontainer, !global.!syntaxcontainermemory, 1, i-1, !global.!extensions.i
   Call dw_container_set_row_title !global.!syntaxcontainermemory, i-1, i
End
Call dw_container_set_stripe !global.!syntaxcontainer, !REXXDW.!DW_CLR_WHITE, !global.!container_colour
Call dw_container_insert !global.!syntaxcontainer, !global.!syntaxcontainermemory, !global.!langs.0
Call dw_container_optimize !global.!syntaxcontainer
/*
 * Only now can we connect our signals, because dw_set_file() causes the signals to fire!!!...
 */
Call dw_signal_connect !global.!syntaxcontainer, !REXXDW.!DW_ITEM_SELECT_EVENT, 'EditSyntaxMappingSelectCallback'

Return

/*
 * Set our edit area with values from the currently selected item
 */
EditSyntaxMappingSelectCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg window, ., idx
Call dw_window_set_text !global.!editsyntax_text, !global.!langs.idx
Call dw_window_set_text !global.!editsyntax_entry, !global.!extensions.idx
!global.!editsyntax_index = idx
Return 1

EditSyntaxMappingSaveButtonCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * If we don't have anything in the edit area yet, just return
 */
If !global.!editsyntax_index = 0 Then Return 0
index = !global.!editsyntax_index
text = dw_window_get_text( !global.!editsyntax_entry )
!global.!extensions.index = text
Call dw_container_set_item !global.!syntaxcontainer, !global.!syntaxcontainermemory, 1, index-1, !global.!extensions.index
Return 0

EditSyntaxMappingApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., win, dialog_wait
/*
 * Convert !global.!langs. and !global.!extensions. to !global.!syntax. and !global.!extension.
 */
idx = 0
Do i = 1 To !global.!langs.0
   If Strip( !global.!extensions.i ) \= '' Then
      Do
         idx = idx + 1
         !global.!syntax.idx = Strip( !global.!langs.i )
         !global.!extension.idx = Strip( !global.!extensions.i )
      End
End
!global.!syntax.0 = idx
!global.!extension.0 = idx
Call dw_dialog_dismiss dialog_wait, 'apply'
Call dw_window_destroy win

Call WriteBuildConf 'Updated syntax mappings'

Return 0

CreateMainWindow: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Create the main window on the desktop
 * Check if the user wants the window to be the same as last time and it was maixised...
 */
mainstyle = !global.!windowstyle
maximised = 0
If Words( !global.!!user.!!window_pos_size ) = 4 & !global.!!user.!!preference.!!save_position = 1 Then
   Do
      Parse Var !global.!!user.!!window_pos_size xpos ypos !global.!window_width !global.!window_height
      If !global.!screen_width = !global.!window_width - xpos & !global.!screen_height = !global.!window_height - ypos Then
         Do
            mainstyle = dw_or( mainstyle, !REXXDW.!DW_FCF_MAXIMIZE )
            maximised = 1
         End
   End
lockicon_size = 26
!global.!mainwindow = dw_window_new( !REXXDW.!DW_DESKTOP, 'QOCCA', mainstyle )
/*
 * Generate all images used by QOCCA. We have to do this here because dwindows
 * under GTK 1.2 NOT using IMLIB, requires a window before an icon can be
 * loaded
 */
Call SetCursorWait
Call GenerateIcons
Call dw_window_set_icon !global.!mainwindow, !global.!qoccaicon
/*
 * ...and a box within the main window.
 */
!global.!topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start !global.!mainwindow, !global.!topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * This box is where the bulk of the windows goes
 */
!global.!mainbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!topbox, !global.!mainbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * This box is for our bookmarks
 */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!topbox, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
/*
 * Create our bookmark details
 */
tmp = dw_button_new( 'Go to bookmark', 0 )
Call dw_box_pack_start tmpbox, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect tmp, !REXXDW.!DW_CLICKED_EVENT, 'SelectBookmarkApplyCallback'
!global.!bookmarks_cb = dw_combobox_new( '', 0 )
Call dw_box_pack_start tmpbox, !global.!bookmarks_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!bookmarks_cb, !REXXDW.!DW_LIST_SELECT_EVENT, 'SelectBookmarkCallback'
/*
 * Create our file filters
 */
!global.!filefilterbuttonapply = dw_button_new( 'Apply Filters', 0 )
Call dw_box_pack_start tmpbox, !global.!filefilterbuttonapply, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!filefilterbuttonapply, !REXXDW.!DW_CLICKED_EVENT, 'RefreshFilesCallback'
Call dw_box_pack_start tmpbox, , 5, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
ffvbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start tmpbox, ffvbox, 0, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Do i = 1 To !global.!filefiltericons.0
--   !global.!filefilterbutton.i = dw_bitmapbutton_new_from_file( !global.!filefiltericonsdesc.i, 0, !global.!icondir||!global.!filefiltericons.i )
   !global.!filefilterbutton.i = dw_bitmapbutton_new_from_file( '', 0, !global.!icondir||!global.!filefiltericons.i )
   Call dw_box_pack_start ffvbox, !global.!filefilterbutton.i, 24, 24, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   Call dw_signal_connect !global.!filefilterbutton.i, !REXXDW.!DW_CLICKED_EVENT, 'ToggleFileFilterCallback', i
End
Call DisableFileFilters
/*
 * This box is for the status line.
 */
!global.!statusbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!topbox, !global.!statusbox, 0, 26, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
/*
 * Create our status areas...
 */
junk = dw_button_new( 'QOCCA:' !global.!version, 0 )
Call dw_box_pack_start !global.!statusbox, junk, !REXXDW.!DW_SIZE_AUTO, 26, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect junk, !REXXDW.!DW_CLICKED_EVENT, 'HelpAboutCallback'
!global.!systemstatus = dw_status_text_new( 'QOCCA Starting', 0 )
Call dw_box_pack_start !global.!statusbox, !global.!systemstatus, 0, 26, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
lockbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!statusbox, lockbox, lockicon_size, lockicon_size, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
!global.!lock_bitmap = dw_bitmapbutton_new_from_file( 'Directory Locked', 0, !global.!icondir'lockdir' )
Call dw_box_pack_start lockbox, !global.!lock_bitmap, lockicon_size, lockicon_size, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_disable !global.!lock_bitmap

!global.!jobstatus = dw_button_new( 'Active Jobs: 0', 0 )
Call dw_box_pack_start !global.!statusbox, !global.!jobstatus, !REXXDW.!DW_SIZE_AUTO, 26, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!jobstatus, !REXXDW.!DW_CLICKED_EVENT, 'JobStatusCallback'
/*
 * Create our directory toolbar boxes...
 */
!global.!dirtoolbarboxperm = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start !global.!mainbox, !global.!dirtoolbarboxperm, !global.!toolbox_size, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
/*
 * Pack the 'Quit' button into the top of the toolbar
 */
abutton = dw_bitmapbutton_new_from_file( 'Quit QOCCA', 0, !global.!icondir||'quit' )
Call dw_box_pack_start !global.!dirtoolbarboxperm, abutton, !global.!toolbox_size, !global.!toolbox_size, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'QuitCallback'
Call dw_box_pack_start !global.!dirtoolbarboxperm, 0, !global.!toolbox_size, 5, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
!global.!dirtoolbarbox = 0
!global.!dirtoolbarbutton.0 = 0 /* this ensures that we don't try and delete the buttons */
/*
 * Create our repository notebook box...
 */
!global.!repnotebookbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start !global.!mainbox, !global.!repnotebookbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Create our file toolbar boxes...
 */
tmp = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start !global.!mainbox, tmp, !global.!toolbox_size, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
!global.!filetoolbarboxperm = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start tmp, !global.!filetoolbarboxperm, !global.!toolbox_size, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
abutton = dw_bitmapbutton_new_from_file( 'Help', 0, !global.!icondir||'help' )
Call dw_box_pack_start !global.!filetoolbarboxperm, abutton, !global.!toolbox_size, !global.!toolbox_size, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'HelpCallback'
Call dw_box_pack_start !global.!filetoolbarboxperm, 0, !global.!toolbox_size, 5, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
abutton = dw_bitmapbutton_new_from_file( 'Clear Messages', 0, !global.!icondir||'clear' )
Call dw_box_pack_start tmp, abutton, !global.!toolbox_size, !global.!toolbox_size, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'ClearMessages'
!global.!filetoolbarbox = 0
!global.!filetoolbarbutton.0 = 0 /* this ensures that we don't try and delete the buttons */
/*
 * Create our repository notebook
 */
!global.!repnotebook = dw_notebook_new( 0, !REXXDW.!DW_TAB_TO_TOP )
Call dw_box_pack_start !global.!repnotebookbox, !global.!repnotebook, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_signal_connect !global.!repnotebook, !REXXDW.!DW_SWITCH_PAGE_EVENT, 'SwitchRepositoryCallback'
/*
 * For each repository, we need to create the tree, file, and message boxen...
 */
Do i = 1 To !global.!!number_repositories
   Call AddRepositoryPage i
End
/*
 * Setup callbacks
 */
Call dw_signal_connect !global.!mainwindow, !REXXDW.!DW_DELETE_EVENT, 'QuitCallback'
Call dw_signal_connect !global.!mainwindow, !REXXDW.!DW_KEY_PRESS_EVENT, 'TopWindowKeyPressCallback'
/*
 * Create our menus..
 */
!global.!mainmenubar = dw_menubar_new( !global.!mainwindow )
/*
 * If we have more than 1 repository, create a sub-menu with all but our current
 * repository to enable them to be removed - TODO
 * How do we recreate this men after removing a repository?
 */
/*
 * Add items to the 'File' menu
 * Note that each menu item MUST have a unique ID, or the callbacks won't work!
 */
menu = dw_menu_new( 0 )
menuitem = dw_menu_append_item( menu, '~Add Repository', 1011, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'AddRepositoryCallback'
/*
menuitem = dw_menu_append_item( menu, '~Remove Repository', 1012, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'RemoveRepositoryCallback'
*/
menuitem = dw_menu_append_item( menu, '~Quit', 1013, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'QuitCallback'
/*
 * Add the 'File' menu to the menubar...
 */
Call dw_menu_append_item !global.!mainmenubar, '~File', 1010, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, menu
/*
 * Add items to the 'Edit' menu
 * Note that each menu item MUST have a unique ID, or the callbacks won't work!
 */
menu = dw_menu_new( 0 )
menuitem = dw_menu_append_item( menu, '~User Preferences', 1021, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'UserPreferencesCallback', 0
/*
 * Add the 'Edit' menu to the menubar...
 */
Call dw_menu_append_item !global.!mainmenubar, '~Edit', 1020, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, menu
/*
 * Add items to the 'Help' menu
 * Note that each menu item MUST have a unique ID, or the callbacks won't work!
 */
menu = dw_menu_new( 0 )
menuitem = dw_menu_append_item( menu, '~Contents', 1091, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'HelpcontentsCallback'
menuitem = dw_menu_append_item( menu, '~About', 1092, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'HelpAboutCallback'
/*
 * Add the 'Help' menu to the menubar...
 */
Call dw_menu_append_item !global.!mainmenubar, '~Help', 1090, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, menu
/*
 * If we have saved our position and size previously, use these details...
 */
Select
   When maximised Then Nop
   When Words( !global.!!user.!!window_pos_size ) = 4 & !global.!!user.!!preference.!!save_position = 1 & \maximised Then
      Do
         Parse Var !global.!!user.!!window_pos_size xpos ypos !global.!window_width !global.!window_height
         If xpos < 0 Then xpos = 0
         If ypos < 0 Then ypos = 0
         Call dw_window_set_pos_size !global.!mainwindow, xpos, ypos, !global.!window_width, !global.!window_height
      End
   Otherwise
      Do
         !global.!window_width = ( !global.!screen_width % 4 ) * 3
         !global.!window_height = Max( ( !global.!screen_height % 4 ) * 3, (!global.!maxfilebuttons * !global.!toolbox_size) + (!global.!maxfileseparators * 5) + 90 )
         Call dw_window_set_pos_size !global.!mainwindow, (!global.!screen_width % 2) - (!global.!window_width % 2), (!global.!screen_height % 2) - (!global.!window_height % 2), !global.!window_width, !global.!window_height
      End
End
/*
 * Display the main window
 */
Call dw_window_show !global.!mainwindow
Call SetCursorNoWait
Return

/*
 * Creates a new repository notebook page and creates the base objects within it.
 * Doesn't populate the items.
 */
AddRepositoryPage: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg i
/*
 * Create our notebook page and box to pack...
 */
!global.!repnotebookpagebox.i = dw_box_new( !REXXDW.!DW_VERT )
!global.!repnotebookpage.i = dw_notebook_page_new( !global.!repnotebook, 0, !REXXDW.!DW_PAGE_TO_BACK )
/*
 * Pack the box for the notebook page contents into the notebook page and set the
 * text of the notebook page tab
 */
Call dw_notebook_pack !global.!repnotebook, !global.!repnotebookpage.i, !global.!repnotebookpagebox.i
Call dw_notebook_page_set_text !global.!repnotebook, !global.!repnotebookpage.i, !global.!!repository.!!name.i
/*
 * Create the tree box. don't pack it as it goes into a splitbar
 */
!global.!treebox.i = dw_box_new( !REXXDW.!DW_VERT )
!global.!tree.i = -1 /* ensures we don't try an destroy this the first time we generate the tree */
/*
 * Create the box for files and messages. don't pack it as it goes into a splitbar
 */
!global.!filemessagebox.i = dw_box_new( !REXXDW.!DW_VERT )
/*
 * Create the first splitbar with tree and file boxen.
 */
!global.!hsplitbar.i = dw_splitbar_new( !REXXDW.!DW_HORZ, !global.!treebox.i, !global.!filemessagebox.i, 0 )
Call dw_box_pack_start !global.!repnotebookpagebox.i, !global.!hsplitbar.i, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_splitbar_set !global.!hsplitbar.i, 30.0
/*
 * Create the file box. don't pack it as it goes into a splitbar
 */
!global.!filebox.i = dw_box_new( !REXXDW.!DW_VERT )
/*
 * Create the message mle. don't pack it as it goes into a splitbar
 */
!global.!mle.i = dw_mle_new( 0 )
Call dw_mle_set_word_wrap !global.!mle.i, !REXXDW.!DW_DONT_WORD_WRAP
!global.!mle_position.i = -1
/*
 * Create the second splitbar with file and message boxen.
 */
!global.!vsplitbar.i = dw_splitbar_new( !REXXDW.!DW_VERT, !global.!filebox.i, !global.!mle.i, 0 )
Call dw_box_pack_start !global.!filemessagebox.i, !global.!vsplitbar.i, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_splitbar_set !global.!vsplitbar.i, 70.0
Call dw_mle_set_editable !global.!mle.i, !REXXDW.!DW_READONLY
Call dw_window_set_color !global.!mle.i, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
/*
 * Create our file notebook box...
 */
!global.!filenotebook.i = dw_notebook_new( 0, !REXXDW.!DW_TAB_TO_TOP )
Call dw_box_pack_start !global.!filebox.i, !global.!filenotebook.i, 100, 100, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_signal_connect !global.!filenotebook.i, !REXXDW.!DW_SWITCH_PAGE_EVENT, 'SwitchFilesCallback'
/*
 * We need a notebook page for:
 * repository files
 * non-repository files
 * deleted files
 * all files
 */
/*
 * First page
 */
!global.!filenotebookpage.?repfiles.i = dw_notebook_page_new( !global.!filenotebook.i, 0, !REXXDW.!DW_PAGE_TO_BACK )
/*
 * We need something to pack into the page...
 */
!global.!repfilescontainerbox.i = dw_box_new( !REXXDW.!DW_VERT )
/*
 * Now pack the box into the notebook page
 */
Call dw_notebook_pack !global.!filenotebook.i, !global.!filenotebookpage.?repfiles.i, !global.!repfilescontainerbox.i
Call dw_notebook_page_set_text !global.!filenotebook.i, !global.!filenotebookpage.?repfiles.i, "Repository Files"
/*
 * Create our container in the box...
 */
!global.!repfilescontainer.i = dw_container_new( 2000+i, !REXXDW.!DW_MULTIPLE_SELECTION )
Call dw_box_pack_start !global.!repfilescontainerbox.i, !global.!repfilescontainer.i, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Use a filesystem container. This has as the first two columns, icon and filename, followed
 * by our user data.
 */
Call dw_filesystem_setup !global.!repfilescontainer.i, '!global.!repfiles_flags.', '!global.!repfiles_titles.'
/*
 * Second page...
 */
!global.!filenotebookpage.?nonrepfiles.i = dw_notebook_page_new( !global.!filenotebook.i, 0, !REXXDW.!DW_PAGE_TO_BACK )
/*
 * We need something to pack into the page...
 */
!global.!nonrepfilescontainerbox.i = dw_box_new( !REXXDW.!DW_VERT )
/*
 * Now pack the box into the notebook page
 */
Call dw_notebook_pack !global.!filenotebook.i, !global.!filenotebookpage.?nonrepfiles.i, !global.!nonrepfilescontainerbox.i
Call dw_notebook_page_set_text !global.!filenotebook.i, !global.!filenotebookpage.?nonrepfiles.i, "Non Repository Files"
/*
 * Create our container in the box...
 */
!global.!nonrepfilescontainer.i = dw_container_new( 3000+i, !REXXDW.!DW_MULTIPLE_SELECTION )
Call dw_box_pack_start !global.!nonrepfilescontainerbox.i, !global.!nonrepfilescontainer.i, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Use a filesystem container. This has as the first two columns, icon and filename, followed
 * by our user data.
 */
Call dw_filesystem_setup !global.!nonrepfilescontainer.i, '!global.!nonrepfiles_flags.', '!global.!nonrepfiles_titles.'
/*
 * Make sure that this list is populated the first time we switch to this
 * page.
 */
!global.!nonrepfiles.?filename.i.0 = -1
/*
 * Third page contains deleted files
 */
!global.!filenotebookpage.?deletedfiles.i = dw_notebook_page_new( !global.!filenotebook.i, 0, !REXXDW.!DW_PAGE_TO_BACK )
/*
 * We need something to pack into the page...
 */
!global.!deletedfilescontainerbox.i = dw_box_new( !REXXDW.!DW_VERT )
/*
 * Now pack the box into the notebook page
 */
Call dw_notebook_pack !global.!filenotebook.i, !global.!filenotebookpage.?deletedfiles.i, !global.!deletedfilescontainerbox.i
Call dw_notebook_page_set_text !global.!filenotebook.i, !global.!filenotebookpage.?deletedfiles.i, "Deleted Files"
/*
 * Create our container in the box...
 */
!global.!deletedfilescontainer.i = dw_container_new( 4000+i, !REXXDW.!DW_MULTIPLE_SELECTION )
Call dw_box_pack_start !global.!deletedfilescontainerbox.i, !global.!deletedfilescontainer.i, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Use a filesystem container. This has as the first two columns, icon and filename, followed
 * by our user data.
 */
Call dw_filesystem_setup !global.!deletedfilescontainer.i, '!global.!deletedfiles_flags.', '!global.!deletedfiles_titles.'
/*
 * Fourth page
 */
!global.!filenotebookpage.?allfiles.i = dw_notebook_page_new( !global.!filenotebook.i, 0, !REXXDW.!DW_PAGE_TO_BACK )
/*
 * We need something to pack into the page...
 */
!global.!allfilescontainerbox.i = dw_box_new( !REXXDW.!DW_VERT )
/*
 * Now pack the box into the notebook page
 */
Call dw_notebook_pack !global.!filenotebook.i, !global.!filenotebookpage.?allfiles.i, !global.!allfilescontainerbox.i
Call dw_notebook_page_set_text !global.!filenotebook.i, !global.!filenotebookpage.?allfiles.i, "All Files"
/*
 * Create our container in the box...
 */
!global.!allfilescontainer.i = dw_container_new( 2000+i, !REXXDW.!DW_MULTIPLE_SELECTION )
Call dw_box_pack_start !global.!allfilescontainerbox.i, !global.!allfilescontainer.i, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Use a filesystem container. This has as the first two columns, icon and filename, followed
 * by our user data. This page is identical to repfiles!!
 */
Call dw_filesystem_setup !global.!allfilescontainer.i, '!global.!allfiles_flags.', '!global.!allfiles_titles.'
/*
 * Make sure that this list is populated the first time we switch to this
 * page.
 */
!global.!deletedfiles.?filename.i.0 = -1
/*
 * Set the first page as default
 */
!global.!filespage.i = !global.!filenotebookpage.?repfiles.!!last_repository
Call dw_notebook_page_set !global.!filenotebook.i, !global.!filenotebookpage.?repfiles.1
Return

/*
 * Project level callbacks...
 */
AddRepositoryCallback: Procedure Expose !REXXDW. !global. !!last_repository
If GetNewRepository() \= 'close' Then
   Do
      /*
       * We now have everything in a state that we can connect to and display the
       * new repository.
       */
      !global.!allowcallbacks = 0
      Call AddRepositoryPage !!last_repository
      Call dw_notebook_page_set !global.!repnotebook, !global.!repnotebookpage.!!last_repository
      !global.!allowcallbacks = 1
      Call DisplayRepository
   End
Return 0

RemoveRepositoryCallback: Procedure Expose !REXXDW. !global. !!last_repository
If GetNewRepository() \= 'close' Then
   Do
      /*
       * We now have everything in a state that we can connect to and display the
       * new repository.
       */
      !global.!allowcallbacks = 0
      Call AddRepositoryPage !!last_repository
      Call dw_notebook_page_set !global.!repnotebook, !global.!repnotebookpage.!!last_repository
      !global.!allowcallbacks = 1
      Call DisplayRepository
   End
Return 0

HelpAboutCallback: Procedure Expose !REXXDW. !global. !!last_repository
window_x = !global.!window_width % 2
window_x = 500
window_y = 200
splashwindow = dw_window_new( !REXXDW.!DW_DESKTOP, 'About QOCCA', !global.!windowstyle )
Call dw_window_set_icon splashwindow, !global.!qoccaicon
box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start splashwindow, box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_color box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
textandimagebox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start box, textandimagebox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
Call dw_window_set_color textandimagebox, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
imagebox = dw_bitmap_new_from_file( 100, !global.!icondir'bigqocca'  )
Call dw_box_pack_start textandimagebox, imagebox, 64, 64, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
textbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start textandimagebox, textbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
Call dw_window_set_color textbox, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
/*
 * Display version details
 */
t1 = dw_text_new( 'QOCCA', 0 )
Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
t1 = dw_text_new( 'Version' !global.!version, 0 )
Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
t1  = dw_text_new( 'Copyright Mark Hessling 2009', 0 )
Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
t1  = dw_text_new( Copies( '-', 30 ), 0 )
Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Parse Version ver
t1  = dw_text_new( 'Rexx Version:' ver, 0 )
Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
t1  = dw_text_new( 'RexxDW Version:' dw_variable( 'VERSION' ), 0 )
Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_environment_query 'ver.'
t1 = dw_text_new( 'dwindows Version:' ver.8'.'ver.9'.'ver.10 'Built:' ver.2 ver.3, 0 )
Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
t1  = dw_text_new( 'RegUtil Version:' sysutilversion() '('sysversion()')', 0 )
Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
If !global.!os = 'WIN32' Then
   Do
      t1  = dw_text_new( 'W32Util Version:' w32version(), 0 )
      Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
      Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
      Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
   End
/*
 * User info
 */
t1  = dw_text_new( Copies( '-', 30 ), 0 )
Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
If userid() \= !global.!user Then userdet = userid() 'mapped to:' !global.!user
Else userdet = !global.!user
t1  = dw_text_new( 'User:' userdet 'Administrator:' !global.!administrator.!!last_repository, 0 )
Call dw_box_pack_start textbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_style t1, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
/*
 * Our OK button
 */
abutton = dw_button_new( 'OK', 0 )
Call dw_box_pack_start box, abutton, window_x, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 1
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', splashwindow
Call dw_signal_connect splashwindow, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', splashwindow
/*
 * Display the splash window
 */
Call dw_window_set_gravity splashwindow, !REXXDW.!DW_GRAV_CENTRE, !REXXDW.!DW_GRAV_CENTRE
Call dw_window_set_pos_size splashwindow, 0, 0, 0, 0
Call dw_window_show splashwindow
Return 0

HelpContentsCallback: Procedure Expose !REXXDW. !global. !!last_repository
Call HTMLBrowse !global.!helpdir'index.html', 'QOCCA Help'
Return 0

/*
 * Callback from each file filter button
 */
ToggleFileFilterCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, idx
If !global.!filefilterstatus.!!last_repository.idx = 'off' Then
   Do
      !global.!filefilterstatus.!!last_repository.idx = ''
      bit = '1'
      status = 'ON'
   End
Else
   Do
      !global.!filefilterstatus.!!last_repository.idx = 'off'
      bit = '0'
      status = 'OFF'
   End
!global.!!repository.!!filefilters.!!last_repository = Overlay( bit, !global.!!repository.!!filefilters.!!last_repository, idx, 1 )
--Call dw_window_set_bitmap !global.!filefilterbutton.idx, , !global.!icondir||!global.!filefiltericons.idx||!global.!filefilterstatus.!!last_repository.idx
Call dw_window_set_tooltip !global.!filefilterbutton.idx, !global.!filefiltericonsdesc.idx '-' status
Return

/*
 * Sets the file filter buttons to enabled
 */
EnableFileFilters: Procedure Expose !REXXDW. !global. !!last_repository
Call dw_window_enable !global.!filefilterbuttonapply
If Left( !global.!!repository.!!filefilters.!!last_repository, 1 ) = '!' Then !global.!!repository.!!filefilters.!!last_repository = Copies( '1', !global.!filefiltericons.0 )
Do i = 1 To !global.!filefiltericons.0
   Call dw_window_enable !global.!filefilterbutton.i
End
Call ShowFileFilters
Return

/*
 * Sets the file filter buttons to disabled
 */
DisableFileFilters: Procedure Expose !REXXDW. !global. !!last_repository
Call dw_window_disable !global.!filefilterbuttonapply
Do i = 1 To !global.!filefiltericons.0
   -- turn off tooltips
   Call dw_window_set_tooltip !global.!filefilterbutton.i
   Call dw_window_disable !global.!filefilterbutton.i
End
Return

/*
 * Set the appropriate file filter button image
 */
ShowFileFilters: Procedure Expose !REXXDW. !global. !!last_repository
If Left( !global.!!repository.!!filefilters.!!last_repository, 1 ) = '!' Then !global.!!repository.!!filefilters.!!last_repository = Copies( '1', !global.!filefiltericons.0 )
Do i = 1 To !global.!filefiltericons.0
   -- set the tooltip to reflect the setting
   If Substr( !global.!!repository.!!filefilters.!!last_repository, i, 1 ) = '1' Then
      Do
         !global.!filefilterstatus.!!last_repository.i = ''
         status = 'ON'
      End
   Else
      Do
         !global.!filefilterstatus.!!last_repository.i = 'off'
         status = 'OFF'
      End
   Call dw_window_set_tooltip !global.!filefilterbutton.i, !global.!filefiltericonsdesc.i '-' status
End
Return

/*
 * Takes the !global.!allfiles. stem and removes any files which don't have the appropriate
 * file filter turned on
 */
ApplyFileFiltering: Procedure Expose !REXXDW. !global. !!last_repository
keep_idx = ''
Do i = 1 To !global.!allfiles.?filestatus.!!last_repository.0
   status = Substr( !global.!allfiles.?filestatus.!!last_repository.i, 5 )
   Do j = 1 To !global.!filefiltericons.0
      filter_on = Substr( !global.!!repository.!!filefilters.!!last_repository, j, 1 )
      If filter_on = 1 & !global.!filefiltericons.j = status Then
         Do
            keep_idx = keep_idx i
            Leave
         End
   End
End
num_rows = Words( keep_idx )
!global.!allfiles.?filerev.!!last_repository.0 = num_rows
!global.!allfiles.?fullfilename.!!last_repository.0 = num_rows
!global.!allfiles.?filename.!!last_repository.0 = num_rows
!global.!allfiles.?filepath.!!last_repository.0 = num_rows
!global.!allfiles.?filedate.!!last_repository.0 = num_rows
!global.!allfiles.?user.!!last_repository.0 = num_rows
!global.!allfiles.?filestatus.!!last_repository.0 = num_rows
!global.!allfiles.?filereminder.!!last_repository.0 = num_rows
!global.!allfiles.?filetype.!!last_repository.0 = num_rows
Do i = 1 To num_rows
   idx = Word( keep_idx, i )
   newi = Right( i, 4 )
   !global.!allfiles.?filerev.!!last_repository.i       = Overlay( newi, !global.!allfiles.?filerev.!!last_repository.idx )
   !global.!allfiles.?fullfilename.!!last_repository.i  = Overlay( newi, !global.!allfiles.?fullfilename.!!last_repository.idx )
   !global.!allfiles.?filename.!!last_repository.i      = Overlay( newi, !global.!allfiles.?filename.!!last_repository.idx )
   !global.!allfiles.?filepath.!!last_repository.i      = Overlay( newi, !global.!allfiles.?filepath.!!last_repository.idx )
   !global.!allfiles.?filedate.!!last_repository.i      = Overlay( newi, !global.!allfiles.?filedate.!!last_repository.idx )
   !global.!allfiles.?user.!!last_repository.i          = Overlay( newi, !global.!allfiles.?user.!!last_repository.idx )
   !global.!allfiles.?filestatus.!!last_repository.i    = Overlay( newi, !global.!allfiles.?filestatus.!!last_repository.idx )
   !global.!allfiles.?filereminder.!!last_repository.i  = Overlay( newi, !global.!allfiles.?filereminder.!!last_repository.idx )
   !global.!allfiles.?filetype.!!last_repository.i      = Overlay( newi, !global.!allfiles.?filetype.!!last_repository.idx )
End
Return

/*
 * The callback executed when the user left-clicks on an item in the tree
 */
DirSelectCallback: Procedure Expose !REXXDW. !global. !!last_repository
If !global.!allowcallbacks = 1 Then
   Do
      Parse Arg window, item, text, itemdata, data
      Call SetCursorWait
      if trace() = 'F' Then say 'DirSelectCallback: window:' window 'item:' item 'text:' text 'itemdata:' itemdata 'data:' data
      Parse Var itemdata parent fulldir
      If fulldir = '' Then !global.!current_dirname.!!last_repository = '.'
      Else !global.!current_dirname.!!last_repository = Substr( fulldir, 2 )
      /*
       * Save the tree item that represents the currently selected directory
       */
      !global.!selected_treeitem.!!last_repository = item
      /*
       * Adjust the directory toolbar
       */
      Call AdjustDirToolbarList
      Call AdjustDirToolbarButtons
      /*
       * Refresh the repository files, non-rep files and deleted files
       * tabs. DO NOT refresh the all files tab; it takes too long unless
       * it is the current tab
       */
      Call DisplayRepositoryFiles
      Call DisplayNonRepositoryFiles
      Call DisplayDeletedFiles
      If !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then Call DisplayAllFiles
      /*
       * If this directory is set for reserved checkout,
       */
      Call SetLockedStatusIcon
      /*
       * Adjust the file toolbar as well!
       */
      Call FileSelectCallback
      Call SetCursorNoWait
   End
Return 0

/*
 * The callback executed when the user right-clicks on an item in the tree
 */
DirMenuCallback: Procedure Expose !REXXDW. !global. !!last_repository
If !global.!allowcallbacks = 1 Then
   Do
      Parse Arg window, text, x, y, itemdata
      if trace() = 'F' Then say 'Dirmenucallback' window ',' text ',' x ',' y ',' itemdata
      Call DisplayDirMenu x, y
   End
Return 1

/*
 * The callback executed when the user left-clicks on an item in file listing
 */
FileSelectCallback: Procedure Expose !REXXDW. !global. !!last_repository
If !global.!allowcallbacks = 1 Then
   Do
      Parse Arg arg1, arg2, arg3, arg4, arg5, arg6, arg7
      if trace() = 'F' Then say 'FileSelectCallback' arg1','arg2','arg3','arg4','arg5','arg6','arg7
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
   End
Return 0

/*
 * The callback executed when the user right-clicks on an item in file listing
 */
FileMenuCallback: Procedure Expose !REXXDW. !global. !!last_repository
If !global.!allowcallbacks = 1 Then
   Do
      Parse Arg window, text, x, y, itemdata
      if trace() = 'F' Then say 'FileMenucallback' window ',' text ',' x ',' y ',' itemdata
      Call AdjustFileToolbarList
      Call DisplayFileMenu x, y
   End
Return 0

/*
 * Sorts the file list based on the column heading clicked
 */
FileSortCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., column
Select
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then
      Do
         !global.!repfile_sort_order = column
         /*
          * Toggle the sort order for the column.
          */
         If !global.!repfile_sort_direction.column = 'ascending' Then !global.!repfile_sort_direction.column = 'descending'
         Else !global.!repfile_sort_direction.column = 'ascending'
         Call SortFileArray
         Call DisplayFileArray
      End
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?nonrepfiles.!!last_repository Then
      Do
         !global.!nonrepfile_sort_order = column
         /*
          * Toggle the sort order for the column.
          */
         If !global.!nonrepfile_sort_direction.column = 'ascending' Then !global.!nonrepfile_sort_direction.column = 'descending'
         Else !global.!nonrepfile_sort_direction.column = 'ascending'
         Call SortNonrepFileArray
         Call DisplayNonRepFileArray
      End
   When !global.!filespage.!!last_repository = !global.!filespage.?deletedrepfiles Then
      Do
      /*   Call DisplayDeletedFiles */
      End
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then
      Do
         !global.!allfile_sort_order = column
         /*
          * Toggle the sort order for the column.
          */
         If !global.!allfile_sort_direction.column = 'ascending' Then !global.!allfile_sort_direction.column = 'descending'
         Else !global.!allfile_sort_direction.column = 'ascending'
         Call SortAllFileArray
         Call DisplayAllFileArray
      End
   Otherwise Nop
End
Return 0

/*
 * The callback executed when the user left-clicks on an item in the link tree
 */
LinkDirSelectCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg window, item, text, itemdata, data
if trace() = 'F' Then say 'LinkDirSelectCallback: window:' window 'item:' item 'text:' text 'itemdata:' itemdata 'data:' data
Parse Var itemdata parent fulldir
!global.!linkdir.?linked_dirname = fulldir
Return 0

LinkDirApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., win, dialog
linked_dirname = !global.!linkdir.?linked_dirname
common_dirname = '/'!global.!current_dirname.!!last_repository
/*
 * Get the last directory component of the current directory
 */
mycomm = Strip( common_dirname, 'T', '/' )
lp = Lastpos( '/', mycomm )
lastdir = Substr( mycomm, lp )
newdir = linked_dirname || lastdir
mytail = mycomm
/*
 * Determine how many common, leading directories there are. This is used to reduce
 * the number of relative directories we need to go back up to find the linked
 * directory.
 * eg:
 * mycomm = /dir1/apath
 * newdir = /dir1/fred/apath
 *
 * Should result in ln -s ../apath . rather than ../../dir1/apath
 */
Do i = 1 To Min( Length( mycomm ), Length( newdir ) )
   If Substr( mycomm, i, 1 ) = '/' Then last_dir_pos = i
   If Substr( mycomm, i, 1 ) \= Substr( newdir, i, 1 ) Then Leave
End
num_common_dirs = Countstr( '/', Substr( mycomm, 1, i ) ) - 1
If num_common_dirs \= 0 Then mytail = Substr( mytail, last_dir_pos )
/*
 * Add linked_dirname into !linked_dir. stem if it isn't there already, first
 * checking to ensure its not already in dir. stem (that's an error)
 */
found = 0
Do i = 1 To !global.!repdir.!!last_repository.0
   If newdir = !global.!repdir.!!last_repository.i Then
      Do
         Call dw_messagebox 'Cannot link!', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), 'Cannot link' quote( !global.!current_dirname.!!last_repository ) 'to' quote( linked_dirname ) 'as that directory already contains:' quote( Substr( mytail, 2 ) )
         Call Directory !global.!!repository.!!working_dir.!!last_repository
         Return 'retry'
      End
End
/*
 * Add !global.!current_dirname.!!last_repository into !global.!current_dir. stem if it isn't there already
 */
found = 0
Do i = 1 To !global.!common_dir.0
   If !global.!common_dir.i = common_dirname Then
      Do
         found = 1
         common_idx = i
         Leave
      End
End
If found = 0 Then
   Do
      ldi = !global.!common_dir.0+1
      !global.!common_dir.0 = ldi
      !global.!common_dir.ldi = common_dirname
      common_idx = ldi
   End
/*
 * See if we already have the linked dir
 */
found = 0
Do i = 1 To !global.!linked_dir.0
   If !global.!linked_dir.i = newdir Then
      Do
         found = 1
         Leave
      End
End
If found = 0 Then
   Do
      i = !global.!linked_dir.0+1
      !global.!linked_dir.0 = i
      !global.!linked_dir.i = newdir
      !global.!linked_idx.0 = i
      !global.!linked_idx.i = common_idx
   End

Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         /*
          * Now symlink the common dir to the linked dirname
          * eg. Linking /tmfps/common to /tmfps/app/common
          *     cd /tmfps/app
          *     ln -sf ../common .
          */
         Call Directory !global.!!repository.!!shortpath.!!last_repository||linked_dirname
         num_dots = Countstr( '/', linked_dirname ) - num_common_dirs
         linkdir = Copies( '../', num_dots ) || Substr( mytail, 2 )
         Address Command !global.!symlink linkdir .
      End
End

Call Directory !global.!!repository.!!working_dir.!!last_repository
/*
 * Update the build.conf file in the repository
 */
Call WriteBuildConf 'Linking' common_dirname 'to' newdir
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Destroy the window
 */
Call dw_window_destroy win
Call DisplayRepository
Return 'apply'

/*
 * The callback executed when the user left-clicks on an item in the moduleselect tree
 */
SelectDirSelectCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg window, item, text, itemdata, data, entry
Parse Var itemdata parent fulldir
!global.!selecteddir = Substr( fulldir, 2 )
Return 0

SelectDirApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., win, dialog, entry
Call dw_window_set_text entry, !global.!selecteddir
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Destroy the window
 */
Call dw_window_destroy win
Return 'apply'

/*
 * This callback handles a user-created menu item.
 */
UserDirCommandCallback: Procedure Expose !REXXDW. !global. !!last_repository
If !global.!allowcallbacks = 1 Then
   Do
      Parse Arg window, idx
      template = GetUserSubstitutionVariables( !global.!!user.!!dirmenu.!!command.idx )
      If template \= '' Then
         Do
            dircmd = Changestr( '%D%', template, GetFullWorkingDirectory( !global.!current_dirname.!!last_repository ) )
            dircmd = Changestr( '%W%', dircmd, !global.!!repository.!!working_dir.!!last_repository )
            dircmd = Changestr( '%%%', dircmd, '%' )
            /*
             * Before executing the user supplied command change directory to the current_dirname
             */
            here = Directory()
            Call Directory !global.!current_dirname.!!last_repository
            Address System !global.!background_start_nowindow dircmd !global.!background_end
            Call Directory here
         End
   End
Return 0

UserFileCommandCallback: Procedure Expose !REXXDW. !global. !!last_repository
If !global.!allowcallbacks = 1 Then
   Do
      Parse Arg window, idx
      template = GetUserSubstitutionVariables( !global.!!user.!!filemenu.!!command.idx )
      If template \= '' Then
         Do
            filecmd = Changestr( '%D%', template, GetFullWorkingDirectory( !global.!current_dirname.!!last_repository ) )
            filecmd = Changestr( '%W%', filecmd, !global.!!repository.!!working_dir.!!last_repository )
            filecmd = Changestr( '%%%', filecmd, '%' )
            /*
             * Get all of the currently selected filenames...
             */
            wd = !global.!!repository.!!working_dir.!!last_repository
            current_filenames = ''
            Do i = 1 To !global.!current_filenames.0
               current_filenames = current_filenames quote( wd || !global.!current_filenames.i )
            End
            filecmd = Changestr( '%F%', filecmd, current_filenames )
            /*
             * Before executing the user supplied command change directory to the current_dirname
             */
            here = Directory()
            Call Directory !global.!current_dirname.!!last_repository
            Address System !global.!background_start_nowindow filecmd !global.!background_end
            Call Directory here
         End
   End
Return 0

GetUserSubstitutionVariables: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg template
/*
 * Ignore %D% %W% and %F%
 * Count how many "reserved" variables, and count how many % (must be 2*)
 * Check if the template contains any substitution variables of the
 * form %n% where n is 1-9.
 */
tmp_template = Changestr( '%D%', template, '   ' )
tmp_template = Changestr( '%W%', tmp_template, '   ' )
tmp_template = Changestr( '%F%', tmp_template, '   ' )
tmp_template = Changestr( '%%%', tmp_template, '   ' )
all_perc = Countstr( '%', tmp_template )
If all_perc = 0 Then Return template
/*
 * There are user variables
 */
If all_perc // 2 = 1 Then
   Do
      /* messagebox*/
      Return ''
   End
!global.!usermenuvariable.0 = 0
Do Forever
   Parse Var tmp_template . '%' name '%' tmp_template
   If name = '' Then Leave
   /*
    * We have a variable name, do we already have it ?
    */
   found = 0
   Do i = 1 To !global.!usermenuvariable.0
      If !global.!usermenuvariable.i = name Then
         Do
            found = 1
            Leave
         End
   End
   If \found Then
      Do
         idx = !global.!usermenuvariable.0 + 1
         !global.!usermenuvariable.idx = name
         !global.!usermenuvariable.0 = idx
      End
End
/*
 * Pop up a dialog box with a text field for each variable
 */
title_width = 130
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Set variable Values', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a description of the command showing the command
 */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Command', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
tmp = dw_entryfield_new( template, 0 )
Call dw_box_pack_start tmpbox, tmp, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_disable tmp
/*
 * Add the boxes and widgets for the variables...
 */
Do i = 1 To !global.!usermenuvariable.0
   varnum = !global.!usermenuvariable.i
   tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
   Call dw_box_pack_start topbox, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   tmp = dw_text_new( varnum, 0 )
   Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
   saved_value = GetSavedUserVariable( !global.!usermenuvariable.i )
   !global.!usermenuvariable_entry.i = dw_entryfield_new( saved_value, 0 )
   Call dw_box_pack_start tmpbox, !global.!usermenuvariable_entry.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
End
/*
 * Create OK
 */
b1 = dw_button_new( 'OK', 0 )
Call dw_box_pack_start topbox, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
width = 400
height = ( 2 * !global.!buttonheight ) + ( ( 1 + !global.!usermenuvariable.0 ) * !global.!widgetheight )
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GetUserSubstitutionVariablesApplyCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
If cancel = 'apply' Then
   Do
      /*
       * As we only have an OK, we will apply the stuff now
       */
      Do i = 1 To !global.!usermenuvariable.0
         varnum = !global.!usermenuvariable.i
         template = Changestr( '%'varnum'%', template, !global.!usermenuvalue.i )
      End
   End
Else template = ''
Drop !global.!usermenuvariable. !global.!usermenuvalue. !global.!usermenuvariable_entry.
Return template

GetUserSubstitutionVariablesApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., win, dialog_wait
/*
 * Get the values for each variable
 */
Do i = 1 To !global.!usermenuvariable.0
   !global.!usermenuvalue.i = Strip( dw_window_get_text( !global.!usermenuvariable_entry.i ) )
   Call SetSavedUserVariable !global.!usermenuvariable.i, !global.!usermenuvalue.i
End
Call dw_dialog_dismiss dialog_wait, 'apply'
Call dw_window_destroy win
Return 0

GetSavedUserVariable: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg varname
Do i = 1 To !global.!usermenu_saved_variable_name.0
   If !global.!usermenu_saved_variable_name.i = varname Then Return !global.!usermenu_saved_variable_value.i
End
Return ''

SetSavedUserVariable: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg varname, varvalue
found = 0
Do i = 1 To !global.!usermenu_saved_variable_name.0
   If !global.!usermenu_saved_variable_name.i = varname Then
      Do
         found = 1
         !global.!usermenu_saved_variable_value.i = varvalue
         Leave
      End
End
If \found Then
   Do
      idx = !global.!usermenu_saved_variable_name.0 + 1
      !global.!usermenu_saved_variable_name.idx = varname
      !global.!usermenu_saved_variable_value.idx = varvalue
      !global.!usermenu_saved_variable_name.0 = idx
   End
Return

SwitchRepositoryCallback: Procedure Expose !REXXDW. !global. !!last_repository
If !global.!allowcallbacks = 1 Then
   Do
      Parse Arg window, page, itemdata
      if trace() = 'F' Then say 'SwitchRepositoryCallback' window ',' page ',' itemdata '|' !global.!allowcallbacks
      Call DisconnectFromRepository
      -- find the pageid to switch to. The page arg is not always
      -- an index; on OS/2 its a pointer
      Do i = 1 To !global.!!number_repositories
         If page = !global.!repnotebookpage.i Then Leave
      End
      !!last_repository = i
      Call ConnectToRepository !!last_repository
      /*
       * Disable our file filters in case we are switching to a different repository tab
       */
      Call DisableFileFilters
      /*
       * If we haven't already loaded the repository details before, do it now
       * otherwise, re-read the local copy of the build.conf
       */
      If !global.!tree.!!last_repository = -1 Then Call DisplayRepository
      Else
         Do
            Call ReadBuildConf
            Call SetRepositoryEnvs
            Call SetupEnvironment
            Call AdjustToolbars
         End
      Call PopulateBookmarks
      Call SetWindowTitle
      Call Directory !global.!!repository.!!working_dir.!!last_repository
      /*
       * If this directory is set for reserved checkout,
       */
      Call SetLockedStatusIcon
      /*
       * if we are displaying the All FIles tab, then enable the file filters
       */
      If !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then Call EnableFileFilters
   End
Return 0

SwitchFilesCallback: Procedure Expose !REXXDW. !global. !!last_repository
If !global.!allowcallbacks = 1 Then
   Do
      Parse Arg window, page, itemdata
      if trace() = 'F' Then say 'SwitchFilesCallback' window ',' page ',' itemdata '|' !global.!allowcallbacks
      /*
       * page argument is return value from dw_notebook_page_new()
       */
      !global.!filespage.!!last_repository = page
      /*
       * Because the valid buttons are different for repository and
       * non-repository files, we have to adjust them when we switch
       */
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
      /*
       * If switching to the "all files" page and we haven't been here before,
       * go and get all the details - TODO
       */
      If page = !global.!filenotebookpage.?allfiles.!!last_repository Then
         Do
            Call EnableFileFilters
            Call DisplayAllFiles
         End
      Else
         Do
            Call DisableFileFilters
         End
   End
Return 0

QuitCallback: Procedure Expose !REXXDW. !global. !!last_repository
If !global.!!user.!!preference.!!confirm_exit = 1 Then exit = dw_messagebox( 'Exit QOCCA', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), 'Are you sure you want to exit QOCCA?' )
Else exit = !REXXDW.!DW_MB_RETURN_YES
If exit = !REXXDW.!DW_MB_RETURN_YES Then
   Do
      /*
       * Save the last position and size of the window
       */
      If !global.!!user.!!preference.!!save_position = 1 Then
         Do
            Parse Value dw_window_get_pos_size( !global.!mainwindow ) With posx posy sizex sizey
            !global.!!user.!!window_pos_size = posx posy sizex sizey
         End
      /*
       * Save the user's last settings...
       */
      Call WriteUserSettingsFile
      /*
       * Disable any callbacks from now on...
       */
      !global.!allowcallbacks = 0
      Call dw_window_destroy !global.!mainwindow
      Call CleanUpAndExit 0
      Call dw_exit 0
      Exit 0
   End
Return 1 /* returning 1 ensures dwindows ignores the event */

/*
 * Called when any key pressed in the top level window
 */
TopWindowKeyPressCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg window, key, virtual_key, state
if trace() = 'F' Then say virtual_key state !REXXDW.!DW_KC_CTRL
Select
   When window = !global.!mainwindow Then
      Do
         If (virtual_key = c2d( 'q' ) | virtual_key = 17 ) & state = !REXXDW.!DW_KC_CTRL Then Call QuitCallback
      End
   Otherwise Nop
End
Return 0

ClearMessages: Procedure Expose !REXXDW. !global. !!last_repository
Call dw_window_enable !global.!mle.!!last_repository
Call dw_mle_set_cursor !global.!mle.!!last_repository, 0
Call dw_mle_delete !global.!mle.!!last_repository
!global.!mle_position.!!last_repository = -1
Return 0

JobStatusCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Create a container box with
 * +---------------------------------------------------+
 * | Type  |  StartDate | StartTime | Comp | Desc      |
 * +---------------------------------------------------+
 * | build | 03/12/03   | 12:23:45  |  Y   | Build of..|
 * +---------------------------------------------------+
 * |       | View |             | Delete |             |
 * +---------------------------------------------------+
 * |                      OK                           |
 * +---------------------------------------------------+
 * and provide right-click menu with 'View...' and 'Delete'
 *
 */
flags.0 = 5
flags.1 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
flags.2 = dw_or( !REXXDW.!DW_CFA_TIME, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
flags.3 = dw_or( !REXXDW.!DW_CFA_BITMAPORICON, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
flags.4 = dw_or( !REXXDW.!DW_CFA_BITMAPORICON, !REXXDW.!DW_CFA_CENTER, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
flags.5 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
titles.0 = 5
titles.1 = 'StartDate'
titles.2 = 'StartTime'
titles.3 = 'Comp'
titles.4 = 'Type'
titles.5 = 'Description'
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Job Details', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Create our container in the box...
 */
!global.!jobscontainer = dw_container_new( 1, !REXXDW.!DW_MULTIPLE_SELECTION )
Call dw_box_pack_start topbox, !global.!jobscontainer, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
!global.!jobscontainer.?firsttime = 1
/*
 * Use a normal container.
 */
Call dw_container_setup !global.!jobscontainer, 'flags.', 'titles.', 0

Call DisplayJobs
/*
 * Create View and Delete buttons...
 */
buttonbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, buttonbox, 0, !global.!buttonheight+10, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_box_pack_start buttonbox, 0, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
b0 = dw_button_new( 'View', 0 )
Call dw_box_pack_start buttonbox, b0, 50, !global.!buttonheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 5
Call dw_box_pack_start buttonbox, 0, 50, !global.!buttonheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
b1 = dw_button_new( 'Delete', 0 )
Call dw_box_pack_start buttonbox, b1, 50, !global.!buttonheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 5
Call dw_box_pack_start buttonbox, 0, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

/*
 * Create OK button
 */
b2 = dw_button_new( 'OK', 0 )
Call dw_box_pack_start topbox, b2, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
width = 640
height = 320  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
    */
   Call dw_signal_connect b0, !REXXDW.!DW_CLICKED_EVENT, 'JobsButtonCallback', 'view'
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'JobsButtonCallback', 'delete'
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
Return 1

DisplayJobs: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Determine how many items in the container...
 */
nativelogdir = FindNativeLogDirectory()
If Stream( nativelogdir, 'C','QUERY EXISTS' ) = '' Then job.0 = 0
Else Call SysFileTree nativelogdir || !global.!ossep'qocca_bg*.desc', 'JOB.', 'FLS'
If job.0 > 1 Then Call SysStemSort 'JOB.','descending', , , , 1, 20
Do i = 1 To job.0
   fulldescfile = Changestr( '/', Word( job.i, 5 ), !global.!ossep )
   len = Length( fulldescfile )
   descfile = Substr( fulldescfile, 2+ Length( nativelogdir ) )
   Parse Var descfile 1 basefile.i '.desc' 1 . 10 date.i 18 time.i '.desc'
   date.i = SortedDateToDisplayDate( date.i )
   /*
    * Check if the indicator file exists. If so, then check the return code
    */
   indfile = nativelogdir||!global.!ossep||basefile.i
   compicon.i = ''
   If Stream( indfile, 'C', 'QUERY EXISTS' ) = '' Then completed.i = 0
   Else
      Do
         completed.i = 1
         Call Stream indfile, 'C', 'OPEN READ'
         rcode = Linein( indfile )
         If rcode = 0 Then compicon.i = !global.!okicon
         Else compicon.i = !global.!noicon
         Call Stream indfile, 'C', 'CLOSE'
      End
   /*
    * Read the description file for details about the job
    */
   Call Stream fulldescfile, 'C', 'OPEN READ'
   desc.i = Linein( fulldescfile )
   type = Linein( fulldescfile )
   type.i = type
   /* determine the icon ... */
   If type = 'testbuild' Then type = 'build'
   pos = Wordpos( type, !global.!jobicons )
   If pos = 0 Then typeicon.i = ''
   Else typeicon.i = !global.!jobicon.pos
   compiler.i = Linein( fulldescfile )
   Call Stream fulldescfile, 'C', 'CLOSE'
End

/*
 * Delete everything in the container so we can replace the existing files
 */
If !global.!jobscontainer.?firsttime = 0 Then
   Do
      Call dw_signal_disconnect !global.!jobscontainer, !REXXDW.!DW_ITEM_CONTEXT_EVENT
      Call dw_container_clear !global.!jobscontainer, !REXXDW.!DW_DONT_REDRAW
   End
!global.!jobscontainer.?firsttime = 0
/*
 * Add the entries into the container
 */
containermemory = dw_container_alloc( !global.!jobscontainer, job.0 )
Do i = 1 To job.0
   Call dw_container_set_item !global.!jobscontainer, containermemory, 0, i-1, date.i
   time = Substr( time.i, 1, 2 )':'Substr( time.i, 3, 2 )':'Substr( time.i, 5, 2 )
   Call dw_container_set_item !global.!jobscontainer, containermemory, 1, i-1, time
   If compicon.i \= '' Then
      Call dw_container_set_item !global.!jobscontainer, containermemory, 2, i-1, compicon.i
   If typeicon.i \= '' Then
      Call dw_container_set_item !global.!jobscontainer, containermemory, 3, i-1, typeicon.i
   Call dw_container_set_item !global.!jobscontainer, containermemory, 4, i-1, desc.i
   Call dw_container_set_row_title containermemory, i-1, type.i','basefile.i
End
Call dw_container_set_stripe !global.!jobscontainer, !REXXDW.!DW_CLR_WHITE, !global.!container_colour
Call dw_container_insert !global.!jobscontainer, containermemory, job.0
Call dw_container_optimize !global.!jobscontainer
/*
 * Only now can we connect our signals, because dw_set_file() causes the signals to fire!!!...
 */
Call dw_signal_connect !global.!jobscontainer, !REXXDW.!DW_ITEM_CONTEXT_EVENT, 'JobsMenuCallback'

Return

/*
 * Create a popup menu with "View" and "Delete" items
 */
JobsMenuCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg window, text, x, y, itemdata
/*
 * Determine which job items are currently selected
 */
basefiles = ''
selected = dw_container_query_start( !global.!jobscontainer, !REXXDW.!DW_CRA_SELECTED )
Do i = 1 While selected \= ''
   basefiles = basefiles selected
   selected = dw_container_query_next( !global.!jobscontainer, !REXXDW.!DW_CRA_SELECTED )
End
basefiles = Strip( basefiles )
/*
 * Create our popup and display it
 */
popup = dw_menu_new( 0 )
menuitem = dw_menu_append_item( popup,'View', 1001, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'JobsViewCallback', basefiles
menuitem = dw_menu_append_item( popup,'Delete', 1002, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'JobsDeleteCallback', basefiles
Call dw_menu_popup popup, !global.!mainwindow, x, y
Return 1

/*
 * Callback for Jobs Buttons
 */
JobsButtonCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., action
/*
 * Determine which job items are currently selected
 */
basefiles = ''
selected = dw_container_query_start( !global.!jobscontainer, !REXXDW.!DW_CRA_SELECTED )
Do i = 1 While selected \= ''
   basefiles = basefiles selected
   selected = dw_container_query_next( !global.!jobscontainer, !REXXDW.!DW_CRA_SELECTED )
End
basefiles = Strip( basefiles ) /* don't quote filenames */
Select
   When basefiles = '' Then Nop
   When action = 'view' & Words( basefiles ) = 1 Then Call JobsViewCallback '', basefiles
   When action = 'delete' Then Call JobsDeleteCallback '', basefiles
   Otherwise Nop
End
Return 1

/*
 * Only one file allowed for view
 */
JobsViewCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., type','jobs
nativelogdir = FindNativeLogDirectory()
/*
 * Convert the .out file into .html if it doesn't already exist
 * Read .desc for description...
 */
html_file = nativelogdir||!global.!ossep||jobs'.html'
desc_file = nativelogdir||!global.!ossep||jobs'.desc'
out_file = nativelogdir||!global.!ossep||jobs'.out'

jobdatetime = Right( jobs, 14 )
jobdate = Date( 'N', Left( jobdatetime, 8 ), 'S' )
jobtime = Substr( jobdatetime, 9, 2 )':'Substr( jobdatetime, 11, 2 )':'Substr( jobdatetime, 13, 2 )
jobdatetime = 'Job started on:' jobdate 'at' jobtime

/*If Stream( html_file, 'C', 'QUERY EXISTS' ) = '' Then*/
   Do
      Call Stream desc_file, 'C', 'OPEN READ'
      jobtitle = Linein( desc_file )
      junk = Linein( desc_file )
      compiler_type = Linein( desc_file )
      Call Stream desc_file, 'C', 'CLOSE'

      Call Stream out_file, 'C', 'OPEN READ'
      Call Stream html_file, 'C', 'OPEN WRITE REPLACE'
      Call Lineout html_file, '<html><head>'
      Call Lineout html_file, '<title>'
      Call Lineout html_file, jobtitle
      Call Lineout html_file, '</title>'
      Call Lineout html_file, '<style type="text/css">'
      Call Lineout html_file, '<!--'
      Call Lineout html_file, '.warning { color:#0000ff; }'
      Call Lineout html_file, '.error { color:#ff0000; }'
      Call Lineout html_file, '//-->'
      Call Lineout html_file, '</style>'
      Call Lineout html_file, '</head>'
      Call Lineout html_file, '<body bgcolor='!global.!color_background'>'
      Call Lineout html_file, '<center><h1>'jobtitle'</h1></center><pre>'
      Call Lineout html_file, '<center><h2>'jobdatetime'</h2></center>'
      Call Lineout html_file, '<hr>'
      Do While Lines( out_file ) > 0
         line = Linein( out_file )
         Select
            When Left( line, 4 ) = Copies( '>', 4 ) Then
               Do
                  Select
                     When type = 'mods' | type = 'testbuild' Then
                        Do
                           Parse Var line '>>>>   ' title ':' . 'qocca_bg_' loc
                           loc = Strip( loc )
                           If loc = '' Then Call Lineout html_file,'</pre><h3>'Substr( line, 5 )'</h3><pre>'
                           Else Call Lineout html_file,'</pre><a href="./qocca_bg_'loc'">'title'</a><pre>'
                        End
                     Otherwise Nop
                        Do
                           Call Lineout html_file,'</pre><h3>'Substr( line, 5 )'</h3><pre>'
                        End
                  End
               End
            Otherwise
               Do
                  Select
                     When type = 'make' | type = 'build' | type = 'testbuild' Then
                        Do
                           Parse Value ParseCompilerLine( compiler_type, line ) With modifier ',' .
                           If modifier = '' Then
                              Call Lineout html_file, line
                           Else
                              Call Lineout html_file, '<span class="'modifier'">'line'</span>'
                        End
                     When type = 'copy' Then
                        Do
                           Select
                              When Subword( line, 1, 2 ) = 'RCS file:' Then Call Lineout html_file, '<b>'line'</b>'
                              When Subword( line, 1, 2 ) = 'Creating directory' Then Call Lineout html_file, '<b>'line'</b>'
                              When Subword( line, 1, 2 ) = 'Linking directory' Then Call Lineout html_file, '<b>'line'</b>'
                              When Subword( line, 1, 2 ) = 'deleting revision' Then Call Lineout html_file, '<span class="warning">'line'</span>'
                              Otherwise Call Lineout html_file, line
                           End
                        End
                     Otherwise Call Lineout html_file, line
                  End
               End
         End
      End
      Call Lineout html_file, '</pre></body></html>'
      Call Stream html_file, 'C', 'CLOSE'
      Call Stream out_file, 'C', 'CLOSE'
   End
Call HTMLBrowse html_file, jobtitle
Return 0

HTMLBrowse: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg fn, title
-- Create a window, same size as main window with an embedded HTML widget
win = dw_window_new( !REXXDW.!DW_DESKTOP, title, !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
-- add box with our action buttons
buttonbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, buttonbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

sbutton = dw_button_new( "Stop", 0 )
Call dw_box_pack_start buttonbox, sbutton, 30, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

bbutton = dw_button_new( "Back", 0 )
Call dw_box_pack_start buttonbox, bbutton, 30, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

fbutton = dw_button_new( "Forward", 0 )
Call dw_box_pack_start buttonbox, fbutton, 30, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

htmlbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start topbox, htmlbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
html = dw_html_new( 11011 )
Call dw_box_pack_start htmlbox, html, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_html_url html, 'file://'fn

Call dw_signal_connect sbutton, !REXXDW.!DW_CLICKED_EVENT, 'HTMLButtonCallback', !REXXDW.!DW_HTML_STOP, html
Call dw_signal_connect bbutton, !REXXDW.!DW_CLICKED_EVENT, 'HTMLButtonCallback', !REXXDW.!DW_HTML_GOBACK, html
Call dw_signal_connect fbutton, !REXXDW.!DW_CLICKED_EVENT, 'HTMLButtonCallback', !REXXDW.!DW_HTML_GOFORWARD, html
/*
 * Create OK
 */
b1 = dw_button_new( 'OK', 0 )
Call dw_box_pack_start topbox, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (!global.!window_width % 2), (!global.!screen_height % 2) - (!global.!window_height % 2), !global.!window_width, !global.!window_height
Call dw_window_show win
 Call dw_main_sleep 10
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
Return 0

HTMLButtonCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., action, html
If !global.!allowcallbacks = 0 Then Return 1
Call dw_html_action html, action
Return 0

JobsDeleteCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., jobs
If dw_messagebox( 'Delete Jobs?', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), 'Are you sure you want to delete the selected jobs?' ) = !REXXDW.!DW_MB_RETURN_NO Then Return 0
/*
 * we are going to delete all the files
 */
exts.mods = '.html .out .desc .currents.html .summary.html .comments.html .differences.html .detail.html'
exts.testbuild = '.html .out .desc .currents.html .summary.html .comments.html .differences.html .detail.html'
exts.build = '.html .out .desc'
exts.promote = '.html .out .desc'
exts.copy = '.html .out .desc'
exts.make = '.html .out .desc .caller'!global.!shell_ext !global.!shell_ext
nativelogdir = FindNativeLogDirectory()
Do i = 1 To Words( jobs )
   Parse Value Word( jobs, i ) With type ',' basefile
   type = Translate( type )
   Do j = 1 To Words( exts.type )
      ext = Word( exts.type, j )
      Call SysFileDelete nativelogdir||!global.!ossep||basefile||ext
   End
   Call SysFileDelete nativelogdir||!global.!ossep||basefile
End
/*
 * Now rebuild the container
 */
Call DisplayJobs
Return 0

EditLabel: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Create a container box with
 * +-------------------+
 * |  Revision | File  |
 * ------------+-------+
 * | 1.3       | a.txt |
 * +-------------------+
 * |    | Edit |       |
 * +-------------------+
 * |     Apply         |
 * +-------------------+
 * |     Cancel        |
 * +-------------------+
 *
 */
Parse Arg based_on, lab, labelbasefile
flags.0 = 2
flags.1 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
flags.2 = dw_or( !REXXDW.!DW_CFA_STRING, !REXXDW.!DW_CFA_LEFT, !REXXDW.!DW_CFA_SEPARATOR, !REXXDW.!DW_CFA_HORZSEPARATOR )
titles.0 = 2
titles.1 = 'Revision'
titles.2 = 'File'
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Edit Label File', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Create our container in the box...
 */
!global.!editlabelcontainer = dw_container_new( 1, !REXXDW.!DW_SINGLE_SELECTION )
Call dw_box_pack_start topbox, !global.!editlabelcontainer, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
!global.!editlabelcontainer.?firsttime = 1
/*
 * Use a normal container.
 */
Call dw_container_setup !global.!editlabelcontainer, 'flags.', 'titles.', 0

Call DisplayLabelFile labelbasefile
/*
 * Create Edit button...
 */
editbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, editbox, 0, !global.!buttonheight+10, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_box_pack_start editbox, 0, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
b0 = dw_button_new( 'Edit', 0 )
Call dw_box_pack_start editbox, b0, 50, !global.!buttonheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 5
Call dw_box_pack_start editbox, 0, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Create Apply and cancel buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
width = 640
height = 320  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b0, !REXXDW.!DW_CLICKED_EVENT, 'EditLabelEditButtonCallback'
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'EditLabelApplyCallback', win, dialog_wait, labelbasefile
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
Return cancel

DisplayLabelFile: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Determine how many items in the container...
 */
Parse Arg labelbasefile
inlabelworkfile = !global.!current_dirname.!!last_repository'/'labelbasefile
/*
 * Read the label file
 */
Call Stream inlabelworkfile, 'C', 'OPEN READ'
idx = 0
Do While Lines( inlabelworkfile) > 0
   Parse Value Linein( inlabelworkfile ) With rev fn
   If rev = ';' Then Iterate
   idx = idx + 1
   !global.!editlabel.?rev.idx = rev
   !global.!editlabel.?file.idx = fn
End
!global.!editlabel.?rev.0 = idx
!global.!editlabel.?file.0 = idx
Call Stream inlabelworkfile, 'C', 'CLOSE'

/*
 * Delete everything in the container so we can replace the existing files
 */
If !global.!editlabelcontainer.?firsttime = 0 Then
   Do
      Call dw_signal_disconnect !global.!editlabelcontainer, !REXXDW.!DW_ITEM_CONTEXT_EVENT
      Call dw_container_clear !global.!editlabelcontainer, !REXXDW.!DW_DONT_REDRAW
   End
!global.!editlabelcontainer.?firsttime = 0
/*
 * Add the entries into the container
 */
!global.!editlabelcontainer.?containermemory = dw_container_alloc( !global.!editlabelcontainer, !global.!editlabel.?rev.0 )
Do i = 1 To !global.!editlabel.?rev.0
   Call dw_container_set_item !global.!editlabelcontainer, !global.!editlabelcontainer.?containermemory, 0, i-1, !global.!editlabel.?rev.i
   Call dw_container_set_item !global.!editlabelcontainer, !global.!editlabelcontainer.?containermemory, 1, i-1, !global.!editlabel.?file.i
   Call dw_container_set_row_title !global.!editlabelcontainer.?containermemory, i-1, i','!global.!editlabel.?file.i
End
Call dw_container_set_stripe !global.!editlabelcontainer, !REXXDW.!DW_CLR_WHITE, !global.!container_colour
Call dw_container_insert !global.!editlabelcontainer, !global.!editlabelcontainer.?containermemory, !global.!editlabel.?rev.0
Call dw_container_optimize !global.!editlabelcontainer
/*
 * Only now can we connect our signals, because dw_set_file() causes the signals to fire!!!...
 */
Call dw_signal_connect !global.!editlabelcontainer, !REXXDW.!DW_ITEM_CONTEXT_EVENT, 'EditLabelMenuCallback'

Return

EditLabelApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., win, dialog, labelbasefile
/*
 * Write the array to the label working file
 */
labelworkfile = !global.!current_dirname.!!last_repository'/'labelbasefile
Call Stream labelworkfile,'C','OPEN WRITE REPLACE'
Call Lineout labelworkfile, '; QOCCA Label File:' labelbasefile 'for' !global.!current_dirname.!!last_repository
Do i = 1 To !global.!editlabel.?file.0
   Call Lineout labelworkfile, !global.!editlabel.?rev.i !global.!editlabel.?file.i
End
Call Stream labelworkfile,'C','CLOSE'
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Destroy the window
 */
Call dw_window_destroy win
Return 0

/*
 * Create a popup menu with "Edit" items
 */
EditLabelMenuCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., ., x, y
/*
 * Determine which config items items are currently selected
 */
basefiles = ''
selected = dw_container_query_start( !global.!editlabelcontainer, !REXXDW.!DW_CRA_SELECTED )
Do i = 1 While selected \= ''
   basefiles = basefiles selected
   selected = dw_container_query_next( !global.!editlabelcontainer, !REXXDW.!DW_CRA_SELECTED )
End
basefiles = Strip( basefiles )
/*
 * Create our popup and display it
 */
popup = dw_menu_new( 0 )
menuitem = dw_menu_append_item( popup,'Edit', 1001, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'EditLabelEditCallback', basefiles
Call dw_menu_popup popup, !global.!mainwindow, x, y
Return 1

/*
 *
 */
EditLabelEditButtonCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Determine which config items are currently selected
 */
basefiles = ''
selected = dw_container_query_start( !global.!editlabelcontainer, !REXXDW.!DW_CRA_SELECTED )
Do i = 1 While selected \= ''
   basefiles = basefiles selected
   selected = dw_container_query_next( !global.!editlabelcontainer, !REXXDW.!DW_CRA_SELECTED )
End
basefiles = Strip( basefiles ) /* don't quote this string */
/*
 * Call the edit callback
 */
Call EditLabelEditCallback '', basefiles
Return 1

EditLabelEditCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., i','fn
/*
 * Get all potential revisions for the file into a combobox...
 */
newrev = GetFileRevisions( fn, 'Change', 'rev', 'nodate', '' )
If newrev \= '' Then
   Do
      !global.!editlabel.?rev.i = newrev
      /*
       * Now update the container with the new revision
       */
      Call dw_container_set_item !global.!editlabelcontainer, !global.!editlabelcontainer.?containermemory, 0, i-1, !global.!editlabel.?rev.i
   End
Return 0
/*
 * Module level callbacks...
 * All module-level callbacks can be called from a button, or from
 * a menu item.
 * The arguments passed to a button callback are: window
 * The arguments passed to a menu   callback are: window, item index
 * Therefore if you are passing more arguments to the callback,
 * then those arguments must come after window and item index.
 */
RefreshModulesCallback: Procedure Expose !REXXDW. !global. !!last_repository
Call DisplayRepository
Return 0

DirPropertiesCallback: Procedure Expose !REXXDW. !global. !!last_repository
if trace() = 'F' Then say 'got dirproperties' !global.!current_dirname.!!last_repository
/*
 * If this callback occurs on the project level "directory" then
 * display repository properties
 */
If !global.!current_dirname.!!last_repository = '.' Then
   Do
      Select
         When !global.!!repository.!!type.!!last_repository = 'rcs' Then Call GetRCSRepository 'N', !!last_repository
         When !global.!!repository.!!type.!!last_repository = 'cvs' Then Call GetCVSRepository 'N', !!last_repository
      End
   End
Else
   Do
      Call DisplayDirProperties
   End
Return 0

ShowModifiedStrictCallback: /* RCS */ Procedure Expose !REXXDW. !global. !!last_repository
if trace() = 'F' Then say 'got showmodifiedstrict'
Return 0

ShowModifiedCallback: /* RCS */ Procedure Expose !REXXDW. !global. !!last_repository
if trace() = 'F' Then say 'got showmodified'
Return 0

/*
 * This function lists all files that are different between the working
 * directory and the repository
 */
ListModifiedCallback: Procedure Expose !REXXDW. !global. !!last_repository oserr.
Call SetCursorWait
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      /* TODO */
      Do smi = 1 To fullfilename.0
         file = RCSFileToFile( Substr(fullfilename.smi,5) )
         Address System 'rcsdiff -q' diffflags Substr(fullfilename.smi,5) file !global.!devnull
         If rc \= 0 Then Call TkItemConfig mclb, smi-1, '-foreground', 'red'
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         /*
          * We need to turn off cvs compression if talking to a public CVS server as newer versions of CVS
          * use the "inflation" method which is not compatible with the client version
          */
         If !global.!!repository.!!public_cvs.!!last_repository = 'Y' Then cvs_compression = ''
         Else cvs_compression = !global.!!repository.!!cvs_compression.!!last_repository
         If !global.!current_dirname.!!last_repository = '' | !global.!current_dirname.!!last_repository = '.' Then
            Call RunOSCommand 'cvs' cvs_compression '-d'!global.!!repository.!!path.!!last_repository '-q -n update *'
         Else
            Call RunOSCommand 'cvs' cvs_compression '-d'!global.!!repository.!!path.!!last_repository '-q -n update' quote( !global.!current_dirname.!!last_repository )
         Call WriteCVSOutputStemsToLogWindow 'list', '?', 'Files modified in' quote( !global.!current_dirname.!!last_repository )
         /*
          * List any reminder files in the current directory recursively
          */
         here = Directory()
         Call Directory( !global.!current_dirname.!!last_repository )
         Call SysFileTree '*.qocca.reminder', 'REM.', 'FLS'
         If rem.0 \= 0 Then
            Do
               Call WriteMessageHeaderToLogWindow 'Reminders set'
               Do lmi = 1 To rem.0
                  Parse Var rem.lmi . . . . fn
                  fn = Translate( fn, '/', !global.!ossep )
                  wd = Translate( !global.!!repository.!!working_dir.!!last_repository, '/', !global.!ossep )
                  Parse Var fn (wd) base 'CVS/' bn '.qocca.reminder'
                  Call WriteMessageToLogWindow ' 'base||bn, 'yellowback'
               End
               Call WriteMessageTrailerToLogWindow 'Reminders set'
            End
         Call Directory( here )
         /*
          * If we have any errors and they indicate that there is a
          * mismatch between the repository and items listed in
          * CVS/Entries
          * then display them in the messages pane, and display a
          * dialog box requesting confirmation to "cleanup" these
          * entries.
          * This "cleaning up" is removing the selected files from
          * the CVS/Entries file in the user's working directory
          */
         If oserr.0 \= 0 Then Call TidyUpCVSDeletions
         Drop rem.
      End
   Otherwise Nop
End
Call SetCursorNoWait
Return 0
/*
 * This function lists all files that are currently locked out
 */
ListLockedCallback: Procedure Expose !REXXDW. !global. !!last_repository oserr.
Call SetCursorWait
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
      /* TODO */
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         If !global.!current_dirname.!!last_repository = '.' | !global.!current_dirname.!!last_repository = '' Then fname_offset = 1
         Else fname_offset = Length(!global.!current_dirname.!!last_repository)+2
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q list' quote(!global.!current_dirname.!!last_repository)
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output Stem item. Error Stem err.
         Do i = 1 To item.0
            Parse Var item.i 1 . 12 rev 28 user 64 . . 84 type 88 flen 91 . 92 filename +(flen) +1 locks
            If locks \= '' Then
               Do
                  filename = Strip( Substr( filename, fname_offset ) )
                  Queue '('locks')' filename
               End
         End
         Call WriteStackToLogWindow 'default', 'Files currently locked in' quote( !global.!current_dirname.!!last_repository )
      End
   Otherwise Nop
End
Call SetCursorNoWait
Return 0

GetLatestAllCallback: Procedure Expose !REXXDW. !global. !!last_repository
refresh = 'N'
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         If dw_messagebox( 'Get Latest?', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), 'Replace all working files in this directory and below with the latest repository version ?' ) = !REXXDW.!DW_MB_RETURN_YES Then
            Do
               Call SetCursorWait
               Call DisplayStatus 'Getting list of all files in and under' !global.!current_dirname.!!last_repository'...'
               Call GetListOfCVSFilesInDirectory !!last_repository, !global.!current_dirname.!!last_repository, 0
               Do i = 1 To item.0
                  Parse Var item.i . rel . . . . flen .
                  Parse Var item.i 92 fn +(flen) +1 .
                  /*
                   * For any files that have changed locally, save a backup
                   * copy in the working directory
                   */
                  cvs_status = CVSStatus( fn )
                  If cvs_status = 'Up-to-date' | cvs_status = 'Needs Checkout' Then
                     Do
                        Call SysFileDelete fn
                     End
                  Else
                     Do
                        /*
                         * Split the filename into path and file. Change the filename
                         * only.
                         */
                        pos = Lastpos( '/', fn )
                        If pos \= 0 Then
                           Do
                              pos = pos + 1
                              Parse Var fn fnpath =(pos) fnfile
                              newfn = fnpath'.#'fnfile'.'rel
                           End
                        Else
                           Do
                              newfn = fn
                           End
                        Call SysMoveObject fn, newfn
                     End
               End
               Call DisplayStatus 'Getting latest of all files in and under' !global.!current_dirname.!!last_repository'...'
               Call RunOsCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout -R' quote( !global.!current_dirname.!!last_repository )
               Call WriteCVSOutputStemsToLogWindow 'get',, 'Get Latest for' quote( !global.!current_dirname.!!last_repository )
               Call CheckForCVSConflicts
               Drop oserr. osout.
               Call SetCursorNoWait
               Call DisplayStatus ''
               refresh = 'Y'
            End
      End
   Otherwise Nop
End
If refresh = 'Y' Then
   Do
      Call RefreshDirectoryIcons
      /*
       * No need to display non-repository files at this stage
       */
      Call DisplayRepositoryFiles
      Call Directory !global.!!repository.!!working_dir.!!last_repository
   End
Return 0

UpdateAllCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * This file option only valid for CVS
 */
Call SetCursorWait
If !global.!!repository.!!enhanced_cvs.!!last_repository \= 'Y' Then
   Do
      If !global.!current_dirname.!!last_repository = '.' Then dir = ''
      Else dir = quote( !global.!current_dirname.!!last_repository )
   End
Else dir = quote( !global.!current_dirname.!!last_repository )
Call RunOSCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository  '-q update -d' dir
Call WriteCVSOutputStemsToLogWindow 'update','?' , 'Update All for' quote( !global.!current_dirname.!!last_repository )
If  !global.!!repository.!!type.!!last_repository = 'cvs' Then Call CheckForCVSConflicts
Drop oserr. osout.
Call RefreshDirectoryIcons
/*
 * No need to display non-repository files at this stage
 */
Call DisplayRepositoryFiles
Call Directory !global.!!repository.!!working_dir.!!last_repository
Call SetCursorNoWait
Return 0

GetAllCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Dialog box to determine which version to get and to where
 *
 * +---------------------------------------------------+
 * | Text                                              |
 * |                                                   |
 * +---------------------------------------------------+
 * |  Get Version        _Latest___v                   |
 * |  To: __________________________  X                |
 * +---------------------------------------------------+
 */
if trace() = 'F' Then say 'got getall'

title_width = 80

win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Get All', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, 300, 50, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_mle_set_cursor t1, 0
Call dw_mle_import t1, 'Select the version of the module to get and the directory in which the module files are to be saved.' , -1
Call dw_window_disable t1
/* version */
module_version_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, module_version_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Get Version', 0 )
Call dw_box_pack_start module_version_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
Call dw_box_pack_start module_version_box, 0, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Parse Value CreateModuleVersionCombobox( module_version_box, 'false', 'Latest', !global.!current_dirname.!!last_repository, 1, 'Latest' ) With module_version_cb boxtopack .
Call dw_box_pack_start module_version_box, boxtopack, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/* dest dir */
dest_dir_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, dest_dir_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
dest = dw_text_new( 'To', 0 )
Call dw_box_pack_start dest_dir_box, dest, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style dest, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
dest_dir_entry = dw_entryfield_new( '', 0 )
Call dw_box_pack_start dest_dir_box, dest_dir_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
abutton = dw_bitmapbutton_new_from_file( 'Browse', 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start dest_dir_box, abutton, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericDirectorySelector', dest_dir_entry, 'Set Temporary Directory to:'
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
width = 300
height = 200  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10

Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GetAllApplyCallback', win, dialog_wait, module_version_cb, dest_dir_entry
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
/*
 * Wait for the dialog to come back
 */
Return 0

GetAllApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., window, dialog, module_version_cb, dest_dir_entry
destdir = dw_window_get_text( dest_dir_entry )
ver = dw_window_get_text( module_version_cb )
Call CreateDirectory destdir
/*
 * Get rid of the dialog
 */
Call dw_dialog_dismiss dialog, 'apply'
Call dw_window_destroy window

Call SetCursorWait
/*
 * Now get all files for the module into the specified directory
 */
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         Call WriteMessageHeaderToLogWindow 'Getting version' quote( ver ) 'of' quote( !global.!current_dirname.!!last_repository )
          Call dw_main_sleep 10 /* get window updated */
         If ver = 'Latest' Then
            Do
               /*
                * Get a list of all files in the module with their revision
                * numbers
                */
               Call GetListOfCVSFilesInDirectory !!last_repository, !global.!current_dirname.!!last_repository, 0
               Do i = 1 To item.0
                  Parse Var item.i . rev . . . . flen .
                  Parse Var item.i 92 fn +(flen) +1 .
                  item.i = rev fn
               End
               Do i = 1 To err.0
                  Call WriteMessageToLogWindow err.i
               End
            End
         Else
            Do
               /*
                * Get the label file from the repository into the item. stem
                */
               If Countstr( ',', ver ) \= 0 Then Parse Var ver ver_dir ',' rev
               labelworkfile = ver_dir'/.label-'rev
               cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q get -p' quote( labelworkfile )
               If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
               Address System cmd With Output Stem item. Error Stem junk.
            End
         /*
          * We now have a stem; item. with all files and their revision numbers
          */
         dirlen = Length( !global.!current_dirname.!!last_repository )
         Do i = 1 To item.0
            Parse Var item.i rev fn
            /*
             * Filter out files that are not in the current directory
             */
            If Left( fn, dirlen ) \= !global.!current_dirname.!!last_repository Then Iterate
            outfile = destdir||Substr( fn, dirlen+1 )
            /*
             * Need to determine if the directory of the file exists;
             * if not make it before checking the file out
             */
            outpos = Lastpos( '/', outfile )
            outdir = Substr( outfile, 1, outpos-1 )
            Call CreateDirectory( outdir )
            Call WriteMessageToLogWindow '  getting' quote( fn ) 'to' quote( outfile )
             Call dw_main_sleep 10 /* get window updated */
            cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q checkout -p -r'rev quote( fn )
            If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
            Address System cmd With Output Stream outfile Error Stem err.
            Call Stream outfile, 'C', 'CLOSE'
            Do j = 1 To err.0
               Call WriteMessageToLogWindow err.j
            End
         End
         Call WriteMessageTrailerToLogWindow 'Getting version' quote( ver ) 'of' quote( !global.!current_dirname.!!last_repository )
      End
   Otherwise Nop
End
Call SetCursorNoWait

Return 0


CheckOutAllCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Checkout and lock every file that is not locked to anyone
 * DO NOT APPLY TO LABEL FILES
 */
Call SetCursorWait
if trace() = 'F' Then say 'got checkoutall'
checkout = 'N'
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         Call WriteMessageHeaderToLogWindow 'Checking out all files in' quote( !global.!current_dirname.!!last_repository ), 'default'
          Call dw_main_sleep 10 /* get window updated */
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q list' quote( !global.!current_dirname.!!last_repository )
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output Stem item. Error Stem err.
         Do i = 1 To item.0
            Parse Var item.i 1 . 12 rev 28 user 64 . . 84 type 88 flen 91 . 92 filename +(flen) +1 locks
            filename = Strip( filename )
            off = Length( !global.!current_dirname.!!last_repository ) + 2
            basename = Substr( filename, off )
            If Left( basename, 7 ) = '.label-' Then Iterate
            If locks = '' Then
               Do
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout' quote( filename )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address Command cmd With Output FIFO '' Error FIFO ''
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'admin -l' quote( filename )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address Command cmd With Output FIFO '' Error FIFO ''
                  checkout = 'Y'
               End
            Else
               Do
                  Queue filename 'not checked out; locked by' locks
               End
            Call WriteStackToLogWindowNoHeaderTrailer 'default'
             Call dw_main_sleep 10 /* get window updated */
         End
         Call WriteMessageTrailerToLogWindow 'Checking out all files in' quote( !global.!current_dirname.!!last_repository ), 'default'
      End
   Otherwise Nop
End
If checkout = 'Y' Then
   Do
      Call RefreshFilesCallback
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
   End
Call SetCursorNoWait
Return 0

UndoCheckOutAllCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Undo Checkout and unlock every file that is locked by us
 */
Call SetCursorWait
if trace() = 'F' Then say 'got undocheckoutall'
undocheckout = 'N'
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         Call WriteMessageHeaderToLogWindow 'Undoing Checkout all files in' quote( !global.!current_dirname.!!last_repository ), 'default'
          Call dw_main_sleep 10 /* get window updated */
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q list' quote( !global.!current_dirname.!!last_repository )
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output Stem item. Error Stem err.
         continue = 'N'
         Do i = 1 To item.0
            ignore_this_file = 'N'
            Parse Var item.i 1 . 12 rev 28 user 64 . . 84 type 88 flen 91 . 92 filename +(flen) +1 locks
            If Translate( locks ) = Translate( !global.!user ) Then
               Do
                  filename = Strip( filename )
                  off = Length( !global.!current_dirname.!!last_repository ) + 2
                  basename = Substr( filename, off )
                  If Left( basename, 7 ) = '.label-' Then Iterate i
                  If continue = 'N' Then
                     Do
                        cvs_status = CVSStatus( filename )
                        If cvs_status = 'Locally Modified' | cvs_status = 'Needs Merge' | cvs_status = 'File had conflicts on merge' Then
                           Do
                              Call SetCursorNoWait
                              rcode = dw_messagebox( 'Undo Checkout?', dw_or( !REXXDW.!DW_MB_YESNOCANCEL, !REXXDW.!DW_MB_QUESTION ), quote( filename ) 'has been modified locally. Do you want to lose these changes ? Select Yes to lose changes on future files; No to ask this question again; Cancel to quit the action.' )
                              Select
                                 When rcode = !REXXDW.!DW_MB_RETURN_YES Then continue = 'Y'
                                 When rcode = !REXXDW.!DW_MB_RETURN_CANCEL Then Return 0
                                 Otherwise ignore_this_file = 'Y'
                              End
                           End
                     End
                  Call SetCursorWait
                  If ignore_this_file = 'N' Then
                     Do
                        /*
                         * Unlock the file...
                         */
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'admin -u' quote( filename )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output FIFO '' Error FIFO ''
                        /*
                         * Delete the file...
                         */
                        Call SysFileDelete filename
                        /*
                         * Get the latest...
                         */
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout' quote( filename )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output FIFO '' Error FIFO ''
                        undocheckout = 'Y'
                        Call WriteStackToLogWindowNoHeaderTrailer 'default'
                         Call dw_main_sleep 10 /* get window updated */
                     End
               End
         End
         Call WriteMessageTrailerToLogWindow 'Undoing Checkout all files in' quote( !global.!current_dirname.!!last_repository ), 'default'
      End
   Otherwise Nop
End
If undocheckout = 'Y' Then
   Do
      Call RefreshFilesCallback
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
   End
Call SetCursorNoWait
Return 0

CheckInAllCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * --------------------------------------------------------------------
 * This function checks in all files that have been checked out from a
 * directory and its sub-dirs.
 * --------------------------------------------------------------------
 */
if trace() = 'F' Then say 'got checkinall'
checkin = 'N'
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do dummy = 1 To 1
         /* TODO */
         Call SysFileTree !global.!current_dirname'/*,v', 'FILE.', 'FLS'
         If file.0 = 0 Then
            Do
               checkin = 'N'
            End
         Else
            Do
               checkin = 'Y'
               Parse Value Strip( GetCheckinDetails( 'Y', 'dir', !global.!current_dirname, 'Apply same text for all files') ) With cancel same .
               If cancel Then
                  Do
                     Leave
                  End
               If same Then
                  Do
                     num = Queued()
                     ci_lines. = ''
                     Do i = 1 To num
                        Parse Pull line
                        cl_lines.i = line
                     End
                     cl_lines.0 = num
                  End
               Call TkConfig textwin,'-state','normal'
               Call TkInsert textwin, 'end', d2c(10)||'Checkin All for' !global.!current_dirname||d2c(10), 'blueforebold'
               Call TkTcl 'update idletasks'
               Do j = 1 To file.0
                  fullfilename = Word(file.j,5)
                  file = RCSFileToFile( fullfilename )
                  locker = GetRCSLocker(fullfilename)
                  If locker = !global.!user Then
                     Do
                        If same = 1 Then
                           Do i = 1 To cl_lines.0
                              Queue cl_lines.i
                           End
                        Call RunOsCommand 'LIFO> ci -u' file fullfilename
                        Call WriteOutputStemsToLogWindow 'default', 'Checkin for' fullfilename
                     End
                  Else Call Tkinsert textwin, 'end', d2c(10)||'Checkin ignored for' fullfilename||d2c(10), 'blueforebold'
                  Call TkSee textwin, 'end'
               End
               Call TkConfig textwin,'-state','disabled'
            End
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         /*
          * Check if we have any reminder files in the current directory or below
          * that may be checked in
          */
         continue = !REXXDW.!DW_MB_RETURN_YES
         here = Directory()
         Call Directory( !global.!current_dirname.!!last_repository )
         Call SysFileTree '*.qocca.reminder', 'REM.', 'FLS'
         If rem.0 \= 0 Then
            Do
               Call WriteMessageHeaderToLogWindow 'Reminders set'
               Do i = 1 To rem.0
                  Parse Var rem.i . . . . fn
                  fn = Translate( fn, '/', !global.!ossep )
                  fn = Changestr( 'CVS/', fn, '' )
                  fn = Changestr( '.qocca.reminder', fn, '' )
                  fn = Changestr( !global.!!repository.!!working_dir.!!last_repository||!global.!current_dirname.!!last_repository'/', fn, '' )
                  Call WriteMessageToLogWindow '  '!global.!current_dirname.!!last_repository'/'fn, 'yellowback'
               End
               Call WriteMessageTrailerToLogWindow 'Reminders set'
               continue = dw_messagebox( 'Continue Checkin ?', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), 'You have reminders on files in this directory (see log window). Do you wish to continue the Check In ?' )
            End
         Drop rem.
         Call Directory( here )
         /*
          * Get the checkin details from a new dialog box
          * and push the log text onto the stack, and write a
          * temporary file
          */
         If continue = !REXXDW.!DW_MB_RETURN_YES Then
            Do
               Parse Value Strip( GetCheckinDetails( 'Y', 'file', !global.!current_dirname.!!last_repository, '' ) ) With cancel same .
               If cancel = 'apply' Then
                  Do
                     tmpfile = GenerateTempFile() -- we manage the cleanup ourselves
                     Call Stream tmpfile, 'C', 'OPEN WRITE REPLACE'
                     Do Queued()
                        Parse Pull line
                        Call Lineout tmpfile,line
                     End
                     Call Stream tmpfile, 'C', 'CLOSE'
                     Call SetCursorWait
                     Call RunOSCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'commit -F'tmpfile quote( !global.!current_dirname.!!last_repository )
                     Call WriteCVSOutputStemsToLogWindow 'checkin',, 'Checkin All for' quote( !global.!current_dirname.!!last_repository )
                     Call SysFileDelete tmpfile -- delete our VERY temporary file
                     Call CheckForCVSConflicts
                     Call SetCursorNoWait
                     Drop oserr. osout.
                     checkin = 'Y'
                  End
            End
      End
   Otherwise Nop
End
If checkin = 'Y' Then
   Do
      Call RefreshFilesCallback
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
   End
Return 0

MakeCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Dialog box to determine make settings
 *
 * +-------------------------------------------+
 * | Text                                      |
 * |                                           |
 * +-------------------------------------------+
 * |      Make Directory    __________________ |
 * |        Make Command    _________________v |
 * |       Compiler used    _________________v |
 * +-------------------------------------------+
 */
if trace() = 'F' Then say 'got make'
title_width = 150
this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
makecommand = Strip( FindMakeCommand( !global.!current_dirname.!!last_repository ) )
debugmakecommand = Strip( FindDebugMakeCommand( !global.!current_dirname.!!last_repository ) )
custommakecommand = Strip( FindCustomMakeCommand( !global.!current_dirname.!!last_repository ) )
makedirectory = FindMakeDirectory( !global.!current_dirname.!!last_repository )

win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Make Settings', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, 400, 50, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call MakeHelpFocusCallback t1, 'INITIAL', t1
/* make directory */
make_dir_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, make_dir_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Make Directory', 0 )
Call dw_box_pack_start make_dir_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
Call dw_box_pack_start make_dir_box, 0, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
make_dir_entry = dw_entryfield_new( makedirectory, 0 )
Call dw_box_pack_start make_dir_box, make_dir_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect make_dir_entry, !REXXDW.!DW_SET_FOCUS_EVENT, 'MakeHelpFocusCallback', 'MD', t1
/* make command(s) */
make_command_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, make_command_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Make Command', 0 )
Call dw_box_pack_start make_command_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
Call dw_box_pack_start make_command_box, 0, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
make_cb = dw_combobox_new( makecommand, 0 )
Call dw_box_pack_start make_command_box, make_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If makecommand \= '' Then Call dw_listbox_append make_cb, makecommand
If debugmakecommand \= '' Then Call dw_listbox_append make_cb, debugmakecommand
If custommakecommand \= '' Then Call dw_listbox_append make_cb, custommakecommand
Call dw_signal_connect make_cb, !REXXDW.!DW_SET_FOCUS_EVENT, 'MakeHelpFocusCallback', 'MC', t1
/* compiler combobox */
compiler_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, compiler_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Compiler Used', 0 )
Call dw_box_pack_start compiler_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
Call dw_box_pack_start compiler_box, 0, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
compiler_type = FindCompilerType( !global.!current_dirname.!!last_repository )
compiler_cb = CreateCompilersCombobox( compiler_box, 'false', compiler_type )
Call dw_box_pack_start compiler_box, compiler_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect compiler_cb, !REXXDW.!DW_SET_FOCUS_EVENT, 'MakeHelpFocusCallback', 'CC', t1
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 400, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 400, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
width = 450
height = 200  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'MakeApplyCallback', win, dialog_wait, make_dir_entry, make_cb, compiler_cb
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
/*
 * Wait for the dialog to come back
 */
Return 0

MakeHelpFocusCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, which, textwin
help.initial = 'Select the version to build, and whether you want to do a debug or custom build.'
help.md = 'Specify the directory where the make is to be run from. This is relative to the current working directory.'
help.mc = 'Select the command line to run to do the make. The configured command line for a non-debug, debug and custom build are available.'
help.cc = 'Select the compiler to use. This is used for highlighting errors and warnings in the job report.'
Call dw_window_enable textwin
Call dw_mle_set_cursor textwin, 0
Call dw_mle_delete textwin
Call dw_mle_import textwin, help.which, -1
Call dw_window_disable textwin
Return 0

MakeApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, make_dir_entry, make_cb, compiler_cb
makedir = dw_window_get_text( make_dir_entry )
makecommand = dw_window_get_text( make_cb )
compiler_name = dw_window_get_text( compiler_cb )
If makedir = '' | makedir = '.' Then makedir = ''
Else makedir = !global.!ossep || makedir
make_dir = Changestr( '/', !global.!current_dirname.!!last_repository || makedir, !global.!ossep )
Call CreateDirectory make_dir
/*
 * Create our description file for job processing
 */
desc = 'Make of' quote( !global.!current_dirname.!!last_repository ) 'using:' quote( makecommand ) 'in' quote( make_dir )
type = 'make'
compiler_type = ConvertCompilerNameToAbbrev( compiler_name )
basefile = GenerateJobBaseFile()
Call CreateDescriptionFile basefile, desc, type, compiler_type
outputfile = basefile'.out'
runfile = basefile||!global.!shell_ext
runfile_caller = basefile'.caller'!global.!shell_ext

Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Increment the number of running jobs
 */
Call IncrementRunningJobs basefile
/*
 * Set the "make" directory and the BUILD and VERSION env vars in this process
 * so that the child inherits them
 */
here = Directory()
Call Directory( make_dir )
Call Value !global.!buildenvvar, '0', 'ENVIRONMENT'
Call Value !global.!versionenvvar, 'make', 'ENVIRONMENT'
/*
 * Run the runfile in the background now
 */
Call WriteMessageHeaderToLogWindow desc 'submitted'
/*
 * Write out the caller native shell file for execution
 */
Call Stream runfile_caller, 'C', 'OPEN WRITE REPLACE'
Call Lineout runfile_caller, !global.!shell_first_line
Call Lineout runfile_caller, !global.!command_prefix runfile '>' outputfile '2>&1'
Call Lineout runfile_caller, 'exit'
Call Stream runfile_caller, 'C', 'CLOSE'
/*
 * Write out the commands to execute to a native shell file for execution
 */
Call Stream runfile, 'C', 'OPEN WRITE REPLACE'
Call Lineout runfile, !global.!shell_first_line
/* split the make command into multiple lines */
Do Forever
   Parse Var makecommand mc ';' makecommand
   mc = Strip( mc )
   If mc = '' Then Leave
   Call Lineout runfile, mc
End
Call Lineout runfile, !global.!cmdecho !global.!shell_rc '>' basefile
Call Lineout runfile, 'exit'
Call Stream runfile, 'C', 'CLOSE'
/*
say .line !global.!background_start_nowindow !global.!shell_command runfile !global.!background_end With Output Append Stream outputfile Error Append Stream outputfile
address system 'type' runfile
*/
Address System !global.!background_start_nowindow !global.!shell_command runfile_caller !global.!background_end
Call Directory( here )
Call Value !global.!buildenvvar, '', 'ENVIRONMENT'
Call Value !global.!versionenvvar, '', 'ENVIRONMENT'
Return 0

ReportModuleHistoryCallback: Procedure Expose !REXXDW. !global. !!last_repository
if trace() = 'F' Then say 'got reportmodulehistory'
/*
 * Dialog box to determine how and what to show mods...
 *
 * +---------------------------------------------------+
 * | Text                                              |
 * |                                                   |
 * +---------------------------------------------------+
 * |       Compare                  x Ignore whitespace|
 * |  Earlier Version _Latest___v   X Strict           |
 * |               in _Current__v                      |
 * |        and                                        |
 * |  Later   Version _Working__v   x Compare Word Doc |
 * |               in _Current__v                      |
 * |                                                   |
 * +---------------------------------------------------+
 */
if trace() = 'F' Then say 'got reportmodulehistory'
tryparent = 1
Call SetCursorWait
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Report Modifications', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
loc = dw_mle_import( t1, 'Select the labeled versions to report modifications made. Comparisons can be done ignoring whitespace or strictly.', -1 )
Call dw_window_disable t1
Call dw_mle_set_cursor t1, loc
/*
 * Add a horiz box with comboboxes on left, radio buttons on right
 */
data_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, data_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a vertical box for the comboboxes and add the comboboxes
 */
cb_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start data_box, cb_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * comboboxes for later revision
 */
cb1_box = dw_groupbox_new( !REXXDW.!DW_VERT, 'Compare (later revision)', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start cb_box, cb1_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
cb1_h_box1 = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb1_box, cb1_h_box1, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
tmp = dw_text_new( 'Version', 0 )
Call dw_box_pack_start cb1_h_box1, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
Parse Value CreateModuleVersionCombobox( cb1_box, 'false', 'Working', !global.!current_dirname.!!last_repository, tryparent, 'Latest' ) With rev1cbox boxtopack realtagstart
Call dw_box_pack_start cb1_h_box1, boxtopack, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
/*
 * Module selector...
 */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb1_box, tmpbox, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
tmp = dw_text_new( "of", 0 )
Call dw_box_pack_start tmpbox, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
later_entry = dw_entryfield_new( !global.!current_dirname.!!last_repository, 0 )
Call dw_box_pack_start tmpbox, later_entry, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
abutton = dw_bitmapbutton_new_from_file( "Browse...", 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start tmpbox, abutton, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/* MUST create repository combobox here so we can pass handle to ModuleSelector */
rep1cbox = CreateRepositoryCombobox( cb1_box, 'false', 'Current Repository', rev1cbox, realtagstart, '', tryparent )
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'ModuleSelector', later_entry, 'Select Module for Later Version:', rep1cbox
/*
 * Repository selector...
 */
cb1_h_box2 = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb1_box, cb1_h_box2, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
tmp = dw_text_new( "in", 0 )
Call dw_box_pack_start cb1_h_box2, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
/* creation of combobox done above */
Call dw_box_pack_start cb1_h_box2, rep1cbox, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
/*
 * comboboxes for earlier revision
 */
cb2_box = dw_groupbox_new( !REXXDW.!DW_VERT, 'with (earlier revision)', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start cb_box, cb2_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
cb2_h_box1 = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb2_box, cb2_h_box1, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
tmp = dw_text_new( 'Version', 0 )
Call dw_box_pack_start cb2_h_box1, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
Parse Value CreateModuleVersionCombobox( cb2_h_box1, 'false', 'Latest', !global.!current_dirname.!!last_repository, tryparent, 'Working' ) With rev2cbox boxtopack realtagstart
Call dw_box_pack_start cb2_h_box1, boxtopack, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
/*
 * Module selector...
 */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb2_box, tmpbox, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
tmp = dw_text_new( "of", 0 )
Call dw_box_pack_start tmpbox, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
earlier_entry = dw_entryfield_new( !global.!current_dirname.!!last_repository, 0 )
Call dw_box_pack_start tmpbox, earlier_entry, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
abutton = dw_bitmapbutton_new_from_file( "Browse...", 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start tmpbox, abutton, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/* MUST create repository combobox here so we can pass handle to ModuleSelector */
rep2cbox = CreateRepositoryCombobox( cb2_box, 'false', 'Current Repository', rev2cbox, realtagstart, '', tryparent )
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'ModuleSelector', earlier_entry, 'Select Module for Earlier Version:', rep2cbox
/*
 * Repository selector...
 */
cb2_h_box2 = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb2_box, cb2_h_box2, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
tmp = dw_text_new( "in", 0 )
Call dw_box_pack_start cb2_h_box2, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
/* creation of combobox done above */
Call dw_box_pack_start cb2_h_box2, rep2cbox, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
/*
 * Add a vertical box for the two groups of radio buttons and add the radio buttons
 */
group_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start data_box, group_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

g1 = dw_groupbox_new( !REXXDW.!DW_VERT, 'Comparison', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start group_box, g1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
compare_rb1 = dw_radiobutton_new( 'Ignore whitespace', 111 )
Call dw_box_pack_start g1, compare_rb1, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
compare_rb2 = dw_radiobutton_new( 'Strict', 111 )
Call dw_box_pack_start g1, compare_rb2, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
Call dw_radiobutton_set compare_rb1, !REXXDW.!DW_CHECKED

Call dw_box_pack_start group_box, 0, 10, 10, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2

-- show all files
showall_compare_cb = dw_checkbox_new( 'Show All Files', 0 )
Call dw_box_pack_start group_box, showall_compare_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
Call dw_checkbox_set showall_compare_cb, !REXXDW.!DW_UNCHECKED

word_compare_cb = dw_checkbox_new( 'Compare Word Documents', 0 )
Call dw_box_pack_start group_box, word_compare_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
Call dw_checkbox_set word_compare_cb, !REXXDW.!DW_UNCHECKED
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * We now have everything setup, we can connect the destroying of the top-level window
 * to the 'close' callback
 */
Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * Display the window
 */
Call dw_window_set_gravity win, !REXXDW.!DW_GRAV_CENTRE, !REXXDW.!DW_GRAV_CENTRE
Call dw_window_set_pos_size win, 0, 0, 0, 0
Call dw_window_show win

Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'ReportModuleHistoryApplyCallback', win, dialog_wait, rev1cbox, rev2cbox, rep1cbox, rep2cbox, compare_rb1, compare_rb2, later_entry, earlier_entry, word_compare_cb, showall_compare_cb
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )

   If cancel \= 'retry' Then Leave
End

Return 0

LabelCallback: Procedure Expose !REXXDW. !global. !!last_repository
Do Forever
   Parse Value GetLabel() With res lab based_on overwrite_label edit_label_file
   Select
      When res = 'apply' Then
         Do
            If edit_label_file = '' Then
               Do
                  Call dw_messagebox 'Error!', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), 'Either new label or based-on label are blank.'
                  Iterate
               End
            /*
             * Validate the label...
             */
            Parse Value ValidLabel( lab ) With status msg
            If status Then Leave
            Call dw_messagebox 'Error!', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), msg
         End
      When res = 'close' Then Return 0
      Otherwise Nop
   End
End

if trace() = 'F' Then say 'got apply for label ok'

/*
 * generate the label file into our working directory
 */
Parse Value GenerateLabelFile( '', based_on, lab, overwrite_label ) With new_label_file labelbasefile
If new_label_file = 'N' Then Return 0
/*
 * If the user wants to edit the label file, do it here
 */
If edit_label_file Then
   Do
      If EditLabel( based_on, lab, labelbasefile ) = 'close' Then Return 0
   End
Call SetCursorWait
/*
 * Apply the actual label now
 */
Call ApplyLabelFile '', new_label_file, edit_label_file, lab, labelbasefile
/*
 * Refresh the current directory, so the label file shows up in a list of labels
 */
Call RefreshFilesCallback
Call SetCursorNoWait
Return 0


GetLabel: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Get new label file.
 *
 * +---------------------------------------------------+
 * | Text                                              |
 * +---------------------------------------------------+
 * | Based on    ---------------------------------v    |
 * | New Label   ---------------------------------v    |
 * |               x Overwrite Existing Label          |
 * |               x Edit Label File                   |
 * +---------------------------------------------------+
 * |               Apply                               |
 * |               Cancel                              |
 * +---------------------------------------------------+
 *
 */
title_width = 80
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Specify Label', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, 400, 50, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call GetLabelFocusCallback t1, 'INITIAL', t1
/*
 * Add a horiz box for the based-on label
 */
bo_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, bo_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Based on', 0 )
Call dw_box_pack_start bo_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
bo_cb = dw_combobox_new( 'Latest', 0 )
Call dw_box_pack_start bo_box, bo_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect bo_cb, !REXXDW.!DW_SET_FOCUS_EVENT, 'GetLabelFocusCallback', 'OLDLABEL', t1
Call dw_listbox_append bo_cb, 'Latest'
Do i = 1 To !global.!label.0
   If Left( !global.!label.idx, 13 ) \= '__testbuild__' Then Call dw_listbox_append bo_cb, !global.!label.i
End
/*
 * Add a horiz box for the based-on label
 */
nl_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, nl_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'New Label', 0 )
Call dw_box_pack_start nl_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
nl_cb = dw_combobox_new( '', 0 )
Call dw_box_pack_start nl_box, nl_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect nl_cb, !REXXDW.!DW_SET_FOCUS_EVENT, 'GetLabelFocusCallback', 'NEWLABEL', t1
Do i = 1 To !global.!label.0
   If Left( !global.!label.idx, 13 ) \= '__testbuild__' Then Call dw_listbox_append nl_cb, !global.!label.i
End
/*
 * Add checkboxes
 */
overwrite_cb = dw_checkbox_new( 'Overwrite Existing Label', 0 )
Call dw_box_pack_start topbox, overwrite_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
edit_label_file_cb = dw_checkbox_new( 'Edit Label File', 0 )
Call dw_box_pack_start topbox, edit_label_file_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

/*
 * Create a dialog for the window
 */
dialog_wait = dw_dialog_new( )
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 400, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GetLabelApplyCallback', win, dialog_wait, bo_cb, nl_cb, overwrite_cb, edit_label_file_cb

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 400, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * We now have everything setup, we can connect the destroying of the top-level window
 * to the 'close' callback
 */
Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * Display the window
 */
width = 450
height = 250  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
/*
 * Wait for the dialog to come back
 */
Return dw_dialog_wait( dialog_wait )

GetLabelFocusCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, which, textwin
help.initial = 'Specify the label to be associated with all configuration item in this directory and below.'
help.oldlabel = 'Select an existing label (or latest) from the drop-down box. This will be the basis from which a new label will be generated.'
help.newlabel = 'Enter the new label, or select an existing label from the drop-down box. If you re-use a label, you will need to check the Overwrite Existing Label checkbox.'
help.overwrite = 'Select this if you wish to overwrite an existing label. Generally it is not good practice to re-use an existing label.'
help.editlabel = 'If you wish to change the revision number of any configuration item from the generated list, the check this box. This will display a dialog to enable you to make the required changes.'
Call dw_window_enable textwin
Call dw_mle_set_cursor textwin, 0
Call dw_mle_delete textwin
Call dw_mle_import textwin, help.which, -1
Call dw_window_disable textwin
Return 0

BuildCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Dialog box to determine build settings
 *
 * +------------------------------------+
 * | Text                               |
 * |                                    |
 * +------------------------------------+
 * |  Version  _Latest___v              |
 * |  x Debug build  x Custom Build     |
 * +------------------------------------+
 */
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Build Settings', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, !REXXDW.!DW_SIZE_AUTO, 50, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call BuildHelpFocusCallback t1, 'INITIAL', t1
/*
 * Add a horiz box for the label
 */
bo_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, bo_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
tmp = dw_text_new( 'Version to build', 0 )
Call dw_box_pack_start bo_box, tmp, 100, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
bo_cb = dw_combobox_new( 'Latest', 0 )
Call dw_box_pack_start bo_box, bo_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect bo_cb, !REXXDW.!DW_SET_FOCUS_EVENT, 'BuildHelpFocusCallback', 'LABEL', t1
Call dw_listbox_append bo_cb, 'Latest'
Do i = 1 To !global.!label.0
   If Left( !global.!label.idx, 13 ) \= '__testbuild__' Then Call dw_listbox_append bo_cb, !global.!label.i
End
/*
 * Add checkboxes
 */
overwrite_cb = dw_checkbox_new( 'Overwrite Existing Build', 0 )
Call dw_box_pack_start topbox, overwrite_cb, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
g1 = dw_groupbox_new( !REXXDW.!DW_VERT, 'Build Type', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start topbox, g1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
build_rb1 = dw_radiobutton_new( 'Normal Build', 111 )
Call dw_box_pack_start g1, build_rb1, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
build_rb2 = dw_radiobutton_new( 'Debug Build', 111 )
Call dw_box_pack_start g1, build_rb2, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
build_rb3 = dw_radiobutton_new( 'Custom Build', 111 )
Call dw_box_pack_start g1, build_rb3, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_radiobutton_set build_rb1, !REXXDW.!DW_CHECKED
/*
 * Create a dialog for the window
 */
dialog_wait = dw_dialog_new( )
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'BuildApplyCallback', win, dialog_wait, bo_cb, overwrite_cb, build_rb1, build_rb2, build_rb3

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * We now have everything setup, we can connect the destroying of the top-level window
 * to the 'close' callback
 */
Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * Display the window
 */
Call dw_window_set_gravity win, !REXXDW.!DW_GRAV_CENTRE, !REXXDW.!DW_GRAV_CENTRE
Call dw_window_set_pos_size win, 0, 0, 0, 0
Call dw_window_show win
/*
 * Wait for the dialog to come back
 */
Return dw_dialog_wait( dialog_wait )

BuildHelpFocusCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, which, textwin
help.initial = 'Select the version to build, and whether you want to do a debug or custom build.'
help.label = 'Select the version to build'
Call dw_window_enable textwin
Call dw_mle_set_cursor textwin, 0
Call dw_mle_delete textwin
Call dw_mle_import textwin, help.which, -1
Call dw_window_disable textwin
Return 0

BuildApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, label_box, overwrite_cb, build_rb1, build_rb2, build_rb3
lab = dw_window_get_text( label_box )
If dw_checkbox_get( overwrite_cb ) Then overwriteflag = '-o'
Else overwriteflag = ''
Select
   When dw_radiobutton_get( build_rb2 ) Then buildflag = '-bdebug'
   When dw_radiobutton_get( build_rb3 ) Then buildflag = '-bcustom'
   Otherwise buildflag = ''
End
/*
 * Call the command-line build
 */
desc = 'Build of version' quote( lab ) 'of' quote( !global.!current_dirname.!!last_repository )
type = 'build'
compiler_type = FindCompilerType( !global.!current_dirname.!!last_repository )
basefile = GenerateJobBaseFile()
Call CreateDescriptionFile basefile, desc, type, compiler_type
outputfile = basefile'.out'
cmd = !global.!qocca_command 'build -n'!global.!!repository.!!name.!!last_repository quote( '-m'!global.!current_dirname.!!last_repository ) quote( '-v'lab ) buildflag overwriteflag '-B'basefile
Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Increment the number of running jobs
 */
Call IncrementRunningJobs basefile
/*
 * Run it now
 */
Call WriteMessageHeaderToLogWindow 'Build of version' quote( lab ) 'of' quote( !global.!current_dirname.!!last_repository ) 'submitted'
Address System !global.!background_start_nowindow cmd !global.!background_end
Return 0

PromoteCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Dialog box to determine what to promote
 *
 * +-----------------------------+
 * | Text                        |
 * |                             |
 * +-----------------------------+
 * |  Version  _Working__v       |
 * |  Environment -------v       | not implemented
 * +-----------------------------+
 */
if trace() = 'F' Then say 'got promote'
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Promote', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, 400, 50, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_mle_set_cursor t1, 0
Call dw_mle_import t1, 'Select the version to promote', -1
Call dw_window_disable t1
/*
 * Add a horiz box for the label
 */
version_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, version_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Version to promote', 0 )
Call dw_box_pack_start version_box, tmp, 150, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
version_cb = dw_combobox_new( 'Latest', 0 )
Call dw_box_pack_start version_box, version_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_listbox_append version_cb, 'Latest'
Do i = 1 To !global.!label.0
   If Left( !global.!label.idx, 13 ) \= '__testbuild__' Then Call dw_listbox_append version_cb, !global.!label.i
End
/*
 * Create a dialog for the window
 */
dialog_wait = dw_dialog_new( )
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 400, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'PromoteApplyCallback', win, dialog_wait, version_cb

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 400, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * We now have everything setup, we can connect the destroying of the top-level window
 * to the 'close' callback
 */
Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * Display the window
 */
width = 450
height = 200  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
/*
 * Wait for the dialog to come back
 */
Return dw_dialog_wait( dialog_wait )

PromoteApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, version_cb
lab = dw_window_get_text( version_cb )
/*
 * Call the command-line promote
 */
desc = 'Promote of version' quote( lab ) 'of' quote( !global.!current_dirname.!!last_repository )
type = 'promote'
compiler_type = '.'
basefile = GenerateJobBaseFile()
Call CreateDescriptionFile basefile, desc, type, compiler_type
outputfile = basefile'.out'
cmd = !global.!qocca_command 'promote -n'!global.!!repository.!!name.!!last_repository quote( '-m'!global.!current_dirname.!!last_repository ) quote( '-v'lab ) '-B'basefile

Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Increment the number of running jobs
 */
Call IncrementRunningJobs basefile
/*
 * Run it now
 */
Call WriteMessageHeaderToLogWindow 'Promote of version' quote( lab ) 'of' quote( !global.!current_dirname.!!last_repository ) 'submitted'
Address System !global.!background_start_nowindow cmd !global.!background_end
Return 0

LinkDirCallback: Procedure Expose !REXXDW. !global. !!last_repository
if trace() = 'F' Then say 'got linkdir'
Call CreateModuleTree !!last_repository, '!link', 'Link Directory:' quote( !global.!current_dirname.!!last_repository ) 'to:'
Return 0

ModuleSelector: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., entry, title, repcombobox
rep = dw_window_get_text( repcombobox )
If rep = 'Current Repository' Then repno = !!last_repository
Else repno = FindRepository( rep )
Call CreateModuleTree repno, '!select', title, entry
Return 0

CreateModuleTree: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg repno, type, title, entry
type = Translate( type )
selectcallback.!link = 'LinkDirSelectCallback'
applycallback.!link = 'LinkDirApplyCallback'
selectcallback.!select = 'SelectDirSelectCallback'
applycallback.!select = 'SelectDirApplyCallback'
Parse Value dw_window_get_pos_size( !global.!tree.!!last_repository ) With . . width height
Parse Value dw_window_get_pos_size( !global.!mainwindow ) With x y . .
win = dw_window_new( !REXXDW.!DW_DESKTOP, title, !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Generate a tree matching the specified repository tree
 */
linktreebox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start topbox, linktreebox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
!global.!linktree = dw_tree_new( 101 )
Call dw_box_pack_start linktreebox, !global.!linktree, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 1
/*
 * Setup our signals. On a tree, these MUST be done before we populate the tree.
 */
Call dw_signal_connect !global.!linktree, !REXXDW.!DW_ITEM_SELECT_EVENT, selectcallback.type, entry
Select
   When !global.!!repository.!!type.repno = 'rcs' Then
      Do
         /* TODO */
      End
   When !global.!!repository.!!type.repno = 'cvs' Then
      Do
         /*
          * Build the repository tree from the !global.!repdir. stem if we have it
          * or go and get the modules from the repository
          */
         If  !global.!tree.repno = -1 Then
            Do
               Call dw_messagebox 'Not implemented!', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Cannot (yet) display tree of repository not previously selected.'
            End
         Else
            Do
               idx = DetermineModuleIcon( )
               icon = !global.!moduleicon.idx
               !global.!linktreeparentitem = dw_tree_insert( !global.!linktree, '['!global.!!repository.!!name.repno']', icon, 0, '0' )
               Do i = 1 To !global.!repdir.repno.0
                  mydir = !global.!repdir.repno.i
                  If mydir = '/admin' | mydir = '/CVSROOT' Then Iterate
                  Parse Value DetermineModuleIconAndName( mydir ) With icon currentdir
                  pos = Lastpos( '/', mydir )
                  parent = Substr( mydir, 1, pos-1 )
                  parentidx = FindTreeItem( parent )
                  If parentidx = 0 Then parentitem = !global.!linktreeparentitem
                  Else parentitem = !global.!linktreeitem.parentidx
                  !global.!linktreeitem.i = dw_tree_insert( !global.!linktree, currentdir, icon, parentitem, parentitem !global.!repdir.repno.i )
               End
            End
      End
   Otherwise Nop
End
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, applycallback.type, win, dialog_wait, entry
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
Return 0

CopyRepositoryCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Dialog box to determine what to copy where...
 *
 * +---------------------------------------------------+
 * | Text                                              |
 * |                                                   |
 * +---------------------------------------------------+
 * |  Version to copy    __________v                   |
 * |  New Directory Base _________ X                   |
 * |  Overwrite x                                      |
 * +---------------------------------------------------+
 */
if trace() = 'F' Then say 'got copyrepository'
title_width = 100
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Copy Repository', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, 400, 50, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_mle_set_cursor t1, 0
Call dw_mle_import t1, 'Select the labeled version to copy to the new repository.', -1
Call dw_window_disable t1
/*
 * Add a horiz box for the version
 */
version_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, version_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Version to copy', 0 )
Call dw_box_pack_start version_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
version_cb = dw_combobox_new( 'Latest', 0 )
Call dw_box_pack_start version_box, version_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_listbox_append version_cb, 'Latest'
Do i = 1 To !global.!label.0
   If Left( !global.!label.idx, 13 ) \= '__testbuild__' Then Call dw_listbox_append version_cb, !global.!label.i
End
/* dest dir */
dest_dir_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, dest_dir_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'New Base Directory', 0 )
Call dw_box_pack_start dest_dir_box, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
dest_dir_entry = dw_entryfield_new( '', 0 )
Call dw_box_pack_start dest_dir_box, dest_dir_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
abutton = dw_bitmapbutton_new_from_file( 'Browse', 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start dest_dir_box, abutton, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericDirectorySelector', dest_dir_entry, 'Set Temporary Directory to:'
/*
 * Add checkboxes
 */
overwrite_cb = dw_checkbox_new( 'Overwrite New Repository', 0 )
Call dw_box_pack_start topbox, overwrite_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Create a dialog for the window
 */
dialog_wait = dw_dialog_new( )
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 400, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'CopyRepositoryApplyCallback', win, dialog_wait, version_cb, dest_dir_entry, overwrite_cb

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 400, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * We now have everything setup, we can connect the destroying of the top-level window
 * to the 'close' callback
 */
Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * Display the window
 */
width = 450
height = 200  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
/*
 * Wait for the dialog to come back
 */
Return dw_dialog_wait( dialog_wait )

CopyRepositoryApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, version_cb, dest_dir_entry, overwrite_cb
lab = dw_window_get_text( version_cb )
destdir = dw_window_get_text( dest_dir_entry )
overwrite = dw_checkbox_get( overwrite_cb )
If overwrite Then overwrite = '-o'
Else overwrite = ''
/*
 * Call the command-line copy
 */
desc = 'Copy of version' quote( lab ) 'of' quote( !global.!current_dirname.!!last_repository ) 'to' quote( destdir )
type = 'copy'
compiler_type = '.'
basefile = GenerateJobBaseFile()
Call CreateDescriptionFile basefile, desc, type, compiler_type
outputfile = basefile'.out'
cmd = !global.!qocca_command "copy -n"!global.!!repository.!!name.!!last_repository "-m"!global.!current_dirname.!!last_repository "-v"lab '-d'destdir '-B'basefile overwrite

Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Increment the number of running jobs
 */
Call IncrementRunningJobs basefile
/*
 * Run it now
 */
Call WriteMessageHeaderToLogWindow 'Copy of version' quote( lab ) 'of' quote( !global.!current_dirname.!!last_repository ) 'to' quote( destdir ) 'submitted'
Address System !global.!background_start_nowindow cmd !global.!background_end
Return 0

AddBookmarkCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Adds a bookmark
 */
if trace() = 'F' Then say 'got addbookmark'
/*
 * Check if the bookmark already exists for this repository
 */
found = 0
Do i = 1 To !global.!!user.!!bookmark.0
   If !global.!!user.!!bookmark_repository.i = !!last_repository & !global.!!user.!!bookmark.i = !global.!current_dirname.!!last_repository Then
      Do
         found = 1
         Leave
      End
End
If found = 0 Then
   Do
      idx = !global.!!user.!!bookmark.0 + 1
      !global.!!user.!!bookmark.idx = !global.!current_dirname.!!last_repository
      !global.!!user.!!bookmark_repository.idx = !!last_repository
      !global.!!user.!!bookmark.0 = idx
      Call PopulateBookmarks
      Call AdjustDirToolbarList
      Call AdjustDirToolbarButtons
      Call WriteUserSettingsFile
   End
Return 0

DeleteBookmarkCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Deletes a bookmark
 */
if trace() = 'F' Then say 'got addbookmark'
/*
 * Check if the bookmark exists for this repository
 */
found = 0
Do i = 1 To !global.!!user.!!bookmark.0
   If !global.!!user.!!bookmark_repository.i = !!last_repository & !global.!!user.!!bookmark.i = !global.!current_dirname.!!last_repository Then
      Do
         found = i
         Leave
      End
End
If found \= 0 Then
   Do
      Call SysStemDelete !global.!!user.!!bookmark., found
      Call SysStemDelete !global.!!user.!!bookmark_repository., found
      Call PopulateBookmarks
      Call AdjustDirToolbarList
      Call AdjustDirToolbarButtons
      Call WriteUserSettingsFile
   End
Return 0

/*
 * File level callbacks...
 * All file-level callbacks can be called from a button, or from
 * a menu item.
 * The arguments passed to a button callback are: window
 * The arguments passed to a menu   callback are: window, item index
 * Therefore if you are passing more arguments to the callback,
 * then those arguments must come after window and item index.
 */

RefreshFilesCallback: Procedure Expose !REXXDW. !global. !!last_repository
Select
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then Call DisplayRepositoryFiles
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?nonrepfiles.!!last_repository Then Call DisplayNonRepositoryFiles
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?deletedfiles.!!last_repository Then Call DisplayDeletedFiles
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then Call DisplayAllFiles
   Otherwise Nop
End
Return 0

FilePropertiesCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Display the properties of the selected file
 * Invalid for multiple selections
 */
Call DisplayStatus 'Getting file properties...'
filepropchange = 'N'
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         Call RunOsCommand 'rlog -t' quote( current_filenames.1 )
         Call WriteOutputStemsToLogWindow 'default', 'Properties of' quote( current_filenames.1 )
         Call RunOsCommand 'rlog' quote( current_filenames.1 )
         Call WriteOutputStemsToLogWindow 'default', 'History for'quote( current_filenames.1 )
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         Select
            When !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then file = !global.!current_filenames.1
            When !global.!filespage.!!last_repository = !global.!filenotebookpage.?deletedfiles.!!last_repository Then file = !global.!current_deletedfilenames.1
            When !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then file = !global.!current_filenames.1
            Otherwise Nop
         End
         -- Get log comments and file properties from CVS
         Call GetCVSFileProperties file
          -- Display the contents of osout. and prop. stems in a seperate window
         filepropchange = ShowFileHistoryWindow( file )
         If !global.!filespage.!!last_repository \= !global.!filenotebookpage.?repfiles.!!last_repository Then filepropchange = 'N'
      End
   Otherwise Nop
End
/*
 * If the file properties were changed, refresh the current file in case the icon type
 * has changed, or the file is no checkoutable.
 */
If filepropchange = 'Y' Then
   Do
      Call RefreshDirectoryIcons
      Call RefreshSelectedFiles
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
   End
Call DisplayStatus ''
Return 0

AddFilesCallback: Procedure Expose !REXXDW. !global. !!last_repository
if trace() = 'F' Then say 'got addfiles'
/*
 * Do some validation first...
 */
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' & !global.!filespage.!!last_repository = !global.!filenotebookpage.?nonrepfiles.!!last_repository Then
      Do
         /*
          * Don't let the user add the selected files/directories if there are
          * any CVS working files; ie look for CVS/Entries. Only valid for non-rep files
          */
         continue = 1
         Do i = 1 To !global.!current_nonrepindexes.0
            Call SetCursorNoWait
            selected = !global.!current_nonrepindexes.i
            selected_file = Substr( !global.!nonrepfiles.?fullfilename.!!last_repository.selected, 5 )
            filetype = Substr( !global.!nonrepfiles.?filetype.!!last_repository.selected, 5 )
            If filetype = 'dir' Then
               Do
                  /*
                   * The selected directory will never be CVS; we have excluded it already.
                   */
                  Call SysFileTree selected_file'/Entries', 'ENTRIES.', 'OSF' /* filename only, search subdirs, files only*/
                  Do j = 1 To entries.0
                     If Right( entries.j, 12 ) = '/CVS/Entries' Then
                        Do
                           continue = 0
                           Leave i /* bail out of everything */
                        End
                  End
                  Drop entries.
               End
         End
         If continue = 0 Then
            Do
               Call dw_messagebox 'Abort!', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'At least 1 directory contains CVS working files in a "CVS" directory; delete these "CVS" directories before continuing.'
               Return 0
            End
      End
   Otherwise Nop
End
Select
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?nonrepfiles.!!last_repository Then
      Do
         /*
          * Get each selected item from the non-repository container, and for each
          * item, determine if it is a file or directory.
          * If we added at least one directory, we need to rebuild the tree with the new
          * directories.
          * After procesing all the selected items, refresh the non-repository container.
          */
         same = 0
         file_added = 0
         dir_added = 0
         Do i = 1 To !global.!current_nonrepindexes.0
            Call SetCursorNoWait
            selected = !global.!current_nonrepindexes.i
            selected_file = Substr( !global.!nonrepfiles.?fullfilename.!!last_repository.selected, 5 )
            /*
             * Do not add the file if the file extension is in !global.!ignorefiles
             */
            If IsFileTypeIgnored( selected_file ) Then Iterate i
            filetype = Substr( !global.!nonrepfiles.?filetype.!!last_repository.selected, 5 )
            /*
             * Get the details for the files/directories to be checked in...
             */
            If same = 0 Then
               Do
                  Parse Value Strip( GetCheckinDetails( 'Y', 'add', selected_file, 'Same Description for all selected modules?', 'Adding:' ) ) With cancel same .
                  If cancel = 'close' Then Leave
                  /*
                   * What is queued is the log message; ie the stuff that is written with the
                   * CVS commit
                   */
                  tmpfile = GenerateTempFile() -- we manage the cleanup ourselves
                  Call Stream tmpfile, 'C', 'OPEN WRITE REPLACE'
                  num = Queued()
                  ci_lines. = ''
                  Do m = 1 To num
                     Parse Pull line
                     cl_lines.m = line
                     Call Lineout tmpfile, line
                  End
                  cl_lines.0 = num
                  Call Stream tmpfile, 'C', 'CLOSE'
               End
            Call SetCursorWait
            If filetype = 'dir' Then
               Do
                  /*
                   * Add the directories(s) to the repository
                   */
                  sub_modules. = ''
                  sub_modules.0 = 0
                  base_modules. = ''
                  base_modules.0 = 0
                  sub_files. = ''
                  sub_files.0 = 0
                  Call SysStemInsert 'base_modules.', 1+base_modules.0, selected_file
                  /* get all directories...*/
                  If Stream( selected_file, 'C','QUERY EXISTS' ) = '' Then newdir.0 = 0
                  Else Call SysFileTree selected_file'/*', 'NEWDIR.', 'DLS'
                  Do ndi = 1 To newdir.0
                     Call SysStemInsert 'sub_modules.', 1+sub_modules.0, Changestr( !global.!ossep, SubWord( newdir.ndi, 5 ), '/' )
                  End
                  /* get all files...*/
                  If Stream( selected_file, 'C','QUERY EXISTS' ) = '' Then newfile.0 = 0
                  Else Call SysFileTree selected_file'/*', 'NEWFILE.', 'LSF'
                  Do ndi = 1 To newfile.0
                     Call SysStemInsert 'sub_files.', 1+sub_files.0, Changestr( !global.!ossep, SubWord( newfile.ndi, 5 ), '/' )
                  End
                  /*
                   * Process all files in each directory recursively...
                   */
                  Select
                     When !global.!!repository.!!type.!!last_repository = 'rcs' Then
                        Do
                           /*
                           TODO
                            * For each directory in base_modules AND sub_modules, determine
                            * the files in each, and add them...
                            * Also for each directory, need to create new directory and 'RCS'
                            * directory in the repository
                            * The below relies on a CVS directory and Entries, Repository, Root
                            * files in the root working dir for the repository TODO
                            */
                        End
                     When !global.!!repository.!!type.!!last_repository = 'cvs' Then
                        Do
                           /*
                            * 'cvs add' each base module
                            */
                           Call WriteMessageHeaderToLogWindow 'Adding Selected Directories'
                           Do k = 1 To base_modules.0
                              bm = base_modules.k
                              sbm = Changestr( !global.!!repository.!!working_dir.!!last_repository, bm, '' )
                              Call WriteStringToLogWindow 'Adding' quote( sbm )'...'
                               Call dw_main_sleep 10 /* get window updated */
                              Call RunOSCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'add -m"'!global.!addfile_text'"' quote( sbm )
                              If result = 0 Then Call WriteMessageToLogWindow 'OK'
                              Else
                                 Do
                                    Call WriteMessageToLogWindow 'ERROR', 'redfore'
                                    Do wtfi = 1 To oserr.0
                                       Call WriteMessageToLogWindow '  'oserr.wtfi, 'redfore'
                                    End
                                 End
                           End
                           Do k = 1 To sub_modules.0
                              sm = sub_modules.k
                              ssm = Changestr( !global.!!repository.!!working_dir.!!last_repository, sm, '' )
                              Call WriteStringToLogWindow 'Adding' quote( ssm )'...'
                               Call dw_main_sleep 10 /* get window updated */
                              Call RunOSCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'add -m"'!global.!addfile_text'"' quote( ssm )
                              If result = 0 Then Call WriteMessageToLogWindow 'OK'
                              Else
                                 Do
                                    Call WriteMessageToLogWindow 'ERROR', 'redfore'
                                    Do wtfi = 1 To oserr.0
                                       Call WriteMessageToLogWindow '  'oserr.wtfi, 'redfore'
                                    End
                                 End
                           End
                           Call WriteMessageTrailerToLogWindow 'Adding Selected Directories'
                            Call dw_main_sleep 10 /* get window updated */
                           /*
                            * Now 'cvs commit' the top-level modules
                            */
                           Call WriteMessageHeaderToLogWindow 'Committing Selected Directories'
                           Do k = 1 To base_modules.0
                              bm = base_modules.k
                              sbm = Changestr( !global.!!repository.!!working_dir.!!last_repository, bm, '' )
                              Call RunOsCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'commit -F'tmpfile quote( sbm )
                              Call WriteOutputStemsToLogWindow 'default', 'Commiting' quote( sbm )
                               Call dw_main_sleep 10 /* get window updated */
                           End
                           Call WriteMessageTrailerToLogWindow 'Committing Selected Directories'
                            Call dw_main_sleep 10 /* get window updated */
                           /*
                            * For each file to be imported, 'cvs add', 'cvs commit' it
                            * If its a binary file, add '-kb'
                            */
                           Call WriteMessageHeaderToLogWindow 'Adding Selected Files'
                           Do k = 1 To sub_files.0
                              sf = sub_files.k
                              /*
                               * Do not add the file if the file extension is in !global.!ignorefiles
                               */
                              If IsFileTypeIgnored( sf ) Then Iterate k
                              /*
                               * Check if its a binary file...
                               */
                              iabf = IsBinaryFile( sf )
                              If iabf Then
                                 Do
                                    ibf = '-kb'
                                    Call WriteStringToLogWindow 'Adding binary file' quote( sf )'...'
                                 End
                              Else
                                 Do
                                    ibf = ''
                                    Call WriteStringToLogWindow 'Adding text file' quote( sf )'...'
                                 End
                              ssf = Changestr( !global.!!repository.!!working_dir.!!last_repository, sf, '' )
                              Call RunOSCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'add' ibf '-m"'!global.!addfile_text'"' quote( ssf )
                              If result = 0 Then
                                 Do
                                    Call RunOsCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'commit -F'tmpfile quote( ssf )
                                    If result = 0 Then
                                       Do
                                          Call WriteMessageToLogWindow 'OK'
                                          /*
                                           * Update description
                                           */
                                          cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'admin -t-"'!global.!addfile_text'"' quote( ssf )
                                          If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                                          Address System cmd With Output Stem junk. Error Stem junk.
                                       End
                                    Else
                                       Do
                                          Call WriteMessageToLogWindow 'ERROR', 'redfore'
                                          Do wtfi = 1 To oserr.0
                                             Call WriteMessageToLogWindow '  'oserr.wtfi, 'redfore'
                                          End
                                       End
                                 End
                              Else
                                 Do
                                    Call WriteMessageToLogWindow 'ERROR', 'redfore'
                                    Do wtfi = 1 To oserr.0
                                       Call WriteMessageToLogWindow '  'oserr.wtfi, 'redfore'
                                    End
                                 End
                              If iabf Then
                                 Do
                                    cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q admin -kb' quote( ssf )
                                    If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                                    Address Command cmd With Output Stem junk. Error Stem junk.
                                    cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q update' quote( ssf )
                                    If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                                    Address Command cmd With Output Stem junk. Error Stem junk.
                                 End
                               Call dw_main_sleep 10 /* get window updated */
                           End
                           Call WriteMessageTrailerToLogWindow 'Adding Selected Files'
                           dir_added = 1
                        End
                     Otherwise Nop
                  End
               End
            Else
               Do
                  Select
                     When !global.!!repository.!!type.!!last_repository = 'rcs' Then
                        Do
                          /* TODO */
                           If same = 0 Then
                              Do
                                 Parse Value Strip( GetCheckinDetails( 'Y', 'add', selected_file, 'Same Description for all files', 'Checking in:' ) ) With cancel same .
                                 If cancel = 'close' Then Leave
                                 num = Queued()
                                 ci_lines. = ''
                                 Do m = 1 To num
                                    Parse Pull line
                                    cl_lines.m = line
                                 End
                                 cl_lines.0 = num
                              End
                           Else
                              Do
                                 Do m = 1 To cl_lines.0
                                    Queue cl_lines.m
                                 End
                              End
                           basefile = Changestr( current_working_dir, selected_file, '' )
                           fullfilename = !global.!current_dirname'/RCS/'basefile',v'
                           Call RunOsCommand 'LIFO> ci -u' quote( selected_file ) quote( fullfilename )
                           Call WriteOutputStemsToLogWindow 'default', 'Adding new file:' quote( selected_file )
                        End
                     When !global.!!repository.!!type.!!last_repository = 'cvs' Then
                        Do
                           /*
                            * We either have cl_lines. stem from the most recent
                            * GetCheckinDetails() or from the first one if "same"
                            * comment for all files selected
                            */
                           Do m = 1 To cl_lines.0
                              Queue cl_lines.m
                           End
                           If filetype = 'binary file' Then kflag = '-kb'
                           Else kflag = ''
         /*
                           aaf_here = Directory()
                           Call Directory !global.!current_working_dir.!!last_repository
         */
                           basefile = Changestr( !global.!current_working_dir.!!last_repository||!global.!ossep, Changestr( '/', selected_file, !global.!ossep ), '' )
                           abs_wd = Stream( Strip( !global.!!repository.!!working_dir.!!last_repository, 'T', '/'), 'C', 'QUERY EXISTS' )
                           basefile = Substr( basefile, Length( abs_wd ) + 2 )
                           Call RunOSCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'add' kflag '-m"'!global.!addfile_text'"' quote( basefile )
                           Call WriteOutputStemsToLogWindow 'default', 'Adding new' filetype':' quote( selected_file )
                           Call RunOsCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'commit -F'tmpfile quote( basefile )
                           Call WriteOutputStemsToLogWindow 'default', 'Commiting new' filetype':' quote( selected_file )
                           file_added = 1
                           /*
                            * After an 'add', there should only be one line on the
                            * queue
                            */
                           cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'admin -t-"'!global.!addfile_text'"' quote( basefile )
                           If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                           Address System cmd With Output Stem osout. Error Stem oserr.
                           Call WriteOutputStemsToLogWindow 'default', 'Updating Description for new' filetype':' quote( selected_file )
         /*                  Call Directory aaf_here */
                        End
                     Otherwise Nop
                  End
               End
         End
         /*
          * If we have added at least one directory, refresh the repository
          */
         If dir_added Then Call DisplayRepository
         Else
            Do
               If file_added Then
                  Do
                     Call DisplayRepositoryFiles
                     Call DisplayNonrepositoryFiles
                  End
            End
         Call SysFileDelete tmpfile
      End
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?deletedfiles.!!last_repository Then
      Do
         /*
          * Get each selected item from the deleted files container, and for each
          * item "undelete" it
          * After procesing all the selected items, refresh the deleted files container.
          */
         same = 0
         file_added = 0
         Select
            When !global.!!repository.!!type.!!last_repository = 'cvs' Then
               Do
                  Do i = 1 To !global.!current_deletedindexes.0
                     selected = !global.!current_deletedindexes.i
                     selected_file = Substr( !global.!deletedfiles.?fullfilename.!!last_repository.selected, 5 )
                     Call GetCVSRevisions !!last_repository, selected_file, 'rev'
                     lastrev = ''
                     Do j = 1 To Queued()
                        Parse Pull newrev
                        If j = 1 Then lastrev = newrev
                     End
                     If lastrev \= '' Then
                        Do
                           cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'update -p -r'lastrev quote( selected_file )
                           If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                           Address Command cmd With Output Stream selected_file Error FIFO ''
                           cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'add' quote( selected_file )
                           If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                           Address Command cmd With Output FIFO '' Error FIFO ''
                           cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'ci -m"Undeleted revision' lastrev'"' quote( selected_file )
                           If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                           Address Command cmd With Output FIFO '' Error FIFO ''
                           Call WriteStackToLogWindow 'cvs_status', 'Undelete for' quote( selected_file )
                           file_added = 1
                        End
                  End
               End
            Otherwise Nop
         End
         If file_added Then
            Do
               Call DisplayRepositoryFiles
               Call DisplayDeletedFiles
            End
      End
   Otherwise Nop
End
Call SetCursorNoWait
Return 0

/*
 * View a selected revision of a file.
 * Only a single file is allowed to be selected to get here, hence we can
 * use current_filenames.1
 */
ViewPriorCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg window
rev = GetFileRevisions( !global.!current_filenames.1, 'View', 'rev tag', 'date', 'Latest' )
If rev = '' Then Return 0
Return ViewLatestCallback( window, 0, rev )

ViewLatestCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * If the current user has the file checked out, view the checked-out
 * file, otherwise, check out the file and view it.
 * Valid for multiple selections only if NO rev arg passed
 */
Parse Arg window, itemidx, rev
files_different = 0
workingfiles. = ''
workingfiles.0 = 0
If !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository | !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then
   Do
      If !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then doing_repfiles = 1
      Else doing_repfiles = 0
      Select
         When !global.!!repository.!!type.!!last_repository = 'rcs' Then
            Do
               /* TODO - multiple filenames */
               locker = GetRCSLocker( current_filenames )
               If Translate( locker ) = Translate( !global.!user ) Then
                  Do
                     workingfile = RCSFileToFile( current_filenames )
                  End
               Else
                  Do
                     tmpfile = GenerateTempFile( '?????-'Translate( !global.!current_filenames.1, '_', '/' ) )
                     Address System 'co -p -r'rev current_filenames '>' tmpfile !global.!stderrdevnull
                     workingfile = tmpfile
                  End
            End
         When !global.!!repository.!!type.!!last_repository = 'cvs' Then
            Do
               If rev \= '' Then
                  Do
                     /*
                      * Only 1 file supported, so can use !global.!current_filenames.1
                      */
                     Select
                        When Words( rev ) = 3 Then rev_flag = '-D'
                        When rev = 'Latest' Then
                           Do
                              rev_flag = ''
                              rev = ''
                           End
                        Otherwise rev_flag = '-r'
                     End
                     tmpfile = GenerateTempFile( '?????-'Translate( !global.!current_filenames.1, '_', '/' ) )
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q checkout -p' rev_flag||rev quote( !global.!current_filenames.1 )
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address System cmd With Output Stream tmpfile Error Stem junk.
                     workingfiles.0 = 1
                     workingfiles.1 = tmpfile
                  End
               Else
                  Do
                  /*
                   * Save the !global.!current_filenames. array and then for each element in
                   * the array set !global.!current_filenames. array to the single element
                   */
                  Call SysStemCopy '!global.!current_filenames.', 'save_current_filenames.'
                  Do vi = 1 To save_current_filenames.0
                     !global.!current_filenames.1 = save_current_filenames.vi
                     !global.!current_filenames.0 = 1
                     workingfile = CVSFileToWorkingFile( !global.!current_filenames.1, !!last_repository )
                     wfi = workingfiles.0 + 1
                     workingfiles.wfi = workingfile
                     workingfiles.0 = wfi
                     If Stream( workingfile, 'C', 'QUERY EXISTS' ) \= '' Then file_checked_out = 1
                     Else file_checked_out = 0
                     Select
                        When !global.!offline Then Nop
                        When file_checked_out = 0 Then
                           Do
                              Call GetLatestCallback
                           End
                        Otherwise
                           Do
                              cvs_status = CVSStatus( !global.!current_filenames.1 )
                              If cvs_status = 'Needs Patch' | cvs_status = 'Needs Merge' Then
                                 Do
                                    answer = dw_messagebox( 'Refresh ?', dw_or( !REXXDW.!DW_MB_YESNOCANCEL, !REXXDW.!DW_MB_QUESTION ), 'The working copy of' quote( !global.!current_filenames.1 ) 'is out of date. Do you want it refreshed before viewing ?' )
                                    Select
                                       When answer = !REXXDW.!DW_MB_RETURN_YES Then Call UpdateCallback window
                                       When answer = !REXXDW.!DW_MB_RETURN_CANCEL Then Return 0
                                       Otherwise Nop
                                    End
                                 End
                           End
                     End
                  End
                  Call SysStemCopy 'save_current_filenames.', '!global.!current_filenames.'
                  Drop save_current_filenames.
               End
            End
         Otherwise Return 'N'
      End
   End
Else
   Do
      Do i = 1 To !global.!current_nonrepindexes.0
         selected = !global.!current_nonrepindexes.i
         workingfiles.i = Substr( !global.!nonrepfiles.?fullfilename.!!last_repository.selected, 5 )
      End
      workingfiles.0 = !global.!current_nonrepindexes.0
   End
/*
 * We now change to the current directory...
 */
here = Directory()
Call Directory !global.!current_dirname.!!last_repository
/*
 * If using win32 file associations, run the files directly using the w32funcs
 * function; w32executestem()
 * Those files that have no association should be run with the configured text or
 * binary pager.
 * For non-win32 platform, execute text or binary pager as appropriate for each file
 */
Select
   When !global.!os = 'WIN32' & !global.!!user.!!preference.!!windowsfileassociations = 1 Then
      Do
         Do i = 1 To workingfiles.0
            workingfiles.i = quote( Changestr( '/', workingfiles.i, !global.!ossep ) )
         End
         Call w32executestem 'workingfiles.', 'results.'
      End
   When !global.!os = 'WIN32' Then
      Do
         /*
          * Determine if each file is to be executed by file associations...
          */
         idx = 0
         Do i = 1 To workingfiles.0
            If UseFileAssociation( workingfiles.i ) Then
               Do
                  idx = idx + 1
                  wf.idx = quote( Changestr( '/', workingfiles.i, !global.!ossep ) )
                  idx.idx = i
               End
         End
         wf.0 = idx
         If wf.0 \= 0 Then Call w32executestem 'wf.', 'res.'
         /*
          * Apply the results of execution by file association back into the
          * results. stem
          */
         results. = 0
         Do i = 1 To wf.0
            idx = idx.i
            results.idx = res.i
         End
      End
   Otherwise
      Do
         results. = 0
         results.0 = workingfiles.0
      End
End
/*
 * We are now using either text pager or binary pager for those files that
 * have a corresponding results.x = 0
 */
/*
 * Check if files to be viewed are all binary or all text...
 */
binary_files = 0
text_files = 0
text_workingfiles = ''
binary_workingfiles = ''
Do i = 1 To workingfiles.0
   If results.i = 0 Then
      Do
         If IsBinaryFile( workingfiles.i ) Then
            Do
               binary_workingfiles = binary_workingfiles quote( workingfiles.i )
            End
         Else
            Do
               text_workingfiles = text_workingfiles quote( workingfiles.i )
            End
      End
End
/*
 * Do we have any files left to view ?
 */
If text_workingfiles \= '' Then
   Do
      If Strip( !global.!!user.!!textpager ) = '' Then
         Do
            Call dw_messagebox 'No viewer', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'You do not have a text viewer configured. Cannot view selected files.'
         End
      Else
         Do
            If !global.!!user.!!textpager_console Then sw = !global.!background_start_window
            Else sw = !global.!background_start_nowindow
            cmd = sw !global.!!user.!!textpager text_workingfiles !global.!background_end
            If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
            Address System cmd
         End
   End
If binary_workingfiles \= '' Then
   Do
      If Strip( !global.!!user.!!binarypager ) = '' Then
         Do
            Call dw_messagebox 'No viewer', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'You do not have a binary viewer configured. Cannot view selected files.'
         End
      Else
         Do
            If !global.!!user.!!binarypager_console Then sw = !global.!background_start_window
            Else sw = !global.!background_start_nowindow
            cmd = sw !global.!!user.!!binarypager binary_workingfiles !global.!background_end
            If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
            Address System cmd
         End
   End

Drop text_workingfiles binary_workingfiles workingfiles. results. idx. wf. res.
/*
 * Change back to the working directory...
 */
Call Directory here
/*
 * In case we checked out a different revision, refresh the files
 */
If files_different = 1 Then Call RefreshSelectedFiles
Return 0

EditLatestCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg window
workingfiles. = ''
workingfiles.0 = 0
this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
If !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository | !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then
   Do
      If !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then doing_repfiles = 1
      Else doing_repfiles = 0
      Select
         When !global.!!repository.!!type.!!last_repository = 'rcs' Then
            Do
               /* TODO - current_filenames. array processing */
               locker = GetRCSLocker( !global.!current_filenames )
               workingfile = RCSFileToFile( ?current.?current_filenames )
               Select
                  When locker = '' Then Call FileCommand 'checkout lock'
                  When Translate( locker ) = Translate( !global.!user ) Then Nop
                  Otherwise
                     Do
                        Call TkMessageBox '-message', 'You cannot edit a file locked by' locker,'-type','ok'
                        Return
                     End
               End
            End
         When !global.!!repository.!!type.!!last_repository = 'cvs' Then
            Do
               /*
                * Save the !global.!current_filenames. array and then for each element in
                * the array set ?current.?current_filenames. array to the single element
                */
               Call SysStemCopy '!global.!current_filenames.', 'save_current_filenames.'
               Do ei = 1 To save_current_filenames.0
                  !global.!current_filenames.1 = save_current_filenames.ei
                  !global.!current_filenames.0 = 1
                  workingfile = CVSFileToWorkingFile( !global.!current_filenames.1, !!last_repository )
                  ignorefile = 0
                  If Stream( workingfile, 'C', 'QUERY EXISTS' ) \= '' Then file_checked_out = 1
                  Else file_checked_out = 0
                  /*
                   * If the file is a binary file, or we are in a reserved checkout directory
                   * if not already checked out (and it can be checked out) check it out first
                   */
                  checkout_first = 0
                  If !global.!dir.!reservedcheckout.this_dir = 'Y' | !global.!reservedcheckout = 'Y' Then checkout_first = 1
                  idx = !global.!current_indexes.ei
                  If doing_repfiles Then ft = !global.!repfiles.?filetype.!!last_repository.idx
                  Else ft = !global.!allfiles.?filetype.!!last_repository.idx
                  If Substr( ft, 5, 6 ) = 'binary' Then checkout_first = 1
                  If checkout_first Then
                     Do
                        /*
                         * Check the lock status...
                         */
                        If doing_repfiles Then ffn = !global.!repfiles.?fullfilename.!!last_repository.idx
                        Else ffn = !global.!allfiles.?fullfilename.!!last_repository.idx
                        locker = GetCVSLocker( Substr( ffn, 5 ) )
                        Select
                           When locker = '' Then Call CheckoutCallback
                           When Translate( locker ) = Translate( !global.!user ) Then Nop
                           Otherwise
                              Do
                                 Call dw_messagebox 'File locked.', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'You cannot edit the file. It is locked by' locker'.'
                                 ignorefile = 1 /* don't edit this one */
                              End
                        End
                     End
                  Else
                     Do
                        Select
                           When !global.!offline.!!last_repository Then Nop
                           When file_checked_out = 0 Then
                              Do
                                 Call GetLatestCallback
                              End
                           Otherwise
                              Do
                                 cvs_status = CVSStatus( !global.!current_filenames.1 )
                                 If cvs_status = 'Needs Patch' | cvs_status = 'Needs Merge' Then
                                    Do
                                       answer = dw_messagebox( 'Refresh ?', dw_or( !REXXDW.!DW_MB_YESNOCANCEL, !REXXDW.!DW_MB_QUESTION ), 'The working copy of' quote( !global.!current_filenames.1 ) 'is out of date. Do you want it refreshed before editing ?' )
                                       Select
                                          When answer = !REXXDW.!DW_MB_RETURN_YES Then Call UpdateCallback window
                                          When answer = !REXXDW.!DW_MB_RETURN_CANCEL Then Return 0
                                          Otherwise Nop
                                       End
                                    End
                              End
                        End
                     End
                  /*
                   * Add the current file into the list of working files...
                   */
                  If ignorefile = 0 Then
                     Do
                        wfi = workingfiles.0 + 1
                        workingfiles.wfi = workingfile
                        workingfiles.0 = wfi
                     End
               End
               Call SysStemCopy 'save_current_filenames.', '!global.!current_filenames.'
               Drop save_current_filenames.
            End
         Otherwise Return 0
      End
   End
Else
   Do
      Do i = 1 To !global.!current_nonrepindexes.0
         selected = !global.!current_nonrepindexes.i
         workingfiles.i = Substr( !global.!nonrepfiles.?fullfilename.!!last_repository.selected, 5 )
      End
      workingfiles.0 = !global.!current_nonrepindexes.0
   End
/*
 * We now change to the current directory...
 */
here = Directory()
Call Directory !global.!current_dirname.!!last_repository
/*
 * If using win32 file associations, run the files directly using the w32funcs
 * function; w32executestem()
 * Those files that have no association should be run with the configured text or
 * binary editor.
 * For non-win32 platform, execute text or binary editor as appropriate for each file
 */
Select
   When !global.!os = 'WIN32' & !global.!!user.!!preference.!!windowsfileassociations = 1 Then
      Do
         Do i = 1 To workingfiles.0
            workingfiles.i =  quote( Changestr( '/', workingfiles.i, !global.!ossep ) )
         End
         Call w32executestem 'workingfiles.', 'results.'
      End
   When !global.!os = 'WIN32' Then
      Do
         /*
          * Determine if each file is to be executed by file associations...
          */
         idx = 0
         Do i = 1 To workingfiles.0
            If UseFileAssociation( workingfiles.i ) Then
               Do
                  idx = idx + 1
                  wf.idx = quote( Changestr( '/', workingfiles.i, !global.!ossep ) )
                  idx.idx = i
               End
         End
         wf.0 = idx
         If wf.0 \= 0 Then Call w32executestem 'wf.', 'res.'
         /*
          * Apply the results of execution by file association back into the
          * results. stem
          */
         results. = 0
         Do i = 1 To wf.0
            idx = idx.i
            results.idx = res.i
         End
      End
   Otherwise
      Do
         results. = 0
         results.0 = workingfiles.0
      End
End
/*
 * We are now using either text editor or binary editor for those files that
 * have a corresponding results.x = 0
 */
/*
 * Check if files to be edited are all binary or all text...
 */
binary_files = 0
text_files = 0
text_workingfiles = ''
binary_workingfiles = ''
Do i = 1 To workingfiles.0
   If results.i = 0 Then
      Do
         If IsBinaryFile( workingfiles.i ) Then
            Do
               binary_workingfiles = binary_workingfiles quote( workingfiles.i )
            End
         Else
            Do
               text_workingfiles = text_workingfiles quote( workingfiles.i )
            End
      End
End
/*
 * Do we have any files left to edit ?
 */
If text_workingfiles \= '' Then
   Do
      If Strip( !global.!!user.!!texteditor ) = '' Then
         Do
            Call dw_messagebox 'No editor', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'You do not have a text editor configured. Cannot edit selected files.'
         End
      Else
         Do
            v_editor = !global.!!user.!!texteditor
            v_editor = Changestr( "%L%", v_editor, '1' )
            v_editor = Changestr( "%C%", v_editor, '1' )
            v_editor = Changestr( "%F%", v_editor, text_workingfiles )
            If !global.!!user.!!texteditor_console Then sw = !global.!background_start_window
            Else sw = !global.!background_start_nowindow
            cmd = sw v_editor !global.!background_end
            If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
            Address System cmd
         End
   End
If binary_workingfiles \= '' Then
   Do
      If Strip( !global.!!user.!!binaryeditor ) = '' Then
         Do
            Call dw_messagebox 'No editor', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'You do not have a binary editor configured. Cannot edit selected files.'
         End
      Else
         Do
            If !global.!!user.!!binaryeditor_console Then sw = !global.!background_start_window
            Else sw = !global.!background_start_nowindow
            cmd = sw !global.!!user.!!binaryeditor binary_workingfiles !global.!background_end
            If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
            Address System cmd
         End
   End

Drop text_workingfiles binary_workingfiles workingfiles. results. idx. wf. res.
/*
 * Change back to the working directory...
 */
Call Directory here
Return 0

/*
 * Called when double-clicking a file or prressing Enter in file list
 */
FileDoubleClickCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., index
Select
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then
      Do
         /*
          * Set the "selected" rep indexes to the index passed in as a parameter
          */
         !global.!current_repindexes.0 = 1
         !global.!current_repindexes.1 = index
      End
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?nonrepfiles.!!last_repository Then
      Do
         /*
          * Set the "selected" norep indexes to the index passed in as a parameter
          */
         !global.!current_nonrepindexes.0 = 1
         !global.!current_nonrepindexes.1 = index
      End
   Otherwise Nop
End
Call ViewLatestCallback
Return 0

/*
 * Called from 'Get Latest' file menu
 * If we have a working file, check if it is locally modified. Warn if so.
 * If no working file, just get latest.
 * Refresh directory icons
 * Valid for multiple files.
 */
GetLatestCallback: Procedure Expose !REXXDW. !global. !!last_repository
checkout = 'N'
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         /* TODO */
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         Do i = 1 To !global.!current_filenames.0
            /*
             * If the file being obtained has been locally modified, ask the
             * user if they want to blow away any changes they have made
             * Not applicable if we are saving the file to a new filename
             * Always use "checkout -p" because this ignores CVS stickiness!
             */
            file = !global.!current_filenames.i
            cvs_status = CVSStatus( file )
            If cvs_status \= '' & cvs_status \= 'Up-to-date' & cvs_status \= 'Needs Checkout' Then
               Do
                  blowaway = dw_messagebox( 'Get Latest?', dw_or( !REXXDW.!DW_MB_YESNOCANCEL, !REXXDW.!DW_MB_QUESTION ), quote( file ) 'has been modified locally. Do you want to lose these changes and retrieve the latest copy from the repository ?' )
                  Select
                     When blowaway = !REXXDW.!DW_MB_RETURN_YES Then
                        Do
                           Call SysFileDelete file
                           cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout' quote( file )
                           If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                           Address Command cmd With Output FIFO '' Error FIFO ''
                           Call WriteStackToLogWindow 'default', 'Get Latest of' quote( file )
                           checkout = 'Y'
                        End
                     When blowaway = !REXXDW.!DW_MB_RETURN_NO Then
                        Do
                           Call WriteStackToLogWindow 'default', 'Get Latest of' quote( file ) 'abandoned'
                        End
                     Otherwise Leave /* cancel */
                  End
               End
            Else
               Do
                  Call RunOsCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout' quote( file )
                  Call WriteOutputStemsToLogWindow 'default', 'Get Latest of' quote( file )
                  checkout = 'Y'
               End
         End
      End
   Otherwise Nop
End
If checkout = 'Y' Then
   Do
      Call RefreshDirectoryIcons
      Call RefreshSelectedFiles
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
   End
Return 0

/*
 * This function is called from the filemenu; update command
 * Valid for multiple selections
 * This command only valid for CVS
 */
UpdateCallback: Procedure Expose !REXXDW. !global. !!last_repository
current_filenames = ''
Do i = 1 To !global.!current_filenames.0
   current_filenames = current_filenames quote( !global.!current_filenames.i )
End
Call RunOSCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'update -d' current_filenames
If result \= 0 Then
   Do
      Call NotifyNoConnection
      Call RefreshFilesCallback
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
   End
Else
   Do
      Call WriteCVSOutputStemsToLogWindow 'update',, 'Update for' current_filenames
      If !global.!!repository.!!type.!!last_repository = 'cvs' Then Call CheckForCVSConflicts
      Call RefreshSelectedFiles
   End
Drop oserr. osout. current_filenames
Return 0

CheckOutLatestCallback: Procedure Expose !REXXDW. !global. !!last_repository
Return 0

GetPriorCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Dialog box to ask user for file revision to work on
 * Only one file allowed to be selected for this option
 *
 * +---------------------------------------------------+
 * | Text                                              |
 * |                                                   |
 * +---------------------------------------------------+
 * |  File Revision: _____Latest___v                   |
 * |  To: __________________________  X                |
 * +---------------------------------------------------+
 */
this_file = !global.!current_filenames.1
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Select File Revision for:' this_file, !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon

gf = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, gf, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

t2 = dw_text_new( 'Select the revision of the file to Get.', 0 )
Call dw_box_pack_start gf, t2, 250, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

boxtopack = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start gf, boxtopack, 250, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Parse Value CreateFileRevisionCombobox( boxtopack, 'false', 'Latest', this_file, '', 'rev tag', 'date' ) With revcbox .
/*
 * Now the destination box
 */
db = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start gf, db, 250, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

thisdir = GetFullWorkingDirectory( !global.!current_dirname.!!last_repository )

oldfile = !global.!!repository.!!working_dir.!!last_repository || this_file
oldfile = Changestr( '/', oldfile, !global.!ossep )

thisfile = Substr( oldfile, Length( thisdir ) + 2 )
t1 = dw_entryfield_new( oldfile, 0 )
Call dw_box_pack_start db, t1, 250-20, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
abutton = dw_bitmapbutton_new_from_file( 'Select Destination', 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start db, abutton, 20, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericFileSelector', t1, 'Get file to:', thisdir, 0, !REXXDW.!DW_FILE_SAVE
/*
 * Create a dialog for the window
 */
dialog_wait = dw_dialog_new( )
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start gf, b1, 250, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GetFileRevisionsAndDestinationApplyCallback', win, dialog_wait, revcbox, t1

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start gf, b2, 250, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * We now have everything setup, we can connect the destroying of the top-level window
 * to the 'close' callback
*/
Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * Display the window
 */
width = 400
height = 160
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
/*
 * Wait for the dialog to come back
 */
Parse Value dw_dialog_wait( dialog_wait ) With cancel rev newfile
if trace() = 'F' Then say 'got rev' cancel rev ':' newfile

/*
 * If the user is asking to overwrite an existing file in their working directory,
 * confirm this.
 */
If newfile = oldfile Then files_different = 0
Else files_different = 1
If files_different = 0 & Stream( newfile, 'C', 'QUERY EXISTS' ) \= '' Then
   Do
      proceed = dw_messagebox( 'Overwrite File?', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), 'Are you sure you want to overwrite the working file?' )
      If proceed = !REXXDW.!DW_MB_RETURN_YES Then newfile = oldfile
      Else Return 0
   End

If rev \= '' Then
   Do
      Select
         When !global.!!repository.!!type.!!last_repository = 'rcs' Then
            Do
               /* TODO */
            End
         When !global.!!repository.!!type.!!last_repository = 'cvs' Then
            Do
               If Left( rev, 5 ) = 'Date:' Then rev = Date( 'N', Substr( rev, 6 ) , 'S' )
               If rev \= '' & rev \= 'Latest' Then
                  Do
                    If Words( rev ) \= 1 Then relflag = '-D'Translate( rev, '-', ' ' )
                    Else relflag = '-r'rev
                  End
               Else relflag = ''
               If Stream( thisdir, 'C', 'QUERY EXISTS' ) = '' Then
                  Do
                     Call dw_messagebox 'Directory non-existent', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'The directory to which you are saving the file does not exist. Cancelling action.'
                     -- Return 0
                  End
               Else
                  Do
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout -p' relflag quote( this_file )
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address Command cmd With Output Replace Stream newfile Error FIFO ''
                     Call Stream newfile, 'C', 'CLOSE'
                     Call WriteStackToLogWindow 'default', 'Get Revision for' quote( this_file ) 'to' newfile
                  End
            End
         Otherwise Nop
      End
   End

If files_different = 0 Then Call RefreshSelectedFiles
Return 0

CheckOutCallback: Procedure Expose !REXXDW. !global. !!last_repository
checkout = 'N'
this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
If !global.!dir.!reservedcheckout.this_dir = 'Y' | !global.!reservedcheckout = 'Y' Then is_reserved_dir = 1
Else is_reserved_dir = 0
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         /* TODO */
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         /*
          * It is possible the list of files contains files we cannot checkout so
          * do a check for each file
          */
         Do i = 1 To !global.!current_filenames.0
            selected = !global.!current_indexes.i
            file = !global.!current_filenames.i
            If CanFileBeCheckedOutForLock( selected, is_reserved_dir ) Then
               Do
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout' quote( file )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address Command cmd With Output FIFO '' Error FIFO ''
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'admin -l' quote( file )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address Command cmd With Output FIFO '' Error FIFO ''
                  checkout = 'Y'
               End
            Else
               Do
                  Queue 'Unable to checkout' quote( file )
               End
            Call WriteStackToLogWindow 'default', 'Check out and lock for' quote( file )
         End
      End
   Otherwise Nop
End
If checkout = 'Y' Then
   Do
      Call RefreshDirectoryIcons
      Call RefreshSelectedFiles
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
   End
Return 0

/*
 * This function is called from the filemenu; undocheckout
 * Valid for multiple selections
 */
UndoCheckOutCallback: Procedure Expose !REXXDW. !global. !!last_repository
undocheckout = 'N'
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         /* TODO */
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         Do i = 1 To !global.!current_filenames.0
            file = !global.!current_filenames.i
            cvs_status = CVSStatus( file )
            If cvs_status = 'Locally Modified' | cvs_status = 'Needs Merge' | cvs_status = 'File had conflicts on merge' Then
               Do
                  If dw_messagebox( 'Undo Checkout?', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), quote( file ) 'has been modified locally. Do you want to lose these changes ?' ) = !REXXDW.!DW_MB_RETURN_YES Then
                     Do
                        /*
                         * Unlock the file...
                         */
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'admin -u' quote( file )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output FIFO '' Error FIFO ''
                        /*
                         * Delete the file...
                         */
                        Call SysFileDelete file
                        /*
                         * Get the latest...
                         */
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout' quote( file )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output FIFO '' Error FIFO ''
                        Call WriteStackToLogWindow 'default', 'Undo Check Out for' quote( file )
                        undocheckout = 'Y'
                     End
                  Else
                     Do
                        Call WriteStackToLogWindow 'default', 'Undo Check Out for' quote( file ) 'abandoned'
                     End
               End
            Else
               Do
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'admin -u' quote( file )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address System cmd With Output FIFO '' Error FIFO ''
                  Call WriteStackToLogWindow 'default', 'Undo Check Out for' quote( file )
                  undocheckout = 'Y'
               End
         End
      End
   Otherwise Nop
End
If undocheckout = 'Y' Then
   Do
      Call RefreshDirectoryIcons
      Call RefreshSelectedFiles
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
   End
Return 0

/*
 * This function is called from the filemenu; checkin
 * Valid for multiple selections
 */
CheckinCallback: Procedure Expose !REXXDW. !global. !!last_repository
same = 0
If !global.!current_filenames.0 > 1 Then dialog_type = 'dir'
Else dialog_type = 'file'
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do cii = 1 To !global.!current_filenames.0
         file = RCSFileToFile( !global.!current_filenames.cii )
         If Stream( file, 'C', 'QUERY EXISTS' ) = '' Then AbortText( 'Unknown file:' quote( file ) )
         /*
          * Check if the working file is not writeable
          * If so, popup a dialog box asking if the
          * user want to checkin anyway
          */
         If Stream( file, 'C', 'WRITABLE' ) \= 1 Then
            Do
               If TkMessageBox( '-message', 'The file you are checking in:' quote( file ) 'is not writeable. This may indicate that you are attempting to check in the wrong file. Are you sure you want to do this ?','-title', 'Checkin?', '-type', 'yesno', '-icon', 'warning') = 'yes' Then checkin = 'Y'
               Else checkin = 'N'
            End
         Else checkin = 'Y'
         /*
          * Check if the working file is later than the repository
          * file date
          * If so, popup a dialog box asking if the
          * user want to checkin anyway
          * etc....
          */
         If checkin = 'Y' Then
            Do
               /*
                * Get the checkin details from a new dialog box
                * and push the log text onto the stack
                */
               Parse Value GetCheckinDetails( 'Y', dialog_type, file, 'Apply same text for all files', 'Checking in:' ) With cancel .
               If cancel = 'close' Then
                  Do
                     checkin = 'N'
                  End
               Else
                  Do
                     Call RunOsCommand 'LIFO> ci -u' quote( file ) quote( current_filenames.cii )
                     Call WriteOutputStemsToLogWindow 'default', 'Checkin for' quote( current_filenames.cii )
                  End
            End
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         tmpfile = GenerateTempFile() -- we delete it ourselves
         Do cii = 1 To !global.!current_filenames.0
            file = CVSFileToWorkingFile( !global.!current_filenames.cii, !!last_repository )
            If Stream( file, 'C', 'QUERY EXISTS' ) = '' Then AbortText( 'Unknown file:' file )
            /*
             * Check if the working file is not writeable
             * If so, popup a dialog box asking if the
             * user want to checkin anyway
             */
            If Stream( file, 'C', 'WRITABLE' ) \= 1 Then
               Do
                  If dw_messagebox( 'Check in?', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), 'The file you are checking in:' quote( file ) 'is not writeable. This may indicate that you are attempting to check in the wrong file. Are you sure you want to do this ?' ) = !REXXDW.!DW_MB_RETURN_YES Then checkin = 'Y'
                  Else checkin = 'N'
               End
            Else checkin = 'Y'
            /*
             * Ask for checkin comments for each fle unless we say we want the same comments for each file...
             */
            If checkin = 'Y' Then
               Do
                  /*
                   * if we haven't said we want the same text for all files
                   * checked in, Get the checkin details from a new dialog box
                   * and push the log text onto the stack, and write a
                   * temporary file
                   */
                  If \same Then
                     Do
                        Parse Value Strip( GetCheckinDetails( 'Y', dialog_type, file, 'Apply same text for all files', 'Checking in:' ) ) With cancel same .
                        If cancel = 'close' Then
                           Do
                              checkin = 'N'
                           End
                        Else
                           Do
                              Call Stream tmpfile, 'C', 'OPEN WRITE REPLACE'
                              Do Queued()
                                 Parse Pull line
                                 Call Lineout tmpfile,line
                              End
                              Call Stream tmpfile, 'C', 'CLOSE'
                           End
                     End
                  If checkin = 'Y' Then
                     Do
                        -- checkin could have been set to N just above
                        Call SetCursorWait
                        Call RunOsCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'commit -F 'tmpfile quote( !global.!current_filenames.cii )
                        Call WriteCVSOutputStemsToLogWindow 'checkin',, 'Checkin for' quote( !global.!current_filenames.cii )
                        Call CheckForCVSConflicts
                        Call SetCursorNoWait
                        Drop oserr. osout.
                     End
               End
         End
         Call SysFileDelete tmpfile
      End
   Otherwise Nop
End
If checkin = 'Y' Then
   Do
      Call RefreshDirectoryIcons
      Call RefreshSelectedFiles
      Call AdjustFileToolbarList
      Call AdjustFileToolbarButtons
   End
Return 0

/*
 * Get the reminder details from a new dialog box
 * and write it to the reminder file
 */
SetReminderCallback: Procedure Expose !REXXDW. !global. !!last_repository
reminder_file = CVSFileToReminderFile( !global.!current_filenames.1 )
/*
 * If we have a reminder file already, read the text into !global.!reminder_text
 */
refresh = 'N'
If Stream( reminder_file, 'C', 'QUERY EXISTS' ) \= '' Then
   Do
     !global.!remindertext = ''
      Call Stream reminder_file, 'C', 'OPEN READ'
      Do While Lines( reminder_file ) > 0
         !global.!remindertext = !global.!remindertext||Linein( reminder_file )||!global.!crlf
      End
      Call Stream reminder_file, 'C', 'CLOSE'
      extra_text = ''
   End
Else
   Do
      If !global.!remindertext = '' Then extra_text = ''
      Else extra_text = '[*** Copied from previous reminder ***]'!global.!crlf
   End

Parse Value Strip( GetCheckinDetails( 'Y', 'remind', !global.!current_filenames.1, extra_text, 'Setting reminder for:' ) ) With cancel same .

If cancel = 'close' Then
   Do
      Call WriteMessageHeaderToLogWindow 'Reminder set for' quote( !global.!current_filenames.1 ) 'cancelled.'
   End
Else
   Do
      Call Stream reminder_file, 'C', 'OPEN WRITE REPLACE'
      Call Charout reminder_file, !global.!remindertext
      Call Stream reminder_file, 'C', 'CLOSE'
      Call WriteMessageHeaderToLogWindow 'Reminder set for' quote( !global.!current_filenames.1 )
      refresh = 'Y'
   End

If refresh = 'Y' Then Call RefreshSelectedFiles
Return 0

/*
 * This function is called from the filemenu; unremind command
 * Invalid for multiple selections
 * This command only valid for CVS
 */
DeleteReminderCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Simpy delete the reminder file if there is one...
 */
refresh = 'N'
reminder_file = CVSFileToReminderFile( !global.!current_filenames.1 )
If Stream( reminder_file, 'C', 'QUERY EXISTS' ) = '' Then
   Do
      Call WriteStackToLogWindow 'default', 'No reminder was set for' quote( !global.!current_filenames.1 )
   End
Else
   Do
      If dw_messagebox( 'Delete Reminder?', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), 'Are you sure you want to delete the reminder for' quote( !global.!current_filenames.1 )'!' ) = !REXXDW.!DW_MB_RETURN_YES Then
         Do
            Call SysFileDelete reminder_file
            Call WriteMessageHeaderToLogWindow 'Reminder deleted for' quote( !global.!current_filenames.1 )
            refresh = 'Y'
         End
   End
If refresh = 'Y' Then Call RefreshSelectedFiles
Return 0

/*
 * This function is called from the filemenu; showdifferences command
 * Not valid for multiple selections. Therefore current_filenames.0 MUST be 1
 */
ShowDifferencesCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Dialog box to determine how and what to show differences for...
 *
 * +---------------------------------------------------+
 * | Text                                              |
 * |                                                   |
 * +---------------------------------------------------+
 * |       Compare           X Ignore whitespace       |
 * |  Revision _Working__v   x Strict                  |
 * |        in _Current__v                             |
 * |        and                                        |
 * |  Revision _Latest___v   x Output in log window    |
 * |        in _Current__v   X Visual output           |
 * |                                                   |
 * +---------------------------------------------------+
 */
text_height = 50
tag_types = 'tag rev'
Call SetCursorWait
/*
 * Check if working file exists
 */
workingfile = CVSFileToWorkingFile( !global.!current_filenames.1, !!last_repository )
If Stream( workingfile, 'C', 'QUERY EXISTS' ) \= '' Then working_file_exists = 1
Else working_file_exists = 0
/*
 * Create the dialog box...
 */
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Show differences', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
loc = dw_mle_import( t1, 'Select the revisions of the file to compare. Comparisons can be done ignoring whitespace or strictly.  Results can be displayed in the Messages Pane or in a graphical display tool.', -1 )
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_mle_set_cursor t1, loc
Call dw_window_disable t1
/*
 * Add a horiz box with comboboxes on left, radio buttons on right
 */
data_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, data_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a vertical box for the comboboxes and add the comboboxes
 */
cb_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start data_box, cb_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * comboboxes for later revision
 */
cb1_box = dw_groupbox_new( !REXXDW.!DW_VERT, 'Compare (later revision)', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start cb_box, cb1_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
cb1_h_box1 = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb1_box, cb1_h_box1, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
tmp = dw_text_new( "Revision ", 0 )
Call dw_box_pack_start cb1_h_box1, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
boxtopack = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb1_h_box1, boxtopack, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
If working_file_exists Then
   Parse Value CreateFileRevisionCombobox( boxtopack, 'false', 'Working', !global.!current_filenames.1, 'Latest', tag_types, 'date' ) With rev1cbox realtagstart
Else
   Parse Value CreateFileRevisionCombobox( boxtopack, 'false', 'Latest', !global.!current_filenames.1, '', tag_types, 'date' ) With rev1cbox realtagstart
cb1_h_box2 = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb1_box, cb1_h_box2, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
tmp = dw_text_new( "in ", 0 )
Call dw_box_pack_start cb1_h_box2, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
rep1cbox = CreateRepositoryCombobox( cb1_box, 'false', 'Current Repository', rev1cbox, realtagstart, !global.!current_filenames.1, tag_types )
Call dw_box_pack_start cb1_h_box2, rep1cbox, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2

cb2_box = dw_groupbox_new( !REXXDW.!DW_VERT, 'with (earlier revision)', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start cb_box, cb2_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
cb2_h_box1 = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb2_box, cb2_h_box1, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
tmp = dw_text_new( "Revision ", 0 )
Call dw_box_pack_start cb2_h_box1, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
boxtopack = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb2_h_box1, boxtopack, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
If working_file_exists Then
   Parse Value CreateFileRevisionCombobox( boxtopack, 'false', 'Latest',  !global.!current_filenames.1, 'Working', tag_types, 'date' ) With rev2cbox realtagstart
Else
   Parse Value CreateFileRevisionCombobox( boxtopack, 'false', 'Latest',  !global.!current_filenames.1, '', tag_types, 'date' ) With rev2cbox realtagstart
cb2_h_box2 = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start cb2_box, cb2_h_box2, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
tmp = dw_text_new( "in ", 0 )
Call dw_box_pack_start cb2_h_box2, tmp, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
rep2cbox = CreateRepositoryCombobox( cb2_box, 'false', 'Current Repository', rev2cbox, realtagstart, !global.!current_filenames.1, tag_types )
Call dw_box_pack_start cb2_h_box2, rep2cbox, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2

/*
 * Add a vertical box for the two groups of radio buttons and add the radio buttons
 */
group_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start data_box, group_box, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

g1 = dw_groupbox_new( !REXXDW.!DW_VERT, 'Comparison', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start group_box, g1, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
compare_rb1 = dw_radiobutton_new( 'Ignore whitespace', 111 )
Call dw_box_pack_start g1, compare_rb1, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
compare_rb2 = dw_radiobutton_new( 'Strict', 111 )
Call dw_box_pack_start g1, compare_rb2, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
Call dw_radiobutton_set compare_rb1, !REXXDW.!DW_CHECKED

g2 = dw_groupbox_new( !REXXDW.!DW_VERT, 'Output to', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start group_box, g2, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
output_rb1 = dw_radiobutton_new( 'Log window', 222 )
Call dw_box_pack_start g2, output_rb1, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
output_rb2 = dw_radiobutton_new( 'Visual', 222 )
Call dw_box_pack_start g2, output_rb2, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
Call dw_radiobutton_set output_rb2, !REXXDW.!DW_CHECKED
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
Call dw_window_set_gravity win, !REXXDW.!DW_GRAV_CENTRE, !REXXDW.!DW_GRAV_CENTRE
Call dw_window_set_pos_size win, 0, 0, 0, 0
Call dw_window_show win

Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'ShowDifferencesApplyCallback', win, dialog_wait, rev1cbox, rev2cbox, rep1cbox, rep2cbox, compare_rb1, compare_rb2, output_rb1, output_rb2
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
/*   Parse Value dw_dialog_wait( dialog_wait ) With cancel ':' rev1 ':' rev2 ':' rep1 ':'  rep2 ':' rep1_idx ':' rep2_idx ':' difftype ':' outputtype*/
if trace() = 'F' Then    say cancel rev1 rev2 rep1 rep2 rep1_idx rep2_idx difftype outputtype
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
/*
 * If we selected 'Cancel' return now
If cancel = 'close' Then Return 0
 */
Return 0

ShowAnnotationsCallback: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * +-----------------------------------------------------------------+-+
 * + Rel | User | Date | Line| contents                               | |
 * +------------------------------------------------------------------+-+
 * +     |      |      |     |                                        |^|
 * +     |      |      |     |                                        |v|
 * +------------------------------------------------------------------+ +
 * + Line:___________        |<                                      >| |
 * +-----------------------------------------------------------------+ +
 */
/*
 * Get the revision of the file to show
 */
Call SetCursorWait
Call DisplayStatus 'Preparing annotations...'

rev = GetFileRevisions( !global.!current_filenames.1, 'Show Annotations', 'rev tag', 'date', 'Latest' )
If rev = '' Then Return 0
/*
 * Read the file into a stem, parse all lines of the stem to determine the
 * width of all of the text boxes...
 */
Select
   When Words( rev ) = 3 Then rev_flag = '-D'rev
   When rev = 'Latest' Then rev_flag = ''
   Otherwise rev_flag = '-r'rev
End
If !REXXDW.!GTK_MAJOR_VERSION > 1 Then
   Do
      cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q annotate' rev_flag quote( !global.!current_filenames.1 ) '| iconv -f ISO-8859-1 -t UTF-8'
      If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
      Address System cmd With Output Stem file. Error Stem err.
   End
Else
   Do
      cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q annotate' rev_flag quote( !global.!current_filenames.1 )
      If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
      Address Command cmd With Output Stem file. Error Stem err.
   End
If file.0 = 0 Then
   Do
      Return 0
   End

/*
 * If there is a valid syntax for the file's extension, use this
 */
pos = Lastpos( '.', !global.!current_filenames.1 )
If pos \= 0 & !global.!have_highlight Then
   Do
      syntax = FindSyntax( Substr( !global.!current_filenames.1, pos+1 ) )
      If syntax = '' Then syntax_flags = ''
      Else syntax_flags = '-S' syntax
   End
Else syntax_flags = ''

/*
 * If we can use syntax highlighting, then do so
 */
If syntax_flags \= '' Then
   Do
      tmpfile = GenerateTempFile( '??shan??' )
      Call Stream tmpfile, 'C', 'OPEN WRITE REPLACE'
      !global.!showannotations.!have_syntax = 1
   End
Else !global.!showannotations.!have_syntax = 0

max_rel = 0
max_user = 0
max_date = 9
max_line = 0
max_numb = 4 /* length of header label */
/*
 * Do some calculation of maximum widths of the columns and copy the
 * local stem into a global one so that the callbacks can see the data
 * Date is fixed length, and the linenumber will be the length of file.0
 */
If Length( file.0 ) > max_numb Then max_numb = Length( file.0 )
Do i = 1 To file.0
   Parse Var file.i rel '(' user date '): ' line
   !global.!showannotations.1.i = Strip( rel )
   !global.!showannotations.2.i = Strip( user )
   !global.!showannotations.3.i = Strip( date )
   !global.!showannotations.4.i = Right( i, max_numb )
   If syntax_flags = '' Then !global.!showannotations.5.i = line
   Else Call Lineout tmpfile, line
/*   Else Call Lineout tmpfile,dw_convert('ISO-8859-1', 'UTF-8', line )*/
/*   Else Call Lineout tmpfile,Changestr( '	', line, '    ')*/
/* WARNING **: Invalid UTF8 string passed to pango_layout_set_text() */
   If Length( !global.!showannotations.1.i ) > max_rel Then max_rel = Length( !global.!showannotations.1.i )
   If Length( !global.!showannotations.2.i ) > max_user Then max_user = Length( !global.!showannotations.2.i )
   If Length( line ) > max_line Then max_line = Length( line )
End
!global.!showannotations.1.0 = file.0
!global.!showannotations.2.0 = file.0
!global.!showannotations.3.0 = file.0
!global.!showannotations.4.0 = file.0
!global.!showannotations.5.0 = file.0
!global.!showannotations.!max_width.1 = max_rel
!global.!showannotations.!max_width.2 = max_user
!global.!showannotations.!max_width.3 = max_date
!global.!showannotations.!max_width.4 = max_numb
!global.!showannotations.!max_width.5 = max_line
Drop file.
/*
 * Close the temporary file if we are processing syntax
 */
If syntax_flags \= '' Then
   Do
      Call Stream tmpfile, 'C', 'CLOSE'
      Call DisplayStatus 'Syntax highlighting...'
      Parse Value GetHighlightedLines( tmpfile, syntax_flags, '!GLOBAL.!SHOWANNOTATIONS.5.' ) With !global.!showannotations.!default_fore !global.!showannotations.!default_back
      Call SysFileDelete tmpfile
      Drop high.
   End
Else
   Do
      !global.!showannotations.!default_fore = !REXXDW.!DW_CLR_BLACK
      !global.!showannotations.!default_back = !REXXDW.!DW_CLR_WHITE
   End

title.1 = 'Rel'
title.2 = 'User'
title.3 = 'Date'
title.4 = 'Line'
title.5 = 'File Contents'

/*
 * Build our window and all the boxes
 */
scrollbarwidth = 14
scrollbarheight = 14
scrollbarspace = !global.!widgetheight - scrollbarheight
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Annotations for' !global.!current_filenames.1'('rev')', !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

depth = dw_color_depth_get()
/*
 * Work out font sizing
 */
junk = dw_render_new( 0 )
Call dw_window_set_font junk, !global.!fixedfont
Parse Value dw_font_text_extents_get( junk, , "g(" ) With font_width font_height
Call dw_window_destroy junk
font_width = ( font_width / 2 ) % 1 -- ensure font is whole number
!global.!showannotations.!font_width = font_width
!global.!showannotations.!font_height = font_height
!global.!showannotations.!current_row = 0
!global.!showannotations.!current_col = 0

spinbutton_title_width = 70
spinbutton_width = (max_rel*font_width) + (max_user*font_width) + (max_date*font_width) + (max_numb*font_width) - 40

tophbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, tophbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * A vertical box for the fixed width render boxes and the line number spinbutton
 */
fixedbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start tophbox, fixedbox, 0, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * A horizontal box for the fixed width columns
 */
hbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start fixedbox, hbox, 0, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Columns for fixed columns...
 */
Do i = 1 To 4
   /*
    * Pack some empty space before the column
    */
   Call dw_box_pack_start hbox, 0, 5, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
   box = dw_box_new( !REXXDW.!DW_VERT )
   !global.!showannotations.!pixmap_width.i = !global.!showannotations.!max_width.i*font_width
   Call dw_box_pack_start hbox, box, !global.!showannotations.!pixmap_width.i, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
   header = dw_text_new( title.i, i )
   Call dw_box_pack_start box, header, !global.!showannotations.!pixmap_width.i, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   Call dw_window_set_style header, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
   !global.!showannotations.!pixmapbox.i = dw_render_new( i+10 )
   Call dw_window_set_font !global.!showannotations.!pixmapbox.i, !global.!fixedfont
   Call dw_box_pack_start box, !global.!showannotations.!pixmapbox.i, !global.!showannotations.!pixmap_width.i, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
End
/*
 * Spinbutton
 */
spinbuttonbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start fixedbox, spinbuttonbox, spinbutton_width+spinbutton_title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
linebutton = dw_button_new( 'Goto Line:', 0 )
Call dw_box_pack_start spinbuttonbox, linebutton, spinbutton_title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
sb = dw_spinbutton_new( '', 0 )
Call dw_box_pack_start spinbuttonbox, sb, spinbutton_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_spinbutton_set_limits sb, !global.!showannotations.5.0, 1
Call dw_spinbutton_set_pos sb, 1
/*
 * Build our boxes for the file contents
 */
hbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start tophbox, hbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

vertbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start hbox, vertbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

header = dw_text_new( title.5, 0 )
Call dw_box_pack_start vertbox, header, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style header, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )

!global.!showannotations.!pixmap_width.5 = !global.!showannotations.!max_width.5*font_width

/* create render box for filedetails pixmap */
!global.!showannotations.!pixmapbox.5 = dw_render_new( 0 )
Call dw_box_pack_start vertbox, !global.!showannotations.!pixmapbox.5, font_width, font_height, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_window_set_font !global.!showannotations.!pixmapbox.5, !global.!fixedfont

/* create horizonal scrollbar */
!global.!showannotations.!hbar = dw_scrollbar_new( !REXXDW.!DW_HORZ, 0 )
Call dw_box_pack_start vertbox, !global.!showannotations.!hbar, 0, scrollbarheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_box_pack_start vertbox, 0, 0, scrollbarspace, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

vscrollbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start hbox, vscrollbox, scrollbarwidth, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/* Pack an area of empty space above the scrollbar */
Call dw_box_pack_start vscrollbox, 0, scrollbarwidth, scrollbarspace+scrollbarheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
!global.!showannotations.!vbar = dw_scrollbar_new( !REXXDW.!DW_VERT, 0 )
Call dw_box_pack_start vscrollbox, !global.!showannotations.!vbar, scrollbarwidth, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/* Pack an area of empty space below the scrollbar */
Call dw_box_pack_start vscrollbox, 0, scrollbarwidth, scrollbarspace+scrollbarheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Create default pixmaps and connect our signals
 */
Do i = 1 To 5
   !global.!showannotations.!pixmap.i = dw_pixmap_new( !global.!showannotations.!pixmapbox.i, font_width, font_height, depth )
   Call dw_signal_connect !global.!showannotations.!pixmapbox.i, !REXXDW.!DW_EXPOSE_EVENT, 'ShowAnnotationsExposeCallback', i
   Call dw_signal_connect !global.!showannotations.!pixmapbox.i, !REXXDW.!DW_CONFIGURE_EVENT, 'ShowAnnotationsConfigureCallback', i
End

Call dw_signal_connect !global.!showannotations.!hbar, !REXXDW.!DW_VALUE_CHANGED_EVENT, 'ShowAnnotationsValueChangedCallback'
Call dw_signal_connect !global.!showannotations.!vbar, !REXXDW.!DW_VALUE_CHANGED_EVENT, 'ShowAnnotationsValueChangedCallback'
-- connect keypress to toplevel window
Call dw_signal_connect win, !REXXDW.!DW_KEY_PRESS_EVENT, 'ShowAnnotationsKeyPressCallback', win
/*
* Create a dialog for the window
*/
dialog_wait = dw_dialog_new( )
/*
* Create Done button
*/
b1 = dw_button_new( 'OK', 0 )
Call dw_box_pack_start topbox, b1, 400, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * We now have everything setup, we can connect the destroying of the top-level window
 * to the 'close' callback
 */
Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
Call dw_signal_connect linebutton, !REXXDW.!DW_CLICKED_EVENT, 'ShowAnnotationsGotoLineCallback', sb
/*
 * Set the Goto Line button be the default for ENTER
 */
Call dw_window_click_default sb, linebutton
/*
 * Reset status
 */
Call SetCursorNoWait
Call DisplayStatus ''
/*
 * Display the window
 */
width = (!global.!screen_width % 3) * 2
height = (!global.!screen_height % 3) * 2
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
/*
 * Wait for the dialog to come back
 */
Call dw_dialog_wait dialog_wait

Drop !global.!showannotations.!rel. !global.!showannotations.!user. !global.!showannotations.!date. !global.!showannotations.!line.
Return 0

ShowAnnotationsGotoLineCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, line_sb
lineno = dw_spinbutton_get_pos( line_sb ) - 1
lineno = lineno - (!global.!showannotations.!rows % 2)
If lineno < 0 Then lineno = 0
Call ShowAnnotationsValueChangedCallback !global.!showannotations.!vbar, lineno
Return 0

ShowAnnotationsConfigureCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, width, height, box
if trace() = 'F' Then say 'got ShowAnnotationsConfigureCallback' win width height box
depth = dw_color_depth_get()

!global.!showannotations.!rows = height % !global.!showannotations.!font_height
!global.!showannotations.!height = height

/* Destroy the old pixmap */
Call dw_pixmap_destroy !global.!showannotations.!pixmap.box

If box = 5 Then
   Do
      !global.!showannotations.!viewport_cols = width % !global.!showannotations.!font_width
      !global.!showannotations.!viewport_width = width
   End
/* Create new pixmap with the current sizes */
!global.!showannotations.!pixmap.box = dw_pixmap_new( !global.!showannotations.!pixmapbox.box, !global.!showannotations.!pixmap_width.box, !global.!showannotations.!height, depth )
/* Update scrollbar ranges with new values if its the file contents box */
If box = 5 Then
   Do
      Call dw_scrollbar_set_range !global.!showannotations.!hbar, !global.!showannotations.!max_width.5, !global.!showannotations.!viewport_cols
      Call dw_scrollbar_set_range !global.!showannotations.!vbar, !global.!showannotations.5.0, !global.!showannotations.!rows
   End

/* Redraw the window */
Call DisplayAnnotations box
Return 0

ShowAnnotationsExposeCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, junk, junk, junk, junk, box
if trace() = 'F' Then say 'got ShowAnnotationsExposeCallback' box
pixmap = !global.!showannotations.!pixmap.box
If box = 5 Then x = !global.!showannotations.!current_col * !global.!showannotations.!font_width
Else x = 0

Parse Value dw_window_get_pos_size( win ) With . . width height

Call dw_pixmap_bitblt win, , 0, 0, width, height, , pixmap, x, 0
Call dw_flush
Return 0

ShowAnnotationsValueChangedCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, value
If win = !global.!showannotations.!vbar Then
   Do
      !global.!showannotations.!current_row = value
      Call dw_scrollbar_set_pos !global.!showannotations.!vbar, value
   End
Else
   Do
      !global.!showannotations.!current_col = value
      Call dw_scrollbar_set_pos !global.!showannotations.!hbar, value
   End

Do i = 1 To 5
   Call DisplayAnnotations i
End
Return 0

/*
 * Handle keypress events for ShowAnnotations
 * Trap those that adjust the vertical position of the file; PgUp, PgDn, Up, Down
 * Return 1 to indicate we have handled the key, otherwise return 0 to pass on to
 * the focus window; in this case the spinbutton where the linenumber can be specified.
 */
ShowAnnotationsKeyPressCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg window, key, virtual_key, state, topwin
max_rows = !global.!showannotations.1.0
rcode = 0
Select
   When window = topwin Then
      Do
         Select
            When virtual_key = !REXXDW.!DW_VK_PRIOR Then
               Do
                  pos = Max( 0, !global.!showannotations.!current_row - !global.!showannotations.!rows )
                  Call ShowAnnotationsValueChangedCallback !global.!showannotations.!vbar, pos
                  Call ShowAnnotationsValueChangedCallback !global.!showannotations.!vbar, pos
                  rcode = 1
               End
            When virtual_key = !REXXDW.!DW_VK_NEXT Then
               Do
                  pos = Min( max_rows - !global.!showannotations.!rows, !global.!showannotations.!current_row + !global.!showannotations.!rows )
                  If pos > 0 Then
                     Do
                        Call ShowAnnotationsValueChangedCallback !global.!showannotations.!vbar, pos
                        Call ShowAnnotationsValueChangedCallback !global.!showannotations.!vbar, pos
                     End
                  rcode = 1
               End
            When virtual_key = !REXXDW.!DW_VK_UP Then
               Do
                  pos = Max( 0, !global.!showannotations.!current_row - 1 )
                  Call ShowAnnotationsValueChangedCallback !global.!showannotations.!vbar, pos
                  Call ShowAnnotationsValueChangedCallback !global.!showannotations.!vbar, pos
                  rcode = 1
               End
            When virtual_key = !REXXDW.!DW_VK_DOWN Then
               Do
                  pos = Min( max_rows - !global.!showannotations.!rows, !global.!showannotations.!current_row + 1 )
                  If pos > 0 Then
                     Do
                        Call ShowAnnotationsValueChangedCallback !global.!showannotations.!vbar, pos
                        Call ShowAnnotationsValueChangedCallback !global.!showannotations.!vbar, pos
                     End
                  rcode = 1
               End
            Otherwise Nop
         End
      End
   Otherwise Nop
End
Return rcode

DisplayAnnotations: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg box
if trace() = 'F' Then say 'got displayannotations' box
If box = 5 Then Call dw_color_foreground_set !global.!showannotations.!default_back
Else Call dw_color_foreground_set !REXXDW.!DW_CLR_WHITE
Call dw_draw_rect 0, !global.!showannotations.!pixmap.box, !REXXDW.!DW_FILL, 0, 0, !global.!showannotations.!pixmap_width.box, !global.!showannotations.!height
Call dw_color_background_set !REXXDW.!DW_CLR_WHITE
Call dw_color_foreground_set !REXXDW.!DW_CLR_BLACK
Do i = 1 To !global.!showannotations.!rows+1 While(i+!global.!showannotations.!current_row) < !global.!showannotations.box.0 + 1
   y = (i-1)*!global.!showannotations.!font_height
   fileline = i + !global.!showannotations.!current_row
   If !global.!showannotations.!have_syntax & box = 5 Then Call WriteAnnotationsTextLine y, !global.!showannotations.box.fileline
   Else Call dw_draw_text 0, !global.!showannotations.!pixmap.box, 0, y, !global.!showannotations.box.fileline
End
Call ShowAnnotationsExposeCallback !global.!showannotations.!pixmapbox.box, 0, 0, 0, 0, box
Return

WriteAnnotationsTextLine: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg y, fileline
/*
 * Start each line with the default colour and font
 */
Call dw_color_background_set !global.!showannotations.!default_back
Call dw_color_foreground_set !REXXDW.!DW_CLR_BLACK
If !REXXDW.!GTK_MAJOR_VERSION < 2 Then Call dw_window_set_font !global.!showannotations.!pixmapbox.5, !global.!fixedfont
/*
 * Now split the line up at each <span>
 */
x = 0
Do Forever
   If Left( fileline, 13 ) = '<span class="' Then
      Do
         Parse Var fileline '<span class="hl ' class '">' fragment '</span>' fileline
         Call dw_color_foreground_set !global.!syntax.!color.class
         If !REXXDW.!GTK_MAJOR_VERSION < 2 Then Call dw_window_set_font !global.!showannotations.!pixmapbox.5, !global.!syntax.!font.class
      End
   Else
      Do
         Parse Var fileline fragment '<span class="hl ' fileline
         If Strip( fileline ) \= '' Then fileline = '<span class="hl 'fileline
         Call dw_color_foreground_set !global.!showannotations.!default_fore
         If !REXXDW.!GTK_MAJOR_VERSION < 2 Then Call dw_window_set_font !global.!showannotations.!pixmapbox.5, !global.!fixedfont
      End
   fragment = ConvertHTMLMnemonics( fragment )
   Call dw_draw_text 0, !global.!showannotations.!pixmap.5, x, y, fragment
   x = x + !global.!showannotations.!font_width * Length( fragment )
   If fileline = '' Then Leave
End
Return

/*
 * This function is called from the filemenu; Delete From Repository command
 * Admin command only
 * Valid for multiple selections.
 * Only valid for Repository and Non-repository files
 */
DeleteFromRepositoryCallback: Procedure Expose !REXXDW. !global. !!last_repository
If !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then
   Do
      /*
       * This is done when deleting from the repository...
       */
      If dw_messagebox( 'Delete ?', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), 'Do you want to delete the selected files from the repository?' ) = !REXXDW.!DW_MB_RETURN_NO Then Return 0
      Select
         When !global.!!repository.!!type.!!last_repository = 'rcs' Then
            Do
               Call SysFileDelete !global.!current_filenames.1
               Call WriteStackToLogWindow 'default', quote( !global.!current_filenames.1 ) 'permanently deleted'
               Call RefreshFilesCallback
            End
         When !global.!!repository.!!type.!!last_repository = 'cvs' Then
            Do
               current_filenames = ''
               Do i = 1 To !global.!current_filenames.0
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'delete -f' quote( !global.!current_filenames.i )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address Command cmd With Output FIFO '' Error FIFO ''
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'commit -m "*** File Deleted ***"' quote( !global.!current_filenames.i )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address Command cmd With Output FIFO '' Error FIFO ''
                  current_filenames = current_filenames quote( !global.!current_filenames.i )
               End
               Call WriteStackToLogWindow 'default', current_filenames 'deleted'
               Call RefreshFilesCallback
               Call DisplayDeletedFiles
            End
         Otherwise Nop
      End
   End
Else
   Do
      /*
       * This is done when deleting from working directory...
       */
      If dw_messagebox( 'Delete ?', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), 'Do you want to delete the selected files from your working directory?' ) = !REXXDW.!DW_MB_RETURN_NO Then Return 0
      current_filenames = ''
      /*
       * For each selected file or directory, try and delete it.
       */
      Do i = 1 To !global.!current_nonrepindexes.0
         selected = !global.!current_nonrepindexes.i
         selected_file = Substr( !global.!nonrepfiles.?fullfilename.!!last_repository.selected, 5 )
         filetype = Substr( !global.!nonrepfiles.?filetype.!!last_repository.selected, 5 )
         If filetype = 'dir' Then Address Command !global.!cmddelr quote( selected_file )
         Else Call SysFileDelete selected_file
         current_filenames = current_filenames quote( !global.!current_nonrepfilenames.i )
      End
      /*
       * If we deleted at least one file, list it.
       */
      If current_filenames \= ''Then Call WriteStackToLogWindow 'default', current_filenames 'deleted'
      Call RefreshFilesCallback
   End
Return 0

/*
 * These callbacks are not called directly by a button, or a menu.
 */
GetFileRevisionsApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, combobox
str = dw_window_get_text( combobox )
if trace() = 'F' Then say 'got getfilerevisionsapply' 'value' str 'from' combobox
Call dw_window_destroy window
/*
 * If the user wants to select a date...
 */
If Left( str, 5 ) = 'Date:' Then str = 'Date:'Date( 'S', SubWord( str, 2 ), 'N' )
Call dw_dialog_dismiss dialog, 'apply' str
Return 0

GetFileRevisionsAndDestinationApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, combobox, entrybox
str = dw_window_get_text( combobox )
dest = dw_window_get_text( entrybox )
If dest = '' Then
   Do
      Call dw_messagebox 'No destination', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'No destination was selected.'
      Call dw_window_destroy window
      Call dw_dialog_dismiss dialog, ''
      Return 0
   End
/*
 * If the user wants to select a date...
 */
If Left( str, 5 ) = 'Date:' Then str = 'Date:'Date( 'S', SubWord( str, 2 ), 'N' )
Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply' str dest
Return 0

GenericDirectorySelector: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, entrybox, title
/*
 * dw_file_browse() at least under Windows changes directory!
 */
here = Directory()
dir = Strip( dw_file_browse( title, '', , !REXXDW.!DW_DIRECTORY_OPEN ) )
If dir \= '' Then Call dw_window_set_text entrybox, dir
Call Directory here
Return 0

GenericFileSelector: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, entrybox, title, default_dir, converttoshort, mode, extension
if trace() = 'F' Then say 'default' default_dir
/*
 * dw_file_browse() at least under Windows changes directory!
 */
here = Directory()
dir = Strip( dw_file_browse( title, default_dir, extension, mode ) )
If dir \= '' Then
   Do
      If !global.!os = 'WIN32' & converttoshort Then
         Do
            dir = w32sysshortfilename( dir )
         End
      Call dw_window_set_text entrybox, dir
   End
Call Directory here
Return 0

GetFileRevisionsDateSelector: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, entrybox
date = Strip( SelectDate() )
If date \= '' Then
   Do
      date = 'Date:' date
      Call dw_window_set_text entrybox, date
   End
Return 0

DateSelectorApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, day_sb, month_cb, year_sb
day = Right( dw_spinbutton_get_pos( day_sb ), 2, 0 )
mon = Right( 1 + dw_listbox_selected( month_cb ), 2, 0 )
year = dw_spinbutton_get_pos( year_sb )
Call dw_window_destroy window
/*
 * Check the date is valid
 */
Signal on Syntax Name ds_syntax
return_date = Date( 'N', year||mon||day, 'S' )
if trace() = 'F' Then say 'return date' return_date
Call dw_dialog_dismiss dialog, 'apply' return_date
Signal ds_cont
ds_syntax:
Call dw_messagebox 'Invalid Date', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), 'The date selected is invalid.'
Call dw_dialog_dismiss dialog, ''
Signal Off Syntax
ds_cont:
Return 0

CalendarSelectorApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., window, dialog, cal
/*
 * Get the date value
 */
newdate = dw_calendar_get_date( cal )
Call dw_dialog_dismiss dialog, 'apply' Date( 'N', newdate, 'S' )
/*
 * Destroy the window
 */
Call dw_window_destroy window
Return 0

/*
 * Callback for checkin, add, and setreminder
 */
GetCheckinDetailsApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., window, dialog, text_handle, cb_handle, want_text_win, type
if trace() = 'F' Then say 'window' window 'text_handle' text_handle 'cb_handle' cb_handle 'want_text_win' want_text_win 'type' type
cancel = 'apply'
If want_text_win = 'Y' & cancel = 'apply'Then
   Do
      Select
         When type = 'add' Then
            Do
               !global.!addfile_text = dw_window_get_text( text_handle )
               If !global.!addfile_text = '' Then
                  Do
                     Call dw_messagebox 'No comment', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'You must enter a description.'
                     cancel = 'retry'
                  End
               Else
                  Do
                     cancel  = 'apply'
                  End
            End
         When type = 'remind' Then
            Do
               !global.!remindertext = dw_mle_export( text_handle )
               cancel  = 'apply'
            End
         Otherwise
            Do
               !global.!checkin_text = dw_mle_export( text_handle )
               If !global.!checkin_text = '' Then
                  Do
                     Call dw_messagebox 'No comment', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), 'You must enter a comment.'
                     cancel  = 'retry'
                  End
               Else
                  Do
                     cancel  = 'apply'
                     Queue !global.!checkin_text
                  End
            End
      End
   End
If type = 'dir' | type = 'delete' | type = 'add' Then same = dw_checkbox_get( cb_handle )
Else same = 0

Call dw_dialog_dismiss dialog, cancel same
If cancel = 'apply' Then Call dw_window_destroy window
Return 0

ShowDifferencesApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, rev1cbox, rev2cbox, rep1cbox, rep2cbox, compare_rb1, compare_rb2, output_rb1, output_rb2
rev1 = dw_window_get_text( rev1cbox )
rev2 = dw_window_get_text( rev2cbox )
rep1 = dw_window_get_text( rep1cbox )
rep2 = dw_window_get_text( rep2cbox )
If dw_radiobutton_get( compare_rb1 ) Then difftype = 'ignorewhitespace'
Else difftype = ''
If dw_radiobutton_get( output_rb1 ) Then outputtype = 'logwindow'
Else outputtype = ''
/*
 * Get the repository indexes
 */
rep1_idx = 0
rep2_idx = 0
Do i = 1 To !global.!!number_repositories
   If !global.!!repository.!!name.i = rep1 Then rep1_idx = i
   If !global.!!repository.!!name.i = rep2 Then rep2_idx = i
End
If rep1 = 'Current Repository' Then rep1_idx = !!last_repository
If rep2 = 'Current Repository' Then rep2_idx = !!last_repository
If rev1 \= 'Latest' & rev1 \= 'Working' & rep1_idx \= !!last_repository Then
   Do
      Call dw_messagebox 'Warning', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), "You can only compare the 'Latest' or 'Working' revision in another repository."
      Call dw_dialog_dismiss dialog, 'retry'
      Return 0
   End
If rev2 \= 'Latest' & rev2 \= 'Working' & rep2_idx \= !!last_repository Then
   Do
      Call dw_messagebox 'Warning', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), "You can only compare the 'Latest' or 'Working' revision in another repository."
      Call dw_dialog_dismiss dialog, 'retry'
      Return 0
   End
If rep1 = rep2 & rev1 = rev2 Then
   Do
      Call dw_messagebox 'Warning', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), "You can't compare the same revision of the same file."
      Call dw_dialog_dismiss dialog, 'retry'
      Return 0
   End
Call dw_dialog_dismiss dialog, 'apply'
/*Call dw_dialog_dismiss dialog, 'apply:'rev1':'rev2':'rep1':'rep2':'rep1_idx':'rep2_idx':'comp':'out*/
Call dw_window_destroy window
/*
 * Run the differences now...
 */
If difftype = 'ignorewhitespace' Then diffflags = '-bw'
Else diffflags = ''

Call SetCursorWait
/*
 * get file 1
 */
If Left( rev1, 5 ) = 'Date:' Then
   Do
      rev1 = SubWord( rev1, 2 )
      rev1_val = '-D'Translate( rev1, '-', ' ' )
   End
Else rev1_val = '-r'rev1
label1 = rep1':' !global.!current_filenames.1'('rev1')'
tmptmp1 = Translate( Changestr( ':', label1, '-???' ), '_', '/' )
Select
   When !global.!!repository.!!type.rep1_idx = 'rcs' Then
      Do
      /* TODO */
         If rev1 = 'Working' Then
            Do
               file1 = RCSFileToFile( current_filenames.1, rep1_idx )
            End
         Else
            Do
               Address System 'co -p'rev1 quote( current_filenames.1 ) '>' file1 !global.!stderrdevnull
               Call Stream file1, 'C', 'CLOSE'
            End
      End
   When !global.!!repository.!!type.rep1_idx = 'cvs' Then
      Do
         /*
          * Save the current value of CVS_RSH because we have to put it
          * back to the saved value after we check out the files...
          */
         save_rsh = Value( 'CVS_RSH', , 'ENVIRONMENT' )
         /*
          * Checkout rev1...
          */
         Select
            When rev1 = 'Working' Then
               Do
                  If !REXXDW.!GTK_MAJOR_VERSION > 1 Then
                     Do
                        file1 = GenerateTempFile( tmptmp1 )
                        Address System 'cat' quote( !global.!current_filenames.1 ) '| iconv -f ISO-8859-1 -t UTF-8' With Output Stream file1 Error Stem junk.
                        Call Stream file1, 'C', 'CLOSE'
                     End
                  Else
                     file1 = CVSFileToWorkingFile( !global.!current_filenames.1, rep1_idx )
               End
            When rev1 = 'Latest' Then
               Do
                  file1 = GenerateTempFile( tmptmp1 )
                  Call SetCVSEnvVars rep1_idx
                  If !REXXDW.!GTK_MAJOR_VERSION > 1 Then
                     Do
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.rep1_idx '-d'!global.!!repository.!!path.rep1_idx 'checkout -p' quote( !global.!current_filenames.1 ) '| iconv -f ISO-8859-1 -t UTF-8'
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address System cmd With Output Stream file1 Error Stem junk.
                     End
                  Else
                     Do
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.rep1_idx '-d'!global.!!repository.!!path.rep1_idx 'checkout -p' quote( !global.!current_filenames.1 )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output Stream file1 Error Stem junk.
                     End
                  Call Stream file1, 'C', 'CLOSE'
               End
            Otherwise
               Do
                  file1 = GenerateTempFile( tmptmp1 )
                  Call SetCVSEnvVars rep1_idx
                  If !REXXDW.!GTK_MAJOR_VERSION > 1 Then
                     Do
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.rep1_idx '-d'!global.!!repository.!!path.rep1_idx 'checkout -p' rev1_val quote( !global.!current_filenames.1 ) '| iconv -f ISO-8859-1 -t UTF-8'
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address System cmd With Output Stream file1 Error Stem junk.
                     End
                  Else
                     Do
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.rep1_idx '-d'!global.!!repository.!!path.rep1_idx 'checkout -p' rev1_val quote( !global.!current_filenames.1 )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output Stream file1 Error Stem junk.
                     End
                  Call Stream file1, 'C', 'CLOSE'
               End
         End
         /*
          * Put back CVS_RSH value saved above...
          */
         Call Value 'CVS_RSH', save_rsh, 'ENVIRONMENT'
      End
   Otherwise Nop
End
/*
 * get file 2
 */
If Left( rev2, 5 ) = 'Date:' Then
   Do
      rev2 = SubWord( rev2, 2 )
      rev2_val = '-D'Translate( rev2, '-', ' ' )
   End
Else rev2_val = '-r'rev2
label2 = rep2':' !global.!current_filenames.1'('rev2')'
tmptmp2 = Translate( Changestr( ':', label2, '-???' ), '_', '/' )
Select
   When !global.!!repository.!!type.rep2_idx = 'rcs' Then
      Do
         /* TODO */
         If rev2 = 'Latest' Then
            Do
               Address System 'co -p' quote( current_filenames.1 ) '>' file2 !global.!stderrdevnull
            End
         Else
            Do
               Address System 'co -p'rev2 quote( current_filenames.1 ) '>' file2 !global.!stderrdevnull
            End
      End
   When !global.!!repository.!!type.rep2_idx = 'cvs' Then
      Do
         /*
          * Save the current value of CVS_RSH because we have to put it
          * back to the saved value after we check out the files...
          */
         save_rsh = Value( 'CVS_RSH', , 'ENVIRONMENT' )
         /*
          * Checkout rev2...
          */
         Select
            When rev2 = 'Working' Then
               Do
                  If !REXXDW.!GTK_MAJOR_VERSION > 1 Then
                     Do
                        file2 = GenerateTempFile( tmptmp2 )
                        Address System 'cat' quote( !global.!current_filenames.1 ) '| iconv -f ISO-8859-1 -t UTF-8' With Output Stream file2 Error Stem junk.
                        Call Stream file2, 'C', 'CLOSE'
                     End
                  Else
                     file2 = CVSFileToWorkingFile( !global.!current_filenames.1, rep2_idx )
               End
            When rev2 = 'Latest' Then
               Do
                  file2 = GenerateTempFile( tmptmp2 )
                  Call SetCVSEnvVars rep2_idx
                  If !REXXDW.!GTK_MAJOR_VERSION > 1 Then
                     Do
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.rep2_idx '-d'!global.!!repository.!!path.rep2_idx 'checkout -p' quote( !global.!current_filenames.1 ) '| iconv -f ISO-8859-1 -t UTF-8'
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address System cmd With Output Stream file2 Error Stem junk.
                     End
                  Else
                     Do
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.rep2_idx '-d'!global.!!repository.!!path.rep2_idx 'checkout -p' quote( !global.!current_filenames.1 )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output Stream file2 Error Stem junk.
                     End
                  Call Stream file2, 'C', 'CLOSE'
               End
            Otherwise
               Do
                  file2 = GenerateTempFile( tmptmp2 )
                  Call SetCVSEnvVars rep2_idx
                  If !REXXDW.!GTK_MAJOR_VERSION > 1 Then
                     Do
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.rep2_idx '-d'!global.!!repository.!!path.rep2_idx 'checkout -p' rev2_val quote( !global.!current_filenames.1 ) '| iconv -f ISO-8859-1 -t UTF-8'
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address System cmd With Output Stream file2 Error Stem junk.
                     End
                  Else
                     Do
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.rep2_idx '-d'!global.!!repository.!!path.rep2_idx 'checkout -p' rev2_val quote( !global.!current_filenames.1 )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output Stream file2 Error Stem junk.
                     End
                  Call Stream file2, 'C', 'CLOSE'
               End
         End
         /*
          * Put back CVS_RSH value saved above...
          */
         Call Value 'CVS_RSH', save_rsh, 'ENVIRONMENT'
      End
   Otherwise Nop
End
file1 = Changestr( '/', file1, !global.!ossep )
file2 = Changestr( '/', file2, !global.!ossep )
/*
 * Win32 .doc differences...
 */
If !global.!os = 'WIN32' & Translate( Right( !global.!current_filenames.1, 4 ) ) = '.DOC' Then
   Do
      /*
       * check if either of the files to be compared is the working copy, and if so
       * whether that file is currently open in Word with changes that haven't been made
       * If this is the case, inform the user we can save the file before the comparison
       * Irrespective of the answer, copy the working file to a temporary file before the
       * comparison i done
       */
      If rev1 = 'Working' Then
         Do
            If CheckIfFileChangedInWord( file1 ) Then
               Do
                  Call SetCursorNoWait
                  Return 0
               End
            -- copy the working file to the temporary file
            wfile1 = Changestr( '/', GenerateTempFile( tmptmp1 ), !global.!ossep )
            Address System !global.!cmdcopy quote( file1 ) quote( wfile1 ) !global.!devnull
            file1 = wfile1
         End
      If rev2 = 'Working' Then
         Do
            If CheckIfFileChangedInWord( file2 ) Then
               Do
                  Call SetCursorNoWait
                  Return 0
               End
            -- copy the working file to the temporary file
            wfile2 = Changestr( '/', GenerateTempFile( tmptmp2 ), !global.!ossep )
            Address System !global.!cmdcopy quote( file2 ) quote( wfile2 ) !global.!devnull
            file2 = wfile2
         End
      file1 = ConvertWordDocumentToText( file1, 0 )
      If file1 = '' Then
         Do
            Call SetCursorNoWait
            Return 0
         End
      Call AddTempFileForCleanup file1
      file2 = ConvertWordDocumentToText( file2, 0 )
      If file2 = '' Then
         Do
            Call SetCursorNoWait
            Return 0
         End
      Call AddTempFileForCleanup file2
   End
/*
 * Do the diff now...
 */
If outputtype = 'logwindow' Then
   Do
      Call RunOsCommand !global.!cmddiff diffflags quote( file1 ) quote( file2 )
      Call WriteOutputStemsToLogWindow 'default', 'Differences between' label1 'and' label2
   End
Else
   Do
      /*
       * If there is a valid syntax for the file's extension, use this
       */
      pos = Lastpos( '.', !global.!current_filenames.1 )
      If pos \= 0 Then
         Do
            syntax = FindSyntax( Substr( !global.!current_filenames.1, pos+1 ) )
            If syntax = '' Then syntax_flags = ''
            Else syntax_flags = '-S'syntax
            /*
             * Now if the user has a default theme, use it
             */
            If Left( !global.!!user.!!style, 1 ) \= '!' & !global.!!user.!!style \= '' & syntax_flags \= '' Then syntax_flags = syntax_flags '-s'!global.!!user.!!style
         End
      Else syntax_flags = ''
      If Left( !global.!!user.!!preference.!!tabspaces, 1 ) = '!' | !global.!!user.!!preference.!!tabspaces = 0 Then tabspaces_flag = ''
      Else tabspaces_flag = '-c'!global.!!user.!!preference.!!tabspaces
      /*
       * Call the external differences program
       */
      If !global.!!user.!!diffprog = '' | Left( !global.!!user.!!diffprog, 1 ) = '!' Then
         Do
            If !global.!os = 'OSX' & !global.!rexxdiff_by_env_variable = 1 Then
               Do
                  /*
                   * MacOS X leopard does not allow arguments to be passed to an application started with "open"
                   * command, so we have to set env variables for our args. rexxdiff, then will check for
                   * these set env variables.
                   */
                  Call Value 'REXXDIFF_ARG1', '-f'file1, 'ENVIRONMENT'
                  Call Value 'REXXDIFF_ARG2', '-F'file2, 'ENVIRONMENT'
                  Call Value 'REXXDIFF_ARG3', '-t'label1, 'ENVIRONMENT'
                  Call Value 'REXXDIFF_ARG4', '-T'label2, 'ENVIRONMENT'
                  i = 4
                  If syntax_flags \= '' Then
                     Do j = 1 To Words( syntax_flags )
                        flag = Word( syntax_flags, j )
                        i = i + 1
                        Call Value 'REXXDIFF_ARG'i, flag, 'ENVIRONMENT'
                     End
                  If tabspaces_flag \= '' Then
                     Do
                        i = i + 1
                        Call Value 'REXXDIFF_ARG'i, tabspaces_flag, 'ENVIRONMENT'
                     End
                  If diffflags \= '' Then
                     Do
                        i = i + 1
                        Call Value 'REXXDIFF_ARG'i, diffflags, 'ENVIRONMENT'
                     End
                  Call Value 'REXXDIFF_ARGS', i, 'ENVIRONMENT'
                  diffcmd = !global.!rexxdiff
               End
            Else
               Do
                  diffcmd = !global.!rexxdiff '-f'quote( file1 ) '-F'quote( file2 ) '-t'quote( label1 ) '-T'quote( label2 ) syntax_flags tabspaces_flag diffflags
               End
         End
      Else
         Do
            diffcmd = Changestr( '%F1%', !global.!!user.!!diffprog, file1 )
            diffcmd = Changestr( '%F2%', diffcmd, file2 )
            diffcmd = Changestr( '%T1%', diffcmd, label1 )
            diffcmd = Changestr( '%T2%', diffcmd, label2 )
         End
      cmd = !global.!background_start_nowindow diffcmd !global.!background_end
      If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
      Address System cmd
   End

Call SetCursorNoWait
Return 0

/*
 * Determines if the supplied files are currently open in word
 */
IsWordOpen: Procedure Expose !REXXDW. !global. !!last_repository
app = w32CreateObject("Word.Application")
num = w32GetProperty( app, 'Documents.Count' )
Call w32ReleaseObject app
Return num -- will return the number of files open; 0 if no files open or we just started word

/*
 * Determines if the supplied file is currently open in word and if there are unsaved changes
 */
CheckIfFileChangedInWord: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg fn
cancel = 0
app = w32CreateObject("Word.Application")
documents = w32getproperty( app, 'Documents' )
Call w32ReleaseObject app
Do Forever
   odoc = w32olenext( documents )
   If odoc = 0 Then Leave
   fp = w32getproperty( odoc, 'FullName' )
   If Translate( fn ) = Translate( fp ) & w32getproperty( odoc, 'Saved' ) = 0 Then
      Do
         -- document is open and has unsaved changes, ask if the file is to be saved
         rcode = dw_messagebox( 'Save file?', dw_or( !REXXDW.!DW_MB_YESNOCANCEL, !REXXDW.!DW_MB_QUESTION ), quote( fn ) 'has not been saved. Do you want the file saved before the comparison? Select Yes to have the file saved; No to compare without saving; Cancel to quit the action.' )
         Select
            When rcode = !REXXDW.!DW_MB_RETURN_NO Then Nop
            When rcode = !REXXDW.!DW_MB_RETURN_CANCEL Then cancel = 1
            Otherwise
               Do
                  -- save the file
                  Call w32CallProc odoc, 'Save'
               End
         End
      End
   Call w32ReleaseObject odoc
End
Return cancel

/*
 * Opens a Word Document and saves it as a text file for Show Differences
 */
ConvertWordDocumentToText: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg docfile, batch
/*
 * Get the version of Word
 */
app = w32CreateObject("Word.Application")
ver = w32GetProperty( app, 'Version' )
Call w32ReleaseObject app

wrd = w32CreateObject("Word.Basic")
Call w32CallProc wrd, "FileOpen", 's', docfile

If rc \= 0 Then
   Do
      If \batch Then
         Do
            Call dw_messagebox 'Conversion Error', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), 'Unable to convert Word document:' docfile 'to text.'
         End
      Call w32CallProc wrd, "FileClose"
      Call w32ReleaseObject wrd
      Return ''
   End

out = docfile'.txt'
Select
   When ver = '11.0removeme' Then Call w32CallProc wrd, "FileSaveAs", , out, 2, 0, , 0, , 0, 0, 0, 0, 0, 0, 1, 1
   Otherwise Call w32CallProc wrd, "FileSaveAs", 'si' , out, 2
End
If rc \= 0 Then
   Do
      If \batch Then
         Do
            Call dw_messagebox 'Conversion Error', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), 'Unable to convert Word document to text.' W32OLEGetError()
         End
      Call w32CallProc wrd, "FileClose"
      Call w32ReleaseObject wrd
      Return ''
   End
Call w32CallProc wrd, "FileClose"
Call w32ReleaseObject wrd
/*
 * When there are tables in the word document, the columns are separated by LF characters. These are treated as separate lines
 * by the Rexx interpreter, but are treated as embedded charcaters by the qdiff.exe. We need to ensure all single LF are converted to CRLF.
 * Simply read the file into memory, and write it out again.
 */
Call Stream out, 'C', 'OPEN READ'
Do i = 1 While Lines( out ) > 0
   line.i = Linein( out )
   line.0 = i
End
Call Stream out, 'C', 'CLOSE'
Call Stream out, 'C', 'OPEN WRITE REPLACE'
Do i = 1 To line.0
   Call Lineout out, line.i
End
Call Stream out, 'C', 'CLOSE'
Drop line.
Return out

ReportModuleHistoryApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, rev1cbox, rev2cbox, rep1cbox, rep2cbox, compare_rb1, compare_rb2, later_entry, earlier_entry, word_compare_cb, showall_compare_cb
rev1 = dw_window_get_text( rev1cbox )
rev2 = dw_window_get_text( rev2cbox )
rep1 = dw_window_get_text( rep1cbox )
rep2 = dw_window_get_text( rep2cbox )
mod1 = dw_window_get_text( later_entry )
mod2 = dw_window_get_text( earlier_entry )
wc = dw_checkbox_get( word_compare_cb )
sa = dw_checkbox_get( showall_compare_cb )
If dw_radiobutton_get( compare_rb1 ) Then diffflags = '-w'
Else diffflags = ''

If rep1 = 'Current Repository' Then rep1 = !global.!!repository.!!name.!!last_repository
If rep2 = 'Current Repository' Then rep2 = !global.!!repository.!!name.!!last_repository

If comp = 'ignorewhitespace' Then diffflags = '-w'
Else diffflags = ''

If wc = 1 Then wcflag = '-W'
Else wcflag = ''

If sa = 1 Then saflag = '-A'
Else saflag = ''

desc = 'Report differences between' rep1':'mod1'('rev1')' 'and' rep2':'mod2'('rev2')'
type = 'mods'
compiler_type = '.'
/*
 * Create the basefile and the job description file
 */
basefile = GenerateJobBaseFile()
Call CreateDescriptionFile basefile, desc, type, compiler_type
cmd = !global.!qocca_command 'mods -n'rep1 '-N'rep2 quote( '-m'mod1 ) quote( '-M'mod2 ) quote( '-v'rev1 ) quote( '-V'rev2 ) '-B'basefile '-p' diffflags wcflag saflag
outputfile = basefile'.out'
basedir = Changestr( !global.!ossep, !global.!!repository.!!working_dir.!!last_repository || !global.!current_dirname.!!last_repository, '/' )
/*
 * Increment the active jobs
 */
Call IncrementRunningJobs basefile
/*
 * Run it now, redirecting output to the outputfile
 */
Call WriteMessageHeaderToLogWindow desc 'submitted'
Call WriteMessageToLogWindow '  Command to run:' cmd
Call WriteMessageTrailerToLogWindow desc 'submitted'
Address System !global.!background_start_nowindow cmd !global.!background_end
Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply'

Return 0

GenericCloseCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., window, dialog
Call dw_window_destroy window
If dialog \= '' Then Call dw_dialog_dismiss dialog, 'close'
Return 0

FilePropertiesApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository !det.
Parse Arg button, window, dialog, file_type_combo, desc_entry, fn, old_type, old_desc, locks_cb
/*
 * If we are an administrator and the description has changed, update it in the
 * repository.
 */
filepropchange = 'N'
If !global.!administrator.!!last_repository = 'Y' Then
   Do
      new_desc = dw_window_get_text( desc_entry )
      idx = dw_listbox_selected( file_type_combo )
      new_type = dw_listbox_get_text( file_type_combo, idx )
      if trace() = 'F' Then say new_type':'old_type
      Select
         When !global.!!repository.!!type.!!last_repository = 'cvs' Then
            Do
               If locks_cb = 'LOCKS_CB' Then unlock = 0
               Else
                  Do
                     unlock = dw_checkbox_get( locks_cb )
                  End
               /*
                * CVS can't run admin with no working file...
                */
               If new_desc \= old_desc | new_type \= old_type | unlock Then
                  Do
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout' quote( fn )
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address Command cmd With Output Stem junk. Error Stem junk.
                     Call RefreshDirectoryIcons
                  End
               If new_desc \= old_desc Then
                  Do
                     Queue new_desc
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'admin -t' quote( fn )
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address Command cmd With Input FIFO '' Output Stem osout. Error Stem oserr.
                     Call WriteOutputStemsToLogWindow 'default', 'Updating module description for' quote( fn )
                     filepropchange = 'Y'
                  End
               If new_type \= old_type Then
                  Do
                     comment = 'Changing file type for' quote( fn ) 'to' new_type
                     If new_type = 'Binary' Then new_type = 'b'
                     Else new_type = 'kv'
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'admin -k'new_type quote( fn )
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address Command cmd With Output Stem osout. Error Stem oserr.
                     Call WriteOutputStemsToLogWindow 'default', comment
                     filepropchange = 'Y'
                  End
               If unlock Then
                  Do
                     Parse Var !det.!lock . ':' rev
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'admin -u'Strip( rev ) quote( fn )
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address Command cmd With Output Stem osout. Error Stem oserr.
                     Call WriteOutputStemsToLogWindow 'default', 'Unlocking module:' quote( fn )
                     filepropchange = 'Y'
                  End
            End
         Otherwise Nop
      End
   End
if trace() = 'F' Then say 'got filepropertiesdone for' window
Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply' filepropchange
Return 0

GetLabelApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, bo_box, nl_box, overwrite_cb, edit_label_file_cb
based_on = dw_window_get_text( bo_box )
lab = dw_window_get_text( nl_box )
overwrite_label = dw_checkbox_get( overwrite_cb )
edit_label_file = dw_checkbox_get( edit_label_file_cb )
Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply' lab based_on overwrite_label edit_label_file
Return 0

/*
 * End of callbacks
 */
DisplayDirProperties: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Directory properties... (from build.conf)
 *
 * +--------------------------------------------------------------+
 * | Text                                                         |
 * |                                                              |
 * +--------------------------------------------------------------+
 * | UNIX | W32 | OS/2 | BeOS | Amiga |                           |
 * |------+-----+------+------+-------+---------------------------|
 * | Make Directory:             ________________________________ | !dir.!makedirectory.this_dir.!platform
 * | Make Command:               ________________________________ | !dir.!makecommand.this_dir.!platform
 * | Debug Make Command:         ________________________________ | !dir.!debugmakecommand.this_dir.!platform
 * | Custom Make Command:        ________________________________ | !dir.!custommakecommand.this_dir.!platform
 * | Build Command:              ________________________________ | !dir.!buildcommand.this_dir.!platform
 * | Debug Build Command:        ________________________________ | !dir.!debugbuildcommand.this_dir.!platform
 * | Custom Build Command:       ________________________________ | !dir.!custombuildcommand.this_dir.!platform
 * | Promote Command:            ________________________________ | !dir.!promotecommand.this_dir.!platform
 * | Install Script:             ________________________________ | !dir.!installcommand.this_dir.!platform
 * | Install Dirs:               ________________________________ | !dir.!installdirs.this_dir.!platform
 * | Default Compiler:           _______________________________v | !dir.!compiler.this_dir.!platform
 * | Email Template:             ________________________________ | !dir.!emailer.this_dir.!platform
 * +--------------------------------------------------------------+
 * | Reserved Checkout:          X                                | !dir.!reservedcheckout.this_dir
 * +--------------------------------------------------------------+
 * |                            Cancel                            |
 * +--------------------------------------------------------------+
 * |                            Apply                             |
 * +--------------------------------------------------------------+
 *
 * On success, updates build.conf
 *
 */
/*
 * If our current platform is not already in the list of platforms, add it now so we can create
 * an empty page
 */
If Wordpos( !global.!os, !global.!platforms ) = 0 Then !global.!platforms = !global.!platforms !global.!os
num_platforms = Words( !global.!platforms )
this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Edit Directory Properties for:' !global.!current_dirname.!!last_repository, !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Add a disabled MLE with instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, 400, 50, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call DirpropertiesHelpFocusCallback t1, 'INITIAL', t1
/*
 * Add a notebook
 */
!global.!dirnotebookbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start topbox, !global.!dirnotebookbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Create our repository notebook
 */
!global.!dirnotebook = dw_notebook_new( 0, !REXXDW.!DW_TAB_TO_TOP )
Call dw_box_pack_start !global.!dirnotebookbox, !global.!dirnotebook, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirnotebook, !REXXDW.!DW_SWITCH_PAGE_EVENT, 'SwitchDirectoryCallback'
/*
 * For each platform, we need to create a notebook tab page
 */
Do i = 1 To num_platforms
   Call AddDirectoryPage i, t1, this_dir
End
/*
 * Create reserved checkout checkbox
 */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
!global.!dirproperties.?rc_cb = dw_checkbox_new( 'Reserved Checkout', 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?rc_cb, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If !global.!dir.!reservedcheckout.this_dir = 'Y' Then Call dw_checkbox_set !global.!dirproperties.?rc_cb, !REXXDW.!DW_CHECKED
Else Call dw_checkbox_set !global.!dirproperties.?rc_cb, !REXXDW.!DW_UNCHECKED
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Save us passing this to the callbacks
 */
!global.!dirproperties.?this_dir = this_dir
/*
 * Display the window
 */
width = 500
height = 520  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'DirPropertiesApplyCallback', win, dialog_wait
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End

Return 0

AddDirectoryPage: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg i, t1, this_dir
title_width = 150
/*
 * Create our notebook page and box to pack...
 */
!global.!dirnotebookpagebox.i = dw_box_new( !REXXDW.!DW_VERT )
!global.!dirnotebookpage.i = dw_notebook_page_new( !global.!dirnotebook, 0, !REXXDW.!DW_PAGE_TO_BACK )
/*
 * Pack the box for the notebook page contents into the notebook page and set the
 * text of the notebook page tab
 */
Call dw_notebook_pack !global.!dirnotebook, !global.!dirnotebookpage.i, !global.!dirnotebookpagebox.i
Call dw_notebook_page_set_text !global.!dirnotebook, !global.!dirnotebookpage.i, Word( !global.!platforms, i )
/*
 * Add the boxes and widgets for the left column...
 */
/* make directory */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Make Directory', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!makedirectory.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!makedirectory.this_dir.i
!global.!dirproperties.?md.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?md.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?md.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'MD', t1
/* make command */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Make Command', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!makecommand.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!makecommand.this_dir.i
!global.!dirproperties.?mc.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?mc.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?mc.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'MC', t1
/* debug make command */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Debug Make Command', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!debugmakecommand.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!debugmakecommand.this_dir.i
!global.!dirproperties.?dmc.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?dmc.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?dmc.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'DMC', t1
-- custom make command
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Custom Make Command', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!custommakecommand.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!custommakecommand.this_dir.i
!global.!dirproperties.?cmc.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?cmc.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?cmc.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'DMC', t1
/* build command */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Build Command', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!buildcommand.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!buildcommand.this_dir.i
!global.!dirproperties.?bc.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?bc.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?bc.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'BC', t1
/* debug build command */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Debug Build Command', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!debugbuildcommand.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!debugbuildcommand.this_dir.i
!global.!dirproperties.?dbc.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?dbc.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?dbc.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'DBC', t1
-- custom build command
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Custom Build Command', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!custombuildcommand.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!custombuildcommand.this_dir.i
!global.!dirproperties.?cbc.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?cbc.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?cbc.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'DBC', t1
/* promote directory */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Promote Directory', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!promotedirectory.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!promotedirectory.this_dir.i
!global.!dirproperties.?pd.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?pd.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?pd.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'PD', t1
/* promote command */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Promote Command', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!promotecommand.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!promotecommand.this_dir.i
!global.!dirproperties.?pc.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?pc.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?pc.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'PC', t1
/* install command */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Install Script', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!installcommand.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!installcommand.this_dir.i
!global.!dirproperties.?is.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?is.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?is.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'IS', t1
/* install directories */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Install Directories', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!installdirs.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!installdirs.this_dir.i
!global.!dirproperties.?id.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?id.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?id.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'ID', t1
/* default compiler */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Default Compiler', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!compiler.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!compiler.this_dir.i
!global.!dirproperties.?dc.i = CreateCompilersCombobox( tmpbox, 'false', oldvalue )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?dc.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/* email template */
tmpbox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start !global.!dirnotebookpagebox.i, tmpbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
tmp = dw_text_new( 'Email Template', 0 )
Call dw_box_pack_start tmpbox, tmp, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
If Left( !global.!dir.!emailer.this_dir.i, 1 ) = '!' Then oldvalue = ''
Else oldvalue = !global.!dir.!emailer.this_dir.i
!global.!dirproperties.?em.i = dw_entryfield_new( oldvalue, 0 )
Call dw_box_pack_start tmpbox, !global.!dirproperties.?em.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect !global.!dirproperties.?em.i, !REXXDW.!DW_SET_FOCUS_EVENT, 'DirPropertiesHelpFocusCallback', 'EM', t1
/*
 * Set the tab for the current platform to be the default
 */
platform_index = Wordpos( !global.!os, !global.!platforms )
Call dw_notebook_page_set !global.!dirnotebook, platform_index-1

Return 0

DirPropertiesApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., window, dialog
num_platforms = Words( !global.!platforms )
this_dir = !global.!dirproperties.?this_dir
/*
 * We have valid responses, get the new values for all platforms...
 */
Do i = 1 To num_platforms
   !global.!dir.!makedirectory.this_dir.i = dw_window_get_text( !global.!dirproperties.?md.i )
   !global.!dir.!makecommand.this_dir.i = dw_window_get_text( !global.!dirproperties.?mc.i )
   !global.!dir.!debugmakecommand.this_dir.i = dw_window_get_text( !global.!dirproperties.?dmc.i )
   !global.!dir.!custommakecommand.this_dir.i = dw_window_get_text( !global.!dirproperties.?cmc.i )
   !global.!dir.!buildcommand.this_dir.i = dw_window_get_text( !global.!dirproperties.?bc.i )
   !global.!dir.!debugbuildcommand.this_dir.i = dw_window_get_text( !global.!dirproperties.?dbc.i )
   !global.!dir.!custombuildcommand.this_dir.i = dw_window_get_text( !global.!dirproperties.?cbc.i )
   !global.!dir.!promotedirectory.this_dir.i = dw_window_get_text( !global.!dirproperties.?pd.i )
   !global.!dir.!promotecommand.this_dir.i = dw_window_get_text( !global.!dirproperties.?pc.i )
   !global.!dir.!installcommand.this_dir.i = dw_window_get_text( !global.!dirproperties.?is.i )
   !global.!dir.!installdirs.this_dir.i = dw_window_get_text( !global.!dirproperties.?id.i )
   !global.!dir.!compiler.this_dir.i = ConvertCompilerNameToAbbrev( dw_window_get_text( !global.!dirproperties.?dc.i )  )
   !global.!dir.!emailer.this_dir.i = dw_window_get_text( !global.!dirproperties.?em.i )
End
!global.!dir.!makedirectory.this_dir.0 = num_platforms
!global.!dir.!makecommand.this_dir.0 = num_platforms
!global.!dir.!debugmakecommand.this_dir.0 = num_platforms
!global.!dir.!custommakecommand.this_dir.0 = num_platforms
!global.!dir.!buildcommand.this_dir.0 = num_platforms
!global.!dir.!debugbuildcommand.this_dir.0 = num_platforms
!global.!dir.!custombuildcommand.this_dir.0 = num_platforms
!global.!dir.!promotedirectory.this_dir.0 = num_platforms
!global.!dir.!promotecommand.this_dir.0 = num_platforms
!global.!dir.!installcommand.this_dir.0 = num_platforms
!global.!dir.!installdirs.this_dir.0 = num_platforms
!global.!dir.!compiler.this_dir.0 = num_platforms
!global.!dir.!emailer.this_dir.0 = num_platforms
/*
 * If the reserved checkout item has changed, we want to update the directory tree
 */
If dw_checkbox_get( !global.!dirproperties.?rc_cb ) Then !global.!dir.!reservedcheckout.this_dir = 'Y'
Else !global.!dir.!reservedcheckout.this_dir = 'N'
If Wordpos( this_dir, !global.!targets ) = 0 Then !global.!targets = !global.!targets this_dir
/*
 * Write the new build.conf
 */
Call WriteBuildConf 'Added make and/or install details for' !global.!current_dirname.!!last_repository
/*
 * Change the icon for the directory in the tree
 */
Parse Value dw_tree_item_get_data( !global.!tree.!!last_repository, !global.!selected_treeitem.!!last_repository ) With parentitem mydir
mydir = Strip( mydir, 'B', '"' )
Parse Value DetermineModuleIconAndName( mydir ) With icon currentdir
Call dw_tree_item_change !global.!tree.!!last_repository, !global.!selected_treeitem.!!last_repository, currentdir, icon
/*
 * In case our directory has changed to/from reserved checkout
 */
Call SetLockedStatusIcon
/*
 * We will also update the directory toolbar buttons...
 */
Call AdjustDirToolbarList
Call AdjustDirToolbarButtons
/*
 * ...and the file toolbar buttons...
 */
Call AdjustFileToolbarList
Call AdjustFileToolbarButtons
/*
 * Close the window and dialog and return
 */
Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply'

Return 0

DirPropertiesHelpFocusCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, which, textwin
help.initial = 'For each platform that is to be a buildable target, enter the details below.'
help.md = "Enter the directory where a make is to be done from. This directory is a relative directory specification from the module directory; generally something as simple as 'make'."
help.mc = "Enter the command used to make the module on this platform. This is the command executed when the 'Make' command is selected. Multiple commands can be separated by ';' on UNIX or '&&' on Win32. Leaving this empty will prevent this module from being a target of a make."
help.dmc = "Enter the command used to do a debug make of the module on this platform. This is the command executed when the 'Debug Make' command is selected. Multiple commands can be separated by ';' on UNIX or '&&' on Win32."
help.cmc = "Enter the command used to do a custom make of the module on this platform. This is the command executed when the 'Custom Make' command is selected. Multiple commands can be separated by ';' on UNIX or '&&' on Win32."
help.bc = "Enter the command used to build the module on this platform. This is the command executed when the 'Build' command is selected. Multiple commands can be separated by ';' on UNIX or '&&' on Win32. Leaving this empty will prevent this module from being a target of a build."
help.dbc = "Enter the command used to do a debug build of the module on this platform. This is the command executed when the 'Debug Build' command is selected. Multiple commands can be separated by ';' on UNIX or '&&' on Win32."
help.cbc = "Enter the command used to do a custom build of the module on this platform. This is the command executed when the 'Custom Build' command is selected. Multiple commands can be separated by ';' on UNIX or '&&' on Win32."
help.pc = "Enter the command used to promote the module on this platform. This is the command executed when the 'Promote' command is selected. Multiple commands can be separated by ';' on UNIX or '&&' on Win32. Leaving this empty will prevent this module from being a target of a promote. Specify %V% as the substitution token for the version, and %D% for the install directories."
help.is = 'Enter the name and path of the installation script to be run when installing on this platform.  The file should be specified relative to the directory you are configuring; eg. install/install.sh.',
          ' Leaving this empty will prevent this directory being a target of an install.'
help.id = 'Enter the directories (relative to the module directory) that are to be included in a promote.'
help.dc = 'From the drop down box, select the compiler to be used as the default compiler for this target.'
help.em = "Enter the command used to email (via SMTP) various users of the repository. Indicate where the filename in the command appears with %F%, a recipient by %R%, and subject by %S%."
help.rc = "Check this button if you want all files in this directory subject to reserved checkout."
Call dw_window_enable textwin
Call dw_mle_set_cursor textwin, 0
Call dw_mle_delete textwin
Call dw_mle_import textwin, help.which, -1
Call dw_window_disable textwin
Return 0

AbortText: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg msg
Say msg
If RxFuncQuery( 'DW_DropFuncs' ) = 0 Then Call dw_DropFuncs
/*
 * Cleanup OLE
 */
If !global.!loadedw32util = 1 Then
   Do
      Call w32olecleanup
      Call w32dropfuncs
   End
Exit 1

novalue:
say 'got a novalue'
Exit 1

ParseCompilerLine: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg compiler_type, line
/*
 * Check if the line is a warning...
 */
f = ''
l = ''
c = ''
m = ''
If Datatype( !global.!compiler_template.??warning.compiler_type.0 ) \= 'NUM' Then Return ''
Do i = 1 To !global.!compiler_template.??warning.compiler_type.0
   tmp = !global.!compiler_template.??warning.compiler_type.i
   If tmp \= '' Then
      Do
         Interpret 'Parse Var line' tmp
         /*
          * Only matters if message and filename are not empty
          */
         If m \= '' & f \= '' Then
            Do
               modifier = 'warning'
               Return modifier','f','l','c','m
            End
      End
End
/*
 * Check if the line is an error...
 */
Do i = 1 To !global.!compiler_template.??error.compiler_type.0
   tmp = !global.!compiler_template.??error.compiler_type.i
   If tmp \= '' Then
      Do
         Interpret 'Parse Var line' tmp
         /*
          * Only matters if message and filename are not empty
          */
         If m \= '' & f \= '' Then
            Do
               modifier = 'error'
               Return modifier','f','l','c','m
            End
      End
End
Return ''

/*
 * List of compilers; their names, and templates for warnings and errors.
 * Only the parameters 'm' and 'f' are significant
 */
SetCompilerTemplates: Procedure Expose !REXXDW. !global. !!last_repository
!global.!compiler_template.??COMPILER_TYPES = 'NONE GCC VC WCCWIN32 WCCQNX JAVAC DELPHI4 CSHARP'
!global.!compiler_template.??COMPILER_NAME.NONE = 'None'
!global.!compiler_template.??COMPILER_NAME.GCC = 'GNU C/C++'
!global.!compiler_template.??COMPILER_NAME.VC = 'Microsoft Visual C++'
!global.!compiler_template.??COMPILER_NAME.WCCWIN32 = 'Watcom C++ on Win32'
!global.!compiler_template.??COMPILER_NAME.WCCQNX = 'Watcom C++ on QNX'
!global.!compiler_template.??COMPILER_NAME.JAVAC = 'Java Compiler'
!global.!compiler_template.??COMPILER_NAME.DELPHI4 = 'Borland Delphi 4'
!global.!compiler_template.??COMPILER_NAME.CSHARP = 'C#'
!global.!compiler_template.??WARNING.NONE.0 = 0
!global.!compiler_template.??WARNING.GCC.0 = 2
!global.!compiler_template.??WARNING.GCC.1 = "f ':' l ':' c ': warning:' m"
!global.!compiler_template.??WARNING.GCC.2 = "f ':' l ': warning:' m"
!global.!compiler_template.??ERROR.NONE.0 = 0
!global.!compiler_template.??ERROR.GCC.0 = 1
!global.!compiler_template.??ERROR.GCC.1 = "f ':' l ':' m"
!global.!compiler_template.??WARNING.VC.0 = 1
!global.!compiler_template.??WARNING.VC.1 = "f '(' l ') : warning' . ':' m"
!global.!compiler_template.??ERROR.VC.0 = 2
!global.!compiler_template.??ERROR.VC.1 = "f '(' l ') : error' . ':' m"
!global.!compiler_template.??ERROR.VC.2 = "f '(' l ') : fatal error' . ':' m"
!global.!compiler_template.??WARNING.WCCWIN32.0 = 1
!global.!compiler_template.??WARNING.WCCWIN32.1 = "f '(' l '): Warning!' . ':' m"
!global.!compiler_template.??ERROR.WCCWIN32.0 = 1
!global.!compiler_template.??ERROR.WCCWIN32.1 = "f '(' l '): Error!' . ':' m"
!global.!compiler_template.??WARNING.WCCQNX.0 = 1
!global.!compiler_template.??WARNING.WCCQNX.1 = "f '(' l '): Warning!' . ':' m"
!global.!compiler_template.??ERROR.WCCQNX.0 = 1
!global.!compiler_template.??ERROR.WCCQNX.1 = "f '(' l '): Error!' . ':' m"
!global.!compiler_template.??WARNING.JAVAC.0 = 0
!global.!compiler_template.??ERROR.JAVAC.0 = 1
!global.!compiler_template.??ERROR.JAVAC.1 = "f ':' l ':' m"
!global.!compiler_template.??WARNING.DELPHI4.0 = 1
!global.!compiler_template.??WARNING.DELPHI4.1 = "f '(' l ') Warning:' m"
!global.!compiler_template.??ERROR.DELPHI4.0 = 2
!global.!compiler_template.??ERROR.DELPHI4.1 = "f '(' l ') Error:' m"
!global.!compiler_template.??ERROR.DELPHI4.2 = "f '(' l ') Fatal:' m"
!global.!compiler_template.??WARNING.CSHARP.0 = 1
!global.!compiler_template.??WARNING.CSHARP.1 = ". '[compile]' f '(' l ',' c '): warning' . ':' m"
!global.!compiler_template.??ERROR.CSHARP.0 = 2
!global.!compiler_template.??ERROR.CSHARP.1 = ". '[compile]' f '(' l ',' c '): error' . ':' m"
!global.!compiler_template.??ERROR.CSHARP.2 = ". '[compile] error ' f ':' m"
Return

FindRepository: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg repository
If Left( !global.!!number_repositories, 1 ) = '!' Then Return 0
Do i = 1 To !global.!!number_repositories
   If !global.!!repository.!!name.i = repository Then Return i
End
Return 0

SetRepositoryEnvs: Procedure Expose !REXXDW. !global. !!last_repository
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do

         Call SetCVSEnvVars !!last_repository
         Call Directory !global.!!repository.!!working_dir.!!last_repository
         /*
          * While we're here, we will check to see if there is a toplevel; ie in
          * !global.!!repository.!!working_dir.!!last_repository a CVS directory and the
          * appropriate admin files. If not create the missing stuff.
          */
         cvsdir = Directory()||!global.!ossep'CVS'
         If Stream( cvsdir, 'C', 'QUERY EXISTS' ) = '' Then
            Do
               Call CreateDirectory( cvsdir )
            End
         cvsentries = cvsdir||!global.!ossep'Entries'
         If Stream( cvsentries, 'C', 'QUERY EXISTS' ) = '' Then
            Do
               Call Stream cvsentries, 'C', 'OPEN WRITE REPLACE'
               Call Lineout cvsentries, 'D'
               Call Stream cvsentries, 'C', 'CLOSE'
            End
         cvsroot = cvsdir||!global.!ossep'Root'
         If Stream( cvsroot, 'C', 'QUERY EXISTS' ) = '' Then
            Do
               Call Stream cvsroot, 'C', 'OPEN WRITE REPLACE'
               Call Lineout cvsroot, !global.!!repository.!!path.!!last_repository
               Call Stream cvsroot, 'C', 'CLOSE'
            End
         cvsrepository = cvsdir||!global.!ossep'Repository'
         If Stream( cvsrepository, 'C', 'QUERY EXISTS' ) = '' Then
            Do
               Call Stream cvsrepository, 'C', 'OPEN WRITE REPLACE'
               Call Lineout cvsrepository,'.'
               Call Stream cvsrepository, 'C', 'CLOSE'
            End
      End
   Otherwise Nop
End
Return

/*
 * Sets environment variables for CVS. DO NOT Expose !!last_repository
 */
SetCVSEnvVars: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg repno
/*Call Value 'CVSROOT', !global.!!repository.!!path.repno, 'ENVIRONMENT'*/
If !global.!!repository.!!contype.repno = 'rsh' Then Call Value 'CVS_RSH', 'rsh', 'ENVIRONMENT'
Else
   Do
      If Datatype( !global.!!repository.!!cvsport.repno ) = 'NUM' Then
         Do
            If !global.!!repository.!!cvsport.repno = 2401 Then cvs_rsh = 'ssh'
            Else cvs_rsh = 'sshcvs'
         End
      Else cvs_rsh = 'ssh'
      Call Value 'CVS_PORT', !global.!!repository.!!cvsport.repno, 'ENVIRONMENT'
      Call Value 'CVS_RSH', cvs_rsh, 'ENVIRONMENT'
   End
Return

ConnectToRepository: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg newrepno
Return

DisconnectFromRepository: Procedure Expose !REXXDW. !global. !!last_repository
Return

/*
 * --------------------------------------------------------------------
 * This function reads the configuration file. The configuration
 * file consists of lines of the form:
 *   keyword = value
 * which are simply interpreted.
 * Blank lines and comment lines (starting with ';') are ignored
 * --------------------------------------------------------------------
 */
GetConfigurationFile: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Check out for readonly, the build.conf from the admin directory. Then
 * read the contents of build.conf into global variables.
 */
!global.!working_conf = Translate( !global.!!repository.!!working_dir.!!last_repository'admin'!global.!ossep'build.conf', !global.!ossep, '/' )
_root = !global.!!repository.!!shortpath.!!last_repository
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         Call CreateDirectory !global.!!repository.!!working_dir.!!last_repository'admin'
         rcsfile = _root'/admin/RCS/build.conf,v'
         Address System 'co -f' rcsfile !global.!working_conf !global.!devnull
         If rc \= 0 Then AbortText('Unable to check out configuration file:' rcsfile)
         /*
          * Now have build.conf in !global.!working_conf file
          */
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         !global.!offline.!!last_repository = 0
         /*
          * Check global ?offline flag (set from -o command line option)
          */
         If  \!global.!offline Then
            Do
               /*
                * Try and checkout admin/build.conf
                */
               cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q checkout admin/build.conf'
               If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
               Address Command cmd With Output Stem osout. Error Stem oserr.
               If rc \= 0 Then
                  Do
                     If !global.!!repository.!!public_cvs.!!last_repository = 'Y' Then
                        Do
                           /*
                            * If there is no admin/build.conf, we still need to work
                            * somehow ?? TODO
                            */
                           dir = Translate( !global.!!repository.!!working_dir.!!last_repository'admin', '/', !global.!ossep )
                           Call CreateDirectory dir
                           Call CreateDummyBuildConf 'N'
                        End
                     Else
                        Do
                           Call NotifyNoConnection
                        End
                  End
            End
      End
   When !global.!!repository.!!type.!!last_repository = 'hg' Then
      Do
         !global.!offline.!!last_repository = 0
         /*
          * Check global ?offline flag (set from -o command line option)
          */
         If  \!global.!offline Then
            Do
               /*
                * Try and checkout admin/build.conf
                */
               cmd = 'hg' '-d'!global.!!repository.!!path.!!last_repository '-q checkout admin/build.conf'
say  .line todo cmd
               If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
               Address Command cmd With Output Stem osout. Error Stem oserr.
               If rc \= 0 Then
                  Do
                     /*
                      * If there is no admin/build.conf, we still need to work
                      * somehow ?? TODO
                      */
                     dir = Translate( !global.!!repository.!!working_dir.!!last_repository'admin', '/', !global.!ossep )
                     Call CreateDirectory dir
                     Call CreateDummyBuildConf 'N'
                  End
            End
      End
   Otherwise
      Do
         If !global.!batch_mode Then Call AbortText 'Cannot handle repository of type:' !global.!!repository.!!type.!!last_repository
         Else Call AbortText 'Cannot handle repository of type:' !global.!!repository.!!type.!!last_repository
      End
End
Call ReadBuildConf
Return

NotifyNoConnection: Procedure Expose !REXXDW. !global. !!last_repository oserr.
/*
 * We are connecting to a cvs repository that is under
 * our control. ie. it has an admin/build.conf,
 * otherwise its an error
 */
If \!global.!batch_mode Then
   Do
      Call WriteMessageHeaderToLogWindow 'Connection failed'
      Do i = 1 To oserr.0
         Call WriteMessageToLogWindow '  'oserr.i, 'redfore'
      End
      Call WriteMessageTrailerToLogWindow 'Connection failed'
      Call dw_messagebox 'No connection', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Unable to contact' quote( !global.!!repository.!!name.!!last_repository ) 'repository. Working offline.'
   End
Else
   Do
      Do i = 1 To oserr.0
         Say oserr.i
      End
      AbortText( 'Unable to contact' quote( !global.!!repository.!!name.!!last_repository ) 'repository.' )
   End
!global.!offline.!!last_repository = 1
Return

ReadBuildConf: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Set !targets to the empty string to simplify processing later
 */
!global.!working_conf = Translate( !global.!!repository.!!working_dir.!!last_repository'admin'!global.!ossep'build.conf', !global.!ossep, '/' )
!global.!targets = ''
If !global.!offline.!!last_repository Then
   Do
      /*
       * Read the config items from the "offline" file
       */
      Call Stream !global.!working_conf'.offline', 'C', 'OPEN READ'
      Do While Lines( !global.!working_conf'.offline' ) > 0
         line = Linein( !global.!working_conf'.offline' )
         If Length( Strip( line ) ) = 0 | Substr( line, 1, 1 ) = ';' Then Nop
         Else
            Do
               Parse Var line keyword '=' val
               Interpret '!GLOBAL.'keyword '=' val
            End
      End
      Call Stream !global.!working_conf'.offline', 'C', 'CLOSE'
   End
Else
   Do
      /*
       * Read the config items from the stack (if there are lines in the stack
       * or from !global.!working_conf) and write the "offline" file
       */
      Call Stream !global.!working_conf'.offline', 'C', 'OPEN WRITE REPLACE'
/*
      If Queued() = 0 Then conf_from_file = 1
      Else conf_from_file = 0
      If conf_from_file Then
*/
         Do
            If Stream( !global.!working_conf, 'C', 'QUERY EXISTS' ) = '' Then Call AbortText 'Problem accessing locally checked out build.conf file:' !global.!working_conf
            Call Stream !global.!working_conf, 'C', 'OPEN READ'
            Do While Lines( !global.!working_conf) > 0
               line = Linein( !global.!working_conf )
               If Length( Strip( line ) ) = 0 | Substr( line, 1, 1 ) = ';' Then Nop
               Else
                  Do
                     Parse Var line keyword '=' val
                     Interpret '!GLOBAL.'keyword '=' val
                     Call Lineout !global.!working_conf'.offline', line
                  End
            End
            Call Stream !global.!working_conf, 'C', 'CLOSE'
         End
/*
      Else
         Do
            Do Queued()
               Parse Pull line
               If Length( Strip( line ) ) = 0 | Substr( line, 1, 1 ) = ';' Then Nop
               Else
                  Do
                     Parse Var line keyword '=' val
                     Interpret '!GLOBAL.'keyword '=' val
                     Call Lineout !global.!working_conf'.offline', line
                  End
            End
         End
*/
      Call Stream !global.!working_conf'.offline', 'C', 'CLOSE'
   End
/*
 * One validation check to ensure we have set up the build.conf file
 * properly
 */
If Left( !global.!admin_users, 1 ) = '!' Then
   Do
      Say 'This repository has not been correctly configured. Aborted.'
      Call CleanupAndExit 1
   End

Return

/*
 * Determine what permissions the user has. We translate any users here from the "real" userid to
 * the equivalent as specified in the repository properties.
 */
SetupEnvironment: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Translate user here if appropriate
 */
!global.!user = TranslateUser( !global.!user, !global.!user_mappings )
If Wordpos( Translate( !global.!user ), Translate( !global.!admin_users ) ) = 0 Then !global.!administrator.!!last_repository='N'
Else !global.!administrator.!!last_repository = 'Y'
If Wordpos( Translate( !global.!user ), Translate( !global.!build_users ) ) = 0 Then !global.!build_user.!!last_repository='N'
Else !global.!build_user.!!last_repository = 'Y'
If Wordpos( Translate( !global.!user ), Translate( !global.!update_users ) ) = 0 Then !global.!update_user.!!last_repository='N'
Else !global.!update_user.!!last_repository = 'Y'
Return

TranslateUser: Procedure Expose !REXXDW.
Parse Arg current, mappings
If Left( mappings, 1 ) = '!' | Strip( mappings ) = '' Then Return current
Do Forever
   Parse Var mappings from '=' to ',' mappings
   If Strip( Translate( from ) ) = Strip( Translate( current ) ) Then
      Do
         new = Strip( to )
         Leave
      End
   If mappings = '' Then
      Do
         new = current
         Leave
      End
End
Return new

GetModulesFromRepository: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Say what's happening...
 */
Call SetCursorWait
If !global.!offline.!!last_repository Then Call DisplayStatus 'Reading modules from working directory...'
Else Call DisplayStatus 'Reading modules from repository...'
/*
 * Do some basic validation - this to go in a function that SetRepository()
 * calls!!!
 */
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         Call SysFileTree _root||!global.!ossep'RCS', '!GLOBAL.!REPDIR.!!LAST_REPOSITORY.', 'DTS'
         If !global.!repdir.!!last_repository.0 == 0 Then Call AbortText 'No RCS files under:' _root
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         offline_directory_file = !global.!!repository.!!working_dir.!!last_repository||'admin/offline.dir.'!global.!!repository.!!name.!!last_repository
say .line offline_directory_file
         Select
            When !global.!offline.!!last_repository Then
               Do
                  /*
                   * When working offline, get the "repository" details from
                   * the working directory's CVS/Entries files. Ensure that
                   * the CVS/Repository items match the current repository.
                   */
                  Call Stream offline_directory_file, 'C', 'OPEN READ'
                  Do i = 1 While Lines( offline_directory_file ) > 0
                     !global.!repdir.!!last_repository.i = Linein( offline_directory_file )
                  End
                  i = i - 1
                  !global.!repdir.!!last_repository.0 = i
               End
            When !global.!!repository.!!public_cvs.!!last_repository = 'Y' & Strip( !global.!!repository.!!public_cvs_dirs.!!last_repository ) \= '' Then
               Do
                  /*
                   * For each root directory, do an rdiff to build up the full tree
                   */
                  idx = 0
                  Call Stream offline_directory_file, 'C', 'OPEN WRITE REPLACE'
                  Do j = 1 To Words( !global.!!repository.!!public_cvs_dirs.!!last_repository )
                     dir = Word( !global.!!repository.!!public_cvs_dirs.!!last_repository, j )
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'rdiff -s -D 1/1/2037' dir
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address Command cmd With Output Stem out. Error Stem err.
                     Do i = 1 To err.0
                        Parse Var err.i . 'Diffing' line
                        line = '/'Strip( line )
                        If line \= '/.' & line \= '/' Then
                           Do
                              If line = '/CVSROOT' & !global.!superadministrator \= 'Y' Then Nop
                              Else
                                 Do
                                    idx = idx + 1
                                    !global.!repdir.!!last_repository.idx = line
                                    Call Lineout offline_directory_file, line
                                 End
                           End
                        Else
                           Do
                              Queue err.i
                           End
                     End
                  End
                  !global.!repdir.!!last_repository.0 = idx
                  If Queued() \= 0 Then
                     Do
                        Call WriteStackToLogWindow 'default', 'Getting modules'
                     End
                  Drop out. err.
               End
            Otherwise
               Do
                  /*
                   * Read the list of directories in the repository, ignoring .
                   * This doesn't use CVSROOT/modules!!
                   * While we are doing this, write a file into the admin directory for this
                   * repository's working directory for running in offline mode
                   */
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'rdiff -s -D 1/1/2037 .'
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address Command cmd With Output Stem out. Error FIFO ''
                  idx = 0
                  Call Stream offline_directory_file, 'C', 'OPEN WRITE REPLACE'
                  Do i = 1 To Queued()
                     Parse Pull . 'Diffing' line
                     line = '/'Strip( line )
                     If line \= '/.' & line \= '/' Then
                        Do
                           If line = '/CVSROOT' & !global.!superadministrator \= 'Y' Then Nop
                           Else
                              Do
                                 idx = idx + 1
                                 !global.!repdir.!!last_repository.idx = line
                                 Call Lineout offline_directory_file, line
                              End
                        End
                  End
                  !global.!repdir.!!last_repository.0 = idx
                  Drop out.
               End
         End
         Call Stream offline_directory_file, 'C', 'CLOSE'
         Call SysStemSort '!global.!repdir.!!last_repository.','ascending' ,!global.!!user.!!preference.!!sort, , , 1, 100
      End
   When !global.!!repository.!!type.!!last_repository = 'hg' Then
      Do
         /*
          * Read the list of files from the repository.
          * Get directories from filenames
          */
         cmd = 'hg status -qA --cwd' !global.!!repository.!!working_dir.!!last_repository
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output FIFO '' Error Stem err.
         idx = 0
         Do i = 1 To Queued()
            Parse Pull status line
            If status = '?' Then Iterate
            pos = LastPos( '/', line )
            If pos = 0 Then Iterate
            dir = '/'Left( line, pos - 1 )
            -- do we already have the directory?
            found = 0
            Do j = 1 To idx
               If !global.!repdir.!!last_repository.j = dir Then
                  Do
                     found = 1
                     Leave
                  End
            End
            If found = 0 Then
               Do
                  idx = idx + 1
                  !global.!repdir.!!last_repository.idx = dir
say .line added dir
               End
         End
         !global.!repdir.!!last_repository.0 = idx
say .line count idx
         Drop out.
         Call SysStemSort '!global.!repdir.!!last_repository.','ascending' ,!global.!!user.!!preference.!!sort, , , 1, 100
      End
   Otherwise Call AbortText 'Cannot handle repository of type:' !global.!!repository.!!type.!!last_repository
End
Call SetCursorNoWait
Call DisplayStatus ''
Return

GenerateTree: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * If the tree has already been created, destroy it first
 */
If !global.!tree.!!last_repository \= -1 Then Call dw_window_destroy( !global.!tree.!!last_repository )
!global.!tree.!!last_repository = dw_tree_new( 201 )
Call dw_box_pack_start !global.!treebox.!!last_repository, !global.!tree.!!last_repository, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 1
/*Call dw_window_set_font !global.!tree.!!last_repository, !global.!fixedfont *//* MH */
/*
 * Setup our signals. On a tree, these MUST be done before we populate the tree.
 */
Call dw_signal_connect !global.!tree.!!last_repository, !REXXDW.!DW_ITEM_SELECT_EVENT, 'DirSelectCallback'
Call dw_signal_connect !global.!tree.!!last_repository, !REXXDW.!DW_ITEM_CONTEXT_EVENT, 'DirMenuCallback'
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         /* TODO */
         drive = ''
         Do i = 1 To !global.!repdir.!!last_repository.0
            Parse Var !global.!repdir.!!last_repository.i . . . fn
            fn = Changestr(_root,fn,'') /* MH */
            If Substr(fn,2,1) = ':' Then
               Do
                  drive = Substr(fn,1,2)
                  fn = Substr(fn,3)
               End
            fn = Translate(fn,' ','/')
            dir = ''
            Do j = 1 To Words(fn)
               If Word(fn,j) \= 'RCS' Then
                  Do
                     dir = dir'/'Word(fn,j)
                     imagetype = DetermineImageType( dir, !!last_repository )
                     Call TkTreeAddNode my_treewin, dir, '-image', imagetype, '-tags', 'dir'
                  End
            End
         End
         If Left( !global.!!repository.!!last_tree_opened.!!last_repository, 9 ) = '!GLOBAL.!!' Then
            Do
               !global.!!repository.!!last_tree_opened.!!last_repository = _root
            End
         openat = !global.!!repository.!!last_tree_opened.!!last_repository
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         /*
          * Add the repository name as the root
          */
         idx = DetermineModuleIcon( )
         icon = !global.!moduleicon.idx
         !global.!treeparentitem.!!last_repository = dw_tree_insert( !global.!tree.!!last_repository, '['!global.!!repository.!!name.!!last_repository']', icon, 0, '0' )
         If Left( !global.!!repository.!!last_tree_opened.!!last_repository, 1 ) = '!' Then
            Do
               !global.!!repository.!!last_tree_opened.!!last_repository = '.'_root
            End
         openat = 0
         Do i = 1 To !global.!repdir.!!last_repository.0
            mydir = !global.!repdir.!!last_repository.i
            If Wordpos( Translate( !global.!user ), Translate( !global.!admin_users ) ) = 0 & mydir = '/admin' Then Iterate
            Parse Value DetermineModuleIconAndName( mydir ) With icon currentdir
            pos = Lastpos( '/', mydir )
            parent = Substr( mydir, 1, pos-1 )
            parentidx = FindTreeItem( parent )
            If parentidx = 0 Then parentitem = !global.!treeparentitem.!!last_repository
            Else parentitem = !global.!treeitem.!!last_repository.parentidx
            !global.!treeitem.!!last_repository.i = dw_tree_insert( !global.!tree.!!last_repository, currentdir, icon, parentitem, parentitem !global.!repdir.!!last_repository.i )
            /*
             * Is this tree item the one we last opened ?
             */
            If !global.!repdir.!!last_repository.i = '/'!global.!!repository.!!last_tree_opened.!!last_repository Then openat = i
         End
         /*
          * Expand the tree and all its parents at the last directory, if
          * there was one
          */
         If openat = 0 Then
            Do
               /*
                * Set the current_dirname
                */
               !global.!current_dirname.!!last_repository = '.'
               /*
                * Set the parent tree item as the selected item
                */
               Call dw_tree_item_select !global.!tree.!!last_repository, !global.!treeparentitem.!!last_repository
               !global.!selected_treeitem.!!last_repository = !global.!treeparentitem.!!last_repository
            End
         Else
            Do
               /*
                * Set the current_dirname
                */
               !global.!current_dirname.!!last_repository = Substr( !global.!repdir.!!last_repository.openat, 2 )
               /*
                * Open the tree at the index
                */
               Call OpenTreeAt openat
            End
      End
   -- TODO the following code is identical to the CVS code above
   When !global.!!repository.!!type.!!last_repository = 'hg' Then
      Do
         /*
          * Add the repository name as the root
          */
         idx = DetermineModuleIcon( )
         icon = !global.!moduleicon.idx
         !global.!treeparentitem.!!last_repository = dw_tree_insert( !global.!tree.!!last_repository, '['!global.!!repository.!!name.!!last_repository']', icon, 0, '0' )
         If Left( !global.!!repository.!!last_tree_opened.!!last_repository, 1 ) = '!' Then
            Do
               !global.!!repository.!!last_tree_opened.!!last_repository = '.'_root
            End
         openat = 0
         Do i = 1 To !global.!repdir.!!last_repository.0
            mydir = !global.!repdir.!!last_repository.i
            If Wordpos( Translate( !global.!user ), Translate( !global.!admin_users ) ) = 0 & mydir = '/admin' Then Iterate
            Parse Value DetermineModuleIconAndName( mydir ) With icon currentdir
            pos = Lastpos( '/', mydir )
            parent = Substr( mydir, 1, pos-1 )
            parentidx = FindTreeItem( parent )
            If parentidx = 0 Then parentitem = !global.!treeparentitem.!!last_repository
            Else parentitem = !global.!treeitem.!!last_repository.parentidx
            !global.!treeitem.!!last_repository.i = dw_tree_insert( !global.!tree.!!last_repository, currentdir, icon, parentitem, parentitem !global.!repdir.!!last_repository.i )
            /*
             * Is this tree item the one we last opened ?
             */
            If !global.!repdir.!!last_repository.i = '/'!global.!!repository.!!last_tree_opened.!!last_repository Then openat = i
         End
         /*
          * Expand the tree and all its parents at the last directory, if
          * there was one
          */
         If openat = 0 Then
            Do
               /*
                * Set the current_dirname
                */
               !global.!current_dirname.!!last_repository = '.'
               /*
                * Set the parent tree item as the selected item
                */
               Call dw_tree_item_select !global.!tree.!!last_repository, !global.!treeparentitem.!!last_repository
               !global.!selected_treeitem.!!last_repository = !global.!treeparentitem.!!last_repository
            End
         Else
            Do
               /*
                * Set the current_dirname
                */
               !global.!current_dirname.!!last_repository = Substr( !global.!repdir.!!last_repository.openat, 2 )
               /*
                * Open the tree at the index
                */
               Call OpenTreeAt openat
            End
      End
   Otherwise Nop
End

Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         /*
          * Prepend the root dir
          */
         !global.!current_dirname.!!last_repository = _root||!global.!ossep||!global.!current_dirname.!!last_repository
         !global.!current_working_dir.!!last_repository = FindWorkingDirectory( !global.!current_dirname.!!last_repository )
      End
   Otherwise
      Do
      /*
         !global.!current_working_dir.!!last_repository = FindWorkingDirectory( !global.!current_dirname.!!last_repository )
       */
      End
End
Call SetLockedStatusIcon
Return

OpenTreeAt: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg openat
parentitem = !global.!treeitem.!!last_repository.openat
/*
 * GTK 2.0 at least requires that tree items are expanded from the top down.
 * So we have to determine all of the tree items for the child and all
 * its parents, and expand them in reverse order.
 */
parentids = parentitem
Do Forever
   Parse Value dw_tree_item_get_data( !global.!tree.!!last_repository, parentitem ) With parentitem .
   If parentitem = 0 Then Leave
   parentids = parentitem parentids
End
/*
 * Open the tree items...
 */
Do i = 1 To Words( parentids )
   parentitem = Word( parentids, i )
   Call dw_tree_item_expand !global.!tree.!!last_repository, parentitem
End
/*
 * Set this tree item as the selected item
 */
Call dw_tree_item_select !global.!tree.!!last_repository, !global.!treeitem.!!last_repository.openat
!global.!selected_treeitem.!!last_repository = !global.!treeitem.!!last_repository.openat
Return

DisplayRepositoryFiles: Procedure Expose !REXXDW. !global. !!last_repository
Call DisplayStatus 'Reading files for current directory...'
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         mydir = drive||!global.!current_dirname.!!last_repository||!global.!ossep'RCS'
         here = Directory()
         Call Directory mydir
         Call SysFileTree '*,v', 'ITEM.', 'FL'
         Do i = 1 To item.0
            newi = Right(i,4)
            Parse Var item.i 1 filedate 20 . . filename ',v' .
            !global.!repfiles.?filedate.!!last_repository.i = newi||Strip( filedate ) /* TODO - convert to time_t */
            !global.!repfiles.?fullfilename.!!last_repository.i = newi||mydir||!global.!ossep||filename||',v'
            !global.!repfiles.?user.!!last_repository.i = newi||GetRCSLocker( Substr( !global.!repfiles.?fullfilename.!!last_repository.i, 5 ) )
            !global.!repfiles.?filename.!!last_repository.i = newi||filename
         End
         !global.!repfiles.?fullfilename.!!last_repository.0 = item.0
         !global.!repfiles.?filename.!!last_repository.0 = item.0
         !global.!repfiles.?filedate.!!last_repository.0 = item.0
         !global.!repfiles.?user.!!last_repository.0 = item.0
         Drop item.
         Call Directory here
         Call DetermineRCSStatusIcons
         Call SortFileArray
         Call DisplayFileArray
         !global.!!repository.!!last_tree_opened.!!last_repository = Changestr( _root||!global.!ossep, !global.!current_dirname, '' )
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         If !global.!offline.!!last_repository Then
            Do
               entries = !global.!!repository.!!working_dir.!!last_repository||!global.!current_dirname.!!last_repository||!global.!ossep'CVS/Entries'
               Call Stream entries, 'C', 'OPEN READ'
               item_idx = 0
               label_idx = 0
               Do While Lines( entries ) > 0
                  line = Linein( entries )
                  If Left( line, 1 ) \= 'D' Then
                     Do
                        Parse Var line '/' filename '/' rev '/' date '/' .
                        If Left( filename, 7 ) = '.label-' Then
                           Do
                              label_idx = label_idx + 1
                              Parse Var filename '.label-' !global.!label.label_idx
                           End
                        Else
                           Do
                              If Datatype( Word( date, 3 ) ) \= 'NUM' Then
                                 Do
                                    filedate = '*Unknown*'
                                 End
                              Else
                                 Do
                                    /*
                                     * Get date/time in Thu Aug  8 10:58:48 2002 format (localtime)
                                     * and convert to time_t
                                     */
                                    Parse Var date . mon day time year
                                    filedate = Right( DateTimeToTime_T( day mon year, time, 'N', 1 ), 12, '0' )
                                 End
                              item_idx = item_idx + 1
                              newi = Right( item_idx, 4 )
                              !global.!repfiles.?filerev.!!last_repository.item_idx = newi||rev
                              !global.!repfiles.?user.!!last_repository.item_idx = newi||'*Unknown*'
                              !global.!repfiles.?filedate.!!last_repository.item_idx = newi||filedate
                              !global.!repfiles.?filename.!!last_repository.item_idx = newi||filename
                              !global.!repfiles.?fullfilename.!!last_repository.item_idx = newi||!global.!current_dirname.!!last_repository||!global.!ossep||filename
                              !global.!repfiles.?filestatus.!!last_repository.item_idx = newi||'fileunknown'
                              type = IsBinaryFile( !global.!current_dirname.!!last_repository||!global.!ossep||filename )
                              !global.!repfiles.?filetype.!!last_repository.item_idx = newi || DetermineFileType( type, !global.!user, '' )
                              !global.!repfiles.?filereminder.!!last_repository.item_idx = newi'blank'
                           End
                     End
               End
               item.0 = item_
               !global.!label.0 = label_idx
               Call Stream entries, 'C', 'CLOSE'
            End
         Else
            Do
               /*
                * Returns all files in item. stem
                */
               Call GetListOfCVSFilesInDirectory !!last_repository, !global.!current_dirname.!!last_repository, 1
               -- log errors in red if there were any
               Call dw_mle_freeze !global.!mle.!!last_repository
               Do wtfi = 1 To err.0
                  Call WriteMessageToLogWindow '  'err.wtfi, 'redfore'
               End
               Call dw_mle_thaw !global.!mle.!!last_repository
               item_idx = 0
               label_idx = 0
               If !global.!current_dirname.!!last_repository = '.' | !global.!current_dirname.!!last_repository = '' Then fname_offset = 1
               Else fname_offset = Length(!global.!current_dirname.!!last_repository)+2
               Do i = 1 To item.0
                  If Left( item.i, 1 ) = '?' Then Iterate -- some platforms return unknown files in stdout
                  Parse Var item.i 1 . 12 rev 28 user 64 filedate 75 filetime 84 type 88 flen 91 rem
                  Parse Var item.i 92 filename +(flen) +1 locks
                  add_file = 1
                  /*
                   * If the file is a label, then add it to the list of labels...
                   */
                  If Left( Substr( filename, fname_offset ), 7 ) = '.label-' Then
                     Do
                        label_idx = label_idx + 1
                        Parse Var filename . '.label-' !global.!label.label_idx
                        /*
                         * ...and if we are a repository admin, then add the labels into the file list
                         */
                        If !global.!superadministrator = 'N' Then add_file = 0
                     End
                  If add_file Then
                     Do
                        item_idx = item_idx + 1
                        newi = Right( item_idx, 4 )
                        /*
                         * MUST set filenames before setting the remainder of the attributes
                         */
                        !global.!repfiles.?fullfilename.!!last_repository.item_idx = newi||Strip( filename )
                        !global.!repfiles.?filename.!!last_repository.item_idx = newi||Strip( Substr( filename, fname_offset ) )
                        filedate = Strip( filedate )
                        filetime = Strip( filetime )
                        /*
                         * Ensure that we don't attempt to get the CVS status for each
                         * individual file; it is too slow. We do this in bulk later...
                         */
                        Call SetCVSFileDetails 'BULK', item_idx, newi, 'arg not used', filedate, filetime, type, rev, user, locks
                        !global.!repfiles.?filestatus.!!last_repository.item_idx = newi
                     End
               End
            End
         !global.!repfiles.?filerev.!!last_repository.0 = item_idx
         !global.!repfiles.?fullfilename.!!last_repository.0 = item_idx
         !global.!repfiles.?filename.!!last_repository.0 = item_idx
         !global.!repfiles.?filedate.!!last_repository.0 = item_idx
         !global.!repfiles.?user.!!last_repository.0 = item_idx
         !global.!repfiles.?filestatus.!!last_repository.0 = item_idx
         !global.!repfiles.?filereminder.!!last_repository.0 = item_idx
         !global.!repfiles.?filetype.!!last_repository.0 = item_idx
         !global.!label.0 = label_idx
         Drop item.
         /*
          * We now need to determine the CVS status of all files in the current
          * directory, because the above call to SetCVSFileDetails() does NOT get
          * the status for each file as it has to do this with a server call for
          * each file and this can be very slow!
          */
         Call DetermineCVSStatusIcons
         Call SortFileArray
         Call DisplayFileArray
         !global.!!repository.!!last_tree_opened.!!last_repository = !global.!current_dirname.!!last_repository
      End
   When !global.!!repository.!!type.!!last_repository = 'hg' Then
      Do
         /*
          * Returns all files in item. stem
          */
         Call GetListOfHGFilesInDirectory !!last_repository, !global.!current_dirname.!!last_repository, 1
         -- log errors in red if there were any
         Call dw_mle_freeze !global.!mle.!!last_repository
         Do wtfi = 1 To err.0
            Call WriteMessageToLogWindow '  'err.wtfi, 'redfore'
         End
         Call dw_mle_thaw !global.!mle.!!last_repository
         item_idx = 0
         label_idx = 0

         If !global.!current_dirname.!!last_repository = '.' | !global.!current_dirname.!!last_repository = '' Then fname_offset = 1
         Else fname_offset = Length(!global.!current_dirname.!!last_repository)+2
         Do i = 1 To item.0
            locks = ''
            Parse Var item.i filename '%rev%' rev '%status%' status '%date%' date offset '%author%' user '%filetype%' filetype '%end%'
            add_file = 1
            /*
             * If the file is a label, then add it to the list of labels...
             */
            If Left( Substr( filename, fname_offset ), 7 ) = '.label-' Then
               Do
                  label_idx = label_idx + 1
                  Parse Var filename . '.label-' !global.!label.label_idx
                  /*
                   * ...and if we are a repository admin, then add the labels into the file list
                   */
                  If !global.!superadministrator = 'N' Then add_file = 0
               End
            If add_file Then
               Do
say .line filename rev status date offset user filetype
                  filedate = date + offset
                  item_idx = item_idx + 1
                  newi = Right( item_idx, 4 )
                  /*
                   * MUST set filenames before setting the remainder of the attributes
                   */
                  !global.!repfiles.?fullfilename.!!last_repository.item_idx = newi||Strip( filename )
                  !global.!repfiles.?filename.!!last_repository.item_idx = newi||Strip( Substr( filename, fname_offset ) )
                  filedate = Strip( filedate )
                  filetime = Strip( filetime )
                  filetype = Strip( filetype )
                  /*
                   * TODO how to get filestatus; ie is it binary or text????
                   */
                  Call SetHGFileDetails 'BULK', item_idx, newi, 'arg not used', filedate, filetime, filetype, status, rev, user, locks
--                  !global.!repfiles.?filestatus.!!last_repository.item_idx = newi
               End
         End
say .line 'item_idx:' item_idx
         !global.!repfiles.?filerev.!!last_repository.0 = item_idx
         !global.!repfiles.?fullfilename.!!last_repository.0 = item_idx
         !global.!repfiles.?filename.!!last_repository.0 = item_idx
         !global.!repfiles.?filedate.!!last_repository.0 = item_idx
         !global.!repfiles.?user.!!last_repository.0 = item_idx
         !global.!repfiles.?filestatus.!!last_repository.0 = item_idx
         !global.!repfiles.?filereminder.!!last_repository.0 = item_idx
         !global.!repfiles.?filetype.!!last_repository.0 = item_idx
         !global.!label.0 = label_idx
         Drop item.
         /*
          * We now need to determine the HG status of all files in the current
          * directory, because the above call to SetHGFileDetails() does NOT get
          * the status for each file as it has to do this with a server call for
          * each file and this can be very slow!
          */
--       Call DetermineHGStatusIcons
         Call SortFileArray
         Call DisplayFileArray
         !global.!!repository.!!last_tree_opened.!!last_repository = !global.!current_dirname.!!last_repository
      End
   Otherwise Nop
End
Call DisplayStatus ''
Return 0

DisplayNonrepositoryFiles: Procedure Expose !REXXDW. !global. !!last_repository
/*
If !global.!current_dirname.!!last_repository = '' Then repdir = '.'
Else repdir = !global.!current_dirname.!!last_repository
*/
repdir = Stream( Strip( !global.!!repository.!!working_dir.!!last_repository, 'T', '/' ), 'C', 'QUERY EXISTS' ) || !global.!ossep
lenwd = Length( repdir )
If !global.!current_dirname.!!last_repository = '.' Then repdir = Left( repdir, Length( repdir ) - 1 )
Else repdir = repdir || !global.!current_dirname.!!last_repository
lenrepdir = Length( repdir )
if trace() = 'F' Then say repdir
If Stream( repdir, 'C','QUERY EXISTS' ) = '' Then newfile.0 = 0
Else Call SysFileTree repdir'/*', 'NEWFILE.', 'DL'
/*
 * Limit the list to those directories that are NOT already in the repository...
 */
idx = 0
Do i = 1 To newfile.0
   Parse Var newfile.i date time size perm dir
   /*
    * Determine if "dir" is in the repository
    */
   If Countstr( !global.!ossep, dir ) \= 0 Then dir = Translate( dir, '/', !global.!ossep )
   If FindTreeItem( '/'Substr( dir, 1 + lenwd ) ) \= 0 Then Iterate

   dirname = Substr( dir, 2 + lenrepdir )
   /*
    * For CVS ignore the CVS directory
    */
   If !global.!!repository.!!type.!!last_repository = 'cvs' & dirname = 'CVS' Then Iterate
   idx = idx + 1
   fidx = Right( idx, 4 )
   !global.!nonrepfiles.?fullfilename.!!last_repository.idx = fidx||dir
   !global.!nonrepfiles.?filename.!!last_repository.idx = fidx||dirname
   date = Changestr( '-', date, '' ) /* date now in 'S' format */
   date = Right( DateTimeToTime_T( date, time, 'S', 0 ), 12, '0' )
   !global.!nonrepfiles.?filedate.!!last_repository.idx = fidx||date
   !global.!nonrepfiles.?filesize.!!last_repository.idx = fidx||Right( size, 12 )
   !global.!nonrepfiles.?fileperm.!!last_repository.idx = fidx||perm
   !global.!nonrepfiles.?filetype.!!last_repository.idx = fidx||'dir'
End
/*
 * List files...
 */
If Stream( repdir, 'C','QUERY EXISTS' ) = '' Then newfile.0 = 0
Else Call SysFileTree repdir'/*', 'NEWFILE.', 'FL'
Do i = 1 To newfile.0
   Parse Var newfile.i date time size perm file
   /*
    * Determine if "file" is in the repository
    */
   If Countstr( !global.!ossep, file ) \= 0 Then file = Translate( file, '/', !global.!ossep )
   If FileIsInRepository( Substr( file, 1 + lenwd ) ) Then Iterate

   filename = Substr( file, 2 + lenrepdir )
   /*
    * Check if the file is a regular file
    */
   If Length( perm ) = 10 & Left( perm, 1 ) \= '-' Then Iterate
   /*
   If Word( Stream( file, 'C', 'FSTAT' ), 8 ) \= 'RegularFile' Then Iterate
   */
   /*
    * Now ignore some other files...
    */
   /*
    * Ignore files in the !ignorefiles list
    */
   If IsFileTypeIgnored( file ) Then Iterate
   If Left( filename, 7 ) = '.label-' Then Iterate
   If Left( file, 16 ) = 'admin/build.conf' Then Iterate
   If Left( file, 24 ) = 'admin/build.conf.offline' Then Iterate
   If Left( file, 17 ) = 'admin/offline.dir' Then Iterate
   /*
    * Now add the files...
    */
   idx = idx + 1
   fidx = Right( idx, 4 )
   !global.!nonrepfiles.?fullfilename.!!last_repository.idx = fidx||file
   !global.!nonrepfiles.?filename.!!last_repository.idx = fidx||filename
   date = Changestr( '-', date, '' ) /* date now in 'S' format */
   date = Right( DateTimeToTime_T( date, time, 'S', 0 ), 12, '0' )
   !global.!nonrepfiles.?filedate.!!last_repository.idx = fidx||date
   !global.!nonrepfiles.?filesize.!!last_repository.idx = fidx||Right( size, 12 )
   !global.!nonrepfiles.?fileperm.!!last_repository.idx = fidx||perm
   !global.!nonrepfiles.?filetype.!!last_repository.idx = fidx||'file'
   If IsBinaryFile( file ) Then !global.!nonrepfiles.?filetype.!!last_repository.idx = fidx||'binary file'
   Else !global.!nonrepfiles.?filetype.!!last_repository.idx = fidx||'text file'
End
!global.!nonrepfiles.?fullfilename.!!last_repository.0 = idx
!global.!nonrepfiles.?filename.!!last_repository.0 = idx
!global.!nonrepfiles.?filedate.!!last_repository.0 = idx
!global.!nonrepfiles.?filesize.!!last_repository.0 = idx
!global.!nonrepfiles.?fileperm.!!last_repository.0 = idx
!global.!nonrepfiles.?filetype.!!last_repository.0 = idx
/*
 * Now sort an display the list of non-rep files.
 */
Call SortNonrepFileArray
Call DisplayNonRepFileArray
Return

/*
 * Populates the "All Files" tab based on the currently selected filter
 */
DisplayAllFiles: Procedure Expose !REXXDW. !global. !!last_repository
Call SetCursorWait
Call DisplayStatus 'Reading files for current directory and below...'
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         mydir = drive||!global.!current_dirname.!!last_repository||!global.!ossep'RCS'
         here = Directory()
         Call Directory mydir
         Call SysFileTree '*,v', 'ITEM.', 'FLS'
         Do i = 1 To item.0
            newi = Right(i,4)
            Parse Var item.i 1 filedate 20 . . filename ',v' .
            !global.!allfiles.?filedate.!!last_repository.i = newi||Strip( filedate ) /* TODO - convert to time_t */
            !global.!allfiles.?fullfilename.!!last_repository.i = newi||mydir||!global.!ossep||filename||',v'
            !global.!allfiles.?user.!!last_repository.i = newi||GetRCSLocker( Substr( !global.!allfiles.?fullfilename.!!last_repository.i, 5 ) )
            pos = Lastpos( '/', filename )
            !global.!allfiles.?filename.!!last_repository.i = newi||Substr( filename, pos + 1 )
            If pos = 0 Then
               !global.!allfiles.?filepath.!!last_repository.i = newi
            Else
               !global.!allfiles.?filepath.!!last_repository.i = newi||Left( filename, pos )
         End
         !global.!allfiles.?fullfilename.!!last_repository.0 = item.0
         !global.!allfiles.?filename.!!last_repository.0 = item.0
         !global.!allfiles.?filepath.!!last_repository.0 = item.0
         !global.!allfiles.?filedate.!!last_repository.0 = item.0
         !global.!allfiles.?user.!!last_repository.0 = item.0
         Drop item.
         Call Directory here
         Call DetermineRCSStatusIcons
         Call SortFileArray
         Call DisplayFileArray
         !global.!!repository.!!last_tree_opened.!!last_repository = Changestr( _root||!global.!ossep, !global.!current_dirname, '' )
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         If !global.!offline.!!last_repository Then
            Do
            /* TODO */
               entries = !global.!!repository.!!working_dir.!!last_repository||!global.!current_dirname.!!last_repository||!global.!ossep'CVS/Entries'
               Call Stream entries, 'C', 'OPEN READ'
               item_idx = 0
               label_idx = 0
               Do While Lines( entries ) > 0
                  line = Linein( entries )
                  If Left( line, 1 ) \= 'D' Then
                     Do
                        Parse Var line '/' filename '/' rev '/' date '/' .
                        If Left( filename, 7 ) = '.label-' Then
                           Do
                              label_idx = label_idx + 1
                              Parse Var filename '.label-' !global.!label.label_idx
                           End
                        Else
                           Do
                              If Datatype( Word( date, 3 ) ) \= 'NUM' Then
                                 Do
                                    filedate = '*Unknown*'
                                 End
                              Else
                                 Do
                                    /*
                                     * Get date/time in Thu Aug  8 10:58:48 2002 format (localtime)
                                     * and convert to time_t
                                     */
                                    Parse Var date . mon day time year
                                    filedate = Right( DateTimeToTime_T( day mon year, time, 'N', 1 ), 12, '0' )
                                 End
                              item_idx = item_idx + 1
                              newi = Right( item_idx, 4 )
                              !global.!allfiles.?filerev.!!last_repository.item_idx = newi||rev
                              !global.!allfiles.?user.!!last_repository.item_idx = newi||'*Unknown*'
                              !global.!allfiles.?filedate.!!last_repository.item_idx = newi||filedate
                              pos = Lastpos( '/', filename )
                              !global.!allfiles.?filename.!!last_repository.item_idx = newi||Substr( filename, pos + 1 )
                              If pos = 0 Then
                                 !global.!allfiles.?filepath.!!last_repository.item_idx = newi
                              Else
                                 !global.!allfiles.?filepath.!!last_repository.item_idx = newi||Left( filename, pos )
                              !global.!allfiles.?fullfilename.!!last_repository.item_idx = newi||!global.!current_dirname.!!last_repository||!global.!ossep||filename
                              !global.!allfiles.?filestatus.!!last_repository.item_idx = newi||'fileunknown'
                              type = IsBinaryFile( !global.!current_dirname.!!last_repository||!global.!ossep||filename )
                              !global.!allfiles.?filetype.!!last_repository.item_idx = newi || DetermineFileType( type, !global.!user, '' )
                              !global.!allfiles.?filereminder.!!last_repository.item_idx = newi'blank'
                           End
                     End
               End
               item.0 = item_
               !global.!label.0 = label_idx
               Call Stream entries, 'C', 'CLOSE'
            End
         Else
            Do
               /*
                * Returns all files in item. stem
                */
               Call GetListOfCVSFilesInDirectory !!last_repository, !global.!current_dirname.!!last_repository, 0
               item_idx = 0
               label_idx = 0
               If !global.!current_dirname.!!last_repository = '.' | !global.!current_dirname.!!last_repository = '' Then fname_offset = 1
               Else fname_offset = Length(!global.!current_dirname.!!last_repository)+2
               Do i = 1 To item.0
                  Parse Var item.i 1 . 12 rev 28 user 64 filedate 75 filetime 84 type 88 flen 91 rem
                  Parse Var item.i 92 filename +(flen) +1 locks
                  add_file = 1
                  /*
                   * If the file is a label, then add it to the list of labels...
                   */
                  If Left( Substr( filename, fname_offset ), 7 ) = '.label-' Then
                     Do
                        label_idx = label_idx + 1
                        Parse Var filename . '.label-' !global.!label.label_idx
                        /*
                         * ...and if we are a repository admin, then add the labels into the file list
                         */
                        If !global.!superadministrator = 'N' Then add_file = 0
                     End
                  /*
                   * If filename starts with "CVSROOT/" and NOT administrator, then
                   * don't add the file
                   * Same for "admin/"
                   */
                  If Left( filename, 8) = 'CVSROOT/' & !global.!superadministrator \= 'Y' Then add_file = 0
                  If Wordpos( Translate( !global.!user ), Translate( !global.!admin_users ) ) = 0 & Left( filename, 6) = 'admin/' Then add_file = 0
                  If add_file Then
                     Do
                        item_idx = item_idx + 1
                        newi = Right( item_idx, 4 )
                        /*
                         * MUST set filenames before setting the remainder of the attributes
                         */
                        !global.!allfiles.?fullfilename.!!last_repository.item_idx = newi||Strip( filename )
                        _fn = Strip( Substr( filename, fname_offset ) )
                        pos = Lastpos( '/', _fn )
                        !global.!allfiles.?filename.!!last_repository.item_idx = newi||Substr( _fn, pos + 1 )
                        If pos = 0 Then
                           !global.!allfiles.?filepath.!!last_repository.item_idx = newi
                        Else
                           !global.!allfiles.?filepath.!!last_repository.item_idx = newi||Left( _fn, pos )
                        filedate = Strip( filedate )
                        filetime = Strip( filetime )
                        /*
                         * Ensure that we don't attempt to get the CVS status for each
                         * individual file; it is too slow. We do this in bulk later...
                         */
                        Call SetCVSAllFileDetails 'BULK', item_idx, newi, 'arg not used', filedate, filetime, type, rev, user, locks
                     End
               End
            End
         !global.!allfiles.?filerev.!!last_repository.0 = item_idx
         !global.!allfiles.?fullfilename.!!last_repository.0 = item_idx
         !global.!allfiles.?filename.!!last_repository.0 = item_idx
         !global.!allfiles.?filepath.!!last_repository.0 = item_idx
         !global.!allfiles.?filedate.!!last_repository.0 = item_idx
         !global.!allfiles.?user.!!last_repository.0 = item_idx
         !global.!allfiles.?filestatus.!!last_repository.0 = item_idx
         !global.!allfiles.?filereminder.!!last_repository.0 = item_idx
         !global.!allfiles.?filetype.!!last_repository.0 = item_idx
         !global.!label.0 = label_idx
         Drop item.
         /*
          * We now need to determine the CVS status of all files in the current
          * directory, because the above call to SetCVSFileDetails() does NOT get
          * the status for each file as it has to do this with a server call for
          * each file and this can be very slow!
          */
         Call DetermineCVSStatusIconsAll
         /*
          * Now we have all of the CVS statii for each file, go and filter them based on the file filters
          * for the current repository.
          */
         Call ApplyFileFiltering
         Call SortAllFileArray
         Call DisplayAllFileArray
         !global.!!repository.!!last_tree_opened.!!last_repository = !global.!current_dirname.!!last_repository
      End
   Otherwise Nop
End
Call ShowFileFilters
Call SetCursorNoWait
Call DisplayStatus ''
Return 0

DisplayNonRepFileArray: Procedure Expose !REXXDW. !global. !!last_repository
moduleicon_idx = Wordpos( 'modulecheckedout', !global.!moduleicons )
binaryfileicon_idx = Wordpos( 'binary', !global.!fileicons )
textfileicon_idx = Wordpos( 'text', !global.!fileicons )
/*
 * Delete everything in the container so we can replace the existing files
 */
Call dw_signal_disconnect !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_SELECT_EVENT
Call dw_signal_disconnect !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_ENTER_EVENT
Call dw_signal_disconnect !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_CONTEXT_EVENT
Call dw_signal_disconnect !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_COLUMN_CLICK_EVENT
Call dw_container_clear !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_DONT_REDRAW
/*
 * Add the files into the container
 */
!global.!nonrepfilescontainermemory.!!last_repository = dw_container_alloc( !global.!nonrepfilescontainer.!!last_repository, !global.!nonrepfiles.?filename.!!last_repository.0 )
Do i = 1 To !global.!nonrepfiles.?filename.!!last_repository.0
   Select
      When Substr( !global.!nonrepfiles.?filetype.!!last_repository.i, 5 ) = 'text file' Then icon = !global.!fileicon.textfileicon_idx
      When Substr( !global.!nonrepfiles.?filetype.!!last_repository.i, 5 ) = 'binary file' Then icon = !global.!fileicon.binaryfileicon_idx
      Otherwise icon = !global.!moduleicon.moduleicon_idx
   End
   date = Time_tDateToDisplayDate( Strip( Substr( !global.!nonrepfiles.?filedate.!!last_repository.i, 5 ) ) )
   If Length( Word( date, 1 ) ) = 1 Then date = ' 'date
   Call dw_filesystem_set_file !global.!nonrepfilescontainer.!!last_repository, !global.!nonrepfilescontainermemory.!!last_repository, i-1, Substr( !global.!nonrepfiles.?filename.!!last_repository.i, 5 ), icon
   Call dw_filesystem_set_item !global.!nonrepfilescontainer.!!last_repository, !global.!nonrepfilescontainermemory.!!last_repository, 0, i-1, date
   Call dw_filesystem_set_item !global.!nonrepfilescontainer.!!last_repository, !global.!nonrepfilescontainermemory.!!last_repository, 1, i-1, Strip( Substr( !global.!nonrepfiles.?filesize.!!last_repository.i, 5 ) )
   Call dw_filesystem_set_item !global.!nonrepfilescontainer.!!last_repository, !global.!nonrepfilescontainermemory.!!last_repository, 2, i-1, Substr( !global.!nonrepfiles.?fileperm.!!last_repository.i, 5 )
   Call dw_container_set_row_title !global.!nonrepfilescontainermemory.!!last_repository, i-1, i
End
Call dw_container_set_stripe !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_CLR_WHITE, !global.!container_colour
Call dw_container_insert !global.!nonrepfilescontainer.!!last_repository, !global.!nonrepfilescontainermemory.!!last_repository, !global.!nonrepfiles.?filename.!!last_repository.0
Call dw_container_optimize !global.!nonrepfilescontainer.!!last_repository
/*
 * Only now can we setup our signals
 */
Call dw_signal_connect !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_SELECT_EVENT, 'FileSelectCallback'
Call dw_signal_connect !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_ENTER_EVENT, 'FileDoubleClickCallback'
Call dw_signal_connect !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_CONTEXT_EVENT, 'FileMenuCallback'
Call dw_signal_connect !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_COLUMN_CLICK_EVENT, 'FileSortCallback'
Return

/*
 * Populate the deleted file tab
 */
DisplayDeletedFiles: Procedure Expose !REXXDW. !global. !!last_repository
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         If !global.!offline.!!last_repository Then
            Do
            /* what to do if offline ? */
               item_idx = 0
            End
         Else
            Do
               /*
                * Just get name, version and type initially
                */
               If !global.!current_dirname.!!last_repository = '' Then dir = 'Attic'
               Else dir = !global.!current_dirname.!!last_repository'/Attic'
               If !global.!!repository.!!enhanced_cvs.!!last_repository = 'Y' Then cvscmd = 'rlog'
               Else cvscmd = 'log'
               cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository cvscmd '-hbN' quote( dir )
               If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
               Address Command cmd With Output Stem log. Error Stem err.
               item_idx = 0
               Do i = 1 To log.0
                  Select
                     When Left( log.i, 13 ) = 'Working file:' Then filename = Subword( log.i, 3 )
                     When Left( log.i, 5 )  = 'head:' Then rev = Word( log.i, 2 )
                     When Left( log.i, 21 ) = 'keyword substitution:' Then type = Word( log.i, 3 )
                     When Left( log.i, 10 ) = Copies( '=', 10 ) Then
                        Do
                           item_idx = item_idx + 1
                           item.item_idx = Left( 'Exp', 11 ) || Left( rev, 16 ) || Left( 'unknown user', 36 ) || '2000/01/01 00:00:00 ' || Left( type, 4 ) || Right( Length( filename ), 3 ) filename
                           newi = Right( item_idx, 4 )
                           !global.!deletedfiles.?filerev.!!last_repository.item_idx = newi||Strip( rev )
                           !global.!deletedfiles.?filename.!!last_repository.item_idx = newi||Strip( filename )
                           !global.!deletedfiles.?fullfilename.!!last_repository.item_idx = newi||!global.!current_dirname.!!last_repository'/'Strip( filename )
                        End
                     Otherwise Nop
                  End
               End
               Drop log.
            End
         !global.!deletedfiles.?filerev.!!last_repository.0 = item_idx
         !global.!deletedfiles.?fullfilename.!!last_repository.0 = item_idx
         !global.!deletedfiles.?filename.!!last_repository.0 = item_idx
      End
   Otherwise Nop
End
/*
 * Now display the list.
 */

/*
 * Delete everything in the container so we can replace the existing files
 */
Call dw_signal_disconnect !global.!deletedfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_SELECT_EVENT
Call dw_signal_disconnect !global.!deletedfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_CONTEXT_EVENT
Call dw_signal_disconnect !global.!deletedfilescontainer.!!last_repository, !REXXDW.!DW_COLUMN_CLICK_EVENT
Call dw_container_clear !global.!deletedfilescontainer.!!last_repository, !REXXDW.!DW_DONT_REDRAW
/*
 * Add the files into the container
 */
moduleicon_idx = Wordpos( 'deletefromrepository', !global.!moduleicons )
icon = !global.!moduleicon.moduleicon_idx
!global.!deletedfilescontainermemory.!!last_repository = dw_container_alloc( !global.!deletedfilescontainer.!!last_repository, !global.!deletedfiles.?filename.!!last_repository.0 )
Do i = 1 To !global.!deletedfiles.?filename.!!last_repository.0
   Call dw_filesystem_set_file !global.!deletedfilescontainer.!!last_repository, !global.!deletedfilescontainermemory.!!last_repository, i-1, Substr( !global.!deletedfiles.?filename.!!last_repository.i, 5 ), icon
   Call dw_container_set_row_title !global.!deletedfilescontainermemory.!!last_repository, i-1, i
   Call dw_filesystem_set_item !global.!deletedfilescontainer.!!last_repository, !global.!deletedfilescontainermemory.!!last_repository, 0, i-1, Substr( !global.!deletedfiles.?filerev.!!last_repository.i, 5 )
End
Call dw_container_set_stripe !global.!deletedfilescontainer.!!last_repository, !REXXDW.!DW_CLR_WHITE, !global.!container_colour
Call dw_container_insert !global.!deletedfilescontainer.!!last_repository, !global.!deletedfilescontainermemory.!!last_repository, !global.!deletedfiles.?filename.!!last_repository.0
Call dw_container_optimize !global.!deletedfilescontainer.!!last_repository
/*
 * Only now can we setup our signals
 */
Call dw_signal_connect !global.!deletedfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_SELECT_EVENT, 'FileSelectCallback'
Call dw_signal_connect !global.!deletedfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_CONTEXT_EVENT, 'FileMenuCallback'
Call dw_signal_connect !global.!deletedfilescontainer.!!last_repository, !REXXDW.!DW_COLUMN_CLICK_EVENT, 'FileSortCallback'
Return 0

/*
 * Gets a list of all files in a CVS directory; one file per line in a format
 * we can parse. Works on current repository, and current directory only
 * Ignore files in CVSROOT if NOT administrator
 */
GetListOfCVSFilesInDirectory: Procedure Expose !REXXDW. !global. !!last_repository item. err.
Parse Arg repno, dirname, local
If \local Then lenroot = Length( !global.!!repository.!!shortpath.repno )
Select
   When !global.!!repository.!!enhanced_cvs.repno \= 'Y' Then
      Do
         /*
          * We are using "unenhanced" CVS ie no "list" command
          * TODO
          */
/*
         If !global.!current_dirname.!!last_repository = '' Then dir = '.'
         Else dir = !global.!current_dirname.!!last_repository
*/
         If dirname = '.' Then dir = ''
         Else dir = quote( dirname )
         /*
          * Get the header information and latest revision from the repository
          * This gets all files in the current directory and below.
          */
         cmd = 'cvs' !global.!!repository.!!cvs_compression.repno '-d'!global.!!repository.!!path.repno '-q log -bN -d 1/1/2037' dir
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output FIFO '' Error Stem err.
         rcode = rc
         item_idx = 0
         /*
          * Force the number of dir levels to be more than we can ever have
          * working on local files only
          */
         If local Then dir_levels = 999999
         Else dir_levels = 999999
         Do i = 1 To Queued()
            Parse Pull log
            Select
               When Left( log, 9 ) = 'RCS file:' Then
                  Do
                     in_rev = 0
                     filename = Subword( log, 3 )
                     num_slashes = Countstr( '/', filename )
                     If num_slashes > dir_levels Then
                        Do
                           ignore_file = 1
                           Iterate i
                        End
                     len = Length( filename )
                     /*
                      * If the file is in the Attic, ignore this as well
                      */
                     lastslashpos = Lastpos( '/', filename )
                     tmpf = Substr( filename, 1, lastslashpos )
                     If Right( tmpf, 6 ) = 'Attic/' Then
                        Do
                           ignore_file = 1
                           Iterate i
                        End
                     ignore_file = 0
                     If local Then dir_levels = num_slashes
                     filename = Left( filename, len - 2 )
                     If local Then
                        Do
                           /*
                            * For local listing, only get the filename...
                            */
                           If dirname = '.' Then
                              filename = Substr( filename, lastslashpos + 1 )
                           Else
                              filename = dirname||Substr( filename, lastslashpos )
                        End
                     Else
                        Do
                          /*
                           * For recursive listing, get the path (minus root) and filename...
                           */
                           filename = Substr( filename, lenroot+2 )
                        End
                  End
               When Left( log, 21 ) = 'keyword substitution:' & ignore_file = 0 Then type = Word( log, 3 )
               When Left( log, 20 ) = Copies( '-', 20 ) & ignore_file = 0 Then
                  Do
                     in_rev = 1
                  End
               When Left( log, 9 ) = 'revision ' & in_rev = 1 Then
                  Do
                     Parse Var log 'revision' rev
                  End
               When Left( log, 6 ) = 'date: ' & in_rev = 1 Then
                  Do
                     Parse Var log 'date: ' date time ';' 'author: ' author ';' .
                     in_rev = 0
                  End
               When Left( log, 10 ) = Copies( '=', 10 ) & ignore_file = 0 Then
                  Do
                     item_idx = item_idx + 1
                     /* still need to add locks TODO */
                     item.item_idx = Left( 'Exp', 11 ) || Left( rev, 16 ) || Left( author, 36 ) || Strip( date ) Strip( time ) Left( type, 4 ) || Right( Length( filename ), 3 ) filename
                  End
               Otherwise Nop
            End
         End
         item.0 = item_idx
      End
   Otherwise
      Do
         If local Then local_flag = '-l'
         Else local_flag = ''
         cmd = 'cvs' !global.!!repository.!!cvs_compression.repno '-d'!global.!!repository.!!path.repno '-q list' local_flag quote( dirname )
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output Stem item. Error Stem err.
         rcode = rc
      End
End
Return rcode

/*
 * Gets a list of all files in a HG repository; one file per line in a format
 * we can parse. Works on current repository.
 */
GetListOfHGFilesInDirectory: Procedure Expose !REXXDW. !global. !!last_repository item. err.
Parse Arg repno, dirname, local
-- get the list of repository files; it will always return all files recursively
cwd = '--cwd' !global.!!repository.!!working_dir.!!last_repository
cmd = 'hg status -qA' cwd
If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
Address Command cmd With Output FIFO '' Error Stem err.
If rc \= 0 Then Return rc
say .line queued() dirname
-- determine file types for all files
If local = 1 Then glob = '*'
Else glob = '**'
bf = 'binary file'
cmd = "hg annotate -f 'glob:"glob"'" cwd||dirname "| grep '"bf"'"
If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
Address Command cmd With Output Stem out. Error Stem err.
idx = 0
annotate.0 = 0
Do i = 1 to out.0
   Parse var out.i fn ':' text
   If text = bf Then
      Do
         idx = idx + 1
         annotate.idx = fn
         annotate.0 = idx
      End
end
-- don't check for rc \= 0 because if there are no binary file you will get a rc = 1
say .line annotate.0
-- for each file get the log details
say .line !global.!!repository.!!working_dir.!!last_repository dirname local
item_idx = 0
If dirname = '.' Then dirname = ''
Else dirname = dirname'/'
lendir = Length( dirname )
Do Queued()
   add_file = 0
   Parse Pull status fn
   If local = 1 Then
      Do
         -- limit files to current directory
         If dirname = '' Then
            Do
               If Countstr( '/', fn ) = 0 Then add_file = 1
            End
         Else
            Do
               Parse Var fn mydir +(lendir) myfn
               If mydir = dirname & Countstr( '/', myfn ) = 0 Then add_file = 1
            End
      End
   Else
      Do
say .line TODO
      End
   If add_file Then
      Do
         cmd = "hg log -l1 --template '%rev% {rev} %status%" status " %date% {date|hgdate} %author% {author} %filetype%\\n'" cwd fn
         -- suppress logging these commands; it happens for each file so can take a while
--         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output Stem out. Error Stem err.
         If rc \= 0 Then Return rc
         -- now find out if the current file is a binary file
         -- This is not foolproof. If a text file has a line with 'binary file' then it will
         -- match as a binary file
         filetype = 't'
         Do i = 1 To annotate.0
            If annotate.i = fn Then
               Do
                  filetype = 'b'
                  Leave i
               End
         End
         item_idx = item_idx + 1
         item.item_idx = fn out.1 filetype '%end%'
         item.0 = item_idx
      End
End
Return rcode

QuoteAndBlankBang: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg str
If Left( str, 1 ) = '!' Then str = ''
Return Quote( str )

Quote: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg str
If Countstr( '"', str ) = 0 Then Return '"'str'"'
Else Return "'"str"'"

EscapeQuotes: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg str
Return Changestr( '"', str, '\"' )

CleanupAndExit: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg exit_code
Call DeleteTempFiles
If RxFuncQuery( 'DW_DropFuncs' ) = 0 Then Call dw_DropFuncs
/*
 * Cleanup OLE
 */
If !global.!loadedw32util = 1 Then
   Do
      Call w32olecleanup
      Call w32dropfuncs
   End
Exit exit_code

DeleteTempFiles: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Remove temporary files
 */
Do i = 1 To !global.!tempfiles.0
   Call SysFileDelete !global.!tempfiles.i
End
Return

Syntax:
Say 'Syntax error at line:' sigl 'in directory:' Directory()
Say '>>>' Sourceline( sigl )
Say Errortext( rc )
If Countstr( '.', Condition( 'E' ) ) \= 0 Then Say Condition( 'D' )
Call DeleteTempFiles
If RxFuncQuery( 'DW_DropFuncs' ) = 0 Then Call dw_DropFuncs
/*
 * Cleanup OLE
 */
If !global.!loadedw32util = 1 Then
   Do
      Call w32olecleanup
      Call w32dropfuncs
   End
Exit 1

GenerateIcons: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * The icon for QOCCA
 */
!global.!qoccaicon = dw_icon_load_from_file( !global.!icondir'qocca' )
If !global.!qoccaicon = 0 Then Call AbortText 'Could not find icon:' quote( 'qocca' ) 'in' !global.!icondir
/*
 * ... and other miscellaneous Icons
 */
!global.!noicon = dw_icon_load_from_file( !global.!icondir'no' )
If !global.!noicon = 0 Then Call AbortText 'Could not find icon:' quote( 'no' ) 'in' !global.!icondir
!global.!okicon = dw_icon_load_from_file( !global.!icondir'ok' )
If !global.!okicon = 0 Then Call AbortText 'Could not find icon:' quote( 'ok' ) 'in' !global.!icondir
/*
 * The icons for modules...
 */
!global.!moduleicons = 'modulenotcheckedout modulebuildablenotcheckedout modulecheckedout modulebuildablecheckedout modulelinkednotcheckedout modulelinkedbuildablenotcheckedout modulelinkedcheckedout modulelinkedbuildablecheckedout deletefromrepository'
Do i = 1 To Words( !global.!moduleicons )
   modname = Word( !global.!moduleicons, i )
   !global.!moduleicon.i = dw_icon_load_from_file( !global.!icondir||modname)
   If !global.!moduleicon.i = 0 Then Call AbortText 'Could not find icon:' quote( modname ) 'in' !global.!icondir
End
/*
 * Icons for file status - these may be icons or text depending of capabilities of GUI containers
 */
!global.!statusicons = 'fileconflict filemodified fileneedscheckout fileneedsmerge fileneedspatch fileuptodate fileunknown setreminder blank deletefromrepository'
statustext = 'Conflict Locally.Modified Needs.Checkout Needs.Merge Needs.Patch Up-to-date *Unknown* Reminder No.Lock Delete'
Do i = 1 To Words( !global.!statusicons )
   modname = Word( !global.!statusicons, i )
   If !global.!container_disallows_icons = 1 Then
      Do
         !global.!statusicon.i = Changestr( '.', Word( statustext,i ), ' ' )
      End
   Else
      Do
         !global.!statusicon.i = dw_icon_load_from_file( !global.!icondir||modname )
      End
End
/*
 * Icons for file type
 */
!global.!fileicons = 'binary binarylocked binarylockedother text textlocked textlockedother fileunknown'
Do i = 1 To Words( !global.!fileicons )
   modname = Word( !global.!fileicons, i )
   !global.!fileicon.i = dw_icon_load_from_file( !global.!icondir||modname )
   If !global.!fileicon.i = 0 Then Call AbortText 'Could not find icon:' quote( modname ) 'in' !global.!icondir
End
/*
 * Icons for Job Status
 */
!global.!jobicons = 'build make copy mods promote'
Do i = 1 To Words( !global.!jobicons )
   modname = Word( !global.!jobicons, i )
   !global.!jobicon.i = dw_icon_load_from_file( !global.!icondir||modname )
   If !global.!jobicon.i = 0 Then Call AbortText 'Could not find icon:' quote( modname ) 'in' !global.!icondir
End
Return

DetermineModuleIcon: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
/*
 * On input 'dir' will be of the form:
 * CVS - /dir1/dir2
 * HG - /dir1/dir2
 * RVS - ??
 */
this_dir = Substr( dir, 2 ) /* strip leading '/' */
this_dir_upper = Translate( this_dir )
/*
 * Check if the module has been checked out...
 */
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         /*
          * If the CVS directory in the working directory
          * doesn't exist, then we haven't checked out anything
          */
         wd = !global.!!repository.!!working_dir.!!last_repository||Substr( dir, 2 )'/CVS'
         If DirectoryExists( wd ) = 1 Then type = 'checkedout'
         Else type = 'notcheckedout'
      End
   When !global.!!repository.!!type.!!last_repository = 'hg' Then
      Do
         -- can there be any other status other than 'checkedout' ?
         type = 'checkedout'
      End
   Otherwise Nop
End
/*
 * Check if the module is linked...
 */
linked = ''
Do i = 1 to !global.!common_dir.0
   If dir = !global.!common_dir.i Then
      Do
         linked = 'linked'
         Leave
      End
End
Do i = 1 to !global.!linked_dir.0
   If dir = !global.!linked_dir.i Then
      Do
         linked = 'linked'
         Leave
      End
End
/*
 * Check if the module is buildable...
 */
If FindBuildCommand( this_dir ) = '' Then buildable = ''
Else buildable = 'buildable'
say Wordpos( 'module'linked||buildable||type, !global.!moduleicons ) 'module'linked||buildable||type
Return Wordpos( 'module'linked||buildable||type, !global.!moduleicons )

/*
 * --------------------------------------------------------------------
 * Utility functions
 * --------------------------------------------------------------------
 */

/*
 * From the current directory, determine the directory icon and change it
 */
RefreshDirectoryIcons: Procedure Expose !REXXDW. !global. !!last_repository
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         /*
          * Start with the current directory and change the icon of all ancestors
          */
         parentitem = !global.!selected_treeitem.!!last_repository
         Do Forever
            thisitem = parentitem
            Parse Value dw_tree_item_get_data( !global.!tree.!!last_repository, parentitem ) With parentitem mydir
            mydir = Strip( mydir, 'B', '"' )
            Parse Value DetermineModuleIconAndName( mydir ) With icon currentdir
            If parentitem = 0 Then currentdir = '['!global.!!repository.!!name.!!last_repository']'
            Call dw_tree_item_change !global.!tree.!!last_repository, thisitem, currentdir, icon
            If parentitem = 0 Then Leave
         End
         /*
          * Now we have to change all the children
          */
         thisdir = '/'!global.!current_dirname.!!last_repository'/'
         len = Length( thisdir )
         Do i = 1 To !global.!repdir.!!last_repository.0
            If Left( !global.!repdir.!!last_repository.i, len ) = thisdir Then
               Do
                  Parse Value DetermineModuleIconAndName( !global.!repdir.!!last_repository.i ) With icon currentdir
                  Call dw_tree_item_change !global.!tree.!!last_repository, !global.!treeitem.!!last_repository.i, currentdir, icon
               End
         End
      End
   Otherwise Nop
End
Return

DetermineModuleIconAndName: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg mydir
idx = DetermineModuleIcon( mydir )
icon = !global.!moduleicon.idx
pos = Lastpos( '/', mydir )
currentdir = Substr( mydir, pos+1 )
/*
 * Is this directory linked ? If so, append the directory we are linked to.
 */
Do j = 1 To !global.!linked_dir.0
   If !global.!linked_dir.j = mydir Then
      Do
         commonidx = !global.!linked_idx.j
         currentdir = Substr( !global.!linked_dir.j, pos+1 ) '->' !global.!common_dir.commonidx
         Leave
      End
End
Return icon currentdir

CreateDirectory: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
/*dir = Changestr( '/', dir, !global.!ossep )*/
If DirectoryExists( dir ) = 0 Then Address System !global.!cmdmkdir dir With Output Stem junk. Error Stem junk.
Return

/*
 * If no template passed, the caller assumes responsibility for deleteing the file
 */
GenerateTempFile: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg template
If template = '' Then
   Do
      tmpfile = SysTempFileName( !global.!!tmpdir'?????' )
   End
Else
   Do
      tmpfile = SysTempFileName( !global.!!tmpdir||template )
      Call AddTempFileForCleanup tmpfile
   End
Return tmpfile

AddTempFileForCleanup: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg tmpfile
idx = !global.!tempfiles.0 + 1
!global.!tempfiles.idx = tmpfile
!global.!tempfiles.0 = idx
Return

DirectoryExists: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
If Right( dir, 1 ) = '/' Then dir = Strip( dir, 'T', '/' )
fstat = Stream( dir, 'C', 'FSTAT' )
ftype = Word( fstat, 8 )
rcode = 0
Select
   When ftype = 'Directory' Then rcode = 1
   When ftype = 'SymbolicLink' Then
      Do
         -- get the canonical name (which resolves symbolic links) ...
         link = Stream( dir, 'C', 'QUERY EXISTS' )
         fstat = Stream( link, 'C', 'FSTAT' )
         -- ... and if that's a directory ...
         If  Word( fstat, 8 ) = 'Directory' Then rcode = 1
         Else rcode = 0
      End
   Otherwise rcode = 0
End
Return rcode

DirToVariable: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
Return Translate( Translate( dir ), '___', '\/ ' )

WorkingDirectoryExists: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
Return DirectoryExists( GetFullWorkingDirectory( dir ) )

GetFullWorkingDirectory: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         current_fullworkingdir = Changestr( '/', !global.!!repository.!!working_dir.!!last_repository||dir, !global.!ossep )
      End
   Otherwise Nop
End
Return current_fullworkingdir

CreateDirToolbarList: Procedure Expose !REXXDW. !global. !!last_repository
!global.!dirpopup_offline. = 'D' /* disable all dirpopup menu items */
/*
 * Start our index counter...
 */
i = 0
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Refresh Modules';      !global.!dirpopup_command.i='refreshmodules';    !global.!dirpopup_state.i='normal'
/*
 * A repository administrator can see the properties...
 */
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Properties';  !global.!dirpopup_command.i='dirproperties';   !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='separator';
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='List Modified';  !global.!dirpopup_command.i='listmodified';    !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='List Locked';  !global.!dirpopup_command.i='listlocked';    !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='separator';
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Get Latest All';     !global.!dirpopup_command.i='getlatestall';!global.!dirpopup_state.i='normal'
If !global.!!repository.!!type.!!last_repository = 'cvs' Then
   Do
      i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Update All'; !global.!dirpopup_command.i='updateall'; !global.!dirpopup_state.i='normal'
   End
Else
   Do
      i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Update All'; !global.!dirpopup_command.i='updateall'; !global.!dirpopup_state.i='disabled'
   End
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Get All...';      !global.!dirpopup_command.i='getall'; !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Check Out All';  !global.!dirpopup_command.i='checkoutall'; !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Undo Check Out All';  !global.!dirpopup_command.i='undocheckoutall'; !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='separator';
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Check In All';   !global.!dirpopup_command.i='checkinall';  !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='separator';
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Make';  !global.!dirpopup_command.i='make';   !global.!dirpopup_state.i='normal'; !global.!dirpopup_offline.i='N'
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Report Module History';  !global.!dirpopup_command.i='reportmodulehistory';   !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='separator';
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Label';  !global.!dirpopup_command.i='label';   !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Build';  !global.!dirpopup_command.i='build';   !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Promote';!global.!dirpopup_command.i='promote'; !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='separator';
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Link Directory';  !global.!dirpopup_command.i='linkdir';   !global.!dirpopup_state.i='normal'
If !global.!!repository.!!type.!!last_repository = 'cvs' Then
   Do
      i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Copy Repository';  !global.!dirpopup_command.i='copyrepository';   !global.!dirpopup_state.i='normal'
   End
i = i + 1;!global.!dirpopup_type.i='separator';
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Add Bookmark';     !global.!dirpopup_command.i='addbookmark';      !global.!dirpopup_state.i='normal'
i = i + 1;!global.!dirpopup_type.i='command'; !global.!dirpopup_label.i='Delete Bookmark';  !global.!dirpopup_command.i='deletebookmark';   !global.!dirpopup_state.i='normal'
!global.!dirpopup_type.0 = i
!global.!dirpopup_command.0 = i
Return

CreateDirToolbar: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Destroy the button box and recreate it
 */
redraw = 0
If !global.!dirtoolbarbox \= 0 Then
   Do
      Call dw_window_destroy !global.!dirtoolbarbox
      redraw = 1
   End
!global.!dirtoolbarbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start !global.!dirtoolbarboxperm, !global.!dirtoolbarbox, !global.!toolbox_size, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Now we have set up the items appropriate for our repository type and user,
 * display the buttons...
 */
Do i = 1 To !global.!dirpopup_type.0
   Select
      When !global.!dirpopup_type.i = 'separator' Then
         Do
            Call dw_box_pack_start !global.!dirtoolbarbox, 0, !global.!toolbox_size, 5, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
         End
      When !global.!dirpopup_type.i = 'command'   Then
         Do
            abutton = dw_bitmapbutton_new_from_file( !global.!dirpopup_label.i, 0, !global.!icondir||!global.!dirpopup_command.i )
            Call dw_box_pack_start !global.!dirtoolbarbox, abutton, !global.!toolbox_size, !global.!toolbox_size, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
            Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, !global.!dirpopup_command.i||'Callback'
            If !global.!dirpopup_state.i = 'disabled' Then Call dw_window_disable abutton
            !global.!dirtoolbarbutton.i = abutton
         End
      Otherwise Nop
   End
End
!global.!dirtoolbarbutton.0 = !global.!dirpopup_type.0
/*
 * Pack a tiny spacing in the bottom that is expandable so that OS/2
 * packs buttons to the top
 */
Call dw_box_pack_start !global.!dirtoolbarbox, 0, !global.!toolbox_size, 1, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * We might have changed the packing, so we need to redraw the top-level window...
 */
Call dw_window_redraw !global.!mainwindow
Return

DisplayDirMenu: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg x, y
/*
 * Now we have set up the items appropriate for our repository type and user,
 * determine which items to add to the menu
 * CAN ONLY BE CALLED FROM signal handler
 */
/*
 * Create the directory popup each time we get here, because dw_menu_popup()
 * destroys it.
 */
!global.!dirpopup = dw_menu_new( 0 )
last_item_is_separator = 0
Do i = 1 To !global.!dirpopup_type.0
   Select
      When !global.!dirpopup_state.i = 'disabled' Then Nop
      When !global.!dirpopup_type.i = 'separator' Then
         Do
            If last_item_is_separator = 0 Then
               menuitem = dw_menu_append_item( !global.!dirpopup, !REXXDW.!DW_MENU_SEPARATOR, 5000+i, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
            last_item_is_separator = 1
         End
      When !global.!dirpopup_type.i = 'command'   Then
         Do
            menuitem = dw_menu_append_item( !global.!dirpopup, !global.!dirpopup_label.i, 5000+i, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
            Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, !global.!dirpopup_command.i||'Callback', i
            last_item_is_separator = 0
         End
      Otherwise Nop
   End
End
!global.!dirtoolbarbutton.0 = !global.!dirpopup_type.0
/*
 * If the user has specified configurable menu items, add them at the
 * bottom. We don't add them into the !global.!dirpopup... stems
 */
If Datatype( !global.!!user.!!dirmenu.!!label.0 ) = 'NUM' Then
   Do
      If last_item_is_separator = 0 Then
         Call dw_menu_append_item !global.!dirpopup, !REXXDW.!DW_MENU_SEPARATOR, 5500, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0
      Do i = 1 To !global.!!user.!!dirmenu.!!label.0
         menuitem = dw_menu_append_item( !global.!dirpopup, !global.!!user.!!dirmenu.!!label.i, 5500+i, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
         Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'UserDirCommandCallback', i
      End
   End
Call dw_menu_popup !global.!dirpopup, !global.!mainwindow, x, y
Return

AdjustDirToolbarList: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Adjust the availability of menu items depending on various conditions...
 * TODO disable most commands if no currently selected module
 * TODO only change the 'disabled'/'normal' flag, another function to
 * change the button status
 */
if trace() = 'F' Then say !global.!offline.!!last_repository !global.!current_dirname.!!last_repository
this_buildcommand = FindBuildCommand( !global.!current_dirname.!!last_repository )
this_promotecommand = FindPromoteCommand( !global.!current_dirname.!!last_repository )
this_makecommand = FindMakeCommand( !global.!current_dirname.!!last_repository )
wde = WorkingDirectoryExists( !global.!current_dirname.!!last_repository )
Do i = 1 To !global.!dirpopup_command.0
   Select
      When !global.!dirpopup_command.i = 'dirproperties' Then
         Do
            If !global.!administrator.!!last_repository = 'Y' | !global.!current_dirname.!!last_repository = '.' Then !global.!dirpopup_state.i = 'normal'
            Else !global.!dirpopup_state.i = 'disabled'
         End
      When !global.!dirpopup_command.i = 'promote' & this_promotecommand = '' Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'addmodules' & wde = 0 Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'updateall' & wde = 0 Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'listmodified' & wde = 0 Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'checkoutall' | !global.!dirpopup_command.i = 'undocheckoutall' Then
         Do
            this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
            If !global.!dir.!reservedcheckout.this_dir = 'Y' | !global.!reservedcheckout = 'Y' Then !global.!dirpopup_state.i = 'normal'
            Else !global.!dirpopup_state.i = 'disabled'
         End
      When !global.!dirpopup_command.i = 'checkinall' & wde = 0 Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'checkinall' & !global.!update_user.!!last_repository \= 'Y' Then !global.!dirpopup_state.i = 'disabled'
      When !global.!offline.!!last_repository & !global.!dirpopup_offline.i = 'D' Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'build' & this_buildcommand = '' Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'build' & !global.!build_user.!!last_repository \= 'Y' Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'label' & !global.!build_user.!!last_repository \= 'Y' Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'promote' & !global.!build_user.!!last_repository \= 'Y' Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'make' & this_makecommand = '' Then !global.!dirpopup_state.i = 'disabled'
      When !global.!dirpopup_command.i = 'linkdir' | !global.!dirpopup_command.i = 'copyrepository' Then
         Do
            Select
               /*
                * Only an administrator can link or copy...
                */
               When !global.!administrator.!!last_repository \= 'Y' Then !global.!dirpopup_state.i = 'disabled'
               When !global.!!repository.!!type.!!last_repository = 'cvs' Then
                  Do
                     /*
                      * If on CVS, only valid if local connection
                      */
                     If !global.!!repository.!!contype.!!last_repository = 'local' Then !global.!dirpopup_state.i = 'normal'
                     Else !global.!dirpopup_state.i = 'disabled'
                  End
               Otherwise !global.!dirpopup_state.i = 'normal'
            End
         End
      When !global.!dirpopup_command.i = 'addbookmark' Then
         Do
            found = 0
            Do j = 1 To !global.!!user.!!bookmark.0
               If !global.!!user.!!bookmark_repository.j = !!last_repository & !global.!!user.!!bookmark.j = !global.!current_dirname.!!last_repository Then
                  Do
                     found = 1
                     Leave
                  End
            End
            If found = 1 Then !global.!dirpopup_state.i = 'disabled'
            Else !global.!dirpopup_state.i = 'normal'
         End
      When !global.!dirpopup_command.i = 'deletebookmark' Then
         Do
            found = 0
            Do j = 1 To !global.!!user.!!bookmark.0
               If !global.!!user.!!bookmark_repository.j = !!last_repository & !global.!!user.!!bookmark.j = !global.!current_dirname.!!last_repository Then
                  Do
                     found = 1
                     Leave
                  End
            End
            If found = 1 Then !global.!dirpopup_state.i = 'normal'
            Else !global.!dirpopup_state.i = 'disabled'
         End
      Otherwise !global.!dirpopup_state.i = 'normal'
   End
End
Return

AdjustDirToolbarButtons: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Based on the status of each dir command, enable or disable the button
 */
Do i = 1 To !global.!dirpopup_command.0
   If !global.!dirpopup_type.i = 'command' Then
      Do
         If !global.!dirpopup_state.i = 'normal' Then Call dw_window_enable !global.!dirtoolbarbutton.i
         Else Call dw_window_disable !global.!dirtoolbarbutton.i
      End
End
Return

CreateFileToolbarList: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Create the popup menu for the file
 *
 * Meaning of "initial_state" flags...
 *
 * 'D' means menu option is disabled if the condition below is true
 * 'N' means menu option is enabled if the condition below is true
 *
 * Word(1): option is enabled if file is locked by current user
 * Word(2): option is enabled if file is locked by another user
 * Word(3): option is enabled if file is not locked by anyone
 * Word(4): option is enabled if multiple files selected
 * Word(5): option is enabled if running in online mode
 * Word(6): option is enabled if working file exists
 * Word(7): option is enabled if in non-repository files tab
 */
/*
 * If you add more buttons, then change these lines at the top of the program:
!global.!maxfilebuttons = 19
!global.!maxfileseparators = 7
 */
i = 0
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Refresh Files';       !global.!filepopup_command.i='refreshfiles';    !global.!filepopup_initial_state.i='N N N N N N N'
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Properties';          !global.!filepopup_command.i='fileproperties';  !global.!filepopup_initial_state.i='N N N D D N D'
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Add Files';           !global.!filepopup_command.i='addfiles';        !global.!filepopup_initial_state.i='N N N N D N N'
i = i + 1;!global.!filepopup_type.i='separator';
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='View';                !global.!filepopup_command.i='viewlatest';      !global.!filepopup_initial_state.i='N N N N N N N'
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Edit';                !global.!filepopup_command.i='editlatest';      !global.!filepopup_initial_state.i='N D N N N N N'
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='View...';             !global.!filepopup_command.i='viewprior';       !global.!filepopup_initial_state.i='N N N D D D D'
i = i + 1;!global.!filepopup_type.i='separator';
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Get Latest';          !global.!filepopup_command.i='getlatest';       !global.!filepopup_initial_state.i='N N N N D N D'
If !global.!!repository.!!type.!!last_repository = 'cvs' Then
   Do
      i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Update';        !global.!filepopup_command.i='update';          !global.!filepopup_initial_state.i='D D N N D N D'
   End
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Get...';     !global.!filepopup_command.i='getprior';       !global.!filepopup_initial_state.i='N N N D D N D'
i = i + 1;!global.!filepopup_type.i='separator';
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Check Out';           !global.!filepopup_command.i='checkout';         !global.!filepopup_initial_state.i='N D D N D N D'
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Undo Check Out';      !global.!filepopup_command.i='undocheckout';         !global.!filepopup_initial_state.i='N D D N D D D'
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Check In';            !global.!filepopup_command.i='checkin';         !global.!filepopup_initial_state.i='N D D N D D D'
i = i + 1;!global.!filepopup_type.i='separator';
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Show Differences...'; !global.!filepopup_command.i='showdifferences'; !global.!filepopup_initial_state.i='N N N D D N D'
If !global.!!repository.!!type.!!last_repository = 'cvs' Then
   Do
      i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Show Annotations...';      !global.!filepopup_command.i='showannotations';        !global.!filepopup_initial_state.i='N N N D D N D'
   End
i = i + 1;!global.!filepopup_type.i='separator';
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Delete';   !global.!filepopup_command.i='deletefromrepository';  !global.!filepopup_initial_state.i='N N N N D N D'
/*i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Rename'; !global.!filepopup_command.i='renameinrepository';!global.!filepopup_initial_state.i='N N N D D N D' */
i = i + 1;!global.!filepopup_type.i='separator';
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Set Reminder';      !global.!filepopup_command.i='setreminder';          !global.!filepopup_initial_state.i='N N N D N D D'
i = i + 1;!global.!filepopup_type.i='command'; !global.!filepopup_label.i='Delete Reminder';      !global.!filepopup_command.i='deletereminder';        !global.!filepopup_initial_state.i='N N N D N D D'
!global.!filepopup_type.0 = i
!global.!filepopup_command.0 = i
Return

CreateFileToolbar: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Destroy the button box and recreate it
 */
redraw = 0
If !global.!filetoolbarbox \= 0 Then
   Do
      Call dw_window_destroy !global.!filetoolbarbox
      redraw = 1
   End
!global.!filetoolbarbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start !global.!filetoolbarboxperm, !global.!filetoolbarbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Now we have set up the items appropriate for our repository type and user, build the
 * buttons and menu...
 */
Do i = 1 To !global.!filepopup_type.0
   Select
      When !global.!filepopup_type.i = 'separator' Then
         Do
            Call dw_box_pack_start !global.!filetoolbarbox, 0, !global.!toolbox_size, 5, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
         End
      When !global.!filepopup_type.i = 'command'   Then
         Do
            abutton = dw_bitmapbutton_new_from_file( !global.!filepopup_label.i, 0, !global.!icondir||!global.!filepopup_command.i )
            Call dw_box_pack_start !global.!filetoolbarbox, abutton, !global.!toolbox_size, !global.!toolbox_size, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
            Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, !global.!filepopup_command.i||'Callback'
            If !global.!filepopup_state.i = 'disabled' Then Call dw_window_disable abutton
            !global.!filetoolbarbutton.i = abutton
         End
      Otherwise Nop
   End
End
!global.!filetoolbarbutton.0 = !global.!filepopup_type.0
/*
 * Pack a tiny spacing in the bottom that is expandable so that OS/2
 * packs buttons to the top
 */
Call dw_box_pack_start !global.!filetoolbarbox, 0, !global.!toolbox_size, 1, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * We might have changed the packing, so we need to redraw the top-level window...
 */
Call dw_window_redraw !global.!mainwindow
Return

DisplayFileMenu: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg x, y
/*
 * Now we have set up the items appropriate for the selected files
 * determine which items to add to the menu
 * CAN ONLY BE CALLED FROM signal handler
 */
/*
 * Create the directory popup each time we get here, because dw_menu_popup()
 * destroys it.
 */
last_item_is_separator = 0
!global.!filepopup = dw_menu_new( 0 )
Do i = 1 To !global.!filepopup_type.0
if trace() = 'F' Then say 'state' !global.!filepopup_command.i !global.!filepopup_state.i
   Select
      When !global.!filepopup_state.i = 'disabled' Then Nop
      When !global.!filepopup_type.i = 'separator' Then
         Do
            If last_item_is_separator = 0 Then
               menuitem = dw_menu_append_item( !global.!filepopup, !REXXDW.!DW_MENU_SEPARATOR, 6000+i, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
            last_item_is_separator = 1
         End
      When !global.!filepopup_type.i = 'command'   Then
         Do
            menuitem = dw_menu_append_item( !global.!filepopup, !global.!filepopup_label.i, 6000+i, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
            Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, !global.!filepopup_command.i||'Callback', i
            last_item_is_separator = 0
         End
      Otherwise Nop
   End
End
!global.!filetoolbarbutton.0 = !global.!filepopup_type.0
/*
 * If the user has specified configurable menu items, add them at the
 * bottom. We don't add them into the !global.!filepopup... stems
 */
If Datatype( !global.!!user.!!filemenu.!!label.0 ) = 'NUM' Then
   Do
      If last_item_is_separator = 0 Then
         Call dw_menu_append_item !global.!filepopup, !REXXDW.!DW_MENU_SEPARATOR, 5500, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0
      Do i = 1 To !global.!!user.!!filemenu.!!label.0
         menuitem = dw_menu_append_item( !global.!filepopup, !global.!!user.!!filemenu.!!label.i, 6500+i, 0, !REXXDW.!DW_MENU_END, !REXXDW.!DW_MENU_NOT_CHECKABLE, 0 )
         Call dw_signal_connect menuitem, !REXXDW.!DW_CLICKED_EVENT, 'UserFileCommandCallback', i
      End
   End
Call dw_menu_popup !global.!filepopup, !global.!mainwindow, x, y
Return

/*
 * Must be called by item-select event on the repository file container
 * This is so that the toolbar buttons are updated to enabled/disabled
 */
AdjustFileToolbarList: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * This function is called when the repository files and the non-repository
 * files page is displayed. We need to do vastly different things for each
 * page.
 */
Select
   When  !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository | !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then
      Do
         If !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then
            Do
               doing_repfiles = 1
               container = !global.!repfilescontainer.!!last_repository
            End
         Else
            Do
               doing_repfiles = 0
               container = !global.!allfilescontainer.!!last_repository
            End
         /*
          * Determine which repository items are currently selected
          */
         current_indexes = ''
         selected = dw_container_query_start( container, !REXXDW.!DW_CRA_SELECTED )
         Do i = 1 While selected \= ''
            current_indexes = current_indexes selected
            If doing_repfiles Then
               !global.!current_filenames.i = Substr( !global.!repfiles.?fullfilename.!!last_repository.selected, 5 )
            Else
               !global.!current_filenames.i = Substr( !global.!allfiles.?fullfilename.!!last_repository.selected, 5 )
            !global.!current_indexes.i = selected
            if trace() = 'F' Then       say 'selected' selected !global.!current_filenames.i !global.!repfiles.?filetype.!!last_repository.selected
            selected = dw_container_query_next( container, !REXXDW.!DW_CRA_SELECTED )
         End
         !global.!current_filenames.0 = i-1
         !global.!current_indexes.0 = i-1
         afpi = Word( current_indexes, 1 )
         changes = ''
         Select
            When !global.!!repository.!!type.!!last_repository = 'cvs' Then
               Do
                  this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
                  If doing_repfiles Then
                     Do
                        filetype = !global.!repfiles.?filetype.!!last_repository.afpi
                        user = !global.!repfiles.?user.!!last_repository.afpi
                     End
                  Else
                     Do
                        filetype = !global.!allfiles.?filetype.!!last_repository.afpi
                        user = !global.!allfiles.?user.!!last_repository.afpi
                     End
                  If !global.!current_filenames.0 = 1 & Substr( filetype, 5,6 ) = 'binary' Then
                     Do
                        /*
                         * We have 1 file and it is a binary file; prepare to change the
                         * menu significantly.
                         */
                        is_binary_file = 1
                        Parse Var user . 5 . '(' locker ')'
                     End
                  Else is_binary_file = 0
                  If !global.!dir.!reservedcheckout.this_dir = 'Y' | !global.!reservedcheckout = 'Y' Then
                     Do
                        is_reserved_file = 1
                        Parse Var user . 5 . '(' locker ')'
                     End
                  Else is_reserved_file = 0
                  if trace() = 'F' Then say 'is_binary_file' is_binary_file 'is_reserved_file' is_reserved_file 'type' !global.!repfiles.?filetype.!!last_repository.afpi
                  /*
                   * Do some preliminary checks for multiple files that involve binary files.
                   * If multiple files are selected and there is a mix between binary
                   * and text, then everything is disabled (except Get Latest)
                   */
                  number_text_files = 0
                  number_binary_files = 0
                  Do j = 1 To !global.!current_filenames.0
                     k = Word( current_indexes, j )
                     If Substr( !global.!repfiles.?filetype.!!last_repository.k, 5, 6 ) = 'binary' Then number_binary_files = number_binary_files + 1
                     Else number_text_files = number_text_files + 1
                  End
                  if trace() = 'F' Then say 'number_text_files' number_text_files 'number_binary_files' number_binary_files
                  Do i = 1 To !global.!filepopup_type.0
                     If !global.!filepopup_type.i = 'command' Then
                        Do
                           /*
                            * Now do the checks...
                            */
                           If !global.!filepopup_command.i = 'refreshfiles' Then
                              Do
                                 /*
                                  * Refresh is always available
                                  */
                                 !global.!filepopup_state.i = 'normal'
                                 Iterate
                              End
                           If !global.!current_filenames.0 = 0 Then
                              Do
                                 /*
                                  * If no files selected, set everything to disabled
                                  */
                                 !global.!filepopup_state.i = 'disabled'
                                 if trace() = 'F' Then say !global.!filepopup_command.i 'disabled no files selected'
                                 Iterate
                              End
                           If !global.!offline.!!last_repository & Word( !global.!filepopup_initial_state.i, 5 ) = 'D' Then
                              Do
                                 /*
                                  * Offline mode is very restrictive...
                                  */
                                 if trace() = 'F' Then say !global.!filepopup_command.i 'disabled offline'
                                 !global.!filepopup_state.i = 'disabled'
                                 Iterate
                              End
                           If Word( !global.!filepopup_initial_state.i, 6 ) = 'D' Then
                              /*
                                * If the working file(s) don't exist, you can't do much!
                                */
                              Do j = 1 To !global.!current_filenames.0
                                 If Stream( CVSFileToWorkingFile( !global.!current_filenames.j, !!last_repository ), 'C', 'QUERY EXISTS' ) = '' Then
                                    Do
                                       if trace() = 'F' Then say !global.!filepopup_command.i 'disabled no working file'
                                       !global.!filepopup_state.i = 'disabled'
                                       Iterate i
                                    End
                              End
                           If Word( !global.!filepopup_initial_state.i, 4 ) = 'D' Then
                              Do
                                 /*
                                  * If the number of selected files is more
                                  * than 1 and Word(!global.!filepopup_initial_state.i,4) = 'D', then disable
                                  * it.
                                  */
                                 If !global.!current_filenames.0 \= 1 Then
                                    Do
                                       if trace() = 'F' Then say !global.!filepopup_command.i 'disabled only valid for single file'
                                       !global.!filepopup_state.i = 'disabled'
                                       Iterate
                                    End
                              End
                           /*
                            * Now tests for individual commands...
                            */
                           !global.!filepopup_state.i = 'normal'
                           Select
                              When !global.!filepopup_command.i = 'fileproperties' Then
                                 /*
                                  * Allow file properties
                                  */
                                 Do
                                    !global.!filepopup_state.i = 'normal'
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                    Iterate
                                 End
                              When !global.!filepopup_command.i = 'addfiles' Then
                                 /*
                                  * We are on the repository files page;
                                  * disable "addfiles"
                                  */
                                 Do
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'disabled'
                                    !global.!filepopup_state.i = 'disabled'
                                    Iterate
                                 End
                              When !global.!filepopup_command.i = 'editlatest' Then
                                 Do
                                    /*
                                     * If a binary file or a file in a reserved
                                     * checkout directory, and someone else
                                     * has the lock, then no go
                                     */
                                    If (is_binary_file | is_reserved_file) & locker \= '' & Translate( locker ) \= Translate( !global.!user ) Then
                                       Do
                                          if trace() = 'F' Then say !global.!filepopup_command.i 'disabled binary file locked by other'
                                          !global.!filepopup_state.i = 'disabled'
                                          Iterate
                                       End
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'update' Then
                                 Do
                                    Select
                                       When Stream( !global.!current_dirname.!!last_repository, 'C', 'QUERY EXISTS' ) = '' Then
                                          Do
                                             /*
                                              * The working directory doesn't exist; disable update
                                              */
                                             !global.!filepopup_state.i = 'disabled'
                                             if trace() = 'F' Then say !global.!filepopup_command.i 'disabled update - no working dir'
                                             Iterate
                                          End
                                       Otherwise Nop
                                    End
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'undocheckout' Then
                                 Do
                                    If is_binary_file | is_reserved_file Then
                                       Do
                                          /*
                                           * We have 1 file and it is a binary file; or
                                           * in a reserved checkout directory,
                                           * its disabled is we don't own the lock
                                           */
                                          If Translate( locker ) \= Translate( !global.!user ) Then
                                             Do
                                                !global.!filepopup_state.i = 'disabled'
                                                if trace() = 'F' Then say !global.!filepopup_command.i 'disabled binary file not locked by me'
                                                Iterate
                                             End
                                       End
                                    Else
                                       Do
                                          /*
                                           * Text files don't use this
                                           */
                                          !global.!filepopup_state.i = 'disabled'
                                          if trace() = 'F' Then say !global.!filepopup_command.i 'disabled text file'
                                          Iterate
                                       End
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'checkout' Then
                                 Do
                                    numcanbecheckedout = 0
                                    Do k = 1 To !global.!current_indexes.0
                                       If CanFileBeCheckedOutForLock( !global.!current_indexes.k, is_reserved_file ) Then
                                          numcanbecheckedout = numcanbecheckedout + 1
                                    End
                                    If numcanbecheckedout \= !global.!current_indexes.0 Then
                                       Do
                                          !global.!filepopup_state.i = 'disabled'
                                          if trace() = 'F' Then say !global.!filepopup_command.i 'disabled all binary files cannot be checked out'
                                          Iterate
                                       End
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'checkin' Then
                                 Do
                                    /*
                                     * If the user is not allowed to checkin, disable that.
                                     */
                                    If !global.!update_user.!!last_repository = 'N' Then
                                       Do
                                          !global.!filepopup_state.i = 'disabled'
                                          if trace() = 'F' Then say !global.!filepopup_command.i 'disabled user cant update'
                                          Iterate
                                       End
                                    /*
                                     * If a binary file or a file in a reserved
                                     * checkout directory, and we don't hold
                                     * the lock, no go
                                     */
                                    If (is_binary_file | is_reserved_file) & Translate( locker ) \= Translate( !global.!user ) Then
                                       Do
                                          if trace() = 'F' Then say !global.!filepopup_command.i 'disabled binary file we arent locker'
                                          !global.!filepopup_state.i = 'disabled'
                                          Iterate
                                       End
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'showdifferences' Then
                                 Do
                                    /*
                                     * This is invalid for binary files, except for Word .doc
                                     * files
                                     */
                                    If is_binary_file Then
                                       Do
                                          If !global.!os = 'WIN32' & Translate( Right( !global.!current_filenames.1, 4 ) ) = '.DOC' Then
                                             Do
                                                !global.!filepopup_state.i = 'normal'
                                             End
                                          Else
                                             Do
                                                !global.!filepopup_state.i = 'disabled'
                                                if trace() = 'F' Then say !global.!filepopup_command.i 'disabled binary file'
                                             End
                                          Iterate
                                       End
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'showannotations' Then
                                 Do
                                    /*
                                     * This is invalid for binary files
                                     */
                                    If is_binary_file Then
                                       Do
                                          !global.!filepopup_state.i = 'disabled'
                                          if trace() = 'F' Then say !global.!filepopup_command.i 'disabled binary file'
                                          Iterate
                                       End
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'deletefromrepository' Then
                                 /*
                                  * If the user is not allowed to delete, then delete
                                  * is unavailable
                                  */
                                 Do
                                    If !global.!administrator.!!last_repository = 'Y' Then
                                       Do
                                          !global.!filepopup_state.i = 'normal'
                                          if trace() = 'F' Then say !global.!filepopup_command.i 'normal - we are administrator'
                                          Iterate
                                       End
                                    If !global.!update_user.!!last_repository = 'N' Then
                                       Do
                                          /*
                                           * If you aren't an update user, you can't delete
                                           */
                                          if trace() = 'F' Then say !global.!filepopup_command.i 'disabled we arent an update user'
                                          !global.!filepopup_state.i = 'disabled'
                                          Iterate
                                       End
                                    If !global.!user_can_delete = 'Y' Then
                                       Do
                                          !global.!filepopup_state.i = 'normal'
                                          if trace() = 'F' Then say !global.!filepopup_command.i 'normal - we are allowed delete'
                                          Iterate
                                       End
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'disabled we arent allowed to delete or not admin'
                                    !global.!filepopup_state.i = 'disabled'
                                 End
                              Otherwise
                                 /*
                                  * If we get here, we allow the button/menu item
                                  */
                                 Do
                                    !global.!filepopup_state.i = 'normal'
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                           End
                     End
                  End
               End
            Otherwise /* not CVS */
               Do
                  this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
                  If !global.!current_filenames.0 = 1 & Substr( !global.!repfiles.?filetype.!!last_repository.afpi, 5,6 ) = 'binary' Then
                     Do
                        /*
                         * We have 1 file and it is a binary file; prepare to change the
                         * menu significantly.
                         */
                        is_binary_file = 1
                        Parse Var !global.!repfiles.?user.!!last_repository.afpi . 5 . '(' locker ')'
                     End
                  Else is_binary_file = 0
                  If !global.!dir.!reservedcheckout.this_dir = 'Y' | !global.!reservedcheckout = 'Y' Then
                     Do
                        is_reserved_file = 1
                        Parse Var !global.!repfiles.?user.!!last_repository.afpi . 5 . '(' locker ')'
                     End
                  Else is_reserved_file = 0
                  if trace() = 'F' Then say 'is_binary_file' is_binary_file 'is_reserved_file' is_reserved_file 'type' !global.!repfiles.?filetype.!!last_repository.afpi
                  /*
                   * Do some preliminary checks for multiple files that involve binary files.
                   * If multiple files are selected and there is a mix between binary
                   * and text, then everything is disabled (except Get Latest)
                   */
                  number_text_files = 0
                  number_binary_files = 0
                  Do j = 1 To !global.!current_filenames.0
                     afpi = Word( current_indexes, j )
                     If Substr( !global.!repfiles.?filetype.!!last_repository.afpi, 5, 6 ) = 'binary' Then number_binary_files = number_binary_files + 1
                     Else number_text_files = number_text_files + 1
                  End
                  if trace() = 'F' Then say 'number_text_files' number_text_files 'number_binary_files' number_binary_files

                  Do i = 1 To !global.!filepopup_type.0
                     If !global.!filepopup_type.i = 'command' Then
                        Do
                           /*
                            * Now do the checks...
                            */
                           If !global.!filepopup_command.i = 'refreshfiles' Then
                              Do
                                 /*
                                  * Refresh is always available
                                  */
                                 !global.!filepopup_state.i = 'normal'
                                 Iterate
                              End
                           If !global.!current_filenames.0 = 0 Then
                              Do
                                 /*
                                  * If no files selected, set everything to disabled
                                  */
                                 !global.!filepopup_state.i = 'disabled'
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled no files selected'
                                 Iterate
                              End
                           If !global.!offline.!!last_repository & Word( !global.!filepopup_initial_state.i, 5 ) = 'D' Then
                              Do
                                 /*
                                  * Offline mode is very restrictive...
                                  */
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled offline'
                                 !global.!filepopup_state.i = 'disabled'
                                 Iterate
                              End
                           If Word( !global.!filepopup_initial_state.i, 6 ) = 'D' Then
                              /*
                                * If the working file(s) don't exist, you can't do much!
                                */
                              Do j = 1 To !global.!current_filenames.0
                                 If Stream( CVSFileToWorkingFile( !global.!current_filenames.j, !!last_repository ), 'C', 'QUERY EXISTS' ) = '' Then
                                    Do
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled no working file'
                                       !global.!filepopup_state.i = 'disabled'
                                       Iterate i
                                    End
                              End
                           If Word( !global.!filepopup_initial_state.i, 4 ) = 'D' Then
                              Do
                                 /*
                                  * If the number of selected files is more
                                  * than 1 and Word(!global.!filepopup_initial_state.i,4) = 'D', then disable
                                  * it.
                                  */
                                 If !global.!current_filenames.0 \= 1 Then
                                    Do
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled only valid for single file'
                                       !global.!filepopup_state.i = 'disabled'
                                       Iterate
                                    End
                              End
                           /*
                            * Now tests for individual commands...
                            */
                           !global.!filepopup_state.i = 'normal'
                           Select
                              When !global.!filepopup_command.i = 'fileproperties' Then
                                 /*
                                  * Allow file properties
                                  */
                                 Do
                                    !global.!filepopup_state.i = 'normal'
   if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                    Iterate
                                 End
                              When !global.!filepopup_command.i = 'addfiles' Then
                                 /*
                                  * We are on the repository files page;
                                  * disable "addfiles"
                                  */
                                 Do
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled'
                                    !global.!filepopup_state.i = 'disabled'
                                    Iterate
                                 End
                              When !global.!filepopup_command.i = 'editlatest' Then
                                 Do
                                    /*
                                     * If a binary file or a file in a reserved
                                     * checkout directory, and someone else
                                     * has the lock, then no go
                                     */
                                    If locker \= '' & Translate( locker ) \= Translate( !global.!user ) Then
                                       Do
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled file locked by other'
                                          !global.!filepopup_state.i = 'disabled'
                                          Iterate
                                       End
   if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'update' Then
                                 Do
                                    !global.!filepopup_state.i = 'disabled'
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled update invalid command'
                                    Iterate
                                 End
                              When !global.!filepopup_command.i = 'undocheckout' Then
                                 Do
                                    /*
                                     * We have 1 file and it is a binary file; or
                                     * in a reserved checkout directory,
                                     * its disabled is we don't own the lock
                                     */
                                    If Translate( locker ) \= Translate( !global.!user ) Then
                                       Do
                                          !global.!filepopup_state.i = 'disabled'
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled file not locked by me'
                                          Iterate
                                       End
   if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'checkout' Then
                                 Do
                                    /*
                                     * We have 1 file and it is a binary file or
                                     * we are in a reserved directory, checkout
                                     * is only valid if its not locked
                                     */
                                    If locker \= '' Then
                                       Do
                                          !global.!filepopup_state.i = 'disabled'
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled file checked out already'
                                          Iterate
                                       End
   if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'checkin' Then
                                 Do
                                    /*
                                     * If the user is not allowed to checkin, disable that.
                                     */
                                    If !global.!update_user.!!last_repository = 'N' Then
                                       Do
                                          !global.!filepopup_state.i = 'disabled'
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled user cant update'
                                          Iterate
                                       End
                                    /*
                                     * If a binary file or a file in a reserved
                                     * checkout directory, and we don't hold
                                     * the lock, no go
                                     */
                                    If Translate( locker ) \= Translate( !global.!user ) Then
                                       Do
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled binary file we arent locker'
                                          !global.!filepopup_state.i = 'disabled'
                                          Iterate
                                       End
   if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'showdifferences' Then
                                 Do
                                    /*
                                     * This is invalid for binary files, except for Word .doc
                                     * files
                                     */
                                    If is_binary_file Then
                                       Do
                                          If !global.!os = 'WIN32' & Translate( Right( !global.!current_filenames.1, 4 ) ) = '.DOC' Then
                                             Do
                                                !global.!filepopup_state.i = 'normal'
                                             End
                                          Else
                                             Do
                                                !global.!filepopup_state.i = 'disabled'
                                                if trace() = 'F' Then say !global.!filepopup_command.i 'disabled binary file'
                                             End
                                          Iterate
                                       End
                                    if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                              When !global.!filepopup_command.i = 'showannotations' Then
                                 Do
                                    !global.!filepopup_state.i = 'disabled'
                                    Iterate
                                 End
                              When !global.!filepopup_command.i = 'deletefromrepository' Then
                                 /*
                                  * If the user is not allowed to delete, then delete
                                  * is unavailable
                                  */
                                 Do
                                    If !global.!administrator.!!last_repository = 'Y' Then
                                       Do
                                          !global.!filepopup_state.i = 'normal'
   if trace() = 'F' Then say !global.!filepopup_command.i 'normal - we are administrator'
                                          Iterate
                                       End
                                    If !global.!user_can_delete = 'Y' Then
                                       Do
                                          !global.!filepopup_state.i = 'normal'
   if trace() = 'F' Then say !global.!filepopup_command.i 'normal - we are allowed delete'
                                          Iterate
                                       End
   if trace() = 'F' Then say !global.!filepopup_command.i 'disabled we arent allowed to delete or not admin'
                                    !global.!filepopup_state.i = 'disabled'
                                 End
                              Otherwise
                                 /*
                                  * If we get here, we allow the button/menu item
                                  */
                                 Do
                                    !global.!filepopup_state.i = 'normal'
   if trace() = 'F' Then say !global.!filepopup_command.i 'normal'
                                 End
                           End
                     End
                  End
               End
         End
      End
   When  !global.!filespage.!!last_repository = !global.!filenotebookpage.?nonrepfiles.!!last_repository Then
      Do
         /*
          * Determine which non-repository items are currently selected
          */
         current_indexes = ''
         selected = dw_container_query_start( !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_CRA_SELECTED )
         Do i = 1 While selected \= ''
            current_indexes = current_indexes selected
            !global.!current_nonrepfilenames.i = Substr( !global.!nonrepfiles.?filename.!!last_repository.selected, 5 )
            !global.!current_nonrepindexes.i = selected
   if trace() = 'F' Then       say 'selected' selected !global.!current_nonrepfilenames.i !global.!nonrepfiles.?filetype.!!last_repository.selected
            selected = dw_container_query_next( !global.!nonrepfilescontainer.!!last_repository, !REXXDW.!DW_CRA_SELECTED )
         End
         !global.!current_nonrepfilenames.0 = i-1
         !global.!current_nonrepindexes.0 = i-1
         Select
            When !global.!!repository.!!type.!!last_repository = 'cvs' Then
               Do
                  Do i = 1 To !global.!filepopup_type.0
                     If !global.!filepopup_type.i = 'command' Then
                        Do
                           /*
                            * Refresh is always available
                            */
                           If !global.!filepopup_command.i = 'refreshfiles' Then
                              Do
                                 !global.!filepopup_state.i = 'normal'
                                 Iterate
                              End
                           /*
                            * If no files selected, set everything to disabled
                            */
                           If !global.!current_nonrepfilenames.0 = 0 Then
                              Do
                                 !global.!filepopup_state.i = 'disabled'
                                 Iterate
                              End
                           /*
                            * If multiple files selected, and multiples disallowed; disable
                            */
                           If !global.!current_nonrepfilenames.0 \= 1 & Word( !global.!filepopup_initial_state.i, 4 ) = 'D' Then
                              Do
                                 !global.!filepopup_state.i = 'disabled'
                                 Iterate
                              End
                           /*
                            * Limit add to update users...
                            */
                           If !global.!filepopup_command.i = 'addfiles' & !global.!update_user.!!last_repository = 'Y' Then
                              Do
                                 !global.!filepopup_state.i = 'normal'
                                 Iterate
                              End
                           /*
                            * To get here at least 1 file is selected
                            * therefore allow refresh, view, edit, delete, rename
                            */
                           If !global.!filepopup_command.i = 'viewlatest' | !global.!filepopup_command.i = 'editlatest' | !global.!filepopup_command.i = 'deletefromrepository' /*| !global.!filepopup_command.i = 'renameinrepository' */ Then
                              Do
                                 !global.!filepopup_state.i = 'normal'
                                 Iterate
                              End
                           /*
                            * Disable everything else
                            */
                           !global.!filepopup_state.i = 'disabled'
                        End
                  End
               End
            Otherwise Nop
         End
      End
   When  !global.!filespage.!!last_repository = !global.!filenotebookpage.?deletedfiles.!!last_repository Then
      Do
         /*
          * Determine which deleted items are currently selected
          */
         current_indexes = ''
         selected = dw_container_query_start( !global.!deletedfilescontainer.!!last_repository, !REXXDW.!DW_CRA_SELECTED )
         Do i = 1 While selected \= ''
            current_indexes = current_indexes selected
            !global.!current_deletedfilenames.i = Substr( !global.!deletedfiles.?fullfilename.!!last_repository.selected, 5 )
            !global.!current_deletedindexes.i = selected
            selected = dw_container_query_next( !global.!deletedfilescontainer.!!last_repository, !REXXDW.!DW_CRA_SELECTED )
         End
         !global.!current_deletedfilenames.0 = i-1
         !global.!current_deletedindexes.0 = i-1
         afpi = Word( current_indexes, 1 )
         changes = ''
         Do i = 1 To !global.!filepopup_type.0
            If !global.!filepopup_type.i = 'command' Then
               Do
                  /*
                   * Now do the checks...
                   */
                  If !global.!filepopup_command.i = 'refreshfiles' Then
                     Do
                        /*
                         * Refresh is always available
                         */
                        !global.!filepopup_state.i = 'normal'
                        Iterate
                     End
                  If !global.!current_deletedfilenames.0 = 0 Then
                     Do
                        /*
                         * If no files selected, set everything to disabled
                         */
                        !global.!filepopup_state.i = 'disabled'
                        Iterate
                     End
                  /*
                   * Now tests for individual commands...
                   */
                  !global.!filepopup_state.i = 'disabled'
                  Select
                     When !global.!filepopup_command.i = 'fileproperties' & !global.!current_deletedfilenames.0 = 1 Then
                        /*
                         * Allow file properties
                         */
                        Do
                           !global.!filepopup_state.i = 'normal'
                           Iterate
                        End
                     When !global.!filepopup_command.i = 'addfiles' Then
                        /*
                         * We are on the deleted files page;
                         * enable "addfiles" if qdmin or user_can_delete
                         */
                        Do
                           If !global.!administrator.!!last_repository = 'Y' Then
                              Do
                                 !global.!filepopup_state.i = 'normal'
                                 Iterate
                              End
                           If !global.!user_can_delete = 'Y' Then
                              Do
                                 !global.!filepopup_state.i = 'normal'
                                 Iterate
                              End
                           !global.!filepopup_state.i = 'disabled'
                        End
                     Otherwise
                        /*
                         * If we get here, we disallow the button/menu item
                         */
                        Do
                           !global.!filepopup_state.i = 'disabled'
                        End
                  End
            End
         End
      End
   Otherwise Nop
End
Return

AdjustFileToolbarButtons: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Based on the status of each dir command, enable or disable the button
 */
Do i = 1 To !global.!filepopup_command.0
   If !global.!filepopup_type.i = 'command' Then
      Do
         If !global.!filepopup_state.i = 'normal' Then Call dw_window_enable !global.!filetoolbarbutton.i
         Else Call dw_window_disable !global.!filetoolbarbutton.i
      End
End
Return

SortFileArray: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Sort into current sort order
 * All field arrays have a 4 digit prefix which is the current sort order;
 * equivalent to the current tail value
 *   Filename         Date
 * ...1a file    ...120001112
 * ...2b file    ...220001110
 * ...3c file    ...320001111
 *
 * Sort on date column ...
 *                    Date
 *               ...220001110
 *               ...320001111
 *               ...120001112
 *
 * Copy Filename stem to tmpfilename.
 * Copy locker   stem to tmplocker.
 * Drop Filename stem
 * Do i = 1 To date.0
 *   idx = Strip(Left(date.i,4))
 *   newi = Right(i,4)
 *
 *   date.i = newi||Substr(date.i,5)              <-- i
 *
 *   !global.!repfiles.?filename.!!last_repository.i = newi||Substr(tmpfilename.idx,5) <-- idx
 *   locker.i = newi||Substr(tmplocker.idx,5)     <-- idx
 * End
 * Drop tmpfilename.
 * Drop tmplocker.
 *
 */
/*
 * Split each file up into the bits we need
 */
/*
 * If there are no files or only 1, there is no need to sort
 * anything
 */
If !global.!repfiles.?fullfilename.!!last_repository.0 > 1 Then
   Do
      Select
         When !global.!repfile_sort_order = 0 Then /* name */
            Do
               Call RegMultiStemSort !global.!repfile_sort_direction.0, !global.!!user.!!preference.!!sort, 5, , ,
                     !global.!repfiles.?filename.!!last_repository., ,
                     !global.!repfiles.?user.!!last_repository., ,
                     !global.!repfiles.?filerev.!!last_repository., ,
                     !global.!repfiles.?filedate.!!last_repository., ,
                     !global.!repfiles.?fullfilename.!!last_repository., ,
                     !global.!repfiles.?filetype.!!last_repository., ,
                     !global.!repfiles.?filestatus.!!last_repository., ,
                     !global.!repfiles.?filereminder.!!last_repository.
            End
         When !global.!repfile_sort_order = 1 Then /* status */
            Do
               Call RegMultiStemSort !global.!repfile_sort_direction.1, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!repfiles.?filestatus.!!last_repository., ,
                  !global.!repfiles.?user.!!last_repository., ,
                  !global.!repfiles.?filerev.!!last_repository., ,
                  !global.!repfiles.?filedate.!!last_repository., ,
                  !global.!repfiles.?fullfilename.!!last_repository., ,
                  !global.!repfiles.?filetype.!!last_repository., ,
                  !global.!repfiles.?filename.!!last_repository., ,
                  !global.!repfiles.?filereminder.!!last_repository.
            End
         When !global.!repfile_sort_order = 2 Then  /* date */
            Do
               Call RegMultiStemSort !global.!repfile_sort_direction.2, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!repfiles.?filedate.!!last_repository., ,
                  !global.!repfiles.?user.!!last_repository., ,
                  !global.!repfiles.?filerev.!!last_repository., ,
                  !global.!repfiles.?filename.!!last_repository., ,
                  !global.!repfiles.?fullfilename.!!last_repository., ,
                  !global.!repfiles.?filetype.!!last_repository., ,
                  !global.!repfiles.?filestatus.!!last_repository., ,
                  !global.!repfiles.?filereminder.!!last_repository.
            End
         When !global.!repfile_sort_order = 3 Then  /* user / locker */
            Do
               Call RegMultiStemSort !global.!repfile_sort_direction.3, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!repfiles.?user.!!last_repository., ,
                  !global.!repfiles.?filename.!!last_repository., ,
                  !global.!repfiles.?filerev.!!last_repository., ,
                  !global.!repfiles.?filedate.!!last_repository., ,
                  !global.!repfiles.?fullfilename.!!last_repository., ,
                  !global.!repfiles.?filetype.!!last_repository., ,
                  !global.!repfiles.?filestatus.!!last_repository., ,
                  !global.!repfiles.?filereminder.!!last_repository.
            End
         When !global.!repfile_sort_order = 4 Then  /* rev */
            Do
               Call RegMultiStemSort !global.!repfile_sort_direction.4, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!repfiles.?filerev.!!last_repository., ,
                  !global.!repfiles.?filename.!!last_repository., ,
                  !global.!repfiles.?user.!!last_repository., ,
                  !global.!repfiles.?filedate.!!last_repository., ,
                  !global.!repfiles.?fullfilename.!!last_repository., ,
                  !global.!repfiles.?filetype.!!last_repository., ,
                  !global.!repfiles.?filestatus.!!last_repository., ,
                  !global.!repfiles.?filereminder.!!last_repository.
            End
         When !global.!repfile_sort_order = 5 Then  /* reminder */
            Do
               Call RegMultiStemSort !global.!repfile_sort_direction.5, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!repfiles.?filereminder.!!last_repository., ,
                  !global.!repfiles.?filename.!!last_repository., ,
                  !global.!repfiles.?user.!!last_repository., ,
                  !global.!repfiles.?filedate.!!last_repository., ,
                  !global.!repfiles.?fullfilename.!!last_repository., ,
                  !global.!repfiles.?filetype.!!last_repository., ,
                  !global.!repfiles.?filestatus.!!last_repository., ,
                  !global.!repfiles.?filerev.!!last_repository.
            End
         Otherwise Nop
      End
   End
Return

SortNonrepFileArray: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Sort into current sort order
 * All field arrays have a 4 digit prefix which is the current sort order;
 * equivalent to the current tail value
 *   Filename         Date
 * ...1a file    ...120001112
 * ...2b file    ...220001110
 * ...3c file    ...320001111
 *
 * Sort on date column ...
 *                    Date
 *               ...220001110
 *               ...320001111
 *               ...120001112
 *
 * Copy Filename stem to tmpfilename.
 * Copy locker   stem to tmplocker.
 * Drop Filename stem
 * Do i = 1 To date.0
 *   idx = Strip(Left(date.i,4))
 *   newi = Right(i,4)
 *
 *   date.i = newi||Substr(date.i,5)              <-- i
 *
 *   !global.!repfiles.?filename.!!last_repository.i = newi||Substr(tmpfilename.idx,5) <-- idx
 *   locker.i = newi||Substr(tmplocker.idx,5)     <-- idx
 * End
 * Drop tmpfilename.
 * Drop tmplocker.
 *
 */
/*
 * Split each file up into the bits we need
 */
/*
 * If there are no files or only 1, there is no need to sort
 * anything
 */
if trace() = 'F' Then say '!global.!nonrepfiles.?fullfilename.!!last_repository.0' !global.!nonrepfiles.?fullfilename.!!last_repository.0
if trace() = 'F' Then say '!global.!nonrepfile_sort_order' !global.!nonrepfile_sort_order
If !global.!nonrepfiles.?fullfilename.!!last_repository.0 > 1 Then
   Do
      Select
         When !global.!nonrepfile_sort_order = 0 Then /* name */
            Do
               Call RegMultiStemSort !global.!nonrepfile_sort_direction.0, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!nonrepfiles.?filename.!!last_repository., ,
                  !global.!nonrepfiles.?filedate.!!last_repository., ,
                  !global.!nonrepfiles.?fullfilename.!!last_repository., ,
                  !global.!nonrepfiles.?filetype.!!last_repository., ,
                  !global.!nonrepfiles.?filesize.!!last_repository., ,
                  !global.!nonrepfiles.?fileperm.!!last_repository.
            End
         When !global.!nonrepfile_sort_order = 1 Then  /* date */
            Do
               Call RegMultiStemSort !global.!nonrepfile_sort_direction.1, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!nonrepfiles.?filedate.!!last_repository., ,
                  !global.!nonrepfiles.?filename.!!last_repository., ,
                  !global.!nonrepfiles.?fullfilename.!!last_repository., ,
                  !global.!nonrepfiles.?filetype.!!last_repository., ,
                  !global.!nonrepfiles.?filesize.!!last_repository., ,
                  !global.!nonrepfiles.?fileperm.!!last_repository.
            End
         When !global.!nonrepfile_sort_order = 2 Then  /* size */
            Do
               Call RegMultiStemSort !global.!nonrepfile_sort_direction.2, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!nonrepfiles.?filesize.!!last_repository., ,
                  !global.!nonrepfiles.?filename.!!last_repository., ,
                  !global.!nonrepfiles.?filedate.!!last_repository., ,
                  !global.!nonrepfiles.?fullfilename.!!last_repository., ,
                  !global.!nonrepfiles.?filetype.!!last_repository., ,
                  !global.!nonrepfiles.?fileperm.!!last_repository.
            End
         When !global.!nonrepfile_sort_order = 3 Then  /* perm */
            Do
               Call RegMultiStemSort !global.!nonrepfile_sort_direction.3, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!nonrepfiles.?fileperm.!!last_repository., ,
                  !global.!nonrepfiles.?filename.!!last_repository., ,
                  !global.!nonrepfiles.?filedate.!!last_repository., ,
                  !global.!nonrepfiles.?fullfilename.!!last_repository., ,
                  !global.!nonrepfiles.?filetype.!!last_repository., ,
                  !global.!nonrepfiles.?filesize.!!last_repository.
            End
         Otherwise Nop
      End
   End
Return

SortAllFileArray: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Sort into current sort order
 * All field arrays have a 4 digit prefix which is the current sort order;
 * equivalent to the current tail value
 *   Filename         Date
 * ...1a file    ...120001112
 * ...2b file    ...220001110
 * ...3c file    ...320001111
 *
 * Sort on date column ...
 *                    Date
 *               ...220001110
 *               ...320001111
 *               ...120001112
 *
 * Copy Filename stem to tmpfilename.
 * Copy locker   stem to tmplocker.
 * Drop Filename stem
 * Do i = 1 To date.0
 *   idx = Strip(Left(date.i,4))
 *   newi = Right(i,4)
 *
 *   date.i = newi||Substr(date.i,5)              <-- i
 *
 *   !global.!allfiles.?filename.!!last_repository.i = newi||Substr(tmpfilename.idx,5) <-- idx
 *   locker.i = newi||Substr(tmplocker.idx,5)     <-- idx
 * End
 * Drop tmpfilename.
 * Drop tmplocker.
 *
 */
/*
 * Split each file up into the bits we need
 */
/*
 * If there are no files or only 1, there is no need to sort
 * anything
 */
If !global.!allfiles.?fullfilename.!!last_repository.0 > 1 Then
   Do
      Select
         When !global.!allfile_sort_order = 0 Then /* name */
            Do
               Call RegMultiStemSort !global.!allfile_sort_direction.0, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!allfiles.?filename.!!last_repository., ,
                  !global.!allfiles.?user.!!last_repository., ,
                  !global.!allfiles.?filerev.!!last_repository., ,
                  !global.!allfiles.?filedate.!!last_repository., ,
                  !global.!allfiles.?fullfilename.!!last_repository., ,
                  !global.!allfiles.?filepath.!!last_repository., ,
                  !global.!allfiles.?filetype.!!last_repository., ,
                  !global.!allfiles.?filestatus.!!last_repository., ,
                  !global.!allfiles.?filereminder.!!last_repository.
            End
         When !global.!allfile_sort_order = 1 Then /* status */
            Do
               Call RegMultiStemSort !global.!allfile_sort_direction.1, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!allfiles.?filestatus.!!last_repository., ,
                  !global.!allfiles.?user.!!last_repository., ,
                  !global.!allfiles.?filerev.!!last_repository., ,
                  !global.!allfiles.?filedate.!!last_repository., ,
                  !global.!allfiles.?fullfilename.!!last_repository., ,
                  !global.!allfiles.?filetype.!!last_repository., ,
                  !global.!allfiles.?filename.!!last_repository., ,
                  !global.!allfiles.?filepath.!!last_repository., ,
                  !global.!allfiles.?filereminder.!!last_repository.
            End
         When !global.!allfile_sort_order = 2 Then  /* date */
            Do
               Call RegMultiStemSort !global.!allfile_sort_direction.2, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!allfiles.?filedate.!!last_repository., ,
                  !global.!allfiles.?user.!!last_repository., ,
                  !global.!allfiles.?filerev.!!last_repository., ,
                  !global.!allfiles.?filename.!!last_repository., ,
                  !global.!allfiles.?fullfilename.!!last_repository., ,
                  !global.!allfiles.?filepath.!!last_repository., ,
                  !global.!allfiles.?filetype.!!last_repository., ,
                  !global.!allfiles.?filestatus.!!last_repository., ,
                  !global.!allfiles.?filereminder.!!last_repository.
            End
         When !global.!allfile_sort_order = 3 Then  /* user / locker */
            Do
               Call RegMultiStemSort !global.!allfile_sort_direction.3, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!allfiles.?user.!!last_repository., ,
                  !global.!allfiles.?filename.!!last_repository., ,
                  !global.!allfiles.?filepath.!!last_repository., ,
                  !global.!allfiles.?filerev.!!last_repository., ,
                  !global.!allfiles.?filedate.!!last_repository., ,
                  !global.!allfiles.?fullfilename.!!last_repository., ,
                  !global.!allfiles.?filetype.!!last_repository., ,
                  !global.!allfiles.?filestatus.!!last_repository., ,
                  !global.!allfiles.?filereminder.!!last_repository.
            End
         When !global.!allfile_sort_order = 4 Then  /* rev */
            Do
               Call RegMultiStemSort !global.!allfile_sort_direction.4, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!allfiles.?filerev.!!last_repository., ,
                  !global.!allfiles.?filename.!!last_repository., ,
                  !global.!allfiles.?filepath.!!last_repository., ,
                  !global.!allfiles.?user.!!last_repository., ,
                  !global.!allfiles.?filedate.!!last_repository., ,
                  !global.!allfiles.?fullfilename.!!last_repository., ,
                  !global.!allfiles.?filetype.!!last_repository., ,
                  !global.!allfiles.?filestatus.!!last_repository., ,
                  !global.!allfiles.?filereminder.!!last_repository.
            End
         When !global.!allfile_sort_order = 5 Then  /* filepath */
            Do
               Call RegMultiStemSort !global.!allfile_sort_direction.5, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!allfiles.?filepath.!!last_repository., ,
                  !global.!allfiles.?filerev.!!last_repository., ,
                  !global.!allfiles.?filename.!!last_repository., ,
                  !global.!allfiles.?user.!!last_repository., ,
                  !global.!allfiles.?filedate.!!last_repository., ,
                  !global.!allfiles.?fullfilename.!!last_repository., ,
                  !global.!allfiles.?filetype.!!last_repository., ,
                  !global.!allfiles.?filestatus.!!last_repository., ,
                  !global.!allfiles.?filereminder.!!last_repository.
            End
         When !global.!allfile_sort_order = 6 Then  /* reminder */
            Do
               Call RegMultiStemSort !global.!allfile_sort_direction.6, !global.!!user.!!preference.!!sort, 5, , ,
                  !global.!allfiles.?filereminder.!!last_repository., ,
                  !global.!allfiles.?filename.!!last_repository., ,
                  !global.!allfiles.?filepath.!!last_repository., ,
                  !global.!allfiles.?user.!!last_repository., ,
                  !global.!allfiles.?filedate.!!last_repository., ,
                  !global.!allfiles.?fullfilename.!!last_repository., ,
                  !global.!allfiles.?filetype.!!last_repository., ,
                  !global.!allfiles.?filestatus.!!last_repository., ,
                  !global.!allfiles.?filerev.!!last_repository.
            End
         Otherwise Nop
      End
   End
Return

/*
 * This function gets the status of all files in the current directory from the
 * repository. It is done this way so that we have 1 call to the repository for all
 * files in the directory, rather than a call to the repository for each file.
 * This approach only works for CVS
 */
DetermineCVSStatusIcons: Procedure Expose !REXXDW. !global. !!last_repository
/*
sm_proc. = 1
sm_proc.0 = !global.!repfiles.?filestatus.!!last_repository.0
*/
If \!global.!offline.!!last_repository Then
   Do
      /*
       * Set all files to 'needs checkout' by default
       * ...and all reminders to blank
       */
      Do smi = 1 To !global.!repfiles.?filestatus.!!last_repository.0
         !global.!repfiles.?filestatus.!!last_repository.smi = Left( !global.!repfiles.?filestatus.!!last_repository.smi, 4 )'fileneedscheckout'
         !global.!repfiles.?filereminder.!!last_repository.smi = Left( !global.!repfiles.?filereminder.!!last_repository.smi, 4 ) || DetermineReminderIcon( Strip( Substr( !global.!repfiles.?fullfilename.!!last_repository.smi, 5 ) ) )
      End
      /*
       * Get the repository status for all files in the current
       * directory, then match the files up with our array
       */
      If !global.!current_dirname.!!last_repository = '' Then dir = '.'
      Else dir = !global.!current_dirname.!!last_repository
      cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q status -l' quote( dir )
      If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
      Address System cmd With Output FIFO '' Error Stem junk.
      Do Queued()
         Parse Pull line
         If Left( line, 5 ) = 'File:' Then
            Do
               Parse Var line 'File: ' sm_fn 'Status: ' status
               -- special test for 'Locally Removed'; remove 'no file' prefix
               If status = 'Locally Removed' Then Parse Var sm_fn . . sm_fn
               If dir = '.' Then
                  sm_ff = Strip( Translate( sm_fn, '', '	' ) ) /* don't use !global.!ossep */
               Else
                  sm_ff = dir'/'Strip( Translate( sm_fn, '', '	' ) ) /* don't use !global.!ossep */
               icon = GetCVSStatusIcon( status )
               Do smj = 1 To !global.!repfiles.?fullfilename.!!last_repository.0
                  file =  Strip( Substr( !global.!repfiles.?fullfilename.!!last_repository.smj, 5 ) )
                  If file = sm_ff Then
                     Do
                        !global.!repfiles.?filestatus.!!last_repository.smj = Left( !global.!repfiles.?filestatus.!!last_repository.smj, 4 ) || icon
                        Leave
                     End
                  End
            End
      End
   End
/*
 * With CVS we have the concept of a "reminder". In the working
 * folder CVS directory is a file; Reminders, containing a list
 * of files for which you want reminders. For each file in this
 * list, that we haven't already processed above, determine if
 * it has a reminder and set the background (only) to yellow
 */
/*
Do smi = 1 to sm_proc.0
   If sm_proc.smi Then
      Do
         !global.!repfiles.?filereminder.!!last_repository.smi = Left( !global.!repfiles.?filereminder.!!last_repository.smi, 4 ) || DetermineReminderIcon( Strip( Substr( !global.!repfiles.?fullfilename.!!last_repository.smi, 5 ) ) )
      End
End
Drop sm_proc.
*/
Return

/*
 * This function gets the status of all files in the current directory from the
 * repository. It is done this way so that we have 1 call to the repository for all
 * files in the directory, rather than a call to the repository for each file.
 * This approach only works for CVS
 */
DetermineCVSStatusIconsAll: Procedure Expose !REXXDW. !global. !!last_repository
/*
sm_proc. = 1
sm_proc.0 = !global.!allfiles.?filestatus.!!last_repository.0
*/
If \!global.!offline.!!last_repository Then
   Do
      /*
       * Set all files to 'needs checkout' by default
       * ...and all reminders to blank
       */
      Do smi = 1 To !global.!allfiles.?filestatus.!!last_repository.0
         !global.!allfiles.?filestatus.!!last_repository.smi = Left( !global.!allfiles.?filestatus.!!last_repository.smi, 4 )'fileneedscheckout'
         !global.!allfiles.?filereminder.!!last_repository.smi = Left( !global.!allfiles.?filereminder.!!last_repository.smi, 4 ) || DetermineReminderIcon( Strip( Substr( !global.!allfiles.?fullfilename.!!last_repository.smi, 5 ) ) )
      End
      /*
       * Get the repository status for all files in the current
       * directory, then match the files up with our array
       */
      If !global.!current_dirname.!!last_repository = '.' | !global.!current_dirname.!!last_repository = '' Then
         Do
            dir = ''
            status_dir = ''
            len = Length( !global.!!repository.!!shortpath.!!last_repository'/' )
         End
      Else
         Do
            dir = !global.!current_dirname.!!last_repository
            status_dir = quote( !global.!current_dirname.!!last_repository )
            len = Length( !global.!!repository.!!shortpath.!!last_repository'/' )
         End
      cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q status' status_dir
      If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
      Address System cmd With Output FIFO '' Error Stem junk.
      Do Queued()
         processfile = 0
         Parse Pull line
         Select
            When Left( line, 5 ) = 'File:' Then
               Do
                  Parse Var line 'File: ' sm_fn '	Status: ' status
               End
            When Countstr( 'Repository revision:', line ) = 1 Then
               Do
                  Parse Var line . '	' . '	' fullfn ',v'
                  processfile = 1
               End
            Otherwise Nop
         End
         If processfile Then
            Do
               sm_ff = Substr( fullfn, len + 1 )
/*
               If dir \= '' Then
                  sm_ff = dir'/'Strip( sm_fn ) -- don't use !global.!ossep
*/
               icon = GetCVSStatusIcon( status )
--say .line status fullfn icon sm_ff
               Do smj = 1 To !global.!allfiles.?fullfilename.!!last_repository.0
                  file =  Strip( Substr( !global.!allfiles.?fullfilename.!!last_repository.smj, 5 ) )
                  If file = sm_ff Then
                     Do
                        !global.!allfiles.?filestatus.!!last_repository.smj = Left( !global.!allfiles.?filestatus.!!last_repository.smj, 4 ) || icon
                        Leave
                     End
                  End
            End
      End
   End
/*
 * With CVS we have the concept of a "reminder". In the working
 * folder CVS directory is a file; Reminders, containing a list
 * of files for which you want reminders. For each file in this
 * list, that we haven't already processed above, determine if
 * it has a reminder and set the background (only) to yellow
 */
/*
Do smi = 1 to sm_proc.0
   If sm_proc.smi Then
      Do
         !global.!allfiles.?filereminder.!!last_repository.smi = Left( !global.!allfiles.?filereminder.!!last_repository.smi, 4 ) || DetermineReminderIcon( Strip( Substr( !global.!allfiles.?fullfilename.!!last_repository.smi, 5 ) ) )
      End
End
Drop sm_proc.
*/
Return

/*
 * This function gets the status of all files in the current directory from the
 * repository. It is done this way so that we have 1 call to the repository for all
 * files in the directory, rather than a call to the repository for each file.
 * This approach only works for HG
 */
DetermineHGStatusIcons: Procedure Expose !REXXDW. !global. !!last_repository
/*
sm_proc. = 1
sm_proc.0 = !global.!repfiles.?filestatus.!!last_repository.0
*/
      /*
       * Set all files to 'needs checkout' by default
       * ...and all reminders to blank
       */
Do smi = 1 To !global.!repfiles.?filestatus.!!last_repository.0
   icon = GetHGStatusIcon( status )
say .line icon
   !global.!repfiles.?filestatus.!!last_repository.smi = Left( !global.!repfiles.?filestatus.!!last_repository.smi, 4 ) || icon
   !global.!repfiles.?filereminder.!!last_repository.smi = Left( !global.!repfiles.?filereminder.!!last_repository.smi, 4 ) || DetermineReminderIcon( Strip( Substr( !global.!repfiles.?fullfilename.!!last_repository.smi, 5 ) ) )
End
Return

/*
 * This function gets the status of all files in the current directory from the
 * repository. It is done this way so that we have 1 call to the repository for all
 * files in the directory, rather than a call to the repository for each file.
 * This approach only works for HG
 */
DetermineHGStatusIconsAll: Procedure Expose !REXXDW. !global. !!last_repository
/*
sm_proc. = 1
sm_proc.0 = !global.!allfiles.?filestatus.!!last_repository.0
*/
If \!global.!offline.!!last_repository Then
   Do
      /*
       * Set all files to 'needs checkout' by default
       * ...and all reminders to blank
       */
      Do smi = 1 To !global.!allfiles.?filestatus.!!last_repository.0
         !global.!allfiles.?filestatus.!!last_repository.smi = Left( !global.!allfiles.?filestatus.!!last_repository.smi, 4 )'fileneedscheckout'
         !global.!allfiles.?filereminder.!!last_repository.smi = Left( !global.!allfiles.?filereminder.!!last_repository.smi, 4 ) || DetermineReminderIcon( Strip( Substr( !global.!allfiles.?fullfilename.!!last_repository.smi, 5 ) ) )
      End
      /*
       * Get the repository status for all files in the current
       * directory, then match the files up with our array
       */
      If !global.!current_dirname.!!last_repository = '.' | !global.!current_dirname.!!last_repository = '' Then
         Do
            dir = ''
            status_dir = ''
            len = Length( !global.!!repository.!!shortpath.!!last_repository'/' )
         End
      Else
         Do
            dir = !global.!current_dirname.!!last_repository
            status_dir = quote( !global.!current_dirname.!!last_repository )
            len = Length( !global.!!repository.!!shortpath.!!last_repository'/' )
         End
      cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q status' status_dir
      If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
      Address System cmd With Output FIFO '' Error Stem junk.
      Do Queued()
         processfile = 0
         Parse Pull line
         Select
            When Left( line, 5 ) = 'File:' Then
               Do
                  Parse Var line 'File: ' sm_fn '	Status: ' status
               End
            When Countstr( 'Repository revision:', line ) = 1 Then
               Do
                  Parse Var line . '	' . '	' fullfn ',v'
                  processfile = 1
               End
            Otherwise Nop
         End
         If processfile Then
            Do
               sm_ff = Substr( fullfn, len + 1 )
/*
               If dir \= '' Then
                  sm_ff = dir'/'Strip( sm_fn ) -- don't use !global.!ossep
*/
               icon = GetHGStatusIcon( status )
--say .line status fullfn icon sm_ff
               Do smj = 1 To !global.!allfiles.?fullfilename.!!last_repository.0
                  file =  Strip( Substr( !global.!allfiles.?fullfilename.!!last_repository.smj, 5 ) )
                  If file = sm_ff Then
                     Do
                        !global.!allfiles.?filestatus.!!last_repository.smj = Left( !global.!allfiles.?filestatus.!!last_repository.smj, 4 ) || icon
                        Leave
                     End
                  End
            End
      End
   End
/*
 * With HG we have the concept of a "reminder". In the working
 * folder HG directory is a file; Reminders, containing a list
 * of files for which you want reminders. For each file in this
 * list, that we haven't already processed above, determine if
 * it has a reminder and set the background (only) to yellow
 */
/*
Do smi = 1 to sm_proc.0
   If sm_proc.smi Then
      Do
         !global.!allfiles.?filereminder.!!last_repository.smi = Left( !global.!allfiles.?filereminder.!!last_repository.smi, 4 ) || DetermineReminderIcon( Strip( Substr( !global.!allfiles.?fullfilename.!!last_repository.smi, 5 ) ) )
      End
End
Drop sm_proc.
*/
Return

DisplayFileArray: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Delete everything in the container so we can replace the existing files
 */
Call dw_signal_disconnect !global.!repfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_SELECT_EVENT
Call dw_signal_disconnect !global.!repfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_ENTER_EVENT
Call dw_signal_disconnect !global.!repfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_CONTEXT_EVENT
Call dw_signal_disconnect !global.!repfilescontainer.!!last_repository, !REXXDW.!DW_COLUMN_CLICK_EVENT
Call dw_container_clear !global.!repfilescontainer.!!last_repository, !REXXDW.!DW_DONT_REDRAW
/*
 * Add the files into the container
 */
!global.!repfilescontainermemory.!!last_repository = dw_container_alloc( !global.!repfilescontainer.!!last_repository, !global.!repfiles.?fullfilename.!!last_repository.0 )
Do i = 1 To !global.!repfiles.?fullfilename.!!last_repository.0
   Call SetRepFileRow i, 'initial'
End
Call dw_container_set_stripe !global.!repfilescontainer.!!last_repository, !REXXDW.!DW_CLR_WHITE, !global.!container_colour
Call dw_container_insert !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, !global.!repfiles.?fullfilename.!!last_repository.0
Call dw_container_optimize !global.!repfilescontainer.!!last_repository
/*
 * Only now can we connect our signals, because dw_set_file() causes the signals to fire!!!...
 */
Call dw_signal_connect !global.!repfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_SELECT_EVENT, 'FileSelectCallback'
Call dw_signal_connect !global.!repfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_ENTER_EVENT, 'FileDoubleClickCallback'
Call dw_signal_connect !global.!repfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_CONTEXT_EVENT, 'FileMenuCallback'
Call dw_signal_connect !global.!repfilescontainer.!!last_repository, !REXXDW.!DW_COLUMN_CLICK_EVENT, 'FileSortCallback'
Return

DisplayAllFileArray: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Delete everything in the container so we can replace the existing files
 */
Call dw_signal_disconnect !global.!allfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_SELECT_EVENT
Call dw_signal_disconnect !global.!allfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_ENTER_EVENT
Call dw_signal_disconnect !global.!allfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_CONTEXT_EVENT
Call dw_signal_disconnect !global.!allfilescontainer.!!last_repository, !REXXDW.!DW_COLUMN_CLICK_EVENT
Call dw_container_clear !global.!allfilescontainer.!!last_repository, !REXXDW.!DW_DONT_REDRAW
/*
 * Add the files into the container
 */
!global.!allfilescontainermemory.!!last_repository = dw_container_alloc( !global.!allfilescontainer.!!last_repository, !global.!allfiles.?fullfilename.!!last_repository.0 )
Do i = 1 To !global.!allfiles.?fullfilename.!!last_repository.0
   Call SetAllRepFileRow i, 'initial'
End
Call dw_container_set_stripe !global.!allfilescontainer.!!last_repository, !REXXDW.!DW_CLR_WHITE, !global.!container_colour
Call dw_container_insert !global.!allfilescontainer.!!last_repository, !global.!allfilescontainermemory.!!last_repository, !global.!allfiles.?fullfilename.!!last_repository.0
Call dw_container_optimize !global.!allfilescontainer.!!last_repository
/*
 * Only now can we connect our signals, because dw_set_file() causes the signals to fire!!!...
 */
Call dw_signal_connect !global.!allfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_SELECT_EVENT, 'FileSelectCallback'
Call dw_signal_connect !global.!allfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_ENTER_EVENT, 'FileDoubleClickCallback'
Call dw_signal_connect !global.!allfilescontainer.!!last_repository, !REXXDW.!DW_ITEM_CONTEXT_EVENT, 'FileMenuCallback'
Call dw_signal_connect !global.!allfilescontainer.!!last_repository, !REXXDW.!DW_COLUMN_CLICK_EVENT, 'FileSortCallback'
Return

/*
 * Determine the CVS file icons based on the CVS repository status
 */
GetCVSStatusIcon: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg status
status = Strip( status )
Select
   When status = 'Up-to-date' Then sm_fore = 'fileuptodate'
   When status = 'Needs Patch' Then sm_fore = 'fileneedspatch'
   When status = 'Needs Merge' Then sm_fore = 'fileneedsmerge'
   When status = 'Needs Checkout' | status = '' Then sm_fore = 'fileneedscheckout'
   When status = 'Locally Modified' Then sm_fore = 'filemodified'
   When status = 'Locally Removed' Then sm_fore = 'deletefromrepository'
   When status = 'File had conflicts on merge' Then sm_fore = 'fileconflict'
   When status = 'Unresolved Conflict' Then sm_fore = 'fileconflict'
   When Word( status, 1 ) = 'Needs' Then sm_fore = 'red'
   Otherwise sm_fore = 'grey'
End
Return sm_fore

/*
 * Determine the HG file icons based on the HG repository status
 */
GetHGStatusIcon: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg status
status = Strip( status )
Select
   When status = 'C' Then sm_fore = 'fileuptodate'
--   When status = 'Needs Patch' Then sm_fore = 'fileneedspatch'
--   When status = 'Needs Merge' Then sm_fore = 'fileneedsmerge'
   When status = '!' Then sm_fore = 'fileneedscheckout'
   When status = 'M' Then sm_fore = 'filemodified'
   When status = 'R' Then sm_fore = 'deletefromrepository'
--   When status = 'File had conflicts on merge' Then sm_fore = 'fileconflict'
--   When status = 'Unresolved Conflict' Then sm_fore = 'fileconflict'
--   When Word( status, 1 ) = 'Needs' Then sm_fore = 'red'
   Otherwise sm_fore = 'fileunknown'
End
Return sm_fore

/*
 * Determines the icon to display in the 'Type' column.
 */
DetermineFileType: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg type, user, locks
type = Strip( type )
user = Strip( user )
locks = Strip( locks )
icon = ''
Select
   When type = 'b' Then icon = 'binary'
   When type = '?' Then icon = 'fileunknown'
   Otherwise icon = 'text'
End
If locks \= '' Then
   Do
      If Translate( user ) = Translate( locks ) Then icon = icon'locked'
      Else icon = icon'lockedother'
   End
say .line icon type
Return icon

CVSFileToWorkingFile: Procedure Expose !REXXDW. !global. !!last_repository
Parse arg rfile, idx
Return Changestr( '/', !global.!!repository.!!working_dir.idx||rfile, !global.!ossep )

CVSFileToReminderFile: Procedure Expose !REXXDW. !global. !!last_repository
Parse arg rfile
rfile = !global.!!repository.!!working_dir.!!last_repository||rfile
pos = Lastpos( '/', rfile )
rfile = Insert( 'CVS/', rfile, pos )
Return Changestr( '/', rfile'.qocca.reminder', !global.!ossep )

CVSStatus: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg file
cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'status' quote( file )
If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
Address Command cmd With Output Stem lines. Error Stem junk.
If rc \= 0 Then Return ''
equalline = 0
status = ''
Do i = 1 To lines.0
   Select
      When equalline = 1 Then
         Do
            equalline = 0
            Parse Var lines.i 'File:' . 'Status:' status
            Leave
         End
      Otherwise
         Do
            If lines.i = Copies( '=', 67 ) Then equalline = 1
         End
   End
End
Return Strip( status )

TidyUpCVSDeletions: Procedure Expose !REXXDW. !global. !!last_repository oserr.
fidx = 0
didx = 0
Do tui = 1 To oserr.0
   /*
    * Check if file is deleted from repository
    */
   If Right( oserr.tui, 30 ) = 'is no longer in the repository' Then
      Do
         Parse Var oserr.tui 'cvs' . ':' fn 'is no longer in the repository' .
         fn = Strip( fn )
         If fn \= '' Then
         Do
            fidx = fidx + 1
            deleted_file.fidx = fn
            Iterate
         End
      End
   If Right( oserr.tui, 29 ) = 'is not (any longer) pertinent' Then
      Do
         Parse Var oserr.tui 'cvs' . ': warning:' fn 'is not (any longer) pertinent' .
         fn = Strip( fn )
         If fn \= '' Then
         Do
            fidx = fidx + 1
            deleted_file.fidx = fn
            Iterate
         End
      End
   /*
    * Check if directory is deleted from repository
    */
   Parse Var oserr.tui 'cvs' . ': cannot open directory' dn ': No such' junk .
   dn = Strip( dn )
   If dn \= '' & junk \= '' Then
      Do
         didx = didx + 1
         deleted_dir.didx = dn
      End
End
If fidx = 0 & didx = 0 Then Return
/*
 * We have at least one deleted file or directory, display a dialog box confirming the tidy up
 * and display the details of the files moved in the log pane
 */
deleted_file.0 = fidx
deleted_dir.0 = didx
If deleted_file.0 \= 0 Then
   Do
      Call WriteMessageHeaderToLogWindow 'Files removed from repository in' quote( !global.!current_dirname.!!last_repository )
      Do tui = 1 To deleted_file.0
         Call WriteMessageToLogWindow '  'deleted_file.tui, 'redfore'
      End
      Call WriteMessageTrailerToLogWindow 'Files removed from repository in' quote( !global.!current_dirname.!!last_repository )
   End
If deleted_dir.0 \= 0 Then
   Do
      Call WriteMessageHeaderToLogWindow 'Directories removed from repository'
      Do tui = 1 To deleted_dir.0
         Call WriteMessageToLogWindow '  'deleted_dir.tui, 'redfore'
      End
      Call WriteMessageTrailerToLogWindow 'Directories removed from repository'
   End
If dw_messagebox( 'Cleanup?', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_QUESTION ), 'The files and/or directories listed in the messages pane have been removed from the repository. Do you want your working area cleaned up ?' ) = !REXXDW.!DW_MB_RETURN_YES Then
   Do
      /*
       * Delete the file entries from the appropriate CVS/Entries file
       */
if trace() = 'F' Then say 'cleaning up'
      Do tui = 1 To deleted_file.0
         pos = Lastpos( '/', deleted_file.tui )
         entries_fn = Left( deleted_file.tui, pos-1 ) || '/CVS/Entries'
         deleted_fn = Substr( deleted_file.tui, pos+1 )
         entries.0 = 0
         Call RegStemRead entries_fn, 'entries.'
         Call Stream entries_fn, 'C', 'OPEN WRITE REPLACE'
         Do idx = 1 To entries.0
            If Left( entries.idx, 1 ) = '/' Then
               Do
                  Parse Var entries.idx '/' filename '/' .
                  If filename \= deleted_fn Then Call Lineout entries_fn,entries.idx
               End
            Else Call Lineout entries_fn,entries.idx
         End
         Call Stream entries_fn, 'C', 'CLOSE'
         Drop entries.
      End
      /*
       * Delete the directory entries from the appropriate CVS/Entries file
       */
      Do tui = 1 To deleted_dir.0
         pos = Lastpos( '/', deleted_dir.tui )
         entries_dn = Left( deleted_dir.tui, pos-1 )
         entries_dn = Changestr( !global.!!repository.!!shortpath.!!last_repository'/', entries_dn, '' )
         entries_dn = !global.!!repository.!!working_dir.!!last_repository || entries_dn || '/CVS/Entries'
         deleted_dn = Substr( deleted_dir.tui, pos+1 )
         entries.0 = 0
         Call RegStemRead entries_dn, 'entries.'
         Call Stream entries_dn, 'C', 'OPEN WRITE REPLACE'
         Do idx = 1 To entries.0
            If Left( entries.idx, 1 ) = 'D' Then
               Do
                  Parse Var entries.idx 'D/' dirname '/' .
                  If dirname \= deleted_dn Then Call Lineout entries_dn,entries.idx
               End
            Else Call Lineout entries_dn,entries.idx
         End
         Call Stream entries_dn, 'C', 'CLOSE'
         Drop entries.
      End
   End
Drop deleted_file.
Drop deleted_dir.
Return

Time_tDateToDisplayDate: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg time_t
date = Date( 'N', time_t, 'T' )
Return date ' ' Time( 'N', time_t, 'T' )

SortedDateToDisplayDate: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg date
Return Substr( date, 7, 2 )'-'Substr( date, 5, 2 )'-'Substr( date, 1, 4 )

/*
 * Convert a date and time to a combined time_t, allowing for UTC if required
 */
DateTimeToTime_T: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg day, time, fmt, in_utc
If in_utc Then to = Time( 'O' ) / 1000000
Else to = 0
day_t = Date( 'T', day, fmt ) + (Time('O') / 1000000) -- always adjust date to localtime
time_t = Time( 'T', time, 'N' )
today_t = Date( 'T', Date(), 'N' )
Return (time_t - today_t) + day_t + to

/*
 * Tests if the filename passsed as an argument has an extension in the
 * configured list of extensions to ignore
 */
IsFileTypeIgnored: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg fn
Do i = 1 To Words( !global.!ignorefiles )
   ext = '.'Word( !global.!ignorefiles, i )
   len = Length( ext )
   If Right( fn, len ) = ext Then Return 1
End
Return 0

UseFileAssociation: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg fn
pos = Lastpos( '.', fn )
ext = Substr( fn, pos + 1 )
If Wordpos( ext, !global.!!user.!!preference.!!windowsfileassociations_extensions ) = 0 Then Return 0
Else Return 1

/*
 * Tests if the given filename is binary; ie there is at least one
 * nul character in the first 1024 characters.
 */
IsBinaryFile: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg fn
If  Stream( fn, 'C', 'QUERY EXISTS' ) = '' Then Return 0
/*
 * If the repository's list of binary file extensions match our file...
 */
pos = Lastpos( '.', fn )
ext = Translate( Substr( fn, pos+1 ) )
If Wordpos( ext, Translate( !global.!binaryfiles ) ) \= 0 Then Return 1
/*
 * Not listed so check contents
 */
size = Stream( fn, 'C', 'QUERY SIZE' )
numchars = Min( size, 1024 )
Call Stream fn, 'C', 'OPEN READ'
file_block = Charin( fn, 1, numchars )
test_block = Translate( file_block, '#', '00'x )
If file_block \== test_block Then
   binary = 1
Else
   binary = 0
Call Stream fn, 'C', 'CLOSE'
Return binary

/*
 * Determines if the specified file is in the repository
 * Does not require a working file
 */
IsFileInRepository: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg fn
found = 0
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q co -p' quote( fn )
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address System cmd With Output Stem out. Error Stem err.
         If err.0 = 0 Then found = 1
      End
   Otherwise Nop
End
Return found

/*
 * Determines if the module is in the repository
 */
IsModuleInRepository: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg fn, repno
fn = Strip( fn )
found = 0
Select
   When !global.!!repository.!!type.repno = 'cvs' Then
      Do
         cmd = 'cvs' !global.!!repository.!!cvs_compression.repno '-d'!global.!!repository.!!path.repno 'rdiff -s -D 1/1/2037' fn
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address System cmd With Error Stem err. Output Stem junk.
         Do i = 1 To err.0
            Parse Var err.i . 'Diffing' line
            If Strip( line ) = fn Then
               Do
                  Drop err. junk.
                  found = 1
                  Leave
               End
         End
      End
   Otherwise Nop
End
Return found

/*
 * Checks if a particular file can be checked out for locking
 */
CanFileBeCheckedOutForLock: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg i, dir_reserved
If dir_reserved | Substr( !global.!repfiles.?filetype.!!last_repository.i, 5, 6 ) = 'binary' Then
   Do
      Parse Var !global.!repfiles.?user.!!last_repository.i . 5 . '(' locker ')'
      If Strip( locker ) = '' Then Return 1
   End
Return 0

GetCVSLocker: /* MUST NOT BE A PROCEDURE */
Parse Arg fn
locks = ''
cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q list' quote( fn )
If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
Address Command cmd With Output Stem item. Error Stem err.
If item.0 = 1 Then
   Do
      Parse Var item.1 . 88 flen 91 . 92 . +(flen) +1 locks
   End
Return locks

SendEmail: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg emailcommand, buildout, admin_email, subject
If emailcommand = '' Then Return
If Countstr( '%F%', emailcommand ) = 0 | Countstr( '%R%', emailcommand ) = 0 | Countstr( '%S%', emailcommand ) = 0 Then
   Do
      Say 'Malformed email command template:' emailcommand
      Return
   End
se_cmd = Changestr( '%F%', emailcommand, buildout )
se_cmd = Changestr( '%S%', se_cmd, subject )
Do i = 1 To Words( admin_email )
   Address System Changestr( '%R%', se_cmd, Word( admin_email, i) )
End
Return

FileIsInRepository: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg file
Do i = 1 To !global.!repfiles.?fullfilename.!!last_repository.0
   If file = Substr( !global.!repfiles.?fullfilename.!!last_repository.i, 5 ) Then Return 1
End
Return 0

/*
 * Determine the index of the supplied directory in the list of directories
 * in the repository
 */
FindTreeItem: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg item
Do i = 1 To !global.!repdir.!!last_repository.0
   If !global.!repdir.!!last_repository.i = item Then Return i
End
Return 0
/*
Return RegStemSearch( item, '!global.!repdir.!!last_repository.', 1, 'CES' )
*/

/*
 * Generic window for obtaining textual comments
 */
GetCheckinDetails: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg want_text_win, type, file, question, title
if trace() = 'F' Then say 'want_text_win' want_text_win 'type' type 'file' file 'question' question
_version = ''
same = 0
win = dw_window_new( !REXXDW.!DW_DESKTOP, title file, !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon
gf = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, gf, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

If want_text_win = 'Y' Then
   Do
      Select
         When type = 'add' Then
            Do
               text_height = 20
               junk = dw_text_new( 'Module Description:', 0 )
               Call dw_window_set_style junk, !REXXDW.!DW_DT_VCENTER , !REXXDW.!DW_DT_VCENTER
               Call dw_box_pack_start gf, junk, !REXXDW.!DW_SIZE_AUTO, text_height, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
               If Left( !global.!addfile_text, 1 ) = '!' Then !global.!addfile_text = ''
               t1 = dw_entryfield_new( !global.!addfile_text, 0 )
               Call dw_box_pack_start gf, t1, 400, text_height, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
            End
         When type = 'remind' Then
            Do
               text_height = 300
               t1 = dw_mle_new( 0 )
               Call dw_box_pack_start gf, t1, 400, text_height, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
               Call dw_mle_set_editable t1, !REXXDW.!DW_EDITABLE
               loc = dw_mle_import( t1, question||!global.!remindertext, -1 )
               Call dw_mle_set_cursor t1, loc
            End
         Otherwise /* checkin */
            Do
               text_height = 300
               t1 = dw_mle_new( 0 )
               Call dw_box_pack_start gf, t1, 400, text_height, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
               Call dw_mle_set_editable t1, !REXXDW.!DW_EDITABLE
               If !global.!checkin_text \= '!GLOBAL.!CHECKIN_TEXT' Then
                  Do
                     loc = dw_mle_import( t1, !global.!checkin_text, -1 )
                     Call dw_mle_set_cursor t1, loc
                  End
         End
      End
   End
Call dw_window_default win, t1
cb_height = 0
If type = 'dir' | type = 'delete' | type = 'add' Then
   Do
      cb_height = 20
      cb = dw_checkbox_new( question, 0 )
      Call dw_box_pack_start gf, cb, !REXXDW.!DW_SIZE_AUTO, cb_height, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   End
Else cb = 0

/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start gf, b1, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start gf, b2, !REXXDW.!DW_SIZE_AUTO, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

/*
 * Display the window
 */
Call dw_window_set_gravity win, !REXXDW.!DW_GRAV_CENTRE, !REXXDW.!DW_GRAV_CENTRE
Call dw_window_set_pos_size win, 0, 0, 0, 0
Call dw_window_show win

Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GetCheckinDetailsApplyCallback', win, dialog_wait, t1, cb, want_text_win, type
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   Parse Value dw_dialog_wait( dialog_wait ) With cancel same
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
Return cancel same

GetFileRevisions: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Dialog box to ask user for file revision to work on
 * Only one file allowed to be selected for this option
 *
 * +---------------------------------------------------+
 * | Text                                              |
 * |                                                   |
 * +---------------------------------------------------+
 * | ____________Latest______________________________v |
 * +---------------------------------------------------+
 */
Parse Arg this_file, prompt, types, allow_date, default
Call SetCursorWait
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Select File Revision for:' prompt, !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon

gf = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, gf, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

t2 = dw_text_new( 'Select the revision of the file to' prompt'.', 0 )
Call dw_box_pack_start gf, t2, 250, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

boxtopack = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start gf, boxtopack, 250, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Parse Value CreateFileRevisionCombobox( boxtopack, 'false', default, this_file, '', types, allow_date ) With revcbox .
/*
 * Create a dialog for the window
 */
dialog_wait = dw_dialog_new( )
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start gf, b1, 250, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GetFileRevisionsApplyCallback', win, dialog_wait, revcbox

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start gf, b2, 250, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * We now have everything setup, we can connect the destroying of the top-level window
 * to the 'close' callback
*/
Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * Display the window
 */
width = 400
height = 160
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
/*
 * Wait for the dialog to come back
 */
Parse Value dw_dialog_wait( dialog_wait ) With cancel rev
if trace() = 'F' Then say 'got rev' cancel rev
Return rev


CreateFileRevisionCombobox: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg boxtopack, editable, default_value, filename, extra_revisions, types, allow_date
realtagstart = 0
/*
 * Create generic combobox for file revisions
 */
revcbox = dw_combobox_new( default_value, 0 )
If default_value \= '' Then
   Do
      Call dw_listbox_append revcbox, default_value
      realtagstart = realtagstart + 1
   End
Call dw_box_pack_start boxtopack, revcbox, 150, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
/*
 * Add any special revisions first...
 */
Do i = 1 To Words( extra_revisions )
   Call dw_listbox_append revcbox, Word( extra_revisions, i )
   realtagstart = realtagstart + 1
End
/*
 * Now find the existing revisions for the file and add them to the combobox
 */
Call PopulateFileRevisionCombobox revcbox, !!last_repository, filename, types
/*
 * Create and pack our date selector
 */
If allow_date = 'date' Then
   Do
      abutton = dw_bitmapbutton_new_from_file( 'Select Date', 0, !global.!icondir||'dateselector' )
      Call dw_box_pack_start boxtopack, abutton, 20, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GetFileRevisionsDateSelector', revcbox
   End
/*
If editable = 'false' Then Call dw_window_disable revcbox
*/
Return revcbox realtagstart

PopulateFileRevisionCombobox: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg revcbox, repno, filename, types
/*
 * Let the user know whats happening...
 */
Call SetCursorWait
Call DisplayStatus 'Getting revision details for' filename
Select
   When !global.!!repository.!!type.repno = 'rcs' Then
      Do
      /* TODO */
         Address System 'rlog' quote( filename ) With Output FIFO '' Error Stem err.
         gotrev = 0
         Do Queued()
            Select
               When gotrev = 1 Then
                  Do
                     Parse Pull 'revision' newrev .
                     If newrev \= '' Then
                        Do
                           Call TkComboboxListInsert revcbox, 'end', newrev
                        End
                     gotrev = 0
                  End
               Otherwise
                  Do
                     Parse Pull line
                     If line = Copies( '-', 28 ) Then gotrev = 1
                  End
            End
         End
      End
   When !global.!!repository.!!type.repno = 'cvs' Then
      Do
         Call GetCVSRevisions repno, filename, types
         idx = 0
         Do Queued()
            idx = idx + 1
            Parse Pull text.idx
         End
         text.0 = idx
         Call dw_listbox_list_append revcbox, 'text.'
         Drop text.
      End
   Otherwise
End
Call SetCursorNoWait
Call DisplayStatus ''
/*
 * Save the repository number for the combobox
 */
!global.!repository_combobox.revcbox = repno
Return

CreateModuleVersionCombobox: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, editable, default_value, dirname, tryparent, extra_versions, associated_cb
realtagstart = 0
/*
 * Let the user know whats happening...
 */
Call SetCursorWait
Call DisplayStatus 'Getting version details for' dirname
/*
 * Create a box to pack the combobox and button for date selection
 */
boxtopack = dw_box_new( !REXXDW.!DW_HORZ )
/*
 * Create generic combobox for module versions
 */
revcbox = dw_combobox_new( default_value, 0 )
If default_value \= '' Then
   Do
      Call dw_listbox_append revcbox, default_value
      realtagstart = realtagstart + 1
   End
Call dw_box_pack_start boxtopack, revcbox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
/*
 * Add any special versions first...
 */
Do i = 1 To Words( extra_versions )
   Call dw_listbox_append revcbox, Word( extra_versions, i )
   realtagstart = realtagstart + 1
End
/*
 * Now find label files for this module and add them to the combobox
 */
Call PopulateModuleRevisionCombobox revcbox, !!last_repository, tryparent
Call SetCursorNoWait
Call DisplayStatus ''

Return revcbox boxtopack realtagstart

PopulateModuleRevisionCombobox: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg revcbox, repno, tryparent
Select
   When !global.!!repository.!!type.repno = 'rcs' Then
      Do
         Call SysFileTree !global.!current_dirname.!!last_repository'/.label-*,v', 'LABELFILE.', 'FLS'
      End
   When !global.!!repository.!!type.repno = 'cvs' Then
      Do
         /*
          * NOTE We can't use ?label. stem here, as we need to traverse up the
          * directory tree, and we only have the current directory labels.
          */
         cdir = !global.!current_dirname.!!last_repository
         Do Forever
            cmd = 'cvs' !global.!!repository.!!cvs_compression.repno '-d'!global.!!repository.!!path.repno '-q list -l' quote( cdir )
            If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
            Address System cmd With Output FIFO '' Error Stem err.
            Do Queued()
               Parse Pull line . . . . . . . '/.label-' lab
               If lab \= '' Then
                  Do
                     Call dw_listbox_append revcbox, cdir','lab
                  End
            End
            If Countstr( '/', cdir ) = 0 | tryparent = 0 Then Leave
            Parse Var cdir cdir '/' .
         End
      End
   Otherwise Nop
End
/*
 * Save the repository number for the combobox
 */
!global.!repository_combobox.revcbox = repno
Return

CreateRepositoryCombobox: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Create generic combobox for repositories
 */
Parse Arg win, editable, default_value, associated_cb, realtagstart, filename, types
repcbox = dw_combobox_new( default_value, 0 )
If default_value \= '' Then
   Do
      Call dw_listbox_append repcbox, default_value
      Call dw_listbox_select repcbox, 0, !REXXDW.!DW_LB_SELECTED
   End
/*
 * Now insert all repositories into the combobox
 */
Do i = 1 To !global.!!number_repositories
   Call dw_listbox_append repcbox, !global.!!repository.!!name.i
End
/*
 * If we have a parameter for an associated listbox, we need to set up a signal to
 * repopulate the associated listbox data
 */
If associated_cb \= '' Then
   Do
      Call dw_signal_connect repcbox, !REXXDW.!DW_LIST_SELECT_EVENT, 'RepositoryComboboxCallback', associated_cb, realtagstart, filename, types
   End
Return repcbox

RepositoryComboboxCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, list_idx, associated_cb, realtagstart, filename, types
/*
 * Get the selected repository from the combobox
 */
text = dw_listbox_get_text( win, list_idx )
Call dw_window_set_text win, text
/*
 * Convert this repository into an index...
 */
repno = FindRepository( text )
If repno = !global.!repository_combobox.associated_cb Then Return 1
If repno = 0 Then repno = !!last_repository
/*
 * Now repopulate the associated_cb
 */
count = dw_listbox_count( associated_cb )
/*
Do i = count-1 To realtagstart-1 By -1
   Call dw_listbox_delete associated_cb, i
End
*/
idx = realtagstart - 1
Do i = realtagstart To count
   Call dw_listbox_delete associated_cb, idx
End

If filename = '' Then Call PopulateModuleRevisionCombobox associated_cb, repno, types
Else Call PopulateFileRevisionCombobox associated_cb, repno, filename, types

Return 1 /* indicate we have handled the event */

CreateCompilersCombobox: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg win, editable, default_abbrev
default_value = ''
/*
 * If supplied, find the name of the default abbrev
 */
Do i = 1 To Words( !global.!compiler_template.??compiler_types )
   tmp = Word( !global.!compiler_template.??compiler_types, i )
   If tmp = default_abbrev Then
      Do
         default_value = !global.!compiler_template.??compiler_name.tmp
         Leave
      End
End
repcbox = dw_combobox_new( default_value, 0 )
If default_value \= '' Then Call dw_listbox_append repcbox, default_value
/*
 * Now insert all compilers into the combobox
 */
Do i = 1 To Words( !global.!compiler_template.??compiler_types )
   tmp = Word( !global.!compiler_template.??compiler_types, i )
   Call dw_listbox_append repcbox, !global.!compiler_template.??compiler_name.tmp
End
Return repcbox

ConvertCompilerNameToAbbrev: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg compiler_name
compiler_type = '*unknown*'
Do idx = 1 To Words( !global.!compiler_template.??compiler_types )
   tmp = Word( !global.!compiler_template.??compiler_types, idx )
   If !global.!compiler_template.??compiler_name.tmp = compiler_name Then
      Do
         compiler_type = tmp
         Leave
      End
End
Return compiler_type

/*
 * Find all revisions for the supplied file, and queue them onto the stack
 */
GetCVSRevisions: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg repno, this_file, types
/*
 * Use another stack as stems are inefficient under Regina (as at 3.3)
 */
sessionq = RxQueue( 'G' )
tmpq = RxQueue( 'C' )
If !global.!!repository.!!enhanced_cvs.!!last_repository \= 'Y' Then
   cvscmd = 'log'
Else
   cvscmd = 'rlog'
cmd = 'cvs' !global.!!repository.!!cvs_compression.repno '-d'!global.!!repository.!!path.repno cvscmd quote( this_file )
If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
Address Command cmd With Output FIFO tmpq Error Stem oserr.
osout.0 = 0
If oserr.0 \= 0 Then Call WriteOutputStemsToLogWindow 'default', 'Getting revisions' this_file
If Wordpos( 'rev', types ) \= 0 Then getrev = 1
Else getrev = 0
If Wordpos( 'tag', types ) \= 0 Then gettag = 1
Else gettag = 0
gotrev = 0
gottag = 0
revs = ''
need_pull = 1
Call RxQueue 'S', tmpq
num_queued = Queued()
num_pulled = 0
If num_queued \= 0 Then
   Do Forever
      If need_pull Then
         Do
            Parse Pull rcsrev
            num_pulled = num_pulled + 1
         End
      Select
         When gotrev = 1 Then
            Do
               need_pull = 1
               Parse Var rcsrev 'revision' newrev .
               If newrev \= '' Then
                  Do
                     Parse Pull rcsrev
                     num_pulled = num_pulled + 1
                     need_pull = 0
                     If Countstr( 'state: dead;', rcsrev ) \= 1 Then
                        Do
                           Call RxQueue 'S', sessionq
                           Queue newrev
                           Call RxQueue 'S', tmpq
                        End
                  End
               gotrev = 0
            End
         When gottag = 1 Then
            Do
               If SubWord( rcsrev, 1, 2 ) = 'keyword substitution:' & gettag Then gottag = 0
               Else
                  Do
                     Parse Var rcsrev newtag ':' .
                     Call RxQueue 'S', sessionq
                     Queue Strip( newtag, 'L', '09'x )
                     Call RxQueue 'S', tmpq
                  End
               need_pull = 1
            End
         Otherwise
            Do
               If rcsrev = Copies( '-', 28 ) & getrev Then gotrev = 1
               If SubWord( rcsrev, 1, 2 ) = 'symbolic names:' & gettag Then gottag = 1
               need_pull = 1
            End
      End
      If num_pulled >= num_queued Then Leave
   End
Call RxQueue 'D', tmpq
Call RxQueue 'S', sessionq /* probably redundant as RxQueue 'D' sets SESSION queue */
Return

SelectDate: Procedure Expose !REXXDW. !global. !!last_repository
topwin = dw_window_new( !REXXDW.!DW_DESKTOP, 'Select Date', !global.!windowstyle )
Call dw_window_set_icon topwin, !global.!qoccaicon
box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start topwin, box, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
datebox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start box, datebox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
today = Date( 'S' )
day = Substr( today, 7, 2 )
mon = Substr( today, 5, 2 )
year = Substr( today, 1, 4 )
/*
 * If we have dw_calendar_new(), use it...
 */
If dw_queryfunction( 'dw_calendar_new' ) = 0 Then
   Do
      date = dw_calendar_new( 100 )
      Call dw_box_pack_start datebox, date, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_SIZE_AUTO, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
      Call dw_calendar_set_date date, today
      window_x = 189
      window_y = 242
   End
Else
   Do
      /*
       * Add our spin buttons
       */
      day_sb = dw_spinbutton_new( day, 0 )
      Call dw_box_pack_start datebox, day_sb, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
      Call dw_spinbutton_set_limits day_sb, 31, 1
      Call dw_spinbutton_set_pos day_sb, day

      month_cb = dw_combobox_new( Word( !global.!months, mon ), 1 )
      Call dw_box_pack_start datebox, month_cb, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
      Do i = 1 To 12
         Call dw_listbox_append month_cb, Word( !global.!months, i )
      End
      Call dw_listbox_select month_cb, mon-1, !REXXDW.!DW_LB_SELECTED

      year_sb = dw_spinbutton_new( year, 2 )
      Call dw_box_pack_start datebox, year_sb, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
      Call dw_spinbutton_set_limits year_sb, 2099, 1990
      Call dw_spinbutton_set_pos year_sb, year
      window_x = 300
      window_y = 130
   End
/*
 * Create a dialog for the window
 */
dialog_wait = dw_dialog_new( )
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start box, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
If dw_queryfunction( 'dw_calendar_new' ) = 0 Then
   Do
      Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'CalendarSelectorApplyCallback', topwin, dialog_wait, date
   End
Else
   Do
      Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'DateSelectorApplyCallback', topwin, dialog_wait, day_sb, month_cb, year_sb
   End

b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start box, b2, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', topwin, dialog_wait
/*
 * We now have everything setup, we can connect the destroying of the top-level window
 * to the 'close' callback
 */
Call dw_signal_connect topwin, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', topwin, dialog_wait
/*
 * Display the splash window
 */
Call dw_window_set_pos_size topwin, (!global.!screen_width % 2) - (window_x % 2), (!global.!screen_height % 2) - (window_y % 2), window_x, window_y
Call dw_window_show topwin

Parse Value dw_dialog_wait( dialog_wait ) With cancel newdate
Return newdate

RunOSCommand: Procedure Expose !REXXDW. !global. !!last_repository osout. oserr.
Parse Arg ro_cmd
If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands ro_cmd
Address System ro_cmd With Output Stem osout. Error Stem oserr.
Return rc

DebugCommands: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg cmd
Say 'Executing:' cmd 'from line:' .sigl
If !global.!batch_mode = 1 Then Say 'Executing:' cmd 'from line:' .sigl
Else
   Do
      Call WriteMessageToLogWindow 'Executing:' cmd
       Call dw_main_sleep 15
   End
Return

/*
 * Displays text in the status area
 */
DisplayStatus: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg msg
Call dw_window_set_text !global.!systemstatus, msg
Call dw_main_sleep 10
Return

/*
 * Write the contents of the stems to the log window
 */
WriteOutputStemsToLogWindow: Procedure Expose !REXXDW. !global. !!last_repository osout. oserr.
Parse Arg conv, msg
Call dw_mle_freeze !global.!mle.!!last_repository
Call WriteMessageHeaderToLogWindow msg
Do wtfi = 1 To osout.0
   mod_colour = DetermineLineColour( conv, osout.wtfi )
   Call WriteMessageToLogWindow '  'osout.wtfi, mod_colour
End
Do wtfi = 1 To oserr.0
   Call WriteMessageToLogWindow '  'oserr.wtfi, 'redfore'
End
Call WriteMessageTrailerToLogWindow msg
Call dw_mle_thaw !global.!mle.!!last_repository
Drop osout. oserr.
Return

/*
 * Write the output from some CVS commands that list actions on files
 */
WriteCVSOutputStemsToLogWindow: Procedure Expose !REXXDW. !global. !!last_repository osout. oserr.
Parse Arg command, ignore, msg
Call dw_mle_freeze !global.!mle.!!last_repository
Call WriteMessageHeaderToLogWindow msg
ignorelen = Length( ignore )
Do i = 1 To osout.0
   If ignorelen = 0 | Left( osout.i, ignorelen ) \= ignore Then Call WriteMessageToLogWindow '  'CVSStatusLine( command osout.i )
End
Do i = 1 To oserr.0
   If ignorelen = 0 | Left( oserr.i, ignorelen ) \= ignore Then Call WriteMessageToLogWindow '  'oserr.i
End
Call WriteMessageTrailerToLogWindow msg
Call dw_mle_thaw !global.!mle.!!last_repository
Return

/*
 * Write the contents of the stems to the screen
 */
WriteOutputStemsToConsole: Procedure Expose !REXXDW. !global. !!last_repository osout. oserr.
Parse Arg conv, msg
Say msg '- Started at:' Date() Time()
Do wtfi = 1 To osout.0
   Say '  'osout.wtfi
End
Do wtfi = 1 To oserr.0
   Say '  'oserr.wtfi
End
Say msg '- Finished at:' Date() Time()
Drop osout. oserr.
Return

WriteStackToLogWindow: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg conv, msg
Call dw_mle_freeze !global.!mle.!!last_repository
Call WriteMessageHeaderToLogWindow msg
Call WriteStackToLogWindowNoHeaderTrailer conv
Call WriteMessageTrailerToLogWindow msg
Call dw_mle_thaw !global.!mle.!!last_repository
Return

WriteStackToLogWindowNoHeaderTrailer: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg conv
Do Queued()
   Parse Pull history_line
   mod_colour = DetermineLineColour( conv, history_line )
   Call WriteMessageToLogWindow '  'history_line, mod_colour
End
Return

WriteMessageHeaderToLogWindow: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg msg, modifier
If modifier = '' Then modifier = 'blueforebold'
Call dw_mle_set_editable !global.!mle.!!last_repository, !REXXDW.!DW_EDITABLE
!global.!mle_position.!!last_repository = dw_mle_import( !global.!mle.!!last_repository, !global.!crlf||msg '- Started at:' Date() Time()||!global.!crlf, !global.!mle_position.!!last_repository )
Parse Value dw_mle_get_size( !global.!mle.!!last_repository ) With bytes lines
Call dw_mle_set_visible !global.!mle.!!last_repository, lines
Call dw_mle_set_editable !global.!mle.!!last_repository, !REXXDW.!DW_READONLY
Return

WriteMessageTrailerToLogWindow: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg msg, modifier
If modifier = '' Then modifier = 'blackforebold'
Call dw_mle_set_editable !global.!mle.!!last_repository, !REXXDW.!DW_EDITABLE
!global.!mle_position.!!last_repository = dw_mle_import( !global.!mle.!!last_repository, msg '- Finished at:' Date() Time()||!global.!crlf, !global.!mle_position.!!last_repository )
Parse Value dw_mle_get_size( !global.!mle.!!last_repository ) With bytes lines
Call dw_mle_set_visible !global.!mle.!!last_repository, lines
Call dw_mle_set_editable !global.!mle.!!last_repository, !REXXDW.!DW_READONLY
Return

WriteMessageToLogWindow: Procedure Expose !REXXDW. !global. !!last_repository osout. oserr.
Parse Arg msg, modifier
If modifier = '' Then modifier = 'blackfore'
Call dw_mle_set_editable !global.!mle.!!last_repository, !REXXDW.!DW_EDITABLE
!global.!mle_position.!!last_repository = dw_mle_import( !global.!mle.!!last_repository, msg||!global.!crlf, !global.!mle_position.!!last_repository )
Parse Value dw_mle_get_size( !global.!mle.!!last_repository ) With bytes lines
Call dw_mle_set_visible !global.!mle.!!last_repository, lines
Call dw_mle_set_editable !global.!mle.!!last_repository, !REXXDW.!DW_READONLY
Return

WriteStringToLogWindow: Procedure Expose !REXXDW. !global. !!last_repository osout. oserr.
Parse Arg msg, modifier
If modifier = '' Then modifier = 'blackfore'
Call dw_mle_set_editable !global.!mle.!!last_repository, !REXXDW.!DW_EDITABLE
!global.!mle_position.!!last_repository = dw_mle_import( !global.!mle.!!last_repository, msg, !global.!mle_position.!!last_repository )
Parse Value dw_mle_get_size( !global.!mle.!!last_repository ) With bytes lines
Call dw_mle_set_visible !global.!mle.!!last_repository, lines
Call dw_mle_set_editable !global.!mle.!!last_repository, !REXXDW.!DW_READONLY
Return

/*
 * Ensure that these still return colours, or you will have to change all calls that
 * expect a colour returned
 */
DetermineLineColour: Procedure Expose !REXXDW. !global. !!last_repository osout. oserr.
Parse Arg conv, str
Select
   When conv = 'cvs_status' Then mod_colour = DetermineCVSStatusLineColour( str )
   Otherwise mod_colour = 'blackfore'
End
Return mod_colour

DetermineCVSStatusLineColour: Procedure Expose !REXXDW. !global. !!last_repository osout. oserr.
Parse Arg action .
Select
   When action = 'U' Then mod_colour = 'greenfore' /* New working file */
   When action = 'P' Then mod_colour = 'greenfore' /* Working file patched */
   When action = 'M' Then mod_colour = 'redfore'   /* Local file modified */
   When action = 'C' Then mod_colour = 'cyan'      /* Conflict in file */
   Otherwise mod_colour = 'blackfore'
End
Return mod_colour

CVSStatusLine: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg command action rem
actions = 'UPMC'
If command = 'update' Then
   Do
      desc.U = 'New working file:'
      desc.P = 'Working file patched:'
   End
Else
   Do
      desc.P = 'New working file:'
      desc.U = 'Working file patched:'
   End
desc.M = 'Local file modified:'
desc.C = 'Conflict in file:'
If Pos( action, actions ) \= 0 Then
   Return Left( desc.action, 21 ) rem
Else
   Return action rem

CheckForCVSConflicts: Procedure Expose !REXXDW. !global. !!last_repository oserr.
/*
 * We now have errors in oserr. stem; see if there are any conflicts
 * and if so, display a messagebox
 * This only works if RunOSCommand() is called, and obviously only for CVS
 */
had_conflicts = 0
Do i = 1 To oserr.0
   If Countstr( 'cvs update: conflicts found', oserr.i ) \= 0 | Countstr( 'warning: conflicts during merge', oserr.i ) \= 0 | Countstr( 'had a conflict and has not been modified', oserr.i ) \= 0 Then
      Do
         had_conflicts = 1
         Leave
      End
End
If had_conflicts Then Call dw_messagebox 'Conflicts found!', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'There were conflicts as a result of this command. See the messages pane for details.'
Return

SetRepFileRow: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg i, initial_flag
this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
fn = Substr( filename.j, 5 )
if trace() = 'F' Then say !global.!repfiles.?filename.!!last_repository.i':'!global.!repfiles.?filestatus.!!last_repository.i
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         line = '{ 'type'}' '{ 'fn'}' '{ 'Substr( filedate.j, 5 )'}' '{ 'Substr(user.j,5)'}'
         /* TODO */
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         icon = Wordpos( Substr( !global.!repfiles.?filetype.!!last_repository.i, 5 ), !global.!fileicons )
         statustype = Wordpos( Substr( !global.!repfiles.?filestatus.!!last_repository.i, 5 ), !global.!statusicons )
         /*
          * Convert the date stored in CVS to localtime
          */
         date = Strip( Substr( !global.!repfiles.?filedate.!!last_repository.i, 5 ) )
         Select
            When date = 0 | date = '*Unknown*' Then date = '* unknown *'
            When Datatype( date ) = 'NUM' Then
               Do
                  date = Time_tDateToDisplayDate( date )
                  If Length( Word( date, 1 ) ) = 1 Then date = ' 'date
               End
            Otherwise Nop
         End
         iconname = Substr( !global.!repfiles.?filereminder.!!last_repository.i, 5 )
         reminder = Wordpos( iconname, !global.!statusicons )
         If initial_flag = 'initial' Then
            Do
               Call dw_filesystem_set_file !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, i-1, Substr( !global.!repfiles.?filename.!!last_repository.i, 5 ), !global.!fileicon.icon
               Call dw_filesystem_set_item !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, 0, i-1, !global.!statusicon.statustype
               Call dw_filesystem_set_item !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, 1, i-1, date
               Call dw_filesystem_set_item !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, 2, i-1, Substr( !global.!repfiles.?user.!!last_repository.i, 5 )
               Call dw_filesystem_set_item !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, 3, i-1, Substr( !global.!repfiles.?filerev.!!last_repository.i, 5 )
               Call dw_filesystem_set_item !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, 4, i-1, !global.!statusicon.reminder
            End
         Else
            Do
               Call dw_filesystem_change_file !global.!repfilescontainer.!!last_repository, i-1, Substr( !global.!repfiles.?filename.!!last_repository.i, 5 ), !global.!fileicon.icon
               Call dw_filesystem_change_item !global.!repfilescontainer.!!last_repository, 0, i-1, !global.!statusicon.statustype
               Call dw_filesystem_change_item !global.!repfilescontainer.!!last_repository, 1, i-1, date
               Call dw_filesystem_change_item !global.!repfilescontainer.!!last_repository, 2, i-1, Substr( !global.!repfiles.?user.!!last_repository.i, 5 )
               Call dw_filesystem_change_item !global.!repfilescontainer.!!last_repository, 3, i-1, Substr( !global.!repfiles.?filerev.!!last_repository.i, 5 )
               Call dw_filesystem_change_item !global.!repfilescontainer.!!last_repository, 4, i-1, !global.!statusicon.reminder
            End
         Call dw_container_set_row_title !global.!repfilescontainermemory.!!last_repository, i-1, i
      End
   When !global.!!repository.!!type.!!last_repository = 'hg' Then
      Do
         icon = Wordpos( Substr( !global.!repfiles.?filetype.!!last_repository.i, 5 ), !global.!fileicons )
         statustype = Wordpos( Substr( !global.!repfiles.?filestatus.!!last_repository.i, 5 ), !global.!statusicons )
         date = Strip( Substr( !global.!repfiles.?filedate.!!last_repository.i, 5 ) )
         date = Time_tDateToDisplayDate( date )
         If Length( Word( date, 1 ) ) = 1 Then date = ' 'date
         iconname = Substr( !global.!repfiles.?filereminder.!!last_repository.i, 5 )
         reminder = Wordpos( iconname, !global.!statusicons )
         If initial_flag = 'initial' Then
            Do
               Call dw_filesystem_set_file !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, i-1, Substr( !global.!repfiles.?filename.!!last_repository.i, 5 ), !global.!fileicon.icon
               Call dw_filesystem_set_item !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, 0, i-1, !global.!statusicon.statustype
               Call dw_filesystem_set_item !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, 1, i-1, date
               Call dw_filesystem_set_item !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, 2, i-1, Substr( !global.!repfiles.?user.!!last_repository.i, 5 )
               Call dw_filesystem_set_item !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, 3, i-1, Right( Format( Substr( !global.!repfiles.?filerev.!!last_repository.i, 5 ),10,0 ), 10, ' ' )
               Call dw_filesystem_set_item !global.!repfilescontainer.!!last_repository, !global.!repfilescontainermemory.!!last_repository, 4, i-1, !global.!statusicon.reminder
            End
         Else
            Do
               Call dw_filesystem_change_file !global.!repfilescontainer.!!last_repository, i-1, Substr( !global.!repfiles.?filename.!!last_repository.i, 5 ), !global.!fileicon.icon
               Call dw_filesystem_change_item !global.!repfilescontainer.!!last_repository, 0, i-1, !global.!statusicon.statustype
               Call dw_filesystem_change_item !global.!repfilescontainer.!!last_repository, 1, i-1, date
               Call dw_filesystem_change_item !global.!repfilescontainer.!!last_repository, 2, i-1, Substr( !global.!repfiles.?user.!!last_repository.i, 5 )
               Call dw_filesystem_change_item !global.!repfilescontainer.!!last_repository, 3, i-1, Substr( !global.!repfiles.?filerev.!!last_repository.i, 5 )
               Call dw_filesystem_change_item !global.!repfilescontainer.!!last_repository, 4, i-1, !global.!statusicon.reminder
            End
         Call dw_container_set_row_title !global.!repfilescontainermemory.!!last_repository, i-1, i
      End
   Otherwise Nop
End
Return

/*
 * For each item in !global.!current_filenames. check if any of its status details have changed
 */
RefreshSelectedFiles: Procedure Expose !REXXDW. !global. !!last_repository
Do i = 1 To !global.!current_filenames.0
   idx = !global.!current_indexes.i
   Select
      When !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then fn = !global.!repfiles.?fullfilename.!!last_repository.idx
      When !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then fn = !global.!allfiles.?fullfilename.!!last_repository.idx
      Otherwise fn = !global.!repfiles.?fullfilename.!!last_repository.idx
   End
   Select
      When !global.!!repository.!!type.!!last_repository = 'rcs' Then Call RefreshRCSFileRow idx, Substr( fn, 5 )
      When !global.!!repository.!!type.!!last_repository = 'cvs' Then Call RefreshCVSFileRow idx, Substr( fn, 5 )
      Otherwise Nop
   End
   Select
      When !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then Call SetRepFileRow idx, ''
      When !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then Call SetAllRepFileRow idx, ''
      Otherwise Nop
   End
End
Return

SetAllRepFileRow: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg i, initial_flag
this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
fn = Substr( filename.j, 5 )
if trace() = 'F' Then say !global.!allfiles.?filename.!!last_repository.i':'!global.!allfiles.?filestatus.!!last_repository.i
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         line = '{ 'type'}' '{ 'fn'}' '{ 'Substr( filedate.j, 5 )'}' '{ 'Substr(user.j,5)'}'
         /* TODO */
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         icon = Wordpos( Substr( !global.!allfiles.?filetype.!!last_repository.i, 5 ), !global.!fileicons )
         statustype = Wordpos( Substr( !global.!allfiles.?filestatus.!!last_repository.i, 5 ), !global.!statusicons )
         /*
          * Convert the date stored in CVS to localtime
          */
         date = Strip( Substr( !global.!allfiles.?filedate.!!last_repository.i, 5 ) )
         Select
            When date = 0 | date = '*Unknown*' Then date = '* unknown *'
            When Datatype( date ) = 'NUM' Then
               Do
                  date = Time_tDateToDisplayDate( date )
                  If Length( Word( date, 1 ) ) = 1 Then date = ' 'date
               End
            Otherwise Nop
         End
         iconname = Substr( !global.!allfiles.?filereminder.!!last_repository.i, 5 )
         reminder = Wordpos( iconname, !global.!statusicons )
         If initial_flag = 'initial' Then
            Do
               Call dw_filesystem_set_file !global.!allfilescontainer.!!last_repository, !global.!allfilescontainermemory.!!last_repository, i-1, Substr( !global.!allfiles.?filename.!!last_repository.i, 5 ), !global.!fileicon.icon
               Call dw_filesystem_set_item !global.!allfilescontainer.!!last_repository, !global.!allfilescontainermemory.!!last_repository, 0, i-1, !global.!statusicon.statustype
               Call dw_filesystem_set_item !global.!allfilescontainer.!!last_repository, !global.!allfilescontainermemory.!!last_repository, 1, i-1, date
               Call dw_filesystem_set_item !global.!allfilescontainer.!!last_repository, !global.!allfilescontainermemory.!!last_repository, 2, i-1, Substr( !global.!allfiles.?user.!!last_repository.i, 5 )
               Call dw_filesystem_set_item !global.!allfilescontainer.!!last_repository, !global.!allfilescontainermemory.!!last_repository, 3, i-1, Substr( !global.!allfiles.?filerev.!!last_repository.i, 5 )
               Call dw_filesystem_set_item !global.!allfilescontainer.!!last_repository, !global.!allfilescontainermemory.!!last_repository, 4, i-1, Substr( !global.!allfiles.?filepath.!!last_repository.i, 5 )
               Call dw_filesystem_set_item !global.!allfilescontainer.!!last_repository, !global.!allfilescontainermemory.!!last_repository, 5, i-1, !global.!statusicon.reminder
            End
         Else
            Do
               Call dw_filesystem_change_file !global.!allfilescontainer.!!last_repository, i-1, Substr( !global.!allfiles.?filename.!!last_repository.i, 5 ), !global.!fileicon.icon
               Call dw_filesystem_change_item !global.!allfilescontainer.!!last_repository, 0, i-1, !global.!statusicon.statustype
               Call dw_filesystem_change_item !global.!allfilescontainer.!!last_repository, 1, i-1, date
               Call dw_filesystem_change_item !global.!allfilescontainer.!!last_repository, 2, i-1, Substr( !global.!allfiles.?user.!!last_repository.i, 5 )
               Call dw_filesystem_change_item !global.!allfilescontainer.!!last_repository, 3, i-1, Substr( !global.!allfiles.?filerev.!!last_repository.i, 5 )
               Call dw_filesystem_change_item !global.!allfilescontainer.!!last_repository, 4, i-1, Substr( !global.!allfiles.?filepath.!!last_repository.i, 5 )
               Call dw_filesystem_change_item !global.!allfilescontainer.!!last_repository, 5, i-1, !global.!statusicon.reminder
            End
         Call dw_container_set_row_title !global.!allfilescontainermemory.!!last_repository, i-1, i
      End
   Otherwise Nop
End
Return

SetWindowTitle: Procedure Expose !REXXDW. !global. !!last_repository
Call dw_window_set_text !global.!mainwindow, 'QOCCA:' !global.!!repository.!!name.!!last_repository'('!global.!!repository.!!type.!!last_repository') Working:' !global.!!repository.!!working_dir.!!last_repository
Return

RefreshRCSFileRow:
/* TODO */
Parse Arg i,fullfilename
newi = Right( i, 4 )
filedate.i = newi||Stream(fullfilename,'C','QUERY TIMESTAMP')
user.i = newi||GetRCSLocker(fullfilename)
Return

/*
 * For an individual file, determine all new file details for displaying
 */
RefreshCVSFileRow: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg idx, fullfilename
If !global.!offline.!!last_repository Then Return
/*
 * Work out how to get file details for an individual file...
 */
Select
   When !global.!!repository.!!enhanced_cvs.!!last_repository \= 'Y' Then
      Do
         If !global.!!repository.!!enhanced_cvs.!!last_repository \= 'Y' Then logcmd = 'log'
         Else logcmd = 'rlog'
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q' logcmd '-bN' quote( fullfilename )
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output Stem log. Error Stem err.
         Do i = 1 To log.0
            Select
               When Left( log.i, 21 ) = 'keyword substitution:' Then type = Word( log.i, 3 )
               When Left( log.i, 20 ) = Copies( '-', 20 ) Then
                  Do
                     tmpidx = i + 1
                     Parse Var log.tmpidx 'revision' rev
                     tmpidx = tmpidx + 1
                     Parse Var log.tmpidx 'date: ' date time ';' 'author: ' author ';' 'state: ' state ';' .
                     /* still need to add locks TODO */
                     item.1 = Left( state, 11 ) || Left( rev, 16 ) || Left( author, 36 ) || Strip( date ) Strip( time ) Left( type, 4 ) || Right( Length( fullfilename ), 3 ) fullfilename
                     Leave i
                  End
               Otherwise Nop
            End
         End
      End
   Otherwise
      Do
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q list -l' quote( fullfilename )
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output Stem item. Error Stem junk.
      End
End
newi = Right( idx, 4 )
Parse Var item.1 1 . 12 rev 28 user 64 filedate 75 filetime 84 type 88 flen 91 .
Parse Var item.1 92 filename +(flen) +1 locks
filetime = Strip( filetime )
filedate = Strip( filedate )
type = Strip( type )
Select
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?repfiles.!!last_repository Then Call SetCVSFileDetails 'INDIVIDUAL', idx, newi, fullfilename, filedate, filetime, type, rev, user, locks
   When !global.!filespage.!!last_repository = !global.!filenotebookpage.?allfiles.!!last_repository Then Call SetCVSAllFileDetails 'INDIVIDUAL', idx, newi, fullfilename, filedate, filetime, type, rev, user, locks
   Otherwise Nop
End
if trace() = 'F' Then say 'refreshcvsfilerow:' fullfilename':'!global.!repfiles.?filestatus.!!last_repository.i
Return

/*
 * For an individual CVS file, determine its new status etc for displaying
 */
SetCVSFileDetails: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg action, idx, newi, fullfilename, filedate, filetime, filetype, rev, user, locks
!global.!repfiles.?filerev.!!last_repository.idx = newi||Strip(rev)
If locks = '' Then !global.!repfiles.?user.!!last_repository.idx = newi||Strip(user)
Else !global.!repfiles.?user.!!last_repository.idx = newi||Strip(user) '('locks')'
Select
   When filedate = 'dummy' & filetime = 'timestamp' Then filedate = 0
   When filetype = '?' Then Nop
   Otherwise
      Do
         filetime = Strip( filetime )
         filedate = Right( DateTimeToTime_T( Changestr( '/', filedate, '' ), filetime, 'S', 1 ), 12, 0 )
      End
End
!global.!repfiles.?filedate.!!last_repository.idx = newi || filedate
!global.!repfiles.?filetype.!!last_repository.idx = newi || DetermineFileType( filetype, !global.!user, locks )
If action = 'INDIVIDUAL' Then
   !global.!repfiles.?filestatus.!!last_repository.idx = newi||DetermineCVSStatus( idx, fullfilename )
!global.!repfiles.?filereminder.!!last_repository.idx = newi||DetermineReminderIcon( Substr( !global.!repfiles.?fullfilename.!!last_repository.idx, 5 ) )
Return

/*
 * For an individual file, determine its new status etc for displaying
 */
SetCVSAllFileDetails: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg action, idx, newi, fullfilename, filedate, filetime, filetype, rev, user, locks
!global.!allfiles.?filerev.!!last_repository.idx = newi||Strip(rev)
If locks = '' Then !global.!allfiles.?user.!!last_repository.idx = newi||Strip(user)
Else !global.!allfiles.?user.!!last_repository.idx = newi||Strip(user) '('locks')'
Select
   When filedate = 'dummy' & filetime = 'timestamp' Then filedate = 0
   When filetype = '?' Then Nop
   Otherwise
      Do
         filetime = Strip( filetime )
         filedate = Right( DateTimeToTime_T( Changestr( '/', filedate, '' ), filetime, 'S', 1 ), 12, 0 )
      End
End
!global.!allfiles.?filedate.!!last_repository.idx = newi || filedate
!global.!allfiles.?filetype.!!last_repository.idx = newi || DetermineFileType( filetype, !global.!user, locks )
If action = 'INDIVIDUAL' Then
   !global.!allfiles.?filestatus.!!last_repository.idx = newi||DetermineCVSStatus( idx, fullfilename )
Else
   !global.!allfiles.?filestatus.!!last_repository.idx = newi||'fileunknown'
!global.!allfiles.?filereminder.!!last_repository.idx = newi||DetermineReminderIcon( Substr( !global.!allfiles.?fullfilename.!!last_repository.idx, 5 ) )
Return

/*
 * For an individual HG file, determine its new status etc for displaying
 */
SetHGFileDetails: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg action, idx, newi, fullfilename, filedate, filetime, filetype, status, rev, user, locks
!global.!repfiles.?filerev.!!last_repository.idx = newi||Right( Strip( rev ), 20, '0' )

icon = GetHGStatusIcon( status )
say .line icon
!global.!repfiles.?filestatus.!!last_repository.idx = Left( !global.!repfiles.?filestatus.!!last_repository.idx, 4 ) || icon
If locks = '' Then !global.!repfiles.?user.!!last_repository.idx = newi||Strip(user)
Else !global.!repfiles.?user.!!last_repository.idx = newi||Strip(user) '('locks')'
/*
Select
   When filedate = 'dummy' & filetime = 'timestamp' Then filedate = 0
   When filetype = '?' Then Nop
   Otherwise
      Do
         filetime = Strip( filetime )
         filedate = Right( DateTimeToTime_T( Changestr( '/', filedate, '' ), filetime, 'S', 1 ), 12, 0 )
      End
End
*/
!global.!repfiles.?filedate.!!last_repository.idx = newi || filedate
say .line filetype
!global.!repfiles.?filetype.!!last_repository.idx = newi || DetermineFileType( filetype, !global.!user, locks )
!global.!repfiles.?filereminder.!!last_repository.idx = newi||DetermineReminderIcon( Substr( !global.!repfiles.?fullfilename.!!last_repository.idx, 5 ) )
Return

/*
 * For an individual file, determine its new status etc for displaying
 */
SetHGAllFileDetails: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg action, idx, newi, fullfilename, filedate, filetime, filetype, status, rev, user, locks
!global.!allfiles.?filerev.!!last_repository.idx = newi||Strip(rev)
If locks = '' Then !global.!allfiles.?user.!!last_repository.idx = newi||Strip(user)
Else !global.!allfiles.?user.!!last_repository.idx = newi||Strip(user) '('locks')'
icon = GetHGStatusIcon( status )
say .line icon
!global.!allfiles.?filestatus.!!last_repository.smi = Left( !global.!allfiles.?filestatus.!!last_repository.smi, 4 ) || icon
!global.!allfiles.?filedate.!!last_repository.idx = newi || filedate
!global.!allfiles.?filetype.!!last_repository.idx = newi || DetermineFileType( filetype, !global.!user, locks )
If action = 'INDIVIDUAL' Then
   !global.!allfiles.?filestatus.!!last_repository.idx = newi||DetermineHGStatus( idx, fullfilename )
Else
   !global.!allfiles.?filestatus.!!last_repository.idx = newi||'fileunknown'
!global.!allfiles.?filereminder.!!last_repository.idx = newi||DetermineReminderIcon( Substr( !global.!allfiles.?fullfilename.!!last_repository.idx, 5 ) )
Return

DetermineReminderIcon: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg file
If Stream( CVSFileToReminderFile( file ), 'C', 'QUERY EXISTS' ) \= '' Then icon = 'setreminder'
Else icon = 'blank'
Return icon

DetermineCVSStatus: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ., filename
cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q status -l' quote( filename )
If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
Address System cmd With Output Stem sm_status. Error Stem junk.
status = ''
Do i = 1 To sm_status.0
   If Left( sm_status.i, 5 ) = 'File:' Then Parse Var sm_status.i . 'Status: ' status
End
icon = GetCVSStatusIcon( status )
Return icon

GetCVSFileproperties: Procedure Expose !REXXDW. !global. !!last_repository osout. prop. !det.
Parse Arg file
osout.0 = 0
If !global.!!repository.!!enhanced_cvs.!!last_repository \= 'Y' Then logcmd = 'log'
Else logcmd = 'rlog'
Call RunOsCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository logcmd quote( file )
prop.0 = 0
cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q status' quote( file )
If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
Address Command cmd With Output Stem prop. Error Stem junk.
Return

ShowFileHistoryWindow: Procedure Expose !REXXDW. !global. !!last_repository osout. prop. !det.
/*
 *
+-Repository Details------------------------------------+
| File Name: /opt/cvs/ffews/sfs/sdfsdf,v                |
| Revision:  1.3             +-------------------------+|
| File Type: Binary          |                       |^||
| Revisions: 3               |                       |v||
| Description:               +-------------------------+|
| This file is a file....                               |
+-Working File Details----------------------------------+
| Status:    Up-to-date (or Unknown)                    |
| Revision:  1.1                                        |
| Date: 28 July 2002 15:03:34                           |
+-Revision History--------------------------------------+
| Revision: 1.3                                       |^|
| Date: 28 July 2002 15:03:34 Author: mark            | |
| Line of changes                                     | |
| The next description line                           | |
| Revision: 1.2                                       | |
| Date: 28 July 2002 15:03:34 Author: mark            | |
| Changes                                             |v|
+-------------------------------------------------------+
 */
Parse Arg fn
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         Call DetermineCVSHistoryDetails fn
         reminder_file = Stream( CVSFileToReminderFile( !global.!current_filenames.1 ), 'C', 'QUERY EXISTS' )
         /*
          * Parse revision history output into !det.!rh_text. array
          */
         Call GetCVSRevisionHistory
      End
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
      End
   Otherwise Nop
End
If reminder_file = '' Then rem_height = 0
Else rem_height = 50 + !global.!widgetheight
filepropchange = 'N'
title_width = 120
rd_height = !global.!widgetheight * 8
wd_height = rem_height + (!global.!widgetheight * 4)

win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Properties of' fn, !global.!windowstyle )
Call dw_window_set_icon win, !global.!qoccaicon

topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Create the group boxen and pack them
 */
rd_box = dw_groupbox_new( !REXXDW.!DW_VERT, 'Repository Details', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start topbox, rd_box, 0, rd_height, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
wfd_box = dw_groupbox_new( !REXXDW.!DW_VERT, 'Working File Details', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start topbox, wfd_box, 0, wd_height, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
rh_box = dw_groupbox_new( !REXXDW.!DW_HORZ, 'Revision History', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start topbox, rh_box, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
/*
 * Create the repository details
 */
file_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start rd_box, file_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color file_box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
file_lab = dw_text_new( 'FileName:', 0 )
Call dw_box_pack_start file_box, file_lab, title_width , !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color file_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_font file_lab, !global.!boldfont
Call dw_window_set_style file_lab, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
file_lab_data = dw_text_new( !det.!rcs_file, 0 )
Call dw_box_pack_start file_box, file_lab_data, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color file_lab_data, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_style file_lab_data, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
/*
 * The rev, file type, revisions and locks lines need to be aligned with the labels box,
 * so we need another HORZ box
 */
com_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start rd_box, com_box, 0, 4*!global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
left_box = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start com_box, left_box, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

rev_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, rev_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color rev_box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
rev_lab = dw_text_new( 'Revision:', 0 )
Call dw_box_pack_start rev_box, rev_lab, title_width , !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color rev_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_font rev_lab, !global.!boldfont
Call dw_window_set_style rev_lab, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
rev_lab_data = dw_text_new( !det.!head, 0 )
Call dw_box_pack_start rev_box, rev_lab_data, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color rev_lab_data, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_style rev_lab_data, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )

filetype_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, filetype_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color filetype_box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
filetype_lab = dw_text_new( 'File Type:', 0 )
Call dw_box_pack_start filetype_box, filetype_lab, title_width , !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color filetype_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_font filetype_lab, !global.!boldfont
Call dw_window_set_style filetype_lab, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
If !global.!administrator.!!last_repository = 'Y' Then
   Do
      If !det.!type = 'Text' Then type_lab_other = 'Binary'
      Else type_lab_other = 'Text'
      type_lab_data = dw_combobox_new( !det.!type, 0 )
      Call dw_box_pack_start filetype_box, type_lab_data, 70, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_listbox_append type_lab_data, !det.!type
      Call dw_listbox_append type_lab_data, type_lab_other
      Call dw_listbox_select type_lab_data, 0, !REXXDW.!DW_LB_SELECTED
   End
Else
   Do
      type_lab_data = dw_text_new( !det.!type, 0 )
      Call dw_box_pack_start filetype_box, type_lab_data, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color type_lab_data, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      Call dw_window_set_style type_lab_data, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
   End

revs_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, revs_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color revs_box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
revs_lab = dw_text_new( 'Revisions:', 0 )
Call dw_box_pack_start revs_box, revs_lab, title_width , !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color revs_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_font revs_lab, !global.!boldfont
Call dw_window_set_style revs_lab, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
revs_lab_data = dw_text_new( !det.!total_revisions, 0 )
Call dw_box_pack_start revs_box, revs_lab_data, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color revs_lab_data, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_style revs_lab_data, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
locks_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start left_box, locks_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color locks_box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
locks_lab = dw_text_new( 'Locked by:', 0 )
Call dw_box_pack_start locks_box, locks_lab, title_width , !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color locks_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_font locks_lab, !global.!boldfont
Call dw_window_set_style locks_lab, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
locks_lab_data = dw_text_new( !det.!lock, 0 )
Call dw_box_pack_start locks_box, locks_lab_data, 50, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color locks_lab_data, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_style locks_lab_data, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
/*
 * Only an administrator on CVS can unlock
 */
If !global.!administrator.!!last_repository = 'Y' & !det.!lock \= '' & !global.!!repository.!!type.!!last_repository = 'cvs' Then
   Do
      abox = dw_box_new( !REXXDW.!DW_HORZ )
      Call dw_box_pack_start locks_box, abox, 0 , !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color abox, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      tmp = dw_text_new( 'Unlock:', 0 )
      Call dw_box_pack_start abox, tmp, 65 , !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color tmp, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      Call dw_window_set_font tmp, !global.!boldfont
      Call dw_window_set_style tmp, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
      locks_cb = dw_checkbox_new( '', 0 )
      Call dw_box_pack_start abox, locks_cb, !global.!widgetheight, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_checkbox_set locks_cb, !REXXDW.!DW_UNCHECKED
      Call dw_window_set_color locks_cb, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      Call dw_box_pack_start locks_box, 0, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   End
/*
 * A box for labels...
 */
right_box = dw_box_new( !REXXDW.!DW_VERT )
lb = dw_listbox_new( 673, !REXXDW.!DW_LB_SINGLE_SELECTION )
maxlen = 0
Do i = !det.!tag_start For !det.!num_tags
   Parse Value dw_font_text_extents_get( lb, 0, osout.i ) With mywidth myheight
   mywidth = mywidth % 1 -- ensure width is a whole number
   If mywidth > maxlen Then maxlen = mywidth
End
len = maxlen + 20
Call dw_box_pack_start com_box, right_box, len, 4*!global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_box_pack_start right_box, lb, len, 4*!global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Do i = !det.!tag_start For !det.!num_tags
   Call dw_listbox_append lb, osout.i
End
/*
 * Add the description and labels headings
 */
desc_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start rd_box, desc_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color desc_box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
desc_lab = dw_text_new( 'Description:', 0 )
Call dw_box_pack_start desc_box, desc_lab, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color desc_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_font desc_lab, !global.!boldfont
Call dw_window_set_style desc_lab, dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_LEFT, !REXXDW.!DW_DT_VCENTER )
labels_lab = dw_text_new( 'Labels    ', 0 )
Call dw_window_set_font labels_lab, !global.!boldfont
Call dw_box_pack_start desc_box, labels_lab, !REXXDW.!DW_SIZE_AUTO, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_style labels_lab, dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_CENTER, !REXXDW.!DW_DT_VCENTER )
Call dw_window_set_color labels_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
/*
 * Add the description entryfield
 */
If !det.!num_desc_lines > 0 Then
   Do
      idx = !det.!desc_start
      !det.!desc = osout.idx
   End
Else
   Do
      !det.!desc = '*** None ***'
   End
desc_entry = dw_entryfield_new( !det.!desc, 0 )
Call dw_box_pack_start rd_box, desc_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Only an administrator on CVS can change the description
 */
If !global.!administrator.!!last_repository = 'Y' & !global.!!repository.!!type.!!last_repository = 'cvs' Then Nop
Else Call dw_window_disable desc_entry
/*
 * Create the Working File Details
 */
status_box = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start wfd_box, status_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color status_box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
status_lab = dw_text_new( 'Status:', 0 )
Call dw_box_pack_start status_box, status_lab, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color status_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
Call dw_window_set_font status_lab, !global.!boldfont
status_lab_data = dw_text_new( !det.!wf_status, 0 )
Call dw_box_pack_start status_box, status_lab_data, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_window_set_color status_lab_data, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
/*
 * If we have a working file, display details about it...
 */
If prop.0 \= 0 Then
   Do
      rev_box = dw_box_new( !REXXDW.!DW_HORZ )
      Call dw_box_pack_start wfd_box, rev_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color rev_box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      rev_lab = dw_text_new( 'Revision:', 0 )
      Call dw_box_pack_start rev_box, rev_lab, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color rev_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      Call dw_window_set_font rev_lab, !global.!boldfont
      rev_lab_data = dw_text_new( !det.!wf_rev, 0 )
      Call dw_box_pack_start rev_box, rev_lab_data, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color rev_lab_data, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE

      date_box = dw_box_new( !REXXDW.!DW_HORZ )
      Call dw_box_pack_start wfd_box, date_box, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color date_box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      date_lab = dw_text_new( 'Date:', 0)
      Call dw_box_pack_start date_box, date_lab, title_width, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color date_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      Call dw_window_set_font date_lab, !global.!boldfont
      date_lab_data = dw_text_new( !det.!wf_date, 0 )
      Call dw_box_pack_start date_box, date_lab_data, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color date_lab_data, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
   End
/*
 * If we have a reminder for this file, display the reminder text in a text box
 */
If reminder_file \= '' Then
   Do
      rem_box = dw_box_new( !REXXDW.!DW_VERT )
      Call dw_box_pack_start wfd_box, rem_box, 0, rem_height, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color rem_box, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      rem_lab = dw_text_new( 'Reminder Text:', 0)
      Call dw_box_pack_start rem_box, rem_lab, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_window_set_color rem_lab, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      Call dw_window_set_font rem_lab, !global.!boldfont
      t1 = dw_mle_new( 0 )
      Call dw_box_pack_start rem_box, t1, 0, rem_height-!global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
      Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
      Call dw_window_set_color t1, !REXXDW.!DW_CLR_BLACK, !REXXDW.!DW_CLR_WHITE
      Call dw_mle_set_cursor t1, 0
      Call dw_mle_delete t1
      pos = -1
      Call Stream reminder_file, 'C', 'OPEN READ'
      Do While Lines( reminder_file ) > 0
         pos = dw_mle_import( t1, Linein( reminder_file ), pos )
      End
      Call Stream reminder_file, 'C', 'CLOSE'
      Call dw_window_disable t1
   End
/*
 * Create the revision history Details
 */
depth = dw_color_depth_get()
scrollbarwidth = 14
!det.!rh_rows = 50
!det.!rh_cols = 100
!det.!current_row=0
!det.!current_col=0
/* create box for filecontents and horz scrollbar */
textboxA = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start rh_box, textboxA, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0

/* create render box for filecontents pixmap */
!det.!t2 = dw_render_new( 0 )
Call dw_window_set_font !det.!t2, !global.!fixedfont
Parse Value dw_font_text_extents_get( !det.!t2, 0, 'g(' ) With font_width font_height
!det.!font_width = (font_width / 2) % 1 -- ensure font width is a whole number
!det.!font_height = font_height
Call dw_box_pack_start  textboxA, !det.!t2, 10, 10, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/* create horizonal scrollbar */
!det.!hscrollbar = dw_scrollbar_new( !REXXDW.!DW_HORZ, 0 )
Call dw_box_pack_start textboxA, !det.!hscrollbar, 100, scrollbarwidth, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0

/* create vertical scrollbar */
vscrollbox = dw_box_new( !REXXDW.!DW_VERT )
!det.!vscrollbar = dw_scrollbar_new( !REXXDW.!DW_VERT, 0 )
Call dw_box_pack_start vscrollbox, !det.!vscrollbar, scrollbarwidth, 100, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/* Pack an area of empty space 14x14 pixels */
Call dw_box_pack_start vscrollbox, 0, scrollbarwidth, scrollbarwidth, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_box_pack_start rh_box, vscrollbox, scrollbarwidth, 0, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
pixmap_width = !det.!font_width * !det.!rh_cols
pixmap_height = !det.!font_height*!det.!rh_rows
!global.!det.!pixmap = dw_pixmap_new( !det.!t2, pixmap_width, pixmap_height, depth )
Call dw_draw_rect 0, !global.!det.!pixmap, !REXXDW.!DW_FILL, 0, 0, pixmap_width, pixmap_height
Call dw_signal_connect !det.!t2, !REXXDW.!DW_EXPOSE_EVENT, 'RevisionHistoryExposeCallback'
Call dw_signal_connect !det.!t2, !REXXDW.!DW_CONFIGURE_EVENT, 'RevisionHistoryConfigureCallback', !global.!det.!pixmap
Call dw_signal_connect !det.!hscrollbar, !REXXDW.!DW_VALUE_CHANGED_EVENT, 'RevisionHistoryValueChangedCallback'
Call dw_signal_connect !det.!vscrollbar, !REXXDW.!DW_VALUE_CHANGED_EVENT, 'RevisionHistoryValueChangedCallback'
If !global.!administrator.!!last_repository = 'Y' Then
   Do
      Call dw_signal_connect !det.!t2, !REXXDW.!DW_BUTTON_RELEASE_EVENT, 'RevisionHistoryChangeLogMessageCallback', fn
   End
Call dw_signal_connect win, !REXXDW.!DW_KEY_PRESS_EVENT, 'RevisionHistoryKeypressCallback'
/*
 * Create a dialog for the window
 */
dialog_wait = dw_dialog_new()
Call dw_scrollbar_set_range !det.!hscrollbar, !det.!rh_max_cols, !det.!rh_cols
Call dw_scrollbar_set_pos !det.!hscrollbar, 0
Call dw_scrollbar_set_range !det.!vscrollbar, !det.!rh_max_rows, !det.!rh_rows
Call dw_scrollbar_set_pos !det.!vscrollbar, 0
/*
 * Draw the first page...
 */
Call DrawRevisionHistory 1, 0
If !global.!administrator.!!last_repository = 'Y' Then
   Do
      /*
       * Create Apply button
       */
      b1 = dw_button_new( 'Apply', 0 )
      Call dw_box_pack_start topbox, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      /*
       * Now we know all of the widgets, we can pass these to the callback
       */
      Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'FilePropertiesApplyCallback', win, dialog_wait, type_lab_data, desc_entry, fn, !det.!type, !det.!desc, locks_cb
      buttontext = 'Cancel'
   End
Else buttontext = 'OK'
/*
 * Create OK button
 */
b1 = dw_button_new( buttontext, 0 )
Call dw_box_pack_start topbox, b1, 0, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Set our signals now that we have al of the widget details to pass
 */
Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * Display the window
 */
width = (!global.!screen_width % 3 ) * 2
height = (!global.!screen_height % 3 ) * 2
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
/*
 * Wait for the dialog to come back
 */
Parse Value dw_dialog_wait( dialog_wait ) With cancel update_flag
If cancel \= 'apply' Then update_flag = 'N'

Drop osout. prop.
Return update_flag

/*
 * Using the output from 'cvs rlog' and 'cvs status' generate the !det. array
 */
DetermineCVSHistoryDetails: Procedure Expose !REXXDW. !global. !!last_repository osout. prop. !det.
Parse Arg fn
!det.!num_tags = 0
!det.!tag_start = 0
!det.!tag_width = 0
!det.!num_desc_lines = 0
!det.!desc_width = 0
!det.!desc_start = 0
!det.!revision_width = 50
!det.!revisions_start = 0
!det.!lock = ''
in_desc = 0
in_revisions = 0
in_tags = 0
in_locks = 0
tab = D2c( 9 )
Do i = 1 To osout.0
   Select
      When in_revisions Then
         Do
            Select
               When osout.i = Copies( '=', 77 ) Then Leave i
               When osout.i = Copies( '-', 28 ) Then Nop
               When Countstr( 'date:', osout.i ) = 1 & Countstr( 'author:', osout.i ) = 1 Then Nop
               Otherwise
                  Do
                     If Length( osout.i ) > !det.!revision_width Then !det.!revision_width  = Length( osout.i )
                  End
            End
         End
      When osout.i = Copies( '-', 28 ) Then
         Do
            !det.!revisions_start = i+1
            in_revisions = 1
            in_desc = 0
         End
      When in_desc Then
         Do
            !det.!num_desc_lines = !det.!num_desc_lines + 1
            If Length( osout.i) > !det.!desc_width Then !det.!desc_width = Length( osout.i )
         End
      When Left( osout.i, 9 ) = 'RCS file:' Then Parse Var osout.i . ': ' !det.!rcs_file
      When Left( osout.i, 5 ) = 'head:' Then Parse Var osout.i . ': ' !det.!head
      When Left( osout.i, 6 ) = 'locks:' Then in_locks = 1
      When Left( osout.i, 15 ) = 'symbolic names:' Then
         Do
            in_tags = 1
            in_locks = 0
         End
      When Left( osout.i, 1 ) == tab & in_locks Then
         Do
            Parse Var osout.i (tab) !det.!lock
         End
      When Left( osout.i, 1 ) == tab & in_tags Then
         Do
            If !det.!tag_start = 0 Then !det.!tag_start = i
            !det.!num_tags = !det.!num_tags + 1
            osout.i = Substr( osout.i, 2 ) /* remove leading tab */
            If Length( osout.i ) > !det.!tag_width Then !det.!tag_width = Length( osout.i )
         End
      When Left( osout.i, 7 ) = 'keyword' Then
         Do
            Parse Var osout.i . ': ' type
            If type = 'b' Then !det.!type = 'Binary'
            Else !det.!type = 'Text'
         End
      When Left( osout.i, 5 ) = 'total' Then Parse Var osout.i . ': ' !det.!total_revisions ';' .
      When Left( osout.i, 12  ) = 'description:' Then
         Do
            in_desc = 1
            !det.!desc_start = i+1
         End
      Otherwise Nop
   End
End
/*
 * Parse the output from cvs status
 */
!det.!wf_status = 'Unknown'
Do i = 1 To prop.0
   Select
      When Countstr( 'File:', prop.i ) = 1 & Countstr( 'Status:', prop.i ) = 1 Then Parse Var prop.i . ':' . 'Status: ' !det.!wf_status
      When Countstr( 'Working revision:', prop.i ) = 1 Then
         Do
            Parse Var prop.i . ':' !det.!wf_rev !det.!wf_date
            If !det.!wf_rev = 'No' Then
               Do
                  !det.!wf_rev = ''
                  !det.!wf_date = ''
               End
         End
      Otherwise Nop
   End
End
/*
 * Date/time returned by CVS is in UTC, so we need to convert
 * to localtime. Unfortunately, CVS does not return the working file
 * date/time if running off the server, so we find it ourselves
 */
utc_offset_secs = Time('O') / 1000000
Parse Var !det.!wf_date . mon day time year
If Strip( !det.!wf_date ) = '' | Datatype( mon ) \= 'NUM' Then
   Do
      Parse Value Stream( fn, 'C', 'QUERY TIMESTAMP' ) With year '-' mon '-' day time
      If Datatype( year ) = 'NUM' Then
         date_data = Date( 'N', year||mon||day, 'S' ) time
      Else
         date_data = '*** unknown ***'
   End
Else
   Do
      revdate = day mon year
      time = Time( 'S', time, 'N' ) + utc_offset_secs
      date = Date( 'B', revdate, 'N' )
      If time > 24 * 60 * 60 Then
         Do
            time = time - (24 * 60 * 60)
            date = date + 1
         End
      If time < 0 Then
         Do
            time = time + (24 * 60 * 60)
            date = date - 1
         End
      date_data = Date( 'N', date, 'B' ) Time( 'N', time, 'S' )
   End
!det.!wf_date = date_data
Return

/*
 * Converts data in osout. array and creates the !det.!rh* arrays
 */
GetCVSRevisionHistory: Procedure Expose !REXXDW. !global. !!last_repository osout. !det.
background = !REXXDW.!DW_CLR_WHITE
utc_offset_secs = Time('O') / 1000000
idx = 1
max_len = 80
/*
 * Get "seashell" color for alternate backgrounds
 */
seashell = !global.!container_colour
Do i = !det.!revisions_start To osout.0
   Select
      When osout.i = Copies( '=', 77 ) Then Leave
      When ( Words( osout.i ) = 2 | Words( osout.i) = 5 ) & Word( osout.i, 1 ) = 'revision' & Strip( Translate( Word( osout.i, 2 ), ' ', '.0123456789' ) ) = '' Then
         Do
            save_revision = Word( osout.i, 2 )
         End
      When osout.i = Copies( '-', 28 ) Then
         Do
            !det.!rh_text.idx = ''
            !det.!rh_type.idx = 'line'
            !det.!rh_back.idx = background
            If background = !REXXDW.!DW_CLR_WHITE Then background = seashell
            Else background = !REXXDW.!DW_CLR_WHITE
            idx = idx + 1 /* increment our line counter */
         End
      When Countstr( 'date:', osout.i ) = 1 & Countstr( 'author:', osout.i ) = 1 Then
         Do
            Parse Var osout.i 'date: ' revdate ';' . 'author: ' author ';' . 'state:' state ';' .
            /*
             * Date/time returned by CVS is is UTC, so we need to convert
             * to localtime.
             */
            time = Time( 'S', Word( revdate, 2 ), 'N' ) + utc_offset_secs
            date = Date( 'B', Changestr( '/', Word( revdate, 1 ), '' ), 'S' )
            If time > 24 * 60 * 60 Then
               Do
                  time = time - (24 * 60 * 60)
                  date = date + 1
               End
            If time < 0 Then
               Do
                  time = time + (24 * 60 * 60)
                  date = date - 1
               End
            !det.!rh_text.idx = save_revision author Strip( state ) Date( 'N', date, 'B' ) Time( 'N', time, 'S' )
            !det.!rh_type.idx = 'header'
            !det.!rh_back.idx = background
            idx = idx + 1
         End
      Otherwise
         Do
            If Length( osout.i ) > max_len Then max_len = Length( osout.i )
            !det.!rh_text.idx = osout.i
            !det.!rh_back.idx = background
            !det.!rh_type.idx = 'line'
            idx = idx + 1
         End
   End
End
lines = idx - 1
!det.!rh_text.0 = lines
!det.!rh_back.0 = lines
!det.!rh_type.0 = lines
!det.!rh_max_rows = lines
!det.!rh_max_cols = max_len
Return

RevisionHistoryExposeCallback: Procedure Expose !REXXDW. !global. !!last_repository osout. prop. !det.
Parse Arg win, x, y, width, height
width = dw_pixmap_width(!global.!det.!pixmap)
height = dw_pixmap_height(!global.!det.!pixmap)
if trace() = 'F' Then say 'got expose'
Call dw_pixmap_bitblt win, , 0, 0, width, height, , !global.!det.!pixmap, 0, 0
Call dw_flush
parse value dw_window_get_preferred_size( !det.!vscrollbar ) with w h
say .line w h
return 1

RevisionHistoryValueChangedCallback: Procedure Expose !REXXDW. !global. !!last_repository osout. prop. !det.
Parse Arg hwnd, value
If hwnd = !det.!vscrollbar Then !det.!current_row = value
Else !det.!current_col = value
if trace() = 'F' Then say 'valuechange' value
Call DrawRevisionHistory !det.!current_row, !det.!current_col
Return 1

RevisionHistoryKeypressCallback: Procedure Expose !REXXDW. !global. !!last_repository osout. prop. !det.
Parse Arg win, char, virtualkey, state, data
if trace() = 'F' Then Say 'got keypress' '('char')' '('virtualkey')' '('state')' data
retcode = 1
Select
   When virtualkey = !REXXDW.!DW_VK_PRIOR Then
      Do
         sb = !det.!vscrollbar
         current = dw_scrollbar_get_pos( sb )
         value = Max( 0, current - !det.!rh_rows )
         key = 'PRIOR'
      End
   When virtualkey = !REXXDW.!DW_VK_NEXT Then
      Do
         sb = !det.!vscrollbar
         current = dw_scrollbar_get_pos( sb )
         value = Min( !det.!rh_max_rows - !det.!rh_rows, current + !det.!rh_rows )
         key = 'NEXT'
      End
   When virtualkey = !REXXDW.!DW_VK_UP Then
      Do
         sb = !det.!vscrollbar
         current = dw_scrollbar_get_pos( sb )
         value = Max( 0, current - 1 )
         key = 'UP'
      End
   When virtualkey = !REXXDW.!DW_VK_DOWN Then
      Do
         sb = !det.!vscrollbar
         current = dw_scrollbar_get_pos( sb )
         value = Min( !det.!rh_max_rows - !det.!rh_rows, current + 1 )
         key = 'DOWN'
      End
/*
   When virtualkey = !REXXDW.!DW_VK_LEFT Then
      Do
         sb = !det.!hscrollbar
         current = dw_scrollbar_get_pos( sb )
         value = Max( 0, current - 1 )
         key = 'LEFT'
      End
   When virtualkey = !REXXDW.!DW_VK_RIGHT Then
      Do
         sb = !det.!hscrollbar
         current = dw_scrollbar_get_pos( sb )
if trace() = 'F' Then          say !det.!rh_max_cols !det.!rh_cols  current + 1
         value = Min( !det.!rh_max_cols - !det.!rh_cols, current + 1 )
         key = 'RIGHT'
      End
*/
   Otherwise
      Do
         sb = ''
         retcode = 0
      End
End
If sb \= '' Then
   Do
if trace() = 'F' Then       say 'key' key 'sb' sb 'current' current 'value' value
      Call dw_scrollbar_set_pos sb, value
   End
Return retcode

/*
 * Called when left mouse button pressed on Revision History window
 */
RevisionHistoryChangeLogMessageCallback: Procedure Expose !REXXDW. !global. !!last_repository osout. prop. !det.
Parse Arg win, x, y, ., file
-- disconnect our signal
Call dw_signal_disconnect win, !REXXDW.!DW_BUTTON_RELEASE_EVENT
-- determine the line in the revision history
fileline = !det.!rh_text.firstline + y % !det.!font_height
If !det.!rh_type.fileline = 'header' Then
   Do
      Parse Var !det.!rh_text.fileline rev author state date
      save_checkin_text = !global.!checkin_text
      text = ''
      crlf = ''
      Do i = fileline To !det.!rh_text.0
         If Strip( !det.!rh_text.i ) = '' Then Leave
         If i \= fileline Then
            Do
               text = text || crlf || !det.!rh_text.i
               crlf = !global.!crlf
            End
      End
      !global.!checkin_text = text
      Parse Value GetCheckinDetails( 'Y', 'checkin', file, , 'Checkin comments: Rev:' rev 'for' ) With cancel .
      If cancel = 'close' Then
         Do
            !global.!checkin_text = save_checkin_text
         End
      Else
         Do
            Select
               When !global.!!repository.!!type.!!last_repository = 'cvs' Then
                  Do
                     text = EscapeQuotes( !global.!checkin_text )
                     If !global.!os \= 'UNIX' Then
                        Do
                           text = Changestr( !global.!crlf, text, ' ' )
                        End
                     cmd = 'cvs admin -m'rev':"'text'"' quote( file )
                     Call RunOsCommand cmd
                     Call WriteOutputStemsToLogWindow 'default', 'Modify log comments for' quote( file )
                     -- get file properties and display
                     Call GetCVSFileProperties file
                     Call DetermineCVSHistoryDetails file
                     -- Parse revision history output into !det.!rh_text. array
                     Call GetCVSRevisionHistory
                     Call DrawRevisionHistory !det.!current_row, !det.!current_col
                  End
               Otherwise Nop
            End
         End
   End
-- restore the mouse capture for the render window
Call dw_signal_connect win, !REXXDW.!DW_BUTTON_RELEASE_EVENT, 'RevisionHistoryChangeLogMessageCallback', file
Return 0

RevisionHistoryConfigureCallback: Procedure Expose !REXXDW. !global. !!last_repository osout. prop. !det.
Parse Arg win, width, height, data
depth = dw_color_depth_get()

!det.!rh_rows = height % !det.!font_height
!det.!rh_cols = width % !det.!font_width

if trace() = 'F' Then say 'got configure' !det.!rh_rows !det.!rh_cols 'MAX' !det.!rh_max_rows !det.!rh_max_cols
If !det.!rh_rows > !det.!rh_max_rows Then !det.!rh_max_rows = !det.!rh_rows
If !det.!rh_cols > !det.!rh_max_cols Then !det.!rh_max_cols = !det.!rh_cols
/* Destroy the old pixmaps */
Call dw_pixmap_destroy !global.!det.!pixmap

/* Create new pixmaps with the current sizes */
!global.!det.!pixmap = dw_pixmap_new( !det.!t2, width, height, depth)

/* Update scrollbar ranges with new values */
Call dw_scrollbar_set_range !det.!hscrollbar, !det.!rh_max_cols, !det.!rh_cols
Call dw_scrollbar_set_range !det.!vscrollbar, !det.!rh_max_rows, !det.!rh_rows

/* Redraw the window */
Call DrawRevisionHistory !det.!current_row, !det.!current_col
return 1

DrawRevisionHistory: Procedure Expose !REXXDW. !global. !!last_repository osout. prop. !det.
Parse Arg row, col
if trace() = 'F' Then say 'got drawrevisionhistory'
Call dw_color_foreground_set !REXXDW.!DW_CLR_WHITE
Call dw_draw_rect , !global.!det.!pixmap, !REXXDW.!DW_FILL, 0, 0, dw_pixmap_width(!global.!det.!pixmap), dw_pixmap_height(!global.!det.!pixmap)
!det.!rh_text.firstline = row + 1 -- save the line number of the first line in the displayed pixmap
Do i = 1 To !det.!rh_rows+1 While (i+row) < !det.!rh_text.0+1
   fileline = i + row
   /* work out the pixel in y direction where text is to start */
   y = (i-1)*!det.!font_height
   /* change the background of the window (by drawing a filled rectangle) */
   Call dw_color_foreground_set !det.!rh_back.fileline
   Call dw_draw_rect 0, !global.!det.!pixmap, !REXXDW.!DW_FILL, 0, y, dw_pixmap_width(!global.!det.!pixmap), !det.!font_height
   /* set the background colour for drawing the text */
   Call dw_color_background_set !det.!rh_back.fileline
   /* set the foreground colour */
   If !det.!rh_type.fileline = 'header' Then
      Do
         Parse Var !det.!rh_text.fileline rev author state date
         x = 0
         Call dw_color_foreground_set !REXXDW.!DW_CLR_BLUE
         Call dw_draw_text 0, !global.!det.!pixmap, !det.!font_width * x, y, 'Revision: '

         x = x + Length( 'Revision: ' )
         Call dw_color_foreground_set !REXXDW.!DW_CLR_BLACK
         Call dw_draw_text 0, !global.!det.!pixmap, !det.!font_width * x, y, rev

         x = x + Length( rev ) + 1
         Call dw_color_foreground_set !REXXDW.!DW_CLR_BLUE
         Call dw_draw_text 0, !global.!det.!pixmap, !det.!font_width * x, y, 'Date: '

         x = x + Length( 'Date: ' )
         Call dw_color_foreground_set !REXXDW.!DW_CLR_BLACK
         Call dw_draw_text 0, !global.!det.!pixmap, !det.!font_width * x, y, date

         x = x + Length( date ) + 1
         Call dw_color_foreground_set !REXXDW.!DW_CLR_BLUE
         Call dw_draw_text 0, !global.!det.!pixmap, !det.!font_width * x, y, 'Author: '

         x = x + Length( 'Author: ' )
         Call dw_color_foreground_set !REXXDW.!DW_CLR_BLACK
         Call dw_draw_text 0, !global.!det.!pixmap, !det.!font_width * x, y, author

         If state = 'dead' Then
            Do
               x = x + Length( author ) + 1
               Call dw_color_foreground_set !REXXDW.!DW_CLR_RED
               Call dw_draw_text 0, !global.!det.!pixmap, !det.!font_width * x, y, '*** Dead Revision ***'
            End
      End
   Else
      Do
         Call dw_color_foreground_set !REXXDW.!DW_CLR_BLACK
         Call dw_draw_text 0, !global.!det.!pixmap, 0, y, Substr( !det.!rh_text.fileline , col+1 )
      End
End
Call RevisionHistoryExposeCallback !det.!t2
Return


FindSyntax: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg ext
If Left( !global.!syntax.0, 1 ) \= '!' Then
   Do
      Do i = 1 To !global.!extension.0
         If Wordpos( ext, !global.!extension.i ) \= 0 Then Return !global.!syntax.i
      End
   End
Return ''

GetHighlightedLines: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg fn, syntax_flags, line_stem
If Left( !global.!!user.!!style, 1 ) = '!' | !global.!!user.!!style = '' Then style = ''
Else style = '-s' !global.!!user.!!style
If Left( !global.!!user.!!preference.!!tabspaces, 1 ) = '!' | !global.!!user.!!preference.!!tabspaces = 0 Then tabspaces_flag = ''
Else tabspaces_flag = '-t' !global.!!user.!!preference.!!tabspaces
--Say .line 'highlight' !global.!datadir '-l -I' syntax_flags style tabspaces_flag '-i' fn
Address System 'highlight' !global.!datadir '-l -I' syntax_flags style tabspaces_flag '-i' fn With Output Stem high. Error Stem junk.
If rc \= 0 Then Return !REXXDW.!DW_CLR_BLACK !REXXDW.!DW_CLR_WHITE

if trace() = 'F' Then say 'got' high.0 'lines from highlight' 'with' junk.0 'errors'
/*
 * Clean up the formatting, so that each line starts with <span><span class="line">
 * and each line ends with </span>
 */
classes = ''
state = 'head'
Do i = 1 To high.0
   Select
      When Strip( high.i ) = '' Then Nop /* ignore blank lines */
      When Left( high.i, 5 ) = '<pre ' Then
         Do
            state = 'file'
            /* remove the <pre> tag */
            Parse Var high.i . '>' high.i
            i = i - 1
         End
      When state = 'head' Then
         Do
            Select
               When Word( high.i, 1 ) = 'pre.hl' Then
                  Do
                     Parse Var high.i . 'color:#' default_fore ';' . 'background-color:#' default_back ';' .
                  End
               When Left( Word( high.i, 1 ), 1 ) = '.' Then
                  Do
                     Parse Var high.i '.hl.' class '{' . 'color:#' class_color ';' fontstuff '}' .
                     class = Strip( Strip( class ), 'B', '	' )
                     classes = classes class
                     If class_color = '' Then class_color = default_fore
                     redx = Substr( class_color, 1, 2 )
                     greenx = Substr( class_color, 3, 2 )
                     bluex = Substr( class_color, 5, 2 )
                     !global.!syntax.!color.class = dw_rgb( x2d( redx), x2d( greenx ), x2d( bluex ) )
                     Select
                        When Countstr( 'bold', fontstuff ) \= 0 & Countstr( 'italic', fontstuff ) \= 0 Then !global.!syntax.!font.class = !global.!bolditalicfont
                        When Countstr( 'bold', fontstuff ) \= 0 Then !global.!syntax.!font.class = !global.!boldfont
                        When Countstr( 'italic', fontstuff ) \= 0 Then !global.!syntax.!font.class = !global.!italicfont
                        Otherwise !global.!syntax.!font.class = !global.!fixedfont
                     End
if trace() = 'F' Then say x2d(redx) x2d(greenx) x2d(bluex) !global.!syntax.!color.class
                  End
               Otherwise Nop
            End
         End
      When state = 'file' Then
         Do
            If Left( high.i, 6 ) = '</pre>' Then Leave
/*
            If Right( high.i, 7 ) \= '</span>' Then
               Do
                  next = i + 1
                  If Left( high.next, 7 ) = '</span>' Then
                     Do
                        high.i = high.i'</span>'
                        high.next = Substr( high.next, 8 )
                     End
               End
*/
            /*
             * Parse Line number out of line
             */
            Parse Var high.i . 'class="hl line">' lno '</span>' line
            lno = Strip( lno )
            /*
             * Remove any spaning code that doesn't change any characters
             */
            Do j = 1 To Words( classes )
               class = Word( classes, j )
               line = Changestr( '<span class="'class'"></span>', line, '' )
            End
            fn.lno = line
            fn.0 = lno
         End
      Otherwise Nop
   End
End
Do i = 1 To Words( classes )
   class = Word( classes, i )
if trace() = 'F' Then    Say 'color for' class 'is' !global.!syntax.!color.class
if trace() = 'F' Then    Say 'font for' class 'is' !global.!syntax.!font.class
End

/*
 * Calculate the default foreground color...
 */
redx = Substr( default_fore, 1, 2 )
greenx = Substr( default_fore, 3, 2 )
bluex = Substr( default_fore, 5, 2 )
default_fore = dw_rgb( x2d( redx), x2d( greenx ), x2d( bluex ) )
/*
 * Calculate the default background color...
 */
redx = Substr( default_back, 1, 2 )
greenx = Substr( default_back, 3, 2 )
bluex = Substr( default_back, 5, 2 )
default_back = dw_rgb( x2d( redx), x2d( greenx ), x2d( bluex ) )
if trace() = 'F' Then say 'default' default_fore default_back
Call SysStemCopy 'fn.', line_stem
Drop fn.
Return default_fore default_back

ConvertHTMLMnemonics: Procedure
Parse Arg fragment
fragment = Changestr( '&lt;', fragment, '<' )
fragment = Changestr( '&gt;', fragment, '>' )
fragment = Changestr( '&amp;', fragment, '&' )
fragment = Changestr( '&quot;', fragment, '"' )
str = fragment
Do Forever
   Parse Var str . '&#' num ';' str
   If Datatype( num ) = 'NUM' Then
      Do
         char = D2c( num )
         fragment = Changestr( '&#'num';', fragment, char )
      End
   If Strip( str ) = '' Then Leave
End
Return fragment

ConvertToHTMLMnemonics: Procedure
Parse Arg fragment
fragment = Changestr( '&', fragment, '&amp;'   )
fragment = Changestr( '<', fragment, '&lt;'    )
fragment = Changestr( '>', fragment, '&gt;'    )
fragment = Changestr( '"', fragment, '&quot;'  )
Return fragment

BackSlashEscape: Procedure
Parse Arg str
Return Changestr( '\', str, '\\' )

FindWorkingDir: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg workingdir
If Left( !global.!!number_repositories, 1 ) = '!' Then Return 0
workingdir = WorkingDirToInternal( workingdir )
Do i = 1 To !global.!!number_repositories
   If WorkingDirToInternal( !global.!!repository.!!working_dir.i ) = workingdir Then Return i
End
Return 0

WorkingDirToInternal: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg workingdir
wd = Changestr( !global.!ossep, workingdir, '/' )
wd = lower( wd )
If Right( wd, 1 ) \= '/' Then wd = wd'/'
Return wd

FindMakeCommand: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!makecommand.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!makecommand.this_dir.pos

FindDebugMakeCommand: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!debugmakecommand.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!debugmakecommand.this_dir.pos

FindCustomMakeCommand: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!custommakecommand.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!custommakecommand.this_dir.pos

FindBuildCommand: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!buildcommand.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!buildcommand.this_dir.pos

FindDebugBuildCommand: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!debugbuildcommand.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!debugbuildcommand.this_dir.pos

FindCustomBuildCommand: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!custombuildcommand.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!custombuildcommand.this_dir.pos

FindPromoteCommand: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!promotecommand.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!promotecommand.this_dir.pos

FindCompilerType: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!compiler.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!compiler.this_dir.pos

FindMakeDirectory: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!makedirectory.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!makedirectory.this_dir.pos

FindPromoteDirectory: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!promotedirectory.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!promotedirectory.this_dir.pos

FindNativeMakeDirectory: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
makedir = FindMakeDirectory( dir )
If makedir = '.' Then makedir = ''
Else makedir = '/'makedir
native_make_dir = Changestr( '/', !global.!!repository.!!working_dir.!!last_repository || dir || makedir, !global.!ossep )
Return native_make_dir

FindNativeBuildDirectory: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir, lab
native_working_dir = Changestr( '/', !global.!!repository.!!working_dir.!!last_repository, !global.!ossep )
build_dir = native_working_dir || !global.!builddir || !global.!ossep || Changestr( '/', dir, !global.!ossep ) || !global.!ossep || lab
Return build_dir

FindNativePromoteDirectory: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir, lab
native_working_dir = Changestr( '/', !global.!!repository.!!working_dir.!!last_repository, !global.!ossep )
promote_dir = native_working_dir || !global.!builddir || !global.!ossep || Changestr( '/', dir, !global.!ossep ) || !global.!ossep || lab
Return promote_dir

FindNativeLogDirectory: Procedure Expose !REXXDW. !global. !!last_repository
Return Changestr( '/', !global.!!repository.!!working_dir.!!last_repository||!global.!logdir, !global.!ossep )

FindEmailCommand: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg dir
this_dir = DirToVariable( dir )
pos = Wordpos( !global.!os, !global.!platforms )
If pos = 0 | Left( !global.!dir.!emailer.this_dir.pos, 1 ) = '!' Then Return ''
Return !global.!dir.!emailer.this_dir.pos

ValidLabel: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg lab
/*
 * Can't use 'Working' or 'Latest'
 */
If lab = 'Working' | lab = 'Latest' Then Return 0 "You cannot use 'Latest' or 'Working' as labels. Please use another value."
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         /*
          * CVS labels can't have '$,.:;@' in them
          */
         test_lab = Translate( lab, '######', '$,.:;@' )
         If lab \= test_lab Then Return 0 'The label supplied:' lab 'must not contain special characters: "$,.:;@"'
      End
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
      End
   Otherwise Nop
End
Return 1

/*
 * Called when the user first starts QOCCA and they don't have a repository,
 * or when they want to add another repository to their accessable list
 */
GetNewRepository: Procedure Expose !REXXDW. !global. !!last_repository
win = dw_window_new( !REXXDW.!DW_DESKTOP, 'Open Repository', !global.!windowstyle )
/*
 * We have to create the window icon here because the main window may not have
 * been created yet.
 */
qoccaicon = dw_icon_load_from_file( !global.!icondir'qocca' )
Call dw_window_set_icon win, qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * Groupbox for radio buttons
 */
g1 = dw_groupbox_new( !REXXDW.!DW_VERT, 'Select Repository Type', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start topbox, g1, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
/*
 * radio buttons
 */
reptype_rb1 = dw_radiobutton_new( 'CVS (Concurrent Versions System)', 111 )
Call dw_box_pack_start g1, reptype_rb1, 200, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
reptype_rb2 = dw_radiobutton_new( 'RCS (Revision Control System)', 222 )
Call dw_box_pack_start g1, reptype_rb2, 200, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
reptype_rb3 = dw_radiobutton_new( 'HG (Mercurial)', 333 )
Call dw_box_pack_start g1, reptype_rb3, 200, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
reptype_rbx = dw_radiobutton_new( 'Select from file...', 444 )
Call dw_box_pack_start g1, reptype_rbx, 200, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
Call dw_radiobutton_set reptype_rbx, !REXXDW.!DW_CHECKED
/*
 * Add Browse button and entry
 */
filebox = dw_box_new( !REXXDW.!DW_HORZ )
Call dw_box_pack_start topbox, filebox, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 2
file_entry = dw_entryfield_new( "", 0 )
Call dw_box_pack_start filebox, file_entry, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
abutton = dw_bitmapbutton_new_from_file( "Browse...", 0, !global.!icondir||'modulecheckedout' )
Call dw_box_pack_start filebox, abutton, 25, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericFileSelector', file_entry, 'Select Repository file:', '', 0, !REXXDW.!DW_FILE_OPEN, 'repository'
/*
 * Create a dialog for the window
 */
dialog_wait = dw_dialog_new( )
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 200, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GetNewRepositoryApplyCallback', win, dialog_wait, file_entry, reptype_rb1, reptype_rb2, reptype_rb3, reptype_rbx
b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 200, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * We now have everything setup, we can connect the destroying of the top-level window
 * to the 'close' callback
 */
Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
/*
 * Display the window
 */
width = 240
height = 260  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10
/*
 * Wait for the dialog to come back
 */
Parse Value dw_dialog_wait( dialog_wait ) With res type fn
/*
 * If we are to close, go back now
 */
If res = 'close' Then Return res
/*
 * If it is a file selection, pre-process the file and then continue with a normal
 * type selection...
 */
If type = 'file' Then
   Do
      /*
       * Check the file is valid, then read the contents and set variables
       * as appropriate
       */
      If Stream( fn, 'C', 'QUERY EXISTS' ) = '' Then
         Do
            Call dw_messagebox 'Invalid file', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), 'File selected does not exist.'
            Return 'close'
         End
      Call Stream fn, 'C', 'OPEN READ'
      If Charin( fn, 1, 2 ) \= '!!' Then
         Do
            Call dw_messagebox 'Invalid file', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), 'File does not contain valid QOCCA repository settings.'
            Call Stream fn, 'C', 'CLOSE'
            Return 'close'
         End
      Call Stream fn, 'C', 'CLOSE'
      /*
       * Set repno to 1 + last repository
       */
      !global.!!number_repositories = !global.!!number_repositories + 1
      repno = !global.!!number_repositories
      Call Stream fn, 'C', 'OPEN READ'
      Do While Lines( fn ) > 0
         Parse Value Linein(fn) With key '=' value
         If key \= '' Then
            Do
               If Left( value, 8 ) = '"!GLOBAL.' Then value = ''
               a = Value( key )'.'Value( repno )
               Interpret '!GLOBAL'.a '=' value
            End
      End
      Call Stream fn, 'C', 'CLOSE'
      /*
       * Now set the repository type
       */
      type = !global.!!repository.!!type.repno
   End
Else repno = 0
Select
   When type = 'rcs' Then res = GetRCSRepository( 'Y', repno )
   When type = 'cvs' Then res = GetCVSRepository( 'Y', repno )
   When type = 'hg'  Then res = GetHGRepository( 'Y', repno )
   Otherwise Nop
End
If res \= 'close' Then Call WriteUserSettingsFile
Return res

GetNewRepositoryApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, file_entry, reptype_rb1, reptype_rb2, reptype_rb3, reptype_rbx
fn = ''
Select
   When dw_radiobutton_get( reptype_rb1 ) Then type = 'cvs'
   When dw_radiobutton_get( reptype_rb2 ) Then type = 'rcs'
   When dw_radiobutton_get( reptype_rb3 ) Then type = 'hg'
   When dw_radiobutton_get( reptype_rbx ) Then
      Do
         type = 'file'
         fn = Strip( dw_window_get_text( file_entry ) )
      End
   Otherwise type = ''
End
Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply' type fn
Return 0

/*
 * Called when the user is adding a new repository to their available list
 * and when displaying repository properties.
 */
GetCVSRepository: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg newrep, repno
/*
 * For CVS repositories, display:
 *
 * +----------------------------------------------------------+
 * | Text                                                     |
 * |                                                          |
 * +----------------------------------------------------------+
 * |  Edit Repository Settings                                |
 * +----------------------------------------------------------+
 * |  o Local                                                 |
 * |  o CVS Password Server                                   |
 * |  o RSH Authentication                                    |
 * |  o SSH Authentication                                    |
 * |                                                          |
 * |  Name:          ___________________________              |
 * |  Server:        ___________________________              |
 * |  Port:          ___________________________              |
 * |  Path:          ___________________________              |
 * |  User:          ___________________________              |
 * |  Password:      ___________________________              |
 * |  Public CVS:    x                                        |
 * |  Enhanced CVS:  x                                        |
 * |  Working Dir:   ___________________________   Browse...  |
 * +----------------------------------------------------------+
 */
cvstype.1 = 'Local repository'
cvstype.2 = 'CVS Password Server'
cvstype.3 = 'RSH Authentication'
cvstype.4 = 'SSH Authentication'
!global.!getcvsrepository.?title.1 = 'Repository Name:'         ;  !global.!getcvsrepository.?type.1 = ''
!global.!getcvsrepository.?title.2 = 'Server:'       ;  !global.!getcvsrepository.?type.2 = ''
!global.!getcvsrepository.?title.3 = 'Port:'         ;  !global.!getcvsrepository.?type.3 = ''
!global.!getcvsrepository.?title.4 = 'Path:'         ;  !global.!getcvsrepository.?type.4 = ''
!global.!getcvsrepository.?title.5 = 'User:'         ;  !global.!getcvsrepository.?type.5 = ''
!global.!getcvsrepository.?title.6 = 'Password:'     ;  !global.!getcvsrepository.?type.6 = 'password'
!global.!getcvsrepository.?title.7 = 'Public CVS:'   ;  !global.!getcvsrepository.?type.7 = 'check'
!global.!getcvsrepository.?title.8 = 'Enhanced CVS:' ;  !global.!getcvsrepository.?type.8 = 'check'
!global.!getcvsrepository.?title.9 = 'CVS Compression:'        ;  !global.!getcvsrepository.?type.9 = ''
!global.!getcvsrepository.?title.10 = 'Public CVS Modules:'        ;  !global.!getcvsrepository.?type.10 = ''
!global.!getcvsrepository.?title.12 = 'Working Dir:'  ;  !global.!getcvsrepository.?type.12 = 'dirbrowse(Select Working Directory)'
!global.!getcvsrepository.?title.0 = 12
If repno = 0 Then
   Do
      default. = ''
      default.0 = 1
   End
Else
   Do
      default.1 = !global.!!repository.!!name.repno
      default.2 = !global.!!repository.!!server.repno
      default.3 = !global.!!repository.!!cvsport.repno
      default.4 = !global.!!repository.!!shortpath.repno
      default.5 = !global.!!repository.!!username.repno
      default.6 = !global.!!repository.!!password.repno
      default.7 = !global.!!repository.!!public_cvs.repno
      default.8 = !global.!!repository.!!enhanced_cvs.repno
      default.9 = !global.!!repository.!!cvs_compression.repno
      default.10 = !global.!!repository.!!public_cvs_dirs.repno
      default.12 = !global.!!repository.!!working_dir.repno
      Do i = 1 To !global.!getcvsrepository.?title.0
         If Left( default.i, 1 ) = '!' Then default.i = ''
      End
   End

win = dw_window_new( !REXXDW.!DW_DESKTOP, 'CVS Repository Details', !global.!windowstyle )
/*
 * We have to create the window icon here because the main window may not have
 * been created yet.
 */
qoccaicon = dw_icon_load_from_file( !global.!icondir'qocca' )
Call dw_window_set_icon win, qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * MLE for entry instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, 400, 50, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
inst = 'Select the type of CVS authentication you will be using, the server that contains the repository, the full path to the repository on that server, and the username used to connect.'
Call dw_mle_import t1, inst, -1
Call dw_window_disable t1
/*
 * Groupbox for radio buttons
 */
g1 = dw_groupbox_new( !REXXDW.!DW_VERT, 'Select CVS Connection Method', !REXXDW.!DW_FONT_BOLD )
Call dw_box_pack_start topbox, g1, 120, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
/*
 * radio buttons
 */
Do i = 1 To 4
   cvstype_rb.i = dw_radiobutton_new( cvstype.i, 100*i )
   Call dw_box_pack_start g1, cvstype_rb.i, 100, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 2
End
If repno = 0 Then idx = 1
Else idx = Wordpos( !global.!!repository.!!contype.repno, !global.!validcvstypes )
Call dw_radiobutton_set cvstype_rb.idx, !REXXDW.!DW_CHECKED
/*
 * If we are an administrator for this repository, add a button to enable
 * editing of the repository (only if not adding a repository)
 */
If !global.!administrator.!!last_repository = 'Y' & newrep \= 'Y' Then
   Do
      b1 = dw_button_new( 'Edit Repository Settings', 0 )
      Call dw_box_pack_start topbox, b1, 200, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'RepositorySettingsCallback'
   End
/*
 * Now the individual entry fields
 */
Do i = 1 To !global.!getcvsrepository.?title.0
   box = dw_box_new( !REXXDW.!DW_HORZ )
   Call dw_box_pack_start topbox, box, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
   !global.!getcvsrepository.?text.i = dw_text_new( !global.!getcvsrepository.?title.i, i )
   Call dw_box_pack_start box, !global.!getcvsrepository.?text.i, 120, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   Call dw_window_set_style !global.!getcvsrepository.?text.i, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
   Select
      When !global.!getcvsrepository.?type.i = 'password' Then
         Do
            !global.!getcvsrepository.?entry.i = dw_entryfield_password_new( default.i, 0 )
         End
      When !global.!getcvsrepository.?type.i = 'check' Then
         Do
            !global.!getcvsrepository.?entry.i = dw_checkbox_new( '', 0 )
            If default.i = 'Y' Then Call dw_checkbox_set !global.!getcvsrepository.?entry.i, !REXXDW.!DW_CHECKED
            Else Call dw_checkbox_set !global.!getcvsrepository.?entry.i, !REXXDW.!DW_UNCHECKED
         End
      Otherwise
         Do
            !global.!getcvsrepository.?entry.i = dw_entryfield_new( default.i, 0 )
         End
   End
   Call dw_box_pack_start box, !global.!getcvsrepository.?entry.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   /*
    * If browse button required...
    */
   If Left( !global.!getcvsrepository.?type.i, 9 ) = 'dirbrowse' Then
      Do
         Parse Var !global.!getcvsrepository.?type.i . '(' title ')'
         abutton = dw_bitmapbutton_new_from_file( title, 0, !global.!icondir||'modulecheckedout' )
         Call dw_box_pack_start box, abutton, 25, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
         Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericDirectorySelector', !global.!getcvsrepository.?entry.i, title':'
      End
End
/*
 * Set the default field to put the cursor in to start with
 */
If repno = 0 | newrep \= 'Y' Then default_idx = 1
Else default_idx = 5
Call dw_window_default win, !global.!getcvsrepository.?entry.default_idx
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 200, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 200, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
width = 425
height = 600  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10

Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GetCVSRepositoryApplyCallback', win, dialog_wait, newrep, repno, cvstype_rb.1, cvstype_rb.2, cvstype_rb.3, cvstype_rb.4
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
Return cancel

GetCVSRepositoryApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, newrep, repno, cvstype_rb1, cvstype_rb2, cvstype_rb3, cvstype_rb4
Select
   When dw_radiobutton_get( cvstype_rb1 ) Then contype = Word( !global.!validcvstypes, 1 )
   When dw_radiobutton_get( cvstype_rb2 ) Then contype = Word( !global.!validcvstypes, 2 )
   When dw_radiobutton_get( cvstype_rb3 ) Then contype = Word( !global.!validcvstypes, 3 )
   When dw_radiobutton_get( cvstype_rb4 ) Then contype = Word( !global.!validcvstypes, 4 )
   Otherwise type = ''
End
/*
 * Get the contents of the entry fields
 */
name = Strip( dw_window_get_text( !global.!getcvsrepository.?entry.1 ) )
server = Strip( dw_window_get_text( !global.!getcvsrepository.?entry.2 ) )
port = Strip( dw_window_get_text( !global.!getcvsrepository.?entry.3 ) )
path = Strip( dw_window_get_text( !global.!getcvsrepository.?entry.4 ) )
user = Strip( dw_window_get_text( !global.!getcvsrepository.?entry.5 ) )
password = Strip( dw_window_get_text( !global.!getcvsrepository.?entry.6 ) )
If dw_checkbox_get( !global.!getcvsrepository.?entry.7 ) Then public_cvs = 'Y'
Else public_cvs = 'N'
If dw_checkbox_get( !global.!getcvsrepository.?entry.8 ) Then enhanced_cvs = 'Y'
Else enhanced_cvs = 'N'
cvs_compression = Strip( dw_window_get_text( !global.!getcvsrepository.?entry.9 ) )
public_cvs_dirs = Strip( dw_window_get_text( !global.!getcvsrepository.?entry.10 ) )
working_dir = Strip( dw_window_get_text( !global.!getcvsrepository.?entry.12 ) )
/*
 * Validate that all mandatory fields are supplied
 * x means not required, o means required
 *   contype       name     server   path  user  password working_dir
 *    local         o         x        o    x        x        o
 *    pserver       o         o        o    o        o        o
 *    rsh           o         o        o    o        x        o
 *    ssh           o         o        o    o        x        o
 */
t_working_dir = Strip( Strip( Changestr( !global.!ossep, working_dir, '/' ) ), 'T', '/' )
Select
   When Words( name ) \= 1 Then
      Do
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Name field is empty or contains spaces. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When type = '' Then
      Do
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'No CVS connection type selected. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When server = '' & contype \= 'local' Then
      Do
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Server field is empty. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When path = '' Then
      Do
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Path field is empty. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When user = '' & contype \= 'local' Then
      Do
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'User field is empty. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When password = '' & contype = 'pserver' Then
      Do
         If dw_messagebox( 'Invalid entry', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_WARNING ), 'Password field is empty. Continue with blank password?' ) = !REXXDW.!DW_MB_RETURN_NO Then
            Do
               Call dw_dialog_dismiss dialog, 'retry'
               Return 0
            End
      End
   When working_dir = '' Then
      Do
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Working Dir is empty please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When Stream( t_working_dir, C, 'QUERY EXISTS' ) \= '' & Word( Stream( t_working_dir, 'C', 'FSTAT' ), 8 ) \= 'Directory' Then
      Do
         /*
          * Stream exists, but its not a directory
          */
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Working Dir specified is not a directory. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When Stream( t_working_dir, C, 'QUERY EXISTS' ) = '' Then
      Do
         /*
          * Stream doesn't exist
          */
         If dw_messagebox( 'Invalid entry', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_WARNING ), 'Working Dir specified does not exist. Do you want to create it?' ) = !REXXDW.!DW_MB_RETURN_NO Then
            Do
               Call dw_dialog_dismiss dialog, 'retry'
               Return 0
            End
      End
   Otherwise Nop
End
/*
 * Validate that name and working_dir are unique
 */
working_dir = WorkingDirToInternal( working_dir )
If Left( !global.!!number_repositories, 1 ) \= '!' & newrep = 'Y' Then
   Do
      foundrep = FindRepository( name )
      If foundrep \= 0 & foundrep \= repno Then
         Do
            Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Name is already used. Please re-enter.'
            Call dw_dialog_dismiss dialog, 'retry'
            Return 0
         End
      foundwd = FindWorkingDir( working_dir )
      If foundwd \= 0 & foundwd \= repno Then
         Do
            Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Working Dir is already used. Please re-enter.'
            Call dw_dialog_dismiss dialog, 'retry'
            Return 0
         End
   End
/*
 * Validate cvsport if supplied
 */
If port = '' | Left( port, 1 ) = '!' Then port = 2401
/*
 * If CVS pserver, we need to Call 'cvs login' to save the password
 * The method employed below allows 'cvs login' to take the password
 * from an environment variable rather than prompting the user.
 */
If contype = 'pserver' Then
   Do
      fullpath = ':pserver:'user'@'server':'path
      Call Value 'CVS_PASS', password, 'ENVIRONMENT'
      Call Value 'CVS_PORT', port, 'ENVIRONMENT'
      rcode = RunOsCommand( 'cvs' '-d' quote( fullpath ) 'login' )
      Call Value 'CVS_PASS', '', 'ENVIRONMENT'
      Call Value 'CVS_PORT', '', 'ENVIRONMENT'
      If rcode \= 0 Then
         Do
            If !global.!mainwindow = '' Then Call WriteOutputStemsToConsole 'default', 'Results of cvs login to' fullpath
            Else Call WriteOutputStemsToLogWindow 'default', 'Results of cvs login to' fullpath
            /*
             * Got an error logging in. Display a message box and
             * ask for the password again.
             */
            Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Invalid password. Please re-enter.'
            Call dw_dialog_dismiss dialog, 'retry'
            Return 0
         End
   End
/*
 * All validation done now, so add the new repository
 */
/*
 * Create the working directory if it doesn't exist
 */
Call CreateDirectory Translate( working_dir, '/', !global.!ossep )

if trace() = 'F' Then say contype name server user path password working_dir

If repno = 0 Then
   Do
      If !global.!!number_repositories = 0 | Left( !global.!!number_repositories, 1 ) = '!' Then !global.!!number_repositories = 1
      Else !global.!!number_repositories = !global.!!number_repositories + 1
      idx = !global.!!number_repositories
   End
Else idx = repno

!global.!!repository.!!type.idx = 'cvs'
Select
   When contype = 'local'   Then !global.!!repository.!!path.idx = ':local:'path
   When contype = 'pserver' Then !global.!!repository.!!path.idx = fullpath
   Otherwise !global.!!repository.!!path.idx = ':ext:'user'@'server':'path
End
!global.!!repository.!!contype.idx = contype
!global.!!repository.!!username.idx = user
!global.!!repository.!!password.idx = password
!global.!!repository.!!shortpath.idx = path
!global.!!repository.!!server.idx = server
!global.!!repository.!!cvsport.idx = port
!global.!!repository.!!public_cvs.idx = public_cvs
!global.!!repository.!!enhanced_cvs.idx = enhanced_cvs
!global.!!repository.!!name.idx = name
!global.!!repository.!!working_dir.idx = working_dir
!global.!!repository.!!last_tree_opened.idx = ''
!global.!!repository.!!public_cvs_dirs.idx = public_cvs_dirs
!global.!!repository.!!cvs_compression.idx = cvs_compression

!!last_repository = idx

Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Save the user's settings now if this is an update of details
 */
If newrep \= 'Y' Then Call WriteUserSettingsFile
Return 0

/*
 * Called when the user is adding a new repository to their available list
 * and when displaying repository properties.
 */
GetHGRepository: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg newrep, repno
/*
 * For HG repositories, display:
 *
 * +----------------------------------------------------------+
 * | Text                                                     |
 * |                                                          |
 * +----------------------------------------------------------+
 * |  Edit Repository Settings                                |
 * +----------------------------------------------------------+
 * |  Name:          ___________________________              |
 * |  URL:           ___________________________              |
 * |  User:          ___________________________              |
 * |  Password:      ___________________________              |
 * |  Working Dir:   ___________________________   Browse...  |
 * +----------------------------------------------------------+
 */
!global.!gethgrepository.?title.1 = 'Repository Name:'         ;  !global.!gethgrepository.?type.1 = ''
!global.!gethgrepository.?title.2 = 'URL:'          ;  !global.!gethgrepository.?type.2 = ''
!global.!gethgrepository.?title.3 = 'User:'         ;  !global.!gethgrepository.?type.3 = ''
!global.!gethgrepository.?title.4 = 'Password:'     ;  !global.!gethgrepository.?type.4 = 'password'
!global.!gethgrepository.?title.5 = 'Working Dir:'  ;  !global.!gethgrepository.?type.5 = 'dirbrowse(Select Working Directory)'
!global.!gethgrepository.?title.0 = 5
If repno = 0 Then
   Do
      default. = ''
      default.0 = 1
   End
Else
   Do
      default.1 = !global.!!repository.!!name.repno
      default.2 = !global.!!repository.!!url.repno
      default.3 = !global.!!repository.!!username.repno
      default.4 = !global.!!repository.!!password.repno
      default.5 = !global.!!repository.!!working_dir.repno
      Do i = 1 To !global.!gethgrepository.?title.0
         If Left( default.i, 1 ) = '!' Then default.i = ''
      End
   End

win = dw_window_new( !REXXDW.!DW_DESKTOP, 'HG Repository Details', !global.!windowstyle )
/*
 * We have to create the window icon here because the main window may not have
 * been created yet.
 */
qoccaicon = dw_icon_load_from_file( !global.!icondir'qocca' )
Call dw_window_set_icon win, qoccaicon
topbox = dw_box_new( !REXXDW.!DW_VERT )
Call dw_box_pack_start win, topbox, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
/*
 * MLE for entry instructions
 */
t1 = dw_mle_new( 0 )
Call dw_box_pack_start topbox, t1, 400, 50, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
Call dw_mle_set_word_wrap t1, !REXXDW.!DW_WORD_WRAP
inst = 'Select the URL that contains the repository and the username used to connect.'
Call dw_mle_import t1, inst, -1
Call dw_window_disable t1
/*
 * If we are an administrator for this repository, add a button to enable
 * editing of the repository (only if not adding a repository)
 */
If !global.!administrator.!!last_repository = 'Y' & newrep \= 'Y' Then
   Do
      b1 = dw_button_new( 'Edit Repository Settings', 0 )
      Call dw_box_pack_start topbox, b1, 200, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
      Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'RepositorySettingsCallback'
   End
/*
 * Now the individual entry fields
 */
Do i = 1 To !global.!gethgrepository.?title.0
   box = dw_box_new( !REXXDW.!DW_HORZ )
   Call dw_box_pack_start topbox, box, 0, 0, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_EXPAND_VERT, 0
   !global.!gethgrepository.?text.i = dw_text_new( !global.!gethgrepository.?title.i, i )
   Call dw_box_pack_start box, !global.!gethgrepository.?text.i, 120, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   Call dw_window_set_style !global.!gethgrepository.?text.i, dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER) , dw_or( !REXXDW.!DW_DT_RIGHT, !REXXDW.!DW_DT_VCENTER )
   Select
      When !global.!gethgrepository.?type.i = 'password' Then
         Do
            !global.!gethgrepository.?entry.i = dw_entryfield_password_new( default.i, 0 )
         End
      When !global.!gethgrepository.?type.i = 'check' Then
         Do
            !global.!gethgrepository.?entry.i = dw_checkbox_new( '', 0 )
            If default.i = 'Y' Then Call dw_checkbox_set !global.!gethgrepository.?entry.i, !REXXDW.!DW_CHECKED
            Else Call dw_checkbox_set !global.!gethgrepository.?entry.i, !REXXDW.!DW_UNCHECKED
         End
      Otherwise
         Do
            !global.!gethgrepository.?entry.i = dw_entryfield_new( default.i, 0 )
         End
   End
   Call dw_box_pack_start box, !global.!gethgrepository.?entry.i, 0, !global.!widgetheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
   /*
    * If browse button required...
    */
   If Left( !global.!gethgrepository.?type.i, 9 ) = 'dirbrowse' Then
      Do
         Parse Var !global.!gethgrepository.?type.i . '(' title ')'
         abutton = dw_bitmapbutton_new_from_file( title, 0, !global.!icondir||'modulecheckedout' )
         Call dw_box_pack_start box, abutton, 25, !global.!widgetheight, !REXXDW.!DW_DONT_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
         Call dw_signal_connect abutton, !REXXDW.!DW_CLICKED_EVENT, 'GenericDirectorySelector', !global.!gethgrepository.?entry.i, title':'
      End
End
/*
 * Set the default field to put the cursor in to start with
 */
If repno = 0 | newrep \= 'Y' Then default_idx = 1
Else default_idx = 5
Call dw_window_default win, !global.!gethgrepository.?entry.default_idx
/*
 * Create Cancel and Apply buttons
 */
b1 = dw_button_new( 'Apply', 0 )
Call dw_box_pack_start topbox, b1, 200, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
b2 = dw_button_new( 'Cancel', 0 )
Call dw_box_pack_start topbox, b2, 200, !global.!buttonheight, !REXXDW.!DW_EXPAND_HORZ, !REXXDW.!DW_DONT_EXPAND_VERT, 0
/*
 * Display the window
 */
width = 425
height = 600  /* text_height + cb_height + 80 */
Call dw_window_set_pos_size win, (!global.!screen_width % 2) - (width % 2), (!global.!screen_height % 2) - (height % 2), width, height
Call dw_window_show win
 Call dw_main_sleep 10

Do Forever
   /*
    * Create a dialog for the window
    */
   dialog_wait = dw_dialog_new( )
   /*
    * We now have everything setup, we can connect the destroying of the top-level window
    * to the 'close' callback
   */
   Call dw_signal_connect b1, !REXXDW.!DW_CLICKED_EVENT, 'GetHGRepositoryApplyCallback', win, dialog_wait, newrep, repno
   Call dw_signal_connect b2, !REXXDW.!DW_CLICKED_EVENT, 'GenericCloseCallback', win, dialog_wait
   Call dw_signal_connect win, !REXXDW.!DW_DELETE_EVENT, 'GenericCloseCallback', win, dialog_wait
   /*
    * Wait for the dialog to come back
    */
   cancel = dw_dialog_wait( dialog_wait )
   /*
    * All successful processing done in the 'applycallback', so simply get
    * out of the loop if we don't have any validation issues
    */
   If cancel \= 'retry' Then Leave
End
Return cancel

GetHGRepositoryApplyCallback: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg button, window, dialog, newrep, repno
/*
 * Get the contents of the entry fields
 */
name = Strip( dw_window_get_text( !global.!gethgrepository.?entry.1 ) )
url = Strip( dw_window_get_text( !global.!gethgrepository.?entry.2 ) )
user = Strip( dw_window_get_text( !global.!gethgrepository.?entry.3 ) )
password = Strip( dw_window_get_text( !global.!gethgrepository.?entry.4 ) )
working_dir = Strip( dw_window_get_text( !global.!gethgrepository.?entry.5 ) )
/*
 * Validate that all mandatory fields are supplied
 * x means not required, o means required
 *       name     url       user  password working_dir
 *        o         o        x        x        o
 */
t_working_dir = Strip( Strip( Changestr( !global.!ossep, working_dir, '/' ) ), 'T', '/' )
Select
   When Words( name ) \= 1 Then
      Do
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Name field is empty or contains spaces. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When url = '' Then
      Do
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'URL field is empty. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When working_dir = '' Then
      Do
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Working Dir is empty please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When Stream( t_working_dir, C, 'QUERY EXISTS' ) \= '' & Word( Stream( t_working_dir, 'C', 'FSTAT' ), 8 ) \= 'Directory' Then
      Do
         /*
          * Stream exists, but its not a directory
          */
         Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Working Dir specified is not a directory. Please re-enter.'
         Call dw_dialog_dismiss dialog, 'retry'
         Return 0
      End
   When Stream( t_working_dir, C, 'QUERY EXISTS' ) = '' Then
      Do
         /*
          * Stream doesn't exist
          */
         If dw_messagebox( 'Invalid entry', dw_or( !REXXDW.!DW_MB_YESNO, !REXXDW.!DW_MB_WARNING ), 'Working Dir specified does not exist. Do you want to create it?' ) = !REXXDW.!DW_MB_RETURN_NO Then
            Do
               Call dw_dialog_dismiss dialog, 'retry'
               Return 0
            End
      End
   Otherwise Nop
End
/*
 * Validate that name and working_dir are unique
 */
working_dir = WorkingDirToInternal( working_dir )
If Left( !global.!!number_repositories, 1 ) \= '!' & newrep = 'Y' Then
   Do
      foundrep = FindRepository( name )
      If foundrep \= 0 & foundrep \= repno Then
         Do
            Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Name is already used. Please re-enter.'
            Call dw_dialog_dismiss dialog, 'retry'
            Return 0
         End
      foundwd = FindWorkingDir( working_dir )
      If foundwd \= 0 & foundwd \= repno Then
         Do
            Call dw_messagebox 'Invalid entry', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Working Dir is already used. Please re-enter.'
            Call dw_dialog_dismiss dialog, 'retry'
            Return 0
         End
   End
/*
 * Clone the repository in the working directory.
 * Save the username and password in .hgrc
 */
rcode = RunOsCommand( 'hg' 'clone' quote( url ) working_dir )
If rcode \= 0 Then
   Do
      If !global.!mainwindow = '' Then Call WriteOutputStemsToConsole 'default', 'Results of "hg clone" to' working_dir
      Else Call WriteOutputStemsToLogWindow 'default', 'Results of "hg clone" to' working_dir
      /*
       * Got an error cloning. Display a message box and
       * ask for the password again.
       */
      Call dw_messagebox 'Clone failed', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Check all details and try again.'
      Call dw_dialog_dismiss dialog, 'retry'
      Return 0
   End
/*
 * Update INI file
 */
Parse Var url prot ':' other '//' server '/' .
base = prot':'other'//'server'/'
inifn = Value( 'HOME', , 'ENVIRONMENT' )'/.hgrc'
basename = Lower( Changestr( ' ', name, '' ) )
trace i
Call SysIni inifn, 'auth', basename'.prefix',base
Call SysIni inifn, 'auth', basename'.username',user
Call SysIni inifn, 'auth', basename'.password',password
trace o
/*
 * All validation done now, so add the new repository
 */
if trace() = 'F' Then say name url user path password working_dir

If repno = 0 Then
   Do
      If !global.!!number_repositories = 0 | Left( !global.!!number_repositories, 1 ) = '!' Then !global.!!number_repositories = 1
      Else !global.!!number_repositories = !global.!!number_repositories + 1
      idx = !global.!!number_repositories
   End
Else idx = repno

!global.!!repository.!!type.idx = 'hg'
!global.!!repository.!!username.idx = user
!global.!!repository.!!password.idx = password
!global.!!repository.!!url.idx = url
!global.!!repository.!!name.idx = name
!global.!!repository.!!working_dir.idx = working_dir
!global.!!repository.!!last_tree_opened.idx = ''

!!last_repository = idx

Call dw_window_destroy window
Call dw_dialog_dismiss dialog, 'apply'
/*
 * Save the user's settings now if this is an update of details
 */
If newrep \= 'Y' Then Call WriteUserSettingsFile
Return 0

/*
 * Generates a job base file, and creates a description file
 * Uses the repository's log directory. If it doesn't exist, create it
 */
GenerateJobBaseFile: Procedure Expose !REXXDW. !global. !!last_repository
nativelogdir = FindNativeLogDirectory()
Call CreateDirectory nativelogdir
bfn = Changestr( '/', nativelogdir||!global.!ossep||'qocca_bg_'||Date('S')||Changestr(':', Time(), '' ), !global.!ossep )
Return bfn

CreateDescriptionFile: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg basefile, desc, type, compiler_type
/*
 * Write the contents of the description file
 */
nativedesc = basefile'.desc'
Call Stream nativedesc, 'C', 'OPEN WRITE REPLACE'
Call Lineout nativedesc, desc
Call Lineout nativedesc, type
Call Lineout nativedesc, compiler_type
Call Stream nativedesc, 'C', 'CLOSE'
Return
/*
 * Check if any pending background events have completed
 */
CheckEventsCallback: Procedure Expose !REXXDW. !global. !!last_repository
done_events = ''
/*
 * Disconnect our timer - IncrementRunningJobs reinstigates it if necessary
 */
Call dw_timer_disconnect !global.!event_timer
Do i = 1 To Words( !global.!running_jobs )
   event_file = Word( !global.!running_jobs, i )
   If Stream( event_file, 'C', 'QUERY EXISTS' ) \= '' Then
      Do
         /*
          * As soon as the event has completed, add the word index of this
          * event into the list of running jobs to delete
          */
         done_events = i done_events
      End
End

/*
 * Remove the events that have completed. We do this by going through a
 * variable with indexes in it rather than at the time we are looping through
 * the array, as that would stuff up the looping, and the indexes are put in
 * in reverse order
 */
Do i = 1 to Words( done_events )
   done_event = Word( done_events, i )
   !global.!running_jobs = Delword( !global.!running_jobs, done_event )
End

Call SetJobsButtonTitle
/* return 0 here to ensure that timer is NOT re-armed */
Return 0

IncrementRunningJobs: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg nativefile
!global.!running_jobs = !global.!running_jobs nativefile
Call SetJobsButtonTitle
Return

SetJobsButtonTitle: Procedure Expose !REXXDW. !global. !!last_repository
Call dw_window_set_text !global.!jobstatus, 'Active Jobs:' Words( !global.!running_jobs )
/*
 * If we have any outstanding jobs, wake up and check them in 5 seconds
 */
If Words( !global.!running_jobs ) > 0 Then !global.!event_timer = dw_timer_connect( 5000, 'CheckEventsCallback' )
Return

WriteBuildConf: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg msg
If msg = '' Then msg = 'new version'
/*
 * Check out the build.conf file for update, write the new contents,
 * check the new version in
 */
Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
      /* TODO */
         rcsfile = _root'/admin/RCS/build.conf,v'
         Address System 'co -l -f' rcsfile !global.!working_conf !global.!devnull
         If rc \= 0 Then
            Do
               Call TkMessageBox '-message', 'Unable to check out configuration file:' rcsfile, '-title', 'Error', '-type', 'ok', '-icon', 'error'
               Return
            End
         Call UpdateBuildConf
         Address System 'ci -q -u -m"'msg'"' !global.!working_conf rcsfile !global.!devnull
         If rc \= 0 Then
            Do
               Call TkMessageBox '-message', 'Unable to check in configuration file:' rcsfile, '-title', 'Error', '-type', 'ok', '-icon', 'error'
               Return
            End
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout admin/build.conf'
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address System cmd With Output Stem osout. Error Stem oserr.
         If rc \= 0 Then
            Do
               Call dw_messagebox 'Error!', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Unable to check out configuration file:' !global.!!repository.!!path.!!last_repository'/admin/build.conf'
               Call WriteOutputStemsToLogWindow 'default', 'Checking out admin/build.conf'
               Return
            End
         Call UpdateBuildConf
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'commit -m "'msg'" admin/build.conf'
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address System cmd With Output Stem osout. Error Stem oserr.
         If rc \= 0 Then
            Do
               Call dw_messagebox 'Error!', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), 'Unable to check in configuration file:' !global.!!repository.!!path.!!last_repository'/admin/build.conf'
               Call WriteOutputStemsToLogWindow 'default', 'Checking in admin/build.conf'
               Return
            End
      End
   Otherwise Nop
End
Return

UpdateBuildConf: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Write the repositories...
 */
Call Stream !global.!working_conf, 'C', 'OPEN WRITE REPLACE'
Call Lineout !global.!working_conf,'; indicates if this repository requires reserved checkouts for all files'
Call Lineout !global.!working_conf,'!reservedcheckout='QuoteAndBlankBang( !global.!reservedcheckout )
Call Lineout !global.!working_conf,'; the list of users that can do admin functions on this repository'
Call Lineout !global.!working_conf,'!admin_users='QuoteAndBlankBang(!global.!admin_users)
Call Lineout !global.!working_conf,'; the list of admin users email addresses'
Call Lineout !global.!working_conf,'!admin_email='QuoteAndBlankBang(!global.!admin_email)
Call Lineout !global.!working_conf,'; the list of users that can build on this repository'
Call Lineout !global.!working_conf,'!build_users='QuoteAndBlankBang(!global.!build_users)
Call Lineout !global.!working_conf,'; the list of users that can update files in this repository. ie CheckOut for RCS, CheckIn for CVS'
Call Lineout !global.!working_conf,'!update_users='QuoteAndBlankBang(!global.!update_users)
Call Lineout !global.!working_conf,'; the user mappings for this repository.'
Call Lineout !global.!working_conf,'!user_mappings='QuoteAndBlankBang(!global.!user_mappings)
Call Lineout !global.!working_conf,'; can a user delete from the repository'
Call Lineout !global.!working_conf,'!user_can_delete='QuoteAndBlankBang(!global.!user_can_delete)
Call Lineout !global.!working_conf,'; the list of platforms on which this repository can be built'
Call Lineout !global.!working_conf,'!platforms='QuoteAndBlankBang(!global.!platforms)
Call Lineout !global.!working_conf,'; the list of directories under the root of this repository that can be worked on'
Call Lineout !global.!working_conf,'!targets='QuoteAndBlankBang(!global.!targets)
Call Lineout !global.!working_conf,'; the list of environments that can be promoted to'
Call Lineout !global.!working_conf,'!runtime_envs='QuoteAndBlankBang(!global.!runtime_envs)
Call Lineout !global.!working_conf,'; this specifies file extensions to ignore when adding new files'
Call Lineout !global.!working_conf,'!ignorefiles='QuoteAndBlankBang(!global.!ignorefiles)
Call Lineout !global.!working_conf,'; this specifies file extensions that are binary'
Call Lineout !global.!working_conf,'!binaryfiles='QuoteAndBlankBang(!global.!binaryfiles)
Call Lineout !global.!working_conf,'; this specifies the environment variable that is set to the version being built'
Call Lineout !global.!working_conf,'!versionenvvar='QuoteAndBlankBang(!global.!versionenvvar)
Call Lineout !global.!working_conf,'; this specifies the environment variable that is set to the build number being built'
Call Lineout !global.!working_conf,'!buildenvvar='QuoteAndBlankBang(!global.!buildenvvar)
Call Lineout !global.!working_conf,'; this specifies the directory where builds are done'
Call Lineout !global.!working_conf,'!builddir='QuoteAndBlankBang(!global.!builddir)
Call Lineout !global.!working_conf,'; this specifies the directory where logs are written'
Call Lineout !global.!working_conf,'!logdir='QuoteAndBlankBang(!global.!logdir)

Call Lineout !global.!working_conf,'; the directories that are common to multiple modules'
Call Lineout !global.!working_conf,'!common_dir.0='QuoteAndBlankBang(!global.!common_dir.0)
Do i = 1 To !global.!common_dir.0
   Call Lineout !global.!working_conf,'!common_dir.'i'='QuoteAndBlankBang(!global.!common_dir.i)
End

Call Lineout !global.!working_conf,'; the directories that are linked to common directories'
Call Lineout !global.!working_conf,'!linked_dir.0='QuoteAndBlankBang(!global.!linked_dir.0)
Call Lineout !global.!working_conf,'!linked_idx.0='QuoteAndBlankBang(!global.!linked_idx.0)
Do i = 1 To !global.!linked_dir.0
   Call Lineout !global.!working_conf,'!linked_dir.'i'='QuoteAndBlankBang(!global.!linked_dir.i)
   Call Lineout !global.!working_conf,'!linked_idx.'i'='QuoteAndBlankBang(!global.!linked_idx.i)
End

Do i = 1 To Words( !global.!targets )
   target = Word( !global.!targets, i )
   If Left( !global.!dir.!makecommand.target.0, 1 ) \= '!' Then
      Do j = 0 To !global.!dir.!makecommand.target.0
         If j \= 0 Then Call Lineout !global.!working_conf,'; directory where a make is to be run from - relative path' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!makedirectory.'target'.'j'='QuoteAndBlankBang(!global.!dir.!makedirectory.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; if this value is non-blank, then this directory can be made under' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!makecommand.'target'.'j'='QuoteAndBlankBang(!global.!dir.!makecommand.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; if this value is non-blank, then this directory can be debug made under' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!debugmakecommand.'target'.'j'='QuoteAndBlankBang(!global.!dir.!debugmakecommand.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; if this value is non-blank, then this directory can be custom made under' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!custommakecommand.'target'.'j'='QuoteAndBlankBang(!global.!dir.!custommakecommand.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; if this value is non-blank, then this directory can be built under' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!buildcommand.'target'.'j'='QuoteAndBlankBang(!global.!dir.!buildcommand.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; if this value is non-blank, then this directory can be debug built under' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!debugbuildcommand.'target'.'j'='QuoteAndBlankBang(!global.!dir.!debugbuildcommand.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; if this value is non-blank, then this directory can be custom built under' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!custombuildcommand.'target'.'j'='QuoteAndBlankBang(!global.!dir.!custombuildcommand.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; directory where a promote is to be run from - relative path' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!promotedirectory.'target'.'j'='QuoteAndBlankBang(!global.!dir.!promotedirectory.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; if this value is non-blank, then this directory can be promoted under' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!promotecommand.'target'.'j'='QuoteAndBlankBang(!global.!dir.!promotecommand.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; if this value is non-blank, then this directory can be installed under' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!installcommand.'target'.'j'='QuoteAndBlankBang(!global.!dir.!installcommand.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; directories to be included in promote under' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!installdirs.'target'.'j'='QuoteAndBlankBang(!global.!dir.!installdirs.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; default compiler for' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!compiler.'target'.'j'='QuoteAndBlankBang(!global.!dir.!compiler.target.j)
         If j \= 0 Then Call Lineout !global.!working_conf,'; email template for' Word( !global.!platforms, j )
         Call Lineout !global.!working_conf,'!dir.!emailer.'target'.'j'='QuoteAndBlankBang(!global.!dir.!emailer.target.j)
      End
   Call Lineout !global.!working_conf,'; indicates if all files in this directory are subject to reserved checkout'
   Call Lineout !global.!working_conf,'!dir.!reservedcheckout.'target'='QuoteAndBlankBang(!global.!dir.!reservedcheckout.target)
End

Call Lineout !global.!working_conf,'; syntax theme to file extension mappings'
Call Lineout !global.!working_conf,'!syntax.0='QuoteAndBlankBang(!global.!syntax.0)
Call Lineout !global.!working_conf,'!extension.0='QuoteAndBlankBang(!global.!extension.0)
If Left( !global.!syntax.0, 1 ) \= '!' Then
   Do i = 1 To !global.!syntax.0
      Call Lineout !global.!working_conf,'!syntax.'i'='QuoteAndBlankBang(!global.!syntax.i)
      Call Lineout !global.!working_conf,'!extension.'i'='QuoteAndBlankBang(!global.!extension.i)
   End

Call Stream !global.!working_conf, 'C', 'CLOSE'
Return

/*
 * For public_cvs = Y, we have to create a dummy build.conf so the rest of
 * QOCCA can run
 */
CreateDummyBuildConf: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg reservedcheckout
!global.!reservedcheckout=reservedcheckout
!global.!admin_users=!global.!user
!global.!admin_email=!global.!user
!global.!build_users=!global.!user
!global.!update_users=!global.!user
!global.!user_can_delete="Y"
!global.!platforms=!global.!os
!global.!targets=""
!global.!runtime_envs=""
!global.!ignorefiles=""
!global.!binaryfiles=""
!global.!versionenvvar="VERSION"
!global.!buildenvvar="BUILD"
!global.!builddir="build"
!global.!logdir="log"
!global.!common_dir.0="0"
!global.!linked_dir.0="0"
!global.!linked_idx.0="0"
!global.!syntax.0="0"
!global.!extension.0="0"
Call UpdateBuildConf
Return

/*
 * Determines where temporary files are saved
 */
GetUserTempDirectory: Procedure Expose !REXXDW. !global. !!last_repository
envs = 'TEMP TMP TMPDIR'
Do i = 1 To Words( envs )
   tmpdir = Value( Word( envs, i ), , 'ENVIRONMENT' )
   If tmpdir \= '' Then Return tmpdir
End
/*
 * To get here we don't have a TEMP env variable, so depending on the platform,
 * set it to a know value.
 */
If !global.!os = 'UNIX' Then tmpdir = Value( 'HOME', , 'ENVIRONMENT' )
Else tmpdir = !global.!home'temp'
Return tmpdir

SetLockedStatusIcon: Procedure Expose !REXXDW. !global. !!last_repository
this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
If !global.!dir.!reservedcheckout.this_dir = 'Y' | !global.!reservedcheckout = 'Y' Then
   Call dw_window_enable !global.!lock_bitmap
Else
   Call dw_window_disable !global.!lock_bitmap
Return

/*
 * Batch commands...
 */

LabelModule: Procedure Expose !REXXDW. !global. !!last_repository
/*
 * Called from batch only
 */
Parse Arg batch, lab, overwrite_label
/*
 * Check if module exists first.
 */
If IsModuleInRepository( !global.!current_dirname.!!last_repository, !!last_repository ) = 0 Then Call Aborttext 'Cannot label: nothing known about module:' !global.!current_dirname.!!last_repository
Parse Value GenerateLabelFile( batch, 'Latest', lab, overwrite_label ) With new_label_file labelbasefile
retcode = ApplyLabelFile( batch, new_label_file, 0, lab, labelbasefile )
Return retcode

GenerateLabelFile: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg batch, based_on, lab, overwrite_label
labelbasefile = '.label-'lab
/*
 * Ensure label does not contain special characters...
 */
test_chars = '$,.:;@'
test_lab = Translate( lab, Copies( '#', Length( test_lab ) ), test_chars )
If lab \= test_lab Then
   Do
      msg = 'The label supplied:' lab 'must not contain special characters:' quote( test_chars )
      If batch = '' Then
         Do
            Call dw_messagebox 'Error!', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_ERROR ), msg
            Return 'N'
         End
      Else
         Do
            Call Aborttext msg
         End
   End
new_label_file = 1
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         If batch = '' Then Call SetCursorWait
         new_rev = '1.1'
         labelworkfile = !global.!current_dirname.!!last_repository'/'labelbasefile
         /*
          * If the working directory doesn't exist, we need to check it out so
          * we can add the new label file.
          */
         If DirectoryExists( !global.!current_dirname.!!last_repository'/CVS' ) = 0 Then
            Do
               cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout -l' !global.!current_dirname.!!last_repository
               If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
               Address System cmd
            End
         /*
          * Check if !global.!current_dirname'label.'ver already exists
          */
         rem = ''
         If !global.!!repository.!!public_cvs.!!last_repository = 'Y' Then
            Do
               root = !global.!!repository.!!shortpath.!!last_repository'/'!global.!current_dirname.!!last_repository'/'
               cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q rlog -hbN' quote( labelworkfile )
               If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
               Address System cmd With Output FIFO '' Error Stem err.
               fn = ''
               Do Queued()
                  Parse Pull Line
                  Select
                     When Left( line, 9 ) = 'RCS file:' Then
                        Do
                           Parse Var line (root) fn ',v'
                        End
                     When Left( line, 5 ) = 'head:' Then
                        Do
                           Parse Var line . ': ' maj '.' min
                        End
                     Otherwise Nop
                  End
               End
            End
         Else
            Do
               cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q list' quote( labelworkfile )
               If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
               Address System cmd With Output Stem out. Error Stem err.
               fn = ''
               If out.0 \= 0 Then Parse Var out.1 . maj '.' min . . . . . fn
            End
         If batch = '' Then Call SetCursorNoWait
         Select
            When fn = labelworkfile & overwrite_label Then
               Do
                  new_label_file = 0
                  If Datatype( min ) = 'NUM' Then new_rev = Strip( maj )'.'min+1
                  /*
                   * In case this label file was created by someone else and we don't have
                   * a copy of it in our working directory, we need to get the latest
                   * version.
                   */
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q get' quote( labelworkfile )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address System cmd With Output Stem junk. Error Stem junk.
               End
            When fn = labelworkfile Then
               Do
                  msg = "Label file exists and you didn't say to overwrite it."
                  If batch = '' Then
                     Do
                        Call dw_messagebox 'Warning!', dw_or( !REXXDW.!DW_MB_OK, !REXXDW.!DW_MB_WARNING ), msg
                        Return 'N'
                     End
                  Else
                     Do
                        Call Aborttext msg
                     End
               End
            Otherwise Nop
         End
         If batch = '' Then Call SetCursorWait
         Call Stream labelworkfile,'C','OPEN WRITE REPLACE'
         Call Lineout labelworkfile, '; QOCCA Label File:' labelbasefile 'for' !global.!current_dirname.!!last_repository
         If based_on = 'Latest' Then
            Do
               /*
                * If based on 'Latest', then get latest revision of files from repository
                */
               If !global.!!repository.!!public_cvs.!!last_repository = 'Y' Then
                  Do
                     root = !global.!!repository.!!shortpath.!!last_repository'/'!global.!current_dirname.!!last_repository'/'
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q rlog -hbN' quote( !global.!current_dirname.!!last_repository )
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address System cmd With Output FIFO '' Error Stem err.
                     Do Queued()
                        Parse Pull Line
                        Select
                           When Left( line, 9 ) = 'RCS file:' Then
                              Do
                                 Parse Var line (root) fn ',v'
                              End
                           When Left( line, 5 ) = 'head:' Then
                              Do
                                 Parse Var line . ': ' maj '.' min
                                 /*
                                  * If the label file exists, don't include it here; we do this later
                                  * with the correct version of the file
                                  */
                                 If fn \= labelworkfile Then Call Lineout labelworkfile,rel fn
                              End
                           Otherwise Nop
                        End
                     End
                  End
               Else
                  Do
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q list' quote( !global.!current_dirname.!!last_repository )
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address System cmd With Output FIFO '' Error Stem err.
                     Do Queued()
                        Parse Pull 1 . 12 rel 28 . 88 flen 91 . 92 fn +(flen) +1 .
                        rel = Strip( rel )
                        fn = Strip( fn )
                        /*
                         * If the label file exists, don't include it here; we do this later
                         * with the correct version of the file
                         */
                        If fn \= labelworkfile Then Call Lineout labelworkfile,rel fn
                     End
                 End
            End
         Else
            Do
               /*
                * If based on a previous label, then 'copy' this label file to our working one
                */
               oldlabelbasefile = '.label-'based_on
               oldlabelworkfile = !global.!current_dirname.!!last_repository'/'oldlabelbasefile
               /*
                * make sure we have the latest copy from the repository...
                */
               cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q get -p' quote( oldlabelworkfile )
               If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
               Address System cmd With Output FIFO '' Error Stem junk.
               Do Queued()
                  Parse Pull rel fn
                  If rel \= ';' Then
                     Do
                        /*
                         * If the label file exists, don't include it here; we do this later
                         * with the correct version of the file
                         */
                        If fn \= labelworkfile Then Call Lineout labelworkfile,rel fn
                     End
               End
            End
         /*
          * Fudge the inclusion of the label file itself into this version's label
          * file.
          */
         Call Lineout labelworkfile, new_rev labelworkfile
         Call Stream labelworkfile,'C','CLOSE'
         If batch = '' Then Call SetCursorNoWait
      End
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         /* TODO */
         labelworkfile = !global.!current_dirname || !global.!ossep || labelbasefile
         labelrcsfile = !global.!current_dirname || !global.!ossep || 'RCS' || !global.!ossep || labelbasefile || ',v'
         se = Stream( labelrcsfile, 'C', 'QUERY EXISTS' )
         Select
            When se = '' Then Nop
            When se \= '' & overwrite_label Then
               Do
                  Call SysFileDelete labelworkfile
                  Call SysFileDelete labelrcsfile
               End
            Otherwise
               Do
                  msg = "Label file exists and you didn't say to overwrite it."
                  If batch = '' Then
                     Do
                        Call TkMessageBox '-message', msg, '-title', 'Warning!', '-type', 'ok', '-icon', 'warning'
                        Return 'N'
                     End
                  Else
                     Do
                        Call Aborttext msg
                     End
               End
         End
         Call Stream labelworkfile,'C','OPEN WRITE REPLACE'
         Call Lineout labelworkfile, '; QOCCA Label File:' labelbasefile 'for' !global.!current_dirname
         If Stream( !global.!current_dirname, 'C','QUERY EXISTS' ) = '' Then rcsfile.0 = 0
         Else Call SysFileTree !global.!current_dirname||!global.!ossep'*,v', 'RCSFILE.', 'FLS'
         Do i = 1 To rcsfile.0
            cmd = 'rlog -h' Word(rcsfile.i,5)
            If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
            Address System cmd With Output FIFO '' Error Stem err.
            Do Queued()
               Parse Pull Line
               Select
                  When Left( line, 9 ) = 'RCS file:' Then
                     Do
                        Parse Var line . 'RCS file:' fn ',v'
                     End
                  When Left( line, 5 ) = 'head:' Then
                     Do
                        Parse Var line . 'head:' ver
                        Call Lineout labelworkfile, Left(Strip(ver),10) Strip(fn)
                     End
                  Otherwise Nop
               End
            End
         End
         /*
          * Fudge the inclusion of the label file itself into this version's label
          * file.
          */
         Call Lineout labelworkfile, Left('1.1',10) labelrcsfile
         Call Stream labelworkfile,'C','CLOSE'
      End
   Otherwise Nop
End
Return new_label_file labelbasefile

ApplyLabelFile: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg batch, new_label_file, edit_label_file, lab, labelbasefile
retcode = 0
/*
 * Change to the working directory
 */
here = Directory()
Call Directory( !global.!current_dirname.!!last_repository )
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         /*
          * Add the label file to the repository
          */
         cvs_tag = Changestr( ' ', Changestr( '/', !global.!current_dirname.!!last_repository, '_'), '-' ) || '_' || lab
         If batch = '' Then
            Do
               If new_label_file Then
                  Do
                     Call RunOSCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'add' '-m"QOCCA label file for:' lab'"' quote( labelbasefile )
                     Call WriteOutputStemsToLogWindow 'default', 'Addition of label file' quote( labelbasefile )
                  End
               Call RunOSCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'commit -m"Addition of label file"' quote( labelbasefile )
               Call WriteOutputStemsToLogWindow 'default', 'Committing label file' quote( labelbasefile )
               /*
                * If we edited the label file, we need to rtag each individual file...
                */
               If edit_label_file Then
                  Do
                     Call WriteMessageHeaderToLogWindow 'Tagging with label:' quote( cvs_tag )
                     Call Stream labelbasefile, 'C', 'OPEN READ'
                     Do While Lines( labelbasefile ) > 0
                        Parse Value Linein( labelbasefile ) With rev fn
                        If rev = ';' Then Iterate
                        Call WriteMessageToLogWindow '  'rev fn
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q rtag -F -l -r'rev quote( cvs_tag ) quote( fn )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output FIFO '' Error FIFO ''
                        Do Queued()
                           Parse Pull line
                           Call WriteMessageToLogWindow '  'line
                        End
                     End
                     Call Stream labelbasefile, 'C', 'CLOSE'
                     Call WriteMessageTrailerToLogWindow 'Tagging with label:' quote( cvs_tag )
                  End
               Else
                  Do
                     Call RunOSCommand 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q rtag -F -R' quote( cvs_tag ) quote( !global.!current_dirname.!!last_repository )
                     Call WriteOutputStemsToLogWindow 'default', 'Tagging with label:' quote( cvs_tag )
                  End
            End
         Else
            Do
               If new_label_file Then
                  Do
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'add' '-m"QOCCA label file for:' lab'"' quote( labelbasefile )
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address System cmd
                  End
               retcode = rc
               cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'commit -m"Addition of label file"' quote( labelbasefile )
               If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
               Address System cmd
               retcode = rc
               /*
                * In batch mode only the latest revision of a module can be labeled, so rtag
                * of the directory is possible
                */
               cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q rtag -F -R' quote( cvs_tag ) quote( !global.!current_dirname.!!last_repository )
               If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
               Address System cmd
               retcode = rc
            End
      End
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
         /* TODO */
         textmsg = 'Label file for:' lab
         Address System 'ci -q -u -t-"'textmsg'"' labelworkfile labelrcsfile
      End
   Otherwise Nop
End
/*
 * Change directory back to the root working dir
 */
Call Directory( here )
Return retcode

/*
 * Run in batch mode
 * Only the "build user(s)" can do this - in build.conf
 */
Batch: Procedure Expose !REXXDW. !global. !!last_repository
_arg.0 = Queued()
Do i = 1 To _arg.0
   Parse Pull _arg.i
End
cmd = Translate( _arg.1 )
!global.!validcmds = 'init label mods build promote copy add export'
/*
 * mods - report module differences (batch or user)
 */
!global.!validargs.mods = '-n -N -m -M -v -V -p -w -B -T -W -A'
!global.!vars.mods = 'repository earlier_repository _current_dirname _earlier_dirname newlab oldlab noprogress ignorewhitespace _basefile _trace_flag _wc_flag _sa_flag'
!global.!opt.mods = 'Repository EarlierRepository Module EarlierModule Version EarlierVersion NoProgress IgnoreWhitespace BaseFile TraceFlag WordCompareFlag ShowAllCompareFlag'
!global.!desc.mods.1 = 'Name of Repository for Later Version'
!global.!desc.mods.2 = 'Name of Repository for Earlier Version'
!global.!desc.mods.3 = 'Module for Later Version; eg mytop/myproj'
!global.!desc.mods.4 = 'Module for Earlier Version; eg mytop/myproj'
!global.!desc.mods.5 = 'Later Version string; eg Latest or Working or mytop/myproj,1_3_2'
!global.!desc.mods.6 = 'Earlier Version string; eg Latest or Working or mytop/myproj,1_3_1'
!global.!desc.mods.7 = 'Indicates no progress dots displayed'
!global.!desc.mods.8 = 'Ignore whitespace in file comparisons'
!global.!desc.mods.9 = 'Base file (internal use only)'
!global.!desc.mods.10 = 'Trace flag (no value: minimal tracing, value: VERY verbose tracing)'
!global.!desc.mods.11 = 'Indicates if Word documents are compared'
!global.!desc.mods.12 = 'Indicates if All files are to be reported or just those that differ'
!global.!desc.mods.0 = 12
!global.!mandatory.mods = '0 0 1 0 1 1 0 0 0 0 0 0'
!global.!valuereq.mods  = '1 1 1 1 1 1 0 0 1 0 0 0'
/*
 * label - label a module (batch or user)
 */
!global.!validargs.label = '-n -m -v -o -B -T'
!global.!vars.label = 'repository _current_dirname lab overwrite _basefile _trace_flag'
!global.!opt.label = 'Repository Module Version OverwriteLabel BaseFile TraceFlag'
!global.!desc.label.1 = 'Name of Repository'
!global.!desc.label.2 = 'Module to label; eg mytop/myproj'
!global.!desc.label.3 = 'Version string; eg 1_3_2'
!global.!desc.label.4 = 'Flag to indicate that existing Version file to be overwritten'
!global.!desc.label.5 = 'Base file (internal use only)'
!global.!desc.label.6 = 'Trace flag (no value: minimal tracing, value: VERY verbose tracing)'
!global.!desc.label.0 = 6
!global.!mandatory.label = '0 1 1 0 0 0'
!global.!valuereq.label  = '1 1 1 0 1 0'
/*
 * build - build a module with configured build command (batch or user)
 */
!global.!validargs.build = '-n -m -v -b -o -C -c -t -B -T -w'
!global.!vars.build = 'repository _current_dirname lab buildtype overwritebuild dontcheckout continuecheckout testbuild _basefile _trace_flag _working_dir'
!global.!opt.build = 'Repository Module Version BuildType OverwriteBuild DontCheckout ContinueCheckout TestBuild BaseFile TraceFlag WorkingDir'
!global.!desc.build.1 = 'Name of Repository'
!global.!desc.build.2 = 'Module to build: eg mytop/myproj'
!global.!desc.build.3 = 'Version string (must have been labelled); eg 1_3_2'
!global.!desc.build.4 = 'Type of build to perform: "debug" or "custom"'
!global.!desc.build.5 = 'Flag to indicate that existing build to be overwritten'
!global.!desc.build.6 = 'Flag to indicate to not checkout files (previous build was done for module/version)'
!global.!desc.build.7 = 'Flag to indicate to continue checking out files that have been checked out (previous checkout failed)'
!global.!desc.build.8 = 'Flag to indicate that current build is a test build'
!global.!desc.build.9 = 'Base file (internal use only)'
!global.!desc.build.10 = 'Trace flag (no value: minimal tracing, value: VERY verbose tracing)'
!global.!desc.build.11 = 'Working Directory (override dir in repository file)'
!global.!desc.build.0 = 11
!global.!mandatory.build = '0 1 1 0 0 0 0 0 0 0 0'
!global.!valuereq.build  = '1 1 1 1 0 0 0 0 1 0 1'
/*
 * promote - promote a module with configured promote command (batch or user)
 */
!global.!validargs.promote = '-n -m -v -e -B -T'
!global.!vars.promote = 'repository _current_dirname lab env _basefile _trace_flag'
!global.!opt.promote = 'Repository Module Version Environment BaseFile TraceFlag'
!global.!desc.promote.1 = 'Name of Repository'
!global.!desc.promote.2 = 'Module to promote: eg mytop/myproj'
!global.!desc.promote.3 = 'Version string (must have been built)'
!global.!desc.promote.4 = 'Environment to promote to'
!global.!desc.promote.5 = 'Base file (internal use only)'
!global.!desc.promote.6 = 'Trace flag (no value: minimal tracing, value: VERY verbose tracing)'
!global.!desc.promote.0 = 6
!global.!mandatory.promote = '0 1 1 0 0 0'
!global.!valuereq.promote  = '1 1 1 1 1 0'
/*
 * copy - copy a module as a new repository (batch or user)
 */
!global.!validargs.copy = '-n -m -d -v -o -B -T'
!global.!vars.copy = 'repository _current_dirname new_dirname lab overwrite _basefile _trace_flag'
!global.!opt.copy = 'Repository Module NewDirectory Version Overwrite BaseFile TraceFlag'
!global.!desc.copy.1 = 'Name of current Repository'
!global.!desc.copy.2 = 'Module to copy: eg mytop/myproj'
!global.!desc.copy.3 = 'Directory for new repository'
!global.!desc.copy.4 = 'Version string (must have been labeled)'
!global.!desc.copy.5 = 'Flag to indicate that existing directory to be overwritten'
!global.!desc.copy.6 = 'Base file (internal use only)'
!global.!desc.copy.7 = 'Trace flag (no value: minimal tracing, value: VERY verbose tracing)'
!global.!desc.copy.0 = 7
!global.!mandatory.copy = '0 1 1 1 0 0 0'
!global.!valuereq.copy  = '1 1 1 1 0 1 0'
/*
 * init - initialise a new repository (batch only)
 */
!global.!validargs.init = '-r -t -u -p -m -e -T'
!global.!vars.init = 'repository _sourcecontrol _username _password _module _extraadminuser _trace_flag'
!global.!opt.init = 'Repository SourceControl UserName Password InitialModule ExtraAdminUser TraceFlag'
!global.!desc.init.1 = 'Location of repository to initialise'
!global.!desc.init.2 = 'Type of repository: cvs, rcs'
!global.!desc.init.3 = 'Username to connect to repository (if required)'
!global.!desc.init.4 = 'Password to connect to repository (if required)'
!global.!desc.init.5 = 'Initial Module to create under Repository Root'
!global.!desc.init.6 = 'Extra Admin User'
!global.!desc.init.7 = 'Trace flag (no value: minimal tracing, "I" or "R": VERY verbose tracing)'
!global.!desc.init.0 = 7
!global.!mandatory.init = '1 1 0 0 0 0 0'
!global.!valuereq.init  = '1 1 1 1 1 1 0'
/*
 * add - add a new repository to a user's profile (.qocca file) (batch only)
 */
!global.!validargs.add = '-n -t -c -e -s -r -u -p -w -d -T'
!global.!vars.add = '_name _sourcecontrol _cvstype _enhanced _server repository _username _password _working _tmpdir _trace_flag'
!global.!opt.add = 'Name SourceControl CVSType EnhancedCVS Server Repository UserName Password WorkingDir TmpDir TraceFlag'
!global.!desc.add.1 = 'Name of repository to add'
!global.!desc.add.2 = 'Type of repository: cvs, rcs'
!global.!desc.add.3 = 'Connection type for CVS. One of:' !global.!validcvstypes
!global.!desc.add.4 = 'Specifies the CVS repository supports "enhanced CVS"'
!global.!desc.add.5 = 'Server where repository is located'
!global.!desc.add.6 = 'Location of repository to add'
!global.!desc.add.7 = 'Username'
!global.!desc.add.8 = 'Password'
!global.!desc.add.9 = 'Working Directory'
!global.!desc.add.10 = 'Temporary Directory'
!global.!desc.add.11 = 'Trace flag (no value: minimal tracing, value: VERY verbose tracing)'
!global.!desc.add.0 = 11
!global.!mandatory.add = '1 1 0 0 0 1 0 0 1 1 0'
!global.!valuereq.add  = '1 1 1 0 1 1 1 1 1 1 0'
/*
 * export - export a module to a location (batch)
 */
!global.!validargs.export = '-n -m -v -T -w'
!global.!vars.export = 'repository _current_dirname lab _trace_flag _working_dir'
!global.!opt.export = 'Repository Module Version TraceFlag WorkingDir'
!global.!desc.export.1 = 'Name of Repository'
!global.!desc.export.2 = 'Module to build: eg mytop/myproj'
!global.!desc.export.3 = 'Version string (must have been labelled); eg 1_3_2'
!global.!desc.export.4 = 'Trace flag (no value: minimal tracing, value: VERY verbose tracing)'
!global.!desc.export.5 = 'Working Directory (override dir in repository file)'
!global.!desc.export.0 = 5
!global.!mandatory.export = '0 1 1 0 1'
!global.!valuereq.export  = '1 1 1 0 1'

retcode = 0

/*
 * Display version and usage if only arg is "-h"
 */
If cmd = '-H' Then
   Do
      Say 'This is QOCCA Version:' !global.!version
      Call Usage
   End
If Wordpos( cmd, Translate( !global.!validcmds ) ) = 0 Then
   Do
      Say 'Invalid batch command:' _arg.1'. Must be one of' !global.!validcmds
      Call Usage
   End

Do i = 2 To _arg.0
   Do j = 1 To Words( !global.!validargs.cmd )
      If Substr( _arg.i, 1, 2 ) = Word( !global.!validargs.cmd, j ) Then
         Do
            optval = Strip( Substr( _arg.i, 3 ) )
            optvar = Word( !global.!vars.cmd, j )
            Interpret optvar '= optval'
            Leave j
         End
   End
End
error = 0
Do i = 1 To Words( !global.!vars.cmd )
   Select
      When Translate( Value( Word( !global.!vars.cmd, i ) ) ) = Translate( Word( !global.!vars.cmd, i ) ) & Word( !global.!mandatory.cmd, i ) = 1 Then
         Do
            Say 'Mandatory parameter' Word( !global.!validargs.cmd, i ) 'not supplied'
            error = 1
         End
      When Word( !global.!valuereq.cmd, i ) = 1 & Value( Word( !global.!vars.cmd, i ) ) = '' Then
         Do
            Say 'No value supplied for parameter' Word( !global.!validargs.cmd, i )
            error = 1
         End
      Otherwise Nop
   End
End
If error = 1 Then Call Usage cmd
/*
 * Lets start the verbose tracing from here if required...
 */
Select
   When _trace_flag = '' Then _trace_flag = 'R'
   When _trace_flag = '_TRACE_FLAG' Then _trace_flag = 'O'
   When Translate( _trace_flag ) = 'I' | Translate( _trace_flag ) = 'R' | Translate( _trace_flag ) = 'L' Then
      Do
         Trace Value _trace_flag
      End
   Otherwise
      Do
         Say 'Invalid Trace Flag:' _trace_flag'. Must be one of "L", "I" or "R"'
         Call Usage
      End
End
/*
 * Get the details from the user's preferences file
 */
Call ReadUserSettingsFile
/*
 * INIT and ADD commands do not require a user to have a .qocca profile file; all
 * other batch commands do!
 */
Select
   When cmd = 'INIT' Then
      Do
         If Wordpos( _sourcecontrol, !global.!valid_repository_types ) = 0 Then
            Call AbortText 'Invalid repository type:' quote( _sourcecontrol ) 'specified. Must be one of:' !global.!valid_repository_types
/*
         If Left( !global.!!tmpdir, 1 ) = '!' Then
            Do
               !global.!!tmpdir = !global.!home||SysTempFileName( 'dir?????' )|| !global.!ossep
               Call SysMkdir !global.!!tmpdir
            End
*/
      End
   When cmd = 'ADD' Then
      Do
         If Wordpos( _sourcecontrol, !global.!valid_repository_types ) = 0 Then
            Call AbortText 'Invalid repository type:' quote( _sourcecontrol ) 'specified. Must be one of:' !global.!valid_repository_types
         If _sourcecontrol = 'cvs' Then
            Do
               If Wordpos( _cvstype, !global.!validcvstypes ) = 0 Then
                  Call AbortText 'Invalid CVS type:' quote( _cvstype ) 'specified. Must be one of:' !global.!validcvstypes
            End
         If FindRepository( _name ) \= 0 Then
            Call AbortText 'You already have a repository of that name in your profile'
         If Stream( _working, C, 'QUERY EXISTS' ) \= '' & Word( Stream( _working, 'C', 'FSTAT' ), 8 ) \= 'Directory' Then
            Call AbortText 'The working directory specified:' quote( _working ) 'is not a directory'
         _working = WorkingDirToInternal( _working )
         If FindWorkingDir( _working ) \= 0 Then
            Call AbortText 'You already have that working directory in your profile'
         If DirectoryExists( _tmpdir ) = 0 Then
            Call AbortText 'The temp directory specified:' quote( _tmpdir ) 'does not exist'
         !global.!!tmpdir = WorkingDirToInternal( _tmpdir )
      End
   Otherwise
      Do
         If repository = 'REPOSITORY' Then
            Do
               repno = !!last_repository
               repository = !global.!!repository.!!name.repno
            End
         Else
            Do
               repno = FindRepository( repository )
               If repno = 0 Then
                  Do
                     Call AbortText 'Could not find any reference to "'repository'" in your user profile.'
                  End
               Else
                  Do
                     !!last_repository = repno
                  End
            End
         /*
          * All generic parameters are now valid, check we are the "build user",
          * and then Call the appropriate function...
          * Can't do this if running "init".
          */
         Call SetRepositoryEnvs
         Call ConnectToRepository !!last_repository
         Call GetConfigurationFile
         Call SetupEnvironment
         /*
          * If not build user, abort
          */
         If !global.!build_user = 'N' Then
            Do
               Call AbortText 'You are not a build user for this repository. Rack off!!'
            End
         If cmd = 'BUILD' | cmd = 'EXPORT' Then
            Do
               If _working_dir \= '_WORKING_DIR' Then
                  Do
                     !global.!!repository.!!working_dir.!!last_repository = _working_dir
                  End
            End
         /*
          * Ensure we are in the "correct" directory
          */
         native_working_dir = Changestr( '/', !global.!!repository.!!working_dir.!!last_repository, !global.!ossep )
         here = Directory()
         Call Directory native_working_dir
      End
End
/*
 * If run direct from batch set our basefile here
 */
If _basefile = '_BASEFILE' Then batch = 1
Else batch = 0
/*
 * Set !global.!current_dirname.!!last_repository to the value of the arg,
 * as existing code expects it set
 * It doesn't matter if _current_dirname is not a valid arg, as it won't be
 * used for those commands that don't expect it as an argument.
 */
!global.!current_dirname.!!last_repository = _current_dirname
/*
 * Execute the code for each of the batch commands...
 */
Select
   When cmd = 'INIT' Then /* batch only */
      Do
         Call InitRepository repository, _sourcecontrol, _username, _password, _module, _extraadminuser
      End
   When cmd = 'LABEL' Then
      Do
         If batch Then _basefile = GenerateJobBaseFile()
         If overwrite = '' Then overwrite_label = 1
         Else overwrite_label = 0
         retcode = LabelModule( 'batch', lab, overwrite_label )
      End
   When cmd = 'MODS' Then
      Do
         /*
          * If no -M specified, then use _current_dirname
          * This check MUST be done BEFORE the test for no -n or no -N
          */
         If _earlier_dirname = '_EARLIER_DIRNAME' Then _earlier_dirname = _current_dirname
         /*
          * If no -n specified, then use !!last_repository (we have already validated this above)
          * If no -N specified, then use same value for -n
          */
         If earlier_repository = 'EARLIER_REPOSITORY' Then
            Do
               earlier_repostitory = repository
               earlier_repno = repno
               !global.!current_dirname.repno = _earlier_dirname
            End
         Else
            Do
               earlier_repno = FindRepository( earlier_repository )
               If earlier_repno = 0 Then
                  Do
                     Call AbortText 'Could not find any reference to "'earlier_repository'" in your user profile.'
                  End
               !global.!current_dirname.earlier_repno = _earlier_dirname
            End

         If IsModuleInRepository( _current_dirname, repno ) = 0 Then Call Aborttext 'Later Module:' _current_dirname 'does not exist.'
         If earlier_repno \= repno Then
            Do
               If IsModuleInRepository( _earlier_dirname, earlier_repno ) = 0 Then Call Aborttext 'Earlier Module:' _earlier_dirname 'does not exist.'
            End

         If batch Then _basefile = GenerateJobBaseFile()
         retcode = ModsReport( repno, earlier_repno, newlab, oldlab, noprogress, ignorewhitespace, _basefile, batch, _current_dirname, _earlier_dirname, _wc_flag, _sa_flag )
         /*
          * Write the return code to the indicator file...
          */
         Call Stream _basefile, 'C', 'OPEN WRITE REPLACE'
         Call Lineout _basefile,retcode
         Call Stream _basefile, 'C', 'CLOSE'
      End
   When cmd = 'COPY' Then
      Do
         If batch Then _basefile = GenerateJobBaseFile()
         retcode = CopyRepository( repository, new_dirname, lab, overwrite, _basefile, batch )
         /*
          * Write the return code to the indicator file...
          */
         Call Stream _basefile, 'C', 'OPEN WRITE REPLACE'
         Call Lineout _basefile,retcode
         Call Stream _basefile, 'C', 'CLOSE'
      End
   When cmd = 'BUILD' Then
      Do
         If batch Then _basefile = GenerateJobBaseFile()
         retcode = RunBuild( repository, lab, buildtype, overwritebuild, testbuild, _basefile, batch, dontcheckout, continuecheckout )
         /*
          * Write the return code to the indicator file...
          */
         Call Stream _basefile, 'C', 'OPEN WRITE REPLACE'
         Call Lineout _basefile,retcode
         Call Stream _basefile, 'C', 'CLOSE'
      End
   When cmd = 'PROMOTE' Then
      Do
         If batch Then _basefile = GenerateJobBaseFile()
         retcode = RunPromote( repository, lab, _basefile, batch )
         /*
          * Write the return code to the indicator file...
          */
         Call Stream _basefile, 'C', 'OPEN WRITE REPLACE'
         Call Lineout _basefile,retcode
         Call Stream _basefile, 'C', 'CLOSE'
      End
   When cmd = 'ADD' Then
      Do
         /*
          * Set the new repository details in the array, and write out the new
          * .qocca
          */
         /*
          * If CVS pserver, we need to Call 'cvs login' to save the password
          * The method employed below allows 'cvs login' to take the password
          * from an environment variable rather than prompting the user.
          */
         If _cvstype = 'pserver' Then
            Do
               /*
                ( If no username/password supplied there must be an existing
                * password in .cvspass for this repository
                */
               fullpath = ':pserver:'_username'@'_server':'repository
               If _password = '_PASSWORD' Then Nop
               Else
                  Do
                     Call Value 'CVS_PASS', _password, 'ENVIRONMENT'
                     cmd = 'cvs -q -d' fullpath 'login'
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address System cmd
                     If rc \= 0 Then
                        Call AbortText 'An error occured logging onto the repository at:' fullpath
                     Call Value 'CVS_PASS', '', 'ENVIRONMENT'
                  End
            End
         If !global.!!number_repositories = 0 | Left( !global.!!number_repositories, 1 ) = '!' Then !global.!!number_repositories = 1
         Else !global.!!number_repositories = !global.!!number_repositories + 1
         idx = !global.!!number_repositories

         !global.!!repository.!!type.idx = _sourcecontrol
         If !global.!!repository.!!type.idx = 'cvs' Then
            Do
               Select
                  When _cvstype = 'local'   Then !global.!!repository.!!path.idx = ':local:'repository
                  When _cvstype = 'pserver' Then !global.!!repository.!!path.idx = fullpath
                  Otherwise !global.!!repository.!!path.idx = ':ext:'user'@'server':'repository
               End
               !global.!!repository.!!contype.idx = _cvstype
               !global.!!repository.!!enhanced_cvs.idx = 'Y'
               !global.!!repository.!!cvsport.idx = '2401'
            End
         Else
            Do
               !global.!!repository.!!contype.idx = ''
               !global.!!repository.!!path.idx = repository
               !global.!!repository.!!enhanced_cvs.idx = ''
               !global.!!repository.!!cvsport.idx = ''
            End
         !global.!!repository.!!username.idx = _username
         !global.!!repository.!!password.idx = ''
         !global.!!repository.!!shortpath.idx = repository
         !global.!!repository.!!server.idx = _server
         !global.!!repository.!!name.idx = _name
         !global.!!repository.!!working_dir.idx = _working
         !global.!!repository.!!last_tree_opened.idx = ''
         !global.!!repository.!!public_cvs.idx = ''
         !global.!!repository.!!public_cvs_dirs.idx = ''
         !global.!!repository.!!filefilters.idx = ''
         !global.!!repository.!!cvs_compression.idx = '-z3'
         /*
          * Create the working directory if it dowsn't exist
          */
         Call CreateDirectory _working
         /*
          * Ensure that WriteUserSettingsFile() actually works
          */
         !!last_repository = idx
         !global.!!user.!!textpager = 'more'
         Call WriteUserSettingsFile
      End
   When cmd = 'EXPORT' Then
      Do
         Call Time 'R'
         /*
          * If not exporting Latest, check if it has been labeled
          */
         If lab \= 'Latest' Then
            Do
               If IsFileInRepository( !global.!current_dirname.!!last_repository'/.label-'lab ) = 0 Then
                  Do
                     Call AbortText '"ERROR: Module:' quote( !global.!current_dirname.!!last_repository ) 'does not have a label of:' lab'"'
                     Return 1
                  End
            End
         build_dir = !global.!current_dirname.!!last_repository
         /*
          * Ensure the specified working directory is not the same as the project's
          * working directory
          */
         If Stream( build_dir, 'C', 'QUERY EXISTS' ) = Stream( native_working_dir, 'C', 'QUERY EXISTS' ) Then
            Do
               Call AbortText '"ERROR: Specified working directory:' quote( native_working_dir ) "cannot be the same as the project's working directory of:" quote( build_dir )
               Return 1
            End
         /*
          * Create the working directory if necessary
          */
         Address System !global.!cmdmkdir quote( native_working_dir )
         /*
          * ... and change to it
          */
         Call Directory( native_working_dir )
         outputfile = Changestr( '/', native_working_dir||!global.!ossep||'qocca_bg_'||Date('S')||Changestr(':', Time(), '' ), !global.!ossep ) || '.out'
         /*
          * Export the directory tree
          * Due to an annoying feature of CVS, we can't export to a local
          * directory pointed to by the -d switch, so we have to check out
          * each file seperately to the correct directory
          */
         If lab = 'Latest' Then
            Do
               /*
                * Get a list of all files in the module with their revision
                * numbers
                */
               Select
                  When !global.!!repository.!!type.!!last_repository = 'cvs' Then
                     Do
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q list' quote( !global.!current_dirname.!!last_repository )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address System cmd With Output Stem out. Error Stem err.
                        /* TODO don't use cvs list!!! */
                        Do i = 1 To out.0
                           Parse Var out.i . rev . . . . flen .
                           Parse Var out.i 92 fn +(flen) +1 .
                           out.i = rev fn
                        End
                     End
                  Otherwise Nop
               End
            End
         Else
            Do
               /*
                * Read the label file from the repository into a stem
                */
               Select
                  When !global.!!repository.!!type.!!last_repository = 'cvs' Then
                     Do
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q checkout -p' quote( !global.!current_dirname.!!last_repository'/.label-'lab )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address System cmd With Output Stem out. Error Stem junk.
                     End
                  Otherwise Nop
               End
            End
         dirlen = Length( !global.!current_dirname.!!last_repository )
         /*
          * If the OS can handle symlinks, then check if the common dirs are in the out. stem
          * and check these out first. Then we can create local symlinks and not have to checkout
          * all the linked files.
          * We then ignore the files we checked out previously
          */
         ignore. = 0
         If !global.!symlink \= '' Then
            Do
               Do j = 1 To !global.!common_dir.0
                  dir = Substr( !global.!common_dir.j, 2 ) || '/'
                  len = Length( dir )
                  Do i = 1 To out.0
                     If Strip( out.i) = '' | Left( out.i, 1 ) = ';' Then Iterate i
                     Parse Var out.i rev fn
                     If Left( fn, len ) = dir Then
                        Do
                           have_common_dir.j = 1
                           ignore.i = 1 -- this ensures we don't check this file out again below
                           Call CheckoutAFile rev, fn, build_dir, dirlen, batch, outputfile, continuecheckout
                        End
                  End
               End
               /*
                * Now for all files check if they are in the linked directories
                * that we have checked out above and set their ignore flag
                */
               Do i = 1 To out.0
                  If Strip( out.i) = '' | Left( out.i, 1 ) = ';' Then Iterate i
                  Parse Var out.i rev fn
                  Do j = 1 To !global.!linked_dir.0
                     ld = Substr( !global.!linked_dir.j, 2 ) || '/'
                     len = Length( ld )
                     If ld = Left( fn, len ) Then
                        Do
                           idx = !global.!linked_idx.j
                           If have_common_dir.idx = 1 Then
                              Do
                                 ignore.i = 1
                              End
                        End
                  End
               End
               /*
                * Now create symbolic links so any updated files get
                * checked out into the common area
                */
               tmpdir = Directory()
               /*
                * We now need to go through the linked directories and setup all of the
                * symbolic links to the common dirs
                */
               Do j = 1 To !global.!linked_dir.0
                  common_idx = !global.!linked_idx.j
                  If have_common_dir.common_idx = 1 Then
                     Do
                        Call Directory tmpdir
                        /*
                         * Work out how to get the correct relative path for linking
                         */
                        pos = Lastpos( '/', !global.!linked_dir.j )
                        linked_dirname = Substr( !global.!linked_dir.j, 1, pos - 1 ) /* eg /test/dir1/dir2 */
                        common_dirname = !global.!common_dir.common_idx /* /common */
                        mycomm = common_dirname
                        newdir = linked_dirname
                        Do i = 1 To Min( Length( mycomm ), Length( newdir ) )
                           If Substr( mycomm, i, 1 ) = '/' Then last_dir_pos = i
                           If Substr( mycomm, i, 1 ) \= Substr( newdir, i, 1 ) Then Leave
                        End
                        rel_linked = Substr( newdir, last_dir_pos + 1 )
                        rel_common = Substr( mycomm, last_dir_pos + 1 )
                        /*
                         * Now symlink the common dir to the linked dirname
                         * eg. Linking /tmfps/common to /tmfps/app/common
                         *     cd /tmfps/app
                         *     ln -sf ../common .
                         */
                        Say '"Linking directory' quote( build_dir'/'rel_common ) 'to current directory:' quote( build_dir'/'rel_linked )'"'
                        /*
                         * Create (it doesn't exist) and change directory to linked dir
                         */
                        Call CreateDirectory build_dir'/'rel_linked
                        Call Directory build_dir'/'rel_linked
                        cmd = !global.!symlink quote( '../'rel_common ) '.'
                        Address Command cmd
                      End
               End
               Call Directory tmpdir
            End
         /*
          * We now have a stem; out. with all files and their revision numbers
          */
         checkedout_dirs.0 = 0
         Do i = 1 To out.0
            If Strip( out.i) = '' | Left( out.i, 1 ) = ';' Then Iterate i
            /*
             * If we have our ignore flag set, ignore the entry
             */
            If ignore.i = 1 Then Iterate i
            Parse Var out.i rev fn
            /*
             * Determine if we have already checked out this directory.
             */
            outfile = build_dir||Substr( fn, dirlen+1 )
            outpos = Lastpos( '/', outfile )
            outdir = Substr( outfile, 1, outpos-1 )
            found = 0
            Do j = 1 To checkedout_dirs.0
               If outdir = checkedout_dirs.j Then
                  Do
                     found = 1
                     Leave
                  End
            End
            If found = 0 Then
               Do
                  Call Runner batch, outputfile, !global.!cmdecho '"Checking out files for directory:' outdir'"'
                  j = 1 + checkedout_dirs.0
                  checkedout_dirs.j = outdir
                  checkedout_dirs.0 = j
               End
            rc = CheckoutAFile( rev, fn, build_dir, dirlen, batch, outputfile, continuecheckout )
            If rc \= 0 Then
               Do
                  Return rc
               End
         End
         -- remove the outputfile; we don't need it
         Call SysFileDelete outputfile
         et = Time( 'E' )
         Parse Var et et '.' .
         If et = '' Then et = 0
         hr = et % 3600
         et = et // 3600
         mi = et % 60
         se = et // 60
         Say  '">>>>Completed successfully at' Date() Time() 'Elapsed:' Right( hr, 2, 0 )':'Right( mi, 2, 0 )':'Right( se, 2, 0)'"'
      End
   Otherwise
      Do
         Say 'This batch command has not been implemented yet.'
      End
End

Call Directory here
Call CleanupAndExit retcode
Return /* shouldn't get here! as CleanupAndExit() exits */

/*
 * Called from batch, so use Runner etc to log details
 */
CopyRepository: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg repository, new_dirname, lab, overwrite, _basefile, batch
outputfile = _basefile'.out'
retcode = 0
/*
 * If called from batch, create our description file so we can see
 * the job in the GUI QOCCA and use "tee" so the output goes to the ".out" file
 * for later viewing
 */
If batch Then
   Do
      desc = 'Copy of version' quote( lab ) 'of' quote( !global.!current_dirname.!!last_repository ) 'to' quote( new_dirname )
      type = 'copy'
      compiler_type = '.'
      Call CreateDescriptionFile _basefile, desc, type, compiler_type
   End
/*
 * Validate the parameters...
 */
here = Directory()
If DirectoryExists( new_dirname ) = 1 & overwrite = 'OVERWRITE' Then
   Do
      Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The new directory:' new_dirname 'already exists and you did not request it be overwritten."'
      Return 1
   End

Select
   When !global.!!repository.!!type.!!last_repository = 'rcs' Then
      Do
      End
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Getting label file for:' lab'"'
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q list' !global.!current_dirname.!!last_repository'/.label-'lab
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output Stem out. Error Stem err.
         If out.0 \= 1 Then
            Do
               Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The supplied module:' !global.!current_dirname.!!last_repository 'does not have a label of:' lab'."'
               Return 1
            End
         /*
          * Checkout the label file into the stack, and for each
          * CI, export it to a temporary directory
          */
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout -p' !global.!current_dirname.!!last_repository'/.label-'lab
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output Stem label. Error Stem err.
         If label.0 = 0 Then
            Do
               Call Runner batch, outputfile, !global.!cmdecho '"ERROR: No configuration items found in label file for version:' lab 'for module:' !global.!current_dirname.!!last_repository'."'
               Do i = 1 To err.0
                  Call Runner batch, outputfile, !global.!cmdecho '"ERROR:' err.i'"'
               End
               Return 1
            End
         /*
          * Clean up the working directory and the new dir...
          */
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Cleaning up working directory and new repository."'
         cvs_tag = Changestr( '/', !global.!current_dirname.!!last_repository, '_') || '_' || lab
         Call Runner batch, outputfile, !global.!cmddelr working_dir
         Call Runner batch, outputfile, !global.!cmdmkdir working_dir

         Call Runner batch, outputfile, !global.!cmddelr new_dirname
         /*
          * Create the new CVS repository
          */
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Creating the new repository."'
         Call Runner batch, outputfile, 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d' new_dirname 'init'
         /*
          * Copy repository files from old repository to new repository...
          * Don't copy files that are in linked directories; copy the originals
          * instead and create the links
          */
         linked_dirs = ''
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Copying repository files..."'
         Do i = 1 To label.0
            Parse Var label.i ver fn
            If ver = ';' Then Iterate i

            /*
             * For each file, copy the CVS repository file from one
             * directory to another
             */
            wrkfn = Changestr( '/', fn , !global.!ossep )
            wkpos = Lastpos( !global.!ossep, wrkfn )
            wkdir = Substr( wrkfn, 1, wkpos-1 )
            new_repdir = new_dirname'/'wkdir
            old_repdir = !global.!!repository.!!shortpath.!!last_repository'/'wkdir
            old_fn = !global.!!repository.!!shortpath.!!last_repository'/'fn',v'
            /*
             * Is the directory linked ?
             */
            Do j = 1 To !global.!linked_dir.0
               If !global.!linked_dir.j = '/'wkdir Then
                  Do
                     If Wordpos( j, linked_dirs ) = 0 Then
                        Do
                           linked_dirs = linked_dirs j
                        End
                     Iterate i
                  End
            End
            If DirectoryExists( new_repdir ) = 0 Then
               Do
                  /*
                   * Create the directory in the repository if it doesn't exist
                   */
                  Call Runner batch, outputfile, !global.!cmdecho '"Creating directory in repository:' new_repdir'"'
                  Call CreateDirectory new_repdir
               End
            /*
             * Copy the file into the repository directory
             */
            Call Runner batch, outputfile, !global.!cmdecho '"Copying:' old_fn 'to' new_repdir'"'
            retcode = Runner( batch, outputfile, 'cp' quote( old_fn ) quote( new_repdir ) )
            If retcode \= 0 Then
               Do
                  Call Runner batch, outputfile, !global.!cmdecho '"ERROR: Copying' fn'"'
                  Return 1
               End
         End
         /*
          * Find the original directories that were linked into this part of the repository
          * and copy all the files to the new repository.
          * Then symbolically link the "common" directories into the copied directories
          */
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Copying linked directories and linking directories..."'
         tmpdir = Directory()
         Call Directory new_dirname
         Do i = 1 To Words( linked_dirs )
            idx = Word( linked_dirs, i )
            common_idx = !global.!linked_idx.idx
            /*
             * If we haven't copied the common directory into the destination,
             * do it now
             */
            full_common = new_dirname || !global.!common_dir.common_idx
            If DirectoryExists( full_common ) = 0 Then
               Do
                  Call Runner batch, outputfile, !global.!cmdecho '"Creating directory in repository:' full_common'"'
                  Call CreateDirectory full_common
                  Call Directory !global.!!repository.!!shortpath.!!last_repository || !global.!common_dir.common_idx
                  Address System 'tar cf - * .??* | (cd' quote( full_common )'; tar xvf -)'
/*
                  Address System 'cp -pR' quote( !global.!!repository.!!shortpath.!!last_repository || !global.!common_dir.common_idx )'/$*' quote( full_common )
 */
               End
            /*
             * Work out how to get the correct relative path for linking
             */
            pos = Lastpos( '/', !global.!linked_dir.idx )
            linked_dirname = Substr( !global.!linked_dir.idx, 1, pos - 1 ) /* eg /test/dir1/dir2 */
            common_dirname = !global.!common_dir.common_idx /* /common */
            /*
             * Get the last directory component of the current directory
             */
            mycomm = Strip( common_dirname, 'T', '/' )
            lp = Lastpos( '/', mycomm )
            lastdir = Substr( mycomm, lp )
            newdir = linked_dirname || lastdir
            mytail = mycomm
            /*
             * Determine how many common, leading directories there are. This is used to reduce
             * the number of relative directories we need to go back up to find the linked
             * directory.
             * eg:
             * mycomm = /dir1/apath
             * newdir = /dir1/fred/apath
             *
             * Should result in ln -s ../apath . rather than ../../dir1/apath
             */
            Do j = 1 To Min( Length( mycomm ), Length( newdir ) )
               If Substr( mycomm, j, 1 ) = '/' Then last_dir_pos = j
               If Substr( mycomm, j, 1 ) \= Substr( newdir, j, 1 ) Then Leave
            End
            num_common_dirs = Countstr( '/', Substr( mycomm, 1, j ) ) - 1
            If num_common_dirs \= 0 Then mytail = Substr( mytail, last_dir_pos )
            /*
             * Now symlink the common dir to the linked dirname
             * eg. Linking /tmfps/common to /tmfps/app/common
             *     cd /tmfps/app
             *     ln -sf ../common .
             */
            Call Directory new_dirname||linked_dirname
            num_dots = Countstr( '/', linked_dirname ) - num_common_dirs
            linkdir = Copies( '../', num_dots ) || Substr( mytail, 2 )
            Call Runner batch, outputfile, !global.!cmdecho '"Linking directory' quote( linkdir ) 'to current directory:' quote( new_dirname||linked_dirname )'"'
            Address Command !global.!symlink quote( linkdir ) .

/*
            Call Directory new_dirname
            dest = Substr( !global.!linked_dir.idx, 2 )
            pos = Lastpos( '/', dest )
            dest = Substr( dest, 1, pos - 1 )
            Call Runner batch, outputfile, !global.!cmdecho '"Linking directory' quote( Substr( !global.!common_dir.common_idx, 2 ) ) 'to' quote( dest )'"'
            Address Command !global.!symlink quote( Substr( !global.!common_dir.common_idx, 2 ) ) quote( dest )
*/


         End
         Call Directory tmpdir
         /*
          * Copy admin/build.conf from old repository and put it
          * in the new one...
          */
         new_repdir = new_dirname'/admin'
         Call CreateDirectory new_repdir
         old_fn = !global.!!repository.!!shortpath.!!last_repository'/admin/build.conf,v'
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Copying admin repository files..."'
         retcode = Runner( batch, outputfile, 'cp' quote( old_fn ) quote( new_repdir ) )

         working_dir = !global.!!tmpdir || 'tmpdir_' || Random( 1, 5000 )
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Changing to temporary working directory:' working_dir'"'
         Call CreateDirectory working_dir
         Call Directory( working_dir )
         /*
          * Checkout the complete copied repository now, so we can work on the individual files
          */
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Checking out repository..."'
         Call Runner batch, outputfile, 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d:local:'new_dirname 'checkout' '.'
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Removing recent revisions..."'
         file_labels = ''
         Do i = 1 To label.0
            Parse Var label.i ver fn
            If ver = ';' Then Iterate i
            /*
             * Get the label information out of the file in the repository
             */
            cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d:local:'new_dirname 'rlog -h' quote( fn )
            If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
            Address System cmd With Output Stem head. Error Stem err.
            If rc \= 0 Then
               Do
                  Call Runner batch, outputfile, !global.!cmdecho '"ERROR: Obtaining labels for' fn'"'
                  Do j = 1 To err.0
                     Call Runner batch, outputfile, !global.!cmdecho '"'err.j'"'
                  End
                  Return 1
               End
            idx = 0
            Do j = 1 To head.0
               If Strip( head.j) \= '' & Left( head.j, 1 ) = '	' Then
                  Do
                     idx = idx + 1
                     tag.idx = head.j
                  End
            End
            tag.0 = idx
            /*
             * For each label find the revision
             */
            Do j = 1 To tag.0
               Parse var tag.j '	' lab ':' rev
               rev = Strip( rev )
               If Datatype( ver ) \= 'NUM' Then
                  Do
                     /*
                      * All numeric versions are > the labeled revision
                      */
                     If Datatype( rev ) = 'NUM' Then
                        Do
                           If Wordpos( lab, file_labels ) = 0 Then file_labels = file_labels lab
                           Call Runner batch, outputfile, 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d:local:'new_dirname 'rtag -d' lab quote( fn )
                        End
                  End
               Else
                  Do
                     If Datatype( rev ) = 'NUM' Then
                        Do
                           If rev > ver Then
                              Do
                                 If Wordpos( lab, file_labels ) = 0 Then file_labels = file_labels lab
                                 Call Runner batch, outputfile, 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d:local:'new_dirname 'rtag -d' lab quote( fn )
                              End
                        End
                  End
            End
            /*
             * All tags for the file after out rev have gone, delete the revisions
             */
            If ver = '1.1.1.1' Then ver = '1.1'
            Call Runner batch, outputfile, 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d:local:'new_dirname 'admin -o'ver'::' quote( fn )
         End
         /*
          * For each label, delete the label file... TODO
          */
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Deleting label files..."'
         Do j = 1 To Words( file_labels )
say 'deleting label:' Word( file_labels, j )
         End
         /*
          * Can't label it as the repository hasn't been set up yet
          * in the user's .qocca - should we?
          */
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Manual intervention required..."'
         Call Runner batch, outputfile, !global.!cmdecho '"If using cvs pserver, you will need to add' new_dirname 'as another repository for cvs pserver to watch."'
         Call Runner batch, outputfile, !global.!cmdecho '"Under Linux, this is done by changing /etc/xinted.d/cvspserver"'
         /*
          * Remove working directory
          */
         Call Directory here
         Call Runner batch, outputfile, !global.!cmddelr working_dir
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Copy complete at' Date() Time()'"'
      End
   Otherwise Nop
End
Return 0

InitRepository: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg repository, _sourcecontrol, _username, _password, _module, _extraadminuser
Select
   When _sourcecontrol = 'cvs' Then
      Do
         !global.!!tmpdir = !global.!home || 'tmpdir_' || Random( 1, 5000 )
         Call CreateDirectory !global.!!tmpdir
         Call Value 'CVSROOT', ':local:'repository, 'ENVIRONMENT'
         Address System !global.!cmdmkdir repository
         Address System 'cvs -d :local:'repository 'init'
         If rc \= 0 Then Call AbortText 'Cannot initialise the CVS repository for' repository
         Call Directory repository
         /*
          * Change to the repository root and create the admin directory and the initial
          * module directory. The initial module is so that an initial checkout can be
          * done by the client.
          */
         Call SysMkdir 'admin'
         If _module \= '_MODULE' Then Call SysMkdir _module
         /*
          * As this is assumed to run on a Unix box (running :local:)
          * then this should be safe...
          */
         Address System 'chmod 775 admin'
         Address System 'chmod 775 .'
         If _module \= '_MODULE' Then Address System 'chmod 775' _module
         /*
          * Create the default build.conf file
          */
         Call Directory !global.!!tmpdir
         Address System 'cvs -d :local:'repository 'get .'
         Call Directory !global.!!tmpdir || !global.!ossep || 'admin'
         tmpfile = 'build.conf'
         If _extraadminuser = '_EXTRAADMINUSER' Then users = !global.!user
         Else users = !global.!user _extraadminuser
         Call BuildFirstBuildConf tmpfile, users
         Address System 'cvs -d :local:'repository 'add' tmpfile
         Address System 'cvs -d :local:'repository 'commit -m"Creation of build.conf"' tmpfile
         /*
          * Create build.no with number = 0
          */
         tmpfile = 'build.no'
         Call Stream tmpfile, 'C', 'OPEN WRITE REPLACE'
         Call Lineout tmpfile, '0'
         Call Stream tmpfile, 'C', 'CLOSE'
         Address System 'cvs -d :local:'repository 'add' tmpfile
         Address System 'cvs -d :local:'repository 'commit -m"Creation of build.no"' tmpfile
         /*
          * Checkout the admin directory
         Call Directory !global.!!repository.!!working_dir.!!last_repository
         Address System 'cvs -d :local:'repository 'get admin'
          */
         Call Directory !global.!home
         Address System !global.!cmddelr !global.!!tmpdir
      End
   Otherwise Nop
End
Return

/*
 * TODO - look at consolidating this and CreateDummyBuildConf
 */
BuildFirstBuildConf: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg tmpfile, users
Call Stream tmpfile, 'C', 'OPEN WRITE REPLACE'
Call Lineout tmpfile, '!admin_users="'users'"'
Call Lineout tmpfile, '!admin_email="'users'"'
Call Lineout tmpfile, '!build_users="'users'"'
Call Lineout tmpfile, '!update_users="'users'"'
Call Lineout tmpfile, '!platforms="'!global.!os'"'
Call Lineout tmpfile, '!common_dir.0="0"'
Call Lineout tmpfile, '!linked_dir.0="0"'
Call Lineout tmpfile, '!linked_idx.0="0"'
Call Lineout tmpfile, '!reservedcheckout="N"'
Call Lineout tmpfile, '!versionenvvar="VERSION"'
Call Lineout tmpfile, '!buildenvvar="BUILD"'
Call Lineout tmpfile, '!builddir="build"'
Call Lineout tmpfile, '!logdir="log"'
Call Lineout tmpfile, '!runtime_envs=""'
Call Lineout tmpfile, '!ignorefiles=""'
Call Stream tmpfile, 'C', 'CLOSE'
Return

/*
 * Run a command with output going to the specified 'outputfile'
 * For a command run in batch the output also goes to 'stdout'
 */
Runner: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg batch, outputfile, cmd
If batch Then
   Do
      Address System cmd '2>&1 |' !global.!cmdtee '-a' outputfile
      retcode = rc
   End
Else
   Do
      Address System cmd With Output Append Stream outputfile Error Append Stream outputfile
      retcode = rc
   End
Return retcode

/*
 * Write the specified text to the specified 'outputfile'
 * For a batch echo, also 'say' it
 */
Echo: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg batch, outputfile, str
Call Stream outputfile, 'C', 'OPEN WRITE APPEND'
Call Lineout outputfile,str
Call Stream outputfile, 'C', 'CLOSE'
If batch Then Say str
Return 0

ModsReport: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg repno, earlier_repno, newlab, oldlab, noprogress, ignorewhitespace, _basefile, batch, _latest_dirname, _earlier_dirname, _wc_flag, _sa_flag
If _wc_flag = '_WC_FLAG' Then _wc_flag = 0
Else _wc_flag = 1
If _sa_flag = '_SA_FLAG' Then _sa_flag = 0
Else _sa_flag = 1
/*
 * - Check if both newlab and oldlab labels exist for the module in the repository
 * - Compare the label files for each label
 */
new_heading = !global.!!repository.!!name.repno':'_latest_dirname'('newlab')'
old_heading = !global.!!repository.!!name.earlier_repno':'_earlier_dirname'('oldlab')'
diffs = old_heading 'and' new_heading
If batch Then
   Do
      /*
       * Called from batch, create our description file so we can see
       * the job in the GUI QOCCA and use "tee" so the output goes to the ".out" file
       * for later viewing
       */
      report_desc = 'Report differences between:' diffs
      type = 'mods'
      compiler_type = '.'
      Call CreateDescriptionFile _basefile, report_desc, type, compiler_type
   End
outputfile = _basefile'.out'
Call Runner batch, outputfile, !global.!cmdecho '">>>>Reading label files from repository..."'

if trace() = 'F' Then say repno earlier_repno newlab oldlab noprogress ignorewhitespace basefile
dirlen = Length( _latest_dirname )
earlier_dirlen = Length( _earlier_dirname )
/*
 * Both repository types must be the same...
 */
If !global.!!repository.!!type.repno \= !global.!!repository.!!type.earlier_repno Then Call AbortText 'The repository types are different;' !global.!!repository.!!name.repno 'is' !global.!!repository.!!type.repno',' !global.!!repository.!!name.earlier_repno 'is' !global.!!repository.!!type.earlier_repno
Select
   When !global.!!repository.!!type.repno = 'cvs' Then
      Do
         /*
          * Check value of label for "earlier" version...
          */
         /*
          * First check if label is "Latest". If so, get the list of files from the
          * repository, not the label file
          */
         Select
            When oldlab = 'Latest' Then
               Do
                  /*
                   * We need to be in the working_dir for earlier_repno
                   */
                  here = Directory()
                  Call Directory !global.!!repository.!!working_dir.earlier_repno
                  olddir = _earlier_dirname
                  If GetListOfCVSFilesInDirectory( earlier_repno, _earlier_dirname, 0 ) \= 0 Then
                     Do
                        Call Runner batch, outputfile, !global.!cmdecho '"ERROR: Getting Latest file list from:' !global.!!repository.!!name.earlier_repno' Command:' cmd'"'
                        Do i = 1 To err.0
                           Call Runner batch, outputfile, !global.!cmdecho '"   'err.i'"'
                        End
                        Call Directory here
                        Return 1
                     End
                  Do i = 1 To item.0
                     Parse Var item.i . rev . . . . flen .
                     Parse Var item.i 92 fn +(flen) +1 .
                     oldfile.i = rev fn
                  End
                  oldfile.0 = item.0
                  Drop item.
                  Call Directory here
               End
            When oldlab = 'Working' Then
               Do
                  offset = Length( !global.!!repository.!!shortpath.earlier_repno||!global.!ossep ) + 1
                  olddir = _earlier_dirname
                  /*
                   * We need to be in the working_dir for earlier_repno
                   */
                  here = Directory()
                  Call Directory !global.!!repository.!!working_dir.earlier_repno
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.earlier_repno '-d'!global.!!repository.!!path.earlier_repno '-q status' quote( _earlier_dirname )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address System cmd With Output Stem out. Error Stem err.
                  If rc \= 0 Then
                     Do
                        Call Runner batch, outputfile, !global.!cmdecho '"ERROR: Getting Working file list from working directory for:' _earlier_dirname 'in repository' !global.!!repository.!!name.earlier_repno'"'
                        Do i = 1 To err.0
                           Call Runner batch, outputfile, !global.!cmdecho err.i
                        End
                        Call Directory here
                        Return 1
                     End
                  idx = 0
                  Do i = 1 To out.0
                     Parse Var out.i . 'Status:' stat
                     If stat \= '' Then
                        Do
                           i_off = i + 2
                           Parse Var out.i_off . 'Working revision:' rev .
                           i_off = i_off + 1
                           Parse Var out.i_off . 'Repository revision:' rep_rev fn
                           If Datatype( Changestr( '.', rep_rev, '' ) ) = 'NUM' Then
                              Do
                                 idx = idx + 1
                                 If stat = 'Locally Modified' Then rev = rev'+'
                                 oldfile.idx = Substr( fn, offset )
                                 oldfile.idx = rev Left( oldfile.idx, Length( oldfile.idx ) - 2 )
                              End
                        End
                  End
                  Call Directory here
                  oldfile.0 = idx
               End
            Otherwise /* explicit version */
               Do
                  Parse Var oldlab olddir ',' oldlab
                  oldlabelfile = olddir || '/.label-' || oldlab
                  /*
                   * We need to be in the working_dir for earlier_repno
                   */
                  here = Directory()
                  Call Directory !global.!!repository.!!working_dir.earlier_repno
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.earlier_repno '-d'!global.!!repository.!!path.earlier_repno '-q get -p' quote( oldlabelfile )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address System cmd With Output Stem oldfile. Error Stem junk.
                  If oldfile.0 = 0 Then
                     Do
                        Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The Earlier Version:' oldlab 'has not been labeled"'
                        Call Directory here
                        Return 1
                     End
                  Call Directory here
               End
         End
         /*
          * Check value of label for "later" version...
          */
         /*
          * First check if label is "Latest". If so, get the list of files from the
          * repository, not the label file
          */
         Select
            When newlab = 'Latest' Then
               Do
                  /*
                   * We need to be in the working_dir for repno
                   */
                  here = Directory()
                  Call Directory !global.!!repository.!!working_dir.repno
                  newdir = _latest_dirname
                  If GetListOfCVSFilesInDirectory( repno, _latest_dirname, 0 ) \= 0 Then
                     Do
                        Call Runner batch, outputfile, !global.!cmdecho '"ERROR: Getting Latest file list from:' !global.!!repository.!!name.repno' Command:' cmd'"'
                        Do i = 1 To err.0
                           Call Runner batch, outputfile, !global.!cmdecho '"   'err.i'"'
                        End
                        Call Directory here
                        Return 1
                     End
                  Do i = 1 To item.0
                     Parse Var item.i . rev . . . . flen .
                     Parse Var item.i 92 fn +(flen) +1 .
                     newfile.i = rev fn
                  End
                  newfile.0 = item.0
                  Drop item.
                  Call Directory here
               End
            When newlab = 'Working' Then
               Do
                  offset = Length( !global.!!repository.!!shortpath.repno||!global.!ossep ) + 1
                  newdir = _latest_dirname
                  /*
                   * We need to be in the working_dir for repno
                   */
                  here = Directory()
                  Call Directory !global.!!repository.!!working_dir.repno
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.repno '-d'!global.!!repository.!!path.repno '-q status' quote( _latest_dirname )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address System cmd With Output Stem out. Error Stem err.
                  If rc \= 0 Then
                     Do
                        Call Runner batch, outputfile, !global.!cmdecho '"ERROR: Getting Working file list from working directory for:' _latest_dirname 'in repository' !global.!!repository.!!name.repno'"'
                        Do i = 1 To err.0
                           Call Runner batch, outputfile, !global.!cmdecho err.i
                        End
                        Call Directory here
                        Return 1
                     End
                  idx = 0
                  Do i = 1 To out.0
                     Parse Var out.i . 'Status:' stat
                     If stat \= '' Then
                        Do
                           i_off = i + 2
                           Parse Var out.i_off . 'Working revision:' rev .
                           i_off = i_off + 1
                           Parse Var out.i_off . 'Repository revision:' rep_rev fn
                           If Datatype( Changestr( '.', rep_rev, '' ) ) = 'NUM' Then
                              Do
                                 idx = idx + 1
                                 If stat = 'Locally Modified' Then rev = rev'+'
                                 newfile.idx = Substr( fn, offset )
                                 newfile.idx = rev Left( newfile.idx, Length( newfile.idx ) - 2 )
                              End
                        End
                  End
                  Call Directory here
                  newfile.0 = idx
               End
            Otherwise /* explicit version */
               Do
                  Parse Var newlab newdir ',' newlab
                  newlabelfile = newdir || '/.label-' || newlab
                  /*
                   * We need to be in the working_dir for repno
                   */
                  here = Directory()
                  Call Directory !global.!!repository.!!working_dir.repno
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.repno '-d'!global.!!repository.!!path.repno '-q get -p' quote( newlabelfile )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address System cmd With Output Stem newfile. Error Stem junk.
                  If newfile.0 = 0 Then
                     Do
                        Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The Later Version:' newlab 'has not been labeled"'
                        Call Directory here
                        Return 1
                     End
                  Call Directory here
               End
         End
      End
   Otherwise Nop
End

Call Runner batch, outputfile, !global.!cmdecho '">>>>Sorting label files..."'
/*
 * Remove comments and other junk from (earlier) label file and sort it
 */
idx = 0
Do i = 1 To oldfile.0
   rev = Word( oldfile.i, 1 )
   mod = Subword( oldfile.i, 2 )
   /*
    * Support old label files that had the ,v files listed. Strip the ,v if it exists
    */
   If Right( mod, 2 ) = ',v' Then mod = Changestr( ',v', mod, '' )
   /*
    * Only include non-comment lines and modules that have the
    * same leading directory as the specified module (this is for
    * reporting on a label done at a higher level than where we are )
    */
   If rev \= ';' & Left( mod, earlier_dirlen ) = _earlier_dirname Then
      Do
         idx = idx + 1
/*
         oldfile.idx = Left( rev, 10 ) mod
*/
         oldfile.idx = Left( rev, 10 ) Substr( mod, earlier_dirlen+2 )
      End
End
oldfile.0 = idx
Call SysStemSort 'oldfile.', 'ascending', 'sensitive', , , 12, 250

/*
 * Remove comments and other junk from (later) label file and sort it
 */
idx = 0
Do i = 1 To newfile.0
   rev = Word( newfile.i, 1 )
   mod = Subword( newfile.i, 2 )
   /*
    * Support old label files that had the ,v files listed. Strip the ,v if it exists
    */
   If Right( mod, 2 ) = ',v' Then mod = Changestr( ',v', mod, '' )
   /*
    * Only include non-comment lines and modules that have the
    * same leading directory as the specified module (this is for
    * reporting on a label done at a higher level than where we are )
    */
   If rev \= ';' & Left( mod, dirlen ) = _latest_dirname Then
      Do
         idx = idx + 1
/*
         newfile.idx = Left( rev, 10 ) mod
*/
         newfile.idx = Left( rev, 10 ) Substr( mod, dirlen+2 )
      End
End
newfile.0 = idx
Call SysStemSort 'newfile.', 'ascending', 'sensitive', , , 12, 250
Call Runner batch, outputfile, !global.!cmdecho '">>>>Comparing label files..."'
old_file = Strip( SubWord( oldfile.1, 2 ) )
old_rev = Strip( Word( oldfile.1, 1 ) )
new_file = Strip( SubWord( newfile.1, 2 ) )
new_rev = Strip( Word( newfile.1, 1 ) )
old_idx = 1
new_idx = 1
comb_idx = 0
eof = Copies('z', 10)
Do Forever
   Select
      When old_file > new_file Then /* added in new */
         Do
            comb_idx = comb_idx + 1
            comb.comb_idx = '-' new_rev new_file
            If new_idx = newfile.0 Then new_file = eof
            Else
               Do
                  new_idx = new_idx + 1
                  new_file = Strip( SubWord( newfile.new_idx, 2 ) )
                  new_rev = Strip( Word( newfile.new_idx, 1 ) )
               End
         End
      When old_file < new_file Then /* deleted in new */
         Do
            comb_idx = comb_idx + 1
            comb.comb_idx = old_rev '-' old_file
            If old_idx = oldfile.0 Then old_file = eof
            Else
               Do
                  old_idx = old_idx + 1
                  old_file = Strip( SubWord( oldfile.old_idx, 2 ) )
                  old_rev = Strip( Word( oldfile.old_idx, 1 ) )
               End
         End
      Otherwise    /* same file */
         Do
            comb_idx = comb_idx + 1
            comb.comb_idx = old_rev new_rev old_file
            If old_idx = oldfile.0 Then old_file = eof
            Else
               Do
                  old_idx = old_idx + 1
                  old_file = Strip( SubWord( oldfile.old_idx, 2 ) )
                  old_rev = Strip( Word( oldfile.old_idx, 1 ) )
               End
            If new_idx = newfile.0 Then new_file = eof
            Else
               Do
                  new_idx = new_idx + 1
                  new_file = Strip( SubWord( newfile.new_idx, 2 ) )
                  new_rev = Strip( Word( newfile.new_idx, 1 ) )
               End
         End
   End
   If old_file = eof & new_file = eof Then Leave
End
comb.0 = comb_idx
color_changed = '#87ceeb'
color_deleted = '#ff7800'
color_added   = '#2eee57'
/*
 * Write the reports
 */

Call Runner batch, outputfile, !global.!cmdecho '">>>>Writing report files..."'

jobdatetime = Right( _basefile, 14 )
jobdate = Date( 'N', Left( jobdatetime, 8 ), 'S' )
jobtime = Substr( jobdatetime, 9, 2 )':'Substr( jobdatetime, 11, 2 )':'Substr( jobdatetime, 13, 2 )
jobdatetime = 'Job started on:' jobdate 'at' jobtime
rbasefile = '.'Substr( _basefile, Lastpos( !global.!ossep, _basefile ) )
If _sa_flag Then
   Do
      detail = _basefile'.detail.html'
      Call Stream detail, 'C', 'OPEN WRITE REPLACE'
      Call Lineout detail, '<html><head><title>Details of Differences between:' diffs'</title></head><body bgcolor='!global.!color_background'>'
      Call Lineout detail, '<center><h1>Details of Differences</h1></center>'
      Call Lineout detail, '<center><h2>'old_heading'</h2></center>'
      Call Lineout detail, '<center><h2>'new_heading'</h2></center>'
      Call Lineout detail, '<center><h2>'jobdatetime'</h2></center>'
      Call Lineout detail, '<hr>'
      Call Lineout detail, '<center><table border=1 cellspacing=5 cellpadding=2><tr align="center">'
      Call Lineout detail, '<td width=33% bgcolor='color_changed'>File Changed</td><td width=33% bgcolor='color_added'>File Added</td><td width=33% bgcolor='color_deleted'>File Deleted</td>'
      Call Lineout detail, '</tr></table><//center><p>'
      Call Lineout detail, '<center><table border=1 cellspacing=5 cellpadding=2><tr align="center">'
      Call Lineout detail, '<td><b>'!global.!!repository.!!name.earlier_repno'<br>'olddir'<br>'oldlab'</b></td>'
      Call Lineout detail, '<td><b>'!global.!!repository.!!name.repno'<br>'newdir'<br>'newlab'</b></td>'
      Call Lineout detail, '<td><b>File</b></td>'
      Call Lineout detail, '</tr>'
   End

summary = _basefile'.summary.html'
Call Stream summary, 'C', 'OPEN WRITE REPLACE'
Call Lineout summary, '<html><head><title>Summary of Differences between:' diffs'</title></head><body bgcolor='!global.!color_background'>'
Call Lineout summary, '<center><h1>Summary of Differences</h1></center>'
Call Lineout summary, '<center><h2>'old_heading'</h2></center>'
Call Lineout summary, '<center><h2>'new_heading'</h2></center>'
Call Lineout summary, '<center><h2>'jobdatetime'</h2></center>'
Call Lineout summary, '<hr>'
Call Lineout summary, '<center><table border=1 cellspacing=5 cellpadding=2><tr align="center">'
Call Lineout summary, '<td width=33% bgcolor='color_changed'>File Changed</td><td width=33% bgcolor='color_added'>File Added</td><td width=33% bgcolor='color_deleted'>File Deleted</td>'
Call Lineout summary, '</tr></table><//center><p>'
Call Lineout summary, '<center><table border=1 cellspacing=5 cellpadding=2><tr align="center">'
Call Lineout summary, '<td><b>'!global.!!repository.!!name.earlier_repno'<br>'olddir'<br>'oldlab'</b></td>'
Call Lineout summary, '<td><b>'!global.!!repository.!!name.repno'<br>'newdir'<br>'newlab'</b></td>'
Call Lineout summary, '<td><b>File</b></td>'
Call Lineout summary, '</tr>'

If _sa_flag Then
   Do
      currents = _basefile'.currents.html'
      Call Stream currents, 'C', 'OPEN WRITE REPLACE'
      Call Lineout currents, '<html><head><title>Current Revisions between:' diffs'</title></head><body bgcolor='!global.!color_background'>'
      Call Lineout currents, '<center><h1>Current Revisions</h1></center>'
      Call Lineout currents, '<center><h2>'new_heading'</h2></center>'
      Call Lineout currents, '<center><h2>'jobdatetime'</h2></center>'
      Call Lineout currents, '<hr>'
      Call Lineout currents, '<center><table border=1 cellspacing=5 cellpadding=2><tr align="center">'
      Call Lineout currents, '<td width=33% bgcolor='color_changed'>File Changed</td><td width=33% bgcolor='color_added'>File Added</td><td width=33% bgcolor='color_deleted'>File Deleted</td>'
      Call Lineout currents, '</tr></table><//center><p>'
      Call Lineout currents, '<center><table border=1 cellspacing=5 cellpadding=2><tr align="center">'
      Call Lineout currents, '<td><b>'!global.!!repository.!!name.repno'<br>'newdir'<br>'newlab'</b></td>'
      Call Lineout currents, '<td><b>File</b></td>'
      Call Lineout currents, '</tr>'
   End

differences_item = 0
differences = _basefile'.differences.html'
rdifferences = '.'Substr( differences, Lastpos( !global.!ossep, differences ) )
Call Stream differences, 'C', 'OPEN WRITE REPLACE'
Call Lineout differences, '<html><head><title>Detailed Differences between:' diffs'</title></head><body bgcolor='!global.!color_background'>'
Call Lineout differences, '<a name="item0"></a>'
Call Lineout differences, '<center><h1>Detailed Differences</h1></center>'
Call Lineout differences, '<center><h2>'old_heading'</h2></center>'
Call Lineout differences, '<center><h2>'new_heading'</h2></center>'
Call Lineout differences, '<center><h2>'jobdatetime'</h2></center>'
Call Lineout differences, '<p>'

comments_item = 0
comments = _basefile'.comments.html'
rcomments = '.'Substr( comments, Lastpos( !global.!ossep, comments ) )
Call Stream comments, 'C', 'OPEN WRITE REPLACE'
Call Lineout comments, '<html><head><title>Detailed Comments between:' diffs'</title></head><body bgcolor='!global.!color_background'>'
Call Lineout comments, '<a name="item0"></a>'
Call Lineout comments, '<center><h1>Detailed Comments</h1></center>'
Call Lineout comments, '<center><h2>'old_heading'</h2></center>'
Call Lineout comments, '<center><h2>'new_heading'</h2></center>'
Call Lineout comments, '<center><h2>'jobdatetime'</h2></center>'
Call Lineout comments, '<center>If comparing between the "Working" version and the "Latest" version, no comments are produced.</center>'
Call Lineout comments, '<p>'
Call Lineout comments, '<hr>'

Call Runner batch, outputfile, !global.!cmdecho '">>>>   Summary report:              ' summary'"'
If _sa_flag Then
   Do
      Call Runner batch, outputfile, !global.!cmdecho '">>>>   Detail report:               ' detail'"'
      Call Runner batch, outputfile, !global.!cmdecho '">>>>   Current Revisions report:    ' currents'"'
   End
Call Runner batch, outputfile, !global.!cmdecho '">>>>   Differences report:          ' differences'"'
Call Runner batch, outputfile, !global.!cmdecho '">>>>   Comments report:             ' comments'"'
If ignorewhitespace = '' Then diffflags = '-bw'
Else diffflags = ''

If !global.!!repository.!!enhanced_cvs.!!last_repository = 'Y' Then logcmd = 'rlog'
Else logcmd = 'log'

Do i = 1 To comb.0
   oldrev = Word( comb.i, 1 )
   newrev = Word( comb.i, 2 )
   fn = SubWord( comb.i, 3 )
   _latest_fn = _latest_dirname'/'fn
   _earlier_fn = _earlier_dirname'/'fn
   /*
    * We need to get the description of the file now so it can be added to the
    * details report filename
    */
   state = 'start'
   fn_desc = ''
   Select
      When !global.!!repository.!!type.repno = 'cvs' Then
         Do
            If _sa_flag | oldrev \= newrev Then
               Do
                  cmd = 'cvs' !global.!!repository.!!cvs_compression.repno '-d'!global.!!repository.!!path.repno '-q' logcmd '-t' quote( _latest_fn )
                  If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                  Address System cmd With Output FIFO ''
                  Do Queued()
                     Parse Pull line
                     Select
                        When state = 'start' Then
                           Do
                              If Left( line, 12 ) = 'description:' Then state = 'desc'
                           End
                        When state = 'desc' Then
                           Do
                              If Left( line, 10 ) = Copies( '=', 10 ) Then state = 'done'
                              Else fn_desc = fn_desc'<br>'line
                           End
                        Otherwise Nop
                     End
                  End
               End
         End
      Otherwise Nop
   End
   If _sa_flag Then
      Do
         Call Lineout currents, '<tr><td>'newrev'</td><td><b>'fn'</b>'fn_desc'</td></tr>'
      End
   If oldrev = newrev Then
      Do
         If _sa_flag Then
            Do
               Call Lineout detail, '<tr><td>'oldrev'</td><td>'newrev'</td><td>'fn'</td></tr>'
            End
      End
   Else
      Do
         Select
            When oldrev = '-' Then color = color_added
            When newrev = '-' Then color = color_deleted
            Otherwise color = color_changed
         End
         row = '<tr bgcolor='color'><td>'oldrev'</td><td>'newrev'</td><td>'fn'</td></tr>'
         If _sa_flag Then
            Do
               Call Lineout detail, row
            End
         Call Lineout summary, row
      End
   If oldrev \= newrev & oldrev \= '-' & newrev \= '-' Then
      Do
         /*
          * When changes exist, display a star
          */
         If noprogress \= '' Then Call Charout ,'*'
         /*
          * Write differences lines...
          */
         Call Lineout differences, ''
         If Right( newrev, 1 ) = '+' Then nrev = ''
         Else nrev = '-r'newrev
         /*
          * We need to get each rev of each file from the
          * appropriate repository to a temp file and do a diff
          * on these files. This is the only way to compare between
          * files in different repositories
          */
         /*
          * For working file, get the full path
          */
         If oldlab = 'Working' Then earlier_tmpfile = !global.!!repository.!!working_dir.earlier_repno || _earlier_fn
         Else
            Do
               earlier_tmpfile = GenerateTempFile() /* we manage this ourselves */
               Select
                  When !global.!!repository.!!type.repno = 'cvs' Then
                     Do
                        /*
                         * We need to be in the working_dir for earlier_repno
                         */
                        here = Directory()
                        Call Directory !global.!!repository.!!working_dir.earlier_repno
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.earlier_repno '-d'!global.!!repository.!!path.earlier_repno '-q get -r'oldrev '-p' quote( _earlier_fn )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output Stream earlier_tmpfile
                        Call Directory here
                     End
                  Otherwise Nop
               End
            End
         /*
          * We are already in the working_dir for repno
          */
         If newlab = 'Working' Then tmpfile = !global.!!repository.!!working_dir.repno || _latest_fn
         Else
            Do
               tmpfile = GenerateTempFile() /* we manage this ourselves */
               Select
                  When !global.!!repository.!!type.repno = 'cvs' Then
                     Do
                        /*
                         * We need to be in the working_dir for repno
                         */
                        here = Directory()
                        Call Directory !global.!!repository.!!working_dir.repno
                        cmd = 'cvs' !global.!!repository.!!cvs_compression.repno '-d'!global.!!repository.!!path.repno '-q get' nrev '-p' quote( _latest_fn )
                        If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                        Address Command cmd With Output Stream tmpfile
                        Call Directory here
                     End
                  Otherwise Nop
               End
            End
         /*
          * Write out a heading for the file being diff'ed, similar to the
          * header that cvs diff produces
          */
         differences_item = differences_item + 1
         Call Lineout differences, '<hr>'
         Call Lineout differences, '<a name="item'differences_item'"></a><a href="'rbasefile'.html">[INDEX]</a><a href="#item0">[TOP]</a><a href="#item'differences_item - 1'">[PREV]</a><a href="#item'differences_item + 1'">[NEXT]</a><a href="'rcomments'#item'differences_item'">[COMMENTS]</a><br>'
         Call Lineout differences, '<b>'fn '(' '-r'oldrev '-r'newrev ')</b><br>'
         /*
          * Win32 .doc differences...
          */
         If !global.!os = 'WIN32' & Translate( Right( fn, 4 ) ) = '.DOC' & _wc_flag Then
            Do
               -- deal with earlier file...
               If oldlab = 'Working' Then
                  Do
                     -- copy the working file to the temporary file
                     earlier_tmpfile1 = GenerateTempFile() -- we delete this ourselves
                     Address System !global.!cmdcopy quote( earlier_tmpfile ) quote( earlier_tmpfile1 ) !global.!devnull
                  End
               Else earlier_tmpfile1 = earlier_tmpfile
               file1 = ConvertWordDocumentToText( earlier_tmpfile1, 1 )
               If file1 = '' Then Queue 'Unable to convert Word document:' fn 'Revision:' oldrev 'to text'
               Else Call AddTempFileForCleanup file1
               -- deal with later file...
               If newlab = 'Working' Then
                  Do
                     -- copy the working file to the temporary file
                     tmpfile2 = GenerateTempFile() -- we delete this ourselves
                     Address System !global.!cmdcopy quote( tmpfile ) quote( tmpfile2 ) !global.!devnull
                  End
               Else tmpfile2 = tmpfile
               file2 = ConvertWordDocumentToText( tmpfile2, 1 )
               If file2 = '' Then Queue 'Unable to convert Word document:' fn 'Revision:' newrev 'to text'
               Else Call AddTempFileForCleanup file2
               /*
                * Run the diff with output to stack if both files converted OK
                */
               If file1 \= '' & file2 \= '' Then Address Command !global.!cmddiff diffflags quote( file1 ) quote( file2 ) With Output FIFO ''
               -- cleanup the copies of the Word documents
               If oldlab = 'Working' Then Call SysFileDelete earler_tmpfile1
               If newlab = 'Working' Then Call SysFileDelete tmpfile2
            End
         Else
            Do
               /*
                * Run the diff with output to stack
                */
               Address Command !global.!cmddiff diffflags quote( earlier_tmpfile ) quote( tmpfile ) With Output FIFO ''
            End
         -- cleanup our temporary files
         If newlab \= 'Working' Then Call SysFileDelete tmpfile
         If oldlab \= 'Working' Then Call SysFileDelete earlier_tmpfile
         Call Lineout differences, '<pre>'
         Do Queued()
            Parse Pull line
            Call Lineout differences, ConvertToHTMLMnemonics( line )
         End
         Call Lineout differences, '</pre>'
         /*
          * Write comments lines...
          */
         Call Lineout comments, '<a name="item'differences_item'"></a>'
         Call Lineout comments, '<table width="100%" border=1>'
         Call Lineout comments, '<tr>'
         Call Lineout comments, '  <td width="70%" colspan=2>'
         Call Lineout comments, '    <b>' fn '(' '-r'oldrev '-r'newrev ')</b>'
         Call Lineout comments, '  </td>'
         Call Lineout comments, '  <td width="30%">'
         Call Lineout comments, '    <a href="'rbasefile'.html">[INDEX]</a><a href="#item0">[TOP]</a><a href="#item'differences_item - 1'">[PREV]</a><a href="#item'differences_item + 1'">[NEXT]<a href="'rdifferences'#item'differences_item'">[DIFFS]</a></a>'
         Call Lineout comments, '  </td>'
         Call Lineout comments, '</tr>'
         /*
          * Don't try and generate comments if we are comparing Latest and Working
          */
         If newlab = 'Working' & oldlab = 'Latest' Then Nop
         Else
            Do
               raw_oldrev = Strip( oldrev, 'T', '+' )
               raw_newrev = Strip( newrev, 'T', '+' )
               If Datatype( raw_oldrev ) \= 'NUM' Then
                  Do
                     comp_revold = 0
                  End
               Else comp_revold = raw_oldrev
               If Datatype( raw_newrev ) \= 'NUM' Then comp_revnew = 0
               Else comp_revnew = raw_newrev
               Select
                  When !global.!!repository.!!type.repno = 'cvs' Then
                     Do
                        /*
                         * We need to convert a CVS version number into a "real" number:
                         * 1.10 is less than 1.9 otherwise
                         * We assume that we never have more than 10000 releases per version
                         */
                        Parse Var comp_revold maj '.' min
                        num_comp_revold = (maj*10000)+min
                        Parse Var comp_revnew maj '.' min
                        num_comp_revnew = (maj*10000)+min
                     End
                  Otherwise Nop
               End
               /*
                * Determine which repository has the latest revision so we can use that one
                * to get the version history
                */
               If num_comp_revnew > num_comp_revold Then
                  Do
                     rlog_repno = repno
                     rlog_fn = _latest_fn
                     later_rev = raw_newrev
                     earlier_rev = raw_oldrev
                  End
               Else
                  Do
                     rlog_repno = earlier_repno
                     rlog_fn = _earlier_fn
                     later_rev = raw_oldrev
                     earlier_rev = raw_newrev
                  End
               /*
                * If the revision numbers are the same, there will be no comments
                */
               If num_comp_revnew \= num_comp_revold Then
                  Do
                     Select
                        When !global.!!repository.!!type.repno = 'cvs' Then
                           Do
                              /*
                               * Do rlog for whichever repository has the later revision
                               * taking into account trailing +
                               */
                              cmd = 'cvs' !global.!!repository.!!cvs_compression.rlog_repno '-d'!global.!!repository.!!path.rlog_repno '-q' logcmd quote( rlog_fn )
                              If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                              Address System cmd With Output FIFO ''
                              state = 'start'
                              Do Queued()
                                 Parse Pull line
                                 Select
                                    When state = 'done' Then Nop -- don't leave; keep pulling all lines off stack
                                    When state = 'start' & Word( line, 1 ) = 'revision' & Word( line, 2 ) >= raw_oldrev & Word( line, 2 ) <= raw_newrev Then
                                       Do
                                          state = 'dateline'
                                          rev = Word( line, 2 )
                                       End
                                    When state = 'more' & Word( line, 1 ) = 'revision' & Word( line, 2 ) <= raw_oldrev Then state = 'done'
                                    When state = 'more' & Word( line, 1 ) = 'revision' Then
                                       Do
                                          state = 'dateline'
                                          rev = Word( line, 2 )
                                       End
                                    When state = 'dateline' Then
                                       Do
                                          Parse Var line 'date: ' yyyy '/' mm '/' dd filetime ';' . 'author:' user ';' .
                                          filedate_t = DateTimeToTime_T( yyyy||mm||dd, filetime, 'S', 1 )
                                          Call Lineout comments, '<tr><td width="10%">' rev '</td><td width="90%" colspan=2><b>Date:</b>' Time_tDateToDisplayDate( filedate_t ) '<b>Author:</b>' user '</td></tr>'
                                          state = 'in'
                                          comment = ''
                                       End
                                    When state = 'in' & Left( line, 20 ) = Copies( '-', 20 ) Then
                                       Do
                                          Call Lineout comments, '<tr><td width="10%"></td><td width="90%" colspan=2>' comment '</td></tr>'
                                          state = 'more'
                                       End
                                    When state = 'in' Then
                                       Do
                                          If comment = '' Then comment = ConvertToHTMLMnemonics( line )
                                          Else comment = comment'<br>'ConvertToHTMLMnemonics( line )
                                       End
                                    Otherwise Nop
                                 End
                              End
                           End
                        Otherwise Nop
                     End
                  End
            End
         Call Lineout comments, '</table>'
      End
   Else
      Do
         /*
          * When no changes, just display a dot
          */
         If noprogress \= '' Then Call Charout ,'.'
      End
End
If _sa_flag Then
   Do
      Call Lineout detail, '</table></center><hr><center><b>End of report</b></center></body></html>'
      Call Stream detail, 'C', 'CLOSE'
      Call Lineout currents, '</table></center><hr><center><b>End of report</b></center></body></html>'
      Call Stream currents, 'C', 'CLOSE'
   End

Call Lineout summary, '</table></center><hr><center><b>End of report</b></center></body></html>'
Call Stream summary, 'C', 'CLOSE'


Call Lineout differences, '<hr><center><b>End of report</b></center></body></html>'
Call Stream differences, 'C', 'CLOSE'

Call Lineout comments, '<hr><center><b>End of report</b></center></body></html>'
Call Stream comments, 'C', 'CLOSE'
Call Runner batch, outputfile, !global.!cmdecho '.'
Call Runner batch, outputfile, !global.!cmdecho '">>>>Completed successfully at' Date() Time()'"'
Return 0

RunBuild: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg repository, lab, buildtype, overwritebuild, testbuild, _basefile, batch, dontcheckout, continuecheckout
outputfile = _basefile'.out'
retcode = 0
Call Time 'R'
/*
 * Check that testbuild is for "Latest" version only
 */
If testbuild \= 'TESTBUILD' & lab \= 'Latest' Then
   Do
      Call Runner batch, outputfile, !global.!cmdecho '"ERROR: TestBuild can only be done with Latest version."'
      Return 1
End
/*
 * If this is a TestBuild, then we need to do the following:
 * - check if there is an existing "__testbuild__" label and if so, do a "Report Module History" between "__testbuild__" and "Latest"
 * - do a build of ourselves without the -t flag
 * - if the build worked, re-label "__testbuild__" and send simple "success" to admin
 * - if build failed, send output and report module details to admin
 */
If testbuild \= 'TESTBUILD' Then
   Do
      /*
       * If called from batch, create our description file so we can see
       * the job in the GUI QOCCA and use "tee" so the output goes to the ".out" file
       * for later viewing
       */
      If batch Then
         Do
            desc = 'Test Build of' quote( !global.!current_dirname.!!last_repository )
            type = 'testbuild'
            compiler_type = FindCompilerType( !global.!current_dirname.!!last_repository )
            Call CreateDescriptionFile _basefile, desc, type, compiler_type
         End
      If IsFileInRepository( !global.!current_dirname.!!last_repository'/.label-__testbuild__'!global.!os'__' ) Then
         Do
            Call Runner batch, outputfile, !global.!cmdecho '">>>>Running Report Modules History between Latest and __testbuild__'!global.!os'__..."'
            /*
             * Following command MUST be run without Runner so that output goes to current outputfile
             */
            Address System !global.!qocca_command 'mods -n'repository '-m'!global.!current_dirname.!!last_repository '-vLatest -V'!global.!current_dirname.!!last_repository',__testbuild__'!global.!os'__' '-B'_basefile '-p -w'
            first_testbuild = 0
         End
      Else first_testbuild = 1
      Select
         When buildtype = 'debug' Then buildtype = '-bdebug'
         When buildtype = 'custom' Then buildtype = '-bcustom'
         Otherwise buildtype = ''
      End
      Call Runner batch, outputfile, !global.!cmdecho '">>>>Building Latest of' !global.!current_dirname.!!last_repository'..."'
      /*
       * Following command MUST be run without Runner so that output goes to current outputfile
       */
      Address System !global.!qocca_command 'build -n'repository '-m'!global.!current_dirname.!!last_repository '-v'lab '-B'_basefile buildtype
      retcode = rc
      emailcommand = FindEmailCommand( !global.!current_dirname.!!last_repository )
      If retcode = 0 Then
         Do
            Call Runner batch, outputfile, !global.!cmdecho '">>>>Build Successful; Creating new __testbuild__'!global.!os'__ label..."'
            /*
             * Following command MUST be run without Runner so that output goes to current outputfile
             */
            Address System !global.!qocca_command 'label -n'repository '-m'!global.!current_dirname.!!last_repository '-v__testbuild__'!global.!os'__' '-o' '-B'_basefile
            If rc = 0 Then
               Do
                  Call Runner batch, outputfile, !global.!cmdecho '">>>>Label Successful!; Sending results to Administrators..."'
                  Call SendEmail emailcommand, outputfile, !global.!admin_email, 'Test Build of' !global.!current_dirname.!!last_repository '- Successful!!'
               End
            Else
               Do
                  Call Runner batch, outputfile, !global.!cmdecho '">>>>Label Failed!; Sending results to Administrators..."'
                  Call SendEmail emailcommand, outputfile, !global.!admin_email, 'Test Build of' !global.!current_dirname.!!last_repository '- Failed!!'
               End
         End
      Else
         Do
            Call Runner batch, outputfile, !global.!cmdecho '">>>>Build Failed!; Sending results to Administrators..."'
            Call SendEmail emailcommand, outputfile, !global.!admin_email, 'Test Build of' !global.!current_dirname.!!last_repository '- Failed!!'
/*
            If first_testbuild = 0 Then
               Do
                  Call SendEmail emailcommand, _basefile'.summary.html', !global.!admin_email, 'Test Build of' !global.!current_dirname.!!last_repository '- Summary'
                  Call SendEmail emailcommand, _basefile'.detail.html', !global.!admin_email, 'Test Build of' !global.!current_dirname.!!last_repository '- Detail'
                  Call SendEmail emailcommand, _basefile'.differences.html', !global.!admin_email, 'Test Build of' !global.!current_dirname.!!last_repository '- Differences'
                  Call SendEmail emailcommand, _basefile'.comments.html', !global.!admin_email, 'Test Build of' !global.!current_dirname.!!last_repository '- Comments'
               End
*/
         End
/*
      Call SysFileDelete _basefile'.summary.html'
      Call SysFileDelete _basefile'.detail'
      Call SysFileDelete _basefile'.differences'
      Call SysFileDelete _basefile'.comments'
*/
   End
Else
   Do
      /*
       * Check that the supplied module is valid and "buildable"
       */
      Select
         When buildtype = 'debug' Then buildcommand = FindDebugBuildCommand( !global.!current_dirname.!!last_repository )
         When buildtype = 'custom' Then buildcommand = FindCustomBuildCommand( !global.!current_dirname.!!last_repository )
         Otherwise buildcommand = FindBuildCommand( !global.!current_dirname.!!last_repository )
      End
      If buildcommand = ''  Then
         Do
            Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The supplied module:' !global.!current_dirname.!!last_repository 'is not buildable."'
            Return 1
         End
      If Left( buildcommand, 1 ) = '!' Then
         Do
            Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The supplied module:' !global.!current_dirname.!!last_repository 'is not valid."'
            Return 1
         End
      /*
       * If not building Latest, check if it has been labeled
       */
      If lab \= 'Latest' Then
         Do
            If IsFileInRepository( !global.!current_dirname.!!last_repository'/.label-'lab ) = 0 Then
               Do
                  Call Runner batch, outputfile, !global.!cmdecho '"ERROR: Module:' quote( !global.!current_dirname.!!last_repository ) 'does not have a label of:' lab'"'
                  Return 1
               End
         End
      /*
       * Check if the build directory exists...
       */
      build_dir = FindNativeBuildDirectory( !global.!current_dirname.!!last_repository, lab )
      Select
         When continuecheckout \= 'CONTINUECHECKOUT' Then
            Do
               If DirectoryExists( build_dir ) = 0 Then
                  Do
                     Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The supplied version:' lab 'for module:' !global.!current_dirname.!!last_repository 'has not been checked out."'
                     Return 1
                  End
            End
         When DirectoryExists( build_dir ) = 0 & dontcheckout \= 'DONTCHECKOUT' & lab \= 'Latest' Then
            Do
               Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The supplied version:' lab 'for module:' !global.!current_dirname.!!last_repository 'has not been checked out and requested it not be checked out."'
               Return 1
            End
         When DirectoryExists( build_dir ) = 1 & dontcheckout = 'DONTCHECKOUT' & overwritebuild = 'OVERWRITEBUILD' & lab \= 'Latest' Then
            Do
               Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The supplied version:' lab 'for module:' !global.!current_dirname.!!last_repository 'has already been built and you did not request it be overwritten."'
               Return 1
            End
         Otherwise Nop
      End
      /*
       * Check if another build for the same version is already running
       */
      lock_file = build_dir || !global.!ossep || '.build.lck'
      If Stream( lock_file, 'C', 'QUERY EXISTS' ) \= '' Then
         Do
            line = Linein( lock_file )
            Call Runner batch, outputfile, !global.!cmdecho '"ERROR: Another build is likely in progress since' line 'for module:' !global.!current_dirname.!!last_repository'."'
            Call Runner batch, outputfile, !global.!cmdecho '"ERROR: If not, delete' lock_file'"'
            Return 1
         End
      If dontcheckout = 'DONTCHECKOUT' Then
         Do
            /*
             * Export the directory tree
             * Due to an annoying feature of CVS, we can't export to a local
             * directory pointed to by the -d switch, so we have to check out
             * each file seperately to the correct directory
             */
            If lab = 'Latest' Then
               Do
                  /*
                   * Get a list of all files in the module with their revision
                   * numbers
                   */
                  Select
                     When !global.!!repository.!!type.!!last_repository = 'cvs' Then
                        Do
                           cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q list' quote( !global.!current_dirname.!!last_repository )
                           If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                           Address System cmd With Output Stem out. Error Stem err.
                           /* TODO don't use cvs list!!! */
                           Do i = 1 To out.0
                              Parse Var out.i . rev . . . . flen .
                              Parse Var out.i 92 fn +(flen) +1 .
                              out.i = rev fn
                           End
                        End
                     Otherwise Nop
                  End
               End
            Else
               Do
                  /*
                   * Read the label file from the repository into a stem
                   */
                  Select
                     When !global.!!repository.!!type.!!last_repository = 'cvs' Then
                        Do
                           cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository '-q checkout -p' quote( !global.!current_dirname.!!last_repository'/.label-'lab )
                           If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                           Address System cmd With Output Stem out. Error Stem junk.
                        End
                     Otherwise Nop
                  End
               End
            /*
             * Get rid any existing files and re-make the directory tree
             */
            If continuecheckout = 'CONTINUECHECKOUT' Then
               Do
                  If overwritebuild \= 'OVERWRITEBUILD' Then Call Runner batch, outputfile, !global.!cmdecho '">>>>Cleaning up previous build"'
                  Call Runner batch, outputfile, !global.!cmddelr quote( build_dir )
                  Call Runner batch, outputfile, !global.!cmdmkdir quote( build_dir )
                  Call Runner batch, outputfile, !global.!cmdecho '">>>>Checking out all files for version' lab 'of' !global.!current_dirname.!!last_repository 'to' build_dir'"'
               End
            Else
               Do
                  Call Runner batch, outputfile, !global.!cmdecho '">>>>Continuing checking out all files for version' lab 'of' !global.!current_dirname.!!last_repository 'to' build_dir'"'
               End
            /*
             * Write the lock file
             */
            Call Stream lock_file, 'C', 'OPEN WRITE REPLACE'
            Call Lineout lock_file, 'Build started at:' Date() Time()
            Call Stream lock_file, 'C', 'CLOSE'
            !global.!build_lock_file = lock_file
            dirlen = Length( !global.!current_dirname.!!last_repository )
            /*
             * If the OS can handle symlinks, then check if the common dirs are in the out. stem
             * and check these out first. Then we can create local symlinks and not have to checkout
             * all the linked files.
             * We then ignore the files we checked out previously
             */
             ignore. = 0
             If !global.!symlink \= '' Then
                Do
                   Do j = 1 To !global.!common_dir.0
                      dir = Substr( !global.!common_dir.j, 2 ) || '/'
                      len = Length( dir )
                      Do i = 1 To out.0
                         If Strip( out.i) = '' | Left( out.i, 1 ) = ';' Then Iterate i
                         Parse Var out.i rev fn
                         If Left( fn, len ) = dir Then
                            Do
                               have_common_dir.j = 1
                               ignore.i = 1
                               Call CheckoutAFile rev, fn, build_dir, dirlen, batch, outputfile, continuecheckout
                            End
                      End
                   End
                   /*
                    * Now for all files check if they are in the linked directories
                    * that we have checked out above and set their ignore flag
                    */
                   Do i = 1 To out.0
                      If Strip( out.i) = '' | Left( out.i, 1 ) = ';' Then Iterate i
                      Parse Var out.i rev fn
                      Do j = 1 To !global.!linked_dir.0
                         ld = Substr( !global.!linked_dir.j, 2 ) || '/'
                         len = Length( ld )
                         If ld = Left( fn, len ) Then
                            Do
                               idx = !global.!linked_idx.j
                               If have_common_dir.idx = 1 Then ignore.i = 1
                            End
                      End
                   End
                End
            /*
             * We now have a stem; out. with all files and their revision numbers
             */
            Do i = 1 To out.0
               If Strip( out.i) = '' | Left( out.i, 1 ) = ';' Then Iterate i
               /*
                * If we have our ignore flag set, ignore the entry
                */
               If ignore.i = 1 Then Iterate i
               Parse Var out.i rev fn
               rc = CheckoutAFile( rev, fn, build_dir, dirlen, batch, outputfile, continuecheckout )
               If rc \= 0 Then
                  Do
                     /*
                      * Delete the lock file
                      */
                     Call Runner batch, outputfile, !global.!cmddelr quote( lock_file )
                     Drop !global.!build_lock_file
                     Return rc
                  End
            End
            If !global.!symlink \= '' Then
               Do
                  tmpdir = Directory()
                  /*
                   * We now need to go through the linked directories and setup all of the
                   * symbolic links to the common dirs
                   */
                  Do j = 1 To !global.!linked_dir.0
                     common_idx = !global.!linked_idx.j
                     If have_common_dir.idx = 1 Then
                        Do
                           Call Directory tmpdir
                           /*
                            * Work out how to get the correct relative path for linking
                            */
                           pos = Lastpos( '/', !global.!linked_dir.j )
                           linked_dirname = Substr( !global.!linked_dir.j, 1, pos - 1 ) /* eg /test/dir1/dir2 */
                           common_dirname = !global.!common_dir.common_idx /* /common */
                           mycomm = common_dirname
                           newdir = linked_dirname
                           Do i = 1 To Min( Length( mycomm ), Length( newdir ) )
                              If Substr( mycomm, i, 1 ) = '/' Then last_dir_pos = i
                              If Substr( mycomm, i, 1 ) \= Substr( newdir, i, 1 ) Then Leave
                           End
                           rel_linked = Substr( newdir, last_dir_pos + 1 )
                           rel_common = Substr( mycomm, last_dir_pos + 1 )
                           /*
                            * Now symlink the common dir to the linked dirname
                            * eg. Linking /tmfps/common to /tmfps/app/common
                            *     cd /tmfps/app
                            *     ln -sf ../common .
                            */
                           Call Runner batch, outputfile, !global.!cmdecho '"Linking directory' quote( build_dir'/'rel_common ) 'to current directory:' quote( build_dir'/'rel_linked )'"'
                           /*
                            * Change directory to linked dir
                            */
                           Call Directory build_dir'/'rel_linked
                           Address Command !global.!symlink quote( '../'rel_common ) .
                         End
                  End
                  Call Directory tmpdir
               End
         End
      /*
       * Change to the working directory to update build number ...
       */
      Call Directory !global.!!repository.!!working_dir.!!last_repository
      /*
       * Increment the build number
       */
      Call Runner batch, outputfile, !global.!cmdecho '">>>>Getting current build number."'
      bnf = 'admin/build.no'
      Select
         When !global.!!repository.!!type.!!last_repository = 'rcs' Then
            Do
            End
         When !global.!!repository.!!type.!!last_repository = 'cvs' Then
            Do
               retcode = Runner( batch, outputfile, 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout' bnf )
            End
         Otherwise Nop
      End
      If retcode \= 0 Then build_no = 1
      Else
         Do
            Call Stream bnf, 'C', 'OPEN READ'
            build_no = Linein( bnf )
            build_no = build_no + 1
            Call Stream bnf, 'C', 'CLOSE'
         End
      Call Runner batch, outputfile, !global.!cmdecho '">>>>Incrementing build number to:' build_no'"'
      /*
       * Check build number back in with history
       */
      Call Stream bnf, 'C', 'OPEN WRITE REPLACE'
      Call Lineout bnf, build_no
      Call Stream bnf, 'C', 'CLOSE'
      msg = 'Build' build_no 'for version' lab 'of' !global.!current_dirname.!!last_repository
      Select
         When !global.!!repository.!!type.!!last_repository = 'rcs' Then
            Do
            End
         When !global.!!repository.!!type.!!last_repository = 'cvs' Then
            Do
               If build_no = 1 Then
                  Do
                     cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'add' bnf
                     If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
                     Address System cmd
                  End
               Call Runner batch, outputfile, 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'commit -m"'msg'"' bnf
            End
         Otherwise Nop
      End
      /*
       * If called from batch, create our description file so we can see
       * the job in the GUI QOCCA and use "tee" so the output goes to the ".out" file
       * for later viewing
       */
      If batch Then
         Do
            desc = 'Build ('build_no') of version' quote( lab ) 'of' quote( !global.!current_dirname.!!last_repository )
            type = 'build'
            compiler_type = FindCompilerType( !global.!current_dirname.!!last_repository )
            Call CreateDescriptionFile _basefile, desc, type, compiler_type
         End
      /*
       * Change to the build directory (top level for the module)...
       * and then to the "make" directory
       */
      Call Directory build_dir
      make_dir = FindMakeDirectory( !global.!current_dirname.!!last_repository )
      If make_dir \= '' Then Call Directory make_dir
      /*
       * Should be able to build now!
       */
      /*
       * Set the BUILD and VERSION environment variables here
       */
      Call Value !global.!buildenvvar, build_no, 'ENVIRONMENT'
      Call Value !global.!versionenvvar, lab, 'ENVIRONMENT'
      /*
       * Split the "build" command into multiple, and run them...
       */
      Do Forever
         Parse Var buildcommand mc ';' buildcommand
         mc = Strip( mc )
         If mc = '' Then Leave
         Call Runner batch, outputfile, !global.!cmdecho '">>>>Executing command:' mc'"'
         retcode = Runner( batch, outputfile, mc )
         If retcode \= 0 Then
            Do
               Call Runner batch, outputfile, !global.!cmdecho '"Failed!"'
               /*
                * Delete the lock file
                */
               Call Runner batch, outputfile, !global.!cmddelr quote( lock_file )
               Drop !global.!build_lock_file
               Return retcode
            End
      End
      et = Time( 'E' )
      Parse Var et et '.' .
      hr = et % 3600
      et = et // 3600
      mi = et % 60
      se = et // 60
      Call Runner batch, outputfile, !global.!cmdecho '">>>>Completed successfully at' Date() Time() 'Elapsed:' Right( hr, 2, 0 )':'Right( mi, 2, 0 )':'Right( se, 2, 0)'"'
      Call Value !global.!buildenvvar, '', 'ENVIRONMENT'
      Call Value !global.!versionenvvar, '', 'ENVIRONMENT'
   End
/*
 * Delete the lock file
 */
Call Runner batch, outputfile, !global.!cmddelr quote( lock_file )
Drop !global.!build_lock_file
Return retcode

CheckoutAFile: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg rev, fn, build_dir, dirlen, batch, outputfile, continuecheckout
outfile = build_dir||Substr( fn, dirlen+1 )
/*
 * If we are continuing our checkout then return here if the file already exists
 */
If continuecheckout \= 'CONTINUECHECKOUT' & Stream( outfile, 'C', 'QUERY EXISTS' ) \= '' Then Return 0
/*
 * Need to determine if the directory of the file exists;
 * if not make it before checking the file out
 */
outpos = Lastpos( '/', outfile )
outdir = Substr( outfile, 1, outpos-1 )
If DirectoryExists( outdir ) = 0 Then Call Runner batch, outputfile, !global.!cmdecho '"Checking out files for directory:' outdir'"'
Call CreateDirectory( outdir )
/*
 * We don't need to checkout label files
 */
If Left( Substr( outfile, outpos ), 8 ) = '/.label-' Then Return 0
/*
 * Get our file
 */
Select
   When !global.!!repository.!!type.!!last_repository = 'cvs' Then
      Do
         /*
          * Need to run cvs checkout with -p flag to avoid cvs stickiness
          * Problem is though that file permissions are lost
          */
         cmd = 'cvs' !global.!!repository.!!cvs_compression.!!last_repository '-d'!global.!!repository.!!path.!!last_repository 'checkout -p -r'rev quote( fn )
         If !global.!!user.!!preference.!!debug_commands = 1 Then Call DebugCommands cmd
         Address Command cmd With Output Stream outfile Error Stem err.
         If rc \= 0 Then
            Do
               Call Runner batch, outputfile, !global.!cmdecho '"Error getting revision' rev 'of' fn 'to' outfile'"'
               Do i = 1 To err.0
                  Call Runner batch, outputfile, !global.!cmdecho err.i
               End
--               Return rc
            End
         Call Stream outfile, 'C', 'CLOSE'
         If !global.!cmdrename = 'mv' Then
            Do
               -- fix permissions for some files on Unixy platforms
               files = 'config.guess config.sub configure install-sh'
               Do i = 1 To Words( files )
                  file = Word( files, i )
                  len = Length( file )
                  If Right( outfile, len ) = file Then Address System 'chmod u+x' outfile
               End
            End
      End
   Otherwise Nop
End
Return 0

RunPromote: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg repository, lab, _basefile, batch
outputfile = _basefile'.out'
retcode = 0
/*
 * Check that the supplied module is valid and "promotable"
 */
promotecommand = FindPromoteCommand( !global.!current_dirname.!!last_repository )
If promotecommand = '' | Left( promotecommand, 1 ) = '!' Then
   Do
      Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The supplied module:' !global.!current_dirname.!!last_repository 'is not valid or promotable."'
   End
/*
 * If env supplied it must be in !global.!runtime_envs
If !global.!runtime_envs = '' | Left( !global.!runtime_envs, 1 ) = '!' Then Nop
Else
   Do
      If Wordpos( env, !global.!runtime_envs ) = 0 Then
         Do
            Call AbortText 'The supplied environment:' env 'is not a valid. Must be one of:' !global.!runtime_envs
         End
   End
 */
/*
 * Check if the build directory exists...
 */
build_dir = FindNativeBuildDirectory( !global.!current_dirname.!!last_repository, lab )
If DirectoryExists( build_dir ) = 0 Then
   Do
      Call Runner batch, outputfile, !global.!cmdecho '"ERROR: The supplied version:' lab 'for module' !global.!current_dirname.!!last_repository 'has not been built."'
   End
/*
 * Change to promote directory
 */
build_dir = FindNativeBuildDirectory( !global.!current_dirname.!!last_repository, lab )
Call Directory build_dir
promote_dir = FindPromoteDirectory( !global.!current_dirname.!!last_repository )
If promote_dir \= '' Then Call Directory promote_dir
/*
 * Should be able to promote now!
 */
Call Runner batch, outputfile, !global.!cmdecho '">>>>Changing to promote directory:' promote_dir'"'
/*
 * Get the !install_script, copy to build_dir - TODO
 */
this_dir = DirToVariable( !global.!current_dirname.!!last_repository )
pos = Wordpos( !global.!os, !global.!platforms )
/*
 * Make our substitutions now...
 */
promotecommand = Changestr( '%D%', promotecommand, !global.!dir.!installdirs.this_dir.pos )
promotecommand = Changestr( '%V%', promotecommand, lab )
/*
 * Set the BUILD and VERSION environment variables here
 */
Call Value !global.!buildenvvar, build_no, 'ENVIRONMENT'
Call Value !global.!versionenvvar, lab, 'ENVIRONMENT'
/*
 * Split the "promote" command into multiple, and run them...
 */
Do Forever
   Parse Var promotecommand mc ';' promotecommand
   mc = Strip( mc )
   If mc = '' Then Leave
   Call Runner batch, outputfile, !global.!cmdecho '">>>>Executing command:' mc'"'
   retcode = Runner( batch, outputfile, mc )
   If retcode \= 0 Then
      Do
         Call Runner batch, outputfile, !global.!cmdecho '"Failed!"'
         Return retcode
      End
End
Call Runner batch, outputfile, !global.!cmdecho '">>>>Completed successfully at' Date() Time()'"'
Call Value !global.!buildenvvar, '', 'ENVIRONMENT'
Call Value !global.!versionenvvar, '', 'ENVIRONMENT'
Return retcode

/*
 * Turn mouse cursor to hourglass
 */
SetCursorWait: Procedure Expose !REXXDW. !global. !!last_repository
Call dw_window_set_pointer !global.!mainwindow, !REXXDW.!DW_POINTER_CLOCK
 Call dw_main_sleep 10
Return

/*
 * Turn mouse cursor to arrow
 */
SetCursorNoWait: Procedure Expose !REXXDW. !global. !!last_repository
Call dw_window_set_pointer !global.!mainwindow, !REXXDW.!DW_POINTER_ARROW
 Call dw_main_sleep 10
Return

Usage: Procedure Expose !REXXDW. !global. !!last_repository
Parse Arg cmd
Say 'Usage:'
Say 'qocca ['Translate( !global.!validcmds, '|', ' ' ) '[options] ]'
If cmd = '' Then validcmds = !global.!validcmds
Else validcmds = cmd
Do i = 1 To Words( validcmds )
   Say '-- options for' Word( validcmds, i )':'
   cmd = Translate( Word( validcmds, i ) )
   Do j = 1 To Words( !global.!validargs.cmd )
      Select
         When Word( !global.!mandatory.cmd, j ) = 0 & Word( !global.!valuereq.cmd, j ) = 1 Then str = '      ' Left( '['Word( !global.!validargs.cmd, j )||Word( !global.!opt.cmd, j )']', 20 ) '->' !global.!desc.cmd.j
         When Word( !global.!mandatory.cmd, j ) = 0 & Word( !global.!valuereq.cmd, j ) = 0 Then str = '      ' Left( '['Word( !global.!validargs.cmd, j )']', 20 ) '->' !global.!desc.cmd.j
         When Word( !global.!mandatory.cmd, j ) = 1 & Word( !global.!valuereq.cmd, j ) = 1 Then str = '      ' Left( Word( !global.!validargs.cmd, j )||Word( !global.!opt.cmd, j ), 20 ) '->' !global.!desc.cmd.j
         When Word( !global.!mandatory.cmd, j ) = 1 & Word( !global.!valuereq.cmd, j ) = 0 Then str = '      ' Left( Word( !global.!validargs.cmd, j ), 20 ) '->' !global.!desc.cmd.j
         Otherwise Nop
      End
      Say str
   End
End
Exit 1

Halt:
If Left( !global.!build_lock_file, 1 ) \= '!' Then Address System !global.!cmddelr quote( !global.!build_lock_file )
Say
Say 'User cancelled process!'
Exit 1
