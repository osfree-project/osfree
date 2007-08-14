/*!
   $Id: all_performrecursiveaction.c,v 1.1.1.1 2003/10/04 08:36:16 prokushev Exp $ 
  
   @file all_performreqursiveaction.c 
   
   @brief directory-tree recurse helper functions
   shared along all code

   (c) osFree Project 2002, <http://www.osFree.org>

   @author Bartosz Tomasik <bart2@asua.org.pl>
   @author Cristiano Guadagnino <criguada@tin.it>

   @todo allow passing 'requested file attributes'
   @todo add new action Codes/options:
        all_RECURSE_FILESFIRST - to process files first, then directories,
        recursion still will be made after these
   @todo add new errors - 
         when user requested file/dir action but no files/dirs were present
         even throught recursion! or... should he check that himself (as there
       are already no_subdir no_files types callbacks)
*/   

#define INCL_DOSERRORS
#include <osfree.h>
#include <all_shared.h> /* shared functions and defines */

/*!
    @brief State of the dynamic stack

    This is the structure that holds the state of the dynamic stack.
    We're using a dynamic stack to avoid problems with out of stack
    space errors when recursing into subdirectories.
*/
typedef struct __all_traverse_state {
    char start[CCHMAXPATH];     //!< Where we start looking for subdirs
    phList dirs;                //!< Pointer to the list of subdirectories
    ULONG num_dirs;             //!< Number of subdirectories (excl files)
    ULONG idx_dirs;             //!< Index to enumerate the directories
} _all_traverse_state;


