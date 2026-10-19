/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  Input file list management and file-type detection.
*
****************************************************************************/

/*!
 * @file file.c
 * @brief Input file list management and file-type detection.
 *
 * Maintains the ordered list of files supplied on the command line,
 * classifies them by extension (or by an explicitly forced language),
 * and hands them out one at a time to the linker's subsystems.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "error.h"
#include "file.h"
#include "memory.h"
#include "pathconv.h"


/*!
 * @struct ListElem
 * @brief One node of the pending input file list.
 */
struct ListElem {
    int                 type;       /*!< File type constant. */
    char *              filename;   /*!< Name of the file to handle. */
    struct ListElem *   next;       /*!< Next item in the list. */
};

/*!
 * @brief Head of the pending input file list.
 */
static struct ListElem *fileList = NULL;

/*!
 * @brief Default file type used when the extension is unknown.
 */
static int              defaultType = TYPE_INVALID_FILE;
/*!
 * @brief Human-readable name for @ref defaultType.
 */
static const char *     defaultName = NULL;

/*!
 * @brief Non-zero when .c files are accepted as input.
 */
static int              allowC = 0;
/*!
 * @brief Non-zero when C++ source files are accepted as input.
 */
static int              allowCPP = 0;
/*!
 * @brief Non-zero when .def files are accepted as input.
 */
static int              allowDEF = 0;
/*!
 * @brief Non-zero when .lib files are accepted as input.
 */
static int              allowLIB = 0;
/*!
 * @brief Non-zero when .obj files are accepted as input.
 */
static int              allowOBJ = 0;
/*!
 * @brief Non-zero when .rc files are accepted as input.
 */
static int              allowRC = 0;
/*!
 * @brief Non-zero when .res files are accepted as input.
 */
static int              allowRES = 0;
/*!
 * @brief Non-zero when .rbj files are accepted as input.
 */
static int              allowRBJ = 0;
/*!
 * @brief Non-zero when .rs files are accepted as input.
 */
static int              allowRS = 0;
/*!
 * @brief Non-zero when .exp files are accepted as input.
 */
static int              allowEXP = 0;

/*!
 * @brief Language forced by the user, or FORCE_NONE.
 */
static int              languageToForce = FORCE_NONE;

/*!
 * @brief Examine a filename's extension to determine its file type.
 *
 * If the extension is not recognised and no default type has been
 * configured, aborts with Zoinks().  Otherwise issues a warning and
 * falls back to @ref defaultType.
 *
 * @param[in] filename  Filename, possibly quoted.  Not NULL.
 * @return One of the TYPE_* constants.
 */
static int file_type( const char *filename )
/******************************************/
{
    char *              newfilename;
    char *              tempfilename;
    char                ext[_MAX_EXT];
    int                 type;

    /*** Strip quotes from filename ***/
    newfilename = DupStrMem( filename );
    if( *newfilename == '"' ) {
        tempfilename = newfilename + 1;                 /* skip leading " */
        tempfilename[ strlen(tempfilename)-1 ] = '\0';  /* smite trailing " */
    } else {
        tempfilename = newfilename;
    }

    _splitpath( tempfilename, NULL, NULL, NULL, ext );
    if( allowC  &&  !stricmp( ext, ".c" ) ) {
        type = TYPE_C_FILE;
    } else if( allowCPP  &&  !stricmp( ext, ".cc" ) ) {
        type = TYPE_CPP_FILE;
    } else if( allowCPP  &&  !stricmp( ext, ".cpp" ) ) {
        type = TYPE_CPP_FILE;
    } else if( allowCPP  &&  !stricmp( ext, ".cxx" ) ) {
        type = TYPE_CPP_FILE;
    } else if( allowCPP  &&  !stricmp( ext, ".odl" ) ) {
        type = TYPE_CPP_FILE;
    } else if( allowCPP  &&  !stricmp( ext, ".idl" ) ) {
        type = TYPE_CPP_FILE;
    } else if( allowDEF  &&  !stricmp( ext, ".def" ) ) {
        type = TYPE_DEF_FILE;
    } else if( allowOBJ  &&  !stricmp( ext, ".obj" ) ) {
        type = TYPE_OBJ_FILE;
    } else if( allowLIB  &&  !stricmp( ext, ".lib" ) ) {
        type = TYPE_LIB_FILE;
    } else if( allowRC  &&  !stricmp( ext, ".rc" ) ) {
        type = TYPE_RC_FILE;
    } else if( allowRES  &&  !stricmp( ext, ".res" ) ) {
        type = TYPE_RES_FILE;
    } else if( allowRBJ  &&  !stricmp( ext, ".rbj" ) ) {
        type = TYPE_RBJ_FILE;
    } else if( allowRS  &&  !stricmp( ext, ".rs" ) ) {
        type = TYPE_RS_FILE;
    } else if( allowEXP  &&  !stricmp( ext, ".exp" ) ) {
        type = TYPE_EXP_FILE;
    } else {
        if( defaultType == TYPE_INVALID_FILE ) {
            Zoinks();
        }
        if( languageToForce == FORCE_NONE ) {
            Warning( "Unrecognized file type '%s' -- %s file assumed",
                     filename, defaultName );
        }
        type = defaultType;
    }
    if( type == TYPE_C_FILE && languageToForce == FORCE_CPP_COMPILE ) {
        type = TYPE_CPP_FILE;
    } else if( type == TYPE_CPP_FILE && languageToForce == FORCE_C_COMPILE ) {
        type = TYPE_C_FILE;
    }
    FreeMem( newfilename );
    return( type );
}


