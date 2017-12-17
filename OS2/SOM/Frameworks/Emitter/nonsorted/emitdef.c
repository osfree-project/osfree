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

#include <scentry.h>
#include <emitlib.h>
#include "def.h"

#define SYMBOLS_FILE   "def.efw"

SOMEXTERN FILE *emit(char *file, Entry * cls, Stab * stab)
{

    FILE * fp;
    FILE * deffile;
    SOMTClassEntryC * oCls;
    SOMTModuleEntryC * mod;
    DEFEmitter *emitter;
    SOMTTemplateOutputC *t;

    if (cls->type == SOMTClassE) {
        file=strcat(file, ""); 
        fp = somtopenEmitFile(file, "def");
        oCls = (SOMTClassEntryC *) somtGetObjectWrapper(cls);
        emitter = DEFEmitterNew();
        __set_somtTargetFile(emitter, fp);
        __set_somtTargetClass(emitter, oCls);
        __set_somtEmitterName(emitter, "def");
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
        _somFree(oCls);

        return (fp);
    }
    else if (cls->type == SOMTModuleE) {
        fp = somtopenEmitFile(file, "def");
        mod = (SOMTModuleEntryC *) somtGetObjectWrapper(cls);
        emitter = DEFEmitterNew();
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
    else {
      return ((FILE *) NULL);
    }
}