/*!
  Performs search for files/directories and fills up phlDirsList with results,
  note: this internal function used by all_PerformRecursiveAction
  Order of resulting data in list:
  1. directories matching mask (if requested by all_RECURSE_DIRACTION file_options)
  2. files matching mask (if requested by all_RECURSE_FILEACTION file_options)
  3. directories for rucursion (if requested by all_RECURSE_DIRS via file_options)

  @param dir           directory to start searching from
  @param file          filemask
  @param phlDirsList   Pointer to hList to be filled up, by function, note
                       that functions first puts directories into list, and
                       then files
  @param file_options  combination of all_RECURSE_* constants (use |), if 
                       all_RECURSE_DIRS or all_RECURSE_DIRACTION are present,
                       function will search for directories, if
                       all_RECURSE_FILEACTION is present, function will search
                       for files
  @param ulSearchAttr  file attributes (FILE_* combination), to match when
                        searhing for files and directories, note that
                        presence of FILE_DIRECTORY has no effect 
                        (it's overriden by function, while processing)
  @param ulDirCount    count of directories returned in phlDirList
  @param ulFileCount   count of files returned in phlDirList
  @param ulRecurseDirCount count of directories to be recursed returned in phlDirList

  @return
         NO_ERROR - if completed succesfully
         ERROR_NOT_ENOUGH_MEMORY - when memory allocation fails
         rc from DosFindFirst if it's error occurs (there is an exception from
          this as we do not return ERROR_NO_MORE_FILES)

  @todo allow passing 'requested attributes'
*/
int _all_CollectSubdirs(char *dir,char *file, hList *phlDirsList, 
                        int file_options,int ulSearchAttr, ULONG *ulDirCount,
                        ULONG *ulFileCount, ULONG *ulRecurseDirCount)
{
  HDIR hDir;
  FILEFINDBUF3 ffbFindBuf;
  APIRET rc; 
  CHAR *pchDirElemt;
//  ULONG ulSearchAttr=FILE_SYSTEM|FILE_READONLY|FILE_HIDDEN|FILE_ARCHIVED;
  ULONG ulEntries; 
  CHAR fileMask[CCHMAXPATH]="";

  /* mask for searching directories, and/or files, when all_RECURSE_DIRACTION,
     and/or all_RECURSE_FILEACTION are present */
  strcat(fileMask,dir); 
  strcat(fileMask,file);

  *ulDirCount=0; /* reset dir, recurse dir and file count */
  *ulFileCount=0;
  *ulRecurseDirCount=0;

 /* if we should match directories against filemask */
 if (file_options & all_RECURSE_DIRACTION)
 {
  hDir=HDIR_CREATE;
  ulEntries = 1L;

  rc = DosFindFirst(fileMask,                  /* all dirs */
                    &hDir,                  /* handle */
                    MUST_HAVE_DIRECTORY|ulSearchAttr, /* return only directories */
                    &ffbFindBuf,            /* buffer */
                    sizeof(FILEFINDBUF3),   /* size of buffer */
                    &ulEntries,             /* number of entries (1) */
                    FIL_STANDARD);          /* return only standard info (no EAs) */



    if ((rc != NO_ERROR) && (rc != ERROR_NO_MORE_FILES)) 
    {
     DosFindClose(hDir);
     return(rc);
    }


    if (rc != ERROR_NO_MORE_FILES) {
        do {
                // allocate memory for a new list element
                pchDirElemt = malloc(strlen(ffbFindBuf.achName) + 1);
                if (pchDirElemt == NULL)
                   return ERROR_NOT_ENOUGH_MEMORY;

                // save the name to the new list element
                strcpy(pchDirElemt, ffbFindBuf.achName);

                // add the new element to the list
                rc = list_add(phlDirsList, pchDirElemt);

                //increase directory count
                *ulDirCount+=1;
        } while (DosFindNext(hDir, &ffbFindBuf, sizeof(FILEFINDBUF3), &ulEntries) != ERROR_NO_MORE_FILES);
    }

    DosFindClose(hDir);
  }; /* END:  if (file_options & all_RECURSE_DIRACTION)  */

    if (file_options & all_RECURSE_FILEACTION) 
    {
      ulEntries=1;
      hDir = HDIR_CREATE;

      rc = DosFindFirst(fileMask,                  /* all dirs */
                        &hDir,                  /* handle */
                        ulSearchAttr&DONT_HAVE_DIRECTORY,                  /* no dirs, maybe readonly and/or archived files */
                            &ffbFindBuf,            /* buffer */
                            sizeof(FILEFINDBUF3),   /* size of buffer */
                            &ulEntries,             /* number of entries (1) */
                            FIL_STANDARD);          /* return only standard info (no EAs) */

    if ((rc != NO_ERROR) && (rc != ERROR_NO_MORE_FILES)) 
    {
     DosFindClose(hDir);
     return(rc);
    }


    if (rc != ERROR_NO_MORE_FILES) {
        do {
   
         // allocate memory for a new list element
            pchDirElemt = malloc(strlen(ffbFindBuf.achName) + 1);
            if (pchDirElemt == NULL)
               return ERROR_NOT_ENOUGH_MEMORY;

            // save the name to the new list element
            strcpy(pchDirElemt, ffbFindBuf.achName);

            // add the new element to the list
            rc = list_add(phlDirsList, pchDirElemt);

            //increase file count
            *ulFileCount+=1;
        } while (DosFindNext(hDir, &ffbFindBuf, sizeof(FILEFINDBUF3), &ulEntries) != ERROR_NO_MORE_FILES);
    };
          DosFindClose(hDir);

    }; /* END: if (file_options & all_RECURSE_FILEACTION) */


 /* if we should match directories against filemask */
 if (file_options & all_RECURSE_DIRS)
 {
  /* prepare new mask, for directory recursing */
    memset(fileMask,0,sizeof(fileMask)); 
   strcat(fileMask,dir); 
   strcat(fileMask,"*");


  hDir=HDIR_CREATE;
  ulEntries = 1L;
  rc = DosFindFirst(fileMask,                  /* all dirs */
                    &hDir,                  /* handle */
                    MUST_HAVE_DIRECTORY|ulSearchAttr, /* return only directories */
                    &ffbFindBuf,            /* buffer */
                    sizeof(FILEFINDBUF3),   /* size of buffer */
                    &ulEntries,             /* number of entries (1) */
                    FIL_STANDARD);          /* return only standard info (no EAs) */



    if ((rc != NO_ERROR) && (rc != ERROR_NO_MORE_FILES)) 
    {
     DosFindClose(hDir);
     return(rc);
    }


    if (rc != ERROR_NO_MORE_FILES) {
        do {
            if ((strcmp(ffbFindBuf.achName, ".") != 0)&&
                (strcmp(ffbFindBuf.achName, "..") != 0))
            {
                // allocate memory for a new list element
                pchDirElemt = malloc(strlen(ffbFindBuf.achName) + 1);
                if (pchDirElemt == NULL)
                   return ERROR_NOT_ENOUGH_MEMORY;

                // save the name to the new list element
                strcpy(pchDirElemt, ffbFindBuf.achName);

                // add the new element to the list
                rc = list_add(phlDirsList, pchDirElemt);

                //increase directory count
                *ulRecurseDirCount+=1;
            }
        } while (DosFindNext(hDir, &ffbFindBuf, sizeof(FILEFINDBUF3), &ulEntries) != ERROR_NO_MORE_FILES);
    }

    DosFindClose(hDir);
  }; /* END:  if (file_options & all_RECURSE_DIRS)  */

    return(NO_ERROR);
};



