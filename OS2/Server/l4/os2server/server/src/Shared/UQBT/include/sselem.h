/*==============================================================================
 * FILE:       sselem.h
 * OVERVIEW:   Provides the definition of various classes related to tree
 *             elements of the semantic string constructed by the SSL
 *             parser (when an RTL is being parsed)
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * 22 Feb 2001 - Simon: created
 */

#ifndef SSELEM_H
#define SSELEM_H

#include "rtl.h"
#include "insnameelem.h"


// basic element of the semstr tree used by the parser to expand rtls
class SSElem {
public:
    SSElem(void);
    SSElem(int e);
    virtual ~SSElem(void);
    virtual void eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var, SemStr &s);
    SemStr *eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var);
    void setvalue(int e);
        
private:
    int value;
};

// a group of SSElem - an SSElem with a deque of arguments (other SSElem's)
class SSListElem : public SSElem {
public:
    SSListElem(void);
    virtual ~SSListElem(void);
    virtual void eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var, SemStr &s);
    virtual void app(SSElem* e);
    virtual void app(int e);
    virtual void prep(SSElem* e);
    virtual void prep(int e);

protected:
    list<SSElem*> arg;
};

// a single SSElem that can be any of a deque of semantic strings
class SSTableElem : public SSElem {
public:
    SSTableElem(string t, string vr);
    virtual void eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var, SemStr &s);

private:
    string tname;
    string vname;
};

// an expression SSElem (SSListElem) the 'operator' of which can be any of a list of values
class SSOpTableElem : public SSListElem {
public:
    SSOpTableElem(string t, string vr);
    virtual void eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var, SemStr &s);

private:
    string tname;
    string vname;
};

#endif
