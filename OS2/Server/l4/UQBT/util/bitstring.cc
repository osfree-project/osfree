#include "bitstring.h"

#define BYTESIZE 8

void BitString::init(int size, int lo, int hi, int val)
{
    char mask;
    
    data = NULL;
    
    size = ((size+BYTESIZE-1)/BYTESIZE); /* Convert to number of bytes */
    str = _BitBytes(size, _BitByte(0,0));

    if( lo == -1 ) /* Provides a way of creating a size wide 'don't care' */
        return;
    
    for( ; lo >= BYTESIZE; lo-=BYTESIZE, hi-=BYTESIZE )
        size--;
    mask = ~((1<<lo)-1); /* Set bits at lo or greater */
    if( hi < BYTESIZE ) {
        mask &= ((1<<(hi+1))-1);
        val <<= lo;
        str[--size] = _BitByte(mask,(char)val&mask);
        return;
    } else {
        char v = (val << lo);
        str[--size] = _BitByte(mask,v);
        val >>= (BYTESIZE-lo);
        for( hi-=BYTESIZE; hi >= BYTESIZE; hi-=BYTESIZE ){
            str[--size] = _BitByte(0xFF,(char)val);
            val >>= BYTESIZE;
        }
        mask = ((1<<(hi+1))-1);
        str[--size] = _BitByte(mask,(char)val&mask);
    }
}

bool BitString::operator <( const BitString &a ) const
{
    _CIterator i,j;
    for( i = str.begin(), j = a.str.begin();
         i != str.end() && j != a.str.end();
         i++, j++ ){
        if( i->mask < j->mask) return true;
        if( i->mask > j->mask) return false;
    }
    if( j != a.str.end() ) return true;
    if( i != str.end() ) return false;
    for( i = str.begin(), j = a.str.begin();
         i != str.end() && j != a.str.end();
         i++, j++ ){
        if( i->value < j->value) return true;
        if( i->value > j->value) return false;
    }
    return false;
}

BitString &BitString::operator +=( const BitString &a )
{
    str.reserve(a.str.size() + str.size());
    _CIterator end = a.str.end();
    for(_CIterator i = a.str.begin(); i != end; i++ )
        str.push_back(*i);
    return *this;
}

BitString BitString::operator +( const BitString &a ) const
{
    BitString r = *this;
    r += a;
    return r;
}

BitString &BitString::operator &=( const BitString &a )
{
    str.reserve( a.str.size() ); /* Ensure iterators won't be invalidated */
    _CIterator end = a.str.end(), j = a.str.begin();
    for( _Iterator i = str.begin(); j != end; j++ ){
        if( i == str.end() ) {
            str.push_back(*j);
            i++;
        } else {
            if( (i->mask & j->mask) != 0 ){
                /* Patterns overlap - need to check if the values are
                 * compatible
                 */
                unsigned char val = i->value | j->value;
                if( ((val & i->mask) != i->value) ||
                    ((val & j->mask) != j->value)){
                    /* nope, they're not */
                    str.clear();
                    return *this;
                }
            }
            i->mask |= j->mask;
            i->value |= j->value;
            i++;
        }
    }
    return *this;
}

BitString BitString::operator &( const BitString &a ) const
{
    BitString r = *this;
    r &= a;
    return r;
}


void BitString::printLongOn( FILE *f ) const
{
    for( _CIterator i = str.begin(); i != str.end(); i++ )
        printBits(f,i->mask);
    fprintf( f, "::" );
    for( _CIterator i = str.begin(); i != str.end(); i++ )
        printBits(f,i->value);
}

void BitString::printOn( FILE *f ) const
{
    for( _CIterator i = str.begin(); i != str.end(); i++ ) {
        for( int j=BYTESIZE-1; j>=0; j-- ) {
            fprintf( f, "%c", ((i->mask)&(1<<j))?( (i->value&(1<<j))?'1':'0' ):'x' );
        }
    }
}    

void BitString::printBits( FILE *f, unsigned char b ) const
{
    for( int i=BYTESIZE-1; i>=0; i-- )
        fprintf( f, "%d", (b&(1<<i))?1:0 );
}

BitString BitString::copyMask( void ) const
{
    BitString r = BitString(*this);
    for( _Iterator i = r.str.begin(); i != r.str.end(); i++ )
        (*i).value = 0;
    return r;
}

/*
 * Determine if the receiver matches b, ie all strings which match b also match
 * the receiver.
 */
