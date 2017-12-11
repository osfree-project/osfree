/*
 * Copyright (C) 1997, Shane Sendall
 * Copyright (C) 1998-1999, David Ung
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       sslinst.cc
 * OVERVIEW:   This file defines the classes used to represent the semantic
 *             definitions of instructions and given in a .ssl file.
 *
 * (C) 1997, Shane Sendall
 * (C) 1998-1999, David Ung
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/
 
/*
 * $Revision: 1.65 $
 *    Dec 1997: Shane: Created
 * 22 Jan 1998 by David Ung - added member function SSLInstDict::RTL.
 * 29 Jan 98 - Cristina
 *  changed list<string> parameter in RTL() function to variable number
 *      of parameters.
 * 3 Feb 98 - Cristina
 *  renamed SSLInstDict to RTLInstDict to avoid confusion between 
 *      SSL instructions and RTL instructions.
 * 3 Feb 98 - David : put all the variable number of params into a list
 *      and added checks to see if it matches its base_name.  Instantiate only
 *      over required operands.
 * 13 Feb 98 - David : converted first param 'name' in RTL to uppercase and
 *      added '_' after it.  eg. add -> ADD_
 * 24 Feb 98 by David - changed RTLInstDict::RTL to accept ADDRESS parameter 
 * 12 Mar 98 - David: changed SSLInst::SSLInst the way it stores opcodes
 * 24 Jun 98 - Mike: mods for SemStr
 * 25 Jun 98 - Mike: RTL()
 * 12 Jul 98 - Mike: Moved MinoParser() to miniparser.cc
 * 16 Jul 98 - Mike: Added RTLInstDict::ReadSSLFile()
 * 27 Oct 98 - Mike: ReadSSLFile() takes bPrint now
 * 06 Jan 99 - Mike: SSLInst::setRTs() takes a reference param now;
 *              SSLInst::getRTL() returns a pointer now;  RTLInstDict::RTL()
 *              returns a reference now
 * 07 Jan 99 - Mike: Added TableEntry::operator=() (does a deep copy)
 * 13 Jan 99 - Ian: Added parameter detailed_r_map to yyparse() for 
 *                  interpreter.
 * 19 Jan 99 - Mike: ReadSSLFile() uses argv[0] if given to locate SSL file
 * 21 Mar 99 - Doug: RTLInstDict::RTL became RTLInstDict::instantiateRTL and now
 *              just returns a list<RT*> value
 * 30 Mar 99 - David: Changed call arguments to yyparse within ReadSSLFile
 * 08 Apr 99 - Mike: Changes for HEADERS
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 17 Aug 00 - Mike: instantiateRTL fixes the type of expressions that are
 *              just r[ int k]
 * 05 Sep 00 - Mike: fixRegType now copes with r[ tmp]
 * 12 Sep 00 - Mike: Changed fixRegType to operate on the whole RTAssgn, and
 *              so updates the type of  both left and right sides at same time.
 * 29 Sep 00 - Mike: Chaged the logic for determining the type of an expression
 *              to include the case where the RHS has a conversion operator
 * 16 Oct 00 - Mike: Base assignment type on operators, not registers.
 * 27 Nov 00 - Mike: Support for -f switch; retabbed
 * 30 Nov 00 - Mike: trunc -> truncu, truncs
 * 11 Feb 01 - Nathan: changed readSSLFile calling convention to take the
 *              complete ssl pathname as a string
 * 14 Feb 01 - Mike: Updated fixType to recognise registers and temps.
 *              Thus each RTAssgn should be created with a sensible Type now
 * 05 Mar 01 - Simon: Removed SSLInst class; various new methods added to
 *              RTLInstDict and TableEntry
 * 20 Mar 01 - Mike: Swapped the loops in instantiateRTL so that instructions
 *              with no parameters and yet RTs still get their initial type
 *              correct
 * 20 Mar 01 - Mike: fixType -> 2 decideType functions; parameters to flag calls
 *              get appropriate type
 * 26 Mar 01 - Mike: decideType returns false for m[...]{size}; added
 *              partialType()
 * 23 Apr 01 - Nathan: Moved decideType and partialType into RTLInstDict, 
 *              changed decideType(RTAssign*) to return success/failure
 * 24 Apr 01 - Nathan: Extended decideType to deal with things like r[rd]
 * 01 May 01 - Nathan: split RTLInstDict::instantiateRTL into two pieces
 * 18 Jun 01 - Nathan: Added postvar handling code.
 * 27 Jun 01 - Nathan: Added fixupParams and lambda handling code
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed setRTL to setHRTL,
 *              and appendRTL to appendHRTL in class TableEntry.
 * 09 Aug 01 - Mike: decideType explicity returns false for memOf (else returns
 *              unsigned 32, the address type, which is plain wrong)
 * 14 Aug 01 - Mike: More smarts in decideType; has extra parameter now
 * 14 Aug 01 - Mike: use upperStr (util.cc) in place of calls to transform()
 * 06 Sep 01 - Mike: decideType copes with FPOP now
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "prog.h"
#include "sslparser.h"
#include "rtl.h"
#include "ss.h"
#include "options.h"


/*==============================================================================
 * FUNCTION:        TableEntry::TableEntry
 * OVERVIEW:        Constructor
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
TableEntry::TableEntry() { flags = 0; }

/*==============================================================================
 * FUNCTION:        TableEntry::TableEntry
 * OVERVIEW:        Constructor
 * PARAMETERS:      p -
 *                  r - reference to a HRTL
 * RETURNS:         <nothing>
 *============================================================================*/
