/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:       sledtree.h
 * OVERVIEW:   Class declarations for the syntax tree constructed by the SLED
 *             parser.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *===========================================================================*/

/* $Revision: 1.6 $
 * 23 Apr 01 - Nathan: Initial version
 */

/*
 * Notes on memory usage: In general each node is owned by it's
 * parent node, and it is the parent destructors responsibility to
 * destroy it's children. Exceptions: Identifiers are owned by the
 * symbol table, and should not be cloned / destroyed.
 *
 * The substitute and solve operations construct a new subtree
 * (again, except for identifiers), and the caller may destroy the
 * original tree if desired
 *
 * Internal tree code should use destroy(obj), rather than delete -
 * the former checks whether the object should actually be deleted
 */

#ifndef genem_sledtree_h
#define genem_sledtree_h

#include <vector>
#include <string>
#include <hash_map.h>
#include <map>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include "bitstring.h"
#include "symtab.h"

class BitSlice {
  public:
    int lo, hi;
};

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_RELOC, TYPE_STRING, TYPE_BITS,
               TYPE_LIST, TYPE_ANY, TYPE_UNKNOWN } ExprType;
typedef enum { EXPR, FIELD, PATTERN, RELOC,
               OPERAND, INSTRUCTION, ATOMIC, UNARY, BINARY, OFFSET,
               BITSLICE, LIST, GENERATOR, NULLEXPR, INVOKER } ExprKind;
typedef enum { REL_LE = OP_LEQUAL, REL_LS = OP_LESS, REL_EQ = OP_EQUAL,
               REL_NE = OP_NOTEQUAL, REL_GS = OP_GREATER, REL_GE = OP_GEQUAL,
               PAT_OR, PAT_AND, PAT_SEQ, PAT_RDOT, PAT_LDOT,
               EXP_ADD, EXP_SUB, EXP_MUL, EXP_DIV } ExprOp;
#define IS_ARITH_OP(x) ((x)>=EXP_ADD && (x)<=EXP_DIV)
#define IS_REL_OP(x)   ((x)>=REL_LE && (x)<=REL_GE)
#define IS_LOG_OP(x)   ((x)>=PAT_OR && (x)<=PAT_SEQ)

class Expr {
  public:
    Expr( ExprType t ) { type = t; posn = -1; }
    virtual ~Expr() {}

    typedef map<Expr *, Expr *> subst_func_t;

    class iterator {
      public:
        iterator( Expr *start = NULL ) { node = start; }
        virtual ~iterator() {}
        bool operator ==( const iterator &a ) const {
            return a.node == node; }
        Expr &operator *() const { return *node; }
        iterator &operator ++() { next( true ); return *this; }
        /*
         * move to the next child of the parent immediately, ie skip over
         * the current subtree entirely
         */
        iterator &skip() { next( false ); return *this; }
        iterator &up(){
            if( stack.empty() ) node = NULL;
            else node = stack.back(); stack.pop_back();
            return *this;
        }
      protected:
        Expr *node;
        vector<Expr *> stack;
        virtual void next( bool descend /* Go down from current node */ ) = 0;
    };
    class PreorderIterator : public iterator {
      public:
        PreorderIterator( Expr *start ) : iterator( start ) {} 
      protected:
        void next( bool descend ) {
            Expr *tmp, *parent = node;
            if( node == NULL ) return;
            node = (descend ? node = parent->getChild( ) : NULL );
            while( node == NULL && !stack.empty() ) {
                tmp = parent;
                parent = stack.back();
                stack.pop_back();
                node = parent->getNextChild( tmp );
            }
            if( node )
                stack.push_back(parent);
        }
    };
    class PostorderIterator : public iterator {
      public:
        PostorderIterator( Expr *start ) : iterator( start ) { if( start ) fall(); }
      protected:
        void next( bool descend /* not really meaningful here */ ) {
            if( node == NULL ) return;
            if( stack.empty() ) {
                node = NULL;
                return;
            }
            Expr *parent = stack.back(); stack.pop_back();
            node = parent->getNextChild( node );
            if( node == NULL )
                node = parent;
            else fall();
        }
        void fall() {
            while( node ) {
                stack.push_back(node);
                node = node->getChild();
            }
            node = stack.back();
            stack.pop_back();
        }
    };
    
    PostorderIterator begin() { return PostorderIterator(this); }
    PostorderIterator end() { return PostorderIterator(NULL); }
    PreorderIterator preorderBegin() { return PreorderIterator(this); }
    PreorderIterator preorderEnd() { return PreorderIterator(this); }
    
    static void destroy( Expr *e ) { if( e && ! e->isIdent() ) delete e; }
    
