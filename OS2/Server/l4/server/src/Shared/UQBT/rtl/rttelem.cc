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
 * FILE:       rttelem.cc
 * OVERVIEW:   Provides the implementation of classes which produce RTs when
 *             when eval() is called; contains (in tree format) SSElem's which
 *             form SemStr's to create HRTLs
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.6 $
 * 22 Feb 2001 - Simon: created
 * 19 Jul 01 - Nathan: Added support for conditional assignments
 * 26 Jul 01 - Mike: minor tidying up
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Added new RTL class.
 */

#include "global.h"
#include "hrtl.h"
#include "rttelem.h"

static Byte extractNum(string s);

RTTListElem::RTTListElem(void)
{
}

RTTListElem::~RTTListElem(void)
{
    while (!rtt.empty()) {
        delete rtt.front();
        rtt.pop_front();
    }
}

HRTL* RTTListElem::eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var)
{
    HRTL* rt = new RTL();
    for(list<RTTElem*>::iterator loc = rtt.begin(); loc != rtt.end(); loc++)
        rt->appendRT((*loc)->eval(table, var));
    return rt;
}

void RTTListElem::append(RTTElem* r)
{
    rtt.push_back(r);
}

RTTAssignElem::RTTAssignElem(SSElem* l, SSElem *r, Byte s) :
    lhs(l), rhs(r), cond(NULL), rt_size(s)
{
}

RTTAssignElem::RTTAssignElem(SSElem* l, SSElem *r, string s) :
    lhs(l), rhs(r), cond(NULL), rt_size(extractNum(s))
{
}

RTTAssignElem::RTTAssignElem(SSElem* l, SSElem *r, string s, SSElem *c) :
        lhs(l), rhs(r), cond(c), rt_size(extractNum(s))
{
}

RTTAssignElem::~RTTAssignElem(void)
{
    delete lhs;
    delete rhs;
    if (cond) delete cond;
}

RT* RTTAssignElem::eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var)
{
    return new RTAssgn(lhs->eval(table, var), rhs->eval(table, var), cond?cond->eval(table,var):NULL, rt_size);
}

RTTFCallElem::RTTFCallElem(string n, list<SSElem*> &p) :
    fname(n), param(p)
{
}

RTTFCallElem::~RTTFCallElem(void)
{
    while(!param.empty()) {
        delete param.front();
        param.pop_front();
    }
}

RT* RTTFCallElem::eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var)
{
    list<SemStr*> l;
    RTFlagCall* r = new RTFlagCall(fname);
    for (list<SSElem*>::iterator loc = param.begin(); loc != param.end(); loc++) {
        l.push_back((*loc)->eval(table, var));
    }
    r->actuals = l;
    return r;
}

RTTFDefElem::RTTFDefElem(string n, list<int> &p, RTTListElem* r) :
    fname(n), param(p), rtls(r)
{
}

RTTFDefElem::~RTTFDefElem(void)
{
    delete rtls;
}

RT* RTTFDefElem::eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var)
{
    RTFlagDef* r = new RTFlagDef(fname);
    r->params = param;
    r->flag_rtl = rtls->eval(table, var);
    return r;
}

static Byte extractNum(string s)
{
    // make sure the string is of the form we expect
    assert(s[0] == '*' && s[s.length() - 1] == '*');

    return atoi(s.substr(1,s.length() - 2).c_str());
}