TableEntry::TableEntry(const list<int>& p, const HRTL& r) :
    params(p), hrtl(r)
{ flags = 0; }

/*==============================================================================
 * FUNCTION:        TableEntry::setParam
 * OVERVIEW:        Set the parameter list.
 * PARAMETERS:      p - a list of indexes in theSemTable
 * RETURNS:         <nothing>
 *============================================================================*/
void TableEntry::setParam(const list<int>& p) { params = p; }

/*==============================================================================
 * FUNCTION:        TableEntry::setHRTL
 * OVERVIEW:        Set the HRTL.
 * PARAMETERS:      r - a HRTL
 * RETURNS:         <nothing>
 *============================================================================*/
void TableEntry::setHRTL(const HRTL& r)
{
    hrtl = r;
}

/*==============================================================================
 * FUNCTION:        TableEntry::operator=
 * OVERVIEW:        Sets the contents of this object with a deepcopy from
 *                  another TableEntry object. Note that this is different from
 *                  the semantics of operator= for an HRTL which only does a
 *                  shallow copy!
 * PARAMETERS:      other - the object to copy
 * RETURNS:         a reference to this object
 *============================================================================*/
const TableEntry& TableEntry::operator=(const TableEntry& other)
{
    params = other.params;
    hrtl = *(new HRTL(other.hrtl));
    return *this;
}

/*==============================================================================
 * FUNCTION:        TableEntry::appendHRTL
 * OVERVIEW:        Appends an HRTL to an exising TableEntry
 * PARAMETERS:      p: reference to list of int... (??)
 *                  r: reference to HRTL with list of RTs to append
 * RETURNS:         0 for success
 *============================================================================*/
int TableEntry::appendHRTL(list<int>& p, HRTL& r)
{
    bool match = (p.size() == params.size());
    list<int>::const_iterator a, b;
    for (a = params.begin(), b = p.begin();
      match && (a != params.end()) && (b != p.end());
      match = (*a == *b), a++, b++)
        ;
    if (match) {
        hrtl.appendRTlist(r);
        return 0;
    }
    return -1;
}

/*==============================================================================
 * FUNCTION:        RTLInstDict::appendToDict
 * OVERVIEW:        Appends one RTL to the dictionary
 * PARAMETERS:      n: name of the instruction to add to
 *                  p: list of parameters for the RTL to add
 *                  r: reference to the RTL to add
 * RETURNS:         0 for success
 *============================================================================*/
int RTLInstDict::appendToDict(string &n, list<int>& p, HRTL& r)
{
    char *opcode = new char[n.size() + 1];
    strcpy(opcode, n.c_str());
    upperStr(opcode, opcode);
    remove(opcode, opcode+strlen(opcode)+1,'.');
    string s(opcode);
    delete opcode;
   
    if (idict.find(s) == idict.end())
        idict[s] = TableEntry(p, r);
    else
        return idict[s].appendHRTL(p, r);
    return 0;
}


