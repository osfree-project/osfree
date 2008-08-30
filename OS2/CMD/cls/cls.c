/*!
   @file

   @brief cls command - clears screen

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>

   @todo: add support of 4OS2 extensions
*/

#define INCL_VIO
#define INCL_DOSERRORS
#include <osfree.h>
#include <cmd_shared.h>

/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'

int main(int argc,char *argv[])
{
  BYTE bCell[2];                       /* Char/Attribute array       */

 if ((argc>1)&&((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)))
 {
  if ((argv[1][1]=='?')&& (argv[1][2]=='\0'))
  {
    cmd_ShowSystemMessage(cmd_MSG_CLS_HELP,0L);
    return NO_ERROR;
  } else
  {
    cmd_ShowSystemMessage(MSG_BAD_SYNTAX,0L);
    return 1;
  };
 };
  bCell[0] = 0x20;                     /* Space Character            */
  bCell[1] = 0x07;                     /* Default Attrib             */
  VioScrollDn( 0, 0, (USHORT)0xFFFF, (USHORT)0XFFFF,
                     (USHORT)0xFFFF, bCell, (HVIO) 0);/* CLS         */
  VioSetCurPos(0, 0, (HVIO) 0);        /*                Pos cursor  */

  return NO_ERROR;
};
