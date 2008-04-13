/***********************************************************************/
/* SINGLE.C - Processing for single input mode                         */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

static char RCSid[] = "$Id: single.c,v 1.3 2002/06/23 04:14:16 mark Exp $";

#include <the.h>
#include <proto.h>

#ifdef THE_SINGLE_INSTANCE_ENABLED

#ifdef WIN32
#include <windows.h>

#define MY_PIPE_CONNECT 0
#define MY_PIPE_READ    1
HANDLE hSemCommandCount, hPipeRead, hThread;
int pipe_state;
BOOL fPendingIO;

/***********************************************************************/
LONG PipeHandlerThread( LPVOID lpThreadData )
/***********************************************************************/
{
   LONG prev,err;
   char LoadError[256];
   HANDLE hPipe = (HANDLE)fifo_fd;
   bool first=TRUE;

   TRACE_FUNCTION("single.c:  PipeHandlerThread");

   for ( ; ; )
   {
      if ( first = FALSE )
      {
         /*
          * Not the first time through, so wait until the parent thread
          * indicates we have processed all the input from the other
          * process. Wait for the event to be signalled and then
          * disconnect us from the named pipe.
          */
         WaitForSingleObject( hPipeRead, INFINITE );
         DisconnectNamedPipe( hPipe );
      }
      first = FALSE;
      if ( ConnectNamedPipe( hPipe, NULL ) )
      {
         /*
          * We have a client connected, tell the parent thread to read the
          * named pipe.
          */
         SetEvent( hPipeRead );
         if ( ReleaseSemaphore( hSemCommandCount, 1, &prev ) == FALSE )
         {
            display_error( 0, (CHARTYPE *)"Incrementing semaphore failed.", FALSE );
            FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
            display_error( 0, LoadError, FALSE );
         }
      }
      else
      {
         /*
          * An error occured. is it that the client disconnected ?
          */
         err = GetLastError();
         switch( err )
         {
            case ERROR_PIPE_CONNECTED:
               break;
            case ERROR_NO_DATA:
               /*
                * Client has closed their connection...disconnect our end
                */
               DisconnectNamedPipe( hPipe );
               break;
            default:
               sprintf( LoadError, "Error Number: %d", err );
               display_error( 0, (CHARTYPE *)LoadError, FALSE );
               FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
               display_error( 0, LoadError, FALSE );
               break;
         }
      }
   }
   TRACE_RETURN();
   return 0;
}

