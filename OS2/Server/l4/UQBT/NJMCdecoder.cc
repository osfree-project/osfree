/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       NJMCdecoder.cc
 * OVERVIEW:   This file contains all the machine independent
 *             decoding functionality.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * $Revision: 1.16 $
 *============================================================================*/ 
/*
 * 17 Oct 00 - Mike: Added overloaded dis_Reg to use numbered (as opposed to
 *              named) registers
 * 27 Jun 01 - Nathan: Added instantiateNamedParam
 * 19 Jul 01 - Simon: Added substituteCallArgs()
 * 19 Oct 01 - Mike: Added unconditionalJump() to replace UNCOND_JUMP macro
 */

#include "global.h"
#include "proc.h"
#include "decoder.h"
#include "ss.h"
#include "rtl.h"
#include "prog.h"
#include "BinaryFile.h"

/**********************************
 * NJMCDecoder methods.
 **********************************/   

/*==============================================================================
 * FUNCTION:       NJMCDecoder::NJMCDecoder
 * OVERVIEW:       Constructor sets the association with the supporting
 *                 dictionaries.
 * PARAMETERS:     rtlDict - reference to the RTLInstDict object that holds
 *                   the summary of the SSL file
 *                 csr - Reference to the CSR object that summarises the PAL
 *                   file
 * RETURNS:        N/A
 *============================================================================*/
NJMCDecoder::NJMCDecoder(RTLInstDict& rtlDict, CSR& csr) :
	rtlDict(rtlDict), csr(csr)
{}

/*==============================================================================
 * FUNCTION:       NJMCDecoder::instantiate
 * OVERVIEW:       Given an instruction name and a variable list of SemStr's
 *                 representing the actual operands of the instruction, use the
 *                 RTL template dictionary to return the instantiated RTL
 *                 representing the semantics of the instruction.
 * PARAMETERS:     name - instruction name
 *                 ... - Semantic String ptrs representing actual operands
 * RETURNS:        an instantiated list of RTs
 *============================================================================*/
list<RT*>* NJMCDecoder::instantiate(ADDRESS pc, const char* name, ...)
{
	// Get the signature of the instruction and extract its parts
	pair<string,unsigned> sig = rtlDict.getSignature(name);
	string opcode = sig.first;
	unsigned numOperands = sig.second;

	// Put the operands into an vector
	vector<SemStr*> actuals(numOperands);
	va_list args;
	va_start(args,name);
	for (unsigned i = 0; i < numOperands; i++)
		actuals[i] = va_arg(args,SemStr*);
	va_end(args);

#ifdef DEBUG_DECODER
	// Display a disassembly of this instruction if necessary
	cout << hex << pc << dec << ": " << name;
	for (vector<SemStr*>::iterator it = actuals.begin();
		it != actuals.end(); it++)
		cout << " " << **it;
	cout << endl;
#endif

	list<RT*>* instance = rtlDict.instantiateRTL(opcode,actuals);

	// Delete the memory used for the actuals
	for (vector<SemStr*>::iterator it = actuals.begin();
		it != actuals.end(); it++)
		delete *it;

	return instance;
}

void NJMCDecoder::substituteCallArgs(const char *name, SemStr* &ss, ...)
{
    if (rtlDict.ParamMap.find(name) == rtlDict.ParamMap.end()) {
        cerr << "No entry for named parameter '" << name << "'\n";
        return;
    }
    ParamEntry &ent = rtlDict.DetParamMap[rtlDict.ParamMap[name]];
    /*if (ent.kind != PARAM_EXPR && ent.kind != PARAM_LAMBDA) {
        cerr << "Attempt to instantiate expressionless parameter '" << name << "'\n";
        return;
    }*/
    
    va_list args;
    va_start(args, ss);
    for (list<int>::iterator it = ent.funcParams.begin();
         it != ent.funcParams.end(); it++) {
        SemStr formal = SemStr(2, idParam, *it);
        SemStr *actual = va_arg(args, SemStr*);
        ss->searchReplaceAll(formal, *actual);
        delete actual;
    }
}

SemStr *NJMCDecoder::instantiateNamedParam(const char *name, ...)
{
    if( rtlDict.ParamMap.find(name) == rtlDict.ParamMap.end() ) {
        cerr << "No entry for named parameter '" << name << "'\n";
        return new SemStr();
    }
    ParamEntry &ent = rtlDict.DetParamMap[rtlDict.ParamMap[name]];
    if( ent.kind != PARAM_EXPR && ent.kind != PARAM_LAMBDA ) {
        cerr << "Attempt to instantiate expressionless parameter '" << name << "'\n";
        return new SemStr();
    }
    SemStr *result = new SemStr(*ent.rta->getRHS());

    va_list args;
    va_start(args,name);
    for( list<int>::iterator it = ent.params.begin();
         it != ent.params.end(); it++ ) {
        SemStr formal = SemStr(2,idParam, *it);
        SemStr *actual = va_arg(args, SemStr *);
        result->searchReplaceAll(formal, *actual);
        delete actual;
    }
    return result;
}

