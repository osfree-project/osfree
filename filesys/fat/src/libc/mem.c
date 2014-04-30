//#include "variety.h"
//#include <string.h>
//#include <ctype.h>

//#include "setbits.h"

//#pragma data_seg ( DATA );

extern const char __Alphabet[];

unsigned char _HShift = 3;

//#pragma data_seg ( );

void _far *memset(void _far *dst, char c, int len)
{
    char _far *p;

    for( p = dst; len; --len ) {
        *p++ = c;
    }
    return( dst );
}

void _far *memmove (void _far *_to, const void _far *_from, int _len)
{

    char _far *from = (char _far *)_from;
    char _far *to   = (char _far *)_to;

    if ( from == to )
    {
        return( to );
    }
    if ( from < to  &&  from + _len > to )
    {
        to += _len;
        from += _len;
        while( _len != 0 )
        {
            *--to = *--from;
            _len--;
        }
    }
    else
    {
        while( _len != 0 )
        {
            *to++ = *from++;
            _len--;
        }
    }

    return( to );
}

void _far *memcpy(void _far *in_dst, void _far *in_src, int len)
{
    char _far *dst = in_dst;
    const char _far *src = in_src;

    for( ; len; --len ) {
        *dst++ = *src++;
    }
    return( in_dst );
}


int memcmp(void _far *in_s1, void _far *in_s2, int len)
{
    const char _far *s1 = in_s1;
    const char _far *s2 = in_s2;

    for( ; len; --len )  {
        if( *s1 != *s2 ) {
            return( *s1 - *s2 );
        }
        ++s1; 
        ++s2;
    }
    return( 0 );    /* both operands are equal */
}


int memicmp( const void _far *in_s1, const void _far *in_s2, int len )
{
    const unsigned char _far *s1 = (const unsigned char _far *)in_s1;
    const unsigned char _far *s2 = (const unsigned char _far *)in_s2;
    unsigned char           c1;
    unsigned char           c2;

    for( ; len; --len )  {
        c1 = *s1;
        c2 = *s2;
        if( c1 >= 'A'  &&  c1 <= 'Z' )  c1 += 'a' - 'A';
        if( c2 >= 'A'  &&  c2 <= 'Z' )  c2 += 'a' - 'A';
        if( c1 != c2 ) return( c1 - c2 );
        ++s1;
        ++s2;
    }
    return( 0 );    /* both operands are equal */
}

char _far *strcpy(char _far *s, const char _far *t )
{
    char _far *dst;

    dst = s;
    while( *dst++ = *t++ )
        ;
    return( s );
}

int strlen(char _far *s)
{
    const char _far *p;

    p = s;
    while( *p != '\0' )
        ++p;
    return( p - s );
}

int strnicmp(const char _far *s, const char _far *t, int n)
{
    unsigned char c1;
    unsigned char c2;

    for( ;; ) {
        if( n == 0 )
            return( 0 );            /* equal */
        c1 = *s;
        c2 = *t;

        if( c1 >= 'A'  &&  c1 <= 'Z' )
            c1 += 'a' - 'A';
        if( c2 >= 'A'  &&  c2 <= 'Z' )
            c2 += 'a' - 'A';

        if( c1 != c2 )
            return( c1 - c2 );      /* less than or greater than */
        if( c1 == '\0' )
            return( 0 );            /* equal */
        ++s;
        ++t;
        --n;
    }
}

char _far *strncpy(char _far *dst, char _far *src, int len)
{
    char _far *ret;

    ret = dst;
    for( ;len; --len ) {
        if( *src == '\0' ) 
            break;
        *dst++ = *src++;
    }
    while( len != 0 ) {
        *dst++ = '\0';      /* pad destination string with null chars */
        --len;
    }
    return( ret );
}

void _far *memchr( const void _far *s, char c, int n )
{
    const char _far *cs = s;

    while( n ) {
        if( *cs == c ) {
            return( (void *)cs );
        }
        ++cs;
        --n;
    }
    return( 0 );
}

char _far *strchr( const char _far *s, int c );

/*
char *strchr( const char *s, int c )
{
    char cc = c;
    do {
        if( *s == cc )
            return( (char *)s );
    } while( *s++ != '\0' );
    return( 0 );
}
*/

#define memeq( p1, p2, len )    ( memcmp((p1),(p2),(len)) == 0 )

