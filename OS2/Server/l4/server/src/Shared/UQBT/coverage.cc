/*==============================================================================
 * FILE:       coverage.cc
 * OVERVIEW:   Implementation for a class that keeps track of the amount of the
 *             source program that has been covered (decoded)
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * $Revision: 1.5 $
 * 09 Jun 00 - Mike: Created
 * 14 Jun 00 - Mike: Changed last element of range to be "1 past the end"
 * 25 Jul 00 - Mike: Added findEndRange, getStartofFirst, and getEndOfLast
 */

#include "coverage.h"

/*==============================================================================
 * FUNCTION:      Coverage::addRange
 * OVERVIEW:      Add the range [st, fi). Check for overlaps and abutting.
 * PARAMETERS:    st: Start of the range
 *                fi: One past the finish of the range. For example, for
 *                  a single 4 byte instruction at 0x1000, st would be 0x1000,
 *                  and fi 0x1004
 * RETURNS:       <nothing>
 *============================================================================*/
void Coverage::addRange(ADDRESS st, ADDRESS fi)
{
    // First find a range already starting with st, if any
    COV_IT it = ranges.find(st);
    if (it != ranges.end()) {
        // There is alreay a range starting at st.
        if (fi < it->second)
            // This new range is already overlapped by a larger old one
            return;
        // Extend the existing range, but be aware of other ranges between
        // the old finish and fi
        COV_IT next(it);
        while ((++next != ranges.end()) && (next->first <= fi)) {
            if (next->second >= fi)
                // Our new range partly overlaps this range, so extend the
                // new range to include the old one
                fi = next->second;
            // Either way, we delete the old range
            ranges.erase(next);
        }
        // Just adjust the end of the existing range
        it->second = fi;
        return;
    }

    // Insert [st, fi) into the map
    pair<ADDRESS, ADDRESS> pr(st, fi);
    pair< COV_IT, bool> ip;    // Insert point
    ip = ranges.insert(pr);
    it = ip.first;

    // Check if new range overlaps any that start between st and fi+1
    COV_IT next(it);
    next++;
    while ((next != ranges.end()) && (next->first <= fi)) {
        if (next->second >= fi)
            // Our new range partly overlaps this range, so extend the
            // new range to include the old one
            it->second = next->second;
        // Either way, we delete the old range
        // Care with iterators, since any iterator to the deleted item
        // becomes invalid
        COV_IT tmp(next);
        next++;
        ranges.erase(tmp);
    }

    // Check for overlap or abutting at the start
    COV_IT prev(it);
    if (prev != ranges.begin()) {
        prev--;                 // Point to the previous entry
        if (prev->second >= it->first) {
            // The previous range abuts or overlaps this one
            // So extend the previous range if needed, and delete this one
            if (it->second > prev->second)
                prev->second = it->second;
            ranges.erase(it);
        }
    }
}

/*==============================================================================
 * FUNCTION:      errorOverlap
 * OVERVIEW:      Display an error message about an overlap between ranges
 * PARAMETERS:    it1: iterator to the first range
 *                it2: iterator to the second range
 * RETURNS:       <nothing>
 *============================================================================*/
void errorOverlap(COV_CIT it1, COV_CIT it2)
{
    ostrstream ost;
    ost << "Overlap of ranges: adding " << hex << it1->first << "-" <<
        it1->second << " with " << it2->first << "-" << it2->second;
    error(str(ost));
}

/*==============================================================================
 * FUNCTION:      Coverage::addRanges
 * OVERVIEW:      Add the ranges from another Coverage object. The ranges
 *                  should not overlap.
 * PARAMETERS:    Reference to the other Coverage object
 * RETURNS:       <nothing>
 *============================================================================*/
