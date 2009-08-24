/*!
   $Id: set.c,v 1.1.1.1 2003/10/04 08:27:50 prokushev Exp $ 
   
   @file set.c

   @brief set command - sets environment variables

   can be built as separate command, or as part of cmd/bathproc

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>

   @bug does not link with current libraries (no setenv function??)
   @bug we do accept second '=' in the command line, but we should not!
   @warning building as embed command not tested (as there is nothing to be
            embed into, yet)

*/

#define INCL_DOSERRORS
#include <osfree.h>
#include <cmd_shared.h> /* comand line tools' shared functions */
#include <env.h>

/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'

int setMain(int argc,char *argv[]);

#ifdef NOT_STANDALONE
/*!
 set's main routine, when built as embed command

 @param cmd_line   command line, with 'set' command (first word) stripped out
                   eg, is user issues "set pra1 pra2  = es", cmd_line should
                   contain "pra1 pra2  = es", this string should be 
                   leading-spaces free

 @warning THIS FUNCTION IS UNTESTED (yet)
*/
int set(char *cmd_line)
{
 int i;
 BOOL bFound=FALSE;
 APIRET rc;
 char *argv1;
 char *argv2;
 char *tmp;
 char *newArgv[2]; /* new arguments, for setMain, note, we strip, standard
                      main's argv[0] argument! */

 if (strlen(cmd_line)==0)
  return setMain(0,NULL);

 for (i=0;i<strlen(cmd_line);i++)

   if (cmd_line[i]=='=') 
  {
   bFound=TRUE;
   break;
  };

 argv1=(char*)calloc(1,i+1); //TODO check is not null
 strncpy(argv1,cmd_line,i);

 newArgv[0]=argv1;

 if (bFound==FALSE)
 {
  newArgv[1]=NULL;
  rc =setMain(1,newArgv);
  free(argv1);
  return rc;
 };
 
 if (i==strlen(cmd_line)-1)
 {
   argv2=(char*)calloc(1,3);
   strcpy(argv2,"\0");
 } else
 {
   tmp=cmd_line+i+1;
   argv2=(char*)calloc(1,strlen(tmp)+1);
   strcpy(argv2,tmp);
 };

 newArgv[1]=argv2;

 rc=setMain(2,newArgv);

 free(argv1);
 free(argv2);

 return rc;  
};
#else
/*!
  set's main procedure when built as standalone command
*/
int main(int argc,char *argv[])
{
 int iEqual,i;
 int rc;
 BOOL bFound=FALSE;
 int iStrLen;
 char *argv1,*argv2;
 char *newArgv[2]; /* new arguments, for setMain, note, we strip, standard
                      main's argv[0] argument! */

 if (argc<=1) /* if we have no real arguments */
   return setMain(0,NULL);

 iStrLen=0;

 /* otherwise search for = sign */
 for (iEqual=1;iEqual<argc;iEqual++)
{
   if (strcmp(argv[iEqual],"=")==0)
   {
    bFound=TRUE;
    break;
   } else 
   iStrLen+=(strlen(argv[iEqual])+1);
};

 argv1=(char*)calloc(1,iStrLen); //TODO check is not null!!!

 strcpy(argv1,argv[1]);

 /* now build argument list for setMain*/
 for (i=2;i<iEqual;i++)
 {
   strcat(argv1," ");
   strcat(argv1,argv[i]); 
 };

 newArgv[0]=argv1;

 if (bFound==FALSE) /* there was no '=' sign */
 {
  newArgv[1]=NULL;
  rc =setMain(1,newArgv);
  free(argv1);
  return rc;
 };

 /* if we are here, there is = sign */

 if (iEqual==argc-1)
 {
    argv2=(char*)calloc(1,3); //TODO check is not null!!!
    strcpy(argv2,"\0");
 } else
 {
   iStrLen=0;
   for (i=iEqual+1;i<argc;i++)
    iStrLen+=(strlen(argv[i])+1);

    argv2=(char*)calloc(1,iStrLen); //TODO check is not null!!!

    strcpy(argv2,argv[iEqual+1]);

   for (i=iEqual+2;i<argc;i++)
   {
     strcat(argv2," ");
     strcat(argv2,argv[i]);   
   };
 };

 newArgv[1]=argv2;

 rc=setMain(2,newArgv);
 free(argv1);
 free(argv2);

 return rc;
};
#endif

/*!
  set's command worker function

  @param argc  number of parameters, valid values:
                0 - list environment variables
                1 - show given environment variable contest (or request help)
                2 - set given environment variable contest
  @argv        pointer to strings table, depending on argc:
                0 - pointer invalid (no usuage)
                1 - argv[0] contains first parameter (/? or env variable, for
                    wich, content should be displayed)
                2 - argv[1] contains value for argv[0] environment variable

  @return
    NO_ERROR succesfull completition
    1 - error occured
*/
int setMain(int argc,char *argv[])
{
 int i;
 char *tmp;

 /* check if user wants info from us (/? or -?) */
 if (argc==1)
   if (((argv[0][0]==PARAM_CHAR1) || (argv[0][0]==PARAM_CHAR2)) &&
      (argv[0][1]=='?'))
 {
  /* show help message */
  cmd_ShowSystemMessage(cmd_MSG_SET_HELP,0L);
  return NO_ERROR;
 }; 

 switch (argc)
 {
  case 0:
   i=0;
   while (environ[i])
    printf("%s\n",environ[i++]);
   break;
  case 2:
    /*! bug this does not link when using libc library!!! */
    if(setenv(strupr(argv[0]),argv[1],1)!=0)
     return 1; //TODO fix return code
    break;
  case 1:
   printf("%s=%s",strupr(argv[0]),getenv(strupr(argv[0])));
   break;
 };

  return NO_ERROR;
};