    virtual void printOn( FILE *f = stdout ) const = 0;
    virtual ExprKind getKind() const { return EXPR; }
    virtual bool isIdent() const { return false; }
    virtual Expr *clone( ) = 0;
    virtual Expr *getChild( ) { return getNextChild( NULL ); }
    virtual Expr *getNextChild( Expr *after ) { return NULL; }

    virtual Expr *expand(int *n) { return clone(); }
    virtual Expr *substitute( subst_func_t *sub )
        {return ((*sub)[this] ? ((*sub)[this])->clone() : clone()); }
    virtual Expr *getContent( void ) { return this; }
    virtual int computePosn( int in ) { posn = in; return in; }
    virtual Expr *solve( void ) { return clone(); }
    
    ExprType type;
    int posn;

#ifdef DEBUG_SLED
    void dprintOn( FILE *f ) const { printOn(f); }
#else
    void dprintOn( FILE *f ) const {}
#endif
};

class Ident;
class BitStringExpr;
#if 0
 class Symtab {
  public:
    Symtab() { syms = new IdentMap(NULL); }
    Ident *lookup( const string *str );
    Ident *add( Ident *id );
    void enterScope(void);
    void leaveScope(void);
    /* Swap the topmost scope with the one immediately below it. Required by
     * typed constructors...
     */
    void swapScope(void);
    virtual void printOn( FILE *f ) const;

    typedef hash_map<string, Ident *, hashString> IdentHash;
    class IdentMap {
      public:
        IdentMap(IdentMap *chain) { next = chain; }
        IdentHash hash;
        IdentMap *next;
    };
    IdentMap *syms;
};
#endif
typedef SymbolTable<Ident,1,false> Symtab;
extern Symtab symtab;

class Ident : public Expr {
  public:
    Ident(string *s, ExprType t) : Expr(t) { name = *s; symtab.add(*s, this); }
    
    virtual void printOn( FILE *f ) const { fprintf(f,"Ident: %s\n", name.c_str()); }
    virtual bool isIdent( ) const { return true; }
    virtual Expr *clone( ) { return this; } /* idents don't clone */
    
    string name;

};

typedef enum { FC_CHECKED, FC_UNCHECKED, FC_GUARANTEED } FieldChecked;

class FieldIdent : public Ident {
  public:
    int lo,hi;
    FieldIdent *parent;
    FieldChecked checked;
    vector<string *> *names;
    int maxDisasmLen;

    FieldIdent( string *s, int w ) : Ident(s, TYPE_BITS) {
        lo = 0; hi = w-1; checked = FC_CHECKED;
        maxDisasmLen = w/3+2;
    }
    FieldIdent( string *s, int l, int h ) : Ident(s, TYPE_BITS) {
        lo = l; hi = h; checked = FC_CHECKED;
        maxDisasmLen = (h-l+1)/3+2;
    }
    ~FieldIdent( ) { for( unsigned i=0; i<names->size(); i++ ) { delete (*names)[i]; }}
    
    int getSize( void );
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return FIELD; }
    virtual int computePosn( int in ) { posn = in; return in + getSize(); }
    
    BitStringExpr *toBitStringExpr( );

    /* return true if this field has another names (ie registers) */
    bool hasNames( void ) const { return ( names && names->size() > 0 ); }
    
    /*
     * find the given name in the field's name list. If found, set
     * result to the index of the name, and return true. Otherwise
     * return false and leave result alone.
     */
    bool findName( const string &name, int *result ) const;

    /*
     * Return whether two fields are equivalent, = they have exactly
     * the same names in the same positions
     */
    bool isEquivalent( const FieldIdent &other ) const;
    
    /* Return whether two fields overlap, = they have some of the
     * same register names, possibly in different positions
     */
    bool overlaps( const FieldIdent &other ) const;
};

class RelocIdent : public Ident {
  public:
    RelocIdent( string *s ) : Ident( s, TYPE_RELOC ) {}
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return RELOC; }
};

class PatternIdent : public Ident {
  public:
    PatternIdent( string *s, Expr *p) : Ident(s, TYPE_UNKNOWN) { expr = p; maxDisasmLen = 0; }
    virtual ~PatternIdent() { destroy(expr); }
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return PATTERN; }
    virtual Expr *getContent( void ) { return expr; }
    virtual int computePosn( int in ) { posn = in; return expr->computePosn(in); }
    virtual Expr *solve( void ) { return expr->solve(); }

    void add( Expr *e, ExprOp op );
    Expr *expr;
    int maxDisasmLen;
};

