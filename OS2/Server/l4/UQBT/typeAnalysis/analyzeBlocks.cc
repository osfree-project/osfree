/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * File:       analyzeBlocks.cc
 * OVERVIEW:   This file contains the implementation of all the data structures'
 *             member functions used for type analysis.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/
 
 /*
 * $Revision: 1.12 $
 * 28 Aug 01 - Bernard: initial version
 * 14 Sep 01 - Cristina: added printType() method for UDChain class.  
 */

#include <string>
#include <map>
#include <list>
#include <iostream.h>
#include <strstream>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include "analyzeBlocks.h"


/******************************************************************
Member Functions for the UDElement class
*******************************************************************/

/*==============================================================================
 * FUNCTION:        UDElement::UDElement
 * OVERVIEW:        Constructor
 * PARAMETERS:      Address, instruction number and use/define
 * RETURNS:         None
 *============================================================================*/
UDElement::UDElement(ADDRESS inRtlListAddress, Byte inInstructNumber,
                     defUse inDefUse) {

    instructNumber = inInstructNumber;	
    rtlListAddress = inRtlListAddress;
    elementUD = inDefUse;
}

/*==============================================================================
 * FUNCTION:        UDElement::~UDElement
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         None
 *============================================================================*/
UDElement::~UDElement(){
    // Nothing needs to be done
}   

/*==============================================================================
 * FUNCTION:        UDElement::outputToStringStream
 * OVERVIEW:        Outputs formatted data to the strstream
 * PARAMETERS:      strstream& instream
 * RETURNS:         None
 *============================================================================*/
void UDElement::outputToStringStream(strstream& inStream){    
    if (elementUD == DEFINE_TYPE)
        // Should never get here, define is already taken 
	// care of by UDChain. There should never be 2
	// defines in a chain
        inStream << " Define ";
    else
    	inStream << " Use ";
    inStream << "@ " << hex << rtlListAddress << " " << dec 
             << (int)instructNumber;	
    inStream <<"\n";

}


/******************************************************************
Member Functions for the UDChain class
*******************************************************************/

/*==============================================================================
 * FUNCTION:        UDChain::UDChain
 * OVERVIEW:        Constructor
 * PARAMETERS:      Address, instruction number and use/define
 * RETURNS:         None
 *============================================================================*/
UDChain::UDChain(ADDRESS rtlListAddress, Byte instructNumber,
                 defUse inDefUse){

    chainType = INT_TYPE; 
    		 
    UDElement * insertElement = new UDElement(rtlListAddress, 
                                              instructNumber, inDefUse);
    
    listOfElements.push_front(insertElement);    	 
}	

/*==============================================================================
 * FUNCTION:        UDChain::~UDChain
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         None
 *============================================================================*/
UDChain::~UDChain(){
    list<UDElement*>::iterator listIt;

    // Need to deallocate all the elements in the UDChain    
    for (listIt = listOfElements.begin(); 
         listIt != listOfElements.end(); 
	 ++listIt)
        delete *listIt;    
}

/*==============================================================================
 * FUNCTION:        UDChain::addElement
 * OVERVIEW:        Add additional UDElements to the chain
 * PARAMETERS:      Address, instruction number and use/define
 * RETURNS:         None
 *============================================================================*/
void UDChain::addElement(ADDRESS rtlListAddress, Byte instructNumber,
                         defUse inDefUse) {
		    
    UDElement * insertElement = new UDElement(rtlListAddress, 
                                              instructNumber, inDefUse);    
        
    listOfElements.push_front(insertElement); 		    		    
}


/*==============================================================================
 * FUNCTION:        UDChain::numOfElements
 * OVERVIEW:        Return the number of UDElements in chain
 * PARAMETERS:      None
 * RETURNS:         number of elements
 *============================================================================*/
int UDChain::numOfElements(){
    return listOfElements.size();
}


