/*!
   @file ansi.c

   @brief ansi command - Frontend for ansi.sys. Allows user to turn ANSI
                         processing on or off.

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author JMA (jma@jma.se)

   @todo /? parameter
*/

#define INCL_DOSERRORS
#define INCL_VIO

#include <osfree.h>             // Include file for osfree
#include <cmd_shared.h>         // Include file for cmd tools
#include <string.h>

//  Constant declarations
#define HVIO            0

//**************************************************************************
// Start of program
//**************************************************************************
int main (int argc, char* argv[], char* envp[])
{
  APIRET             rc;
  USHORT             usAnsiState;


  // Check for parameter (ON or OFF)
  switch (argc)
         {
         case 1:
              // No parameter, just print ANSI state and exit
              if (( rc = VioGetAnsi(&usAnsiState, HVIO)) == NO_ERROR )
                 {
                 // Display message
                 if (usAnsiState == 1)
                    cmd_ShowSystemMessage(cmd_MSG_ANSI_EXT_SCR_KEY_ON, 0);
                 else
                    cmd_ShowSystemMessage(cmd_MSG_ANSI_EXT_SCR_KEY_OFF, 0);
                 }
              else
                 {
                 // VIO call failed, display error msg
                 cmd_ShowSystemMessage(cmd_MSG_ANSI_EXT_SCR_KEY_OFF, 0);
                 }
              break;

         case 2:
              // One parameter, should be on or off

              // Assume there is an error
              usAnsiState = 2;

              // Look for "ON" or "OFF"
              if ( !(strcmpi(argv[1], "ON")) )
                 usAnsiState = 1;
              else
                 if ( !(strcmpi(argv[1], "OFF")) )
                    usAnsiState = 0;

              if (usAnsiState < 2)
                 {
                 // Set ANSI state
                 if ( (rc = VioSetAnsi(usAnsiState, HVIO)) == NO_ERROR )
                    {
                    // Display message
                    if (usAnsiState == 1)
                       cmd_ShowSystemMessage(cmd_MSG_ANSI_EXT_SCR_KEY_ON, 0);
                    else
                       cmd_ShowSystemMessage(cmd_MSG_ANSI_EXT_SCR_KEY_OFF, 0);
                       }
                 else
                    {
                    // VIO call failed, display error msg
                    cmd_ShowSystemMessage(cmd_MSG_ANSI_EXT_SCR_KEY_OFF, 0);
                    }
                 }

              else
                 {
                 // Invalid parameter
                 cmd_ShowSystemMessage(MSG_BAD_PARM1, 0);
                 }
              break;

         default:
              // Invalid parameter
              cmd_ShowSystemMessage(MSG_BAD_PARM1, 0);
              break;
         }
  return (rc);
}

