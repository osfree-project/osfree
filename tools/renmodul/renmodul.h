#ifndef RENMODUL_H
#define RENMODUL_H

#define VERSION "v2.0.0"

#define RC_PARAM_ERROR 1
#define RC_NAME_LENGTHS_DIFFERENT 2
#define RC_FILE_ERROR 3
#define RC_INVALID_FORMAT 4
#define RC_WRONG_FORMAT 5
#define RC_NAME_MISMATCH 6
#define RC_WRITE_ERROR 7

typedef enum 
{
  RENAME_MODULE,
  RENAME_IMPORTED_MODULE,
  LIST_NAMES
} RenameAction;

#ifdef __GNUC__
#define _System
#define stricmp strcasecmp
#endif

/* _System == APIENTRY */
int _System RenameModule( char* Filename, 
			  RenameAction action,
			  char* OldModuleName, 
			  char* NewModuleName );

#endif
