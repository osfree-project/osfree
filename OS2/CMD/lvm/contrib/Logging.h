/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Module: Logging.h
 */

/*
 * Change History:
 *
 */

/*
 * Functions: The following functions are actually declared in
 *            LVM_INTERFACE.H but are implemented in Logging.C:
 *
 *              Start_Logging
 *              Stop_Logging
 *
 *
 * Description: Since the LVM Engine Interface is very
 *              large, it became undesirable to put all of the code
 *              to implement it in a single file.  Instead, the
 *              implementation of the LVM Engine Interface was divided
 *              among several C files which would have access to a
 *              core set of data.  Engine.H and Engine.C define,
 *              allocate, and initialize that core set of data.
 *              Logging.H and Logging.C implement the logging
 *              functions described in the LVM Engine Interface.
 *
 *
 *    Partition_Manager.C                                     Volume_Manager.C
 *            \                                               /
 *              \  |-------------------------------------|  /
 *                \|                                     |/
 *                 |        Engine.H and Engine.C        |
 *                /|                                     |\
 *              /  |-------------------------------------|  \
 *            /                  |                            \
 *    BootManager.C          Logging.H                       Handle_Manager.C
 *
 * Notes: This module is used to maintain a copy of the original partitioning
 *        information for inclusion in a log file if logging is active.  It
 *        can also be used to restore the original configuration.  If a
 *        Commit_Changes operation is performed, and it completes successfully,
 *        then the new configuration will be saved as the original configuration
 *        by this module.
 *
 */

#ifndef MANAGE_LOGGING

#define MANAGE_LOGGING

#include "gbltypes.h"  /* CARDINAL32 */
#include <stdio.h>     /* sprintf */

/*********************************************************************/
/*                                                                   */
/*   Function Name:  Log_Current_Configuration                       */
/*                                                                   */
/*   Descriptive Name: This function writes to the log file all of   */
/*                     the information that LVM has stored for each  */
/*                     drive, volume, and partition it knows about.  */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects: If disk space is available, then the current      */
/*                 information about drives, volumes, and partitions */
/*                 will be appended to the current contents of the   */
/*                 log file.                                         */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Log_Current_Configuration( void );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Write_Log_Buffer                                 */
/*                                                                   */
/*   Descriptive Name: Appends the contents of the log buffer to the */
/*                     log file.                                     */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects: If disk space is available, then the contents of  */
/*                 the log buffer are appended to the log file.      */
/*                                                                   */
/*   Notes:  This function assumes that the contents of the log      */
/*           buffer is a single, null terminated string.  This       */
/*           function will add a CF/LF to the end of the string.     */
/*                                                                   */
/*********************************************************************/
void _System Write_Log_Buffer( void );


#define LOG_BUFFER_SIZE   512


/*--------------------------------------------------
 * Macros
 --------------------------------------------------*/

#define LOG_EVENT( Event_Text )                         \
	if ( Logging_Enabled )                          \
        {                                               \
                                                        \
           sprintf(Log_Buffer, "     %s", Event_Text);  \
           Write_Log_Buffer();                          \
                                                        \
        }                                               \


#define LOG_EVENT1( Event_Text, Event_Code1_Text, Event_Code1 )  \
		if ( Logging_Enabled )                           \
                {                                                \
                                                                 \
                 sprintf(Log_Buffer,                             \
                         "     %s\n       %s = %x",              \
                         Event_Text,                             \
                         Event_Code1_Text,                       \
                         (CARDINAL32) Event_Code1);              \
                 Write_Log_Buffer();                             \
                                                                 \
               }                                                 \


#define LOG_EVENT2( Event_Text, Event_Code1_Text, Event_Code1, Event_Code2_Text, Event_Code2 )                       \
                                                            if (  Logging_Enabled )                                  \
                                                            {                                                        \
                                                                                                                     \
                                                              sprintf(Log_Buffer,                                    \
                                                                      "     %s\n       %s = %x\n       %s = %x",     \
                                                                      Event_Text,                                    \
                                                                      Event_Code1_Text,                              \
                                                                      (CARDINAL32) Event_Code1,                      \
                                                                      Event_Code2_Text,                              \
                                                                      (CARDINAL32) Event_Code2);                     \
                                                              Write_Log_Buffer();                                    \
                                                                                                                     \
                                                            }                                                        \


#define LOG_EVENT3( Event_Text, Event_Code1_Text, Event_Code1, Event_Code2_Text, Event_Code2, Event_Code3_Text, Event_Code3 )       \
                                                           if (  Logging_Enabled )                                                  \
                                                           {                                                                        \
                                                                                                                                    \
                                                             sprintf(Log_Buffer,                                                    \
                                                                     "     %s\n       %s = %x\n       %s = %x\n       %s = %x",     \
                                                                     Event_Text,                                                    \
                                                                     Event_Code1_Text,                                              \
                                                                     (CARDINAL32) Event_Code1,                                      \
                                                                     Event_Code2_Text,                                              \
                                                                     (CARDINAL32 ) Event_Code2,                                     \
                                                                     Event_Code3_Text,                                              \
                                                                     (CARDINAL32) Event_Code3);                                     \
                                                             Write_Log_Buffer();                                                    \
                                                                                                                                    \
                                                           }                                                                        \