/*==============================================================================
 * FUNCTION:        RTLInstDict::readSSLFile
 * OVERVIEW:        Read and parse the SSL file, and initialise the expanded
 *                  instruction dictionary (this object). This also reads and
 *                  sets up the register map and flag functions.
 * PARAMETERS:      SSLFileName - the name of the file containing the SSL
 *                    specification.
 *                  bPrint - instructions are displayed on cout as they are
 *                    parsed if this is true
 * RETURNS:         the file was successfully read
 *============================================================================*/
bool RTLInstDict::readSSLFile(const string& SSLFileName, bool bPrint /*= false*/)
{
    // emptying the rtl dictionary
    idict.erase(idict.begin(),idict.end());
    
    // Attempt to Parse the SSL file
    SSLParser theParser(SSLFileName,
#ifdef DEBUG_SSLPARSER
    true
#else
    false
#endif
);
    addRegister( "%CTI", -1, 1, false );
    addRegister( "%NEXT", -1, 32, false );
    
    theParser.yyparse(*this);

    fixupParams();
    
    if (bPrint) {
        cout << "\n=======Expanded RTL template dictionary=======" << endl;
        print();
        cout << "\n==============================================" << endl;
    }
    
    return true;
}

/*==============================================================================
 * FUNCTION:        RTLInstDict::addRegister
 * OVERVIEW:        Add a new register definition to the dictionary
 * PARAMETERS:      
 * RETURNS:         <nothing>
 *============================================================================*/
void RTLInstDict::addRegister( const char *name, int id, int size, bool flt )
{
    RegMap[name] = id;
    if( id == -1 ) {
        SpecialRegMap[name].s_name(name);
        SpecialRegMap[name].s_size(size);
        SpecialRegMap[name].s_float(flt);
        SpecialRegMap[name].s_address(NULL);
        SpecialRegMap[name].s_mappedIndex(-1);
        SpecialRegMap[name].s_mappedOffset(-1);
    } else {
        DetRegMap[id].s_name(name);
        DetRegMap[id].s_size(size);
        DetRegMap[id].s_float(flt);
        DetRegMap[id].s_address(NULL);
        DetRegMap[id].s_mappedIndex(-1);
        DetRegMap[id].s_mappedOffset(-1);
    }    
}


/*==============================================================================
 * FUNCTION:        RTLInstDict::print
 * OVERVIEW:        Print a textual representation of the dictionary.
 * PARAMETERS:      cout - stream used for printing
 * RETURNS:         <nothing>
 *============================================================================*/
void RTLInstDict::print(ostream& os /*= cout*/) const
{
    for (map<string,TableEntry,StrCmp>::const_iterator p = idict.begin();
                p != idict.end(); p++)
    {
        // print the instruction name
        os << (*p).first << "  ";

        // print the parameters
        list<int> const &params = (*p).second.params;
        int i = params.size();
        for (list<int>::const_iterator s = params.begin();s != params.end();
                s++,i--)
            os << theSemTable[*s].sName << (i != 1 ? "," : "");
        os << "\n";
    
        // print the RTL
        HRTL const &rtlist = (*p).second.hrtl;
        rtlist.print(os, 5);
        os << "\n";
    }   
}

/*==============================================================================
 * FUNCTION:         RTLInstDict::fixupParams
 * OVERVIEW:         Runs after the ssl file is parsed to fix up variant params
 *                   where the arms are lambdas.
 * PARAMETERS:       -
 * RETURNS:          -
 *============================================================================*/
void RTLInstDict::fixupParams( )
{
    int mark = 1;
    for( map<int,ParamEntry>::iterator param = DetParamMap.begin();
         param != DetParamMap.end(); param++ ) {
        param->second.mark = 0;
    }
    for( map<int,ParamEntry>::iterator param = DetParamMap.begin();
         param != DetParamMap.end(); param++ ) {
        list<int> funcParams;
        bool haveCount = false;
        if( param->second.kind == PARAM_VARIANT ) {
            fixupParamsSub( param->first, funcParams, haveCount, mark++ );
        }
    }
}