/*==============================================================================
 * FUNCTION:        UDChain::outputToStringStream
 * OVERVIEW:        Writes formatted data to the strstream
 * PARAMETERS:      strstream& inStream
 * RETURNS:         None
 *============================================================================*/
void UDChain::outputToStringStream(strstream& inStream){
    
    
    // Reverse iterator needed because we want to iterate
    // the list backwards. This is because the address 
    // is stored backwards but we want to display it forwards
    list<UDElement*>::reverse_iterator listIt;
    
    // We are checking whether the address is 0, which means it
    // is a parameter. We want to print out "param" instead of
    // address 0
    UDElement* tempElement = listOfElements.back();
    if (tempElement != NULL){
        if (tempElement->elementUD == DEFINE_TYPE){
	    if (tempElement->rtlListAddress == 0){
	        inStream << "param   ";
	    }
	    else {
	        inStream << hex << tempElement->rtlListAddress << dec << "   ";
	    }
	} 
	else {
	    inStream << "<no_def>";
	}   	
    }
    inStream << " --> ";    
    
    
    // The following prints out type information. This is not 100%
    // correct! The size is assumed to be 32 bits. We actually need
    // to get the size information from the RTAssgn, but this is
    // not implemented yet.
    if (chainType == INT_TYPE){
        inStream <<" <32i>  ";	 
    }
    else if (chainType == POINTER_D) {
    	inStream <<" <32pd> ";
    }
    else if (chainType == POINTER_I) {
    	inStream <<" <32pi> ";
    }
    else if (chainType == FLOAT_TYPE) {
    	inStream <<" <32f>  ";
    }
        
    string tabString = "\t\t\t               ";	
    
    // If tabFlag is true, then the tabs do not exist
    // yet for this line and needs to be added
    bool tabFlag = false;
    
    for (listIt = listOfElements.rbegin();
         listIt != listOfElements.rend();
	 ++listIt) {
	 
         // Defination already displayed
         if ((*listIt)->elementUD == DEFINE_TYPE)
	     continue;
	 
         if (tabFlag)
	     inStream << tabString;	 
	     
	 (*listIt)->outputToStringStream(inStream);	 	 	 
	 tabFlag = true;
    }
    
    // Printing out whether this contains aliases
    if (weakListOfChains.size() != 0){
        if (tabFlag == true)
	    inStream << tabString;    
        inStream << " Contains Alias\n";
	tabFlag = true; // Since it now needs tabbing	
    }    
    
    // If tabFlag is still false, there must be no use
    // of this register in this basic block
    if (tabFlag == false)            
        inStream << " <no_use>\n";
}


/*==============================================================================
 * FUNCTION:        UDChain::printType
 * OVERVIEW:        Prints the type of this element to the given output stream
 * PARAMETERS:      Output stream
 * RETURNS:         None
 *============================================================================*/
void UDChain::printType (strstream &oStream)
{
    // The following prints out type information. This is not 100%
    // correct! The size is assumed to be 32 bits. We actually need
    // to get the size information from the RTAssgn, but this is
    // not implemented yet.
    if (this->chainType == INT_TYPE){
        oStream <<"<32i>  ";
    }
    else if (this->chainType == POINTER_D) {
        oStream <<"<32pd> ";
    }
    else if (this->chainType == POINTER_I) {
        oStream <<"<32pi> ";
    }
    else if (this->chainType == FLOAT_TYPE) {
        oStream <<"<32f>  ";
    }
}


/******************************************************************
Member Functions for the BBBlock class
*******************************************************************/

/*==============================================================================
 * FUNCTION:        BBBlock::BBBlock
 * OVERVIEW:        Constructor
 * PARAMETERS:      None
 * RETURNS:         None
 *============================================================================*/
BBBlock::BBBlock(){
    // Just init values
    callInfoStruct = NULL;
    endsWithARet = false;
}

