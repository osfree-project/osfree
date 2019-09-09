/* Replaced by osFree team implementation
// create directory
int md_cmd( int argc, char **argv )
{
    int rval = 0;
    char *ptr;
    unsigned int fUpdateFuzzy;
    long fMD;
    char *dirname;

    // check for and remove switches; abort if no directory name arguments
    if (( GetSwitches( argv[1], "SN", &fMD, 0 ) != 0 ) || ( first_arg( argv[1]) == NULL ))
        return ( usage( MD_USAGE ));
    fUpdateFuzzy = (( fMD & 2 ) == 0 );

    for ( argc = 0; (( dirname = ntharg( argv[1], argc )) != NULL ); argc++) {

        mkfname( dirname, 0 );

        // remove trailing '/' or '\'
        strip_trailing( dirname+3, SLASHES );

        // create directory tree if /S specified
        if ( fMD & 1 ) {

            // skip UNC name
            if ( dirname[0] == '\\' ) {
            for ( ptr = dirname + 2; (( *ptr != '\0' ) && ( *ptr++ != '\\' )); )
                ;
            } else
            ptr = dirname + 3;

            for ( _doserrno = 0; ( *ptr != '\0' ); ptr++ ) {

            if (( *ptr == '\\' ) || ( *ptr == '/' )) {
                *ptr = '\0';
                if (( MakeDirectory( dirname, fUpdateFuzzy ) == -1 ) && ( _doserrno != ERROR_ACCESS_DENIED ))
                break;
                *ptr = '\\';
            }
            }
        }

        if ( MakeDirectory( dirname, fUpdateFuzzy ) == -1 )
            rval = error( _doserrno, dirname );
    }

    return rval;
}

*/