/***********************************************************************/
int initialise_fifo( _LINE *first_file_name, LINETYPE startup_line, LENGTHTYPE startup_column, bool ro )
/***********************************************************************/
{
   _LINE *current_file_name;
   char buf[MAX_FILE_NAME+1];
   char LoadError[256];
   char *ronly;
   HANDLE hPipe;
   DWORD cbWritten,dwThreadID;
   int am_client = 0, rc;
   BOOL fSuccess;

   TRACE_FUNCTION("single.c:  initialise_fifo");

   sprintf( buf, "\\\\.\\pipe\\%s", fifo_name );

   /*
    * Try and open the existing pipe...
    */
   hPipe = CreateFile( buf,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
   if ( hPipe != INVALID_HANDLE_VALUE )
   {
      /*
       * The pipe exists; we are the client
       */
      fifo_fd = (unsigned long)hPipe;

      /*
       * Must determine how many files are being edited. This gets sent to
       * the server to know when to release the client details
       */
      current_file_name = first_file_name;
      trec_len = 0;
      while( current_file_name != NULL )
      {
         if ( ( rc = splitpath( current_file_name->line ) ) == RC_OK )
         {
            trec_len++;
         }
         current_file_name = current_file_name->next;
      }
      if ( !WriteFile( hPipe, &trec_len, sizeof( trec_len ), &cbWritten, NULL ) )
      {
         FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
         display_error( 0, LoadError, FALSE );
         TRACE_RETURN();
         return 1;
      }
      current_file_name = first_file_name;
      while( current_file_name != NULL )
      {
         if ( ( rc = splitpath( current_file_name->line ) ) != RC_OK )
         {
            display_error( 10, current_file_name->line, FALSE );
         }
         else
         {
            /*
             * If started with -r, set READONLY FORCE
             */
            if ( ro )
               ronly = "#readonly force";
            else
               ronly = "";
            /*
             * If line and/or column specified on command line, use
             * them to reposition file...
             */
            if ( startup_line != 0L
            ||   startup_column != 0 )
            {
               trec_len = sprintf( trec, "x %s%s#cursor goto %ld %d%s",
                                   sp_path, sp_fname,
                                   (startup_line) ? startup_line : 1,
                                   (startup_column) ? startup_column : 1,
                                   ronly );
            }
            else
            {
               trec_len = sprintf( trec, "x %s%s%s", sp_path, sp_fname, ronly );
            }
            /*
             * Write command length...
             */
            if ( !WriteFile( hPipe, &trec_len, sizeof( trec_len ), &cbWritten, NULL ) )
            {
               FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
               display_error( 0, LoadError, FALSE );
               am_client = 1;
               break;
            }
            /*
             * Write command. Only 1 command allowed as server expects
             * a single command and then disconnetcs the client.
             */
            if ( !WriteFile( hPipe, trec, trec_len, &cbWritten, NULL ) )
            {
               FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
               display_error( 0, LoadError, FALSE );
               am_client = 1;
               break;
            }
         }
         current_file_name = current_file_name->next;
      }
      Sleep( 1000 );  /* ??? */
      CloseHandle( hPipe );
      /*
       * We are the client, so return with 1 to enable the caller to stop
       */
      am_client = 1;
   }
   else
   {
      /*
       * The pipe needs to be created here...
       */
      hPipe = CreateNamedPipe( buf,
                               PIPE_ACCESS_INBOUND,
                               PIPE_TYPE_BYTE | PIPE_WAIT,
                               1,
                               MAX_COMMAND_LENGTH,
                               MAX_COMMAND_LENGTH,
                               0,
                               NULL );
      if ( hPipe == INVALID_HANDLE_VALUE )
      {
         display_error( 0, (CHARTYPE *)"Warning: Unable to run in single instance mode:", FALSE );
         FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
         display_error( 0, LoadError, FALSE );
         am_client = 1;
      }
      else
      {
         fifo_fd = (unsigned long)hPipe;
         /*
          * Create a semaphore on which the parent thread will wait...
          */

         hPipeRead = CreateEvent( NULL, TRUE, FALSE, NULL );

         hSemCommandCount = CreateSemaphore( NULL, 0, 1024, NULL );
         if ( hSemCommandCount == NULL )
         {
            display_error( 0, (CHARTYPE *)"Warning: Unable to run in single instance mode:", FALSE );
            FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
            display_error( 0, LoadError, FALSE );
            am_client = 1;
         }
         else
         {
            hThread = CreateThread( NULL,              // security attributes
                                    0,                 // initial stack size
                                    (LPTHREAD_START_ROUTINE) PipeHandlerThread,
                                    NULL,              // argument
                                    CREATE_SUSPENDED,  // creation flag
                                    &dwThreadID );     // new thread ID
            if ( !hThread )
            {
               display_error( 0, (CHARTYPE *)"Warning: Unable to run in single instance mode:", FALSE );
               FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
               display_error( 0, LoadError, FALSE );
               am_client = 1;
            }
            else
            {
               ResumeThread( hThread );
               /*
                * We are the server, so return with 0 to enable the caller to continue...
                */
               single_instance_server = TRUE;
            }
         }
      }
   }
   TRACE_RETURN();
   return am_client;
}

/***********************************************************************/
int process_fifo_input( int key )
/***********************************************************************/
{
   int fd,num_files,i;
   HANDLE hEvents[2],hPipe;
   DWORD dwWait,cbRead;
   char LoadError[256];
   bool le_status=CURRENT_VIEW->linend_status;
   CHARTYPE le_value=CURRENT_VIEW->linend_value;
   VIEW_DETAILS *le_view;
   BOOL fSuccess;

   TRACE_FUNCTION("single.c:  process_fifo_input");

   hPipe = (HANDLE)fifo_fd;
   if ( key == -1 )
   {
      for ( ; ; )
      {
         /*
          * Add curses input and the input fifo
          */
         hEvents[0] = (HANDLE)PDC_get_input_fd();
         if ( hEvents[0] == 0 )
         {
            /*
             * We don't have a valid handle to wait on.
             */
            TRACE_RETURN();
            return key;
         }
         hEvents[1] = hSemCommandCount; /* semaphore associated with the named pipe */

         dwWait = WaitForMultipleObjects( 2,
                                          hEvents,
                                          FALSE,
                                          INFINITE);

         if ( dwWait == WAIT_FAILED )
         {
            FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LoadError, 256, NULL ) ;
            display_error( 0, LoadError, FALSE );
         }
         else
         {
            fd = dwWait - WAIT_OBJECT_0; /* determines which pipe */
            if ( fd == 0 )
            {
               /*
                * A key is ready...
                * We have to increment the semaphore, because we have
                * decremented it by waiting on it. Isn't this crap!
                */
               ReleaseSemaphore( hEvents[0], 1, NULL );
               TRACE_RETURN();
               return (-1);
            }
            else if ( fd == 1 )
            {
               /*
                * To get here we must be in a state to read from the pipe...
                * Read the number of files coming down the pipe...
                */
               fSuccess = ReadFile( hPipe, (LPVOID)&trec_len, sizeof(trec_len), &cbRead, NULL );
               if ( !fSuccess || cbRead == 0 )
               {
                  TRACE_RETURN();
                  return key;
               }
               num_files = (int)trec_len;
               /*
                * save view and LINEND details to reset later
                */
               le_status = CURRENT_VIEW->linend_status;
               le_value = CURRENT_VIEW->linend_value;
               le_view = CURRENT_VIEW;
               /*
                * Read each file from the pipe...
                */
               for ( i = 0; i < num_files; i++ )
               {
                  if ( !ReadFile( hPipe, (LPVOID)&trec_len, sizeof(trec_len), &cbRead, NULL ) )
                  {
                     TRACE_RETURN();
                     le_view->linend_status = le_status;
                     le_view->linend_value = le_value;
                     return key;
                  }
                  /*
                   * Sleep for 100 milliseconds to ensure the remainder of the
                   * data is in the fifo. Yuck!
                   */
                  napms( 100 );
                  if ( !ReadFile( hPipe, (LPVOID)trec, trec_len*sizeof(CHARTYPE), &cbRead, NULL ) )
                  {
                     TRACE_RETURN();
                     le_view->linend_status = le_status;
                     le_view->linend_value = le_value;
                     return key;
                  }
                  trec[trec_len] = '\0';
                  CURRENT_VIEW->linend_status = TRUE;
                  CURRENT_VIEW->linend_value = '#';
                  (void)command_line( trec, TRUE );
               }
               ResetEvent( hPipeRead );
               le_view->linend_status = le_status;
               le_view->linend_value = le_value;
               key = 0;
               break;
            }
         }
      }
   }
   TRACE_RETURN();
   return key;
}