void Coverage::addRanges(const Coverage& other)
{
    COV_CIT src; COV_IT dst;
    for (src = other.ranges.begin(); src != other.ranges.end(); src++) {
        // First, check for abutting at the start of this range
        dst = ranges.find(src->first);
        if (dst != ranges.end()) {
            // There is alreay a range starting at st.
            errorOverlap(src, dst);
            continue;
        }

        // Next, insert this range
        pair< COV_IT, bool> ip;    // Insert point
        ip = ranges.insert(*src);
        dst = ip.first;

        // Check if new range overlaps any that start between st and fi+1
        COV_IT next(dst);
        next++;
        while ((next != ranges.end()) && (next->first <= src->second)) {
            if (next->first < src->second)
                // Shouldn't be overlap, but abutting is OK
                errorOverlap(src, next);            
            if (next->second > src->second)
                // The next range partly overlaps this range, so extend this
                // range to include the next one
                dst->second = next->second;
            // Either way, we delete the old range (next)
            // Care with iterators, since any iterator to the deleted item
            // becomes invalid
            COV_IT tmp(next);
            next++;
            ranges.erase(tmp);
        }

        // Check for overlap or abutting at the start
        COV_IT prev(dst);
        if (prev != ranges.begin()) {
            prev--;                 // Point to the previous entry
            if (prev->second >= dst->first) {
                if (prev->second > dst->first)
                    errorOverlap(src, prev);
                // The previous range abuts or overlaps this one
                // So extend the previous range if needed, and delete this one
                if (dst->second > prev->second)
                    prev->second = dst->second;
                ranges.erase(dst);
            }
        }

    }
}

/*==============================================================================
 * FUNCTION:      Coverage::print
 * OVERVIEW:      Print a representation of this object to stream os
 * PARAMETERS:    os: the stream to print to (defaults to cout)
 * RETURNS:       <nothing>
 *============================================================================*/
void Coverage::print(ostream& os /* = cout */) const
{
    COV_CIT it;
    os << hex;
    for (it = ranges.begin(); it != ranges.end(); it++)
        os << it->first << "-" << it->second << "\t";
    os << endl;
}

/*==============================================================================
 * FUNCTION:      Coverage::totalCover
 * OVERVIEW:      Find the total coverage (sum of the extent of all the ranges
 *                  in this object)
 * PARAMETERS:    <none>
 * RETURNS:       The coverage, in bytes
 *============================================================================*/
unsigned Coverage::totalCover() const
{
    unsigned total = 0;
    COV_CIT it;
    for (it = ranges.begin(); it != ranges.end(); it++)
        total += it->second - it->first;
    return total;
}

/*==============================================================================
 * FUNCTION:      Coverage::getFirstGap
 * OVERVIEW:      Get the first gap to a1 and a2. Sets up an iterator that
 *                  must be passed to getNextGap()
 * PARAMETERS:    a1, a2: set to the start and end+1 of the first gap
 *                it: set to point to the first range of the first gap
 * RETURNS:       true if there was a first gap
 * NOTE:          a1, a2, and it are not valid if false is retuned
 *============================================================================*/
bool Coverage::getFirstGap(ADDRESS& a1, ADDRESS& a2, COV_CIT& it) const
{
    it = ranges.begin();
    if (it == ranges.end())         // No ranges at all
        return false;
    COV_CIT it2(it);
    if (++it2 == ranges.end())
        return false;               // Only 1 range, so no gaps
    a1 = it->second;
    a2 = it2->first;
    return true;
}

/*==============================================================================
 * FUNCTION:      Coverage::getNextGap
 * OVERVIEW:      Get the next gap to a1 and a2.
 * NOTE:          Requires a prior call to getFirstGap, and the it parameter
 *                  must be an iterator that was set by that call
 * PARAMETERS:    a1, a2: set to the start and end+1 of the next gap
 *                it: set to point to the first range of the next gap
 * RETURNS:       true if there is a next gap
 * NOTE:          a1, a2, and it are not valid if false is retuned
 *============================================================================*/
bool Coverage::getNextGap(ADDRESS& a1, ADDRESS& a2, COV_CIT& it) const
{
    if (  it == ranges.end()) return false;     // Should not happen
    if (++it == ranges.end()) return false;     // Should not happen
    COV_CIT it2(it);
    if (++it2 == ranges.end())
        return false;                           // No more gaps
    a1 = it->second;
    a2 = it2->first;
    return true;
}

/*==============================================================================
 * FUNCTION:      Coverage::findEndRange
 * OVERVIEW:      If the given address is not part of any range, return the
 *                  given address. Otherwise, return the end of the range
 *                  (exclusive)
 * NOTE:          Can't be made const because of the required test insertion
 * PARAMETERS:    a1: address that may be the start (or middle) of a range
 * RETURNS:       End of the range, or given parameter, as above
 *============================================================================*/