/*==============================================================================
 * FUNCTION:        BBBlock::deleteListFromMap
 * OVERVIEW:        Used by destructor to delete lists from maps
 * PARAMETERS:      Address, instruction number and use/define
 * RETURNS:         None
 *============================================================================*/
template <class IntSize> void BBBlock::deleteListFromMap(map<IntSize, 
                                                     list<UDChain*>*>& inMap){
    
    map<IntSize, list<UDChain*>*>::iterator mapIt;
        
    for (mapIt = inMap.begin(); mapIt != inMap.end(); ++mapIt){

        list<UDChain*>::iterator listIt;
    
        for (listIt = (mapIt->second)->begin();
	     listIt != (mapIt->second)->end();
	     ++listIt){	        
	     delete (*listIt);
        }	
	delete mapIt->second;    
    }    
}

/*==============================================================================
 * FUNCTION:        BBBlock::~BBBlock
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         None
 *============================================================================*/
BBBlock::~BBBlock(){

    deleteListFromMap<Byte>(mapOfReg);
    deleteListFromMap<ADDRESS>(mapOfMemAddress);
}


/*==============================================================================
 * FUNCTION:        BBBlock::addUDElementCommon
 * OVERVIEW:        Template class that adds an UDElement
 * PARAMETERS:      Register number, address ,instruction number, use/def, map
 *                  to insert UDElement into
 * RETURNS:         None
 *============================================================================*/
template <class IntSize> 
       void BBBlock::addUDElementCommon(IntSize regNum, 
                                        ADDRESS rtlListAddress,
			      		Byte instructNumber,
		                        defUse inDefUse,
				        map<IntSize, list<UDChain*>*>& inMap){

    map<IntSize, list<UDChain*>*>::iterator mapIt = inMap.find(regNum);

    // Found!
    if (mapIt != inMap.end()) {    
        // Check if it is a define or a use
	if (inDefUse == DEFINE_TYPE){
	    
	    // Create a new element in the list	    
	    UDChain * insertChain = new UDChain(rtlListAddress, 
	                                        instructNumber, inDefUse);
	    mapIt->second->push_back(insertChain);	
	}
	else {
	    // Gives me the last UDChain* for this reg and insert
	    // there this UDElement
	    (mapIt->second->back())->addElement(rtlListAddress, 
	                                        instructNumber, inDefUse);
	}           
    }
    else {        
        // First time this register is in the map
	
	// Create the appropriate pointer to chain
	UDChain * insertChain = new UDChain(rtlListAddress, 
	                                    instructNumber, inDefUse);
	
	// Create the pointer to list
	list<UDChain*> * insertList = new list<UDChain*>();			
	
	// Insert the newly create pointer to chain into the list
	insertList->push_back(insertChain);
			
	// Put the pointer to the List in the map			
	inMap[regNum] = insertList;    
    }	
}        						     

/*==============================================================================
 * FUNCTION:        BBBlock::addUDElementReg
 * OVERVIEW:        Calls template class addUDElementCommon
 * PARAMETERS:      Register number, address ,instruction number, use/def
 * RETURNS:         None
 *============================================================================*/
void BBBlock::addUDElementReg(Byte regNum, 
                              ADDRESS rtlListAddress, 
			      Byte instructNumber,			      
			      defUse inDefUse) {
    addUDElementCommon(regNum, rtlListAddress, 
                       instructNumber, inDefUse, mapOfReg);
}

/*==============================================================================
 * FUNCTION:        BBBlock::addUDElementMem
 * OVERVIEW:        Calls template class addUDElementCommon
 * PARAMETERS:      Register number, address ,instruction number, use/def
 * RETURNS:         None
 *============================================================================*/
void BBBlock::addUDElementMem(ADDRESS regNum, 
                              ADDRESS rtlListAddress,
			      Byte instructNumber, 
		              defUse inDefUse) {
    addUDElementCommon(regNum, rtlListAddress, 
                       instructNumber, inDefUse, mapOfMemAddress);
}			      			


