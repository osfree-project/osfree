/*==============================================================================
 * FILE:       table.cc
 * OVERVIEW:   Provides the implementation of classes Table, OpTable, and
 *             ExprTable
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * 25 Feb 2001 - Simon: sorted out dependancies
 */


#include "global.h"
#include "table.h"
#include "sselem.h"

Table::Table(TABLE_TYPE t) :
    type(t)
{
}

Table::Table(deque<string>& recs, TABLE_TYPE t /* = NAMETABLE */) :
    records(recs),type(t)
{
}

TABLE_TYPE Table::getType() const { return type; }

OpTable::OpTable(deque<string>& ops) :
    Table(ops, OPTABLE)
{
}

ExprTable::ExprTable(deque<SSElem*>& exprs) :
    Table(EXPRTABLE),expressions(exprs)
{
}

ExprTable::~ExprTable(void)
{
	deque<SSElem*>::iterator loc;
	for (loc = expressions.begin(); loc != expressions.end(); loc++)
		delete (*loc);
}
