/********************************
*
* REName MODULe
* Renames OS/2 modules (EXE or DLL)
* Partial rewrite by Aaron Lawrence
* From RNDLL By Harald Pollack
*
*
* 1989 08 23 h.p.
* 1990 11 19 h.p.
* 1997 05 24 h.p.
* 1999 09 08 h.p.
*
********************************/

#include <stdio.h>
#include <string.h>

#include "renmodul.h"

/* pack structures! */
#pragma pack(1)

/* Module header structure */
typedef struct
{
  /* sign1, 2 */  /* 00 */
  unsigned short partpag; /* 02 */
  unsigned short pagecnt; /* 04 */
  unsigned short relocnt; /* 06 */
  unsigned short hdrsize; /* 08 */
  unsigned short minmem; /* 10 */
  unsigned short maxmem; /* 12 */
  unsigned short reloss; /* 14 */
  unsigned short exesp; /* 16 */
  unsigned short chksum; /* 18 */
  unsigned short exeip; /* 20 */
  unsigned short relocs; /* 22 */
  unsigned short tabloff; /* 24 Should be 64 (0x40) for compatibility header */
  unsigned short overlay; /* 26 */
  unsigned short unknown[16]; /* 28 */
  unsigned long LXHeaderOffset; /* 60 */
} MZ_HEADER;

/* LX (new) header */
typedef struct
{
  char unk1[6];
  unsigned short cpu_type;
  unsigned short os_type; /* should be 1 for OS/2 */
  unsigned long modul_vers;
  unsigned long modul_flag;
  char unk2[68];
  unsigned long ResidentNameTableOffset; /* First in table is module name */
  unsigned long EntryTableOffset;
  unsigned long ModuleDirectivesOffset;
  unsigned long ModuleDirectivesCount;
  unsigned long FixupPageTableOffset;
  unsigned long FixupRecordTableOffset;
  unsigned long ImportModuleTableOffset;
  unsigned long ImportModuleTableCount;
  /* plus more stuff we don't care about */
} LX_HEADER;

int error( int rc, char* message, FILE* f )
{
  fclose( f );
  printf( "Error: %s\n", message );
  return rc;
}

