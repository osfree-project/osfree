/*
 * Copyright (C) 1998-1999, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: rt_hide.h
 * Desc: hidden interface for RT class and its derivates as well as the
 *       HRTL class
 *
 * Copyright (C) 1998-1999, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * $Revision: 1.7 $
 * 16 Jul 98 - Mike: Changed mset to an actual set
 * 22 Feb 99 - Mike: Added RTCond copy constructor
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. 
*/

#ifndef RT_HIDE_H
#define RT_HIDE_H

// A RTCond represents a conditional statement (c.f. if-then). It consists of a
// set of numbers and a list of RT's. This RT will always appear in the RHS of
// a table-op instruction definition (i.e. defines multiple instructions through
// a table). 

class RTCond : public RT {
public:
	// Constructor
	RTCond(const set<int, less<int> >& mems, const HRTL& rtls);
	RTCond(const RTCond& other);
    virtual ~RTCond();

	//bool operator==(const RTCond &other) const;
	const HRTL* getRTL() const;			// return the list of RT's

	// return the membership set
	const set<int, less<int> >& getMemSet() const;
	void print(ostream& os = cout) const;
	void writeRT(ofstream& of) const;

private:
	// the membership set against which idx is tested
	set<int, less<int> > mset;		
	HRTL rtlist;		// the guarded list of RT's
};

#endif
