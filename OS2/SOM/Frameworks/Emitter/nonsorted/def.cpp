#ifndef SOM_Module_def_Source
#define SOM_Module_def_Source
#endif
#define DEFEmitter_Class_Source

#include "def.xih"
//#include <scparm.h>
//#include <sctdef.h>
///#include <scseqnce.h>
#include <scmodule.xh>
//#include <scstruct.h>
//#include <scconst.h>
//#include <scenum.h>
//#include <scenumnm.h>
//#include <scunion.h>
//#include <scattrib.h>
#include <scclass.xh>
#include <string.h>
#include <stdio.h>

boolean flag=FALSE;

SOM_Scope boolean  SOMLINK defemit_somtGenerateSections(DEFEmitter *somSelf)
{
  char *DllName;
  char *obj;
  char buf[1024];
  SOMTTemplateOutputC   *templ;
  SOMTClassEntryC       *cls;
  
  templ= somSelf->_get_somtTemplate();
  cls= somSelf->_get_somtTargetClass();
  DEFEmitterMethodDebug("DEFEmitter","somtGenerateSections");

  if ( cls )
 {
   DllName=somSelf->somtGetGlobalModifierValue("dllname");
   if (!DllName) DllName= cls->somtGetModifierValue("dllname");
   if (!DllName) DllName= cls->somtGetModifierValue("filestem");
   strcpy(buf, DllName);
   templ->somtSetSymbolCopyBoth("lnkDLLName", buf );
 };

 if ( cls )
 {
   obj = somSelf->somtGetGlobalModifierValue("file");
   templ->somtSetSymbolCopyBoth("lnkObjects", obj);

   obj = somSelf->somtGetGlobalModifierValue("include");

   if (obj)
   {
    FILE *fp;
    char *content;

     fp = fopen(obj, "r" );
     if( fp != NULL )
     {
       long length = 0;
       fseek(fp, 0, SEEK_END);
       length = ftell(fp);
       fseek(fp, 0, SEEK_SET);

       content=(char *)malloc(length);
       fread(content, length, 1, fp);
       fclose(fp);
       templ->somtSetSymbolCopyBoth("lnkInclude", content);
       free(content);
     }
   }

   obj = somSelf->somtGetGlobalModifierValue("lib");
   templ->somtSetSymbolCopyBoth("lnkLibs", obj);
 };

  somSelf->somtFileSymbols();

  if (!flag)
  {
    somSelf->somtEmitProlog();
    flag=TRUE;
  }


  if ( cls )
  {
    somSelf->somtEmitClass();
  }

  return TRUE;
}


