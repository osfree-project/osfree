/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: BinaryFile.cc
 * Desc: This file contains the implementation of the class BinaryFile
 * 
 * This file implements the abstract BinaryFile class.
 * All classes derived from this class must implement the Load()
 * function.
 *
 * Copyright (C) 1997-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
*/

/*
 *	MVE 30/9/97
 *	MVE 11/Dec/97 Major mods, e.g. removing pseudo-headers
 *  6 Feb 01 - Nathan: added getInstanceFor()
 *  3 Mar 98 - Cristina
 *	changed ADDR for ADDRESS for consistency with other tools.
 * 11 Mar 98 - Cristina  
 *  replaced BOOL for bool type (C++'s), same for TRUE and FALSE.
 * 29 Apr 99 - Mike: Added GetSectionInfoByAddr()
 * 05 Aug 99 - Mike: GetSectionInfoByAddr() returns null if not found (thanks
 *				for the suggestion, Ian!)
 * 04 Feb 00 - Mike: Fixed GetSectionInfoByAddr to start at section 0
 * 16 Feb 01 - Nathan: Added static load, merged magic back in
 * 16 Mar 01 - Mike: MZ file with nothing special at 0x3C is treated as an
 *              ExeBinaryFile now
 * 31 Mar 01 - Mike: Use NO_ADDRESS instead of -1
 * 31 Jul 01 - Mike: Fixed minor fencepost bug in GetSectionInfoByAddr()
 * 10 Aug 01 - Mike: Added GetDynamicGlobalMap()
*/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "BinaryFile.h"
#include "ElfBinaryFile.h"
#include "Win32BinaryFile.h"
#include "PalmBinaryFile.h"
#include "HpSomBinaryFile.h"
#include "ExeBinaryFile.h"
#include <stdio.h>


BinaryFile::BinaryFile(bool bArch /*= false*/)
{
	m_bArchive = bArch;			// Remember whether an archive member
	m_iNumSections = 0;			// No sections yet
	m_pSections = 0;			// No section data yet
}

BinaryFile *BinaryFile::Load( const char *sName )
{
    BinaryFile *pBF = BinaryFile::getInstanceFor( sName );
    if( pBF == NULL ) return NULL;
    if( pBF->RealLoad( sName ) == 0 ) {
        fprintf( stderr, "Loading '%s' failed\n", sName );
        delete pBF;
        return NULL;
    }
    return pBF;
}

#define TESTMAGIC2(buf,off,a,b)     (buf[off] == a && buf[off+1] == b)
#define TESTMAGIC4(buf,off,a,b,c,d) (buf[off] == a && buf[off+1] == b && \
                                     buf[off+2] == c && buf[off+3] == d)

BinaryFile *BinaryFile::getInstanceFor( const char *sName )
{
    FILE *f;
    char buf[64];
    BinaryFile *res = NULL;

    f = fopen (sName, "ro");
    if( f == NULL ) {
        fprintf(stderr, "Unable to open binary file: %s\n", sName );
        return NULL;
    }
    fread (buf, sizeof(buf), 1, f);
    if( TESTMAGIC4(buf,0, '\177','E','L','F') ) {
        /* ELF Binary */
        res = new ElfBinaryFile();
    } else if( TESTMAGIC2( buf,0, 'M','Z' ) ) { /* DOS-based file */
        int peoff = LMMH(buf[0x3C]);
        if( peoff != 0 && fseek(f, peoff, SEEK_SET) != -1 ) {
            fread( buf, 4, 1, f );
            if( TESTMAGIC4( buf,0, 'P','E',0,0 ) ) {
                /* Win32 Binary */
                res = new Win32BinaryFile();
            } else if( TESTMAGIC2( buf,0, 'N','E' ) ) {
                /* Win16 / Old OS/2 Binary */
            } else if( TESTMAGIC2( buf,0, 'L','E' ) ) {
                /* Win32 VxD (Linear Executable) */
            } else if( TESTMAGIC2( buf,0, 'L','X' ) ) {
                /* New OS/2 Binary */
            }
        }
        /* MS-DOS Real-mode binary. */
        if( !res )
            res = new ExeBinaryFile();
    } else if( TESTMAGIC4( buf,0x3C, 'a','p','p','l' ) ||
               TESTMAGIC4( buf,0x3C, 'p','a','n','l' ) ) {
        /* PRC Palm-pilot binary */
        res = new PalmBinaryFile();
    } else if( buf[0] == 0x02 && buf[2] == 0x01 &&
               (buf[1] == 0x10 || buf[1] == 0x0B) &&
               (buf[3] == 0x07 || buf[3] == 0x08 || buf[4] == 0x0B) ) {
        /* HP Som binary (last as it's not really particularly good magic) */
        res = new HpSomBinaryFile();
    } else {
        fprintf( stderr, "Unrecognized binary file\n" );
    }
    
    fclose(f);
    return res;
}

