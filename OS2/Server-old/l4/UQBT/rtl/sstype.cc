/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * File:       sstype.cc
 * OVERVIEW:   SSL type checker.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/
/*
 * $Revision: 1.10 $
 * 18 Jun 01 - Nathan: File added to cvs
 * 19 Jun 01 - Nathan: Removed exceptions, seems gcc 2.8.1 can't deal
 * 01 Aug 01 - Brian: Changed include file to hrtl.h from rtl.h.
 * 14 Aug 01 - Mike: Removed some SemStr != SemStr (gcc v3 doesn't do this)
 * 06 Sep 01 - Brian: Made PartialType.sprint() more robust in case
 *              of partial types.
 * 07 Sep 01 - Brian: Fixed computeTypes() to make genemu not crash when
 *              processing mc68k specifications.
 */

/*
 * Ok, what we're trying to do here is determine the correct type at every
 * point of the semantic string. Also any implicit casting/conversions that
 * might be required will get made explicit at this point. Unfortunately since
 * not all subtrees will have any definite type, and a lot of things are only
 * partially typed, this is a little more complex than we might like. 
 *
 * Approach taken is basically two passes. First pass traverses the tree and
 * propagate any types found back up. Second pass goes back and sets a type
 * on everything, as well as inserting any conversions. 
 *
 */
#include "global.h"
#include "type.h"
#include "hrtl.h"
#include "ss.h"

static char *type_names[] = { "void","integer","float","dataptr","funcptr","...","bool" };

/*
 * Helper class which stores an incomplete type, so that type data can be added
 * as it's discovered
 */
class PartialType {
public:
    LOC_TYPE type;
    int size;
    bool signd;
    
    bool hasType;
    bool hasSize;
    bool hasSign;
    bool hasErr; /* Error flag to pass back */

    PartialType() {
        hasType = hasSize = hasSign = hasErr = false;
    }
        
    PartialType( const Type &t ) {
        setSign(t.getSigned());
        setSize(t.getSize());
        setType(t.getType());
        hasErr = false;
    }
    PartialType( LOC_TYPE t, int sz, bool sn ) {
        setSign(sn);
        setSize(sz);
        setType(t);
        hasErr = false;
    }
    PartialType( LOC_TYPE t, int sz ) {
        hasSign = false;
        setSize(sz);
        setType(t);
        hasErr = false;
    }
    PartialType( LOC_TYPE t, bool sn ) {
        hasSize = false;
        setSign(sn);
        setType(t);
        hasErr = false;
    }
    PartialType( LOC_TYPE t ) {
        hasSize = hasSign = false;
        setType(t);
        hasErr = false;
    }
    PartialType( int sz ) {
        hasType = hasSign = false;
        setSize(sz);
        hasErr = false;
    }

    string sprint() const {
        char buf[15], buf2[25];
        const char* typeName;
        if ((type >= 0) && (type <= UNKNOWN)) {
            typeName = (hasType? type_names[type] : "*");
        } else {
            sprintf( buf2, "BAD TYPE[%d]", type );
            typeName = buf2;
        }
        sprintf( buf, "%d", size );
        return string() +
            typeName + "-" +
            (hasSize ? buf : "*") + "-" +
            (hasSign ? (signd ? "signed" : "unsigned") : "*");
    }

    Type finalize() {
        if( !isFinal() ) {
#ifdef DEBUG_TYPES            
            fprintf( stderr, "Warning: Converting incomplete type %s", sprint().c_str() );
#endif
            if( !hasType ) type = INTEGER;  
            if( !hasSize ) size = 32;
            if( !hasSign ) signd = true;
            hasType = hasSize = hasSign = true;
#ifdef DEBUG_TYPES            
            fprintf( stderr, " - defaulting to %s\n", sprint().c_str() );
#endif
        }
        return Type(type, size, signd);
    }
    
    void setType( LOC_TYPE t ) {
        type = t;
        hasType = true;
        if( t == BOOLEAN ) {
            size = 1;
            signd = false;
            hasSize = hasSign = true;
        } else if( t == FLOATP ) {
            signd = true;
            hasSign = true;
        }
    }
    void setSize( int sz ) {
//        if( sz == 1 )
//            setSign(false); // signed 1 bit numbers don't make a lot of sense
        size = sz;
        hasSize = true;
    }