/***********************************************************************/
void close_fifo( void )
/***********************************************************************/
{
   TRACE_FUNCTION("single.c:  close_fifo");
   TerminateThread( hThread, 0 );
   DisconnectNamedPipe( (HANDLE)fifo_fd );
   CloseHandle( (HANDLE)fifo_fd );
   CloseHandle( (HANDLE)hPipeRead );
   CloseHandle( (HANDLE)hSemCommandCount );
   TRACE_RETURN();
}

#elif defined(HAVE_SELECT) && defined(HAVE_MKFIFO)

/*
 * Most Unixen; determine where select() is
 */
# if defined(HAVE_SYS_SELECT_H)
#   include <sys/select.h>
# elif defined(HAVE_SYS_SOCKET_H)
#   include <sys/socket.h>
# elif defined(SELECT_IN_TIME_H)
#   include <time.h>
# endif

/***********************************************************************/
#ifdef HAVE_PROTO
int initialise_fifo( _LINE *first_file_name, LINETYPE startup_line, LENGTHTYPE startup_column, bool ro )
#else
int initialise_fifo( first_file_name, startup_line, startup_column, ro )
_LINE *first_file_name;
LINETYPE startup_line;
LENGTHTYPE startup_column;
bool ro;
#endif
/***********************************************************************/
{
   _LINE *current_file_name;
   int am_client = 0, rc;
   char *ronly;

   TRACE_FUNCTION("single.c:  initialise_fifo");

   if ( file_exists( fifo_name ) )
   {
      /*
       * The FIFO exists, so we assume we are the client here...
       */
      if ( file_writable( fifo_name ) )
      {
         fifo_fd = open( fifo_name, O_WRONLY );
         if ( fifo_fd == (-1) )
         {
            display_error( 0, (CHARTYPE *)"Warning: Unable to run in single instance mode:", FALSE );
            display_error( 0, strerror( errno ), FALSE );
         }
         else
         {
            current_file_name = first_file_name;
            while( current_file_name != NULL )
            {
               if ( ( rc = splitpath( current_file_name->line ) ) != RC_OK )
               {
                  display_error( 10, current_file_name->line, FALSE );
               }
               else
               {
                  /*
                   * If started with -r, set READONLY FORCE
                   */
                  if ( ro )
                     ronly = "#readonly force";
                  else
                     ronly = "";
                  /*
                   * If line and/or column specified on command line, use
                   * them to reposition file...
                   */
                  if ( startup_line != 0L
                  ||   startup_column != 0 )
                  {
                     trec_len = sprintf( trec, "x %s%s#cursor goto %ld %d%s",
                                   sp_path, sp_fname,
                                   (startup_line) ? startup_line : 1,
                                   (startup_column) ? startup_column : 1,
                                   ronly );
                  }
                  else
                  {
                     trec_len = sprintf( trec, "x %s%s%s", sp_path, sp_fname, ronly );
                  }
                  if ( write( fifo_fd, &trec_len, sizeof( trec_len ) ) == (-1) )
                  {
                     display_error( 0, strerror( errno ), FALSE );
                  }
                  if ( write( fifo_fd, trec, trec_len ) == (-1) )
                  {
                     display_error( 0, strerror( errno ), FALSE );
                  }
               }
               current_file_name = current_file_name->next;
            }
            close( fifo_fd );
            /*
             * We are the client, so return with 1 to enable the caller to stop
             */
            am_client = 1;
         }
      }
      else
      {
         display_error( 0, (CHARTYPE *)"Warning: Unable to run in single instance mode:", FALSE );
         display_error( 0, strerror( errno ), FALSE );
      }
   }
   else
   {
      /*
       * The FIFO doesn't exists, so we assume we are the server here...
       */
      if ( mkfifo( (DEFCHAR*)fifo_name, S_IWUSR | S_IRUSR ) == (-1) )
      {
         display_error( 0, (CHARTYPE *)"Warning: Unable to run in single instance mode:", FALSE );
         display_error( 0, strerror( errno ), FALSE );
      }
      else
      {
         fifo_fd = open( (DEFCHAR*)fifo_name, O_RDWR );
         if ( fifo_fd == -1 )
         {
            display_error( 0, (CHARTYPE *)"Warning: Unable to run in single instance mode:", FALSE );
            display_error( 0, strerror( errno ), FALSE );
         }
         else
         {
            single_instance_server = TRUE;
         }
      }
      /*
       * We are the server, so return with 0 to enable the caller to continue...
       */
   }
   TRACE_RETURN();
   return am_client;
}

