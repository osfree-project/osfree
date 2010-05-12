/*
 * bitstring.h - defines a class which maintains strings of bits and associated
 * masks, as suitable for specifying an instruction
 *
 * Copyright (C) 1999, The University of Queensland, BT group
 */

#ifndef bitstring_h
#define bitstring_h 1

#include <vector>
#include <set>
#include <stdio.h>

using namespace std;

/* A single string a bits. Normally internal use only */
class BitString {
    /* Invariant: forall i:str, i.value & ~i.mask == 0,
     * ie all bits not in the mask are zeroed out.
     */
  public:
    void *data;
    
    BitString() {}

    /*
     * Construct a BitString from a bit range and a value. Assumes that the
     * value fits into an int (obviously), hi >= lo, and big-endian ordering.
     * size specifies the width of the initial string (in bits)
     */
    BitString(int size, int lo, int hi, int value ){ init(size,lo,hi,value); }

    /* Ditto, but the size is the minimum needed to fit the given bits */     
    BitString(int lo, int hi, int value){ init(hi,lo,hi,value); }
    
    /* Conjoin two strings, ie the resulting bitstring will match iff both
     * the receiver and the argument match. If the result is empty (no string
     * exists which matches both), then it will set the receiver to the empty
     * string.
     */
    BitString &operator &=( const BitString &a );
    BitString operator &( const BitString &a ) const;

    /* Concatenate two strings together */
    BitString &operator +=( const BitString &a );
    BitString operator +( const BitString &a ) const;

    bool operator <(const BitString &a ) const;
    bool matches( const BitString &a ) const;

    bool isEmpty() const { return str.size() == 0; }
    
    void printOn( FILE *f ) const;
    void printLongOn( FILE *f ) const;
    
    BitString copyMask( void ) const;
  private:
    class _BitByte {
      public:
        _BitByte(unsigned char m, unsigned char v) { mask = m; value = v; }
        unsigned char mask;
        unsigned char value;
    };
    typedef vector<_BitByte> _BitBytes;
    typedef _BitBytes::iterator _Iterator;
    typedef _BitBytes::const_iterator _CIterator;
        
    _BitBytes str;

    void init(int size, int lo, int hi, int value);
    void printBits( FILE *f, unsigned char b ) const;
};


/* A collection of bit strings. This is the usual class you'd use to do stuff
 * with.
 */

#define OP_LESS 0
#define OP_LEQUAL 1
#define OP_EQUAL 2
#define OP_NOTEQUAL 3
#define OP_GREATER 4
#define OP_GEQUAL 5

class BitStringSet {
  public:
    BitStringSet operator &( const BitStringSet &a ) const;
    BitStringSet operator +( const BitStringSet &a ) const;
    BitStringSet &operator |=( const BitStringSet &a );

    const BitString &add( const BitString &a ) { if( !a.isEmpty() ) strs.insert(a); return a; }
    /* add one or more bitstrings to the set to represent the given inequality */
    void add( int size, int op, int lo, int hi, int val );

    /* add one or more bitstrings to the set to represent an
     * inequality between two ranges */
    void add( int size, int op, int xlo, int xhi, int ylo, int yhi );
    /* Return a set which is a copy of this one, except with the values zeroed out */
    BitStringSet getMasks( void );
    /* Return whether the given argument would be matched by the receiver */
    bool matches( const BitStringSet &a ) const;
    void printOn( FILE *f ) const;

  private:
    typedef set<BitString> _BitStrings;
    typedef _BitStrings::iterator _Iterator;
    
    _BitStrings strs;
    
  public:
    typedef set<BitString>::iterator iterator;
    iterator begin() { return strs.begin(); }
    iterator end() { return strs.end(); }
};

#endif /* !bitstring_h */
