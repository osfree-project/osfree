/* @(#) some/gen_emit.efc 2.2 5/11/93 16:26:58 [6/4/93 21:50:01] */

/*
 * 96F8647, 96F8648 (C) Copyright IBM Corp. 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 */

/*
 *         File:    emitrep.c
 *       Author:    SOMObjects Emitter Framework
 *     Contents:    Generic framework emitter for ReportEmitter.
 *         Date:    Mon Jan  3 18:24:51 1994.
 */

#include <scentry.h>
#include <emitlib.h>
#include <rep.h>

#define SYMBOLS_FILE   "rep.efw"

SOMEXTERN FILE *emit(char *file, Entry * cls, Stab * stab)
{
    FILE *fp = somtopenEmitFile(file, "rep");
    FILE *deffile;
    SOMTClassEntryC oCls;
    SOMTModuleEntryC mod;
    ReportEmitter emitter;
    SOMTTemplateOutputC t;

    /* if this is a class, rather than a module: */
    if (cls->type == SOMTClassE) {

        oCls = (SOMTClassEntryC) somtGetObjectWrapper(cls);
        emitter = ReportEmitterNew();
        __set_somtTargetFile(emitter, fp);
        __set_somtTargetClass(emitter, oCls);
	__set_somtEmitterName(emitter, "rep");
        t = __get_somtTemplate(emitter);
        __set_somtCommentStyle(t, somtCPPE);
        if (deffile = _somtOpenSymbolsFile(emitter, SYMBOLS_FILE, "r")) {
            _somtReadSectionDefinitions(t, deffile);
            fclose(deffile);
        }
        else {
            fprintf(stderr, "Cannot open Symbols file \" %s \".\n", 
			     SYMBOLS_FILE);
            exit(-1);
        }
        _somtGenerateSections(emitter);
        _somFree(emitter);
	_somFree(oCls);

        return (fp);
    }
    else if (cls->type == SOMTModuleE) {
	mod = (SOMTModuleEntryC) somtGetObjectWrapper(cls);
        emitter = ReportEmitterNew();
        __set_somtTargetFile(emitter, fp);
        __set_somtTargetModule(emitter, mod);
        t = __get_somtTemplate(emitter);
        __set_somtCommentStyle(t, somtCPPE);
        if (deffile = _somtOpenSymbolsFile(emitter, SYMBOLS_FILE, "r")) {
            _somtReadSectionDefinitions(t, deffile);
            fclose(deffile);
        }
        else {
            fprintf(stderr, "Cannot open Symbols file \" %s \".\n", 
			     SYMBOLS_FILE);
            exit(-1);
        }
        _somtGenerateSections(emitter);
        _somFree(emitter);
        _somFree(mod);
        somtfclose(fp);
        return (fp);
    }

    else return ((FILE *) NULL);
}