/*==============================================================================
 * FUNCTION:        BBBlock::returnGenericChainListAt
 * OVERVIEW:        Returns a list of UDChain of the specified reg/mem address
 * PARAMETERS:      Register number, map to look from
 * RETURNS:         list of UDChain pointers
 *============================================================================*/
template <class IntSize> 
    list<UDChain*>* BBBlock::returnGenericChainListAt(IntSize regNum, 
	                             map<IntSize, list<UDChain*>*>& inMap) {
				  
    map<IntSize, list<UDChain*>*>::iterator mapIt = 
                                        inMap.find(regNum);

    // Found!
    if (mapIt != inMap.end())     
    	return (mapIt->second);
    return NULL;
}	

/*==============================================================================
 * FUNCTION:        BBBlock::returnRegChainListAt
 * OVERVIEW:        Calls template class returnGenericChainListAt
 * PARAMETERS:      Register number
 * RETURNS:         list of UDChain pointers
 *============================================================================*/
list<UDChain*>* BBBlock::returnRegChainListAt(Byte regNum) {
    return (returnGenericChainListAt(regNum, mapOfReg));
}

/*==============================================================================
 * FUNCTION:        BBBlock::returnMemChainListAt
 * OVERVIEW:        Calls template class returnGenericChainListAt
 * PARAMETERS:      Register number
 * RETURNS:         list of UDChain pointers
 *============================================================================*/
list<UDChain*>* BBBlock::returnMemChainListAt(ADDRESS regNum) {
    return (returnGenericChainListAt(regNum, mapOfMemAddress));
}


/*==============================================================================
 * FUNCTION:        BBBlock::returnNoDefGenericChains
 * OVERVIEW:        Return a map of registers that has UDChains with no defines
 * PARAMETERS:      Map to look in to find UDChains with no defines
 * RETURNS:         A map of reg numbers to UDChain pointers
 *============================================================================*/
template <class IntSize> 
    map <IntSize, UDChain*> BBBlock::returnNoDefGenericChains(map<IntSize, 
                                                     list<UDChain*>*>& inMap){

    map<IntSize, UDChain*> returnMap;
	
    map<IntSize, list<UDChain*>*>::iterator mapIt;
    
    for (mapIt = inMap.begin(); mapIt != inMap.end(); ++mapIt){
	
	// Only need to look at the front one
	UDChain* frontChain = mapIt->second->front();
	
	if (frontChain == NULL)  // Just in case
	    continue;
	
	UDElement * lastElement = frontChain->listOfElements.back();
	
	if (lastElement == NULL)  // Just in case
	    continue;
	
	if (lastElement->elementUD == USE_TYPE){
	    returnMap[mapIt->first] = frontChain;
	}	
    }
    return returnMap;
}


/*==============================================================================
 * FUNCTION:        BBBlock::returnNoDefRegChains
 * OVERVIEW:        Calls template class returnNoDefGenericChains
 * PARAMETERS:      None
 * RETURNS:         A map of reg numbers to UDChain pointers
 *============================================================================*/
map<Byte, UDChain*> BBBlock::returnNoDefRegChains(){
    // Always the first UDChain for each reg for returnNoDefRegChains
    return returnNoDefGenericChains(mapOfReg);
}


/*==============================================================================
 * FUNCTION:        BBBlock::returnNoDefMemChains
 * OVERVIEW:        Calls template class returnNoDefGenericChains
 * PARAMETERS:      None
 * RETURNS:         A map of memory address to UDChain pointers
 *============================================================================*/
map<ADDRESS, UDChain*> BBBlock::returnNoDefMemChains(){
    return returnNoDefGenericChains(mapOfMemAddress);
}


/*==============================================================================
 * FUNCTION:        BBBlock::returnLastDefGenericChains
 * OVERVIEW:        Return a map of registers of the last UDChain with defines
 * PARAMETERS:      Map to look in and find UDChains with no defines
 * RETURNS:         A map of reg numbers to UDChain pointers
 *============================================================================*/
