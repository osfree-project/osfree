#include <scentry.xh>
#include <emitlib.h>
#include "modsemit.xh"

#define SYMBOLS_FILE   "modsemit.efw"

SOMEXTERN FILE * SOMLINK emitSL(char *file, Entry * cls, Stab * stab)
{

    FILE *fp;
    FILE *deffile;
    SOMTClassEntryC *oCls;
    SOMTModuleEntryC *mod;
    MODSEmitter *emitter;
    SOMTTemplateOutputC *t;

    /* if this is a class, rather than a module: */
    if (cls->type == SOMTClassE) {
        fp = somtopenEmitFileSL("__mods__", "ini");
        oCls = (SOMTClassEntryC *)somtGetObjectWrapper(cls);
        emitter = new MODSEmitter();
        emitter->_set_somtTargetFile(fp);
        emitter->_set_somtTargetClass(oCls);
        emitter->_set_somtEmitterName("mods");
        t = emitter->_get_somtTemplate();
        t->_set_somtCommentStyle(somtCPPE);
        if (deffile = emitter->somtOpenSymbolsFile(SYMBOLS_FILE, "r")) {
            t->somtReadSectionDefinitions(deffile);
            somtfcloseSL(deffile);
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

        fp = somtopenEmitFileSL("__mods__", "ini");
        mod = (SOMTModuleEntryC *) somtGetObjectWrapper(cls);
        emitter = new MODSEmitter();
        emitter->_set_somtTargetFile(fp);
        emitter->_set_somtTargetModule(mod);
        t = emitter->_get_somtTemplate();
        t->_set_somtCommentStyle(somtCPPE);
        if (deffile = emitter->somtOpenSymbolsFile(SYMBOLS_FILE, "r")) {
            t->somtReadSectionDefinitions(deffile);
            somtfcloseSL(deffile);
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
