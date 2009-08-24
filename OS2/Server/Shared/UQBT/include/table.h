/*==============================================================================
 * FILE:       table.h
 * OVERVIEW:   Provides the definition of class Table and children used by
 *             the SSL parser
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * 25 Feb 2001 - Simon: updated post creation
 */

#ifndef TABLE_H
#define TABLE_H

class Table {
public:
    Table(deque<string>& recs, TABLE_TYPE t = NAMETABLE);
    Table(TABLE_TYPE t);
    TABLE_TYPE getType() const;
    deque<string> records;

private:
    TABLE_TYPE type;
};

class OpTable : public Table {
public:
    OpTable(deque<string>& ops);
};

class SSElem;

class ExprTable : public Table {
public:
    ExprTable(deque<SSElem*>& exprs);
	~ExprTable(void);
    deque<SSElem*> expressions;
};

#endif
