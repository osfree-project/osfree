// DBCS.C - DBCS utility functions

#include "4all.h"

static CHAR uchDBCSLead[12];

// Get DBCS environment from the OS
void InitDBCSLead( void )
{
    COUNTRYCODE cc = {0, 0};

    DosQueryDBCSEnv( sizeof( uchDBCSLead ), &cc, uchDBCSLead );
}

// Query if character is a DBCS lead char
BOOL IsDBCSLead( UCHAR c )
{
    int i;

    for ( i = 0; uchDBCSLead[i] != 0 || uchDBCSLead[i + 1] != 0; i += 2 ) {
        if ( c >= uchDBCSLead[i] && c <= uchDBCSLead[i + 1] )
            return TRUE;
    }
    return FALSE;
}

// Find if a char in a string is 'split'
BOOL IsDBCSTrailStr( char *base, char *s )
{
    char *ptr;

    for ( ptr = base; ptr < s; ptr++ )
        if ( IsDBCSLead( *ptr ) )
            ptr++;

    return( ptr != s );
}

