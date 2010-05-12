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
 * FILE:       HRTLUDStruct.cc
 * OVERVIEW:   Implementation of the methods in the HRTL class that are required
 *             for type analysis. These methods are in a separate file because
 *             they are optional additions to the HRTL class. For tools such as
 *             genemu that depends on the HRTL class but do not require type
 *             analysis, the file typeAnalysisStub.cc will be compiled and
 *             linked instead of this file.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

 /*
 * $Revision: 1.11 $
 * 28 Aug 01 - Bernard: initial version
 * 30 Aug 01 - Mike: Parameters are lists now, were vectors
 */

#include "global.h"
#include "hrtl.h"
#include "rtl.h"
#include "prog.h"

/*==============================================================================
 * FUNCTION:        HRTL::storeUseDefineStruct
 * OVERVIEW:        Store use/def info into inBlock.
 * PARAMETERS:      BBBlock& inBlock 
 * RETURNS:         <nothing>
 *============================================================================*/
void HRTL::storeUseDefineStruct(BBBlock& inBlock) {
    int counter = 0;
    for (RT_CIT p = rtlist.begin(); p != rtlist.end(); p++) {
        (*p)->storeUseDefineStruct(inBlock, getAddress(), counter++);
    }
}

/*==============================================================================
 * FUNCTION:        HLJump::storeUseDefineStruct
 * OVERVIEW:        Store use/def info into inBlock.
 * PARAMETERS:      BBBlock& inBlock 
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJump::storeUseDefineStruct(BBBlock& inBlock)
{
    // Returns can all have semantics (e.g. ret/restore)
    if (rtlist.size() != 0)
        HRTL::storeUseDefineStruct(inBlock);

/*  Should never get here

    if (getKind() == HL_RET){
        // no parameters
        return;
    }
*/
    if (pDest != NULL && pDest->getFirstIdx() != idIntConst) {
        // Must be a USE        
        list<int> tempIndices = pDest->indices;
        tempIndices.push_front(idHLCTI);
        typeLex inLex(&tempIndices, getAddress(), 99);
        typeAnalysis testAnalysis(&inLex, &inBlock, INT_TYPE);
        testAnalysis.yyparse();
    }
}    

/*==============================================================================
 * FUNCTION:        HLJcond::storeUseDefineStruct
 * OVERVIEW:        Store use/def info into inBlock.
 * PARAMETERS:      BBBlock& inBlock 
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::storeUseDefineStruct(BBBlock& inBlock)
{
    if (pDest != NULL && pDest->getFirstIdx() != idIntConst) {
        // Must be a USE	
        list<int> tempIndices = pDest->indices;
        tempIndices.push_front(idHLCTI);
        typeLex inLex(&tempIndices, getAddress(), 99);
        typeAnalysis testAnalysis(&inLex, &inBlock, INT_TYPE);
        testAnalysis.yyparse();
    }
    
    // Try storing conditions as well    
    if (pCond){
        typeLex inLex(&(pCond->indices), getAddress(), 99);
        typeAnalysis testAnalysis(&inLex, &inBlock, INT_TYPE);
        testAnalysis.yyparse();	    
    }        
}   

/*==============================================================================
 * FUNCTION:        HLNwayJump::storeUseDefineStruct
 * OVERVIEW:        Store use/def info into inBlock.
 * PARAMETERS:      BBBlock& inBlock 
 * RETURNS:         <nothing>
 *============================================================================*/
void HLNwayJump::storeUseDefineStruct(BBBlock& inBlock)
{
    if (pDest != NULL) {
        // Must be a USE
        list<int> tempIndices = pDest->indices;
        tempIndices.push_front(idHLCTI);
        typeLex inLex(&tempIndices, getAddress(), 99);
        typeAnalysis testAnalysis(&inLex, &inBlock, INT_TYPE);
        testAnalysis.yyparse();
    }
}    


/*==============================================================================
 * FUNCTION:        HLCall::storeUseDefineStruct
 * OVERVIEW:        Store use/def info into inBlock.
 * PARAMETERS:      BBBlock& inBlock 
 * RETURNS:         <nothing>
 *============================================================================*/