/*!
  Perform user action on files and/or directories, optionally traversing
  through sub-directories.  

  Note: This is  not a recursive implementation. It rather uses a dynamic 
  stack to avoid problems with out-of-stack-space conditions. It is faster, too.

  @param fileMask      file mask for file and/or dir searching, if 
                       all_RECURSE_DIRS is specified, '*' filemask will be used
                       for directories, otherwise the same as for files
  @param actionOptions action options and callback enablement, combination of
                       all_RECURSE_* constants (use |), meaning of respective
                       constants:
                       all_RECURSE_FILEACTION - action callback will be called
                        on every file (this does not include directories!) that 
                        match given fileMask (action_callback 'path' parameter
                        contains path part of that file - where this file 
                        resides in, and 'name' it's name)
                       all_RECURSE_DIRACTION - action callback will be called
                        on every directory (this does not include files!) that 
                        match given fileMask (action_callback 'path' parameter
                        contains path part of that directory - where this
                        directory resides in, and 'name' it's name)
                       all_RECURSE_DIRS - all actions (if any selected) will
                        be made for every subdirectory starting from a given
                        in fileMask (recursing into subdirectories)
                       all_RECURSE_IN - action callback will be called upon
                        entering new directory (action_callback 'path' 
                        parameter contains, contains entered directory name,
                        and 'name' parameter file-mask to be used for
                        matching files and/or directories)
                       all_RECURSE_NOFILES - action callback will be called
                        when user requested actions on files 
                        (all_RECURSE_FILEACTION) but no files mathing mask part
                        of file mask, were found in currently-searched 
                        directory (action_callback 'path' parameter contains 
                        that directory name, and 'name' parameter used mask)
                       all_RECURSE_NODIRS - action callback will be called
                        when user requested actions on directories
                        (all_RECURSE_DIRACTION) but no directories mathing mask
                        part of file mask, were found in currently-searched 
                        directory (action_callback 'path' parameter contains 
                        that directory name, and 'name' parameter used mask)
                       all_RECURSE_NOSUBDIRS - action callback will be
                        called when user selected to recurse into directories
                        (all_RECURSE_DIRS) but no subdirectories were found in
                        currently-searched directory (action_callback 'path' 
                        parameter contains, contains currently-searched 
                        directory name, and 'name' parameter file-mask to be
                        used for matching files and/or directories)
                        Note: this callback is called only for 2nd and more
                        depth directories, if such situation happens in 1st 
                        depth directory, function returns (after processing
                        files and/or directories) with return code
                        all_ERR_RECURSE_NORECURSION

  @param fileAttrs      file attributes (FILE_* combination), to match when
                        searhing for files and directories, note that
                        presence of FILE_DIRECTORY has no effect 
                        (it's overriden by function, while processing)

  @param action_callback action_callback function, to be called upon files and
                         directories or after condition (look above) has
                         occured. As a parameters it gets, file (directory)
                         name, upon wich is called, fileMask and action type.
                         To break function (all_PerformRecursiveAction) 
                         execution, it should return non 0 value.
                         Simplest example of action callback function:

       int ActionFunc(char *path, char *name,char action,void *data)
       {
         switch (action) 
         {
           case all_RECURSE_DIRACTION:
            printf("Directory: %s in path %s\n",name,path);
            break;
           case all_RECURSE_FILEACTION:
            printf("File: %s in path %s\n",name,path);
            break;
         };
         return 0;
       };

  @param action_callback_data pointer, for additional data, that should be
                              passed to action_callback function, upon
                              execution

  @param error_callback  error callback function, to be called when 
                        _all_CollectSubdirs returns an error, as a parameter
                        it's return code, should return non 0 value, to break
                        function (all_PerformRecursiveAction) execution.
                        Simplest example of error callback function:
	
	int ErrorFunc(ULONG rc,void *data)
        {
          if (rc>5) return 1; //break owner execution
          return 0;
        };

  @param error_callback_data pointer, for additional data, that should be
                             passed to action_callback function, upon execution
  @return
         NO_ERROR - if completed succesfully
         all_ERR_RECURSE_ACTIONBREAK - when broke by action_callback
         all_ERR_RECURSE_ERRORBREAK  - when broke by error_callback
         all_ERR_RECURSE_BADPARAMS   - when bad parameters were passed
         all_ERR_RECURSE_NORECURSION - no directories to recursion were found
                                       at first deep level

  @todo allow passing 'requested attributes'
*/
int all_PerformRecursiveAction(char *fileMask,int actionOptions,int fileAttrs,
            int (*action_callback)(char*,char*,char,void *),void *action_callback_data,
            int (*error_callback)(ULONG,void *),void *error_callback_data)
{
    hList          *phlDirsList;    /* dynamic list of directories      */
    hStack         *phsStack;       /* dynamic stack                    */
    _all_traverse_state *ptsState;       /* state to save on the dyn stack   */
    ULONG           ulLevel = 1L;   /* depth level */
    ULONG ulDirs, ulRecurseDirs, ulFiles;
    ULONG ulrc, ulIdx;
    ULONG ulCounter;
    CHAR dir[CCHMAXPATH] = ""; 
    char *mask;
    APIRET rc;
    ULONG ulEntries;
    char *tmp;

    /* check are the parameters correct */ 
    if ((actionOptions==0)||(action_callback==NULL))
     return all_ERR_RECURSE_BADPARAMS; 

    // Get a new dynamic stack handle
    phsStack = stack_init();    /* @todo - check return code */

     mask=all_GetFileFromPath(fileMask);
     tmp=all_GetDirFromPath(fileMask);

     strcpy(dir,tmp);

     free(tmp);

lbl_begin:
    // Get a new dynamic list handle
    phlDirsList = list_init();  /* @todo - check return code */

    /* call call-back only, when user requested that */
    if (actionOptions&all_RECURSE_IN)
    {
     if (action_callback(dir,mask,all_RECURSE_IN,action_callback_data)!=0)
     {
      /* @todo cleanup.. and set correct return value*/
      free(mask);
      return all_ERR_RECURSE_ACTIONBREAK; 
     };
    };

    // Collect subdirs and store them in the list
    rc=_all_CollectSubdirs(dir,mask, phlDirsList, actionOptions,fileAttrs,
                          &ulDirs,&ulFiles,&ulRecurseDirs);

    if ((rc) && (error_callback!=NULL))
      {
        if (error_callback(rc,error_callback_data)!=0)
        {
         //! @todo: stack and list cleanup
         free(mask);
         return all_ERR_RECURSE_ERRORBREAK;
        };
      };

    list_set_start(phlDirsList);



    /* if user requested actions on files */
    if (actionOptions&all_RECURSE_DIRACTION) 
    {
     if (ulDirs==0) /* but there may be no files */
     {
       /* execute action callback only if requested */
       if (actionOptions&all_RECURSE_NODIRS)
       {
         /* execute action callback */
         if (action_callback(dir,mask,all_RECURSE_NODIRS,
                             action_callback_data)!=0)
         {
          /* @todo cleanup.. and set correct return value */
          free(mask);
          return all_ERR_RECURSE_ACTIONBREAK; 
         };
       };

     } else /* if (ulDirs==0) */
     {
       /* if we are here, user requested actions on dirs and there are dirs!*/

       for (ulCounter=0;ulCounter<ulDirs;ulCounter++)
       {
         if (action_callback(dir,list_get(phlDirsList),
                           all_RECURSE_DIRACTION,action_callback_data)!=0)
         {
          /* @todo cleanup.. and set correct return value */
          free(mask);
          return all_ERR_RECURSE_ACTIONBREAK; 
         };
        list_next(phlDirsList);
       };
     };  /*END: if (ulDirs==0) */
    }; /* END: if (actionOptions&all_RECURSE_FILEACTION) */


    /* if user requested actions on files */
    if (actionOptions&all_RECURSE_FILEACTION) 
    {
     if (ulFiles==0) /* but there may be no files */
     {
       /* execute action callback only when requested*/
       if (actionOptions&all_RECURSE_NOFILES)
       {
         if (action_callback(dir,mask,all_RECURSE_NOFILES,
                             action_callback_data)!=0)
         {
          /* @todo cleanup.. and set correct return value */
          free(mask);
          return all_ERR_RECURSE_ACTIONBREAK; 
         };
       };

     } else /* if (ulFiles==0) */
     {
       /* if we are here, user requested actions on files and there are files!*/

       for (ulCounter=0;ulCounter<ulFiles;ulCounter++)
       {
         if (action_callback(dir,list_get(phlDirsList),
                           all_RECURSE_FILEACTION,action_callback_data)!=0)
         {
          /* @todo cleanup.. and set correct return value */
          free(mask);
          return all_ERR_RECURSE_ACTIONBREAK; 
         };
        list_next(phlDirsList);
       };
     }; /*END: if (ulFiles==0) */
    }; /* END: if (actionOptions&all_RECURSE_FILEACTION) */
  
    /* if no real recurse was requested quit without error */
    if (!(actionOptions&all_RECURSE_DIRS))
    {
      //! @todo list and stack cleanup
      return NO_ERROR;
    };

    /* but if it was, and we're at level 1, we need to check are there any
       directories, and if not, return immediatly */
    if ((ulLevel==1) && (ulRecurseDirs==0))
      return all_ERR_RECURSE_NORECURSION;

    if ((list_isempty(phlDirsList)) || (ulRecurseDirs == 0)) {
        // End condition #1: no subdirs

   /* execute callback, only when  requested by user */
    if (actionOptions&all_RECURSE_NOSUBDIRS)
    {
       if (action_callback(dir,mask,all_RECURSE_NOSUBDIRS,
                           action_callback_data)!=0)
       {
        /* @todo cleanup.. and set correct return value */
        free(mask);
        return all_ERR_RECURSE_ACTIONBREAK; 
       };
    };

        goto lbl_recur_back;
    } else {
        // recur in each directory
        for (ulIdx = 1; ulIdx <= ulRecurseDirs; ulIdx++) {
            goto lbl_recur;

lbl_continue:
            if (ulLevel < 1) return(0);
            list_next(phlDirsList);
        }

        // End condition #2: no more subdirs
        goto lbl_recur_back;
    }

//    printf("returning from continue\n");

    free(mask);
    return(NO_ERROR); // what's this?

lbl_recur:
    // Maintain the dynamic stack
    ptsState = malloc(sizeof(_all_traverse_state));
    strcpy(ptsState->start, dir);
    ptsState->dirs = phlDirsList;
    ptsState->num_dirs = ulRecurseDirs;
    ptsState->idx_dirs = ulIdx;
    stack_push(phsStack, ptsState);

    strcat(dir, list_get(phlDirsList));

    if (dir[strlen(dir) - 1] != '\\') strcat(dir, "\\");

    // we go one level deeper
    ulLevel++;
    goto lbl_begin;

lbl_recur_back:
    // Maintain the dynamic stack
    ulLevel--;
    stack_pop(phsStack, &ptsState);
    if(ptsState != NULL) {
        strcpy(dir, ptsState->start);
        if (dir[strlen(dir) - 1] != '\\') strcat(dir, "\\");
        list_free(phlDirsList);
        phlDirsList = ptsState->dirs;
        ulRecurseDirs = ptsState->num_dirs;
        ulIdx = ptsState->idx_dirs;
        free(ptsState);
    }
    goto lbl_continue;
};

