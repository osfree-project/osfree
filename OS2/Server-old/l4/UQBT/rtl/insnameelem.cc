/*
 * insnameelem.cc
 *
 * an element of an instruction name - contains definition of class InsNameElem
 *
 * Last Modified
 * 19 Feb 01 - Simon: created
 * 21 Mar 01 - Simon: Fixed increment/overflow problem in InsNameElem::increment
 *
 */

#include "global.h"
#include "insnameelem.h"

InsNameElem::InsNameElem(string name)
{
	elemname = name;
    value = 0;
    nextelem = NULL;
}

InsNameElem::~InsNameElem(void)
{
    delete nextelem;
}

int InsNameElem::ntokens(void)
{
    return 1;
}

string InsNameElem::getinstruction(void)
{
    return (nextelem != NULL)? (elemname + nextelem->getinstruction()): elemname;
}

string InsNameElem::getinspattern(void)
{
    return (nextelem != NULL)? (elemname + nextelem->getinspattern()): elemname;
}

void InsNameElem::getrefmap(map<string, InsNameElem*, StrCmp> &m)
{
	if (nextelem != NULL)
        nextelem->getrefmap(m);
    else
        m.erase(m.begin(), m.end());
}

int InsNameElem::ninstructions(void)
{
    return (nextelem != NULL)? (nextelem->ninstructions() * ntokens()): ntokens();
}

void InsNameElem::append(InsNameElem* next)
{
    if (nextelem == NULL)
        nextelem = next;
    else
        nextelem->append(next);
}

bool InsNameElem::increment(void)
{
    if ((nextelem == NULL) || nextelem->increment())
        value++;
    if (value >= ntokens()) {
        value = 0;
        return true;
    }
    return false;
}

void InsNameElem::reset(void)
{
    value = 0;
    if (nextelem != NULL) nextelem->reset();
}

int InsNameElem::getvalue(void)
{
    return value;
}

InsOptionElem::InsOptionElem(string name):
    InsNameElem(name)
{
}

int InsOptionElem::ntokens(void)
{
    return 2;
}

string InsOptionElem::getinstruction(void)
{
    string s = (nextelem != NULL)
        ? ((getvalue() == 0)
            ? (elemname + nextelem->getinstruction())
            : nextelem->getinstruction())
        : ((getvalue() == 0)
            ? elemname
            : "");
    return s;
}

string InsOptionElem::getinspattern(void)
{
    return (nextelem != NULL)
        ? ('\'' + elemname + '\'' + nextelem->getinspattern())
        : ('\'' + elemname + '\'');
}

InsListElem::InsListElem(string name, Table* t, string idx):
    InsNameElem(name)
{
    indexname = idx;
    thetable = t;
}

int InsListElem::ntokens(void)
{
    return thetable->records.size();
}

string InsListElem::getinstruction(void)
{
    return (nextelem != NULL)
        ? (thetable->records[getvalue()] + nextelem->getinstruction())
        : thetable->records[getvalue()];
}

string InsListElem::getinspattern(void)
{
    return (nextelem != NULL)
        ? (elemname + '[' + indexname + ']' + nextelem->getinspattern())
        : (elemname + '[' + indexname + ']');
}

void InsListElem::getrefmap(map<string, InsNameElem*, StrCmp> &m)
{
	if (nextelem != NULL)
		nextelem->getrefmap(m);
    else
		m.erase(m.begin(), m.end());
	m[indexname] = this;
    // of course, we're assuming that we've already checked (try in the parser)
    // that indexname hasn't been used more than once on this line ..
}

string InsListElem::getindex(void)
{
    return indexname;
}
