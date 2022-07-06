/**************************************************************************
 *
 *  Copyright 2015, 2017 Yuri Prokushev
 *
 *  This file is part of osFree project
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef SOM_Module_scemit_Source
#define SOM_Module_scemit_Source
#endif

#define SOMTEmitC_Class_Source

#include <time.h>

#include "scemit.xih"
#include "scconst.xh"
#include "scattrib.xh"
#include "scbase.xh"

SOM_Scope boolean SOMLINK somtGenerateSections(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTClassEntryC *cls = _somtTargetClass;
  SOMTEmitCMethodDebug("SOMTEmitC","somtGenerateSections");
  
  somSelf->somtFileSymbols();
  
  somSelf->somtEmitProlog();
  
  if (cls != (SOMTClassEntryC *) NULL) 
  {
    somSelf->somtScanBaseIncludes("somtEmitBaseIncludesProlog",
                                  "somtEmitBaseIncludes",
                                  "somtEmitBaseIncludesEpilog");
                                   
    somSelf->somtEmitMetaInclude();
    
    somSelf->somtEmitClass();
    
    somSelf->somtScanBases("somtEmitBaseProlog",
                           "somtEmitBase",
                           "somtEmitBaseEpilog");
  
    somSelf->somtEmitMeta();
  }
  
  somSelf->somtScanConstants("somtEmitConstantProlog",
                             "somtEmitConstant",
                             "somtEmitConstantEpilog");

  somSelf->somtScanTypedefs("somtEmitTypedefProlog",
                            "somtEmitTypedef",
                            "somtEmitTypedefEpilog");

  somSelf->somtScanStructs("somtEmitStructProlog",
                           "somtEmitStruct",
                           "somtEmitStructEpilog");

  somSelf->somtScanUnions("somtEmitUnionProlog",
                          "somtEmitUnion",
                          "somtEmitUnionEpilog");

  somSelf->somtScanEnums("somtEmitEnumProlog",
                         "somtEmitEnum",
                         "somtEmitEnumEpilog");

  if (cls != (SOMTClassEntryC *) NULL) 
  {
    somSelf->somtScanAttributes("somtEmitAttributeProlog",
                                "somtEmitAttribute",
                                "somtEmitAttributeEpilog");

    somSelf->somtScanMethods(NULL,
                             "somtEmitMethodsProlog",
                             "somtEmitMethods",
                             "somtEmitMethodsEpilog",
                             TRUE);
                            
    somSelf->somtEmitRelease();
  
    somSelf->somtScanPassthru(TRUE,
                              "somtEmitPassthruProlog",
                              "somtEmitPassthru",
                              "somtEmitPassthruEpilog");

    somSelf->somtScanPassthru(FALSE,
                              "somtEmitPassthruProlog",
                              "somtEmitPassthru",
                              "somtEmitPassthruEpilog");

    somSelf->somtScanData("somtEmitDataProlog",
                          "somtEmitData",
                          "somtEmitDataEpilog");
  }
  
  if (_somtTargetModule != (SOMTModuleEntryC *) NULL) 
  {
    somSelf->somtScanInterfaces("somtEmitInterfaceProlog",
                                "somtEmitInterface", 
                                "somtEmitInterfaceEpilog");

    somSelf->somtScanModules("somtEmitModuleProlog",
                             "somtEmitModule", 
                             "somtEmitModuleEpilog");
  }

  somSelf->somtEmitEpilog();

  return TRUE;
}

  /*
For emitters having a target module:
somtEmitProlog
somtGenerateSections Method
somtEmitConstantProlog
somtEmitConstant (for each constant)
somtEmitConstantEpilog
somtEmitTypedefProlog
somtEmitTypedef (for each typedef)
somtEmitTypedefEpilog
somtEmitStructProlog
somtEmitStruct (for each struct)
somtEmitStructEpilog
somtEmitUnionProlog
somtEmitUnion (for each union)
somtEmitUnionEpilog
somtEmitEnumProlog
somtEmitEnum (for each enum)
somtEmitEnumEpilog
somtEmitInterfaceProlog
somtEmitInterface (for each interface)
somtEmitInterfaceEpilog
somtEmitModuleProlog
somtEmitModule (for each embedded module)
somtEmitModuleEpilog
somtEmitEpilog

Repeating sections (such as somtEmitBase) are emitted using the corresponding
somtScan<Section> Methods.
To rearrange the order of sections, or to add or delete sections in your emitter, override the
somtGenerateSections method.
*/

