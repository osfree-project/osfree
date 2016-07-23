#include "sctmplt.h"
#include "scemit.h"
#include <scentry.h>
#include <emitlib.h>
#include <sctypes.h>
#include "def.h"

#define SYMBOLS_FILE   "test.efw"

FILE *emit(char *file, Entry * cls, Stab * stab);

int main(int argc, char *argv[])
{
  FILE *f;
  Entry cls;

  /*
  SOMTTemplateOutputC *demo2 = SOMTTemplateOutputCNew();
  SOMTEmitC *emitter = SOMTEmitCNew();
  
  // Добавляем секции из шаблона
  f = _somtOpenSymbolsFile(emitter, "test.efw", "rb");
  _somtReadSectionDefinitions(demo2, f);
  somtfclose(f);
  

  _somtSetSymbol(demo2, "lnkDLLName", "emitdef");
  _somtSetSymbol(demo2, "className", "DEFEmitter");
  _somtSetSymbol(demo2, "classCScopedName", "DEFEmitter");
  
  somPrintf("%s", _somtGetSymbol(demo2, "prologS"));
  //_somto(demo2, tpl);
  _somtOutputComment(demo2, "This is test of comment\noutput");
  __set_somtCommentStyle(demo2, somtCBlockE);
  _somtOutputComment(demo2, "This is test of comment\noutput");
  _somtOutputSection(demo2, "prologS");
  _somFree(demo2);
  */
  
  // Эмулируем вызов эмиттера
  
  cls.name="test";
  cls.type=SOMTClassE;
  f=emit("test.out", &cls, NULL);
  //somtfclose(f);
  
  return 0;
}

