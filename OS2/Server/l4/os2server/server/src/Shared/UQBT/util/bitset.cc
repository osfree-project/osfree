/*==============================================================================
 * FILE:       bitset.cc
 * OVERVIEW:   Provides the implementation of the BitSet class
 *             Note: There is a class in STL called bitset, but it is not (as
 *             of this writing) available with gcc (but is with egcs).
 *             But the STL set is fixed in size; we want a variable sized set
 *
 * Copyright (C) 1999-2000, The University of Queensland, BT group
 *============================================================================*/

/*
 * 04 May 00 - Mike: rewrote to use a vector of ints (variable size)
 * 24 May 00 - Mike: usedBits set correctly in &=, |=
 * 30 May 00 - Mike: Moved the above change to setUsed(Bitset& other), and
 *              adjust the size of the vector if needed. Also take care of
 *              operator&= and |= when this->words.size() > other.words.size()
 *              Also &= and |= the universal bits
 *              Fixes some bizzare problems with large programs, e.g. adding
 *              256 bit wide extra parameters, or adding hundreds of dummys
 */

#include "global.h"
#include "bitset.h"

#ifdef WIN32
namespace NameSpaceBitSet {
#endif

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

/*==============================================================================
 * FUNCTION:      BitSet::BitSet
 * OVERVIEW:      Constructor.
 * NOTES:         The vector is guessed at a size of 1 word, suitable for quite
 *                  short programs. Typically, vectors will at least double in
 *                  size each time they are reallocated
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
BitSet::BitSet()
    : words(1), usedBits(0), universal(0)
{
    reset();
}

/*==============================================================================
 * FUNCTION:      BitSet::BitSet
 * OVERVIEW:      Copy constructor.
 * PARAMETERS:    other - bitset to copy
 * RETURNS:       <nothing>
 *============================================================================*/
BitSet::BitSet(const BitSet& other)
{
    words = other.words;
    usedBits = other.usedBits;
    universal = other.universal;
}

/*==============================================================================
 * FUNCTION:      BitSet::setUsed
 * OVERVIEW:      Updates the usedBits member, based on the fact that bit n
 *                  is now used
 * PARAMETERS:    n: bit that is now used
 * NOTE:          Overloaded to take another bitset (see below)
 * RETURNS:       <nothing>
 *============================================================================*/
void BitSet::setUsed(int n)
{
    if (n >= usedBits) {
        int word = n / WORDSIZE;
        while (word >= (int)words.size())
            words.insert(words.end(), universal ? (unsigned)-1 : 0);
        usedBits = n+1;
    }
}

/*==============================================================================
 * FUNCTION:      BitSet::setUsed
 * OVERVIEW:      Updates the usedBits member, based on the fact that we are
 *                  using at least the same number of bits as the other BitSet
 * PARAMETERS:    other: BitSet we are about to "and" or "or" with
 * NOTE:          Overloaded to take a bit number (see above)
 * RETURNS:       <nothing>
 *============================================================================*/
void BitSet::setUsed(const BitSet& other)
{
    int max = MAX(usedBits, other.usedBits);
    int word = max / WORDSIZE;
    while (word >= (int)words.size())
       words.insert(words.end(), universal ? (unsigned)-1 : 0);
    usedBits = max;
}

/*==============================================================================
 * FUNCTION:      BitSet::set
 * OVERVIEW:      Sets this set to represent the universal set
 * PARAMETERS:    <none>
 * RETURNS:       reference to *this
 *============================================================================*/
BitSet& BitSet::set()
{
    for (unsigned i=0; i < words.size(); i++)
        words[i] = (unsigned)-1;
    // Set the "universal bit". Then, if later a test is made on a bit in a
    // word not yet allocated, it will return true, rather than false.
    // This bit is necessary to represent the universal set in a finite,
    // variable sized vector
    universal = 1;
    return *this;
}

/*==============================================================================
 * FUNCTION:      BitSet::set
 * OVERVIEW:      Sets bit n if val is nonzero, and clears bit n if val is zero.
 * PARAMETERS:    n - bit number
 *                val - value to set it to
 * RETURNS:       reference to *this
 *============================================================================*/
BitSet& BitSet::set(int n, int val /*= 1*/)
{
    // Make sure the vector is big enough for this bit number
    setUsed(n);

    // Divide n by the size of a word to find which word to operate on.
    int word = n / WORDSIZE;

    // Modulo n by the size of a word to find which bit to set in the current
    // word
    unsigned mask = 1 << (n % WORDSIZE);

    if (val == 0)
        words[word] &= ~mask;
    else
        words[word] |= mask;

    return *this;
}

/*==============================================================================
 * FUNCTION:      BitSet::reset
 * OVERVIEW:      Clears bit n.
 * PARAMETERS:    n - bit to clear
 * RETURNS:       reference to *this
 *============================================================================*/
BitSet& BitSet::reset(int n)
{
    return set(n, 0);
}

/*==============================================================================
 * FUNCTION:      BitSet::reset
 * OVERVIEW:      Clears all bits.
 * PARAMETERS:    <none>
 * RETURNS:       reference to *this
 *============================================================================*/
BitSet& BitSet::reset()
{
    for (unsigned i=0; i < words.size(); i++)
        words[i] = 0;
    universal = 0;
    return *this;
}

/*==============================================================================
 * FUNCTION:      BitSet::none
 * OVERVIEW:      Returns true if no bits are set.
 * PARAMETERS:    <none>
 * RETURNS:       no bits are set
 *============================================================================*/
bool BitSet::none()
{
    return (usedBits == 0);
}

/*==============================================================================
 * FUNCTION:      BitSet::highestBit
 * OVERVIEW:      Return the highest set bit.
 * PARAMETERS:    <none>
 * RETURNS:       the highest set bit
 *============================================================================*/
int BitSet::highestBit() const
{
    return usedBits-1;
}

#if 0
/*==============================================================================
 * FUNCTION:      BitSet::countBits
 * OVERVIEW:      Return the number of bits set.
 * NOTE:          Not used or tested (found other ways to do the same thing)
 * PARAMETERS:    <none>
 * RETURNS:       the highest set bit
 *============================================================================*/
// "Local" function to count the number of bits in one word (at most `max' bits)
int countWord(unsigned int w, int max)
{
    int count = 0;
    unsigned int mask = 1 << max-1;
    for (int i=max-1; i >= 0; i--, mask >>= 1) {
        if (w & mask)
            count++;
    }
    return count;
}

int BitSet::countBits() const
{
    int count = 0;

    // Divide usedBits by the size of a word to find the number of whole words
    int full = usedBits / WORDSIZE;

    for (int i=0; i < full; i++)
        count += countWord(words[i], WORDSIZE);

    // The last word has fewer bits to count
    int part = usedBits % WORDSIZE;
    if (part)
        count += countWord(words[full], part);

    return count;
}
#endif

/*==============================================================================
 * FUNCTION:      BitSet::test
 * OVERVIEW:      Returns true if bit n is set.
 * PARAMETERS:    n - bit number
 * RETURNS:       true if bit n is set
 *============================================================================*/
bool BitSet::test(int n) const
{
    if (n >= usedBits)
        return (universal != 0);
    int word = n / WORDSIZE;
    unsigned mask = 1 << (n % WORDSIZE);
    return words[word] & mask;
}

/*==============================================================================
 * FUNCTION:      BitSet::operator==
 * OVERVIEW:      The equality operator.
 * PARAMETERS:    other - the set being compared with
 * RETURNS:       *this == other
 *============================================================================*/
bool BitSet::operator==(const BitSet& other) const
{
    if (usedBits != other.usedBits)
        return false;
    for (unsigned i=0; i < words.size(); i++)
        if (words[i] != other.words[i])
            return false;
    return true;
}

/*==============================================================================
 * FUNCTION:      BitSet::operator!=
 * OVERVIEW:      The inequality operator.
 * PARAMETERS:    other - the set being compared with
 * RETURNS:       *this != other
 *============================================================================*/
bool BitSet::operator!=(const BitSet& other) const
{
    return !(*this == other);
}

/*==============================================================================
 * FUNCTION:      operator&
 * OVERVIEW:      Bitwise and of two bitsets. This is a global function, not a
 *                member function. Note that the expression b1 & b2 is
 *                equivalent to creating a temporary copy of b1, using
 *                operator&=, and returning the temporary copy.
 * PARAMETERS:    b1 - first bitset
 *                b2 - second bitset
 * RETURNS:       b1 & b2
 *============================================================================*/
BitSet operator&(const BitSet& b1, const BitSet& b2)
{
    BitSet ret(b1);
    return ret &= b2;
}
    
/*==============================================================================
 * FUNCTION:      operator|
 * OVERVIEW:      Bitwise and of two bitsets. This is a global function, not a
 *                member function. Note that the expression b1 | b2 is
 *                equivalent to creating a temporary copy of b1, using
 *                operator|=, and returning the temporary copy.
 * PARAMETERS:    b1 -
 *                b2 -
 * RETURNS:       b1 | b2
 *============================================================================*/
BitSet operator|(const BitSet& b1, const BitSet& b2)
{
    BitSet ret(b1);
    return ret |= b2;
}

    
/*==============================================================================
 * FUNCTION:      BitSet::operator~
 * OVERVIEW:      Returns a copy of *this with all of its bits flipped.
 * PARAMETERS:    <none>
 * RETURNS:       a copy of *this with all of its bits flipped
 *============================================================================*/
BitSet BitSet::operator~() const
{
    BitSet ret(*this);
    for (unsigned i=0; i < words.size(); i++)
    {
        ret.words[i] = ~words[i];
    }
    // Must also toggle the universal bit
    ret.universal = !universal;
    return ret;
}

/*==============================================================================
 * FUNCTION:      BitSet::operator&=
 * OVERVIEW:      Bitwise and.
 * PARAMETERS:    other - the other BitSet to "and" with
 * RETURNS:       *this & other
 *============================================================================*/
BitSet& BitSet::operator&=(const BitSet& other)
{
    setUsed(other);             // Adjust our size and usedbits if needed
    unsigned i;
    unsigned min = MIN(words.size(), other.words.size());
    for (i=0; i < min; i++)
        words[i] &= other.words[i];
    // Remember, we may have more words than other. If so, we clear our
    // extra words, unless other.universal is set
    if (!other.universal)
        for (i=min; i < words.size(); i++)
            words[i] = 0;
    // We also need to and the universal bits
    universal &= other.universal;
    return *this;
};

/*==============================================================================
 * FUNCTION:      BitSet::operator|=
 * OVERVIEW:      Bitwise or.
 * PARAMETERS:    other - the other BitSet to "or" with
 * RETURNS:       *this | other
 *============================================================================*/
BitSet& BitSet::operator|=(const BitSet& other)
{
    setUsed(other);             // Adjust our size and usedbits if needed
    unsigned i;
    unsigned min = MIN(words.size(), other.words.size());
    for (i=0; i < min; i++)
        words[i] |= other.words[i];
    // Remember, we may have more words than other. If so, we set our
    // extra words to all 1s, if other.universal is set
    if (other.universal)
        for (i=min; i < words.size(); i++)
            words[i] = (unsigned)-1;
    // We also need to or the universal bits
    universal |= other.universal;
    return *this;
};

/*==============================================================================
 * FUNCTION:      operator<<
 * OVERVIEW:      Prints the bitset as a string of 1's and 0's, preceeded by
 *                  0* or 1* depending on the universal flag
 * PARAMETERS:    os - stream to print to
 *                b - bitset to print
 * RETURNS:       os
 *============================================================================*/
ostream& operator<<(ostream& os, const BitSet& b)
{
    os << (int)b.universal << "*";
    for (short i = b.highestBit(); i >=0; i--)
        if (b.test(i))
            os << "1";
        else
            os << "0";
    return os;
}

/*==============================================================================
 * FUNCTION:      BitSet::testInteg()
 * OVERVIEW:      Tests the integrity of this bitset, in the sense that all
 *                  bits from usedBits to infinity are set to the universal bit
 * USE:           Only for debugging
 * PARAMETERS:    none
 * RETURNS:       none, but prints "Here!" when error (breakpoint that line)
 *============================================================================*/
void BitSet::testInteg() const
{
    for (unsigned i=(usedBits%WORDSIZE); i<32; i++) {
        unsigned mask = 1 << i;
        unsigned word = usedBits / WORDSIZE;
        if (word >= words.size()) break;
        if (((words[word] & mask) != 0) != universal)
            cout << "Here!";
    }
}

#ifdef WIN32
}
#endif