SOM_Scope FILE *SOMLINK somtOpenSymbolsFile(SOMTEmitC SOMSTAR somSelf,
	                                                 /* in */ string fileName,
	                                                 /* in */ string mode)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtOpenSymbolsFile");
  
  return fopen(fileName, mode);
}

SOM_Scope string SOMLINK somtGetGlobalModifierValue(SOMTEmitC SOMSTAR somSelf,
	                                                         /* in */ string modifierName)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtGetGlobalModifierValue");
  
  return NULL;
}

SOM_Scope void SOMLINK somtFileSymbols(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  time_t     now = time(0);
  struct tm  tstruct;
  char       buf[80];
  SOMTClassEntryC *cls = _somtTargetClass;
  SOMTTemplateOutputC *templ = _somtTemplate;
  SOMTEmitCMethodDebug("SOMTEmitC","somtFileSymbols");

  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    
  templ->somtSetSymbolCopyBoth("timeStamp", buf);
  
  if (cls != (SOMTClassEntryC *) NULL) 
  {
    cls->somtSetSymbolsOnEntry(somSelf, "class");
  }
}

SOM_Scope void SOMLINK somtEmitProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitProlog");
  //somPrintf(_somtTemplate->somtGetSymbol("prologSN"));
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("prologSN"));
}

SOM_Scope void SOMLINK somtEmitClass(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitClass");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("classSN"));
}

SOM_Scope void SOMLINK somtEmitModule(SOMTEmitC SOMSTAR somSelf,
	                                           /* in */ SOMTModuleEntryC SOMSTAR mod)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitModule");
  
  return;
}

SOM_Scope void SOMLINK somtEmitPassthruProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitPassthruProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("passthruPrologSN"));
}

SOM_Scope void SOMLINK somtEmitConstantEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitConstantEpilog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("constantEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitModuleEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitModuleEpilog");
  
  return;
}

SOM_Scope void SOMLINK somtEmitData(SOMTEmitC SOMSTAR somSelf,
	                                         /* in */ SOMTDataEntryC SOMSTAR entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitData");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("dataSN"));
}

SOM_Scope void SOMLINK somtEmitInterface(SOMTEmitC SOMSTAR somSelf,
	                                              /* in */ SOMTClassEntryC SOMSTAR intfc)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitInterface");
  
  return;
}

SOM_Scope void SOMLINK somtEmitTypedefEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitTypedefEpilog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("typedefEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitMethod(SOMTEmitC SOMSTAR somSelf,
	                                           /* in */ SOMTMethodEntryC SOMSTAR entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitMethod");
  
  entry->somtSetSymbolsOnEntry(somSelf, "method");
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("methodSN"));
}

SOM_Scope void SOMLINK somtEmitAttribute(SOMTEmitC SOMSTAR somSelf,
	                                              /* in */ SOMTAttributeEntryC SOMSTAR att)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitAttribute");

  att->somtSetSymbolsOnEntry(somSelf, "attribute");
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("attributeSN"));
}

SOM_Scope void SOMLINK somtEmitEnumProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitEnumProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("enumPrologSN"));
}

SOM_Scope void SOMLINK somtEmitAttributeProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitAttributeProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("attributePrologSN"));
}

SOM_Scope void SOMLINK somtEmitBaseIncludesProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitBaseIncludesProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("baseIncludesPrologSN"));
}

SOM_Scope void SOMLINK somtEmitStructProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitStructProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("structPrologSN"));
}

SOM_Scope void SOMLINK somtEmitUnionProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitUnionProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("unionPrologSN"));
}

SOM_Scope void SOMLINK somtEmitDataEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitDataEpilog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("dataEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitPassthruEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitPassthruEpilog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("passthruEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitEpilog");
  

  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("epilogSN"));

}

SOM_Scope void SOMLINK somtEmitEnum(SOMTEmitC SOMSTAR somSelf,
	                                         /* in */ SOMTEnumEntryC SOMSTAR en)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitEnum");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("enumSN"));
}