void RTLInstDict::fixupParamsSub( int n, list<int> &funcParams, bool &haveCount, int mark )
{
    ParamEntry &param = DetParamMap[n];

    if( param.params.size() == 0 ) {
        cerr << "Error in SSL File: Variant operand "
             << theSemTable[n].sName
             << " has no branches. Well that's really useful...\n";
        return;
    }
    if( param.mark == mark )
        return; /* Already seen this round. May indicate a cycle, but may not */
    
    param.mark = mark;
    
    for( list<int>::iterator it = param.params.begin();
         it != param.params.end(); it++ ) {
        ParamEntry &sub = DetParamMap[*it];
        if( sub.kind == PARAM_VARIANT ) {
            fixupParamsSub( *it, funcParams, haveCount, mark );
            if( !haveCount ) { /* Empty branch? */
                continue;
            }
        } else if( !haveCount ) {
            haveCount = true;
            char buf[10];
            for( unsigned i=1; i <= sub.funcParams.size(); i++ ) {
                sprintf( buf, "__lp%d", i );
                funcParams.push_back( theSemTable.addItem( buf ) );
            }
        }

        if( funcParams.size() != sub.funcParams.size() ) {
            cerr << "Error in SSL File: Variant operand "
                 << theSemTable[n].sName
                 << " does not have a fixed number of functional parameters:\n"
                 << "Expected " << funcParams.size() << ", but branch "
                 << theSemTable[*it].sName << " has "
                 << sub.funcParams.size() << ".\n";
        } else if( funcParams != sub.funcParams && sub.rta != NULL ) {
            /* Rename so all the parameter names match */
            list<int>::iterator i,j;
            for( i = funcParams.begin(), j = sub.funcParams.begin();
                 i != funcParams.end(); i++, j++ ) {
                SemStr match(2,idParam,*j);
                SemStr replace(2,idParam,*i);
                sub.rta->searchAndReplace( match, replace );
            }
            sub.funcParams = funcParams;
        }
    }

    if( param.funcParams.size() != funcParams.size() )
        theSemTable.setItem( n, cFUNCTION, 0, 0, funcParams.size(),
                             theSemTable[n].sName.c_str() );
    param.funcParams = funcParams;
}

/*==============================================================================
 * FUNCTION:         RTLInstDict::getNumOperands
 * OVERVIEW:         Returns the signature of the given instruction.
 * PARAMETERS:       name -
 * RETURNS:          the signature (name + number of operands)
 *============================================================================*/
pair<string,unsigned> RTLInstDict::getSignature(const char* name)
{
    // Take the argument, convert it to upper case and remove any _'s and .'s
    char *opcode = new char[strlen(name) + 1];
    upperStr(name, opcode);
//  remove(opcode,opcode+strlen(opcode)+1,'_');
    remove(opcode,opcode+strlen(opcode)+1,'.');

    // Look up the dictionary
    map<string,TableEntry,StrCmp>::iterator it = idict.find(opcode);
    if (it == idict.end())
    {
        ostrstream ost;
        ost << "no entry for `" << name << "' in RTL dictionary";
        error(str(ost));
        it = idict.find("NOP");     // At least, don't cause segfault
    } 

    return pair<string,unsigned>(opcode,(it->second).params.size());
}

#if 0
/*==============================================================================
 * FUNCTION:         fixRegType ("local" function)
 * OVERVIEW:         Scan the SemStr pointed to by exp; if it is of the form
 *                      r[ int k] or tmp[ k], then set its type to the
 *                      intrinsic type for that kind of register
 * NOTE:             Also sets the type of the other side at the same time,
 *                      so LHS and RHS will always have the same type
 * PARAMETERS:       exp - points to a SemStr to be scanned
 *                   rta - points to the RTAssgn* object to update
 * RETURNS:          None
 * SIDE EFFECT:      May change the type of the RTAssgn *rta
 *============================================================================*/
void fixRegType(SemStr* exp, RTAssgn* rta)
{
    if (exp->getFirstIdx() == idRegOf) {
        if (exp->getSecondIdx() == idIntConst) {
            int regNum = exp->getThirdIdx();
            // Get the register's intrinsic type
            Type rType = prog.RTLDict.DetRegMap[regNum].g_type();
            rta->updateType(rType);
        }
        else if (exp->getSecondIdx() == idTemp) {
            Type rType = getTempType(theSemTable[exp->getThirdIdx()].sName);
            rta->updateType(rType);
        }
    }
}
#endif