ADDRESS Coverage::findEndRange(ADDRESS a1)
{
    COV_IT it = ranges.find(a1);
    if (it != ranges.end())
        // There is a range that starts at a1. Return the end of this range.
        return (*it).second;
    // Might be in the middle of a range. Find out by a test insertion
    pair<ADDRESS, ADDRESS> p(a1, a1);
    pair<COV_IT, bool> ret;
    ret = ranges.insert(p);
    it = ret.first;
    if (it == ranges.begin()) {
        // There is no intersection
        ranges.erase(it);
        return a1;
    }
    p = *(--it);                // Look at the previous range
    ranges.erase(ret.first);    // Erase the test insertion
    if (p.second > a1)          // Does previous range overlap?
        return p.second;        // Yes, return end of the overlapping range
    // Otherwise, the previous range ends earlier than a1, so it does not
    // overlap
    return a1;
}

/*==============================================================================
 * FUNCTION:      Coverage::getStartOfFirst
 * OVERVIEW:      Get the first address of the first range
 * PARAMETERS:    <none>
 * RETURNS:       Returns the start of the first range, or 0 if no ranges
 *============================================================================*/
ADDRESS Coverage::getStartOfFirst() const
{
    if (ranges.size() == 0)
        return 0;
    return (*ranges.begin()).first;
}

/*==============================================================================
 * FUNCTION:      Coverage::getEndOfLast
 * OVERVIEW:      Get the last address of the last range
 * PARAMETERS:    <none>
 * RETURNS:       Returns the end of the last range, or 0 if no ranges
 *============================================================================*/
ADDRESS Coverage::getEndOfLast() const
{
    if (ranges.size() == 0)
        return 0;
    return (*--ranges.end()).second;
}


#if 0
/*==============================================================================
 * FUNCTION:      main
 * OVERVIEW:      A main program, so this file can be tested without the rest
 *                  of UQBT
 * NOTE:          For testing purposes only
 * PARAMETERS:    <none>
 * RETURNS:       0
 *============================================================================*/
int main()
{
    Coverage c;
    // Insert 3 easy ranges
    c.addRange(0x1000, 0x2000);
    c.addRange(0x3000, 0x4000);
    c.addRange(0x5000, 0x6000);
    cout << "Should be\t1000-2000\t3000-4000\t5000-6000\n";
    cout << "Currently\t"; c.print(); cout << endl;

    // Add one overlapping with the first at start only
    c.addRange(0x1000, 0x1100);

    // Add one that overlaps whole first range
    c.addRange(0x1000, 0x2000);
    cout << "Should be\t1000-2000\t3000-4000\t5000-6000\n";
    cout << "Currently\t"; c.print(); cout << endl;

    // Add abutting ranges
    c.addRange(0x2000, 0x2500);
    c.addRange(0x4000, 0x5000);
    cout << "Should be\t1000-2500\t3000-6000\n";
    cout << "Currently\t"; c.print(); cout << endl;

    // Add completely overlapping ranges
    c.addRange(0x1500, 0x2000);
    c.addRange(0x5200, 0x6000);
    cout << "Should be\t1000-2500\t3000-6000\n";
    cout << "Currently\t"; c.print(); cout << endl;

    // Larger completely overlapping ranges
    c.addRange(0x0F00, 0x2600);
    c.addRange(0x2F00, 0x6100);
    cout << "Should be\tf00-2600\t2f00-6000\n";
    cout << "Currently\t"; c.print(); cout << endl;

    // Partially overlapping ranges
    c.addRange(0x0E00, 0x1500);
    c.addRange(0x1600, 0x2700);
    c.addRange(0x2E00, 0x5000);
    c.addRange(0x6000, 0x6200);
    cout << "Should be\te00-2700\t2e00-6200\n";
    cout << "Currently\t"; c.print(); cout << endl;

    // Totally overlapping range
    c.addRange(0x0100, 0xFFFF);
    cout << "Should be\t100-ffff\n";
    cout << "Currently\t"; c.print(); cout << endl;

    // Totally overlapped range
    c.addRange(0x1000, 0x2000);
    cout << "Should be\t100-ffff\n";
    cout << "Currently\t"; c.print(); cout << endl;

    return 0;
}
#endif