SOM_Scope void SOMLINK somtEmitBaseIncludes(SOMTEmitC SOMSTAR somSelf,
	                                                 /* in */ SOMTBaseClassEntryC SOMSTAR base)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitBaseIncludes");

  base->somtSetSymbolsOnEntry(somSelf, "base");
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("baseIncludesSN"));
}

SOM_Scope void SOMLINK somtEmitAttributeEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitAttributeEpilog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("attributeEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitBaseEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitBaseEpilog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("baseEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitModuleProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitModuleProlog");
  
  return;
}

SOM_Scope void SOMLINK somtEmitStruct(SOMTEmitC SOMSTAR somSelf,
	                                           /* in */ SOMTStructEntryC SOMSTAR struc)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitStruct");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("structSN"));
}

SOM_Scope void SOMLINK somtEmitConstant(SOMTEmitC SOMSTAR somSelf,
	                                             /* in */ SOMTConstEntryC SOMSTAR con)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitConstant");

  con->somtSetSymbolsOnEntry(somSelf, "constant");
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("constantSN"));
}                                               

SOM_Scope void SOMLINK somtEmitConstantProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitConstantProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("constantPrologSN"));
}

SOM_Scope void SOMLINK somtEmitInterfaceProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitInterfaceProlog");
  
  return;
}

SOM_Scope void SOMLINK somtEmitDataProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitDataProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("dataPrologSN"));
}

SOM_Scope void SOMLINK somtEmitPassthru(SOMTEmitC SOMSTAR somSelf,
	                                             /* in */ SOMTPassthruEntryC SOMSTAR entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitPassthru");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("passthruSN"));
}

SOM_Scope void SOMLINK somtEmitUnion(SOMTEmitC SOMSTAR somSelf,
	                                          /* in */ SOMTUnionEntryC SOMSTAR un)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitUnion");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("unionSN"));
}

SOM_Scope void SOMLINK somtEmitMeta(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitMeta");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("metaSN"));
}

SOM_Scope void SOMLINK somtEmitTypedefProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitTypedefProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("typedefPrologSN"));
}

SOM_Scope void SOMLINK somtEmitBaseIncludesEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitBaseIncludesEpilog");
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("baseIncludesEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitEnumEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitEnumEpilog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("enumEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitBaseProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitBaseProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("basePrologSN"));
}

SOM_Scope void SOMLINK somtEmitMethodsProlog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitMethodsProlog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("methodsPrologSN"));
}

SOM_Scope void SOMLINK somtEmitBase(SOMTEmitC SOMSTAR somSelf,
	                                         /* in */ SOMTBaseClassEntryC SOMSTAR base)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitBase");

  base->somtSetSymbolsOnEntry(somSelf, "base");
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("baseSN"));
}

SOM_Scope void SOMLINK somtEmitRelease(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitRelease");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("releaseSN"));
}

SOM_Scope void SOMLINK somtEmitMetaInclude(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitMetaInclude");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("metaIncludeSN"));
}

SOM_Scope void SOMLINK somtEmitStructEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitStructEpilog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("structEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitMethods(SOMTEmitC SOMSTAR somSelf,
	                                            /* in */ SOMTMethodEntryC SOMSTAR method)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitMethods");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("methodsSN"));
}

SOM_Scope void SOMLINK somtEmitUnionEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitEpilog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("unionEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitInterfaceEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitInterfaceEpilog");
  
  return;
}

SOM_Scope void SOMLINK somtEmitMethodsEpilog(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitMethodsEpilog");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("methodEpilogSN"));
}

SOM_Scope void SOMLINK somtEmitTypedef(SOMTEmitC SOMSTAR somSelf,
	                                            /* in */ SOMTTypedefEntryC SOMSTAR td)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitTypedef");
  
  _somtTemplate->somtOutputSection(_somtTemplate->somtGetSymbol("typedefSN"));
}

SOM_Scope void SOMLINK somDefaultInit(SOMTEmitC SOMSTAR somSelf, somInitCtrl *ctrl)
{
  SOMTEmitCData *somThis; // set by BeginInitializer 
  somInitCtrl globalCtrl;
  somBooleanVector myMask;
  SOMTEmitCMethodDebug("SOMTEmitC", "somDefaultInit");
  
  SOMTEmitC_BeginInitializer_somDefaultInit;
  SOMTEmitC_Init_SOMObject_somDefaultInit(somSelf, ctrl);
  
  _somtTemplate=new SOMTTemplateOutputC();
  somSelf->somtSetPredefinedSymbols();
}