/*==============================================================================
 * FUNCTION:         RTLInstDict::decideType
 * OVERVIEW:         Scan the SemStr pointed to by exp; if its top level
 *                      operator decrees a type, then set the experssion's
 *                      type, and return true
 * NOTE:             This version only inspects one expression
 * PARAMETERS:       exp - points to a SemStr to be scanned
 *                   assignSize - size of the assignment
 * RETURNS:          True if type is decided
 *============================================================================*/
bool RTLInstDict::decideType(SemStr* exp, int assignSize)
{
    switch (exp->getFirstIdx()) {
        // Operators with explicit sizes
        case idFsize:
        case idItof:
            exp->setType(Type(FLOATP, exp->getThirdIdx(), true));
            return true;

        case idFtoi:
        case idTruncs:
        case idSgnEx:
            exp->setType(Type(INTEGER, exp->getThirdIdx(), true));
            return true;

        case idZfill:
        case idTruncu:
            exp->setType(Type(INTEGER, exp->getThirdIdx(), false));
            return true;

        case idSize: {
            // Beware... even though we know the size, we may not know the
            // broad type (e.g. m[]{size})
            SemStr* sub = exp->getSubExpr(0);
            if (decideType(sub, assignSize)) {
                Type subType = sub->getType();
                exp->setType(Type(subType.getType(), exp->getSecondIdx(),
                    subType.getSigned()));
                return true;
            }
            else
                // We have a size but no type
                return false;
        }

        case idMult:            // Implicitly unsigned operators
        case idDiv:
        case idMod:
        case idShiftR:
            exp->setType(Type(INTEGER, assignSize, false));
            return true;

        case idMults:           // Implicitly signed operators
        case idDivs:
        case idMods:
        case idShiftRA:
        case idNeg:
        case idSignExt:
            exp->setType(Type(INTEGER, assignSize, true));
            return true;

        case idFPlus:
        case idFMinus:
        case idFMult:
        case idFDiv:
        // MVE: idFplusd etc should no longer be used, but I believe that they
        // still are
        case idFPlusd:  case idFMinusd: case idFMultd:  case idFDivd:
        case idFPlusq:  case idFMinusq: case idFMultq:  case idFDivq:
        case idSQRTs: case idSQRTd: case idSQRTq: case idSqrt:
        case idSin: case idCos: case idTan: case idArcTan:
        case idLog2: case idLog10: case idLoge:
        {
            SemStr* sub = exp->getSubExpr(0);
            if (decideType(sub, assignSize)) {
                exp->setType(sub->getType());
                return true;
            }
            if( theSemTable[exp->getFirstIdx()].iNumVarArgs > 1 ) {
                sub = exp->getSubExpr(1);
                if(decideType(sub, assignSize)) {
                    exp->setType(sub->getType());
                    return true;
                }
            }
            /* If we didn't get anything specific, just set the basic type
             * and assume the assign size is correct */
            exp->getType().setType(FLOATP);
            return true;
        }

        case idFMultdq:
            exp->setType(Type(FLOATP, 128));
            return true;
        case idFMultsd:
            exp->setType(Type(FLOATP, 64));
            return true;

        case idRegOf:
            if (exp->getSecondIdx() == idIntConst) {
                int regNum = exp->getThirdIdx();
                // Get the register's intrinsic type
                Type rType = DetRegMap[regNum].g_type();
                exp->setType(rType);
                return true;
            } else if (exp->getSecondIdx() == idTemp) {
                // All temps should be r[ tmp[ T ]]
                Type tType =
                    Type::getTempType(theSemTable[exp->getThirdIdx()].sName);
                exp->setType(tType);
                return true;
            } else { /* r[param] or r[expr] */
                /* This is really only useful when processing uninstantiated
                 * rtls, and assumes someone has actually set the types in
                 * DetParamMap (the information isn't in the SSL)
                 */
                int id = exp->findParam();
                if( id != -1 && DetParamMap.find(id) != DetParamMap.end() ) {
                    ParamEntry &ent = DetParamMap[id];
                    if( ent.regType.getType() != TVOID ) {
                        exp->setType( ent.regType );
                        return true;
                    }
                }
            }
            return false;
        case idTemp: {
            // This is just in case there are any bare tmp[ T ] out there
            Type tType =
                Type::getTempType(theSemTable[exp->getSecondIdx()].sName);
            exp->setType(tType);
            return true;
        }

        case idFltConst:
            // Deal with this in partialType, unless other side has better
            // type info
            return false;

        case idMemOf:
            // This operator has no relationship to its subexpressions, so we
            // have no type at all
            return false;

        // Can't be a var; these are created later. Besides, we don't have a
        // reference to the needed proc object here
        default:
            if (exp->len() == 0) return false;  // E.g. FPOP
            for( int i = 0; i < theSemTable[exp->getFirstIdx()].iNumVarArgs;
              i++ ) {
                SemStr *sub = exp->getSubExpr(i);
                if( decideType(sub, assignSize) ) {
                    exp->setType( sub->getType() );
                    return true;
                }
            }
            
    }
    return false;               // Can't decide the type
}