    void setSign( bool sn ) {
        signd = sn;
        hasSign = true;
    }

    bool inject( const PartialType &t ) {
        if(!( *this == t )) return false;
        *this += t;
        return true;
    }
    
    bool isFinal() const {
        return hasType && hasSign && hasSize;
    }
    bool isForcibleTo( const PartialType &t ) const {
        return (!hasSize || !t.hasSize || size == t.size) &&
            (!hasSign || !t.hasSign || signd == t.signd);
    }
    bool operator ==( const PartialType &t ) const {
        return (!hasType || !t.hasType || type == t.type) &&
            (!hasSize || !t.hasSize || size == t.size) &&
            (!hasSign || !t.hasSign || signd == t.signd);
    }
    bool operator ==( const Type &t ) const {
        return (!hasType || type == t.getType() ) &&
            (!hasSize || size == t.getSize() ) &&
            (!hasSign || signd == t.getSigned() );
    }
    bool operator ==( LOC_TYPE t ) const {
        return (!hasType || type == t );
    }
    PartialType &operator +=( const PartialType &t ) {
        if( t.hasType ) { hasType = true; type = t.type; }
        if( t.hasSize ) { hasSize = true; size = t.size; }
        if( t.hasSign ) { hasSign = true; signd = t.signd; }
        hasErr = t.hasErr || hasErr;
        return *this;
    }
    PartialType operator +( const PartialType &t ) const {
        PartialType r = *this;
        r += t;
        return r;
    }
    PartialType operator +( LOC_TYPE t ) const {
        PartialType r = *this;
        r.setType(t);
        return r;
    }
};

static void injectError() {
    /* useful place to plant debugger breakpoints */
}

#define INJECT(x) \
        if( !in.inject(x) ) { \
            cerr << "Type mismatch: expected " << in.sprint() \
                 << ", got " << PartialType(x).sprint() << "\n" \
                 << "   in \'" << ss.sprint() << "\'\n"; \
            in.hasErr = true; \
            injectError(); \
        }
#define REGISTER(x) (DetRegMap.find(x)->second)
#define SPECIALREG(x) (SpecialRegMap.find(x)->second)
#define PARAMETER(x) (DetParamMap.find(x)->second)