SOM_Scope void SOMLINK somtSetPredefinedSymbols(SOMTEmitC SOMSTAR somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtSetPredefinedSymbols");

  _somtTemplate->somtSetSymbolCopyBoth("prologSN", "prologS"); //+
  _somtTemplate->somtSetSymbolCopyBoth("baseIncludesPrologSN", "baseIncludesPrologS"); //+
  _somtTemplate->somtSetSymbolCopyBoth("baseIncludesSN", "baseIncludesS");//+
  _somtTemplate->somtSetSymbolCopyBoth("baseIncludesEpilogSN", "baseIncludesEpilogS"); //+
  _somtTemplate->somtSetSymbolCopyBoth("metaIncludeSN", "metaIncludeS"); //+
  _somtTemplate->somtSetSymbolCopyBoth("classSN", "classS"); //+
  _somtTemplate->somtSetSymbolCopyBoth("metaSN", "metaS"); //+
  _somtTemplate->somtSetSymbolCopyBoth("basePrologSN", "basePrologS"); //+
  _somtTemplate->somtSetSymbolCopyBoth("baseSN", "baseS");//+
  _somtTemplate->somtSetSymbolCopyBoth("baseEpilogSN", "baseEpilogS"); //+
  _somtTemplate->somtSetSymbolCopyBoth("constantPrologSN", "constantPrologS"); //+
  _somtTemplate->somtSetSymbolCopyBoth("constantSN", "constantS");//+
  _somtTemplate->somtSetSymbolCopyBoth("constantEpilogSN", "constantEpilogS"); //+
  _somtTemplate->somtSetSymbolCopyBoth("typedefPrologSN", "typedefPrologS");//+
  _somtTemplate->somtSetSymbolCopyBoth("typedefSN", "typedefS");//+
  _somtTemplate->somtSetSymbolCopyBoth("typedefEpilogSN", "typedefEpilogS");//+
  _somtTemplate->somtSetSymbolCopyBoth("structPrologSN", "structPrologS");//+
  _somtTemplate->somtSetSymbolCopyBoth("structSN", "structS");//+
  _somtTemplate->somtSetSymbolCopyBoth("structEpilogSN", "structEpilogS");//+
  _somtTemplate->somtSetSymbolCopyBoth("unionPrologSN", "unionPrologS");//+
  _somtTemplate->somtSetSymbolCopyBoth("unionSN", "unionS");//+
  _somtTemplate->somtSetSymbolCopyBoth("unionEpilogSN", "unionEpilogS");//+
  _somtTemplate->somtSetSymbolCopyBoth("enumPrologSN", "enumPrologS");//+
  _somtTemplate->somtSetSymbolCopyBoth("enumSN", "enumS");//+
  _somtTemplate->somtSetSymbolCopyBoth("enumEpilogSN", "enumEpilogS");//+
  _somtTemplate->somtSetSymbolCopyBoth("attributePrologSN", "attributePrologS");//++
  _somtTemplate->somtSetSymbolCopyBoth("attributeSN", "attributeS");//+
  _somtTemplate->somtSetSymbolCopyBoth("attributeEpilogSN", "attributeEpilogS");//++
  _somtTemplate->somtSetSymbolCopyBoth("interfacePrologSN", "interfacePrologS");
  _somtTemplate->somtSetSymbolCopyBoth("interfaceSN", "interfaceS");
  _somtTemplate->somtSetSymbolCopyBoth("interfaceEpilogSN", "interfaceEpilogS");
  _somtTemplate->somtSetSymbolCopyBoth("modulePrologSN", "modulePrologS");
  _somtTemplate->somtSetSymbolCopyBoth("moduleSN", "moduleS");
  _somtTemplate->somtSetSymbolCopyBoth("moduleEpilogSN", "moduleEpilogS");
  _somtTemplate->somtSetSymbolCopyBoth("passthruPrologSN", "passthruPrologS");//+
  _somtTemplate->somtSetSymbolCopyBoth("passthruSN", "passthruS");//+
  _somtTemplate->somtSetSymbolCopyBoth("passthruEpilogSN", "passthruEpilogS");//+
  _somtTemplate->somtSetSymbolCopyBoth("releaseSN", "releaseS");//+
  _somtTemplate->somtSetSymbolCopyBoth("dataPrologSN", "dataPrologS");//+
  _somtTemplate->somtSetSymbolCopyBoth("dataSN", "dataS");//+
  _somtTemplate->somtSetSymbolCopyBoth("dataEpilogSN", "dataEpilogS");//+
  _somtTemplate->somtSetSymbolCopyBoth("methodsPrologSN", "methodsPrologS");//+
  _somtTemplate->somtSetSymbolCopyBoth("methodsSN", "methodsS");//+
  _somtTemplate->somtSetSymbolCopyBoth("overrideMethodsSN", "overrideMethodsS");
  _somtTemplate->somtSetSymbolCopyBoth("overriddenMethodsSN", "overriddenMethodsS");
  _somtTemplate->somtSetSymbolCopyBoth("inheritedMethodsSN", "inheritedMethodsS");
  _somtTemplate->somtSetSymbolCopyBoth("methodsEpilogSN", "methodsEpilogS");//+
  _somtTemplate->somtSetSymbolCopyBoth("epilogSN", "epilogS"); //+
}

