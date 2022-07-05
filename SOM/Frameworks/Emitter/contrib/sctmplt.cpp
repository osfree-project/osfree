/**************************************************************************
 *
 *  Copyright 2015, 2017, Yuri Prokushev
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

#ifndef SOM_Module_sctmplt_Source
#define SOM_Module_sctmplt_Source
#endif

#define SOMTTemplateOutputC_Class_Source

#include "sctmplt.xih"

#include <somstrt.xh>


SOM_Scope string SOMLINK somtExpandSymbol(SOMTTemplateOutputC SOMSTAR somSelf,
	                                        /* in */ string s,
	                                        /* in */ string buf)
{
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtExpandSymbol");
  
  return NULL;
}                                          

SOM_Scope void SOMLINK somtSetOutputFile(SOMTTemplateOutputC SOMSTAR somSelf,
	                                       /* inout */ FILE *fp)
{
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtSetOutputFile");
  
  _fp=fp;
}                                          

/****************************************************
 *
 * Добавляет определения секций из буфера defString
 * 
 * !TODO: Добавить обработку для ескейпинга двоеточия \:
 *
 */
SOM_Scope void SOMLINK somtAddSectionDefinitions(SOMTTemplateOutputC SOMSTAR somSelf,
	                                               /* in */ string defString)
{
  string line = NULL;
  string section = NULL;
  string content = NULL;
  string m = NULL;
  string buf = NULL;
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtAddSectionDefinitions");

  buf = strdup(defString);
  line = strtok(buf, "\n");
  while(line) {
    // wait for colon in first column
    if (strchr(line, ':')==line)
    {
      if (section) 
      {
        somSelf->somtSetSymbolCopyBoth(section, content);
        SOMFree(content);
        content=NULL;
      }
      section=line+1;
    } else {
      if (content) 
      {
        m=(string)SOMMalloc(strlen(content)+strlen(line)+2);
        sprintf(m, "%s%s\n", content, line);
        SOMFree(content);
        content=NULL;
      } else {
        m=(string)SOMMalloc(strlen(line)+2);
        sprintf(m, "%s\n", line);
      }
      
      content=m;
    }
    line  = strtok(NULL, "\n");
  }

  if (section) 
  {
    somSelf->somtSetSymbolCopyBoth(section, content);
    SOMFree(content);
    content=NULL;
  }


  free(buf);
}

SOM_Scope void SOMLINK somtSetSymbolCopyBoth(SOMTTemplateOutputC SOMSTAR somSelf,
	                                           /* in */ string name,
	                                           /* in */ string value)
{
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtSetSymbolCopyBoth");

  if (!name) return;
  if (!strlen(name)) return;
  _stab->somstAssociateCopyBoth(name, value);
}
                                             
SOM_Scope void SOMLINK somtOutputSection(SOMTTemplateOutputC SOMSTAR somSelf,
	                                       /* in */ string sectionName)
{
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtOutputSection");
  if (sectionName)
  {
    somSelf->somto(somSelf->somtGetSymbol(sectionName));
  }
}

SOM_Scope string SOMLINK somtGetSymbol(SOMTTemplateOutputC SOMSTAR somSelf,
	                                     /* in */ string name)
{
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtGetSymbol");

  return _stab->somstGetAssociation(name);
}

SOM_Scope boolean SOMLINK somtCheckSymbol(SOMTTemplateOutputC SOMSTAR somSelf,
	                                        /* in */ string name)
{
  string value=NULL;
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtCheckSymbol");
  
  value=somSelf->somtGetSymbol(name);
  if (!value) return FALSE;
  if (!strlen(value)) return FALSE;
  return TRUE;
}

SOM_Scope void SOMLINK somtSetSymbol(SOMTTemplateOutputC SOMSTAR somSelf,
	                                   /* in */ string name,
	                                   /* in */ string value)
{
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtSetSymbol");
  
  _stab->somstAssociate(name, value);
}

/*********************************************************
 *
 * Считывает определения секций из файла и добаляет их в массив определения секций
 *
 * !TODO: Нет никакой проверки, что все влезет в выделенную память. Никаких проверок
 * на размеры и кодировки...
 *
 * !TODO: Непонятно, как определиться, в каком режиме работает fread при открытии 
 * в режиме !b. Виндовый рантайм меняет \n\r на \n и все...
 *
 */

SOM_Scope void SOMLINK somtReadSectionDefinitions(SOMTTemplateOutputC SOMSTAR somSelf,
	                                                /* inout */ FILE *fp)
{
  size_t fsize=0;
  size_t afsize=0;
  string buffer = NULL;
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtReadSectionDefinitions");
  
  fseek(fp, 0, SEEK_END);
  fsize = ftell(fp);
  rewind(fp);

  buffer = (string)SOMMalloc(fsize + 1);
  memset (buffer, 0, fsize + 1);

  afsize=fread(buffer, fsize, 1, fp);
  *(buffer+fsize)  = 0;

  somSelf->somtAddSectionDefinitions(buffer);
  
  SOMFree(buffer);
}

