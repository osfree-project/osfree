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