void HLCall::storeUseDefineStruct(BBBlock& inBlock)
{
    if (rtlist.size() != 0)
        HRTL::storeUseDefineStruct(inBlock);


    /* For this part, I want to add code to save all the registers
       passed as parameters in the call data structure

       Need to identify the list of registers by looking at the semstr
       and then after parsing do a returnRegChainListAt that register,
       look at only the back of the chain and save the poiner to the
       data structure.
    */
    list<int> paramListNumber;
    CallRegInformation* blockCallRegInfo = inBlock.createCallRegInfo();
  
    for (list<SemStr>::iterator it = params.begin(); it != params.end(); it++) {

        for (list<int>::const_iterator paramIt = it->indices.begin();
            paramIt != it->indices.end(); ++paramIt){
        
            // NOTE! Also need to look at v[], not just r[]
	    // Will definately need to implement this later
	    //
	    // Update: Has been implemented in the parser
            if (*paramIt == idRegOf){
                if(*(++paramIt) == idIntConst){
                    paramListNumber.push_back(*(++paramIt));
                    --paramIt;
                    --paramIt;
                }
                else {
                    --paramIt;
                }
            }
        }


        typeLex tempLex(&(it->indices), getAddress(), 99);
        typeAnalysis testAnalysis(&tempLex, &inBlock, INT_TYPE);
        testAnalysis.yyparse(); 

        list<int>::iterator listIt;
        for (listIt = paramListNumber.begin(); listIt != paramListNumber.end();
             ++listIt) {
            // We just want the last chain that is added
            UDChain* tempInsertChain = 
	                    inBlock.returnRegChainListAt(*listIt)->back();
            
            if (tempInsertChain == NULL)  // Just in case
                continue;
            
            blockCallRegInfo->addToParamRegMap(*listIt, tempInsertChain);
        }
    }


    typeLex * inLex = NULL;
    list<int> tempIndicesRight;
    list<int> tempIndicesLeft;

    // Holds a list of registers numbers that gets
    // returned as a return value
    list<int> returnRegNumber;

    if (pDest != NULL) {
        tempIndicesRight.push_back(idHLCTI);

        // Finding info on the callee function
        // saved this in the call table
        if (pDest->getFirstIdx() == idIntConst){
            blockCallRegInfo->addProc(prog.findProc(getFixedDest()));
        }


        for (list<int>::const_iterator it = pDest->indices.begin();
             it != pDest->indices.end(); ++it){
            tempIndicesRight.push_back(*it);
        }
        if (returnLoc.len() != 0){
            /* For this part, I want to add code to save all the registers
               passed as parameters in the call data structure

               Need to identify the list of registers by looking at the semstr
               and then after parsing do a returnRegChainListAt that register,
               look at only the back of the chain and save the poiner to the
               data structure.
            */
            for (list<int>::const_iterator it = returnLoc.indices.begin();
                 it != returnLoc.indices.end(); ++it){
                tempIndicesLeft.push_back(*it);

                // Note! This can Possibly be a v[], not just
		// a r[]. Will definately need to add later
		//
		// Update: Added in the parser
                if (*it == idRegOf){
                    if(*(++it) == idIntConst){
                        returnRegNumber.push_back(*(++it));
                        --it;
                        --it;
                    }
                    else {
                        --it;
                    }
                }
                
            }
	    
	    // Use 99 for registers used in CTI
            inLex = new typeLex(&tempIndicesLeft, 
	                        &tempIndicesRight, 
				getAddress(), 
				99);
        }
        else {
	    // Use 99 for registers used in CTI	
            inLex = new typeLex(&tempIndicesRight, getAddress(), 99);
        }
        typeAnalysis testAnalysis(inLex, &inBlock, INT_TYPE);
        testAnalysis.yyparse(); 

        delete inLex;

        list<int>::iterator listIt;
        for (listIt = returnRegNumber.begin(); listIt != returnRegNumber.end();
             ++listIt) {
            // We just want the last chain that is added
            UDChain* tempInsertChain = 
	            inBlock.returnRegChainListAt(*listIt)->back();
            
            if (tempInsertChain == NULL)  // Just in case
                continue;
            
            blockCallRegInfo->addToReturnRegMap(*listIt, tempInsertChain);
        }
    }

    // Print the post call RTLs, if any
    // Use instruct number 100 and up for these
    int counter = 99;
    if (postCallRtlist) {
        for (RT_CIT it = postCallRtlist->begin(); it != postCallRtlist->end();
          it++) {
            (*it)->storeUseDefineStruct(inBlock, getAddress(), ++counter);
        }
    }
} 


/*==============================================================================
 * FUNCTION:        HLReturn::storeUseDefineStruct
 * OVERVIEW:        Just basically sets the inBlock to say that this is a 
 *                  BB that contains a RET
 * PARAMETERS:      BBBlock& inBlock 
 * RETURNS:         <nothing>
 *============================================================================*/
void HLReturn::storeUseDefineStruct(BBBlock& inBlock)
{
    // Returns can all have semantics (e.g. ret/restore)
    if (rtlist.size() != 0)
        HRTL::storeUseDefineStruct(inBlock);
	
    inBlock.setRetInfo(true);
}    


/*==============================================================================
 * FUNCTION:        HLScond::storeUseDefineStruct
 * OVERVIEW:        Store use/def info into inBlock.
 * PARAMETERS:      BBBlock& inBlock 
 * RETURNS:         <nothing>
 *============================================================================*/
void HLScond::storeUseDefineStruct(BBBlock& inBlock)
{
    list<int> tempIndices = getDest()->indices;
    tempIndices.push_front(idHLCTI);
    typeLex inLex(&tempIndices, getAddress(), 99);
    typeAnalysis testAnalysis(&inLex, &inBlock, INT_TYPE);
    testAnalysis.yyparse(); 
}


