/*!
   @file

   @ingroup attrib

   @brief attrib command - sets file attributes

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>

   @todo: return code, different when an error is occurinng and option
         that allows to break execution after first error /x ?
   @todo: add support of 4OS2 extensions
   @todo: add extended attributes support (like .COMMENT, .TYPE)
   @todo: add "-" modifier to /a switch.
*/


/* C standard library headers */
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <cmd_shared.h> /* comand line tools' shared functions and defines */

/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'
#define PARAM_NEG   '~'
#define PARAM_EQUAL '='
#define PARAM_SET   '+'

#define CMD_SCAN_OFF "--" /*!< "turn off cmd-line scanning" parameter */

/* attrib characters */
#define ATR_CHAR_R 'R' /*!< Readonly-attrib's character definition  */
#define ATR_CHAR_S 'S' /*!< System-attrib's character definition */
#define ATR_CHAR_H 'H' /*!< Hidden-attrib's character definition */
#define ATR_CHAR_D 'D' /*!< file is directory (only for filter) */
#define ATR_CHAR_A 'A' /*!< Archive-attrib's character definition */
#define ATR_CHAR_U '_' /*!< Ignore-attrib's character definition  */

/* attribute setting options */
#define ATR_ACTION_IGN 0 //!< igore attribute (don't change state)
#define ATR_ACTION_SET 1 //!< attribute should be set
#define ATR_ACTION_REM 2 //!< clear
#define ATR_ACTION_NEG 3 //!< negate

/*!
   @brief attributes action structure

   Structure holding, 'action to berformed on attributes' code values.
   Each field can contain one af ATR_ACTION_* values
*/
typedef struct _attrib
{
 BYTE bReadOnly;  //!< Readonly attribute code action
 BYTE bHidden;    //!< Hidden attribute code action
 BYTE bSystem;    //!< System attribute code action
 BYTE bArchived;  //!< Archived attribute code action
 BYTE bDirectory; //!< File is directory (used only in filter)
} ATTRIB;

/*!
  @brief program options structure

  This structure holds options set by user, from command line
*/
typedef struct _options
{
 BOOL bNoScan;          /*!< command line scaning indicator, TRUE when command line scanning is truned off */
 BOOL bFlagsSet;        /*!< are attrib parameters for files set? */
 BOOL bRecurse;         /*!< should we recurse into subdirs */
 BOOL bNoUpdateDirs;    /*!< Should directory attribs be updated when recusing */
 BOOL bAttributeSelect; /*!< Make action only for attribute mask */
 BOOL bPause;           /*!< Pause each screen */
 BOOL bQuiet;           /*!< Be quiet  */
 BOOL bNoErrors;        /*!< Show only critical error messages */
} OPTIONS;

/*! macro to test is a specified chaacter, parameter specifier */
#define ISPARAM(character) \
      ((character==PARAM_CHAR1) || (character==PARAM_CHAR2) || \
      (character==PARAM_NEG) || (character==PARAM_SET) || \
      (character==PARAM_EQUAL) )

/*! macro to modify attribute (inetified by bit_attr), depending on flags set */
#define MODIFYATTRS(flag,bit_attr) \
 switch(flag) \
 { \
   case ATR_ACTION_NEG: \
    if ( fileStatus.attrFile & bit_attr) \
    { \
     fileStatus.attrFile &= ~bit_attr; \
     break; \
    }; \
   case ATR_ACTION_SET: \
    fileStatus.attrFile|=bit_attr; \
    break; \
   case ATR_ACTION_REM: \
     fileStatus.attrFile &= ~bit_attr; \
    break; \
 }


//--- local functions

/*! @brief Process file masks given in argv,using given flags and options */
int ProcessFiles(int start,int end,char* argv[],ATTRIB *flags,ATTRIB *filterflags,OPTIONS *options);
/*! @brief physical modyfication of given file's attributes */
int ModifyFileAttribs(char *file,ATTRIB *flags);
/*! @brief function to be called on each file/dir found by all_PerfomRecursiveAction */
int RecurseActionFunction(char *path,char*file,int iAction,void *data);
/*! @brief Dos* error handling function */
int RecurseErrorFunction(ULONG rc,void *data);
/*! @brief sets flags and/or options field based on characters found in param
   string, and action_type specifier */
int SetMemFlags(ATTRIB *filterflags,ATTRIB *flags,char *param,int len,char action_type,OPTIONS *options);
/*! @brief gets length of current action parameter */
int GetParamLen(char *param);
/*! @brief Parse given string and based on it set options and parameters */
int ProcessCmdParam(char *param,ATTRIB *atr,ATTRIB *filter,OPTIONS *options);