PartialType RTLInstDict::computeTypes( SemStr &ss, SSIT &it, SSIT &end,
                                       const PartialType &expected,
                                       vector<PartialType> &types ) const
{
    int idx = *it++;
    PartialType in = expected;
    PartialType out;

    int posn = types.size();
    types.push_back(in); // allocate space in vector;
    switch(idx) {
        case idIntConst:
            it++;
            INJECT(PartialType(INTEGER));
            break;
        case idFltConst:
            it++;
            it++;
            INJECT(PartialType(FLOATP));
            break;
        case idMemOf:
            computeTypes( ss, it, end, PartialType(INTEGER), types );
            return in;
        case idRegOf: {
            if( *it == idTemp ) { /* Special case - r[tmpx] is one unit */
                it++;
                INJECT(Type::getTempType(theSemTable[*it++].sName));
                types.push_back(PartialType(INTEGER,32,false)); // dummy value
                break;
            }
            set<int> regs = evaluateSemStr( SemStr(it,end) );
            assert(regs.size() > 0);
            PartialType regType;
            if( regs.size() == 1 ) {
                regType = REGISTER(*regs.begin()).g_type();
            } else {
                int block = matchRegIdx( SemStr(it,end) );
                if( block != -1 )
                    regType = PARAMETER(block).regType;
                else {
                    /* Hmm... should I throw a tantrum here? */
                    cerr << "Warning: Unable to determine type of register expr r["
                         << SemStr(it,end).sprint() << "]\n";
                    computeTypes( ss, it, end, PartialType(INTEGER), types );
                    break;
                }
            }
            /* Special case - allow referring to registers as the wrong type,
             * and insert forced casts to cope. Ugly but needed for sparc.
             */
            if( !(in == regType) && in.isForcibleTo(regType) ) {
                SSIT ins = it;
                ins--; /* Insert before the r[ */
                if( in.type == INTEGER )
                    ss.getIndices()->insert(ins, idForceInt);
                else if( in.type == FLOATP )
                    ss.getIndices()->insert(ins, idForceFlt);
                /* Note of explanation: We already have in on the stack, so
                 * this leaves that in as the type of the idForce*...
                 */
                types.push_back(in);
                posn++;
                cerr << "Warning: Type conversion inserted - expected "
                     << in.sprint() << ", got " << regType.sprint() << "\nin "
                     << ss.sprint() << "\n";
                in = regType;
            } else INJECT(regType);
            computeTypes( ss, it, end, PartialType(INTEGER), types );
        }
            break;
        case idAddrOf:
            INJECT(PartialType(INTEGER));
            in = computeTypes( ss, it, end, in, types );
            break;
        case idPC: case idAFP: case idAGP: case idZF: case idCF: case idNF:
        case idOF: case idFZF: case idFLF: case idFGF: case idCTI:
            INJECT(SPECIALREG(theSemTable[idx].sName).g_type());
            break;
        case idParam:
            INJECT(PARAMETER(*it).type);
            it++;
            break;
        case idTern:
            computeTypes( ss, it,end,PartialType(BOOLEAN), types );
            in = computeTypes( ss, it,end,in, types );
            in = computeTypes( ss, it,end,in, types );
            return in;
        case idAt: {
            out = computeTypes( ss, it, end, PartialType(INTEGER), types );
            SSCIT ita = it;
            computeTypes( ss, it, end, PartialType(INTEGER), types );
            SSCIT itb = it;
            computeTypes( ss, it, end, PartialType(INTEGER), types );
            SSCIT itc = it;
            SemStr loss(ita,itb);
            SemStr hiss(itb,itc);
            if( loss == hiss ) { /* minor optimization */
                INJECT( PartialType(INTEGER, 1) );
                break;
            }
            set<int> rangeset = evaluateBitsliceRange( loss, hiss );
            if( rangeset.size() == 0 ) {
                /* Couldn't evaluate, assume can be anything */
                INJECT(out); /* Insize == outsize */
            } else {
                int range = *rangeset.rbegin();
                if( out.hasSize && range > out.size ) {
                    fprintf( stderr, "Warning: Bogus bitslice can take bits that aren't actually there...\n" );
                }
                INJECT( PartialType(INTEGER, range) );
            }
            break;
        }
        case idSize:
            INJECT(PartialType(*it++));
            in = computeTypes( ss, it, end, in, types );
            break;
        case idPlus: case idMinus: case idMult: case idDiv: case idMults:
        case idDivs: case idMod: case idMods: 
        case idBitAnd: case idBitOr: case idBitXor:
            INJECT(PartialType(INTEGER));
            in = computeTypes( ss, it, end, in, types );
            in = computeTypes( ss, it, end, in, types );
            break;
        case idNot: case idNeg:
            INJECT(PartialType(INTEGER));
            in = computeTypes( ss, it, end, in, types );
            break;
        case idSignExt:
            INJECT(PartialType(INTEGER,true));
            in = computeTypes( ss, it, end, in, types );
            break;
        case idShiftL: case idShiftR: case idShiftRA:
        case idRotateL: case idRotateR: case idRotateLC: case idRotateRC:
            INJECT(PartialType(INTEGER));
            in = computeTypes( ss, it, end, in, types );
            computeTypes( ss, it, end, PartialType(INTEGER), types );
            break;
        case idFPlus: case idFMinus: case idFMult: case idFDiv:
            INJECT(PartialType(FLOATP));
            in = computeTypes( ss, it, end, in, types );
            in = computeTypes( ss, it, end, in, types );
            break;
        case idFPlusd: case idFMinusd: case idFMultd: case idFDivd:
            INJECT(PartialType(FLOATP,64,true));
            in = computeTypes( ss, it, end, in, types );
            in = computeTypes( ss, it, end, in, types );
            break;
        case idFPlusq: case idFMinusq: case idFMultq: case idFDivq:
            INJECT(PartialType(FLOATP,128,true));
            in = computeTypes( ss, it, end, in, types );
            in = computeTypes( ss, it, end, in, types );
            break;
        case idAnd: case idOr:
            INJECT(PartialType(BOOLEAN));
            in = computeTypes( ss, it, end, in, types );
            in = computeTypes( ss, it, end, in, types );            
            break;
        case idEquals: case idNotEqual: case idLess: case idGtr: case idLessEq:
        case idGtrEq: case idLessUns: case idLessEqUns: case idGtrEqUns:
            INJECT(PartialType(BOOLEAN));
            out = computeTypes( ss, it, end, PartialType(), types ); // Anything goes...
            out = computeTypes( ss, it, end, out, types );
            types[posn+1] = out; // set type of first subexpr to overall type
            break;
        case idSin: case idCos: case idTan: case idArcTan: case idLog2: case idLog10:
        case idLoge: case idSqrt:
            INJECT(PartialType(FLOATP));
            in = computeTypes( ss, it, end, in, types );
            break;
        case idTruncu: case idZfill: case idTruncs: case idSgnEx:
            out = in;
            out.setSize(*it++);
            out.setType(INTEGER);
            INJECT(PartialType(INTEGER, *it++));
            computeTypes( ss, it, end, out, types );
            break;
        case idItof:
            out = in;
            out.setSize(*it++);
            out.setType(INTEGER);
            INJECT(PartialType(FLOATP, *it++));
            computeTypes( ss, it, end, out, types );
            break;
        case idFsize: case idFround:
            out = in;
            out.setSize(*it++);
            out.setType(FLOATP);
            INJECT(PartialType(FLOATP, *it++));
            computeTypes( ss, it, end, out, types );
            break;            
        case idFtoi:
            out = in;
            out.setSize(*it++);
            out.setType(FLOATP);
            INJECT(PartialType(INTEGER, *it++));
            computeTypes( ss, it, end, out, types );
            break;
        case idExecute:
            INJECT(PartialType(INTEGER));
            computeTypes( ss, it, end, out, types );
            break;
        case idNEXT:
            INJECT(PartialType(INTEGER,32));
            break;
        default:
            if( SpecialRegMap.find(theSemTable[idx].sName) != SpecialRegMap.end() ) {
                INJECT(SPECIALREG(theSemTable[idx].sName).g_type());
            } else if( DetParamMap.find(idx) != DetParamMap.end() ) {
                INJECT(PARAMETER(idx).type);
                for( unsigned i = 0; i < PARAMETER(idx).funcParams.size(); i++ ) {
                    /* Lambda parameters aren't really typed at the moment */
                    computeTypes( ss, it, end, PartialType(), types );
                }
            } else {
                fprintf( stderr, "case branch not handled in computeTypes: %d\n", idx );
                assert(0);
            }
    }

    types[posn] = in;
    return in;
}