SOM_Scope void SOMLINK somto(SOMTTemplateOutputC SOMSTAR somSelf,
	                           /* in */ string tmplt)
{
  string line;
  string srcbuf;
  char tagbuf[2048];
  unsigned long tagpos=0;
  char ch;
  
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somto");
  
  memset (tagbuf,0,2048);
  if (tmplt)
  {
    // Search keys for substitution.
    srcbuf = strdup(tmplt);

    line = strtok(srcbuf, "\n");
    while(line) 
    {
      while (ch=*(line++))
      {
        switch (ch)
        {
          case '\\':
            ch=*(line++);
            switch (ch)
            {
              case 'n':
                fputc('\n', _fp);
                break;
              case '<':
                fputc('<', _fp);
                break;
              case '\\':
                fputc('\\', _fp);
                break;
              default:
                fputc(ch, _fp);
                break;
            }
            break;
          case '<':
            while ((ch=*(line++))!='>')
            {
              tagbuf[tagpos]=ch;
              tagpos++;
            }
            
            fprintf(_fp, "%s", somSelf->somtGetSymbol(tagbuf));
            memset (tagbuf,0,2048);
            tagpos=0;
            break;
          default:
            fputc(ch, _fp);
            break;
        }
      }
      fputc('\n', _fp);
      line  = strtok(NULL, "\n");
    }
    free(srcbuf);
  }
}

SOM_Scope void SOMLINK somtSetSymbolCopyValue(SOMTTemplateOutputC SOMSTAR somSelf,
	                                            /* in */ string name,
	                                            /* in */ string value)
{
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtSetSymbolCopyValue");
  
  _stab->somstAssociateCopyValue(name, value);
}

SOM_Scope void SOMLINK somtOutputComment(SOMTTemplateOutputC SOMSTAR somSelf,
	                                       /* in */ string comment)
{
  string line;
  string buf;
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtOutputComment");
  
  if (_somtCommentNewline) fprintf(_fp, "\n");
  if (_somtCommentStyle==somtCBlockE) fprintf(_fp, "/*\n");
  if (_somtCommentStyle==somtPBlockE) fprintf(_fp, "(*\n");
  buf = strdup(comment);
  line = strtok(buf, "\n");
  while(line) 
  {
    switch (_somtCommentStyle)
    {
      case somtDashesE: fprintf(_fp, "-- %s\n", line);
        break;
      case somtCPPE: fprintf(_fp, "// %s\n", line);
        break;
      case somtCSimpleE: fprintf(_fp, "/* %s */\n", line);
        break;
      case somtCBlockE: fprintf(_fp, " *  %s\n", line);
        break;
      case somtPSimpleE: fprintf(_fp, "(* %s *)\n", line);
        break;
      case somtPBlockE: fprintf(_fp, " *  %s\n", line);
        break;
    }
    
    line  = strtok(NULL, "\n");
  }
  if (_somtCommentStyle==somtCBlockE) fprintf(_fp, " */\n");
  if (_somtCommentStyle==somtPBlockE) fprintf(_fp, " *)\n");
  free(buf);
}

SOM_Scope void SOMLINK somtSetSymbolCopyName(SOMTTemplateOutputC SOMSTAR somSelf,
	                                           /* in */ string name,
	                                           /* in */ string value)
{
  SOMTTemplateOutputCData *somThis = SOMTTemplateOutputCGetData(somSelf);
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somtSetSymbolCopyName");
  
  _stab->somstAssociateCopyKey(name, value);
}


SOM_Scope void SOMLINK somDefaultInit(SOMTTemplateOutputC SOMSTAR somSelf, somInitCtrl *ctrl)
{
  SOMTTemplateOutputCData *somThis; // set by BeginInitializer 
  somInitCtrl globalCtrl;
  somBooleanVector myMask;
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC", "somDefaultInit");
  
  SOMTTemplateOutputC_BeginInitializer_somDefaultInit;
  SOMTTemplateOutputC_Init_SOMObject_somDefaultInit(somSelf, ctrl);
  _stab=new SOMStringTableC();
  _fp=stdout;
  _somtLineLength=72;
  _somtCommentNewline=FALSE;
  _somtCommentStyle=somtDashesE;
}

SOM_Scope void SOMLINK somDestruct(SOMTTemplateOutputC *somSelf, octet doFree, somDestructCtrl* ctrl)
{
  SOMTTemplateOutputCData *somThis; /* set by BeginDestructor */
  somDestructCtrl globalCtrl;
  somBooleanVector myMask;
  SOMTTemplateOutputCMethodDebug("SOMTTemplateOutputC","somDestruct");
  SOMTTemplateOutputC_BeginDestructor;
  /*
  * local SOMTTemplateOutputC deinitialization code added by programmer
  */
  _stab->somFree();
  
  SOMTTemplateOutputC_EndDestructor;
}