typedef void (SOMLINK * somtEmitProc)(SOMTEmitC SOMSTAR somSelf);
typedef void (SOMLINK * somtEmitProcEach)(SOMTEmitC SOMSTAR somSelf, SOMTEntryC SOMSTAR base);
                                                   
SOM_Scope boolean SOMLINK somtScanStructs(SOMTEmitC SOMSTAR somSelf,
                                                 /* in */ string prolog,
                                                 /* in */ string each,
                                                 /* in */ string epilog)
{
  somtEmitProc Proc;
  somtEmitProcEach ProcEach;
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanStructs");

  Proc = (somtEmitProc) somResolveByName(somSelf, prolog);
  Proc(somSelf);

  ProcEach = (somtEmitProcEach) somResolveByName(somSelf, each);
  ProcEach(somSelf, NULL);

  Proc = (somtEmitProc) somResolveByName(somSelf, epilog);
  Proc(somSelf);

  return FALSE;
}

SOM_Scope boolean SOMLINK somtScanEnums(SOMTEmitC SOMSTAR somSelf,
                                               /* in */ string prolog,
                                               /* in */ string each,
                                               /* in */ string epilog)
{
  somtEmitProc Proc;
  somtEmitProcEach ProcEach;
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanEnums");

  Proc = (somtEmitProc) somResolveByName(somSelf, prolog);
  Proc(somSelf);

  ProcEach = (somtEmitProcEach) somResolveByName(somSelf, each);
  ProcEach(somSelf, NULL);

  Proc = (somtEmitProc) somResolveByName(somSelf, epilog);
  Proc(somSelf);

  return FALSE;
}

SOM_Scope boolean SOMLINK somtScanData(SOMTEmitC SOMSTAR somSelf,
                                              /* in */ string prolog,
                                              /* in */ string each,
                                              /* in */ string epilog)
{
  somtEmitProc Proc;
  somtEmitProcEach ProcEach;
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanData");

  Proc = (somtEmitProc) somResolveByName(somSelf, prolog);
  Proc(somSelf);

  ProcEach = (somtEmitProcEach) somResolveByName(somSelf, each);
  ProcEach(somSelf, NULL);

  Proc = (somtEmitProc) somResolveByName(somSelf, epilog);
  Proc(somSelf);

  return FALSE;
}

SOM_Scope boolean SOMLINK somtScanModules(SOMTEmitC SOMSTAR somSelf,
                                                 /* in */ string prolog,
                                                 /* in */ string each,
                                                 /* in */ string epilog)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanModules");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtScanPassthru(SOMTEmitC SOMSTAR somSelf,
                                                  /* in */ boolean before,
                                                  /* in */ string prolog,
                                                  /* in */ string each,
                                                  /* in */ string epilog)
{
  somtEmitProc Proc;
  somtEmitProcEach ProcEach;
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanPassthru");

  Proc = (somtEmitProc) somResolveByName(somSelf, prolog);
  Proc(somSelf);

  ProcEach = (somtEmitProcEach) somResolveByName(somSelf, each);
  ProcEach(somSelf, NULL);

  Proc = (somtEmitProc) somResolveByName(somSelf, epilog);
  Proc(somSelf);

  return FALSE;
}
                                                  