template <class IntSize> 
    map <IntSize, UDChain*> BBBlock::returnLastDefGenericChains(
                                    map<IntSize,list<UDChain*>*>& inMap){
    
    map<IntSize, UDChain*> returnMap;
	
    map<IntSize, list<UDChain*>*>::iterator mapIt;
    
    for (mapIt = inMap.begin(); mapIt != inMap.end(); ++mapIt){
	
	// Only need to look at the *back* one
	UDChain* backChain = mapIt->second->back();
	
	if (backChain == NULL)  // Just in case
	    continue;
	
	// Still looking at the last element
	UDElement * lastElement = backChain->listOfElements.back();
	
	if (lastElement == NULL)  // Just in case
	    continue;
	
	// If define type, then add it to the map
	if (lastElement->elementUD == DEFINE_TYPE){
	    returnMap[mapIt->first] = backChain;
	}	
    }
    return returnMap;
}

/*==============================================================================
 * FUNCTION:        BBBlock::returnLastDefRegChains
 * OVERVIEW:        Calls template class returnLastDefGenericChains
 * PARAMETERS:      None
 * RETURNS:         A map of register numbers to UDChain pointers
 *============================================================================*/
map<Byte, UDChain*> BBBlock::returnLastDefRegChains(){
    // Always the last UDChain for each reg for returnLastDefRegChains
    return returnLastDefGenericChains(mapOfReg);
}

/*==============================================================================
 * FUNCTION:        BBBlock::returnLastDefMemChains
 * OVERVIEW:        Calls template class returnLastDefGenericChains
 * PARAMETERS:      None
 * RETURNS:         A map of memory address to UDChain pointers
 *============================================================================*/
map<ADDRESS, UDChain*> BBBlock::returnLastDefMemChains(){
    // Always the last UDChain for each reg for returnLastDefMemChains
    return returnLastDefGenericChains(mapOfMemAddress);
}

/*==============================================================================
 * FUNCTION:        BBBlock::outputToString
 * OVERVIEW:        Return formatted data as C string
 * PARAMETERS:      None
 * RETURNS:         C String
 *============================================================================*/
const char* BBBlock::outputToString(){

    map<Byte, list<UDChain*>*>::const_iterator mapIt;
            
    returnStream << "Registers stored in this Basic Block are:\n";

    list<UDChain*>::const_iterator listIt;    
    for (mapIt = mapOfReg.begin(); mapIt != mapOfReg.end(); ++mapIt) {
    
        // v* registers are mapped to registers 180 and above
	// however, tmp registers are mapped as 255 and down
	// We assume anything above 240 is a tmp and under 240
	// but 180 and above to be a v register.
        if ((int)mapIt->first >= 180 && (int)mapIt->first <= 240){
	    returnStream << "v" << (int)mapIt->first - 180;	    
	}
	else if ((int)mapIt->first > 240){
	    int tmpRegValue = (((int)mapIt->first - 255) * -1) / 2;	    
            returnStream << "tmp";
	    if (tmpRegValue != 0)
	        returnStream << tmpRegValue;
	}
	else {		
	    returnStream << (int)mapIt->first;
	}

	for (listIt = mapIt->second->begin(); 
	     listIt != mapIt->second->end();
	     ++listIt){
	    returnStream << "\t--> Def @ ";
	    (*listIt)->outputToStringStream(returnStream);    	     
	}
    }
    
    returnStream << '\0';
    returnStream.flush();
    
    return returnStream.str();
}


/*==============================================================================
 * FUNCTION:        BBBlock::setTypeOfLastChainForReg
 * OVERVIEW:        Set the type of a Chain
 * PARAMETERS:      Register number, type
 * RETURNS:         None
 *============================================================================*/
