/**************************************************************************
 *
 *  Copyright 2015, 2017 Yuri Prokushev
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

#ifndef SOM_Module_somstrt_Source
  #define SOM_Module_somstrt_Source
#endif /* SOM_Module_somstrt_Source */

#define SOMStringTableC_Class_Source


#include "somstrt.xih"

 
  typedef struct entryT {
      string key;
      string value;
      entryT *next;
  } *tablePT;

SOM_Scope short SOMLINK somstAssociate(SOMStringTableC SOMSTAR somSelf,
                                       /* in */ string key,
	                                     /* in */ string value)
{
  entryT *map;
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","somstAssociate");

  if (!key) return 0;
  if (!strnlen(key, SIZE_MAX)) return 0;
  if (_numberOfEntries==ULONG_MAX) return 0;
  
  if(_table->key==NULL) 
  {
    _table->key=key;
    _table->value=value;
    _numberOfEntries++;
    _table->next=NULL;
    return 1;
  }

  for(map=_table;;map=map->next) 
  {
    if(!strnicmp(key,map->key, SIZE_MAX)) 
    {
      map->value=value;
      _numberOfEntries++;
      return -1;
    }

    if(map->next==NULL) 
    {
      map->next=(struct entryT*)SOMMalloc(sizeof(struct entryT));
      if(!map->next) return 0;
      map=map->next;
      map->key=key;
      map->value=value;
      _numberOfEntries++;
      map->next=NULL;
      return 1;
    }
  }
};

SOM_Scope boolean SOMLINK somstClearAssociation(SOMStringTableC SOMSTAR somSelf,
	                                              /* in */ string key)
{
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","somstClearAssociation");

  if (!key) return 0;
  if (!strnlen(key, SIZE_MAX)) return 0;
  
  return TRUE;  
}                                                

SOM_Scope short SOMLINK somstAssociateCopyBoth(SOMStringTableC SOMSTAR somSelf,
	                                             /* in */ string key,
	                                             /* in */ string value)
{
  entryT *map;
  unsigned long value_len=0;
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","somstAssociateCopyBoth");
  
  if (!key) return 0;
  if (!strnlen(key, SIZE_MAX)) return 0;
  if (_numberOfEntries==ULONG_MAX) return 0;
  
  if (value) value_len=strnlen(value, SIZE_MAX);
  
  if(_table->key==NULL) 
  {
    _table->key=(string)SOMMalloc(strnlen(key, SIZE_MAX)+1);
    if(!_table->key) return 0;
    strncpy(_table->key,key, SIZE_MAX);
    _table->value=(string)SOMMalloc(value_len+1);
    if(!_table->value) return 0;
    if (value)
    {
      strncpy(_table->value,value, SIZE_MAX);
    } else {
      SOMFree(_table->value);
    }
    _numberOfEntries++;
    _table->next=NULL;
    return 1;
  }

  for(map=_table;;map=map->next) 
  {
    if(!strnicmp(key,map->key, SIZE_MAX)) 
    {
      if(map->value!=NULL) 
      {
        SOMFree(map->value);
        map->value=(string)SOMMalloc(value_len+1);
        if(!map->value) return 0;
        if (value)
        {
          strncpy(map->value,value, SIZE_MAX);
        } else {
          SOMFree(_table->value);
        }
        _numberOfEntries++;
        return -1;
      }
    }

    if(map->next==NULL) 
    {
      map->next=(struct entryT*)SOMMalloc(sizeof(struct entryT));
      if(!map->next) return 0;
      map=map->next;
      map->key=(string)SOMMalloc(strnlen(key, SIZE_MAX)+1);
      if(!map->key) return 0;
      strncpy(map->key,key, SIZE_MAX);
      map->value=(string)SOMMalloc(value_len+1);
      if(!map->value) return 0;
      if (value)
      {
        strncpy(map->value,value, SIZE_MAX);
      } else {
        SOMFree(_table->value);
      }
      _numberOfEntries++;
      map->next=NULL;
      return 1;
    }
  }
}

SOM_Scope string SOMLINK somstGetIthValue(SOMStringTableC SOMSTAR somSelf,
	                                        /* in */ unsigned long i)
{
  struct entryT *map;
  unsigned long count = 0;
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","somstGetIthValue");

  for(map=_table;map!=NULL;map=map->next) 
  {
    count++;
    if(map->key && count==i) 
    {
      return map->value;
    }
  }
  return NULL;
}

SOM_Scope string SOMLINK somstGetAssociation(SOMStringTableC SOMSTAR somSelf,
	                                           /* in */ string key)
{
  struct entryT *map;
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","somstGetAssociation");

  if (!key) return NULL;
  if (!strnlen(key, SIZE_MAX)) return NULL;
  
  for(map=_table;map!=NULL;map=map->next) 
  {
    if(map->key && !strnicmp(key,map->key, SIZE_MAX)) 
    {
      return map->value;
    }
  }
  return NULL;
}

SOM_Scope string SOMLINK somstGetIthKey(SOMStringTableC SOMSTAR somSelf,
	                                      /* in */ unsigned long i)
{
  struct entryT *map;
  unsigned long count=0;
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","somstGetIthKey");

  for(map=_table;map!=NULL;map=map->next) 
  {
    count++;
    if(map->key && count==i) 
    {
      return map->key;
    }
  }
  return NULL;
}

