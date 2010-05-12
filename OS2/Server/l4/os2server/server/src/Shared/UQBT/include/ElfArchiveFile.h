/* File: ElfArchiveFile.h
 * Desc: This file contains the definition of the ElfArchiveFile class
 * Derived from BinaryFile class
 *
 * Copyright (C) 1998, The University of Queensland, BT group
*/

/* $Revision: 1.5 $
 * This class provides a machine independent interface for programs
 * that read binary archive files. For details on usage, see the
 * bintrans tex file (bubtrans/tex/bintrans/loader.tex)
 * Created by Mike, 22 May 98
 * Note: at present, it is assumed that elf archive files contain only
 * elf image files (executable or object)
 * 21 Aug 98 - Mike: changes for gcc 2.8.1
*/

#ifndef __ELFARCHIVEFILE_H__
#define __ELFARCHIVEFILE_H__

class ElfArchiveFile : public ArchiveFile {

public:
	// General loader functions
				ElfArchiveFile();			// Default constructor
	virtual		~ElfArchiveFile();			// Destructor
	bool		Load(const char* sName);	// Load the archive file
	void		UnLoad();					// Unload archive file

	// Member access functions.
	// Most return a pointer to a BinaryFile object (or 0 if fail)
	BinaryFile*	GetMember(int i);			// Get indexed member

	// Symbol functions
		// Since an archive file contains a symbol table, it seems
		// reasonable that at some time in the future we will require
		// some more functions to deal with them. For example, some
		// way of iterating through all the symbols in an archive,
		// and/or all the files in an archive member

	// Data
protected:
	Elf*			m_arf;					// Elf handle to the archive
	int				m_filedes;				// File handle for archive
};

#endif		// #ifndef __ELFARCHIVEFILE_H__
