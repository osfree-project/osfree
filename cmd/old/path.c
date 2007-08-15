/*!
   $Id: path.c,v 1.1.1.1 2003/10/04 08:27:50 prokushev Exp $ 
   
   @file path.c

   @brief path command - sets search path

   can be built as separate command, or as part of cmd/bathproc

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>

   @warning building as embed command not tested (as there is nothing to be
            embed into, yet)

*/

#define INCL_DOSERRORS
#include <osfree.h>
#include <cmd_shared.h> /* comand line tools' shared functions */

/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'

int pathMain(int argc,char *argv[]);

#ifdef NOT_STANDALONE
/*!
 path's main routine, when built as embed command

 @param cmd_line   command line, with 'path' command (first word) stripped out
                   eg, is user issues "path pra1 pra2  = es", cmd_line should
                   contain "pra1 pra2  = es", this string should be 
                   leading-spaces free

 @warning THIS FUNCTION IS UNTESTED (yet)
*/
int path(char *cmd_line)
{
 int i;
 APIRET rc;
 char *argv1;

 if (strlen(cmd_line)==0)
  return pathMain(NULL);

 for (i=0;i<strlen(cmd_line);i++)
 
 if (cmd_line[i]=='=') ||(cmd_line[i]==' ')
 break;

 if (i==0) return 1;

 argv1=(char*)calloc(1,i+1); //TODO check is not null
 strncpy(argv1,cmd_line,i);

 rc =pathMain(argv1);
 free(argv1);
 return rc;
};
#else
/*!
  path's main procedure when built as standalone command
*/
int main(int argc,char *argv[])
{
 if (argc<=1) /* if we have no real arguments */
   return pathMain(NULL);

 return pathMain(argv[1]);
};
#endif

/*!
  path's command worker function

  @newstring    newstring, to be set as path, or NULL value, if user requested
                printing only

  @return
    NO_ERROR succesfull completition
    1 - error occured
*/
int pathMain(char *newstring)
{
 int rc;
 char chPath="PATH";
 char *tmp[2];

 if (newstring!=NULL)
 {
   /* check if user wants info from us (/? or -?) */
    if (((newstring[0]==PARAM_CHAR1) || (newstring[0]==PARAM_CHAR2)) &&
      (newstring[1]=='?') && (newstring[2]=='\0'))
   {
    /* show help message */
    cmd_ShowSystemMessage(cmd_MSG_PATH_HELP,0L);
    return NO_ERROR;
   }; 

   tmp[0]=chPath;
   tmp[1]=newstring;
   return setMain(2,tmp);
 };
 
 tmp[0]=chPath;
 return setMain(1,tmp);

};