/*==============================================================================
 * FUNCTION:       getLowLevelRTs
 * OVERVIEW:       Decode the instructions in a given range and return the
 *                 concatenation of all their lists of RTs.
 * PARAMETERS:     hostPC - where to start decoding
 *                 length - number of bytes to decode
 *                 pc - the logical address of the pc
 *                 rtlDict - the dictionary of RTL templates used to instantiate
 *                   the RTL for the instruction being decoded
 * RETURNS:        the concatenated lists of RTs for the instruction in the
 *                 range [hostPC .. hostPC+length)
 *============================================================================*/
list<RT*>* NJMCDecoder::getLowLevelRTs(ADDRESS hostPC, int length, ADDRESS pc,
	DecodeResult& result)
{
	list<RT*>* returnList = new list<RT*>;

	for (ADDRESS addr = hostPC; addr != hostPC+length; ) {
		list<RT*>* nextList =decodeLowLevelInstruction(addr,pc,result);
		if (nextList != NULL) {
			returnList->insert(returnList->end(),nextList->begin(),
				nextList->end());
			delete nextList;
		}
		addr = addr + result.numBytes;
		pc = pc + result.numBytes;
	}
	return returnList;
}

/*==============================================================================
 * FUNCTION:       DecodeResult::reset
 * OVERVIEW:       Resets the fields of a DecodeResult to their default values.
 * PARAMETERS:     <none>
 * RETURNS:        <nothing>
 *============================================================================*/
void DecodeResult::reset()
{
	numBytes = 0;
	type = NCT;
	valid = true;
	rtl = NULL;
    forceOutEdge = 0;	
}

/*==============================================================================
 * These are functions used to decode instruction operands into
 * SemStrs.
 *============================================================================*/

/*==============================================================================
 * FUNCTION:        NJMCDecoder::dis_Reg
 * OVERVIEW:        Converts a named register to a suitable expression.
 * PARAMETERS:      reg - the register name, e.g. %f4 or %ebx
 * RETURNS:         the SemStr for the register NUMBER (e.g. "int 36" for %f4)
 *============================================================================*/
SemStr* NJMCDecoder::dis_Reg(const char* reg)
{
	if (rtlDict.RegMap.find(reg) == rtlDict.RegMap.end()) {
		ostrstream ost;
		ost << "register `" << reg << "' is unknown";
		error(str(ost));
	}

    int regNum = rtlDict.RegMap[reg];
	SemStr* expr = new SemStr;
	expr->push(idIntConst);
	expr->push(regNum);

	return expr;
}

/*==============================================================================
 * FUNCTION:        NJMCDecoder::dis_Reg
 * OVERVIEW:        Converts a numbered register to a suitable expression.
 * NOTE:            Overloaded parameter
 * PARAMETERS:      reg - the register number, e.g. 0 for eax
 * RETURNS:         the SemStr for the register NUMBER (e.g. "int 36" for %f4)
 *============================================================================*/
SemStr* NJMCDecoder::dis_Reg(int regNum)
{
      SemStr* expr = new SemStr;
      expr->push(idIntConst);
      expr->push(regNum);
      return expr;
}

/*==============================================================================
 * FUNCTION:        NJMCDecoder::dis_Num
 * OVERVIEW:        Converts a number to a SemStr expression.
 * PARAMETERS:      num - a number
 * RETURNS:         the SemStr representation of the given number
 *============================================================================*/
SemStr* NJMCDecoder::dis_Num(unsigned num)
{
	SemStr* expr = new SemStr();

	expr->push(idIntConst);
	expr->push(num);
	return expr;
}

/*==============================================================================
 * FUNCTION:        NJMCDecoder::getRtlDict
 * OVERVIEW:        Returns a reference to the RTLInstDict object that
 *                      summarises the SSL file
 * PARAMETERS:      <none>
 * RETURNS:         the reference to the RTLInstDict object
 *============================================================================*/
RTLInstDict& NJMCDecoder::getRtlDict()
{
    return rtlDict;
}

/*==============================================================================
 * FUNCTION:        NJMCDecoder::unconditionalJump
 * OVERVIEW:        Process an unconditional jump instruction
 *                  Also check if the destination is a label
 * PARAMETERS:      <none>
 * RETURNS:         the reference to the RTLInstDict object
 *============================================================================*/
void NJMCDecoder::unconditionalJump(const char* name, int size,
  ADDRESS relocd, UserProc* proc, int delta, ADDRESS pc, list<RT*>* RTs,
  DecodeResult& result) {
    ADDRESS dest = relocd-delta;
    // Check for a pointer to a label, but not a branch that happens to be
    // to the top of this function (this would a rare function with no
    // prologue).
    const char* fname = prog.pBF->SymbolByAddress(dest);
    if ((fname == 0) || (proc == 0) || (proc->getNativeAddress() == dest)) {
        HLJump* jump = new HLJump(pc, RTs);
        result.rtl = jump;
        result.numBytes = size;
        jump->setDest(dest);
        SHOW_ASM(name<<" "<<relocd)
    } else {
        // The jump is to another function. Handle this as a call/return pair
        HLCall* call = new HLCall(pc, 0, RTs);
        result.rtl = call;
        result.numBytes = size;
        call->setDest(dest);
        call->setReturnAfterCall(true);
        SHOW_ASM(name<<" "<<fname)
    }
}
