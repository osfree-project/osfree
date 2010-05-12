/* File: expr_hide.h
 * Desc: hidden interface for the OpTableExpr and CondTableExpr classes
 *
 * Copyright (C) 1997-1999, The University of Queensland, BT group
 *
 * $Revision: 1.6 $
 * Last modified: 11 Dec 1997
 * 24 Jun 98 - Mike: Mods for SemStr
 * 06 Jan 99 - Mike: Added virtual destructors for OpTableExpr and CondTableExpr
*/

#ifndef EXPR_HIDE_H
#define EXPR_HIDE_H

// OpTableExpr is an expression that will be expanded into multiple BinExprs
// It is made a subclass of SemStr so that the type of $$ in the parser
// can still be <ss> (i.e. SemStr*), and still have all this info

class OpTableExpr : public SemStr {
public:
				OpTableExpr(string t, SemStr* l, SemStr* r); // constructor
virtual 		~OpTableExpr(void);							 // destructor
				OpTableExpr(const OpTableExpr& other);		 // copy constr

	bool		operator==(class OpTableExpr &other) const;
	string		getName() const;			// return the operator table name
virtual			SemStr* expand(Table* table, int i);
virtual	void	print(ostream& os = cout) const;
virtual	void	printPrefix(ostream& os = cout) const;

private:
	string tname;				// the operator table name
	SemStr	*left;				// left expression
	SemStr	*right;				// right expression

	void buildMap();			// Function to build the map, if needed
};

// CondTableExpr is the template for an expression defined by an SSL table
class CondTableExpr : public SemStr {
public:
				CondTableExpr(string t);					// constructor
virtual			~CondTableExpr(void);						// destructor
				CondTableExpr(const CondTableExpr& other);	// copy cons

	bool 		operator==(class CondTableExpr &other) const;
	string		getName() const;		// return the conditional table name
virtual	SemStr* expand(Table* table, int i);
virtual	void	print(ostream& os = cout) const;
virtual	void	printPrefix(ostream& os = cout) const;

private:
	string tname;				// the conditional table name
};

#endif