/***********************************************************************/
#ifdef HAVE_PROTO
int process_fifo_input( int key )
#else
int process_fifo_input( key )
int key;
#endif
/***********************************************************************/
{
   int s;
   fd_set readfds;
   int curses_fd;
   bool le_status=CURRENT_VIEW->linend_status;
   CHARTYPE le_value=CURRENT_VIEW->linend_value;
   VIEW_DETAILS *le_view;

   TRACE_FUNCTION("single.c:  process_fifo_input");

   if ( key == -1 )
   {
      /*
       *
       * Add curses input and the input fifo
       */
#ifdef XCURSES
      curses_fd = PDC_get_input_fd();
#else
      curses_fd = fileno( stdin );
#endif
      FD_ZERO ( &readfds );
      FD_SET ( curses_fd, &readfds );
      FD_SET ( fifo_fd, &readfds );
      if ( ( s = select ( FD_SETSIZE, &readfds, NULL, NULL, NULL ) ) < 0 )
      {
         TRACE_RETURN();
         return key;
      }
      if ( s == 0 ) /* no requests pending - should never happen!*/
      {
         TRACE_RETURN();
         return key;
      }
      if ( FD_ISSET ( fifo_fd, &readfds ) )
      {
         if ( read( fifo_fd, (char *)&trec_len, sizeof(trec_len) ) < 0 )
         {
            TRACE_RETURN();
            return key;
         }
         /*
          * Sleep for 100 milliseconds to ensure the remainder of the
          * data is in the fifo. Yuck!
          */
         napms( 100 );
         if ( read( fifo_fd, trec, trec_len*sizeof(CHARTYPE) ) < 0 )
         {
            TRACE_RETURN();
            return key;
         }
         /*
          * Save the current linend.
          * Force LINEND to # (which is what initialise_lifo() sets).
          * Run the command
          * set LINEND back to default.
          */
         trec[trec_len] = '\0';
         le_status = CURRENT_VIEW->linend_status;
         le_value = CURRENT_VIEW->linend_value;
         le_view = CURRENT_VIEW;
         CURRENT_VIEW->linend_status = TRUE;
         CURRENT_VIEW->linend_value = '#';
         (void)command_line( trec, TRUE );
         le_view->linend_status = le_status;
         le_view->linend_value = le_value;
         key = 0;
      }
   }
   TRACE_RETURN();
   return key;
}

/***********************************************************************/
#ifdef HAVE_PROTO
void close_fifo( void )
#else
void close_fifo( )
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("single.c:  close_fifo");
   close( fifo_fd );
   remove_file( fifo_name );
   TRACE_RETURN();
}
#endif

#endif