/*==============================================================================
 * FUNCTION:         RTLInstDict::partialType
 * OVERVIEW:         Scan the SemStr pointed to by exp; if its top level
 *                      operator indicates even a partial type, then set
 *                      the experssion's type, and return true
 * NOTE:             This version only inspects one expression
 * PARAMETERS:       exp - points to a SemStr to be scanned
 * RETURNS:          True if a partial type is found
 *============================================================================*/
bool RTLInstDict::partialType(SemStr* exp)
{
    switch (exp->getFirstIdx()) {
        case idSize:
            exp->setType(Type(INTEGER, exp->getSecondIdx(), true));
            return true;
        case idFltConst:
            exp->setType(Type(FLOATP, 64, true));
            return true;
    }
    return false;
}

/*==============================================================================
 * FUNCTION:         RTLInstDict::decideType
 * OVERVIEW:         Scan the right and left hand sides of the given assignment
 *                      and update the type appropriately
 * PARAMETERS:       rta - pointer to the RTAssgn to decide the type of
 * RETURNS:          True if the type is decideable, else false
 *============================================================================*/
bool RTLInstDict::decideType(RTAssgn* rta)
{
    int assignSize = rta->getSize();
    SemStr* exp = rta->getRHS();
    if (decideType(exp, assignSize)) {
        rta->updateType(exp->getType());
        return true;
    }

    // The RHS is typeless. Let's try the LHS for clues
    SemStr* lhs = rta->getLHS();
    if (decideType(lhs, assignSize)) {
        rta->updateType(lhs->getType());
        return true;
    }

    // We have struck out on both sides. See if we can get partial type info
    if (partialType(exp)) {
        rta->updateType(exp->getType());
        return true;
    }

    if (partialType(lhs)) {
        rta->updateType(lhs->getType());
        return true;
    }
    // Dammit...
    return false;
}

/*==============================================================================
 * FUNCTION:         RTLInstDict::instantiateRTL
 * OVERVIEW:         Returns an instance of a register transfer list for
 *                   the instruction named 'name' with the actuals
 *                   given as the second parameter.
 * PARAMETERS:       name - the name of the instruction (must correspond to one
 *                     defined in the SSL file).
 *                   actuals - the actual values
 * RETURNS:          the instantiated list of RTs
 *============================================================================*/
list<RT*>* RTLInstDict::instantiateRTL(const string& name,
    const vector<SemStr*>& actuals)
{ 
    // If -f is in force, use the fast (but not as precise) name instead
    const string* lname = &name;
    if (progOptions.fastInstr) {
        map<string, string>::iterator itf = fastMap.find(name);
        if (itf != fastMap.end()) 
            lname = &itf->second;
    }
    // Retrieve the dictionary entry for the named instruction
    assert( idict.find(*lname) != idict.end() ); /* lname is in dictionary */
    TableEntry& entry = idict[*lname];

    return instantiateRTL( entry.hrtl, entry.params, actuals );
}

