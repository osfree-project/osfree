#include <scentry.xh>
#include <emitlib.h>
#include "genemit.xh"

SOMEXTERN FILE * SOMLINK emit(char *file, Entry * cls, Stab * stab)
{

    FILE *fp;
    FILE *genfile;
    SOMTClassEntryC *oCls;
    SOMTModuleEntryC *mod;
    GENEmitter *emitter;
    SOMTTemplateOutputC *t;

    /* if this is a class, rather than a module: */
    if (cls->type == SOMTClassE) {
        file=strcat(file, ""); //  Модификатор файла. Если такую штуку не делать, то может быть трап.
        fp = somtopenEmitFileSL(file, "gen");
        oCls = (SOMTClassEntryC *)somtGetObjectWrapper(cls);
        emitter = new GENEmitter();
        emitter->_set_somtTargetFile(fp);
        emitter->_set_somtTargetClass(oCls);
        emitter->_set_somtEmitterName("gen");
        t = emitter->_get_somtTemplate();
        t->_set_somtCommentStyle(somtCPPE);
printf("a=%s\n", emitter->somtGetGlobalModifierValue("deffile"));
	
        if (genfile = emitter->somtOpenSymbolsFile(emitter->somtGetGlobalModifierValue("deffile"), "r")) {
            t->somtReadSectionDefinitions(genfile);
            somtfcloseSL(genfile);
        }
        else {
//            debug("Не могу открыть файл символов \" %s \".\n",
//                             SYMBOLS_FILE);
            exit(-1);
        }
        emitter->somtGenerateSections();
        delete emitter;
        delete oCls;

        return (fp);
    }
    else if (cls->type == SOMTModuleE) {

        fp = somtopenEmitFileSL(file, "gen");
        mod = (SOMTModuleEntryC *) somtGetObjectWrapper(cls);
        emitter = new GENEmitter();
        emitter->_set_somtTargetFile(fp);
        emitter->_set_somtTargetModule(mod);
        t = emitter->_get_somtTemplate();
        t->_set_somtCommentStyle(somtCPPE);
	if (genfile = emitter->somtOpenSymbolsFile(emitter->somtGetGlobalModifierValue("deffile"), "r")) {
            t->somtReadSectionDefinitions(genfile);
            somtfcloseSL(genfile);
        }
        else {
//            debug("Cannot open Symbols file \" %s \".\n",
//                             SYMBOLS_FILE);
            exit(-1);
        }
        emitter->somtGenerateSections();
        delete emitter;
        delete mod;

        return (fp);
    }
    else {
//      debug("Неизвестно, что делать. Выходим.\n");
      return ((FILE *) NULL);
    }
}
