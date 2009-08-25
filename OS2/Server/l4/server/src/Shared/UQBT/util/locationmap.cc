/*==============================================================================
 * FILE:       locationmap.cc
 * OVERVIEW:   Provides the implementation of the LocationMap class
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * 15 Oct 01 - Mike: toBit causes any reference to an integer register location
 *              to be stored with the full width of the register
 * 19 Oct 01 - Mike: Reversed the above. The correct fix for this problem is
 *              to only mention INTEGER.16 etc in the RETURNS section of a .pal
 *              file if a different register number is involved. The above
 *              change causes problem with X86 source program's return types
 * 26 Oct 01 - Mike: LocationMap is back to sign insensitive again
 */

#include "global.h"
#include "csr.h"
#include "prog.h"

/**********************************
 * LocationMap methods
 **********************************/

LocationMap::~LocationMap()
{
}

/*==============================================================================
 * FUNCTION:        LocationMap::toLocation
 * OVERVIEW:        Return a reference to the location that has the given ID.
 *                  Assumes that the location is already in the map.
 * PARAMETERS:      id - integer representation of a location
 * RETURNS:         reference to the location that corresponds to id
 *============================================================================*/
const SemStr& LocationMap::toLocation(int locID) const
{
#if 0
if ((unsigned)locID >= IDlocations.size()) {
    cerr << "locID '" << locID << "' is greater than '" << IDlocations.size();
    cerr << "'. Entries in location map:" << endl;
    for (IDlocations_CIT it = IDlocations.begin(); it != IDlocations.end(); it++)
        cerr << "  " << (*it)->second << " -> " << (*it)->first << endl;
}
#endif

    assert((unsigned)locID < IDlocations.size());
    return IDlocations[locID]->first;
}

/*==============================================================================
 * FUNCTION:        LocationMap::toLocations
 * OVERVIEW:        Convert a given bitset into a set of locations.
 * PARAMETERS:      locs - a bitset represntation of a set of locations
 *                  result - set of locations as SemStrs
 * NOTE:            The returned set is "sign insensitive"
 * RETURNS:         <none>
 *============================================================================*/
void LocationMap::toLocations(const BITSET& locs, setSgiSemStr& result) const
{
    for (int i = 0; i < size(); i++)
        if (locs.test(i))
            result.insert(toLocation(i));
}


/*==============================================================================
 * FUNCTION:        LocationMap::toBit
 * OVERVIEW:        Return the bit position corresponding to the given
 *                  location. Will add the location to the map if it
 *                  isn't already in it.
 * PARAMETERS:      loc - a location
 * RETURNS:         the bit position of loc
 *============================================================================*/
int LocationMap::toBit(const SemStr& loc)
{
    int uniqueID;

    // See if it is in the ID map already
    locationIDs_CIT it = locationIDs.find(loc);
    if (it == locationIDs.end()) {

        // Not in the map; add it and return true
        uniqueID = IDlocations.size();

        locationIDs[loc] = uniqueID;
        IDlocations.push_back(locationIDs.find(loc));
    }
    else {
        // It was in the map
        uniqueID = it->second;
    }
    return uniqueID;
}

/*==============================================================================
 * FUNCTION:        LocationMap::toBitset
 * OVERVIEW:        Return the bitset corresponding to the given set
 *                  of locations. Will add any locations to the map if
 *                  they aren't already in it.
 * PARAMETERS:      locs - a set of locations
 * NOTE:            The set is of type setSgiSemStr, which is a type-insensitive
 *                    set of SemStrs
 * RETURNS:         the bitset representing the given locs
 *============================================================================*/
BITSET LocationMap::toBitset(const setSgiSemStr& locs)
{
    BITSET result;
    for (setSgiSemStr::const_iterator it = locs.begin();
      it != locs.end(); it++)
        result.set(toBit(*it));
    return result;
}

/*==============================================================================
 * FUNCTION:        LocationMap::test
 * OVERVIEW:        Test if the given location is in the given bitmap
 * PARAMETERS:      loc: a reference to the SemStr being tested for
 *                  bs: a reference to the bitset being tested against
 * RETURNS:         true if the location is found; false otherwise
 *============================================================================*/
bool LocationMap::test(const SemStr& loc, const BITSET& bs) const
{
    locationIDs_CIT it = locationIDs.find(loc);
    if (it == locationIDs.end())
        // Not even in the map... unusual
        return false;

    int bit = it->second;
    return bs.test(bit);
}

/*==============================================================================
 * FUNCTION:        LocationMap::size
 * OVERVIEW:        Return the number of locations in the map.
 * PARAMETERS:      <none>
 * RETURNS:         the number of locations in the map
 *============================================================================*/
int LocationMap::size() const
{
    return IDlocations.size();
}


/*==============================================================================
 * FUNCTION:        LocationMap::printBitset
 * OVERVIEW:        Print the expressions represented a given bitset whose bit
 *                  representation is determined by this map.
 * PARAMETERS:      os - the output stream to use
 *                  exprs - a set of expressions
 * RETURNS:         the given stream
 *============================================================================*/
ostream& LocationMap::printBitset(ostream& os, const BITSET& exprs)
{
    for (int i = 0; i < size(); i++)
        if (exprs.test(i))
            os << toLocation(i) << " ";
    return os;
}

/*==============================================================================
 * FUNCTION:        LocationMap::printLocs
 * OVERVIEW:        Print the expressions represented a given list of integers.
 * PARAMETERS:      os - the output stream to use
 *                  exprs - a set of expressions
 * RETURNS:         the given stream
 *============================================================================*/
ostream& LocationMap::printLocs(ostream& os, const vector<int>& exprs)
{
    for (unsigned i = 0; i < exprs.size(); i++)
        os << toLocation(exprs[i]) << " ";
    return os;
}

