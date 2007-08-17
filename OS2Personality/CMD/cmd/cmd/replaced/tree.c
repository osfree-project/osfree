/* Replaced by osFree team implementation

static long fTree;

#define TREE_ASCII      0x01
#define TREE_BARE       0x02
#define TREE_CDD        0x04
#define TREE_FILES      0x08
#define TREE_HIDDEN     0x10
#define TREE_PAGE       0x20
#define TREE_SIZE       0x40
#define TREE_TIME       0x80

// display a directory list or tree
int tree_cmd( int argc, char **argv )
{
        char *arg;
        char szFilename[MAXFILENAME], szSequelBuffer[MAXPATH];
        int rval = 0, fTime;

        szSequelBuffer[0] = '\0';
        init_dir();

        argv++;
        fTime = GetMultiCharSwitch( *argv, "T", szFilename, 4 );

        // check for and remove switches
        if ( GetSwitches( *argv, "ABCFHPS", &fTree, 0 ) != 0 )
                return ( usage( TREE_USAGE ));

        if ( fTime ) {

                fTree |= TREE_TIME;
                arg = szFilename;
                if ( *arg ) {
                        if ( *arg == ':' )
                                arg++;
                        if ( _ctoupper( *arg ) == 'A' )
                                gnDirTimeField = 1;
                        else if ( _ctoupper( *arg ) == 'C' )
                                gnDirTimeField = 2;
                }
        }

        // if no filename arguments, use current directory
        if ( first_arg( *argv ) == NULL ) {
                *argv = gcdir( NULL, 0 );
                AddQuotes( *argv );
        }

        if ( fTree & TREE_PAGE ) {
                gnPageLength = GetScrRows();
        }

        arg = _alloca( strlen( *argv ) + 1 );
        *argv = strcpy( arg, *argv );
        for ( argc = 0; (( arg = ntharg( *argv, argc )) != NULL ); argc++ ) {

                if ( mkfname( arg, 0 ) == NULL ) {
                        rval = ERROR_EXIT;
                        break;
                }

                if ( is_dir( arg ) == 0 ) {
                        rval = error( ERROR_PATH_NOT_FOUND, arg );
                        continue;
                }

                // display "c:\wonky..."
                _nxtrow();
                printf( FMT_STR, arg );
                _nxtrow();

                copy_filename( szFilename, arg );
                mkdirname( szFilename, WILD_FILE );
                if ((( rval = DrawTree( szFilename, szSequelBuffer )) != 0 ) || ( cv.exception_flag ))
                        break;
        }

        return rval;
}


// display a directory tree
static int DrawTree( char *pszCurrent, char *pszSequelBuffer )
{
        int rval = 0, fOK, nAttrib;
        unsigned int uMode = 0x9110;
        char *pszName, *pszSave, cAmPm = ' ';
        FILESEARCH dir;

        if ( fTree & TREE_HIDDEN )
                uMode |= 0x07;

        if (( fTree & TREE_FILES ) == 0 )
                uMode |= 0x200;

        fOK = ( find_file( FIND_FIRST, pszCurrent, uMode, &dir, NULL ) != NULL );

        while ( fOK ) {

                pszName = strdup( dir.name );
                nAttrib = dir.attrib;

                if ( fTree & TREE_SIZE ) {
                        printf( (( nAttrib & _A_SUBDIR ) ? DIR_LABEL : "%9lu" ), dir.size );
                        printf("  ");
                }

                if ( fTree & TREE_TIME ) {

                    int month, day, year, minutes, hours;
                    if ( gnDirTimeField == 0 ) {
                        day = dir.fdLW.fdateLastWrite.day;
                        month = dir.fdLW.fdateLastWrite.month;
                        year = dir.fdLW.fdateLastWrite.year;
                        hours = dir.ftLW.ftimeLastWrite.hours;
                        minutes = dir.ftLW.ftimeLastWrite.minutes;
                    } else if ( gnDirTimeField == 1 ) {
                        day = dir.fdLA.fdateLastAccess.day;
                        month = dir.fdLA.fdateLastAccess.month;
                        year = dir.fdLA.fdateLastAccess.year;
                        hours = dir.ftLA.ftimeLastAccess.hours;
                        minutes = dir.ftLA.ftimeLastAccess.minutes;
                    } else {
                        day = dir.fdC.fdateCreation.day;
                        month = dir.fdC.fdateCreation.month;
                        year = dir.fdC.fdateCreation.year;
                        hours = dir.ftC.ftimeCreation.hours;
                        minutes = dir.ftC.ftimeCreation.minutes;
                    }

                    printf( "%s ", FormatDate( month, day, year + 80 ));

                    if ( gaCountryInfo.fsTimeFmt == 0 ) {
                        if ( hours >= 12 ) {
                                hours -= 12;
                                cAmPm = 'p';
                        } else
                                cAmPm = 'a';
                    }

                    printf( "%2u%c%02u%c  ", hours,
                        gaCountryInfo.szTimeSeparator[0], minutes, cAmPm );
                }

                // last directory on its level?
                fOK = ( find_file( FIND_NEXT, pszCurrent, uMode, &dir, NULL ) != NULL );

                if (( fTree & TREE_BARE ) || ( fTree & TREE_CDD )) {

                    char *arg, *pszPath;

                    pszPath = path_part( pszCurrent );
                    arg = strend( pszPath );
                    strcpy( arg, pszName );
                    printf( FMT_STR, pszPath );

                } else if ( fTree & TREE_ASCII )
                    printf( "%s%c--%s", pszSequelBuffer, (( fOK == 0 ) ? '\\' : '+' ), pszName );
                else
                    printf( "%s%cÄÄ%s", pszSequelBuffer, (( fOK == 0 ) ? 'À' : 'Ã' ), pszName );
                _nxtrow();

                if ( nAttrib & _A_SUBDIR ) {

                        strcat( pszSequelBuffer, (( fOK == 0 ) ? "   " : (( fTree & TREE_ASCII ) ? "|  " : "³  " ) ));

                        pszSave = strdup( pszCurrent );
                        insert_path( pszCurrent, pszName, pszCurrent );
                        mkdirname( pszCurrent, WILD_FILE );
                        free( pszName );

                        rval = DrawTree( pszCurrent, pszSequelBuffer );

                        // check for ^C and reset ^C trapping
                        if (( rval == CTRLC ) || ( cv.exception_flag ))
                                break;
                        strcpy( pszCurrent, pszSave );
                        free( pszSave );

                        // Truncate sequel buffer to its previous value
                        pszSequelBuffer[ strlen( pszSequelBuffer ) - 3 ] = '\0';
                } else
                        free( pszName );
        }

        return rval;
}

*/
