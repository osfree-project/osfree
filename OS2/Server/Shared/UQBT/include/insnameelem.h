/*
 * insnameelem.h
 *
 * header for insnameelem.cc
 * class declarations for class InsNameElem
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 *
 * Last Modified
 * 19 Feb 01 - Simon: created
 * 28 Mar 01 - Simon: Added class InsOptionElem
 */

#ifndef INSNAMEELEM_H
#define INSNAMEELEM_H

#include "table.h"

class InsNameElem {

public:
    InsNameElem(string name);
    virtual ~InsNameElem(void);
    virtual int ntokens(void);
    virtual string getinstruction(void);
    virtual string getinspattern(void);
    virtual void getrefmap(map<string, InsNameElem*, StrCmp> &m);
 
    int ninstructions(void);
    void append(InsNameElem* next);
    bool increment(void);
    void reset(void);
    int getvalue(void);
    
protected:
    InsNameElem* nextelem;
    string elemname;
    int value;
};

class InsOptionElem : public InsNameElem {

public:
    InsOptionElem(string name);
    virtual int ntokens(void);
    virtual string getinstruction(void);
    virtual string getinspattern(void);
 
};

class InsListElem : public InsNameElem {

public:
    InsListElem(string name, Table* t, string idx);
    virtual int ntokens(void);
    virtual string getinstruction(void);
    virtual string getinspattern(void);
    virtual void getrefmap(map<string, InsNameElem*, StrCmp> &m);

    string getindex(void);

protected:
    string indexname;
    Table* thetable;
};

#endif
