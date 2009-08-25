/*
 * Copyright (C) 2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       rttelem.h
 * OVERVIEW:   Provides the definition of various Register Transfer Tree
 *             ELEMent classes, all derived from class RT. Used by the SSL
 *             parser
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * 22 Feb 2001 - Simon: created
 * 01 Aug 01 - Brian: New class HRTL replaces RTlist.
 *
 */

#ifndef RTTELEM_H
#define RTTELEM_H

#include "sselem.h"

class RTTElem {
public:
    virtual RT* eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var) = 0;
};

class RTTListElem {
public:
    RTTListElem(void);
    virtual ~RTTListElem(void);
    virtual HRTL* eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var);
    void append(RTTElem* r);

private:
    list<RTTElem*> rtt;
};

class RTTAssignElem : public RTTElem {
public:
    RTTAssignElem(SSElem* l, SSElem* r, Byte s);
    RTTAssignElem(SSElem* l, SSElem* r, string s); // note: string s is '*NUM*'s
    RTTAssignElem(SSElem* l, SSElem* r, string s, SSElem *c);
    virtual ~RTTAssignElem(void);
    virtual RT* eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var);

private:
    SSElem* lhs;
    SSElem* rhs;
    SSElem* cond;
    Byte rt_size;
};


class RTTFCallElem : public RTTElem {
public:
    RTTFCallElem(string n, list<SSElem*> &p);
    virtual ~RTTFCallElem(void);
    virtual RT* eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var);

private:
    string fname;
    list<SSElem*> param;
};


class RTTFDefElem : public RTTElem {
public:
        RTTFDefElem(string n, list<int> &p, RTTListElem* r);
        virtual ~RTTFDefElem(void);
        virtual RT* eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var);
private:
        string fname;
        list<int> param;
        RTTListElem* rtls;
};
#endif