PartialType RTLInstDict::computeTypes( SemStr &ss, const PartialType &in,
                                       vector<PartialType> &types ) const
{
    SSIT it = ((SemStr &)ss).getIndices()->begin();
    SSIT end = ((SemStr &)ss).getIndices()->end();
    if( it == end ) return in;
    return computeTypes( ss, it, end, in, types );
}

typedef vector<PartialType>::iterator vpt_it;
/*
 * Second pass - distribute the type information and convert to real Types
 * (as opposed to our PartialTypes)
 */
void RTLInstDict::distributeTypes( const SemStr &ss, SSCIT &it, SSCIT &end,
                                   const PartialType &parent,
                                   vpt_it &type, vector<Type> &out ) const
{
    int idx = *it++;
    const SemItem &si = theSemTable[idx];
    PartialType in = parent;

    /* Note that this really has more to do with sanity checking than
     * anything else. Although it may become important under some (admittedly
     * obscure) circumstances, which is why this is more complex then
     * one might expect...
     */
    INJECT(*type++);
    out.push_back( in.finalize() );
    
    switch(idx){
        case idTruncu: case idTruncs: case idZfill: case idSgnEx:
        case idFsize: case idFtoi: case idItof: case idFround:
        case idEquals: case idNotEqual: case idLess: case idGtr: case idLessEq:
        case idGtrEq: case idLessUns: case idLessEqUns: case idGtrEqUns:
        case idRegOf: case idMemOf: case idForceInt: case idForceFlt:
            /* When the child type is independent of the parent type,
             * don't muddle the current type into it.
             */
            in = PartialType();
    }
    
    for( int i=0; i<si.iNumIntArgs + si.iNumFixedArgs; i++ ) it++;
    for( int i=0; i<si.iNumVarArgs; i++ ) {
        switch(idx) {
            case idShiftL: case idShiftR: case idShiftRA:
            case idRotateL: case idRotateR: case idRotateLC: case idRotateRC:
                if( i > 0 ) {
                    distributeTypes( ss, it, end, PartialType(INTEGER), type, out );
                    continue;
                }
                break;
            case idAt:
                distributeTypes( ss, it, end, PartialType(INTEGER), type, out );
                continue;
            case idTern:
                if( i == 0 ) {
                    distributeTypes( ss, it, end, PartialType(), type, out );
                    continue;
                }
                break;
        }
        distributeTypes( ss, it, end, in, type, out );
    }
}