/*==============================================================================
 * FUNCTION:         RTLInstDict::instantiateRTL
 * OVERVIEW:         Returns an instance of a register transfer list for
 *                   the parameterized rtlist with the given formals replaced
 *                   with the actuals given as the third parameter.
 * PARAMETERS:       rtl - a register trace list
 *                   params - a list of formal parameters
 *                   actuals - the actual parameter values
 * RETURNS:          the instantiated list of RTs
 *============================================================================*/
list<RT*>* RTLInstDict::instantiateRTL(const HRTL& rtl,
                                       const list<int> &params,
                                       const vector<SemStr*>& actuals)
{
    assert(params.size() == actuals.size());

    // Get a deep copy of the template RTL
    list<RT*>* newList = new list<RT*>();
    rtl.deepCopyList(*newList);

    // Iterate through each RT of the new list of RTs
    for (list<RT*>::iterator rt = newList->begin();
      rt != newList->end(); rt++) {
        // Search for the formals and replace them with the actuals
        list<int>::const_iterator param = params.begin();
        vector<SemStr*>::const_iterator actual = actuals.begin();
        for (; param != params.end(); param++, actual++) {
            if( DetParamMap[*param].funcParams.size() != 0 ) {
                /* It's a function, so a little more complicated */
                SemStr match(1, *param);
                ParamEntry &ent = DetParamMap[*param];
                for( unsigned i=0; i<ent.funcParams.size(); i++ ) {
                    match.push(WILD);
                }
                list<SemStr *> result;
                if( (*rt)->searchAll(match, result) ) {
                    /*
                     * Two stage substitution - extract the args from the call
                     * and sub them into the parameter body, and then substitute
                     * that back in place of the call
                     */
                    for( list<SemStr *>::iterator it = result.begin();
                         it != result.end(); it++ ) {
                        SemStr replace = **actual;
                        list<int>::iterator pit = ent.funcParams.begin();
                        for( int i = 0; pit != ent.funcParams.end(); pit++, i++ ) {
                            SemStr formal = SemStr( 2, idParam, *pit );
                            SemStr *actual = (*it)->getSubExpr(i);
                            replace.searchReplaceAll(formal,*actual);
                            delete actual;
                        }
                        (*rt)->searchAndReplace(**it, replace);
                        delete *it;
                    }
                }
            } else {
                /* Simple parameter - just construct the formal to search for */
                SemStr formal;
                formal.push(idParam);
                formal.push(*param);
                
                (*rt)->searchAndReplace(formal, **actual);
            }
        }
        RT_KIND kd = (*rt)->getKind();
        if (kd == RTASSGN) {
            RTAssgn* rta = (RTAssgn*) *rt;
            // Get the correct initial (top level) type for this RTAssgn
            decideType(rta);
        }
        else if (kd == RTFLAGCALL) {
            // Give each parameter a sensible type
            RTFlagCall* rtf = (RTFlagCall*) *rt;
            list<SemStr*>::iterator pp;
            for (pp = rtf->actuals.begin(); pp != rtf->actuals.end(); pp++)
                // Decide the complete type if possible
                if (!decideType(*pp, 32))
                    // Otherwise, just a partial type will have to do
                    partialType(*pp);
        }
    }

    transformPostVars( newList, true );
    return newList;
}

/* Small struct for transformPostVars */
class transPost {
public:
    bool used;
    bool isNew;
    SemStr *tmp;
    SemStr *match;
    SemStr *base;
    Type type;
};

/*
 * Transform an RTL to eliminate any uses of post-variables. Note that
 * the algorithm used expects to deal with simple expressions as post
 * vars, ie r[22], m[r[1]], generally things which aren't parameterized
 * at a higher level. This is ok for the translator (we do substitution
 * first anyway), but may miss some optimizations for the emulator. If
 * parameters are detected within a postvar, we just force the temporary,
 * which is always safe to do.
 */