int BinaryFile::GetNumSections() const
{
	return m_iNumSections;
}

PSectionInfo BinaryFile::GetSectionInfo(int idx) const
{
	return m_pSections + idx;
}

int	BinaryFile::GetSectionIndexByName(const char* sName)
{
	for (int i=0; i < m_iNumSections; i++)
	{
		if (strcmp(m_pSections[i].pSectionName, sName) == 0)
		{
			return i;
		}
	}
	return -1;
}

PSectionInfo BinaryFile::GetSectionInfoByAddr(ADDRESS uEntry) const
{
	PSectionInfo pSect;
	for (int i=0; i < m_iNumSections; i++)
	{
		pSect = &m_pSections[i];
		if ((uEntry >= pSect->uNativeAddr) &&
			(uEntry < pSect->uNativeAddr + pSect->uSectionSize))
		{
			// We have the right section
			return pSect;
		}
	}
	// Failed to find the address
	return NULL;
}

PSectionInfo BinaryFile::GetSectionInfoByName(const char* sName)
{
	int i = GetSectionIndexByName(sName);
	if (i == -1) return 0;
	return &m_pSections[i];
}


	///////////////////////
	// Trivial functions //
	// Overridden if reqd//
	///////////////////////

const char* BinaryFile::SymbolByAddress(ADDRESS uNative)
{
	return 0;		// Overridden by subclasses that support syms
}

ADDRESS BinaryFile::GetAddressByName(const char* pName, bool bNoTypeOK)
{
	return 0;
}

int	BinaryFile::GetSizeByName(const char* pName, bool bNoTypeOK)
{
	return 0;
}


bool BinaryFile::IsAddressRelocatable(ADDRESS uNative)
{
	return false;
}

ADDRESS BinaryFile::GetRelocatedAddress(ADDRESS uNative)
{
	return NO_ADDRESS;
}

#if 0
WORD  BinaryFile::ApplyRelocation(ADDRESS uNative, WORD wWord)
{
	return 0;
}
#endif
				// Get symbol associated with relocation at address, if any
const char* BinaryFile::GetRelocSym(ADDRESS uNative)
{
	return 0;
}

bool BinaryFile::IsDynamicLinkedProc(ADDRESS uNative)
{
	return false;
}

list<RegAddr> Empty(0);
list<RegAddr>& BinaryFile::GetInitialState()
{
	// Trivial function to return an empty list
	// Note: don't return a list<RegAddr>(0) since this is generated
	// on the stack!
	return Empty;
}

bool BinaryFile::DisplayDetails(const char* fileName, FILE* f /* = stdout */)
{
	return false;			// Should always be overridden
							// Should display file header, program 
							// headers and section headers, as well 
							// as contents of each of the sections. 
}

// Specific to BinaryFile objects that implement a "global pointer"
// Gets a pair of unsigned integers representing the address of %agp, and
// a machine specific value (GLOBALOFFSET)
// This is a stub routine that should be overridden if required
pair<unsigned,unsigned> BinaryFile::GetGlobalPointerInfo()
{
    return pair<unsigned, unsigned>(0, 0);
}

// Get a pointer to a new map of dynamic global data items.
// If the derived class doesn't implement this function, return an empty map
// Caller should delete the returned map
map<ADDRESS, const char*>* BinaryFile::GetDynamicGlobalMap()
{
    return new map<ADDRESS, const char*>;
}

// Get an array of exported function stub addresses. Normally overridden.
ADDRESS* BinaryFile::GetImportStubs(int& numExports)
{
    numExports = 0;
    return NULL;
}