/* _System == APIENTRY */
int _System RenameModule( char* Filename,
                          RenameAction action,
                          char* OldModuleName,
                          char* NewModuleName )
{
  MZ_HEADER MZHeader;
  LX_HEADER LXHeader;
  FILE* f;

  char sign[ 2 ];
  int ModuleNameLen;
  char ModuleName[ 256 ];
  int LXHeaderOffset;
  int ImportNameIndex;
  int NumImportNamesChanged;
  long ImportNameOffset;

  if ( action == RENAME_MODULE || action == RENAME_IMPORTED_MODULE )
  {
    /* validate parameters */
    if ( strlen( OldModuleName ) == 0 || strlen( NewModuleName ) == 0 )
      return RC_PARAM_ERROR;

    if ( strlen( OldModuleName ) != strlen( NewModuleName ) )
    {
      printf( "Error: Length of new module name must be same as old\n" );
      return RC_NAME_LENGTHS_DIFFERENT;
    }

    if ( stricmp( OldModuleName, NewModuleName ) == 0 )
    {
      printf( "Error: New module name same as old\n" );
      return RC_PARAM_ERROR;
    }
  }

  /* Open file for binary read and write  */
  f = fopen( Filename, "r+b" );
  if ( f == NULL )
  {
    printf( "Error: Unable to open file %s\n", Filename );
    return RC_FILE_ERROR;
  }

  memset( sign, 0, sizeof( sign ) );
  if ( fread( sign, sizeof( sign ), 1, f ) != 1 )
    return error( RC_INVALID_FORMAT, "Reading signature", f );

  LXHeaderOffset = 0;

  if ( sign[ 0 ] == 'M' && sign[ 1 ] == 'Z' )
  {
    /* Old DOS header - LX header comes later */
    if ( fread( & MZHeader, sizeof( MZHeader ), 1, f ) != 1 )
      return error( RC_INVALID_FORMAT, "Reading MZ Header", f );

    if ( MZHeader.tabloff != 0x40 )
      return error( RC_INVALID_FORMAT, "DOS Module", f );

    /* seek to LX Header */
    LXHeaderOffset = MZHeader.LXHeaderOffset;
    if ( fseek( f, LXHeaderOffset, SEEK_SET ) != 0 )
      return error( RC_INVALID_FORMAT, "Seeking LX Header", f );

    /* read LX sign bytes */
    if ( fread( sign, sizeof( sign ), 1, f ) != 1 )
      return error( RC_INVALID_FORMAT, "Reading LX signature", f );

  }

  /* Check LX header signature */
  if ( sign[ 0 ] != 'L' || sign[ 1 ] != 'X' )
    return error( RC_INVALID_FORMAT, "Module format not recognised", f );

  /* Read LX Header */
  if ( fread( & LXHeader, sizeof( LXHeader ), 1, f ) != 1 )
    return error( RC_INVALID_FORMAT, "Reading LX Header", f );

  if ( LXHeader.os_type != 1 )
    return error( RC_WRONG_FORMAT, "Not an OS/2 module", f );

  switch( action )
  {
  case RENAME_MODULE:
    printf( "Change module name\n" );
    break;

  case RENAME_IMPORTED_MODULE:
    printf( "Changing import names\n" );
    break;

  case LIST_NAMES:
    printf( "Listing names\n" );
    break;
  }

  if ( action == RENAME_MODULE || action == LIST_NAMES )
  {
    /* Get the file's current module name */
    if ( fseek( f, LXHeaderOffset + LXHeader.ResidentNameTableOffset, SEEK_SET ) != 0 )
      return error( RC_INVALID_FORMAT, "Seeking module name", f );

    if ( fread( & ModuleNameLen, sizeof( ModuleNameLen ), 1, f ) != 1 )
      return error( RC_INVALID_FORMAT, "Reading module name", f );

    if ( ModuleNameLen < 1 || ModuleNameLen > 8 )
      return error( RC_INVALID_FORMAT, "Current module name has invalid length!", f );

    if ( fread( & ModuleName, ModuleNameLen, 1, f ) != 1 )
      return error( RC_INVALID_FORMAT, "Reading module name", f );

    /* Zero terminate */
    ModuleName[ ModuleNameLen ] = 0;

    printf( "Module name: %s\n", ModuleName );

    if ( action == RENAME_MODULE )
    {
      if ( stricmp( ModuleName, OldModuleName ) != 0 )
        return error( RC_NAME_MISMATCH, "Specified old name does not match actual module name", f );
      /* update name */
      /* Seek back to start of module name, plus len byte */
      if ( fseek( f, LXHeaderOffset + LXHeader.ResidentNameTableOffset + 1, SEEK_SET ) != 0 )
        return error( RC_WRITE_ERROR, "Seeking module name", f );

      if ( fwrite( NewModuleName, ModuleNameLen, 1, f ) != 1 )
        return error( RC_WRITE_ERROR, "Writing new module name", f );

      printf( "Done: Module name changed to %s\n", NewModuleName );
    }
  }

  if ( action == RENAME_IMPORTED_MODULE || action == LIST_NAMES )
  {
    if ( fseek( f, LXHeaderOffset + LXHeader.ImportModuleTableOffset, SEEK_SET ) != 0 )
      return error( RC_INVALID_FORMAT, "Seeking import module name table", f );

    NumImportNamesChanged = 0;
    ImportNameIndex = 0;

    while ( ImportNameIndex < LXHeader.ImportModuleTableCount )
    {
      /* Read import name length byte */
      ImportNameOffset = ftell( f );

      if ( fread( & ModuleNameLen, sizeof( ModuleNameLen ), 1, f ) != 1 )
        return error( RC_INVALID_FORMAT, "Reading import name", f );
      /* read import name */
      if ( fread( ModuleName, ModuleNameLen, 1, f ) != 1 )
        return error( RC_INVALID_FORMAT, "Reading import name", f );

      /* zero terminate */
      ModuleName[ ModuleNameLen ] = 0;
      printf( "  Import: %s\n",
              ModuleName );
      if ( action == RENAME_IMPORTED_MODULE )
      {
        if ( stricmp( ModuleName, OldModuleName ) == 0 )
        {
          printf( "    Found match, changing to %s\n",
                  NewModuleName );

          /* seek back to start of name, plus the length byte */
          if ( fseek( f, ImportNameOffset + 1, SEEK_SET ) != 0 )
            return error( RC_WRITE_ERROR, "Seeking module name", f );

          if ( fwrite( NewModuleName, ModuleNameLen, 1, f ) != 1 )
            return error( RC_WRITE_ERROR, "Writing new module name", f );

          /* After fwrite file pointer seems to go somewhere random!!? */
          if ( fseek( f, ImportNameOffset + 1 + ModuleNameLen, SEEK_SET ) != 0 )
            return error( RC_WRITE_ERROR, "Seeking next module name", f );

          NumImportNamesChanged ++;
        }
      }

      ImportNameIndex ++;
    }

    if ( action == RENAME_IMPORTED_MODULE )
    {
      if ( NumImportNamesChanged == 0 )
        printf( "No import of '%s' was found, no changes made\n",
                OldModuleName );
      else
        printf( "Done: %d import names changed\n",
                NumImportNamesChanged );
    }
    else
    {
      printf( "Done: %d import names listed\n", ImportNameIndex );
    }
  }

  fclose( f );

  return( 0 );
}