#define LOG_ERROR( Error_Text )  if (  Logging_Enabled )                                             \
                                 {                                                                   \
                                                                                                     \
                                   sprintf( Log_Buffer,ERROR_BORDER);                                \
                                   Write_Log_Buffer();                                               \
                                   sprintf( Log_Buffer, "     %s", Error_Text);                      \
                                   Write_Log_Buffer();                                               \
                                   sprintf( Log_Buffer,ERROR_BORDER);                                \
                                   Write_Log_Buffer();                                               \
                                                                                                     \
                                 }                                                                   \

#define LOG_ERROR1( Error_Text, Error1_Text, Error_Code )  if ( Logging_Enabled )                            \
                                                           {                                                 \
                                                                                                             \
                                                             sprintf(Log_Buffer,                             \
                                                                     "     %s\n       %s = %x",              \
                                                                     Error_Text,                             \
                                                                     Error1_Text,                            \
                                                                     (CARDINAL32) Error_Code);               \
                                                             Write_Log_Buffer();                             \
                                                                                                             \
                                                           }                                                 \

#define LOG_ERROR2( Error_Text, Error_Code1_Text, Error_Code1, Error_Code2_Text, Error_Code2 )                       \
                                                            if (  Logging_Enabled )                                  \
                                                            {                                                        \
                                                                                                                     \
                                                              sprintf(Log_Buffer,                                    \
                                                                      "     %s\n       %s = %x\n       %s = %x",     \
                                                                      Error_Text,                                    \
                                                                      Error_Code1_Text,                              \
                                                                      (CARDINAL32) Error_Code1,                      \
                                                                      Error_Code2_Text,                              \
                                                                      (CARDINAL32) Error_Code2);                     \
                                                              Write_Log_Buffer();                                    \
                                                                                                                     \
                                                            }                                                        \

#define FUNCTION_ENTRY( FunctionName )  if (  Logging_Enabled >1)                                           \
                                        {                                                                   \
                                                                                                            \
                                          sprintf( Log_Buffer,FUNCTION_ENTRY_BORDER);                       \
                                          Write_Log_Buffer();                                               \
                                          sprintf( Log_Buffer, "     %s", FunctionName);                    \
                                          Write_Log_Buffer();                                               \
                                          sprintf( Log_Buffer,FUNCTION_ENTRY_BORDER);                       \
                                          Write_Log_Buffer();                                               \
                                                                                                            \
                                        }                                                                   \

#define FUNCTION_EXIT( FunctionName )  if (  Logging_Enabled >1)                                           \
                                       {                                                                   \
                                                                                                           \
                                         sprintf(Log_Buffer,FUNCTION_EXIT_BORDER);                         \
                                         Write_Log_Buffer();                                               \
                                         sprintf(Log_Buffer, "     %s", FunctionName);                     \
                                         Write_Log_Buffer();                                               \
                                         sprintf(Log_Buffer,FUNCTION_EXIT_BORDER);                         \
                                         Write_Log_Buffer();                                               \
                                                                                                           \
                                       }                                                                   \

#define API_ENTRY( FunctionName )  if (  Logging_Enabled > 1)                                          \
                                   {                                                                   \
                                                                                                       \
                                     sprintf( Log_Buffer,API_ENTRY_BORDER);                            \
                                     Write_Log_Buffer();                                               \
                                     sprintf( Log_Buffer, "     %s", FunctionName);                    \
                                     Write_Log_Buffer();                                               \
                                     sprintf( Log_Buffer,API_ENTRY_BORDER);                            \
                                     Write_Log_Buffer();                                               \
                                                                                                       \
                                   }                                                                   \

#define API_EXIT( FunctionName )  if (  Logging_Enabled  > 1)                                         \
                                  {                                                                   \
                                                                                                      \
                                    sprintf( Log_Buffer,API_EXIT_BORDER);                             \
                                    Write_Log_Buffer();                                               \
                                    sprintf( Log_Buffer, "     %s", FunctionName);                    \
                                    Write_Log_Buffer();                                               \
                                    sprintf( Log_Buffer,API_EXIT_BORDER);                             \
                                    Write_Log_Buffer();                                               \
                                                                                                      \
                                  }                                                                   \



#ifdef DECLARE_LOGGING_GLOBALS

char       Log_Buffer[LOG_BUFFER_SIZE];   /* Used to hold messages to be written to the log file. */
int        Logging_Enabled = 0;           /* If 0 = No Logging, 1= partial, 2- full, he log file is open and logging is active. */

#else

extern char        Log_Buffer[LOG_BUFFER_SIZE];     /* Used to hold messages to be written to the log file. */
extern BOOLEAN     Logging_Enabled;                 /* If TRUE, then the log file is open and logging is active. */

#endif



#endif
