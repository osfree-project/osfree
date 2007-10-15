/* @(#) somc/gen_emit.efs 2.7 7/14/95 12:05:40 [7/30/96 14:44:56] */

/*
 *   COMPONENT_NAME: some
 *
 *   ORIGINS: 27
 *
 *
 *   10H9767, 10H9769  (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *         File:    emitpas.c
 *       Author:    SOMObjects Emitter Framework
 *     Contents:    Generic framework emitter for Pascal.
 *         Date:    Tue Sep 21 20:16:44 2004.
 */

#pragma aux somtopenEmitFile "somtopenEmitFile"
#pragma aux somtfclose "somtfclose"
#pragma aux emit "emit"

#include <scentry.h>
#include <emitlib.h>
#include "pas.h"

//#define DEBUG

#ifdef DEBUG
  #define debug(s,...) printf(__FILE__"/"__func__"(): "##s"\n" ,##__VA_ARGS__)
#else
  #define debug(x, ...)
#endif



#define SYMBOLS_FILE   "lnk.efw"

SOMEXTERN FILE *emit(char *file, Entry * cls, Stab * stab)
{

    FILE *fp;
    FILE *deffile;
    SOMTClassEntryC *oCls;
    SOMTModuleEntryC *mod;
    PascalEmitter *emitter;
    SOMTTemplateOutputC *t;

    debug("Вход в эмитер\n");

    /* if this is a class, rather than a module: */
    if (cls->type == SOMTClassE) {
        debug("Обработка класса\n");

        file=strcat(file, ""); //  Модификатор файла. Если такую штуку не делать, то может быть трап.
        fp = somtopenEmitFile(file, "lnk");
        debug("Создаем какой-то объект враппер\n");
        oCls = (SOMTClassEntryC *)somtGetObjectWrapper(cls);
        debug("Создаем экземпляр класса эмиттера\n");
        emitter = PascalEmitterNew();
        debug("Указываем целевой файл\n");
        __set_somtTargetFile(emitter, fp);
        debug("Указываем целевой класс\n");
        __set_somtTargetClass(emitter, oCls);
        debug("Указываем имя эмиттера\n");
        __set_somtEmitterName(emitter, "pas");
        debug("Получаем шаблон\n");
        t = __get_somtTemplate(emitter);
        debug("Устанавливаем тип коментариев\n");
        __set_somtCommentStyle(t, somtCPPE);
        debug("Открываем символьный файл\n");
        if (deffile = _somtOpenSymbolsFile(emitter, SYMBOLS_FILE, "r")) {
            debug("Читаем определения секций\n");
            _somtReadSectionDefinitions(t, deffile);
            debug("Закрываем файл\n");
            somtfclose(deffile);
        }
        else {
            debug("Не могу открыть файл символов \" %s \".\n",
                             SYMBOLS_FILE);
            exit(-1);
        }
        debug("Генерируем секции\n");
        _somtGenerateSections(emitter);
        debug("Освобождаем класс эмиттера\n");
        _somFree(emitter);
        debug("Освобождаем объект-враппер\n");
        _somFree(oCls);

        return (fp);
    }
    else if (cls->type == SOMTModuleE) {
        debug("Обработка модуля\n");

        fp = somtopenEmitFile(file, "pas");
        mod = (SOMTModuleEntryC *) somtGetObjectWrapper(cls);
        emitter = PascalEmitterNew();
        __set_somtTargetFile(emitter, fp);
        __set_somtTargetModule(emitter, mod);
        t = __get_somtTemplate(emitter);
        __set_somtCommentStyle(t, somtCPPE);
        if (deffile = _somtOpenSymbolsFile(emitter, SYMBOLS_FILE, "r")) {
            _somtReadSectionDefinitions(t, deffile);
            somtfclose(deffile);
        }
        else {
            debug("Cannot open Symbols file \" %s \".\n",
                             SYMBOLS_FILE);
            exit(-1);
        }
        _somtGenerateSections(emitter);
        _somFree(emitter);
        _somFree(mod);

        return (fp);
    }
    else {
      debug("Неизвестно, что делать. Выходим.\n");
      return ((FILE *) NULL);
    }
}