void BBBlock::setTypeOfLastChainForReg(Byte regNum, varType inVarType) {
    list<UDChain*>* currentChain = returnRegChainListAt(regNum);
    if (currentChain == NULL)
    	return;
    currentChain->back()->chainType = inVarType;
}


/*==============================================================================
 * FUNCTION:        BBBlock::createCallRegInfo
 * OVERVIEW:        Creates a CallRegInformation object and return a pointer to
 *                  it
 * PARAMETERS:      None
 * RETURNS:         CallRegInformation pointer
 *============================================================================*/
CallRegInformation* BBBlock::createCallRegInfo(){
    callInfoStruct = new CallRegInformation();
    return callInfoStruct;
}

/*==============================================================================
 * FUNCTION:        BBBlock::returnCallRegInfo
 * OVERVIEW:        Return callInfoStruct pointer
 * PARAMETERS:      None
 * RETURNS:         CallRegInformation pointer
 *============================================================================*/
CallRegInformation * BBBlock::returnCallRegInfo(){
    return callInfoStruct;
}

/*==============================================================================
 * FUNCTION:        BBBlock::aliasGenericLink
 * OVERVIEW:        Template function that creates alias links
 * PARAMETERS:      Variable 1 and variable 2 and map where their chains are 
 *                  stored
 * RETURNS:         None
 *============================================================================*/
template <class IntSize> 
    void BBBlock::aliasGenericLink(IntSize inVar1, 
				   IntSize inVar2, 
				   map<IntSize, list<UDChain*>*>& inMap){

    map<IntSize, list<UDChain*>*>::iterator mapIt1 = 
                                        inMap.find(inVar1);   

    map<IntSize, list<UDChain*>*>::iterator mapIt2 = 
                                        inMap.find(inVar2);   
					
    if (mapIt1 == inMap.end() || mapIt2 == inMap.end()){
    	// One or both of the reg not found, just return without doing anything
        return;
    }
    
    // For both variables, get the last chain (because we want to only connect
    // the most recently added chains together   
    mapIt1->second->back()->weakListOfChains.push_back(mapIt2->second->back());
    mapIt2->second->back()->weakListOfChains.push_back(mapIt1->second->back());
}

/*==============================================================================
 * FUNCTION:        BBBlock::aliasRegLink
 * OVERVIEW:        Calls template function aliasGenericLink
 * PARAMETERS:      Register 1 and register 2
 * RETURNS:         None
 *============================================================================*/
void BBBlock::aliasRegLink(Byte inReg1, Byte inReg2){
    aliasGenericLink(inReg1, inReg2, mapOfReg);    
}

/*==============================================================================
 * FUNCTION:        BBBlock::returnGenericAliasChains
 * OVERVIEW:        Returns the alias chains of a variable
 * PARAMETERS:      Map where the chains can be found
 * RETURNS:         list of chain pointers
 *============================================================================*/
template <class IntSize> 
    list<UDChain*> BBBlock::returnGenericAliasChains(
                                          map<IntSize,list<UDChain*>*>& inMap){

    list <UDChain*> returnChain;	
    map<IntSize, list<UDChain*>*>::iterator mapIt;
    
    for (mapIt = inMap.begin(); mapIt != inMap.end(); ++mapIt) {
        list<UDChain*>::iterator listIt;
	
	for (listIt = mapIt->second->begin(); 
	     listIt != mapIt->second->end();
	     ++listIt){
	    if ((*listIt)->weakListOfChains.size() != 0){
	        returnChain.push_back(*listIt);
	    }	     
	}    
    }    
    return returnChain;
}

/*==============================================================================
 * FUNCTION:        BBBlock::returnRegAliasChains
 * OVERVIEW:        Calls template function returnGenericAliasChains
 * PARAMETERS:      None
 * RETURNS:         list of UDChain pointers
 *============================================================================*/
list<UDChain*> BBBlock::returnRegAliasChains(){
    return returnGenericAliasChains(mapOfReg);
}

