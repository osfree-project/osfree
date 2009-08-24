/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:       symtab.h
 * OVERVIEW:   Definition and implementation of a lexically-scoped symbol
 *             table, implemented using linked hash tables.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *===========================================================================*/

/* $Revision: 1.4 $
 * 23 Apr 01 - Nathan: Initial version
 */

#include <stdio.h>
#include <hash_map.h>
#include <slist.h>

/*
 * class SymbolTable
 *
 * Template parameters:
 *         TIdent: base type of an identifier
 *    TNamespaces: Number of distinct namespaces, by default 1
 *     ownsIdents: Flag to indicate whether the table is considered to
 *                 own its identifiers - if so they will be deleted
 *                 automatically when passing out of scope
 *
 * General notes:
 *    An instance of SymbolTable stores a mapping from strings to TIdent
 *    pointers as a series of chained hash tables. It handles the usual 
 *    form of lexical scoping[0], and can hold several independent 
 *    namespaces[1] at the same time.
 *
 *    Each scope is referenced by a scope id, which is simply the 0-based 
 *    depth of the scope stack (ie the root scope is 0). Functions taking a
 *    scope id can also be given a negative id, which is treated as relative to
 *    the current scope, ie -1 gives the immediately surrounding scope.
 *
 *    [0] ie scopes are strictly nested - names declared in outer scopes are 
 *        visible to inner scopes, but not vice versa. Inner scopes may 
 *        override the definition of any names declared in outer scope, but 
 *        names may not normally be redeclared within the same scope.
 *
 *    [1] Note that this has nothing to do with C++ style namespaces. For the
 *        purpose of this class, namespaces are independent identifier domains,
 *        that are grouped together because they are share the same scopes.
 *        This is useful for classes of identifiers that are mutually exclusive
 *        grammatically, such as types and variables in some languages.
 *        
 * Known issues:
 *    Memory management is tricky. With ownsIdents on, Idents are deleted when
 *    passing out of scope, with is often not what you want. With it off,
 *    Idents will leak unless the caller does its own GC. It might be useful
 *    to add a third option, where all idents are deleted only on destruction
 *    of the table.
 *    
 * ToDo:
 *    Implement automatic forward reference resolution, as at parse completion.
 *    Implement a way to iterate over all defined identifiers, not just in the
 *      current scope.
 *    Add printing capability for debugging purposes
 *
 * Invariant: scopes.size() >= 1, scopes[0..scopes.size()] != NULL
 */

template<class TIdent, int TNamespaces = 1, bool ownsIdents = true>
class SymbolTable {
public:
  
    /* This is really intended to be private, but it seems that it needs
     * to be declared this way to allow us to declare iterator below? 
     */
    struct hashString : hash<string> {
        hash<const char *> h;
        size_t operator()(string __s) const { return h(__s.c_str()); }
    };
    
    typedef hash_map<string, TIdent *, hashString> symtab_hash_t;
    typedef symtab_hash_t::iterator iterator;
    
public:
    /* construct a new SymbolTable, with a root scope having the given name
     * (every SymbolTable must have at least 1 scope)
     */
    SymbolTable( const string &name ) { enterScope(name); }
    ~SymbolTable( ) { clear(); delete scopes.front(); }

    /*
     * deletes all identifiers and scopes (other than the root) from the 
     * table, effectively leaving the table in it's initially constructed form.
     */
    void clear( void ) {
        while( scopes.size() > 1 ) {
            delete scopes.front();
            scopes.pop_front();
        }
        scopes.front()->clear();
    }

    /*
     * Add an identifier with the given name into the table at the current
     * scope. Fails if the current scope already has an identifier with the
     * same name.
     * Returns: NULL on failure, otherwise the identifier which was added
     */
    TIdent *add( const string &name, TIdent *id, int ns = 0 ) {
        if( topContains( name, ns ) ) return NULL;
        scopes.front()->add( name, id, ns );
        return id;
    }

