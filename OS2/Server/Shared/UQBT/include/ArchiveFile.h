/* File: ArchiveFile.h
 * Desc: This file contains the definition of the ArchiveFile class
 *
 * Copyright (C) 1998, The University of Queensland, BT group
*/

/* $Revision: 1.5 $
 * This class provides a machine independent interface for programs
 * that read binary archive files. For details on usage, see the
 * bintrans tex file (bubtrans/tex/bintrans/loader.tex)
 * Note: at present, only elf archives are supported (though they
 * may conatin other supported members)
 * Created by Mike, 22 May 98
 * 21 Aug 98 - Mike: changes for gcc 2.8.1
*/

#ifndef __ARCHIVEFILE_H__
#define __ARCHIVEFILE_H__

class ArchiveFile {

public:
	// General loader functions
				ArchiveFile();					// Constructor
	virtual		~ArchiveFile();					// Destructor
	bool		Load(const char* sName);		// Load the archive file
	void		UnLoad();						// Unload archive file
	int			GetNumMembers() const;			// Get number of members

	// Member access functions.
	// Most return a pointer to a BinaryFile object (or 0 if fail)
virtual BinaryFile*	GetMember(int i) = 0;		// Get indexed member
	BinaryFile*		GetMemberByProcName(const string& pSym);// Get member by symbol
	BinaryFile*		GetMemberByFileName(const string& pName);// Get member by file name

	// Information functions
	const char*		GetMemberFileName(int i) const;	// Get name of indexed member

	// Symbol functions
		// Since an archive file contains a symbol table, it seems
		// reasonable that at some time in the future we will require
		// some more functions to deal with them. For example, some
		// way of iterating through all the symbols in an archive,
		// and/or all the files in an archive member

protected:
	// A function to call BinaryFile::PostLoad(void*);
	bool			PostLoadMember(BinaryFile* pBF, void* handle);

	// Data
	bool			m_bArchive;				// True if archive member
	vector<BinaryFile*>	m_Members;			// Member pointers
	StrIntMap		m_SymMap;				// Map from symbol to index
	StrIntMap		m_FileMap;				// Map from filename to index
	vector<char*>	m_FileNames;			// vector of archive headers
	vector<int>		m_Offsets;				// vector of offsets
};

#endif		// #ifndef __ARCHIVEFILE_H__
