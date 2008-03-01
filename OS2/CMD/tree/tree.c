/*!
   $Id: tree.c,v 1.1 2003/12/12 18:18:47 prokushev Exp $

   @file tree.c

   @brief tree command - displays directory structure

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Cristiano Guadagnino <criguada@tin.it>
   @author Bartosz Tomasik <bart2@asua.org.pl>

   @todo: add proper syntax description to system msg file
   @todo: extend options (very low priority todo)
   @todo: fix return codes
   @todo: ctrl+c support
   @todo: add support of 4OS2 extensions
*/

#define INCL_DOSERRORS
#include <osfree.h>
#include <stdio.h>
#include <string.h>
#include <cmd_shared.h>

/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'

int RecurseActionFunction(char *path,char*file,int iAction,void *data);
int RecurseErrorFunction(ULONG errornum,void *data);

/*!
  tree command's main function
*/
int main(int argc,char *argv[])
{
  int iTreeOptions= /* we want to: */
       all_RECURSE_DIRACTION| /* perform it on directories too */
       all_RECURSE_DIRS| /* recurse into subdirectories */
       all_RECURSE_IN| /* be notified when entering new directory */
       all_RECURSE_NOSUBDIRS| /* be notified that given directory has
                                no directories to be recused in */
       all_RECURSE_NOFILES; /* be notified that no files were found matching
                               our mask in returned directory */

  CHAR chStartPath[CCHMAXPATH]="";
  char *tmp;
  /*! @todo: add an option to mach files by attributes example: /a:h */
  int fileAttributes=FILE_READONLY|FILE_ARCHIVED;

  switch (argc)
  {
   case 1:
     strcpy(chStartPath,"\\*");
     break;

   case 2:
     if (((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
      (argv[1][1]=='?')&& (argv[1][2]=='\0'))
     {
       cmd_ShowSystemMessage(cmd_MSG_TREE_HELP,0L);
       return NO_ERROR;
     };

     if (((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
      ((argv[1][1]=='f')||(argv[1][1]=='F')))
     {
      strcpy(chStartPath,"\\*");
      iTreeOptions|=all_RECURSE_FILEACTION; /* perform action on files */
      break;
     };

   case 3:
     tmp=all_GetFileFromPath(argv[1]);

     printf("get file form: %s\n",tmp);
     if (strlen(tmp)!=0)
     {
        cmd_ShowSystemMessage(MSG_BAD_PARM1, 0L);
        return 1;
     };

     tmp=all_GetDirFromPath(argv[1]);
      strcpy(chStartPath,tmp);
     printf("get file form2: %s\n",chStartPath);
     free(tmp);
      strcat(chStartPath,"*");

     if (argc==2) break;

     if (((argv[2][0]==PARAM_CHAR1) || (argv[2][0]==PARAM_CHAR2)) &&
      ((argv[2][1]=='f')||(argv[2][1]=='F')))
     {
      strcpy(chStartPath,"\\*");
      iTreeOptions|=all_RECURSE_FILEACTION; /* perform action on files */
      break;
     };

   default:
        cmd_ShowSystemMessage(MSG_BAD_PARM1, 0L);
        return 1;
  };

    /*! @bug When printing, the real TREE tool wraps text without
             breaking words. If it can't find a space, it wraps right
             from the start, to gain some space. If it still can't fit
             the path on one line, and can't find a space, it breaks
             the text.
    */

  cmd_ShowSystemMessage(cmd_MSG_TREE_START, 0L);
  printf("\n");

  if (all_PerformRecursiveAction(chStartPath,iTreeOptions,fileAttributes,
      RecurseActionFunction,NULL,RecurseErrorFunction,NULL)
       ==all_ERR_RECURSE_NORECURSION)
  {
    cmd_ShowSystemMessage(cmd_MSG_TREE_NODIRS, 0L);
    return 1; //TODO fix, or maybe this is normal completition?
  };

  return NO_ERROR;
};

int RecurseActionFunction(char *path,char*file,int iAction,void *data)
{
  static BOOL bFirstLevel=TRUE;
  static BOOL bDisplayDirectories=FALSE;
  static BOOL bFirstFileInThisDir=FALSE;
  static BOOL bFirstSubdirInThisDir=FALSE;
  char dir[CCHMAXPATH]="";

  switch (iAction)
  {
    /* when entering new directory */
    case all_RECURSE_IN:
     if (!bFirstLevel)
       {
         /* we're not listing directories at first depth level */
         printf("\n\n\n");
         strcpy(dir,path); /* remove ending '\' charcter */
         dir[strlen(path)-1]='\0';
         cmd_ShowSystemMessage(cmd_MSG_TREE_PATH, 1L, "%s", dir);
         bDisplayDirectories=TRUE;
       };
      bFirstFileInThisDir=TRUE;
      bFirstSubdirInThisDir=TRUE;
     break;
    /* files were found */
    case all_RECURSE_FILEACTION: /* file was found */
       /* we need to show 'header' if this is first file in this directory */
       if (bFirstFileInThisDir)
       {
         printf("\n");
         cmd_ShowSystemMessage(cmd_MSG_TREE_FILES, 1L, "%s", file);
         bFirstFileInThisDir=FALSE;
       } else
           cmd_ShowSystemMessage(cmd_MSG_TREE_SPACER, 1L, "%s", file);
     break;
    case all_RECURSE_DIRACTION: /* directory was found */
     /* we do not list . and .. entries */
     if ((strcmp(file,".")!=0)&&(strcmp(file,"..")!=0))
     {
       if (!bFirstLevel)
       {
         if (bDisplayDirectories)
         {
           if (bFirstSubdirInThisDir)
           {
             printf("\n");
             cmd_ShowSystemMessage(cmd_MSG_TREE_SUBDIRS, 1L, "%s", file);
             bFirstSubdirInThisDir=FALSE;
           } else
            cmd_ShowSystemMessage(cmd_MSG_TREE_SPACER, 1L, "%s", file);
         };
       } else bFirstLevel=FALSE;
     }; /* if ((strcmp(file,".")!=0)&&(strcmp(file,"..")!=0)) */
     break;
    case all_RECURSE_NOFILES:
       printf("\n");
       cmd_ShowSystemMessage(cmd_MSG_TREE_NOFILES, 0L);
     break;
    case all_RECURSE_NOSUBDIRS:
       printf("\n");
      cmd_ShowSystemMessage(cmd_MSG_TREE_NOSUBDIRS, 0L);
     break;
  };
  return NO_ERROR;
};

int RecurseErrorFunction(ULONG errornum,void *data)
{
  cmd_ShowSystemMessage(errornum, 0L);
  return 1;
};