/*!
  main attrib routine

  @todo add exception handling (ctrl+c/brak) and returning correct rc then
*/
int main (int argc, char* argv[], char* envp[])
{
  APIRET rc=0;
  int i;
  ATTRIB flags={(0)};       /* current attrib settings */
  ATTRIB filterflags={(0)}; /* set attrib if attrib is set */
  OPTIONS options={(0)};    /* current processing options */
  int iEndArgc=0;

  options.bNoUpdateDirs=TRUE;

  if (argc<2)
  {
    if (!options.bQuiet) cmd_ShowSystemMessage(MSG_BAD_SYNTAX,0L);
    return 1; //@todo fix
  };

  if ((((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
       (argv[1][1]=='?') || ((argv[1][0]==PARAM_CHAR1) && toupper(argv[1][1])=='H'))&& (argv[1][2]=='\0'))
  {
    if (!options.bQuiet) cmd_ShowSystemMessage(cmd_MSG_ATTRIB_HELP,0L);
    return NO_ERROR;
  };

  /* check eveything on command line */
  for (i=1;i<argc;i++)
  {
   if (ISPARAM(argv[i][0]) && (options.bNoScan==FALSE))
   {
    if (ProcessCmdParam(argv[i],&flags,&filterflags,&options)!=0)
    {
     if (!options.bQuiet) cmd_ShowSystemMessage(MSG_BAD_PARM2,1L,"%s",argv[i]);
     return 1; //@todo fix
    };
   } else
   {

//    if (!options.bFlagsSet) /* we cannot process files without atrrib params set */
//    {
//     if (!options.bQuiet) cmd_ShowSystemMessage(MSG_BAD_SYNTAX,0L);
//     printf("info\n");
//     return 1; //@todo fix
//    };

    /* if command line scaning is turned off, we can safely select whole
       rest of it, as file specifier */
    if (options.bNoScan==TRUE)
        iEndArgc=argc;
     else
    /* otherwise, we need to find, where attrib params start again */
    for (iEndArgc=i;iEndArgc<argc;iEndArgc++)
    {
       /* it looks like a pram, but may be off-scaning as well */
       if (ISPARAM(argv[iEndArgc][0]))
       {
         if (strcmp(argv[iEndArgc],CMD_SCAN_OFF)!=0)
           /* it is a parameter, so we found ending file specifier
              (at least for that parameters) */
           break;
          else
         {
         /* it is only cmd-scan-off param, so process current list,
            turn off scaning and let it process rest! */

           options.bNoScan=TRUE; /* probably useless now, but for safety ; */

           /* process current file list (to '--') */
           if ( (ProcessFiles(i,iEndArgc,argv,&flags,&filterflags,&options)!=0) && (!rc))
           rc=1;

           i=iEndArgc+1; /* skip '--' */
           iEndArgc=argc; /* and select all the rest for processing */
           break;
         }; /* END: if ((strcmp(argc[iEndArgc],CMD_SCAN_OFF)!=0) */
       }; /* END: if ((ISPARAM(argv[iEndArgc][0])) */
    }; /* END: for (iEndArgc=i;iEndArgc<argc;iEndArgc++) */

    /* process files */
    if ( (ProcessFiles(i,iEndArgc,argv,&flags,&filterflags,&options)!=0) && (!rc))
    rc=1; //@todo: fix?

    /* skip file specifications when command line parsing */
    i=iEndArgc-1;
    options.bFlagsSet=FALSE; /* reset options and flags */
    options.bRecurse=FALSE;
    options.bNoUpdateDirs=TRUE;
    options.bAttributeSelect=FALSE;
    options.bPause=FALSE;
    options.bQuiet=FALSE;
    options.bNoErrors=FALSE;
    memset(&flags,0,sizeof(flags));
  }; /* END: if (ISPARAM(argv[i][0]) && (bNoScan==FALSE)) */
 }; /* END: for (i=1;i<argc;i++) */

 return rc;
};

/*!
   @brief Process file masks given in argv (from start to end indexes),using given
   flags and options. Depending on options, files processing may include
   directories and can be recursive.

   @param start    index of first filemask in argv
   @param end      index of last filemask in argv
   @param argv     argument list (simply pass main's argv here)
   @param flags    flags to be used when processing files
   @param options  options to be used when processing files

   @return
     NO_ERROR - files were processed succesfully
     1 - an error occured during files processing
*/
int ProcessFiles(int start,int end,char* argv[],ATTRIB *flags,ATTRIB *filterflags,OPTIONS *options)
{
 int i;
 char cActionParams=all_RECURSE_FILEACTION;
 /*! @todo: add an option to mach files by attributes example: /a:h */
 int fileAttributes=FILE_SYSTEM|FILE_READONLY|FILE_HIDDEN|FILE_ARCHIVED;

 if (filterflags->bDirectory==1) fileAttributes=fileAttributes|MUST_HAVE_DIRECTORY;
 if (filterflags->bReadOnly==1) fileAttributes=fileAttributes|MUST_HAVE_READONLY;
 if (filterflags->bArchived==1) fileAttributes=fileAttributes|MUST_HAVE_ARCHIVED;
 if (filterflags->bSystem==1) fileAttributes=fileAttributes|MUST_HAVE_SYSTEM;
 if (filterflags->bHidden==1) fileAttributes=fileAttributes|MUST_HAVE_HIDDEN;

 if (options->bNoUpdateDirs==FALSE) fileAttributes=fileAttributes|FILE_DIRECTORY;

 if (options->bRecurse) cActionParams|=all_RECURSE_DIRS;
 if (options->bNoUpdateDirs==FALSE)
 {
   cActionParams|=all_RECURSE_DIRACTION;
 }

 for (i=start;i<end;i++)
 {
  if (all_PerformRecursiveAction(argv[i],cActionParams, fileAttributes,
       RecurseActionFunction, (void *) flags, RecurseErrorFunction ,NULL)!=0)
   {
     //! @todo change it to something reasonable ;)
     printf("bad things returned by all_PerformRecursiveAction\n");
     return 1;
   };
 }; /* END: for (i=start;i<end;i++) */

 return NO_ERROR;
};

/*!
  Parse given string and based on it set options and parameters

  @param param    parameter string to be parsed
  @param atr      pointer to ATTRIB structure, which may be updated by
                  parameter
  @param options  pointer to OPTIONS structure, which may be updated by
                  parameter

  @return
    NO_ERROR given parameter was sucessfylly processed
    1 - given parameter contained invalid characters, and couldn't be
        processed
*/
int ProcessCmdParam(char *param,ATTRIB *atr,ATTRIB *filter,OPTIONS *options)
{
  char *tmp=param;
  char *tmp2;
  int strCount;

  while (*tmp) /* while there is string */
  {
   /* the following switch is here to, catch up -- string, parameters and
      skip the rest of chars */
   switch (*tmp) /* switch current charcter */
   {
     case PARAM_CHAR2: /* '-' */
      if (strcmpi(param,CMD_SCAN_OFF)==0) /* is this turning cmd-scan off? */
      {
        options->bNoScan=TRUE;
        return NO_ERROR;
      }
     case PARAM_CHAR1: /* '/' */
     case PARAM_SET:   /* '+' */
     case PARAM_NEG:   /* '~' */
     case PARAM_EQUAL: /* '=' */
      strCount=GetParamLen(tmp+1); /* count this param len, skipping param char */
      if (SetMemFlags(filter,atr,tmp+1,strCount,*tmp,options)!=0)
      return 1;    /* @todo fix */
      tmp++;
      break;
   };
   tmp++;
  }; /* END; while (*tmp) */

  return NO_ERROR;
};

/*!
  Gets number of valid parameter characters (not a paramater specifiers)
  found on start of given parameter.

  @param param parameter string to be scanned

  @return
     number of valid parameter characters (not a parameter specifiers)
*/
int GetParamLen(char *param)
{
 char *tmp=param;
 int result=0;

 while (*tmp)
 {
   if (ISPARAM(*tmp)) break;
   result++;
   tmp++;
 };

 return (result);
};

int ParseFilter(ATTRIB * filterflags, char *param,int len)
{
 short iCounter;
 BYTE F=1; // 0 - ignore, 1 - set, 2 - not set
 char cParamChar;

 for (iCounter=0;iCounter<len;iCounter++)
 {
  cParamChar=toupper(param[iCounter]);

  /* recognize param char and select flag field */
  switch (cParamChar)
  {
    case ':': // Skip ":" for compatibility
     break;
    case PARAM_CHAR2:
      F=2;
     break;
    case ATR_CHAR_D: // Skip underline character
      filterflags->bDirectory=F;
      F=1;
     break;
    case ATR_CHAR_R:
      filterflags->bReadOnly=F;
      F=1;
     break;
    case ATR_CHAR_A:
      filterflags->bArchived=F;
      F=1;
     break;
    case ATR_CHAR_S:
      filterflags->bSystem=F;
      F=1;
     break;
    case ATR_CHAR_H:
      filterflags->bHidden=F;
      F=1;
     break;
    default:
     return 1; //@todo fix
  }; /* END: switch (cParamChar) */

 }; /* END: for (iCounter=0;iCounter<len;iCounter++) */
 return NO_ERROR;
}

/* scans param for len length for attrib letters and performs requested action */

/*!
  Sets program otions or attribute action codes, based on parameter string, and
  action type passed in. Parameter string, is being examined only for len
  characters.

  @param flags        pointer to an ATTRIB structure, where recognized
                      attribute action codes should be stored
  @param param        parameter string
  @param len          length of valid parameter characters. Characters above
                      len'th are ignored
  @param action_type  action-type character, can be one of following:
                      PARAM_EQUAL - '=', which means setting specified by
                                         param attributes, and removing rest
                      PARAM_SET   - '+', which means setting on, specified
                                         attributes
                      PARAM_CHAR1 - '/', which means that, param string
                                         contains an option selector
                      PARAM_CHAR2 - '-', which means removing, specified
                                         attributes
                      PARAM_NEG   - '~', which means negating, specified
                                         attributes
  @param options      pointer to an OPTIONS structure, where recognized
                      options settings should be stored

  @return
     NO_ERROR - successfull completition
     1 - parameter contained invalid character (could not be recognized)
*/
int SetMemFlags(ATTRIB *filterflags,ATTRIB *flags,char *param,int len,char action_type,OPTIONS *options)
{
 char cActionType=action_type; /* current action type */
 char cParamChar;    /* current parameter char */
 int iCounter;       /* character counter */
 BYTE *bCurrentFlag;  /* selected flag from ATTRIB struct */


 if (len==0) return NO_ERROR; /* nothing to be set */

 /* equal means, set specified, and reset others */
 if (cActionType==PARAM_EQUAL)
 {
   flags->bReadOnly=ATR_ACTION_REM; /* so first, we reset all of them */
   flags->bHidden=ATR_ACTION_REM;
   flags->bSystem=ATR_ACTION_REM;
   flags->bArchived=ATR_ACTION_REM;
   cActionType=PARAM_SET; /* and then, we'll set specified ones */
 };

 /* we support / here, rest (+-~=) later */
 if (cActionType==PARAM_CHAR1) /* '/' */
 {
  if (len==1) // one-char switches
  {
   switch(toupper(param[0]))
   {
     case 'S':
        options->bRecurse=TRUE;
        return NO_ERROR;
     case 'D':
        options->bNoUpdateDirs=FALSE;
        return NO_ERROR;
     case 'P':
        options->bPause=TRUE;
        return NO_ERROR;
     case 'Q':
        options->bQuiet=TRUE;
        return NO_ERROR;
     case 'E':
        options->bNoErrors=TRUE;
        return NO_ERROR;
   };
   return 1; //@todo fix
  } else {
   switch(toupper(param[0]))
   {
     case 'A':
        options->bAttributeSelect=TRUE;
        return ParseFilter(filterflags, param, len);
   };
   return 1; //@todo fix
  };
   return 1; //@todo fix
 };

 for (iCounter=0;iCounter<len;iCounter++)
 {
  cParamChar=toupper(param[iCounter]);

  /* recognize param char and select flag field */
  switch (cParamChar)
  {
    case ATR_CHAR_U: // Skip underline character
     break;
    case ATR_CHAR_R:
      bCurrentFlag=&(flags->bReadOnly);
     break;
    case ATR_CHAR_A:
      bCurrentFlag=&(flags->bArchived);
     break;
    case ATR_CHAR_S:
      bCurrentFlag=&(flags->bSystem);
     break;
    case ATR_CHAR_H:
      bCurrentFlag=&(flags->bHidden);
     break;
    default:
     return 1; //@todo fix
  }; /* END: switch (cParamChar) */

  /* perform requested action */
  switch(cActionType)
  {
   case PARAM_SET:
     *bCurrentFlag=ATR_ACTION_SET;
    break;
   case PARAM_CHAR2: /* '-' */
     *bCurrentFlag=ATR_ACTION_REM;
    break;
   case PARAM_NEG:
     *bCurrentFlag=ATR_ACTION_NEG;
    break;
   default:
    return 1; //@todo fix
  }; /* END: switch(cActionType) */
 }; /* END: for (iCounter=0;iCounter<len;iCounter++) */

 options->bFlagsSet=TRUE;

 return NO_ERROR;
};

/*!
   Modifies given file attributes according to given flags (attribute
   modification action codes)

  @param file   file name, which attributes should be modified
  @param flags  pointer to ATTRIB structure containing, attribute
                modification action codes
  @return
    return code from DosQueryPathInfo or DosSetPathInfo
*/
int ModifyFileAttribs(char *file,ATTRIB *flags)
{
#ifdef __386__
 FILESTATUS3 fileStatus;
#else
 FILESTATUS fileStatus;
#endif
 APIRET rc;

#ifdef __386__
 rc=DosQueryPathInfo(file,FIL_STANDARD,&fileStatus,sizeof(fileStatus));
#else
 rc=DosQPathInfo(file,FIL_STANDARD,(PBYTE)&fileStatus,sizeof(fileStatus),0);
#endif

 if (rc) return rc;

  if ((flags->bReadOnly==ATR_ACTION_IGN) &&
      (flags->bHidden==ATR_ACTION_IGN)   &&
      (flags->bSystem==ATR_ACTION_IGN)   &&
      (flags->bArchived==ATR_ACTION_IGN))
  {
    if ((fileStatus.attrFile&FILE_READONLY)==FILE_READONLY) printf("R"); else printf("_");
    if ((fileStatus.attrFile&FILE_HIDDEN)==FILE_HIDDEN) printf("H"); else printf("_");
    if ((fileStatus.attrFile&FILE_SYSTEM)==FILE_SYSTEM) printf("S"); else printf("_");
    if ((fileStatus.attrFile&FILE_ARCHIVED)==FILE_ARCHIVED) printf("A"); else printf("_");
    if ((fileStatus.attrFile&FILE_DIRECTORY)==FILE_DIRECTORY) printf("D"); else printf("_");
    printf(" %s\n", file);
  } else {


 /* modify it's attributes according to flags */

 /* Hidden */
 MODIFYATTRS(flags->bHidden,FILE_HIDDEN);

 /* Archived */
 MODIFYATTRS(flags->bArchived,FILE_ARCHIVED);

 /* System */
 MODIFYATTRS(flags->bSystem,FILE_SYSTEM);

 /* Readonly */
 MODIFYATTRS(flags->bReadOnly,FILE_READONLY);

#ifdef __386__
 rc=DosSetPathInfo(file,FIL_STANDARD,&fileStatus,sizeof(fileStatus),
      DSPI_WRTTHRU);
#else
 rc=DosSetPathInfo(file,FIL_STANDARD,(PBYTE)&fileStatus,sizeof(fileStatus),
      DSPI_WRTTHRU, 0);
#endif
 }

 return rc;
};

/*!
  Action function, to be called upon every file (and/or dir) found by
  all_PerformRecursiveAction function. If action codes match, performs
  modification of given dir/file using given data - attributes modification
  codes

  @param path     - path to given file (dir) name
  @param file     - file (dir) name
  @param iAction  - action code (can be one of all_RECURSE_*, but we support
                    only all_RECURSE_FILEACTION and all_RECURSE_DIRACTION)
  @param data     - pointer to ATTRIB data structure, containing attribute
                    action modification codes
  @return
     NO_ERROR - completed succesfully
     1 - an error occured and calling function should break it's execution
*/
int RecurseActionFunction(char *path,char*file,int iAction,void *data)
{
  APIRET rc=0;
  CHAR fullname[CCHMAXPATH]="";
  ATTRIB *flags=(ATTRIB *)data;

  /* we support only files actions and directories here */
  if ((iAction&all_RECURSE_FILEACTION)||(iAction&all_RECURSE_DIRACTION))
  {
    strcpy(fullname,path);
    strcat(fullname,file);

    rc=ModifyFileAttribs(fullname,flags);

    /* and since we have result handling function already (for recusive actions),
       we can use it here as well */
    return (RecurseErrorFunction(rc,NULL));
   };

  return NO_ERROR;
};

/*!
  Selected Dos* api return codes handling function

  @param rc    error return code, to be handled
  @param data  ignored, pass whatever (NULL is the best ;))

  @return
    NO_ERROR - program (calling function) should not break execution
    1 - program (calling function) should break execution
*/
int RecurseErrorFunction(ULONG rc,void *data)
{
  switch (rc)
  {
    /* these do not break programs execution */
    case NO_ERROR: break;
    case ERROR_FILE_NOT_FOUND: /* 2 */
    case ERROR_PATH_NOT_FOUND: /* 3 */
    case ERROR_SHARING_VIOLATION: /*32 */
     /*if (!options->bQuiet)*/ cmd_ShowSystemMessage(rc,0L); //@todo fix
     rc=0;
    default:
      /* all other errors go there, and they do break */
     /*if (!options->bQuiet)*/ cmd_ShowSystemMessage(rc,0L); //@todo fix
  }; /* END: switch (rc) */

  if (rc) return 1; // break master's execution
  return NO_ERROR;
};
