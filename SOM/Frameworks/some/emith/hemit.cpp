/**************************************************************************
 *
 *  Copyright 2014, 2017 Yuri Prokushev
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
 
#ifndef SOM_Module_hemit_Source
#define SOM_Module_hemit_Source
#endif
#define HEmitter_Class_Source

 
#include "hemit.xih"
#include <scmodule.xh>
#include <scclass.xh>
#include <emitlib.h>
#include <string.h>
#include <stdio.h>


boolean flag=FALSE;

SOM_Scope boolean  SOMLINK hemit_somtGenerateSections(HEmitter *somSelf)
{
  char *DllName;
  char *obj;
  char buf[1024];
  SOMTTemplateOutputC   *templ;
  SOMTClassEntryC       *cls;

  templ= somSelf->_get_somtTemplate();
  cls= somSelf->_get_somtTargetClass();
  HEmitterMethodDebug("HEmitter","somtGenerateSections");

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


#define SYMBOLS_FILE   "hemit.efw"

FILE *emit(char *file, Entry * cls, Stab * stab)
{

    FILE * fp;
    FILE * deffile;
    SOMTClassEntryC * oCls;
    SOMTModuleEntryC * mod;
    SOMTEmitC *emitter;
    SOMTTemplateOutputC *t;

    int SOM_TraceLevel = 0 ; /* Request maximum debugging information */
    int SOM_WarnLevel = 0 ;
    int SOM_AssertLevel = 0 ;

 
    if (cls->type == SOMTClassE) {
        fp = stdout;//somtopenEmitFile(file, "out");
        oCls = (SOMTClassEntryC *) somtGetObjectWrapper(cls);
        //oCls->somDumpSelf(0);
        emitter = new SOMTEmitC();
          
        emitter->_set_somtTargetFile(fp);
        emitter->_set_somtTargetClass(oCls);
        emitter->_set_somtEmitterName("h");
        t = emitter->_get_somtTemplate();
        t->_set_somtCommentStyle(somtCPPE);

        if (deffile = emitter->somtOpenSymbolsFile(SYMBOLS_FILE, "r")) {
            t->somtReadSectionDefinitions(deffile);
            somtfclose(deffile);
        }
        else {
            exit(-1);
        }
        emitter->somtGenerateSections();

        delete emitter;
        delete oCls;

        return (fp);
    }
    #if 0
    else if (cls->type == SOMTModuleE) {
        fp = somtopenEmitFile(file, "def");
        mod = (SOMTModuleEntryC *) somtGetObjectWrapper(cls);
        emitter = SOMTEmitCNew();
        __set_somtTargetFile(emitter, fp);
        __set_somtTargetModule(emitter, mod);
        t = __get_somtTemplate(emitter);
        __set_somtCommentStyle(t, somtCPPE);
        if (deffile = _somtOpenSymbolsFile(emitter, SYMBOLS_FILE, "r")) {
            _somtReadSectionDefinitions(t, deffile);
            somtfclose(deffile);
        }
        else {
            exit(-1);
        }
        _somtGenerateSections(emitter);
        _somFree(emitter);
        _somFree(mod);

        return (fp);
    }
    #endif
    else {
      return ((FILE *) NULL);
    }
}