class BinaryExpr : public Expr {
  public:
    BinaryExpr( ExprType t, ExprOp o, Expr *l, Expr *r ) : Expr(t) {
        op = o; left = l; right = r; }
    virtual ~BinaryExpr() { destroy(left); destroy(right); }
    
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return BINARY; }
    virtual Expr *clone( ) { return new BinaryExpr(type, op, left->clone(),
                                                   right->clone() ); }
    virtual Expr *getNextChild( Expr *a ) {
        if( !a ) return left;
        else if( a == left ) return right;
        else return NULL;
    }
    virtual Expr *expand(int *n);
    virtual Expr *substitute( subst_func_t *sub );
    virtual Expr *solve( void );
    virtual int computePosn( int in );
    ExprOp op;
    Expr *left;
    Expr *right;
};

class UnaryExpr : public Expr {
  public:
    UnaryExpr( ExprType t, int o, Expr *l ) : Expr(t) { op = o; left = l; }
    virtual ~UnaryExpr() { destroy(left); }
    
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return UNARY; }
    virtual Expr *clone( ) { return new UnaryExpr( type, op, left->clone() ); }
    virtual Expr *getNextChild( Expr *a ) { return (a == NULL ? left : NULL); }
    
    virtual Expr *expand(int *n);
    virtual Expr *substitute( subst_func_t *sub );
    virtual Expr *solve( void );
    virtual int computePosn( int in ) { posn = in; return in + left->computePosn(in); }
    int op;
    Expr *left;
};

class IntegerExpr : public Expr {
  public:
    IntegerExpr( int val ) : Expr(TYPE_INT) { value = val; }
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return ATOMIC; }
    virtual Expr *clone( ) { return new IntegerExpr(value); }

    int value;
};

class FloatExpr : public Expr {
  public:
    FloatExpr( double val ) : Expr(TYPE_FLOAT) { value = val; }
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return ATOMIC; }
    virtual Expr *clone( ) { return new FloatExpr(value); }

    double value;
};

class StringExpr : public Expr {
  public:
    StringExpr( string *s ) : Expr(TYPE_STRING) { value = *s; }
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return ATOMIC; }
    virtual Expr *clone( ) { return new StringExpr(&value); }

    string value;
};

class OffsetExpr : public UnaryExpr {
  public:
    OffsetExpr( Expr *e ) : UnaryExpr(TYPE_UNKNOWN, 0, e) {};
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return OFFSET; }
    virtual Expr *clone( ) { return new OffsetExpr(left->clone()); }
};

class BitSliceExpr : public UnaryExpr {
  public:
    BitSliceExpr( Expr *l, BitSlice r ) : UnaryExpr(TYPE_INT, 0, l) { range = r; }
    virtual void printOn( FILE *f ) const;
    virtual Expr *clone( ) { return new BitSliceExpr( left->clone(), range ); }

    BitSlice range;
};

class BitStringExpr : public Expr {
  public:
    BitStringExpr() : Expr(TYPE_BITS) { }
    BitStringExpr( FieldIdent *f, ExprOp op, int val ) : Expr(TYPE_BITS) {
        strs.add(f->getSize(), op, f->lo, f->hi, val );
    }
    BitStringExpr( FieldIdent *a, FieldIdent *b, ExprOp op ) : Expr(TYPE_BITS){
        strs.add( a->getSize() > b->getSize() ? a->getSize() : b->getSize(),
                  op, a->lo, a->hi, b->lo, b->hi );
    }
    BitStringExpr( BitStringSet &str ) : Expr(TYPE_BITS) { strs = str; }
    virtual void printOn( FILE *f ) const { strs.printOn(f); }
    virtual ExprKind getKind( ) const { return ATOMIC; }
    virtual Expr *clone( ) { return new BitStringExpr(*this); }
    
    BitStringSet strs;
};

class RangeExpr : public Expr {
  public:
    RangeExpr( int l, int h ) : Expr(TYPE_INT) { lo = l; hi = h; cols = 1;}
    RangeExpr( int l, int h, int c ) : Expr(TYPE_INT) {
        lo = l; hi = h; cols = c; }

    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return GENERATOR; }
    virtual Expr *clone( ) { return new RangeExpr(lo,hi,cols); }
    virtual Expr *expand(int *n);

    int lo,hi,cols;
};

