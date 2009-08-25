/*
 * Copyright (C) 1998, David Ung
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       rt.cc
 * OVERVIEW:   Implementation of the RT, RTAssgn, RTFlagDef, RTFlagCall, RTCall,
 *             and RTCond classes.
 *
 * (C) 1998-2001, The University of Queensland, BT group
 * (C) 1998, David Ung
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *============================================================================*/
 
/* Updates:
 * $Revision: 1.57 $
 * 16 Feb, 98 - David 
 *      updated expand function to fully copy both LHS and RHS of the
 *      assignment
 * 27 Feb 98 - Mike : Changed IsRegUsed() to test the result of the 
 *  dynamic cast in case a class of different type is passed;
 *  if this is so, avoids test
 * 4 Mar, 98 - David : changed IsRegDefined's to used dynamic casts
 * 11 Mar 98 - Cristina 
 *  replaced TRUE value for true (defined in C++).
 * 24 Apr 98 - David : added ModRM class to expand.
 * 29 Apr 98 - David : added new RTFlag constructor.
 * 24 Jun 98 - Mike: mods for SemStr
 * 16 Jul 98 - Mike: split RTFlag into RTFlagDef and RTFlagCall
 * 10 Dec 98 - Mike: added writeRT()
 * 22 Feb 99 - Mike: added copy constructor
 * 09 Mar 99 - Mike: changed print of RTAssgn for FPOP and FPUSH
 * 16 Mar 99 - Mike: RT::print() forces size to print in decimal now
 * 19 Mar 99 - Mike: Added RTFlagCall copy contructor
 * 19 Mar 99 - Mike: Fixed RTFlagDef/Call::print to include commas
 * 08 Apr 99 - Mike: Changes for HEADERS
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 15 Mar 00 - Cristina: RTAssgn::setAFP and RTFlagCalls::setAFP 
 *  transformed into setAXP. 
 * 16 Mar 00 - Mike: Set the size of the semantic strings in RTAssgn constructor
 * 12 Sep 00 - Mike: Added RTAssgn::updateType()
 * 02 Oct 00 - Mike: Removed the code that hacks off the size and sign casts.
 *              This may break some Palm translations
 * 07 Nov 00 - Mike: retabbed
 * 05 Mar 01 - Simon: Removed expand(), class RTCond, removed updateSize(string s)
 * 20 Mar 01 - Mike: Added a clone() method to all RT derived classes
 * 26 Jul 01 - Mike: Update RTAssgn::clone() to clone the condition
 * 26 Jul 01 - Mike: added addGuard(); RTAssgn::print displays condition
 * 30 Jul 01 - Brian: Added new print() methods. Removed unused writeRT()
 *              methods.
 * 01 Aug 01 - Brian: New class HRTL replaces RTlist. Changed include file to
 *              hrtl.h from rtl.h.
 * 07 Aug 01 - Simon: Fixed RTAssgn::searchReplaceAll() to search pCOND,
 *              ditto for RTAssgn::searchAll()
 * 08 Aug 01 - Mike: Added simplifyGuard()
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "rtl.h"
#include "hrtl.h"
#include "ss.h"
#include "rt_hide.h"

//////////////
//          //
// class RT //
//          //
//////////////

RT::RT(RT_KIND t) : kind(t) {}

RT::~RT()
{
}

#if 0
bool RT::operator==(const RT &other) const
{
    return kind == other.kind;
}
#endif

RT_KIND RT::getKind() const { return kind; }


//////////////////////
//                  //
//  class RTAssgn   //
//                  //
//////////////////////

// derived class RTAssgn functions
RTAssgn::RTAssgn()
    : RT(RTASSGN), added(false), pLHS(NULL), pRHS(NULL), pCOND(NULL), size(0)
{
}

RTAssgn::RTAssgn(SemStr *l, SemStr* e, Byte s)
    : RT(RTASSGN), added(false), pLHS(l), pRHS(e), pCOND(NULL), size(s)
{
    // Set the left and right hand expressions to the correct size
    pLHS->getType().setSize(size);
    pRHS->getType().setSize(size);
}

// Constructor with condition (guard, c)
RTAssgn::RTAssgn(SemStr *l, SemStr* e, SemStr* c, Byte s)
    : RT(RTASSGN), added(false), pLHS(l), pRHS(e), pCOND(c), size(s)
{
    // Set the left and right hand expressions to the correct size
    pLHS->getType().setSize(size);
    pRHS->getType().setSize(size);
}

RTAssgn::~RTAssgn()
{
    if (pLHS) delete pLHS;
    if (pRHS) delete pRHS;
    if (pCOND) delete pCOND;
}


/*==============================================================================
 * FUNCTION:        RTAssgn::clone
 * OVERVIEW:        Make a clone of this RTAssgn
 * PARAMETERS:      None
 * RETURNS:         A pointer to a new clone
 *============================================================================*/
