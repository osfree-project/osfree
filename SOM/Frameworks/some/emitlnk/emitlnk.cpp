#include <scentry.xh>
#include <emitlib.h>
#include "lnkemit.xh"


#define SYMBOLS_FILE   "lnkemit.efw"

SOMEXTERN FILE * SOMLINK emit(char *file, Entry * cls, Stab * stab)
{

    FILE *fp;
    FILE *deffile;
    SOMTClassEntryC *oCls;
    SOMTModuleEntryC *mod;
    LnkEmitter *emitter;
    SOMTTemplateOutputC *t;

    /* if this is a class, rather than a module: */
    if (cls->type == SOMTClassE) {

        file=strcat(file, ""); //  Модификатор файла. Если такую штуку не делать, то может быть трап.
        fp = somtopenEmitFileSL(file, "lnk");
        oCls = (SOMTClassEntryC *)somtGetObjectWrapper(cls);
        emitter = new LnkEmitter();
        emitter->_set_somtTargetFile(fp);
        emitter->_set_somtTargetClass(oCls);
        emitter->_set_somtEmitterName("lnk");
        t = emitter->_get_somtTemplate();
        t->_set_somtCommentStyle(somtCPPE);
        if (deffile = emitter->somtOpenSymbolsFile(SYMBOLS_FILE, "r")) {
            t->somtReadSectionDefinitions(deffile);
            somtfcloseSL(deffile);
        }
        else {
            //debug("Не могу открыть файл символов \" %s \".\n",
                             //SYMBOLS_FILE);
            exit(-1);
        }
        emitter->somtGenerateSections();
        delete emitter;
        delete oCls;

        return (fp);
    }
    else if (cls->type == SOMTModuleE) {
        fp = somtopenEmitFileSL(file, "lnk");
        mod = (SOMTModuleEntryC *) somtGetObjectWrapper(cls);
        emitter = new LnkEmitter();
        emitter->_set_somtTargetFile(fp);
        emitter->_set_somtTargetModule(mod);
        t = emitter->_get_somtTemplate();
        t->_set_somtCommentStyle(somtCPPE);
        if (deffile = emitter->somtOpenSymbolsFile(SYMBOLS_FILE, "r")) {
            t->somtReadSectionDefinitions(deffile);
            somtfcloseSL(deffile);
        }
        else {
            //debug("Cannot open Symbols file \" %s \".\n",
                             //SYMBOLS_FILE);
            exit(-1);
        }
        emitter->somtGenerateSections();
        delete emitter;
        delete mod;

        return (fp);
    }
    else {
      return ((FILE *) NULL);
    }
}
