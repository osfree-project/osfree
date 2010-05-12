/* File: PalmBinaryFile.h
 * Desc: This file contains the definition of the class PalmBinaryFile.
 *
 * Copyright (C) 2000-2001, The University of Queensland, BT group
*/

/* $Revision: 1.9 $
 * 02 Feb 2000 - Mike: Initial revision
 * 01 Aug 01 - Mike: GetGlobalPointerInfo() returns unsigned ints now
*/

#ifndef __PALMBINARYFILE_H__
#define __PALMBINARYFILE_H__

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#ifdef TEST
#include "palmglobal.h"             // Don't want to drag in too much
#else
#include "global.h"
#endif
#include "BinaryFile.h"

class PalmBinaryFile : public BinaryFile
{
public:
                PalmBinaryFile();               // Constructor
  virtual       ~PalmBinaryFile();
  virtual void  UnLoad();                       // Unload the image
  virtual bool  Open(const char* sName);        // Open the file for r/w; pv
  virtual void  Close();                        // Close file opened with Open()
  virtual bool  PostLoad(void* handle);         // For archive files only
  virtual LOAD_FMT GetFormat() const;           // Get format i.e. LOADFMT_PALM

  virtual bool isLibrary() const;
  virtual list<const char *> getDependencyList();
  virtual ADDRESS getImageBase();
  virtual size_t getImageSize();

  // Get a symbol given an address
  char*         SymbolByAddress(const ADDRESS dwAddr);
  // Return true if the address matches the convention for A-line system calls
  bool          IsDynamicLinkedProc(ADDRESS uNative);

  // Specific to BinaryFile objects that implement a "global pointer"
  // Gets a pair of unsigned integers representing the address of %agp
  // (first) and the value for GLOBALOFFSET (second)
  virtual pair<unsigned,unsigned> GetGlobalPointerInfo();

  // Palm specific calls
  
  // Get the ID number for this application. It's possible that the app uses
  // this number internally, so this needs to be used in the final make
  int           GetAppID() const;

  // Generate binary files for non code and data sections
  void          GenerateBinFiles(const string& path) const;

//
//  --  --  --  --  --  --  --  --  --  --  --
//
// Internal information
// Dump headers, etc
//virtual bool    DisplayDetails(const char* fileName, FILE* f = stdout);


    // Analysis functions
    virtual list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main");
    virtual ADDRESS GetMainEntryPoint();
    virtual ADDRESS GetEntryPoint();

//    bool        IsDynamicLinkedProc(ADDRESS wNative);
//    ADDRESS     NativeToHostAddress(ADDRESS uNative);

  protected:
    virtual bool  RealLoad(const char* sName); // Load the file; pure virtual

  private:
    unsigned char* m_pImage;                       // Points to loaded image
    unsigned char* m_pData;                        // Points to data
    // Offset from start of data to where a5 should be initialised to   
    unsigned int   m_SizeBelowA5;
};

#endif      // #ifndef __PALMBINARYFILE_H__