bool BitString::matches( const BitString &b ) const
{
    _CIterator i = str.begin();
    _CIterator j = b.str.begin();
    while( i != str.end() && j != b.str.end() ) {
        /* If i has any mask bits that j does not, then j can match a string
         * which i can't, therefore i doesn't match j
         */
        if( i->mask & j->mask == i->mask )
            return false;
        /* The bits required for i obviously must be the same for j */
        if( (i->value & i->mask) != (j->value & i->mask) )
            return false;
        i++;
        j++;
    }
    return true;
}

/****************************************************************************/

//#undef __GNUC__

BitStringSet BitStringSet::operator +( const BitStringSet &a ) const
{   BitStringSet r;
    for( _Iterator i = strs.begin(); i != strs.end(); i++ ) {
        for( _Iterator j = a.strs.begin(); j != a.strs.end(); j++ ) {
            r.add( *i + *j );
        }
    }

    return r;
}

BitStringSet BitStringSet::operator &( const BitStringSet &a ) const
{   BitStringSet r;
    for( _Iterator i = strs.begin(); i != strs.end(); i++ ) {
        for( _Iterator j = a.strs.begin(); j != a.strs.end(); j++ ) {
            r.add( *i & *j );
        }
    }

    return r;
}

BitStringSet &BitStringSet::operator |=( const BitStringSet &a )
{
    for( _Iterator i = a.strs.begin(); i !=a.strs.end(); i++ )
        add( *i );
    return *this;
}
    
void BitStringSet::printOn( FILE *f ) const
{
    fprintf(f, "{ " );
    for( _Iterator i = strs.begin(); i != strs.end();) {
        i->printOn(f);
        i++;
        if( i != strs.end() )
            fprintf( f, "\n  " );
    }
    fprintf(f, "}" );
}

void BitStringSet::add( int size, int op, int lo, int hi, int val )
{
    int range = 1<<(hi - lo + 1); /* number of possible values */

    if( val >= range )
        val = range - 1; /* cap the value, just in case */

    if( op == OP_EQUAL || op == OP_LEQUAL || op == OP_GEQUAL )
        add(BitString(size,lo,hi,val));
    if( op == OP_LESS || op == OP_LEQUAL || op == OP_NOTEQUAL )
        for( int i=0; i<val; i++ )
            add(BitString(size,lo,hi,i));
    if( op == OP_GREATER || op == OP_GEQUAL || op == OP_NOTEQUAL )
        for( int i=val+1; i<range; i++ )
            add(BitString(size,lo,hi,i));
}

/* WARNING: This can result in quite a large set... */
void BitStringSet::add( int size, int op, int xlo, int xhi, int ylo, int yhi )
{
    int xrange = 1<<(xhi - xlo + 1);
    int yrange = 1<<(yhi - ylo + 1);
    int range = (xrange<yrange?xrange:yrange);

    if( op == OP_EQUAL || op == OP_LEQUAL || op == OP_GEQUAL )
        for( int i=0; i<range; i++ )
            add(BitString(size,xlo,xhi,i)&BitString(size,ylo,yhi,i));
    if( op == OP_LESS || op == OP_LEQUAL || op == OP_NOTEQUAL )
        for( int i=1; i<range; i++ )
            for( int j=0; j<i; j++ )
                add(BitString(size,xlo,xhi,j)&BitString(size,ylo,yhi,i));
    if( op == OP_GREATER || op == OP_GEQUAL || op == OP_NOTEQUAL )
        for( int i=0; i<range; i++ )
            for( int j=i+1; j<range; j++ )
                add(BitString(size,xlo,xhi,j)&BitString(size,ylo,yhi,i));
}

BitStringSet BitStringSet::getMasks( void )
{
    BitStringSet r;
    for( _Iterator i = strs.begin(); i != strs.end(); i++ ) {
        r.add(i->copyMask());
    }
    return r;
}

bool BitStringSet::matches( const BitStringSet &b ) const
{
    if( strs.size() == 0 || b.strs.size() == 0 )
        return 0;
    
    for( _Iterator it = b.strs.begin(); it != b.strs.end(); it++ ) {
        bool test = false;
        for( _Iterator it2 = strs.begin(); it2 != strs.end(); it2++ ) {
            if( it2->matches( *it ) ) {
                test = true;
                break;
            }
        }
        if( test == false )
            return false;
    }
    return true;
}
