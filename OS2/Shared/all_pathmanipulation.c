/*!
   $Id: all_pathmanipulation.c,v 1.1.1.1 2003/10/04 08:36:16 prokushev Exp $

   @file all_pathmanipulation.c

   @brief path manipulation helper functions
   shared along all code

   (c) osFree Project 2002, <http://www.osFree.org>

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#include <string.h>
#include <all_shared.h> /* shared functions and defines */

/*
  Extract directory part from full-path file mask eg.: extracting from
  c:\dir1\dir2\abc.d will result in c:\dir1\dir2\ returned

  @param fileMask fileMask, from wich directory should be extracted (eg.:
                  c:\dir1\dir2\abc.d

  @return
     pointer to zero terminated string, containing extracted directory
     note that user is responsible for freeing that string memory

  @todo check for NULL when allocation memory, and do something... (what?)
*/
char* all_GetDirFromPath(char *fileMask)
{
  int iStrLen,iCounter;
  char *result=(char*)calloc(1,strlen(fileMask));

  iStrLen=strlen(fileMask);

  if (iStrLen==0)
    return result;

  for (iCounter=iStrLen;iCounter>0;iCounter--)
    if (fileMask[iCounter]=='\\') break;

  if (iCounter==0)
  {
    if ((fileMask[iCounter]=='\\')&&(strlen(fileMask)>1))
      strncpy(result,fileMask,1);
    else
    {
      /* there are no subdirs specified, but ther may be drive letter! */
      if ((iStrLen>=1) && (fileMask[1]==':'))
      {
       strncpy(result,fileMask,2);
       strcat(result,"\\");
      };
    };
  } else
  strncpy(result,fileMask,iCounter+1);

  return result;
};


/*
  Extract file mask part from full-path file mask eg.: extracting from
  c:\dir1\dir2\abc.d will result in abc.d returned

  @param fileMask fileMask, from wich file mask should be extracted (eg.:
                  c:\dir1\dir2\abc.d

  @return
     pointer to zero terminated string, containing extracted file mask
     note that user is responsible for freeing that string memory

  @todo check for NULL when allocation memory, and do something... (what?)
*/
char * all_GetFileFromPath(char *fileMask)
{
  int iStrLen,iCounter;
  char *result;
  char *tmp;

  iStrLen=strlen(fileMask);

  if (iStrLen==0) return "";

  for (iCounter=iStrLen;iCounter>0;iCounter--)
    if (fileMask[iCounter]=='\\') break;

  if (iCounter==0)
  {
    if (fileMask[iCounter]=='\\')
      tmp=fileMask+1;
    else
    {
      /* there are no subdirs specified, but ther may be drive letter! */
      if ((iStrLen>=1) && (fileMask[1]==':'))
       tmp=fileMask+2;
      else
       tmp=fileMask;
    };
  } else
   tmp=fileMask+iCounter+1;

   if (strlen(tmp)==0) return "";

   result=(char*)calloc(1,strlen(tmp)); /* @todo: check is null! */

  strcpy(result,tmp);
  return result;
};

/*!
 Gets full path for specified file (simple wrapper for DosQueryPathInfo)

 @param file     file name

 @return
    pointer to string with full path

 @todo error handling!!
*/
char *all_GetFullPathForFile(char *file)
{
 char *result=(char *)calloc(1,CCHMAXPATH);    /* resulting string */
 APIRET rc;

 rc=DosQueryPathInfo(file,FIL_QUERYFULLNAME,result,CCHMAXPATH);
 return result;
};


/*!
  Gets current process working path or current path for specified disk

  @param disk    disk number, drive numbering scheme:
                   0 - means current, 1 - a: 2 - b: 3 - c: and so on...
  @param ppath   address of buffer where path should be placed

  @return
    0 - successfull completition, and *path contains requested path (user
         is responsible for freeing string memory)
    rc from Dos* functions when error occured, bufer data remains unchanged
*/
int all_GetCurrentPath(int disk, char **ppath)
{
 char *path=*ppath;
 char *buf=NULL;
 ULONG size=0;
 ULONG ulAvail;
 APIRET rc;
 PSZ pszDisk="C:";

 /* first ask for needed space */
 rc=DosQueryCurrentDir(disk,buf,&size);
 if (rc!=ERROR_BUFFER_OVERFLOW)
  return rc;

 if (disk==0) /* current disk, we need to find, which one is this now */
 {
  rc=all_QueryCurrentDisk(pszDisk,&ulAvail);

  if (rc) return rc;
 } else
  pszDisk[0]='A'-1+disk;

 /* there is  directory info */
 if (size!=0)
 {
  buf=(char*)calloc(1,size+5);
  /* the trick is, we'll put disk letter in front of the dir */
  rc=DosQueryCurrentDir(disk,&buf[3],&size);
  if (rc)
  {
   free(buf);
   return rc;
  };
  if (path!=NULL) free(path);
  *ppath=buf;
  buf[0]=pszDisk[0]; // drive letter
  buf[1]=pszDisk[1]; // ':'
  buf[2]='\\';
 } else
 {
  if (path!=NULL) free(path);
  path=(char *)calloc(1,5);
  strcpy(path,pszDisk);
  strcat(path,"\\");
  *ppath=path;
 };

 return 0;
};