char _far *strstr(const char _far *s1, const char _far *s2 )
{
    char _far *end_of_s1;
    int     s1len, s2len;

    if( s2[0] == '\0' ) {
        return( (char *)s1 );
    } else if( s2[1] == '\0' ) {
        return( strchr( s1, s2[0] ) );
    }
    end_of_s1 = memchr( s1, '\0', ~0 );
    s2len = strlen( (char *)s2 );
    for( ;; ) {
        s1len = end_of_s1 - s1;
        if( s1len < s2len )
            break;
        s1 = memchr( s1, *s2, s1len );  /* find start of possible match */

        if( s1 == 0 )
            break;
        if( memeq( (void *)s1, (void *)s2, s2len ) )
            return( (char *)s1 );
        ++s1;
    }
    return( 0 );
}

char _far *strcat( char _far *dst, const char _far *t )
{
    char _far *s;

    s = dst;
    while( *s != '\0' )
        ++s;
    while( *s++ = *t++ )
        ;
    return( dst );
}

char _far *strpbrk( const char _far *str, const char _far *charset )
{
    char            tc;
    //unsigned char  vector[ CHARVECTOR_SIZE ];

    //__setbits( vector, charset );
    for( ; tc = *str; ++str ) {
        /* quit when we find any char in charset */
        //if( GETCHARBIT( vector, tc ) != 0 )
        if (strchr(charset, tc))
             return( (char _far *)str );
    }
    return( 0 );
}

int strcmp( const char _far *s, const char _far *t )
{
    for( ; *s == *t; s++, t++ )
        if( *s == '\0' )
            return( 0 );
    return( *s - *t );
}

char _far *utoa( unsigned value, char _far *buffer, int radix )
{
    char     _far *p = buffer;
    char     _far *q;
    unsigned    rem;
    unsigned    quot;
    char        buf[34];    // only holds ASCII so 'char' is OK

    buf[0] = '\0';
    q = &buf[1];
    do {
        rem = value % radix;
        quot = value / radix;

        *q = __Alphabet[rem];
        ++q;
        value = quot;
    } while( value != 0 );
    while( (*p++ = (char)*--q) )
        ;
    return( buffer );
}

char _far *itoa( int value, char _far *buffer, int radix )
{
    char _far *p = buffer;

    if( radix == 10 ) {
        if( value < 0 ) {
            *p++ = '-';
            value = - value;
        }
    }
    utoa( value, p, radix );
    return( buffer );
}

char _far *ultoa( unsigned long value, char _far *buffer, int radix )
{
    char  _far  *p = buffer;
    char        *q;
    unsigned    rem;
    char        buf[34];        // only holds ASCII so 'char' is OK

    buf[0] = '\0';
    q = &buf[1];
    do {
        rem = value % radix;
        value = value / radix;

        *q = __Alphabet[rem];
        ++q;
    } while( value != 0 );
    while( (*p++ = (char)*--q) )
        ;
    return( buffer );
}


char _far *ltoa( long value, char _far *buffer, int radix )
{
    char _far *p = buffer;

    if( radix == 10 ) {
        if( value < 0 ) {
            *p++ = '-';
            value = - value;
        }
    }
    ultoa( value, p, radix );
    return( buffer );
}

int isspace( int c )
{
  switch (c)
    {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      return 1;
    default:
      break;
    }

  return 0;
}

int isdigit( int c )
{
   return (c <= 0x39 && c >= 0x30) ? 1 : 0;
}

int atoi( const char _far *p )  /* convert ASCII string to integer */
{
    int             value;
    char            sign;

    //__ptr_check( p, 0 );

    while( isspace( *p ) )
        ++p;
    sign = *p;
    if( sign == '+' || sign == '-' )
        ++p;
    value = 0;
    while( isdigit(*p) ) {
        value = value * 10 + *p - '0';
        ++p;
    }
    if( sign == '-' )
        value = - value;
    return( value );
}

long int atol( const char _far *p )
{
    long int        value;
    char            sign;

    //__ptr_check( p, 0 );

    while( isspace( *p ) )
        ++p;
    sign = *p;
    if( sign == '+' || sign == '-' )
        ++p;
    value = 0;
    while( isdigit(*p) ) {
        value = value * 10 + *p - '0';
        ++p;
    }
    if( sign == '-' )
        value = - value;
    return( value );
}