RT* RTAssgn::clone()
{
    SemStr* newLHS = new SemStr(*pLHS);
    SemStr* newRHS = new SemStr(*pRHS);
    SemStr* newCond;
    if (pCOND) newCond = new SemStr(*pCOND);
    else newCond = NULL;
    RTAssgn* rta = new RTAssgn(newLHS, newRHS, newCond, size);
    rta->added = added;
    return rta;
}

#if 0
bool RTAssgn::operator==(const RTAssgn &rt) const
{
    return RT::operator==(rt) &&
        *pLHS == *rt.pLHS && 
        *pRHS == *rt.pRHS &&
        type == rt.type;
}
#endif

// change lhs
void RTAssgn::updateLHS(SemStr* l) { pLHS = l; }

// change expression
void RTAssgn::updateRHS(SemStr* e) { pRHS = e; }

// change size in bits of assignment
void RTAssgn::updateSize(Byte s)
{
    size = s;
    // Set the left and right hand expressions to the correct size
    pLHS->getType().setSize(size);
    pRHS->getType().setSize(size);
}

// get lhs
SemStr* RTAssgn::getLHS() const { return pLHS; }

// get rhs
SemStr* RTAssgn::getRHS() const { return pRHS; }

// Get size of assignment transfer
// More or less redundant now, since left side has size
Byte RTAssgn::getSize() const { return size; }

// Get the type of the assignment (= type of the LHS expression)
const Type& RTAssgn::getType() const { return pLHS->getType(); }

// Check if special reg is defined
//bool RTAssgn::isSpRegDefined(int idReg) const
//{
//  return pLHS->isSpRegEqual(idReg);
//}

// Check if numbered reg is defined
bool RTAssgn::isNumRegDefined(int idReg) const
{
    return pLHS->isNumRegEqual(idReg);
}

// Check if special reg is used
//bool RTAssgn::isSpRegUsed(int idReg) const
//{
//  return pRHS->isSpRegCont(idReg);
//}

// Check if numbered reg is used
bool RTAssgn::isNumRegUsed(int idReg) const
{
    return pRHS->isNumRegCont(idReg);
}

#if 0       // HACK
// number of registers used
int RTAssgn::numRegUse() { return exp->numRegUse(); }

// number of registers defined
int RTAssgn::numRegDef()
{
    return (loc->getType() == eREGISTER || loc->getType() == eSPECIALREG ? 1 : 0);
}

#endif      // HACK

// Add a guard to this assignment. Makes a copy of the condition, and ANDs with
// existing guards, if present
void RTAssgn::addGuard(const SemStr* cond)
{
    // Check if there is a condition there
    if (pCOND) {
        // There is already a condition here. We must
        // AND the existing condition with the new one
        *pCOND >> idAnd;
        *pCOND << *cond;
    } else {
        pCOND = new SemStr(*cond);
    }
}

// Simplify the guard. Also, remove the guard or the whole expression if the
// result is !0 or 0 respectively
bool RTAssgn::simplifyGuard()
{
    if (pCOND == 0) return false;
    pCOND->simplify();
    if (pCOND->getFirstIdx() == idIntConst) {
        if (pCOND->getSecondIdx() == 0) {
            // The guard is "false". This whole assignment can be removed.
            return true;
        } else {
            // The guard is "true". Just remove the guard altogether
            delete pCOND;
            pCOND = 0;
        }
    }
    return false;
}