    /*
     * Same as above, except that the scope to be added to is given explicitly
     */
    TIdent *add( int scopenum, const string &name, TIdent *id, int ns = 0 ) {
        Scope *sc = getScopeById(scopenum);
        if( sc->lookup( name, ns ) != NULL )
            return NULL;
        sc->add( name, id, ns );
        return id;
    }

    /*
     * Return the identifier corresponding to the given string, or NULL if no
     * such name can be found in the table.
     */
    TIdent *lookup( const string &name, int ns = 0 ) const {
        for( slist<Scope *>::const_iterator it = scopes.begin();
             it != scopes.end(); it++ ) {
            TIdent *elem = (*it)->lookup(name, ns);
            if( elem != NULL )
                return elem;
        }
        return NULL;
    }

    /*
     * Returns true if the given name is in the table
     */
    bool contains( const string &name, int ns = 0 ) const {
        return lookup( name, ns ) != NULL;
    }

    /*
     * Returns true if the given name is declared in the current scope, ie
     * if attempting to add an identifier with the same name would fail.
     */
    bool topContains( const string &name, int ns = 0 ) const {
        return (scopes.front()->lookup( name, ns )) != NULL;
    }

    /*
     * Push a new scope onto the stack. Returns the scope id of the new scope.
     */
    int enterScope( const string &name = string("") ) {
        scopes.push_front( new Scope( name ));
        return scopes.size()-1;
    }
    /*
     * Pop the current scope from the stack. Will not remove the root scope.
     * Returns the scope id of the resulting current scope
     */
    int leaveScope( void ) {
        if( scopes.size() > 1 ) {
            delete scopes.front();
            scopes.pop_front();
        }
        return scopes.size()-1;
    }

    /*
     * Return the current scope id
     */
    int getScope( void ) { return scopes.size() - 1; }
        
    /* Note: Iterates over top scope only */
    iterator begin(int ns = 0) { return scopes.front()->begin(ns); }
    iterator end(int ns = 0) { return scopes.front()->end(ns); }

protected:
    /* Inner class to manage the scopes, and incidentally does the real work
     * of maintaining the hash tables.
     */
    class Scope {
    protected:
        symtab_hash_t hash[TNamespaces];
        string name;
        
    public:
        Scope( const string &scopename ) { name = scopename; }
        ~Scope( ) { if( ownsIdents ) clear(); }

        /* lookup the given name in this scope, returning the associated ident
	 * if found, else NULL
	 */
        TIdent *lookup( const string &name, int ns = 0 ) const {
            if( hash[ns].find(name) == hash[ns].end() ) return NULL;
            else return hash[ns].find(name)->second;
        }

        /* Add a new ident to this scope. Does not check for an existing entry
	 * with the same name.
	 */
        void add( const string &name, TIdent *id, int ns = 0 ) {
            hash[ns][name] = id;
        }

        /* Delete all elements from this scope */
        void clear( void ) {
            if( ownsIdents ) for( int i=0; i<TNamespaces; i++ ) {
                for( iterator it = hash[i].begin();
                     it != hash[i].end(); it ++ ) {
                    if( it->second ) delete it->second;
                }
                hash[i].clear();
            }
        }

        /* Standard iterator functions to iterate over this scope 
	 * (one namespace at a time)
	 */
        iterator begin( int ns = 0 ) { return hash[ns].begin(); }
        iterator end( int ns = 0 ) { return hash[ns].end(); }    
    };

    /* return the scope matching a given scope id (see class notes for
     * details.
     */
    Scope *getScopeById( int n ) {
        slist<Scope *>::iterator it = scopes.begin();
        for( int i = (n < 0 ? -n : scopes.size() - n); i > 0; i-- )
            it++;
        return *it;
    }
            
    /* maintains the list of scopes. The current scope is always at the head */
    slist<Scope *> scopes;
    
};
    
