/*!
   $Id: pause.c,v 1.1.1.1 2003/10/04 08:27:50 prokushev Exp $ 
   
   @file pause.c

   @brief pause command - suspends batch operations

   can be built as separate command, or as part of cmd/bathproc

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>

   @bug     because of getch usuage, does not reads extended keys, see more
            in the code below (pauseMain function)
   @bug     because of above: does not link with current libraries

   @warning building as embed command not tested (as there is nothing to be
            embed into, yet)

*/

#define INCL_DOSERRORS
#include <osfree.h>
#include <cmd_shared.h> /* comand line tools' shared functions */

/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'

int pauseMain(int argc,char *argv[]);

#ifdef NOT_STANDALONE
/*!
 pause's main routine, when built as embed command

 @param cmd_line   command line, with 'pause' command (first word) stripped out
                   eg, is user issues "pause pra1 pra2  = es", cmd_line should
                   contain "pra1 pra2  = es", this string should be 
                   leading-spaces free (i know, this is silly example, as
                   most of us, never passes any arguments to pause)

 @warning THIS FUNCTION IS UNTESTED (yet)
*/
int set(char *cmd_line)
{
 APIRET rc;
 char *newArgv[2]; /* new arguments, for pauseMain, note, we strip, standard
                      main's argv[0] argument! */

 if (strlen(cmd_line)==0)
 {
  return pauseMain(0,NULL);
 };

 newArgv[1]=NULL;
 newArgv[0]=calloc(1,strlen(cmd_line)+1);

 strcpy(newArgv[0],cmd_line);

 rc=pauseMain(1,newArgv);

 free(newArgv[0]);
 return rc;  
};
#else
/*!
  pause's main procedure when built as standalone command
*/
int main(int argc,char *argv[])
{
 int rc;
 char *newArgv[2]; /* new arguments, for pauseMain, note, we strip, standard
                      main's argv[0] argument! */

 if (argc<=1) /* if we have no real arguments */
   return pauseMain(0,NULL);


 newArgv[1]=NULL;
 newArgv[0]=calloc(1,strlen(argv[1])+1);

 strcpy(newArgv[0],argv[1]);

 rc=pauseMain(1,newArgv);

 free(newArgv[0]);
 return rc;   
};
#endif

/*!
  pause's command worker function

  @param argc  number of parameters, valid values:
                1 - if argv equals /? user is requesting help,
                   otherwise ignored
  @argv        pointer to strings table, depending on argc:
                0 - pointer invalid (no usuage)
                1 - argv[0] contains first parameter (only /? is valid,
                    other values are ignored)

  @return
    NO_ERROR succesfull completition
*/
int pauseMain(int argc,char *argv[])
{

 /* check if user wants info from us (/? or -?) */
 if (argc==1)
   if (((argv[0][0]==PARAM_CHAR1) || (argv[0][0]==PARAM_CHAR2)) &&
      (argv[0][1]=='?'))
 {
  /* show help message */
  cmd_ShowSystemMessage(cmd_MSG_PAUSE_HELP,0L);
  return NO_ERROR;
 }; 

  cmd_ShowSystemMessage(cmd_MSG_PAUSE_WAITING,0L);

  /*! @bug we need to change ot to something reasonable, maybe someone will
          create cmd_GetResponse function? for such things? */
  getch();

  return NO_ERROR;
};