void RTAssgn::print(ostream& os /*= cout*/) const
{
    if (pLHS->len())
    {
        os << "*" << dec << (int)size << "* ";
    }
    if (pCOND) {
        os << "(";
        pCOND->print(os);
        os << ") => ";
    }
    if (pLHS->len())
    {
        pLHS->print(os);
        os << " := ";
        pRHS->print(os);
    }
    else
        // Only has an RHS (e.g. FPOP)
        pRHS->print(os);
}


/*==============================================================================
 * FUNCTION:        RTAssgn::subAXP
 * OVERVIEW:        (see comment for Proc::subAXP)
 * PARAMETERS:      subMap - a map from register to expressions
 * RETURNS:         this is a definition of a register that is being
 *                  substituted for
 *============================================================================*/
bool RTAssgn::subAXP(map<SemStr,SemStr>& subMap)
{
    // Record whether or not this RT is a definition of a register
    // being substituted
    bool isDef = false;

    // Go through each entry in the substitution map
    for (map<SemStr,SemStr>::iterator it = subMap.begin();
        it != subMap.end(); it++) {

        // Replace any uses of the current register with its
        // corresponding substitution
        if (pRHS->searchReplaceAll((SemStr&)it->first,it->second)) {
            // Remove any sizes and sign extends; these will just complicate
            // simplification. Address expressions shouldn't have these anyway
            // No! This actually hacks off the size and sign casts from the
            // whole expression (not just the address part)
            // Commenting out the below may break some Palm code. But another
            // way has to be found!
//          pRHS->removeSize();
            pRHS->simplify();
// cout << "subAXP: RHS now "; pRHS->print(); cout << endl;
        }
        
        // If this is a definition of the current register, then
        // update the map accordingly otherwise replace any use of
        // the register in the LHS of this assignment
        if (*pLHS == it->first) {
            it->second = *pRHS;
            isDef = true;
        }
        else {
            if(pLHS->searchReplaceAll((SemStr&)it->first,it->second)) {
                pLHS->simplify();
// cout << "subAXP: LHS now "; pLHS->print(); cout << endl;
            }
        }
    }
    return isDef;
}


/*==============================================================================
 * FUNCTION:        RTAssgn::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 *                  typeSens - if true, consider type when matching
 * NOTES:           search can't easily be made const
 * RETURNS:         <nothing>
 *============================================================================*/
void RTAssgn::searchAndReplace(SemStr& search, const SemStr& replace,
    bool typeSens)
{
	if (pCOND != NULL)
        pCOND->searchReplaceAll(search, replace, typeSens);
    pLHS->searchReplaceAll(search, replace, typeSens);
    pRHS->searchReplaceAll(search, replace, typeSens);
}

/*==============================================================================
 * FUNCTION:        RTAssgn::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * NOTES:           search can't easily be made const
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool RTAssgn::searchAll(SemStr& search, list<SemStr *> &result,
                        bool typeSens)
{
    return ((pCOND != NULL) && (pCOND->searchAll(search, result, typeSens)))
	 || pLHS->searchAll(search, result, typeSens)
     || pRHS->searchAll(search, result, typeSens);
}

/*==============================================================================
 * FUNCTION:        RTAssgn::updateType
 * OVERVIEW:        Change the type of the LHS and RHS Semantic Strings
 * PARAMETERS:      t - new type
 * RETURNS:         <nothing>
 *============================================================================*/
void RTAssgn::updateType(const Type& t)
{
    pLHS->setType(t);
    pRHS->setType(t);
}


//////////////////////
//                  //
// classes RTFlag*  //
//                  //
//////////////////////

RTFlagDef::RTFlagDef(const string& fname) : 
    RT(RTFLAGDEF), func_name(fname)
{}

RTFlagCall::RTFlagCall(const string& fname) : 
    RT(RTFLAGCALL), func_name(fname)
{}