void RTLInstDict::distributeTypes( const SemStr &ss, const PartialType &parent,
                      vector<PartialType> &types, vector<Type> &out ) const
{
    SSCIT it = ((SemStr &)ss).getIndices()->begin();
    SSCIT end = ((SemStr &)ss).getIndices()->end();
    if( it == end ) return;
    vpt_it type = types.begin();
    distributeTypes( ss, it, end, parent, type, out );
}

vector<Type> RTLInstDict::computeRHSTypes( SemStr &ss, int ssize ) const
{
    vector<PartialType> partial;
    vector<Type> result;
    PartialType root;

    root = computeTypes(ss, root, partial);
    if( root.hasErr ) { /* failed */
        return result;
    }
#ifdef DEBUG_TYPES
    if( !root.isFinal() )
        fprintf( stderr, "Warning: Expression could not be fully typed: %s\n",
                 ss.sprint().c_str() );
#endif
    if( !root.hasSign )
        root.setSign( true ); /* Assume signed */
    if( !root.hasSize ) {
        root.setSize( ssize );
    } else if( root.size != ssize ) {
        /* We allow these, since too much code depends on it. Add implicit
         * conversions where needed
         */
        ss.prep(ssize);      /* To */
        ss.prep(root.size);  /* From */
        if( root.size < ssize && root.signd == false )
            ss.prep(idZfill);
        else if( root.size < ssize && root.signd == true )
            ss.prep(idSgnEx);
        else if( root.size > ssize && root.signd == false )
            ss.prep(idTruncu);
        else if( root.size > ssize && root.signd == true )
            ss.prep(idTruncs);
        else assert(0);
        root.setSize(ssize);
        partial.clear();
        computeTypes(ss, root, partial); // little inefficient, really
    }
    distributeTypes( ss, root, partial, result );
    return result;
}

vector<Type> RTLInstDict::computeLHSTypes( SemStr &ss ) const
{
    vector<PartialType> partial;
    vector<Type> result;
    PartialType root;

    root = computeTypes(ss, root, partial);
    if( root.hasErr ) { /* failed */
        return result;
    }
#ifdef DEBUG_TYPES
    if( !root.isFinal() )
        fprintf( stderr, "Warning: Expression could not be fully typed: %s\n",
                 ss.sprint().c_str() );
#endif
    distributeTypes( ss, root, partial, result );
    return result;
}

