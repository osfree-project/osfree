/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       RTUDStruct.cc
 * OVERVIEW:   Implementation of the methods in the RT class that are required
 *             for type analysis. These methods are in a separate file because
 *             they are optional additions to the RT class. For tools such as
 *             genemu that depends on the RT class but do not require type
 *             analysis, the file typeAnalysisStub.cc will be compiled and
 *             linked instead of this file.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

 /*
 * $Revision: 1.8 $
 * 28 Aug 01 - Bernard: initial version
 */
 
#include "global.h"
#include "hrtl.h"
#include "rtl.h"

/*==============================================================================
 * FUNCTION:        RTAssgn::storeUseDefinStruct
 * OVERVIEW:        Store use/define info in inBlock
 * PARAMETERS:      BBBlock& inBlock, ADDRESS memAddress, int counter
 *                  - memAddress is the address of the source instruction
 *                  - counter is the RT inside the RTL at that memAddress
 *                    since there can be more than one RT at one address
 * RETURNS:         <none>
 *
 *============================================================================*/
void RTAssgn::storeUseDefineStruct(BBBlock& inBlock, 
                                   ADDRESS memAddress, 
				   int counter)
{
    typeLex * inLex;

    if (pLHS->len()){
        inLex = new typeLex(&(pLHS->indices), 
	                    &(pRHS->indices), 
			    memAddress, 
			    counter); 
    }
    else {
        inLex = new typeLex(&(pRHS->indices), memAddress, counter); 
    }
    
    varType paramType = INT_TYPE;    
    LOC_TYPE typeInfo = getType().getType();

    // Should really integrate this with libraryParamPropagation
    switch(typeInfo){
        case TVOID:         break; 
        case INTEGER:       paramType = INT_TYPE;
                            break;
        case FLOATP:        paramType = FLOAT_TYPE;
                            break;
        case DATA_ADDRESS:  paramType = POINTER_D;
                            break;
        case FUNC_ADDRESS:  paramType = POINTER_I;
                            break;
        case VARARGS:       return; // Don't know what to do with this yet
        case BOOLEAN:       break;  // Treated as a integer
        case UNKNOWN:       break;  // Assume integer

        default:            break;  // Shouldn't get here
    }    
    
    typeAnalysis testAnalysis(inLex, &inBlock, paramType);
    testAnalysis.yyparse();
    delete inLex;
}

/*==============================================================================
 * FUNCTION:        RTFlagDef::storeUseDefinStruct
 * OVERVIEW:        Store use/define info in inBlock
 * PARAMETERS:      BBBlock& inBlock, ADDRESS memAddress, int counter
 *                  - memAddress is the address of the source instruction
 *                  - counter is the RT inside the RTL at that memAddress
 *                    since there can be more than one RT at one address
 * RETURNS:         <none>
 *
 *============================================================================*/
void RTFlagDef::storeUseDefineStruct(BBBlock& inBlock, 
                                     ADDRESS memAddress, 
				     int counter){
    // Do nothing since it doesn't use or define any regs
} 

/*==============================================================================
 * FUNCTION:        RTFlagCall::storeUseDefinStruct
 * OVERVIEW:        Store use/define info in inBlock
 * PARAMETERS:      BBBlock& inBlock, ADDRESS memAddress, int counter
 *                  - memAddress is the address of the source instruction
 *                  - counter is the RT inside the RTL at that memAddress
 *                    since there can be more than one RT at one address
 * RETURNS:         <none>
 *============================================================================*/
void RTFlagCall::storeUseDefineStruct(BBBlock& inBlock, 
                                      ADDRESS memAddress, 
				      int counter)
{
    list<SemStr*>::const_iterator i;
    for (i = actuals.begin(); i != actuals.end(); i++) {
	typeLex inLex(&((*i)->indices), memAddress, counter);
        typeAnalysis testAnalysis(&inLex, &inBlock, INT_TYPE);
        testAnalysis.yyparse();
    }
}
