/*==============================================================================
 * FILE:       type.h
 * OVERVIEW:   Definition of the Type class: low level type information
 *
 * Copyright (C) 2000-2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * $Revision: 1.9 $
 *
 * 20 Mar 01 - Mike: Added operator*= (compare, ignore sign, and consider all
 *                  floats > 64 bits to be the same
 * 26 Apr 01 - Mike: Added class typeLessSI
 */

#ifndef __TYPE_H__
#define __TYPE_H__

#include "global.h"

class Type
{
    LOC_TYPE    type;               // The broad type, e.g. INTEGER
    int         size;               // Size in bits, e.g. 16
    bool        signd;              // True if a signed quantity

public:
    // Constructors
                Type();
                Type(LOC_TYPE ty, int sz = 32, bool sg = true);

    // Named constructor: Return type for given temporary variable name
    static Type getTempType(const string &name);

    // Comparisons
    bool        operator==(const Type& other) const;    // Considers sign
    bool        operator-=(const Type& other) const;    // Ignores sign
    bool        operator< (const Type& other) const;    // Considers sign
    bool        operator<<(const Type& other) const;    // Ignores sign 
    bool        operator*=(const Type& other) const;    // Considers all float
                                                        // sizes > 64 bits to be
                                                        // the same

    // Access functions
    int         getSize() const { return size;}
    LOC_TYPE    getType() const { return type;}
    bool        getSigned() const { return signd;}

    // Set functions
    void        setSize(int s) { size = s;}
    void        setType(LOC_TYPE t) {type = t;}
    void        setSigned(bool s) {signd = s;}

    // Update functions
    // ? Not used?
    // void        apply(int op, int newSize);    // Apply type change operator

    // Format functions
    const char* getCtype() const;   // Get the C type, e.g. "unsigned short"
    string getTempName() const; // Get a temporary name for the type
};

/*==============================================================================
 * FUNCTION:    typeLessSI::operator()
 * OVERVIEW:    This class can be used as the third parameter to a map etc
 *                to produce an object which is sign insensitive
 *                Example: map<Type, SemStr, typeLessSI> is a map from Type
 *                  to SemStr where signed int32 and unsigned int32 map to the
 *                  same SemStr
 * PARAMETERS:  None
 * RETURNS:     True if x < y, disregarding sign, and considering all float
 *                types greater than 64 bits to be the same
 *============================================================================*/
class typeLessSI : public binary_function<Type, Type, bool> {
public:
    bool operator() (const Type& x, const Type& y) const {
        return (x << y);    // This is sign insensitive less, not "left shift"
    }
};

#endif  // __TYPE_H__