vector<Type> RTLInstDict::computeRTAssgnTypes( RTAssgn &rta ) const
{
    vector<PartialType> leftpart, rightpart;
    vector<Type> result;
    PartialType left, right;
    SemStr &lhs = *rta.getLHS();
    SemStr &rhs = *rta.getRHS();
    int ssize = rta.getSize();

    right = computeTypes(rhs, right, rightpart);
    if( right.hasErr ){ /* failed */
        left = computeTypes(lhs, left, leftpart);
        return result;
    }
#ifdef DEBUG_TYPES
    if( !right.isFinal() )
        fprintf( stderr, "Warning: Expression could not be fully typed: %s\n",
                 rhs.sprint().c_str() );
#endif
    if( !right.hasSign )
        right.setSign( true ); /* Assume signed */
    if( !right.hasSize ) {
        right.setSize( ssize );
    } else if( right.size != ssize ) {
        cerr << "Warning: Assignment size " << ssize
             << " doesn't match rhs size " << right.size << " at:\n";
        rta.print(cerr);
        cerr << "\n";
        /* We allow these, since too much code depends on it. Add implicit
         * conversions where needed
         */
        rhs.prep(ssize);      /* To */
        rhs.prep(right.size);  /* From */
        if( right.size < ssize && right.signd == false )
            rhs.prep(idZfill);
        else if( right.size < ssize && right.signd == true )
            rhs.prep(idSgnEx);
        else if( right.size > ssize && right.signd == false )
            rhs.prep(idTruncu);
        else if( right.size > ssize && right.signd == true )
            rhs.prep(idTruncs);
        else assert(0);
        right.setSize(ssize);
        rightpart.clear();
        computeTypes(rhs, right, rightpart); // little inefficient, really
    }

    left = computeTypes(lhs, left, leftpart);
    if( !(left == right) ) {
        if( left.isForcibleTo(right) ) {
            /* special case to deal with the fact that sparc needs this... */
            if( left.type == INTEGER ) {
                rightpart.insert(rightpart.begin(),left);
                rhs.prep(idForceInt);
            } else if( left.type == FLOATP ) {
                rightpart.insert(rightpart.begin(),left);
                rhs.prep(idForceFlt);
            } else { /* UMMM??? */
                cerr << "Incredibly unexpected type on LHS of RT Assign: "
                     << left.sprint() << "\n";
            }
            cerr << "Warning: (recovering) Type mismatch in RT Assign: "
                 << left.sprint() << " := " << right.sprint() << "\n  in ";
            right = left;
        } else {
            cerr << "Type mismatch in RT Assign: " << left.sprint() << " := "
                 << right.sprint() << "\n  in";
        }
        rta.print(cerr);
        cerr << "\n";
    }
    PartialType root = left + right;

    if( !left.hasErr ) 
        distributeTypes( lhs, root, leftpart, result );
    distributeTypes( rhs, root, rightpart, result );
    
#ifdef DEBUG_TYPES
    if( !root.isFinal() )
        fprintf( stderr, "Warning: Expression could not be fully typed: %s\n",
                 lhs.sprint().c_str() );
#endif
    return result;

#if 0    
    vector<Type> left, right;

    right = computeRHSTypes(*rta.getRHS(), rta.getSize());
    if( rta.getLHS() ) {
        left = computeLHSTypes(*rta.getLHS());
        if( left.size() == 0 )
            return right;
        if( left[0] != right[0] ) {
        }
        left.insert( left.end(), right.begin(), right.end() );
        return left;
    }
    return right;
#endif
}


/*
 * General notes: This is used by the register access routines to
 * determine which actual blocks of registers a particular expression
 * can access. It's not breathtakingly elegant (just runs through all
 * possible values), but it does have the advantage of being at least
 * nominally correct :) (Doing this in general with sets is somewhat
 * complicated by the ternary expressions).
 *
 * This does depend on the fact that the input sets are generally
 * pretty small - in fact the evaluator will currently reject
 * expressions representing a large range (such as %g1).
 *
 * Generalizing this to deal (correctly) with any kind of expression
 * is left as an exercise for the reader...
 */

