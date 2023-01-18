/* Replaced by osFree team implementation

// return the current volume name(s)
int volume_cmd( int argc, char **argv )
{
    int rval = 0;

    // if no arguments, return the label for the default disk
    if ( argc == 1 ) {
        argv[1] = gcdir( NULL, 0 );
        argv[2] = NULL;
    }

    while (*(++argv ) != NULL ) {

        if ( getlabel( *argv ) != 0 )
            rval = ERROR_EXIT;
        else
            crlf();
    }

    return rval;
}



*/