SOM_Scope boolean SOMLINK somtScanBasesF(SOMTEmitC SOMSTAR somSelf,
                                                /* in */ string filter,
                                                /* in */ string prolog,
                                                /* in */ string each,
                                                /* in */ string epilog,
                                                /* in */ boolean forceProlog)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanBasesF");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtScanTypedefs(SOMTEmitC SOMSTAR somSelf,
                                                  /* in */ string prolog,
                                                  /* in */ string each,
                                                  /* in */ string epilog)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanTypedefs");
  
  return FALSE;
}


SOM_Scope boolean SOMLINK somtScanBases(SOMTEmitC SOMSTAR somSelf,
                                               /* in */ string prolog,
                                               /* in */ string each,
                                               /* in */ string epilog)
{
  somtEmitProc Proc;
  somtEmitProcEach ProcEach;
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanBases");
  SOMTClassEntryC *cls = _somtTargetClass;
  SOMTBaseClassEntryC *bases;

  Proc = (somtEmitProc) somResolveByName(somSelf, prolog);
  Proc(somSelf);

  boolean done;
  for (done = ((bases=cls->somtGetFirstBaseClass())!=NULL); done;
       done = ((bases=cls->somtGetNextBaseClass())!=NULL))
  {       
    ProcEach = (somtEmitProcEach) somResolveByName(somSelf, each);
    ProcEach(somSelf, (SOMTEntryC *)bases);
  }


  Proc = (somtEmitProc) somResolveByName(somSelf, epilog);
  Proc(somSelf);

  return FALSE;
}

SOM_Scope boolean SOMLINK somtScanBaseIncludes(SOMTEmitC SOMSTAR somSelf,
                                               /* in */ string prolog,
                                               /* in */ string each,
                                               /* in */ string epilog)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanBaseIncludes");
  
  return somSelf->somtScanBases(prolog, each, epilog);
}

SOM_Scope boolean SOMLINK somtScanInterfaces(SOMTEmitC SOMSTAR somSelf,
                                                    /* in */ string prolog,
                                                    /* in */ string each,
                                                    /* in */ string epilog)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanInterfaces");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtScanConstants(SOMTEmitC SOMSTAR somSelf,
                                                   /* in */ string prolog,
                                                   /* in */ string each,
                                                   /* in */ string epilog)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanBases");

  somtEmitProc Proc;
  somtEmitProcEach ProcEach;
  SOMTClassEntryC *cls = _somtTargetClass;
  SOMTConstEntryC *cons;

  Proc = (somtEmitProc) somResolveByName(somSelf, prolog);
  Proc(somSelf);

  boolean done;

  for (done = ((cons=cls->somtGetFirstConstant())!=NULL); done;
       done = ((cons=cls->somtGetNextConstant())!=NULL))
  {       
    ProcEach = (somtEmitProcEach) somResolveByName(somSelf, each);
    ProcEach(somSelf, (SOMTEntryC *)cons);
  }

  Proc = (somtEmitProc) somResolveByName(somSelf, epilog);
  Proc(somSelf);

  return FALSE;
}

SOM_Scope boolean SOMLINK somtScanMethods(SOMTEmitC SOMSTAR somSelf,
                                                 /* in */ string filter,
                                                 /* in */ string prolog,
                                                 /* in */ string each,
                                                 /* in */ string epilog,
                                                 /* in */ boolean forceProlog)
{
  somtEmitProc Proc;
  somtEmitProcEach ProcEach;
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanMethods");

  Proc = (somtEmitProc) somResolveByName(somSelf, prolog);
  Proc(somSelf);

  ProcEach = (somtEmitProcEach) somResolveByName(somSelf, each);
  ProcEach(somSelf, NULL);

  Proc = (somtEmitProc) somResolveByName(somSelf, epilog);
  Proc(somSelf);

  return FALSE;
}
                                               