int RTLInstDict::evaluateSemStr( SSCIT &it, SSCIT &end, map<int,int> &params,
                                map<int,int> &regs ) const
{
    int idx = *it++;
    const SemItem &si = theSemTable[idx];
    
    switch( si.cat ) {
        case cCONSTANT:
            switch(idx) {
                case idIntConst:
                    return *it++;
            }
            break;
        case cOPERATOR:
            int a,b,c;
            a = evaluateSemStr( it, end, params, regs );
            if( si.iNumVarArgs > 1 )
                b = evaluateSemStr( it, end, params, regs );
            if( si.iNumVarArgs > 2 )
                c = evaluateSemStr( it, end, params, regs );
            switch(idx) {
                case idPlus: return a+b;
                case idMinus: return a-b;
                case idMult: return (unsigned)a*(unsigned)b;
                case idDiv: return b==0?0:(unsigned)a/(unsigned)b;
                case idMults: return a*b;
                case idDivs: return b==0?0:a/b;
                case idMod: return (unsigned)a%(unsigned)b;
                case idMods: return a%b;
                case idNeg: return ~a;
                case idAnd: return a&&b;
                case idOr: return a||b;
                case idEquals: return a==b;
                case idNotEqual: return a!=b;
                case idLess: return a<b;
                case idGtr: return a>b;
                case idLessEq: return a<=b;
                case idGtrEq: return a>=b;
                case idLessUns: return (unsigned)a<(unsigned)b;
                case idGtrUns: return (unsigned)a>(unsigned)b;
                case idLessEqUns: return (unsigned)a<=(unsigned)b;
                case idGtrEqUns: return (unsigned)a>=(unsigned)b;
                case idNot: return !a;
                case idSignExt: return a;
                case idBitAnd: return a&b;
                case idBitOr: return a|b;
                case idBitXor: return a^b;
                case idShiftL: return a<<b;
                case idShiftR: return (unsigned)a>>b;
                case idShiftRA: return a>>b;
                case idTern: return a?b:c;
                default: break;
            }
            break;
        case cOPERATION:
            switch( idx ) {
                case idRegOf:
                    if( *it == idTemp ) {
                        it++;
                        idx = *it++;
                        if( params.find(idx) != params.end() )
                            return params[idx];
                        fprintf( stderr, "Failed to match r[%s]\n",
                                 theSemTable[idx].sName.c_str() );
                        abort();
                    } else {
                        assert( *it == idIntConst );
                        it++;
                        idx = *it++;
                        if( regs.find(idx) != regs.end() )
                            return regs[idx];
                        fprintf( stderr, "Failed to match regof %d\n", idx );
                        abort();
                    }
                    break;
                case idParam:
                    idx = *it++;
                case idPC: case idAFP: case idAGP:
                case idZF: case idCF: case idNF: case idOF:
                case idFZF: case idFLF: case idFGF: case idCTI:
                    if( params.find(idx) != params.end() )
                        return params[idx];
                    fprintf( stderr, "Failed to match param %d\n", idx );
                    abort();
                    break;
                case idSize:
                    it++;
                    return evaluateSemStr(it,end,params, regs);
            }
            break;
        default:
            break;
    }
    return 0;
}

int RTLInstDict::evaluateSemStr( const SemStr &ss, map<int,int> &params,
                                map<int,int> &regs ) const
{
    SSCIT in = ((SemStr &)ss).getIndices()->begin();
    SSCIT end = ((SemStr &)ss).getIndices()->end();
    return evaluateSemStr( in, end, params, regs );
}

typedef map<int,set<int> > valsets;

static int evalcb_head(list<int> &l)
{
    assert(l.size()==1);
    return *l.begin();
}

static int evalcb_range(list<int> &l)
{
    assert(l.size()==2);
    return *l.rbegin() - *l.begin() + 1;
}

set<int> RTLInstDict::evaluateSemStr( const SemStr &ss ) const
{
    return evaluateSemStr( list<SemStr>(1,ss), evalcb_head );
}

set<int> RTLInstDict::evaluateBitsliceRange( const SemStr &lo,
                                             const SemStr &hi ) const
{
    list<SemStr> l;
    l.push_back(lo);
    l.push_back(hi);
    return evaluateSemStr( l, evalcb_range );
}

