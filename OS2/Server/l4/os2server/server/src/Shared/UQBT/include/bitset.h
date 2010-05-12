/*==============================================================================
 * FILE:       bitset.h
 * OVERVIEW:   Provides the definition of the BitSet class
 *			   The bitset class is used with data flow analysis
 *
 * Copyright (C) 1999, The University of Queensland, BT group
 *============================================================================*/

/* $Revision: 1.14 $
 * 13 May 99 - Mike: initial version
 * 17 May 99 - Mike: Added #ifndef __BITSET_H__
*/

#ifndef __BITSET_H__
#define __BITSET_H__

// The size of a byte and an unsigned word in bits
#define BYTESIZE 8
#define WORDSIZE (sizeof(unsigned) * BYTESIZE)

#ifdef WIN32
namespace NameSpaceBitSet {
#endif

class BitSet {
	/*
	 * The vector of unsigned words storing the bits.
	 */
	vector<unsigned> words;
    /*
     * The number of bits used (needed for functions like set() which sets all
     * bits to ones). We use a short in the hope that usedBits and universal
     * can share the same memory word. These bitsets could take up a lot of
     * space in BasicBlock objects (there are currently 6 in each BB)
     */
    short usedBits;
    /*
     * If set, this bool indicates that bits higher than usedBits are
     * effectively set (all the way to infinity). This allows setting a finite
     * vector of words to represent the universal set. set() sets this flag;
     * reset() resets it
     */
    char universal;

    /*
     * Set the number of bits used, and expand the vector if needed,
     * given that bit n is now used
     */
    void setUsed(int n);

    /*
     * Set the number of bits used, and expand the vector if needed,
     * given that we are about to "and" or "or" with the other bitset
     */
    void setUsed(const BitSet& other);

public:
	/**
	 * Default constructor.
	 */
	BitSet();

	/**
	 * Copy constructor.
	 */
	BitSet(const BitSet& other);

	/**
	 * Sets every bit.
	 */
	BitSet& set();

	/**
	 * Sets bit n if val is nonzero, and clears bit n if val is zero.
	 */
	BitSet& set(int n, int val = 1);

	/**
	 * Clears bit n.
	 */
	BitSet& reset(int n);

	/**
	 * Returns true if no bits are set.
	 */
	bool none();

	/**
	 * Clears all bits.
	 */
	BitSet& reset();

	/**
	 * Return the highest set bit.
	 */
	int highestBit() const;

	/**
	 * Returns true if bit n is set.
	 */
	bool test(int n) const;

	/**
	 * The equality operator.
	 */
	bool operator==(const BitSet& other) const;

	/**
	 * The inequality operator.
	 */
	bool operator!=(const BitSet& other) const;

	/**
	 * Bitwise and.
	 */
	BitSet& operator&=(const BitSet& other);

	/**
	 * Bitwise or.
	 */
	BitSet& operator|=(const BitSet& other);

	/**
	 * Returns a copy of *this with all of its bits flipped.
	 */
	BitSet operator~() const;

	/**
	 * Prints the bitset as a string of 1's and 0's.
	 */
	friend ostream& operator<<(ostream& os, const BitSet& b);

    /*
     * Test the integrity of this BitSet
     */
    void testInteg() const;

};

/**
 * Bitwise and of two bitsets. This is a global function, not a member
 * function. Note that the expression b1 & b2 is equivalent to creating a
 * temporary copy of b1, using operator&=, and returning the temporary copy.
 */
BitSet operator&(const BitSet& b1, const BitSet& b2);

/**
 * Bitwise or of two bitsets. This is a global function, not a member
 * function. Note that the expression b1 & b2 is equivalent to creating a
 * temporary copy of b1, using operator&|, and returning the temporary copy.
 */
BitSet operator|(const BitSet& b1, const BitSet& b2);

#ifdef WIN32
}
#endif

#endif