list<RT*>* RTLInstDict::transformPostVars( list <RT*> *rts, bool optimize )
{
    map<SemStr,transPost> vars;    // Map from var to details
    int tmpcount = 1;
    SemStr matchParam(1,idParam);

#ifdef DEBUG_POSTVAR
    cout << "Transforming from:\n";
    for (RT_CIT p = rts->begin(); p != rts->end(); p++) {
        cout << setw(8) << " ";
        (*p)->print(cout);
        cout << "\n";
    }
#endif
    
    // First pass: Scan for post-variables and usages of their referents
    for( list<RT*>::iterator rt = rts->begin(); rt != rts->end(); rt++ ) {
        list<SemStr *> *ss, tmpss;
        if( (*rt)->getKind() == RTASSGN ) {
            SemStr *lhs = ((RTAssgn *) *rt)->getLHS();
            SemStr *rhs = ((RTAssgn *) *rt)->getRHS();

            // Look for assignments to post-variables
            if( lhs && lhs->getFirstIdx() == idPostVar ) {
                if( vars.find(*lhs) == vars.end() ) {
                    transPost &el = vars[*lhs];
                    el.used = false;
                    el.type = ((RTAssgn *)*rt)->getType();
                    el.tmp = new SemStr();
                    
                    // Constuct a temporary. We should probably be smarter
                    // and actually check that it's not otherwise used here.
                    string tmpname = el.type.getTempName() + (tmpcount++) + "post" ;
                    *el.tmp << idRegOf << idTemp << theSemTable.addItem(tmpname.c_str());
                    el.tmp->setType( el.type );

                    el.base = lhs->getSubExpr(0);
                    el.match = lhs->getSubExpr(0);
                    *el.match >> WILD;
                    el.isNew = true;

                    if( !optimize ) {
                        el.used = true;
                        el.isNew = false;
                    }
                    
                }
            }
            tmpss.clear();
            tmpss.push_back(lhs);
            tmpss.push_back(rhs);
            ss = &tmpss;
        } else if( (*rt)->getKind() == RTFLAGCALL ) {
            ss = &((RTFlagCall *)*rt)->actuals;
        }

        /* Look for usages of post-variables referents
         * Trickier than you'd think, as we need to make sure to skip over
         * the post-variables themselves. ie match r[0] but not r[0]'
         */
        for( map<SemStr,transPost>::iterator sr = vars.begin();
             sr != vars.end(); sr++ ) {
            if( sr->second.isNew ) {
                // Make sure we don't match a var in its defining statement
                sr->second.isNew = false;
                continue;
            }
            for( list<SemStr *>::iterator s = ss->begin();
                 s != ss->end(); s++ ) {
                if( sr->second.used )
                    break; // don't bother
                if( !*s ) continue;
                if( **s == *sr->second.base ) {
                    sr->second.used = true;
                    break;
                }
                list<SemStr *> res;
                (*s)->searchAll( *sr->second.match, res );
                for( list<SemStr *>::iterator it = res.begin();
                     it != res.end(); it++ ) {
                    if( (*it)->getFirstIdx() != idPostVar )
                        sr->second.used = true;
                    delete *it;
                }
            }
        }
    }

    // Second pass: Replace post-variables with temporaries where needed
    for( list<RT*>::iterator rt = rts->begin(); rt != rts->end(); rt++ ) {
        for( map<SemStr,transPost>::iterator sr = vars.begin();
             sr != vars.end(); sr++ ) {
            if( sr->second.used ) {
                (*rt)->searchAndReplace( (SemStr &)sr->first, *sr->second.tmp);
            } else {
                (*rt)->searchAndReplace( (SemStr &)sr->first, *sr->second.base);
            }
        }
    }

    // Finally: Append assignments back from temporaries
    for( map<SemStr,transPost>::iterator sr = vars.begin();
         sr != vars.end(); sr++ ) {
        if( sr->second.used ) {
            RTAssgn *rta = new RTAssgn( sr->second.base, sr->second.tmp,
                                        sr->second.type.getSize() );
            rta->getLHS()->setType( sr->second.type );
            rts->push_back( rta );
        } else {
            delete sr->second.tmp;
            delete sr->second.base;
        }
        delete sr->second.match;
    }

#ifdef DEBUG_POSTVAR
    cout << "\nTo =>\n";
    for (RT_CIT p = rts->begin(); p != rts->end(); p++) {
        cout << setw(8) << " ";
        (*p)->print(cout);
        cout << "\n";
    }
    cout << "\n";
#endif

    return rts;
}