RTFlagCall::RTFlagCall(const RTFlagCall& other):
    RT(RTFLAGCALL), func_name(other.func_name)
{
    list<SemStr*>::const_iterator i;
    for (i=other.actuals.begin(); i != other.actuals.end(); i++)
        actuals.push_back(*i);
}

/*==============================================================================
 * FUNCTION:        RTFlagCall::clone
 * OVERVIEW:        Make a clone of this RTFlagCall
 * PARAMETERS:      None
 * RETURNS:         A pointer to a new clone
 *============================================================================*/
RT* RTFlagCall::clone()
{
    RTFlagCall* rtf = new RTFlagCall(func_name);
    list<SemStr*>::const_iterator it;
    for (it=actuals.begin(); it != actuals.end(); it++) {
        SemStr* pss = new SemStr(**it);
        rtf->actuals.push_back(pss);
    }
    return rtf;
}

/*==============================================================================
 * FUNCTION:        RTFlagDef::clone
 * OVERVIEW:        Make a clone of this RTFlagDef: should never happen
 * PARAMETERS:      None
 * RETURNS:         A pointer to a new clone
 *============================================================================*/
RT* RTFlagDef::clone()
{
    error("Deep copy of RTFlagDef object");
    return this;
}

RTFlagDef::~RTFlagDef()
{}

RTFlagCall::~RTFlagCall()
{}

#if 0
// Don't expect this to ever get called, so it's not complete.
// Suspect it's needed for gcc to find the class's Virtual Table
// since it's the first virtual function
bool RTFlag::operator==(const RTFlag &other)
{
    return RT::operator==(other) && func_name == other.func_name;
}
#endif

void RTFlagDef::print(ostream& os /*= cout*/) const
{
    os << func_name.c_str() << "( ";
    list<int>::const_iterator i, l;
    l = --params.end();
    for (i = params.begin(); i != params.end(); i++)
    {
        os << theSemTable[*i].sName.c_str();
        if (i != l) os << ",";
        os << " ";
    }
    os << ")";
}

/*==============================================================================
 * FUNCTION:        RTFlagCall::subAXP
 * OVERVIEW:        (see comment for Proc::subAXP)
 * PARAMETERS:      subMap -
 * RETURNS:         this is a definition of a register that is being
 *                  substituted for
 *============================================================================*/
bool RTFlagCall::subAXP(map<SemStr,SemStr>& subMap)
{
    // Go through each entry in the substitution map
    for (map<SemStr,SemStr>::iterator it = subMap.begin();
        it != subMap.end(); it++)

        // Go through each actual
        for (list<SemStr*>::iterator actual = actuals.begin();
            actual != actuals.end(); actual++)

            // Replace any uses of the current register with its
            // corresponding substitution in the current actual
            if ((*actual)->searchReplaceAll((SemStr&)it->first,it->second))
                (*actual)->simplify();
    return false;
}

void RTFlagCall::print(ostream& os /*= cout*/) const
{
    os << func_name.c_str() << "( ";
    list<SemStr*>::const_iterator i, l;
    l = --actuals.end();
    for (i = actuals.begin(); i != actuals.end(); i++)
    {
        (*i)->print(os);
        if (i != l) os << ",";
        os << " ";
    }
    os << ")";
}

void RTFlagCall::searchAndReplace(SemStr& search, const SemStr& replace,
    bool typeInsens)
{
    list<SemStr*>::iterator it;
    for (it = actuals.begin(); it != actuals.end(); it++) {
        (*it)->searchReplaceAll(search, replace, typeInsens);
    }
}
        
/*==============================================================================
 * FUNCTION:        RTFlagCall::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * NOTES:           search can't easily be made const
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool RTFlagCall::searchAll(SemStr& search, list<SemStr *> &result,
                        bool typeSens)
{
    list<SemStr*>::iterator it;
    bool found = false;
    for (it = actuals.begin(); it != actuals.end(); it++) {
        if( (*it)->searchAll(search, result, typeSens) )
            found = true;
    }
    return found;
}