/*==============================================================================
 * FUNCTION:        BBBlock::propagateRegWithinBB
 * OVERVIEW:        Propagate type information across alias links
 * PARAMETERS:      None
 * RETURNS:         None
 *============================================================================*/
void BBBlock::propagateRegWithinBB(){
    list<UDChain*> aliasChains = returnRegAliasChains();
    
    list<UDChain*>::iterator listIt;
    
    for (listIt = aliasChains.begin();
         listIt != aliasChains.end();
	 ++listIt){
	
	list<UDChain*>::iterator weakListIt;
	 
	for (weakListIt = (*listIt)->weakListOfChains.begin();
	     weakListIt != (*listIt)->weakListOfChains.end(); 
	     ++weakListIt){
	     if ((*weakListIt)->chainType > (*listIt)->chainType){
	         (*listIt)->chainType = (*weakListIt)->chainType;	     
	     }
	     else {
	         (*weakListIt)->chainType = (*listIt)->chainType; 	     
	     }	     	     
	}		 
    }
}



/******************************************************************
Member Functions for the CallRegInformation class
*******************************************************************/

/*==============================================================================
 * FUNCTION:        CallRegInformation::CallRegInformation
 * OVERVIEW:        Constructor
 * PARAMETERS:      None
 * RETURNS:         None
 *============================================================================*/
CallRegInformation::CallRegInformation(){
    calleeProc = NULL;
}

/*==============================================================================
 * FUNCTION:        CallRegInformation::~CallRegInformation
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         None
 *============================================================================*/
CallRegInformation::~CallRegInformation(){
}


/*==============================================================================
 * FUNCTION:        CallRegInformation::addProc
 * OVERVIEW:        Set calleeProc to inProc
 * PARAMETERS:      inProc
 * RETURNS:         None
 *============================================================================*/
void CallRegInformation::addProc(Proc * inProc){
    calleeProc = inProc;
}

/*==============================================================================
 * FUNCTION:        CallRegInformation::addToReturnRegMap
 * OVERVIEW:        Add a register to the returnReg map
 * PARAMETERS:      Register number, pointer to a UDChain
 * RETURNS:         None
 *============================================================================*/
void CallRegInformation::addToReturnRegMap(Byte regNum, UDChain* inUDChain){
    returnReg[regNum] = inUDChain;
}

/*==============================================================================
 * FUNCTION:        CallRegInformation::addToParamRegMap
 * OVERVIEW:        Add a register to the paramReg map
 * PARAMETERS:      Register number, pointer to a UDChain
 * RETURNS:         None
 *============================================================================*/
void CallRegInformation::addToParamRegMap(Byte regNum, UDChain* inUDChain){
    paramReg[regNum] = inUDChain;
}

/*==============================================================================
 * FUNCTION:        CallRegInformation::outputToString
 * OVERVIEW:        Return formatted data as C string
 * PARAMETERS:      None
 * RETURNS:         C String
 *============================================================================*/
const char* CallRegInformation::outputToString(){
        
    returnStream << "CallRegInformation : \n";
    returnStream << "Callee procedure address is : " << hex << (int) calleeProc
                 << dec <<"\n";
        
    returnStream << "Param Registers are : ";
    
    map<Byte, UDChain*>::iterator mapIt;
    
    for (mapIt = paramReg.begin(); mapIt != paramReg.end(); ++mapIt){
		returnStream << "r[" << (int) mapIt->first <<"]:";       
		mapIt->second->printType (returnStream); 
    }
    returnStream <<"\n";
    
    returnStream <<"Return Registers are : ";
    
    for (mapIt = returnReg.begin(); mapIt != returnReg.end(); ++mapIt){
		returnStream << "r[" << (int) mapIt->first <<"]:";       
		mapIt->second->printType (returnStream);
	}

    returnStream << "\n" << '\0';     
    returnStream.flush();
    
    return returnStream.str();
}

