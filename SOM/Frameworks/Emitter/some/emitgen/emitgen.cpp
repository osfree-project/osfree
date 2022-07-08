#include <scentry.xh>
#include <emitlib.h>
#include "genemit.xh"

#define SYMBOLS_FILE   "genemit.efw"

SOMEXTERN FILE * SOMLINK   emit(char *file, Entry * cls, Stab * stab)
{

    FILE *fp;
    FILE *genfile;
    SOMTClassEntryC *oCls;
    SOMTModuleEntryC *mod;
    GENEmitter *emitter;
    SOMTTemplateOutputC *t;

printf("1\n");
    /* if this is a class, rather than a module: */
    if (cls->type == SOMTClassE) {
printf("2\n");
        file=strcat(file, ""); //  Модификатор файла. Если такую штуку не делать, то может быть трап.
printf("3\n");
        fp = somtopenEmitFile(file, "gen");
printf("4\n");
        oCls = (SOMTClassEntryC *)somtGetObjectWrapper(cls);
printf("5\n");
        emitter = new GENEmitter();
printf("6\n");
        emitter->_set_somtTargetFile(fp);
printf("7\n");
        emitter->_set_somtTargetClass(oCls);
printf("8\n");
        emitter->_set_somtEmitterName("gen");
printf("9\n");
        t = emitter->_get_somtTemplate();
printf("10\n");
        t->_set_somtCommentStyle(somtCPPE);
printf("11\n");
        if (genfile = emitter->somtOpenSymbolsFile(SYMBOLS_FILE, "r")) {
printf("12\n");
            t->somtReadSectionDefinitions(genfile);
printf("13\n");
            somtfclose(genfile);
printf("14\n");
        }
        else {
printf("15\n");
//            debug("Не могу открыть файл символов \" %s \".\n",
//                             SYMBOLS_FILE);
            exit(-1);
printf("15\n");
        }
printf("16\n");
        emitter->somtGenerateSections();
printf("17\n");
        delete emitter;
        delete oCls;

        return (fp);
    }
    else if (cls->type == SOMTModuleE) {

        fp = somtopenEmitFile(file, "gen");
        mod = (SOMTModuleEntryC *) somtGetObjectWrapper(cls);
        emitter = new GENEmitter();
        emitter->_set_somtTargetFile(fp);
        emitter->_set_somtTargetModule(mod);
        t = emitter->_get_somtTemplate();
        t->_set_somtCommentStyle(somtCPPE);
        if (genfile = emitter->somtOpenSymbolsFile(SYMBOLS_FILE, "r")) {
            t->somtReadSectionDefinitions(genfile);
            somtfclose(genfile);
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
