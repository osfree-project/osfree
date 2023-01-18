/* Replaced by osFree team implementation

#define ATTRIB_BY_ATTRIBUTES 1
#define ATTRIB_DIRS 2
#define ATTRIB_NOERRORS 4
#define ATTRIB_PAUSE 8
#define ATTRIB_QUIET 0x10
#define ATTRIB_SUBDIRS 0x20

typedef struct {
    char szSource[MAXFILENAME];
    char szTarget[MAXFILENAME];
    char *szFilename;
    long fFlags;
    int nMode;
    char cAnd;
    char cOr;
} ATTRIB_STRUCT;

static int _attrib( ATTRIB_STRUCT * );

// change file attributes (HIDDEN, SYSTEM, READ-ONLY, and ARCHIVE)
int attrib_cmd( int argc, char **argv )
{
    char *arg;
    char *ptr;
    int n, rval = 0;
    union {
        // file attribute structure
        unsigned char attribute;
        struct {
            unsigned rdonly : 1;
            unsigned hidden : 1;
            unsigned system : 1;
            unsigned reserved : 2;
            unsigned archive : 1;
        } bit;
    } and_mask, or_mask;
    ATTRIB_STRUCT Attrib;

    Attrib.nMode = FIND_CREATE;

    init_page_size();               // clear row & page length vars

    // get date/time/size ranges
    if ( GetRange( argv[1], &aRanges, 0 ) != 0 )
        return ERROR_EXIT;

    // check for flags
    if ( GetSwitches( argv[1], "*DEPQS", &(Attrib.fFlags), 0 ) != 0 )
        return(usage( ATTRIB_USAGE ));

    if ( Attrib.fFlags & ATTRIB_PAUSE ) {
        gnPageLength = GetScrRows();
    }

    Attrib.nMode |= (( Attrib.fFlags & ATTRIB_DIRS ) ? 0x17 : 0x07 );

    // suppress error message from find_file
    if ( Attrib.fFlags & ATTRIB_NOERRORS )
        Attrib.nMode |= FIND_NOERRORS;  // 0x100

    and_mask.attribute = 0x37;
    or_mask.attribute = 0;

    argc = 0;
    strcpy( Attrib.szSource, WILD_FILE );   // default to *.*

    do {

        if ( ( arg = ntharg( argv[1], argc++ )) != NULL ) {

            if ( *arg == '-' ) {

                // remove attribute (clear OR and AND bits)
                while ( *(++arg ) ) {

                    switch ( _ctoupper( *arg ) ) {
                        case 'A':
                            or_mask.bit.archive = and_mask.bit.archive = 0;
                            break;
                        case 'H':
                            or_mask.bit.hidden = and_mask.bit.hidden = 0;
                            break;
                        case 'R':
                            or_mask.bit.rdonly = and_mask.bit.rdonly = 0;
                            break;
                        case 'S':
                            or_mask.bit.system = and_mask.bit.system = 0;
                            break;
                        case '-':
                            // kludge for RTPatch bug
                            break;
                        case '_':
                            // ignore for ___A_ syntax
                            break;
                        default:
                            error( ERROR_INVALID_PARAMETER, arg );
                            return(usage( ATTRIB_USAGE ));
                    }
                }

            } else if ( *arg == '+' ) {

                // add attribute (set OR bits)
                while ( *(++arg ) ) {

                    switch ( _ctoupper( *arg ) ) {
                        case 'A':
                            or_mask.bit.archive = 1;
                            break;
                        case 'H':
                            or_mask.bit.hidden = 1;
                            break;
                        case 'R':
                            or_mask.bit.rdonly = 1;
                            break;
                        case 'S':
                            or_mask.bit.system = 1;
                            break;
                        case '+':
                            // kludge for RTPatch bug
                            break;
                        case '_':
                            // ignore for ___A_ syntax
                            break;
                        default:
                            error( ERROR_INVALID_PARAMETER, arg );
                            return( usage( ATTRIB_USAGE ));
                    }
                }

            } else {

                // it must be a filename
                copy_filename( Attrib.szSource, arg );

                // check for trailing switches
                // KLUDGE for COMMAND.COM compatibility (ATTRIB *.c +h)
                for ( n = argc; (( ptr = ntharg( argv[1], n )) != NULL ); n++ ) {
                    // check for another file spec
                    if ( ( *ptr != '-' ) && ( *ptr != '+' ) )
                        goto change_atts;
                }
            }

        } else {

            change_atts:
            if ( mkfname( Attrib.szSource, 0 ) == NULL )
                return ERROR_EXIT;

            // if source is a directory & no /D specified, add "\*"
            if ( Attrib.fFlags & ATTRIB_DIRS )
                strip_trailing( Attrib.szSource, "\\/" );
            else if ( is_dir( Attrib.szSource ) )
                mkdirname( Attrib.szSource, WILD_FILE );

            // save the source filename part (for recursive calls and
            //   include lists)
            Attrib.szFilename = Attrib.szSource + strlen( path_part( Attrib.szSource));
            ptr = _alloca( strlen( Attrib.szFilename ) + 1 );
            Attrib.szFilename = strcpy( ptr, Attrib.szFilename );

            // change attributes
            Attrib.cAnd = and_mask.attribute;
            Attrib.cOr = or_mask.attribute;

            rval = _attrib( &Attrib );
            if ( ( setjmp( cv.env ) == -1 ) || ( rval == CTRLC ) )
                break;
            EnableSignals();
        }

    } while ( ( rval != CTRLC ) && ( arg != NULL ) );

    // disable signal handling momentarily
    HoldSignals();

    return rval;
}


static int _attrib( ATTRIB_STRUCT *Attrib )
{
    unsigned int old_attrib;
    int fval, rval = 0, rc;
    char *source_name;
    FILESEARCH dir;

    EnableSignals();

    // copy date/time range info
    memmove( &(dir.aRanges), &aRanges, sizeof(RANGES) );

    // change attributes
    for ( fval = FIND_FIRST; ( find_file( fval, Attrib->szSource, ( Attrib->nMode | FIND_BYATTS | FIND_DATERANGE | FIND_NO_DOTNAMES ), &dir, Attrib->szTarget ) != NULL ); fval = FIND_NEXT ) {

        if ( ( setjmp( cv.env ) == -1 ) || ( cv.exception_flag ) ) {
            (void)DosFindClose( dir.hdir );
            return CTRLC;
        }

        // display old attributes, new attributes, and filename
        old_attrib = ( dir.attrib & 0x37 );
        dir.attrib = (unsigned char)(( old_attrib & Attrib->cAnd ) | Attrib->cOr );

        // if attributes were changed, show new ones
        if ( ( Attrib->fFlags & ATTRIB_QUIET ) == 0 ) {

            qputs( show_atts( old_attrib ));

            if ( ( Attrib->cAnd != 0x37 ) || ( Attrib->cOr != 0 ) )
                printf( " -> %s", show_atts( dir.attrib ));

            printf("  %s", Attrib->szTarget );
            crlf();
            _page_break();
        }

        // check for a change; ignore if same
        //   (can't change directory attribute)
        if ( ( old_attrib != (unsigned int)dir.attrib ) && (( rc = SetFileMode( Attrib->szTarget, (dir.attrib & 0x27))) != 0 ) )
            rval = error( rc, Attrib->szTarget );
    }

    // modify matching subdirectory files too?
    if ( Attrib->fFlags & ATTRIB_SUBDIRS ) {

        // strip the filename & add wildcards for directory search
        sprintf( Attrib->szSource, FMT_DOUBLE_STR, path_part( Attrib->szSource ), WILD_FILE );

        // save the current subdirectory start
        source_name = strchr( Attrib->szSource, '*' );

        // search for all subdirectories in this (sub)directory
        //   tell find_file() not to display errors, & to only
        //   return subdir names
#define M (FIND_NOERRORS | FIND_DIRONLY | FILE_DIRECTORY)
        for ( fval = FIND_FIRST; ( find_file( fval, Attrib->szSource, ( Attrib->nMode | M ),&dir,NULL ) != NULL ); fval = FIND_NEXT ) {
#undef M

            // make the new "source"
            sprintf( source_name, FMT_PATH_STR, dir.name, Attrib->szFilename );

            // process directory tree recursively
            rval = _attrib( Attrib );
            if ( ( setjmp( cv.env ) == -1 ) || ( rval == CTRLC ) ) {
                // reset the directory search handle
                (void)DosFindClose( dir.hdir );
                return CTRLC;
            }

            // restore the original name
            strcpy( source_name, WILD_FILE );
        }
    }

    // disable signal handling momentarily
    HoldSignals();

    return rval;
}


char *show_atts( int attribute )
{
    static char atts[6];

    strcpy( atts, "_____" );
    if ( attribute & _A_RDONLY )
        atts[0] = 'R';
    if ( attribute & _A_HIDDEN )
        atts[1] = 'H';
    if ( attribute & _A_SYSTEM )
        atts[2] = 'S';
    if ( attribute & _A_ARCH )
        atts[3] = 'A';
    if ( attribute & _A_SUBDIR )
        atts[4] = 'D';

    return atts;
}
*/