set<int> RTLInstDict::evaluateSemStr( const list<SemStr> &lss,
                                      int (*callback)(list<int> &) ) const
{
    map<int,int> params;
    map<int,int> regs;
    valsets pvalues, rvalues;
    set<int> results;

    /* Build param list */
    SemStr match(2,idParam,WILD);
    list<SemStr *> result;
    for( list<SemStr>::const_iterator lit = lss.begin();
         lit != lss.end(); lit++ ) {
        ((SemStr *)&*lit)->searchAll(match,result);
    }
    for( list<SemStr *>::iterator it = result.begin();
         it != result.end(); it++ ) {
        int parm = (*it)->getSecondIdx();
        if( DetParamMap.find(parm) != DetParamMap.end() ) {
            if( PARAMETER(parm).regIdx.size() > 0 ) {
                pvalues[parm] = PARAMETER(parm).regIdx;
            } else {
                int sz = PARAMETER(parm).type.getSize();
                if( sz > 8 ) {
                    fprintf( stderr, "Large parameter subexpressions not supported in evaluateSemStr\n" );
                    abort();
                }
                for( int i=0; i<(1<<sz); i++ )
                    pvalues[parm].insert(i);
            }
            params[parm] = *pvalues[parm].begin();
        }
    }
    result.clear();
    match = SemStr(3,idRegOf,WILD,WILD);
    for( list<SemStr>::const_iterator lit = lss.begin();
         lit != lss.end(); lit++ ) {
        ((SemStr *)&*lit)->searchAll(match,result);
    }
    for( list<SemStr *>::iterator it = result.begin();
         it != result.end(); it++ ) {
        if( (*it)->getSecondIdx() == idTemp ) {
            int parm = (*it)->getThirdIdx();
            int sz = Type::getTempType(theSemTable[parm].sName).getSize();
            if( sz > 8 ) {
                fprintf( stderr, "Large temporary subexpressions not supported in evaluateSemStr\n" );
                abort();
            }
            for( int i=0; i<(1<<sz); i++ )
                pvalues[parm].insert(i);
            params[parm] = *pvalues[parm].begin();
        } else if( (*it)->getSecondIdx() != idIntConst ) {
            fprintf( stderr, "Complex register subexpressions not supported in evaluateSemStr\n" );
            abort();
        }
        int reg = (*it)->getThirdIdx();
        int sz = REGISTER(reg).g_size();
        if( sz > 8 ) {
            fprintf( stderr, "Large register subexpressions not supported in evaluateSemStr\n" );
            abort();
        }
//        for( int i= -(1<<(sz-1)); i < 1<<(sz-1); i++ )
        for( int i=0; i<(1<<sz); i++ )
            rvalues[reg].insert(i);
        regs[reg] = *rvalues[reg].begin();
    }

    /* Run through all values */
    bool done = false;
    do {
        list<int> run;
        for( list<SemStr>::const_iterator lit = lss.begin();
             lit != lss.end(); lit++ ) {
            run.push_back( evaluateSemStr( *lit, params, regs ) );
        }
        results.insert( callback(run) );
        
        done = true;
        for( map<int,int>::iterator it = params.begin();
             it != params.end(); it++ ) {
            set<int>::iterator vsit = pvalues[it->first].find(it->second);
            vsit++;
            if( vsit == pvalues[it->first].end() ) {
                vsit = pvalues[it->first].begin();
                it->second = *vsit;
            } else {
                it->second = *vsit;
                done = false;
                break;
            }
        }
        if( done ) {
            for( map<int,int>::iterator it = regs.begin();
                 it != regs.end(); it++ ) {
                set<int>::iterator vsit = rvalues[it->first].find(it->second);
                vsit++;
                if( vsit == rvalues[it->first].end() ) {
                    vsit = rvalues[it->first].begin();
                    it->second = *vsit;
                } else {
                    it->second = *vsit;
                    done = false;
                    break;
                }
            }
        }
    } while( !done );
    
    return results;
}

/*
 * Given a set of registers, return the parameter id of a register param
 * that contains those registers, or -1 if no such parameter exists.
 */
int RTLInstDict::matchRegIdx( const SemStr &ss ) const
{
    set<int> regs = evaluateSemStr( ss );
    
    for( map<int,ParamEntry>::const_iterator parm = DetParamMap.begin();
         parm != DetParamMap.end(); parm++ ) {
        unsigned count = 0;
        if( parm->second.regIdx.size() == 0 )
            continue;
        for( set<int>::iterator it = regs.begin(); it != regs.end(); it++ ) {
            if( parm->second.regIdx.find(*it) != parm->second.regIdx.end() ) {
                count++;
            }
        }
        if( count == regs.size() )
            return parm->first;
    }

    fprintf( stderr, "Register expression r[%s] can't be matched to any known blocks\n",
             ss.sprint().c_str() );
    fprintf( stderr, "Map was: { " );
    for( set<int>::iterator it = regs.begin(); it != regs.end(); it++ ) {
        if( DetRegMap.find(*it) == DetRegMap.end() ) {
            fprintf( stderr, "r[%d], ", *it );
        } else {
            fprintf( stderr, "%s, ", DetRegMap.find(*it)->second.g_name()  );
        }
    }
    fprintf( stderr, "}\n" );
             
    return -1;
}

/* Given a pair of semantic strings representing the second and third
 * (respectively) arguments to a bitslice expression, return the
 * maximum possible width of the range
 */