SOM_Scope boolean SOMLINK somtScanAttributes(SOMTEmitC SOMSTAR somSelf,
                                                    /* in */ string prolog,
                                                    /* in */ string each,
                                                    /* in */ string epilog)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanAttributes");
  somtEmitProc Proc;
  somtEmitProcEach ProcEach;

  SOMTClassEntryC *cls = _somtTargetClass;
  SOMTAttributeEntryC *atts;


  Proc = (somtEmitProc) somResolveByName(somSelf, prolog);
  Proc(somSelf);

  boolean done;
  for (done = ((atts=cls->somtGetFirstAttribute())!=NULL); done;
       done = ((atts=cls->somtGetNextAttribute())!=NULL))
  {       
    ProcEach = (somtEmitProcEach) somResolveByName(somSelf, each);
    ProcEach(somSelf, (SOMTEntryC *)atts);
  }

  Proc = (somtEmitProc) somResolveByName(somSelf, epilog);
  Proc(somSelf);

  return FALSE;
}

SOM_Scope boolean SOMLINK somtScanUnions(SOMTEmitC SOMSTAR somSelf,
                                                /* in */ string prolog,
                                                /* in */ string each,
                                                /* in */ string epilog)
{
  somtEmitProc Proc;
  somtEmitProcEach ProcEach;
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf);
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanUnions");

  Proc = (somtEmitProc) somResolveByName(somSelf, prolog);
  Proc(somSelf);

  ProcEach = (somtEmitProcEach) somResolveByName(somSelf, each);
  ProcEach(somSelf, NULL);

  Proc = (somtEmitProc) somResolveByName(somSelf, epilog);
  Proc(somSelf);

  return FALSE;
}

SOM_Scope boolean SOMLINK somtScanDataF(SOMTEmitC SOMSTAR somSelf,
                                               /* in */ string filter,
                                               /* in */ string prolog,
                                               /* in */ string each,
                                               /* in */ string epilog,
                                               /* in */ boolean forceProlog)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtScanDataF");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtInherited(SOMTEmitC * somSelf,
                                        /* in */ SOMTMethodEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtInherited");
  
  return FALSE;
}
                                               
SOM_Scope boolean SOMLINK somtOverridden(SOMTEmitC * somSelf,
                                         /* in */ SOMTMethodEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtOverridden");
  
  return FALSE;                                         
}

SOM_Scope void SOMLINK somtEmitFullPassthru(SOMTEmitC * somSelf,
                                            /* in */ boolean before,
                                            /* in */ string language)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtEmitFullPassthru");
  
  
}

SOM_Scope SOMTEntryC * SOMLINK somtGetNextGlobalDefinition(SOMTEmitC * somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtGetNextGlobalDefinition");
  
  return NULL;
}

SOM_Scope boolean SOMLINK somtNewNoProc(SOMTEmitC * somSelf,
                                        /* in */ SOMTEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtNewNoProc");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtPrivOrPub(SOMTEmitC * somSelf,
                                        /* in */ SOMTEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtPrivOrPub");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtCheckVisibility(SOMTEmitC * somSelf,
                                              /* in */ SOMTMethodEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtCheckVisibility");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtImplemented(SOMTEmitC * somSelf,
                                          /* in */ SOMTMethodEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtImplemented");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtVA(SOMTEmitC * somSelf,
                                 /* in */ SOMTEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtVA");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtAll(SOMTEmitC * somSelf,
                                  /* in */ SOMTMethodEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtAll");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtLink(SOMTEmitC * somSelf,
                                   /* in */ SOMTEntryC * entry)
{

  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtLink");
  
  return FALSE;
}

SOM_Scope void SOMLINK somDumpSelfInt(SOMTEmitC * somSelf,
                                      /* in */ long level)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somDumpSelfInt");
  
}

SOM_Scope boolean SOMLINK somtAllVisible(SOMTEmitC * somSelf,
                                         /* in */ SOMTMethodEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtAllVisible");
  
  return FALSE;
}

SOM_Scope boolean SOMLINK somtNew(SOMTEmitC * somSelf,
                                  /* in */ SOMTMethodEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtNew");
  
  return FALSE;
}

SOM_Scope SOMTEntryC * SOMLINK somtGetFirstGlobalDefinition(SOMTEmitC * somSelf)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtGetFirstGlobalDefinition");
  
  return NULL;
}

SOM_Scope boolean SOMLINK somtNewProc(SOMTEmitC * somSelf,
                                      /* in */ SOMTEntryC * entry)
{
  SOMTEmitCData *somThis = SOMTEmitCGetData(somSelf); 
  SOMTEmitCMethodDebug("SOMTEmitC","somtNewProc");
  
  return FALSE;
}

