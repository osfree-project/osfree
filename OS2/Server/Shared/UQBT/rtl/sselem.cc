/*==============================================================================
 * FILE:       sselem.cc
 * OVERVIEW:   Defines the tree elements of the semantic string tree constructed
 *             by the parser when an rtl is parsed.
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * 22 Feb 2001 - Simon: created
 */


#include "global.h"
#include "sselem.h"

bool builtOpMap = false;
map<string, int> opMap;

void buildOpMap(void);

SSElem::SSElem(void)
{
}

SSElem::SSElem(int e) :
    value(e)
{
}

SSElem::~SSElem(void)
{
}

void SSElem::eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var, SemStr &s)
{
    s.push(value);
}

SemStr *SSElem::eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var)
{
    SemStr* s = new SemStr();
    eval(table, var, *s);
    return s;
}

void SSElem::setvalue(int e)
{
    value = e;
}

SSListElem::SSListElem(void) :
    SSElem()
{
}

SSListElem::~SSListElem(void)
{
    // kill all of the argument SSElems before destruction
    while (!arg.empty()) {
        delete arg.back();
        arg.pop_back();
    }
}

void SSListElem::eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var, SemStr &s)
{
    // s.push(value);
    for(list<SSElem*>::iterator loc = arg.begin(); loc != arg.end(); loc++)
        (*loc)->eval(table, var, s);
}

void SSListElem::app(SSElem* e)
{
    arg.push_back(e);
}

void SSListElem::prep(SSElem* e)
{
    arg.push_front(e);
}
void SSListElem::app(int e)
{
    arg.push_back(new SSElem(e));
}

void SSListElem::prep(int e)
{
    arg.push_front(new SSElem(e));
}

SSTableElem::SSTableElem(string t, string vr) :
    tname(t), vname(vr)
{
}

void SSTableElem::eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var, SemStr &s)
{
    s.pushSS(((ExprTable*)(table[tname]))->expressions[var[vname]->getvalue()]->eval(table, var));
}

SSOpTableElem::SSOpTableElem(string t, string vr) :
    tname(t), vname(vr)
{
    buildOpMap();
}

void SSOpTableElem::eval(map<string, Table*, StrCmp> &table, map<string, InsNameElem*, StrCmp> &var, SemStr &s)
{
    s.push(opMap[((OpTable*)(table[tname]))->records[var[vname]->getvalue()]]);
}

void buildOpMap(void)
{
  if (!builtOpMap) {
    opMap["+"] = idPlus;
	opMap["-"] = idMinus;
	opMap["*"] = idMult;
	opMap["/"] = idDiv;
	opMap["%"] = idMod;
	opMap["+f"] = idFPlus;
	opMap["+fd"] = idFPlusd;
	opMap["+fq"] = idFPlusq;
	opMap["-f"] = idFMinus;
	opMap["-fd"] = idFMinusd;
	opMap["-fq"] = idFMinusq;
	opMap["*f"] = idFMult;
	opMap["*fd"] = idFMultd;
	opMap["*fq"] = idFMultq;
	opMap["*fsd"] = idFMultsd;
	opMap["*fdq"] = idFMultdq;
	opMap["SQRTs"] = idSQRTs;
	opMap["SQRTd"] = idSQRTd;
	opMap["SQRTq"] = idSQRTq;
	opMap["/f"] = idFDiv;
	opMap["/fd"] = idFDivd;
	opMap["/fq"] = idFDivq;
	opMap["%"] = idMod;
	opMap["rlc"] = idRotateLC;
	opMap["rrc"] = idRotateRC;
	opMap["rl"] = idRotateL;
	opMap["rr"] = idRotateR;
	opMap[">>"] = idShiftR;
	opMap["<<"] = idShiftL;
	opMap[">>A"] = idShiftRA;
	opMap["|"] = idBitOr;
	opMap["&"] = idBitAnd;
	opMap["^"] = idBitXor;
  }
  builtOpMap = true;
}