/*!
 * @brief Append a file to the pending input list.
 *
 * If @p type is TYPE_DEFAULT_FILE, file_type() is invoked to determine
 * the actual type from the filename's extension.
 *
 * @param[in] type      File type, or TYPE_DEFAULT_FILE.
 * @param[in] filename  Filename as supplied by the user.  Not NULL.
 */
void AddFile( int type, const char *filename )
/********************************************/
{
    struct ListElem *   newElem;
    struct ListElem *   curElem;
    char *              newfilename;

    /*** Initialize a new ListElem structure ***/
    newElem = AllocMem( sizeof(struct ListElem) );
    newfilename = PathConvert( filename, '"' );
    newElem->filename = newfilename;
    if( type == TYPE_DEFAULT_FILE ) {
        newElem->type = file_type( newfilename );
    } else {
        newElem->type = type;
    }
    newElem->next = NULL;

    /*** Add it to the end of the list (to maintain order) ***/
    if( fileList == NULL ) {
        fileList = newElem;
    } else {
        curElem = fileList;
        while( curElem->next != NULL )  curElem = curElem->next;
        curElem->next = newElem;
    }
}


/*!
 * @brief Remove and return the next file of one of the accepted types.
 *
 * Searches the pending list for the first entry whose type matches
 * @p type or any of the following types up to TYPE_INVALID_FILE.  If
 * @p typeBuf is non-NULL, the chosen entry's type is stored there.
 *
 * @param[out] typeBuf  Optional receiver for the chosen file's type.
 * @param[in]  type     First accepted file type.
 * @param[in]  ...      Additional accepted types, terminated by
 *                      TYPE_INVALID_FILE.
 * @retval NULL  No matching file remained.
 * @return The matching filename.  Caller must FreeMem() it.
 */
char *GetNextFile( int *typeBuf, int type, ... )
/**********************************************/
{
    int                 found = 0;
    struct ListElem *   curElem = fileList;
    struct ListElem *   prevElem = NULL;
    char *              filename;
    va_list             args;
    int                 curType;

    /*** Search the list for the file ***/
    while( curElem != NULL ) {
        if( curElem->type == type ) {   /* check the first type */
            found = 1;
            break;
        } else {                        /* check any other types */
            va_start( args, type );
            while( !found ) {
                curType = va_arg( args, int );
                if( curType == TYPE_INVALID_FILE )  break;
                if( curElem->type == curType ) {
                    found = 1;
                }
            }
            va_end( args );
            if( found )  break;
        }
        prevElem = curElem;
        curElem = curElem->next;
    }

    /*** Extract info if found, or return NULL if not ***/
    if( found ) {
        filename = curElem->filename;
        if( prevElem != NULL ) {
            prevElem->next = curElem->next;
        } else {
            fileList = curElem->next;
        }
        if( typeBuf != NULL )  *typeBuf = curElem->type;
        FreeMem( curElem );
    } else {
        filename = NULL;
    }
    return( filename );
}


/*!
 * @brief Set the default file type and its human-readable name.
 *
 * @param[in] type  TYPE_* constant used when the extension is unknown.
 * @param[in] name  Description used in the "unrecognized file type"
 *                  warning.
 */
void SetDefaultFile( int type, const char *name )
/***********************************************/
{
    defaultType = type;
    defaultName = name;
}


/*!
 * @brief Enable a set of file types for input.
 *
 * Each type named in the argument list sets its corresponding "allow"
 * flag.  TYPE_DEFAULT_FILE clears all of them.  TYPE_INVALID_FILE
 * terminates the list.
 *
 * @param[in] type  First type to enable or disable.
 * @param[in] ...   Additional types, terminated by TYPE_INVALID_FILE.
 */
void AllowTypeFile( int type, ... )
/*********************************/
{
    va_list             args;
    int                 alive = 1;
    int                 curType;
    int                 gotFirst = 0;

    va_start( args, type );
    while( alive ) {
        if( !gotFirst ) {
            curType = type;
            gotFirst = 1;
        } else {
            curType = va_arg( args, int );
        }
        switch( curType ) {
          case TYPE_DEFAULT_FILE:
            allowC = 0;
            allowCPP = 0;
            allowDEF = 0;
            allowOBJ = 0;
            allowLIB = 0;
            allowRC = 0;
            allowRES = 0;
            allowRBJ = 0;
            allowRS = 0;
            allowEXP = 0;
            break;
          case TYPE_C_FILE:
            allowC = 1;
            break;
          case TYPE_CPP_FILE:
            allowCPP = 1;
            break;
          case TYPE_DEF_FILE:
            allowDEF = 1;
            break;
          case TYPE_OBJ_FILE:
            allowOBJ = 1;
            break;
          case TYPE_LIB_FILE:
            allowLIB = 1;
            break;
          case TYPE_RC_FILE:
            allowRC = 1;
            break;
          case TYPE_RES_FILE:
            allowRES = 1;
            break;
          case TYPE_RBJ_FILE:
            allowRBJ = 1;
            break;
          case TYPE_RS_FILE:
            allowRS = 1;
            break;
          case TYPE_EXP_FILE:
            allowEXP = 1;
            break;
          case TYPE_INVALID_FILE:
            alive = 0;
            break;
          default:
            Zoinks();
        }
    }
    va_end( args );
}


/*!
 * @brief Force all files to be treated as a given language.
 *
 * Also updates @ref defaultType accordingly.
 *
 * @param[in] language  FORCE_C_COMPILE or FORCE_CPP_COMPILE.
 */
void ForceLanguage( int language )
{
    languageToForce = language;
    if( language == FORCE_C_COMPILE ) {
        defaultType = TYPE_C_FILE;
    } else if( language == FORCE_CPP_COMPILE ) {
        defaultType = TYPE_CPP_FILE;
    }
}