class ListExpr : public Expr {
  public:
    ListExpr( ) : Expr(TYPE_LIST) { }
    virtual ~ListExpr() { for( unsigned i=0; i<exprs.size();i++) destroy(exprs[i]); }
    typedef vector<Expr *>::iterator iterator;
    typedef vector<Expr *>::const_iterator const_iterator;
    
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return LIST; }
    virtual Expr *clone( ) {
        ListExpr *copy = new ListExpr();
        cloneMembers( copy );
        return copy;
    }
    virtual Expr *getNextChild( Expr *a ) {
        if( a == NULL ) lastChild = exprs.begin();
        else if( *lastChild == a ) lastChild++;
        else lastChild = find(exprs.begin(), exprs.end(), a);
        if( lastChild == exprs.end() )
            return NULL;
        return *lastChild;
    }
    virtual Expr *substitute( subst_func_t *sub );
    virtual Expr *expand(int *n);
    virtual Expr *solve();

    Expr *add( Expr *exp ) { exprs.push_back( exp ); return exp; }
    Expr *addAll( Expr *exp );
    Expr *addAll( ListExpr *exp );
    
    vector<Expr *> exprs;

  protected:
    void cloneMembers( ListExpr *copy ) {
        for( iterator i = exprs.begin(); i != exprs.end(); i++ )
            copy->add( (*i)->clone() );
    }

    /* When iterating with getNextChild, hold onto the last referenced node for
     * efficiency.
     */
    iterator lastChild;
};
class GeneratorExpr : public ListExpr {
  public:
    virtual ExprKind getKind( ) const { return GENERATOR; }
    virtual Expr *expand(int *n);
    virtual Expr *clone( ) {
        ListExpr *copy = new GeneratorExpr();
        cloneMembers( copy );
        return copy;
    }
};

class NullExpr : public Expr {
  public:
    NullExpr( ) : Expr(TYPE_ANY) { }
    virtual void printOn( FILE *f ) const { fprintf( f, "epsilon" ); }
    virtual ExprKind getKind( ) const { return NULLEXPR; }
    virtual Expr *clone() { return new NullExpr(); }
};

class OperandIdent : public PatternIdent {
  public:
    OperandIdent( string *s, bool sign, Expr *e ) : PatternIdent( s, e ) {
        isSigned = sign; type = TYPE_UNKNOWN;
    }
    virtual void printOn ( FILE *f ) const;
    virtual ExprKind getKind( ) const { return OPERAND; }
    virtual int computePosn( int in ) { posn = in; return in; }
    virtual Expr *solve(void) { return (expr ? expr->clone() : this); }
    bool isSigned;
    bool isImmed; /* ie disassembly produces a numeric value */
    int maxDisasmLen;
};
    
class InstIdent : public Ident {
  public:
    InstIdent( string *s, Expr *e, Expr *oe, vector<OperandIdent *> *ops,
               string &asmstr, int mdl ) : Ident( s, TYPE_UNKNOWN ) {
        expr = e; params = *ops; disasm = asmstr; maxDisasmLen = mdl;
        type = NULL; origExpr = oe;
    }
    virtual ~InstIdent() { destroy(expr); destroy(origExpr); }
    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return INSTRUCTION; }
    virtual bool specializes( const InstIdent &inst ) const;
    virtual bool isSynth( ) const;
    virtual Expr *solve();
    
    Expr *expr, *origExpr;
    PatternIdent *type; /* as in typed constructor, not operation type *sigh*/
    vector<OperandIdent *> params;
    string disasm;
    int maxDisasmLen;
};

class InvokeExpr : public Expr {
  public:
    InvokeExpr( InstIdent *i, ListExpr *p ) : Expr(TYPE_UNKNOWN){
        inst = i; params = p->exprs;
    }

    virtual void printOn( FILE *f ) const;
    virtual ExprKind getKind( ) const { return INVOKER; }
    virtual Expr *clone() { return new InvokeExpr(*this); }
    InstIdent *inst;
    vector<Expr *> params;
};

typedef struct {
    string *name;
    ListExpr *list;
    Expr *expr;
} OpcodePart;

class SledDict {
  public:
    map<string, InstIdent *> insts;
    map<string, PatternIdent *> types;
    map<string, FieldIdent *> fields;

    typedef map<string, InstIdent *>::const_iterator inst_iterator;
    typedef map<string, PatternIdent *>::const_iterator type_iterator;
    typedef map<string, FieldIdent *>::const_iterator field_iterator;

    void printOn( FILE *out = stdout ) const;

    /* Parser function */
    bool readSLEDFile( const string &fname );

    /* Public so the parser can access them, but should be considered private
     * to everyone else
     */
    void InstantiateInstructions( vector<OpcodePart> *opcodes,
                                  vector<OperandIdent *> *operands,
                                  string *asmstr, string *type,
                                  Expr *equations, Expr *branches );
    Expr *InvokeInstruction( string *name, ListExpr *params );
    list<InstIdent *> orderInstructions( );
};

/* Sortof unrelated */
int yylex();
int yyerror(char *);
extern int yylineno;
extern const char *yyfilename;

#endif /*!genem_sledtree_h*/
