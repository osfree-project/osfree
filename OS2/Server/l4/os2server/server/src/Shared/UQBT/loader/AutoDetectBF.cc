/* File: AutoDetectBF.cc
 * Desc: This file contains the implementation of the AutoDetectBF class
 *		 It is designed to read the first few bytes of a binary file,
 *		 and based on the type, return a BinaryFile pointer to the
 *		 correct class (e.g. ElfBinaryFile, etc).
 *
 * Copyright (C) 1998, The University of Queensland, BT group
*/

// 27 May 1998 - Mike: Created

#include "global.h"

AutoDetectBF::AutoDetectBF()
{
}

const char sElf[] = "\x7f""ELF";
const char sExe[] = "MZ";

BinaryFile* AutoDetectBF::Load(string sName)
{
	FILE* f;
	char buf[4];

	f = fopen(sName.c_str(), "r");
	if (f == 0) return 0;
	if (fread(buf, 4, 1, f) == 0)		// Read the file
		return 0;
	if (memcmp(buf, sElf, 4) == 0)
		return new ElfBinaryFile;
	if (memcmp(buf, sExe, 2) == 0)
		return new ExeBinaryFile;

	return 0;
}