SOM_Scope void SOMLINK somDefaultInit(SOMStringTableC SOMSTAR somSelf, somInitCtrl *ctrl)
{
  SOMStringTableCData *somThis; /* set by BeginInitializer */
  somInitCtrl globalCtrl;
  somBooleanVector myMask;
  SOMStringTableCMethodDebug("SOMStringTableC", "somDefaultInit");
  
  SOMStringTableC_BeginInitializer_somDefaultInit;
  SOMStringTableC_Init_SOMObject_somDefaultInit(somSelf, ctrl);
  
  _table=(struct entryT *)SOMMalloc(sizeof(struct entryT));
  if(!_table) return;
  _table->key=NULL;
  _table->value=NULL;
  _table->next=NULL;
  _tableSize=100;
  _numberOfEntries=0;
}

SOM_Scope short SOMLINK somstAssociateCopyKey(SOMStringTableC SOMSTAR somSelf,
	                                            /* in */ string key,
	                                            /* in */ string value)
{
  entryT *map;
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","somstAssociateCopyKey");

  if (!key) return 0;
  if (!strnlen(key, SIZE_MAX)) return 0;
  if (_numberOfEntries==ULONG_MAX) return 0;

  if(_table->key==NULL) 
  {
    _table->key=(string)SOMMalloc(strnlen(key, SIZE_MAX)+1);
    if(!_table->key) return 0;
    strncpy(_table->key,key, SIZE_MAX);
    _table->value=value;
    _numberOfEntries++;
    _table->next=NULL;
    return 1;
  }

  for(map=_table;;map=map->next) 
  {
    if(!strnicmp(key,map->key, SIZE_MAX)) 
    {
      if(map->value!=NULL) 
      {
        map->value=value;
        _numberOfEntries++;
        return -1;
      }
    }

    if(map->next==NULL) 
    {
      map->next=(struct entryT*)SOMMalloc(sizeof(struct entryT));
      if(!map->next) return 0;
      map=map->next;
      map->key=(string)SOMMalloc(strnlen(key, SIZE_MAX)+1);
      if(!map->key) return 0;
      strncpy(map->key,key, SIZE_MAX);
      map->value=value;
      _numberOfEntries++;
      map->next=NULL;
      return 1;
    }
  }
}

SOM_Scope unsigned long SOMLINK _get_somstTargetCapacity(SOMStringTableC SOMSTAR somSelf)
{
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","_get_somstTargetCapacity");

  return _tableSize;
};


SOM_Scope void SOMLINK _set_somstTargetCapacity(SOMStringTableC SOMSTAR somSelf,
	                                              /* in */ unsigned long somstTargetCapacity)
{
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","_set_somstTargetCapacity");

  _tableSize=somstTargetCapacity;
}

SOM_Scope short SOMLINK somstAssociateCopyValue(SOMStringTableC SOMSTAR somSelf,
	                                              /* in */ string key,
	                                              /* in */ string value)
{
  entryT *map;
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","somstAssociateCopyValue");
  
  if (!key) return 0;
  if (!strnlen(key, SIZE_MAX)) return 0;
  if (_numberOfEntries==ULONG_MAX) return 0;

  if(_table->key==NULL) 
  {
    _table->key=key;
    _table->value=(string)SOMMalloc(strnlen(value, SIZE_MAX)+1);
    if(!_table->value) return 0;
    strncpy(_table->value,value, SIZE_MAX);
    _numberOfEntries++;
    _table->next=NULL;
    return 1;
  }

  for(map=_table;;map=map->next) 
  {
    if(!strnicmp(key,map->key, SIZE_MAX)) 
    {
      if(map->value!=NULL) 
      {
        SOMFree(map->value);
        map->value=(string)SOMMalloc(strnlen(value, SIZE_MAX)+1);
        if(!map->value) return 0;
        strncpy(map->value,value, SIZE_MAX);
        _numberOfEntries++;
        return -1;
      }
    }

    if(map->next==NULL) 
    {
      map->next=(struct entryT*)SOMMalloc(sizeof(struct entryT));
      if(!map->next) return 0;
      map=map->next;
      map->key=key;
      map->value=(string)SOMMalloc(strnlen(value, SIZE_MAX)+1);
      if(!map->value) return 0;
      strncpy(map->value,value, SIZE_MAX);
      _numberOfEntries++;
      map->next=NULL;
      return 1;
    }
  }
}
	 
SOM_Scope void SOMLINK somDestruct(SOMStringTableC *somSelf, octet doFree, somDestructCtrl* ctrl)
{
  struct entryT *m, *mp;
  SOMStringTableCData *somThis; /* set by BeginDestructor */
  somDestructCtrl globalCtrl;
  somBooleanVector myMask;
  SOMStringTableCMethodDebug(”SOMStringTableC”,”somDestruct”);
  SOMStringTableC_BeginDestructor;
  /*
  * local SOMStringTableC deinitialization code added by programmer
  */
  if(!_table) return;
  m=_table;
  while(m!=NULL) 
  {
    if(m->key!=NULL) SOMFree(m->key);
    if(m->value!=NULL) SOMFree(m->value);
    mp=m;
    m=m->next;
    SOMFree(mp);
  }
  SOMFree(_table);
  
  SOMStringTableC_EndDestructor;
}

SOM_Scope void SOMLINK somDumpSelfInt(SOMStringTableC SOMSTAR somSelf, long level)
{
  struct entryT *m;
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  Environment *ev = somGetGlobalEnvironment();
  SOMStringTableC_parent_SOMObject_somDumpSelfInt(somSelf, level);
  
  if(!_table) return;
  m=_table;
  while(m!=NULL) 
  {
    somLPrintf(level, "Key: %s Value: %s\n", m->key, m->value);
    m=m->next;
  }
}

SOM_Scope unsigned long SOMLINK _get_somstAssociationsCount(SOMStringTableC SOMSTAR somSelf)
{
  SOMStringTableCData *somThis = SOMStringTableCGetData(somSelf);
  SOMStringTableCMethodDebug("SOMStringTableC","_get_somstAssociationsCount");

  return _numberOfEntries;
}
