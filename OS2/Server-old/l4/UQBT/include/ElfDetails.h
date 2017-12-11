/* File: Details.h
 * Desc: This file contains definitions for various verbose functions
 *
 * Copyright (C) 1998, The University of Queensland, BT group
*/

/* $Revision: 1.6 $
 * 3 Feb 98 - Cristina
 *	added RTL dictionary argument to dumpShdr().
 * 2 Jun 98 - Mike
 *	Loader->BinaryFile; renamed from Verbose.cc; integrated into BinaryFile
*/

#ifndef _DETAILS_H_
#define _DETAILS_H_

void dumpElf32 (Elf32_Ehdr *ehdr, const char *fileName, FILE* f);
void dumpPhdr (Elf32_Phdr *phdr, FILE* f);

void dumpPhdr (Elf32_Phdr *phdr);
void dumpShdr (Elf32_Shdr *shdr, int idxElf, FILE* f);

#endif
