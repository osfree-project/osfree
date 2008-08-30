/*==============================================================================
 * FILE:        changerel.cc
 * OVERVIEW:    This is a stand alone program to change the value of one or
 *              more symbols in a .rel[a].bss section to another value
 *              This is needed when a program being translated has rela.bss
 *              entries; these are symbols that have to be "joined" to like
 *              named symbols in a library file. As they come out of uqbt,
 *              they will be joined, but they won't have the correct value
 *              (i.e. the value that the original program used, and where the
 *              translated program is going to expect it).
 *              This executable will be called automatically from the uqbtss-
 *              generated Makefile as needed
 * COMMAND LINE PARAMETERS:
 *              1: name of binary file to modify, e.g. "fibo"
 *              2: Either "-f" in which case parameter 3 will be the name of
 *                 a file with "symbol" "value" pairs
 *                 or else the name of one symbol to change
 *              3: if parameter 2 is not -f, the value of the symbol
 * NOTE:        This is an experimental version of copyrelbss that attempts to
 *              put the symbols for the rel.bss section into a new symbol
 *              table called .uqbtsym, with strings in .uqbtstr
 *
 * Copyright (C) 1999, The University of Queensland, BT group
 *============================================================================*/

/* $Revision: 1.3 $
 * 07 Dec 99 - Mike: First version
 * 10 Dec 99 - Mike: Need to change not only the .rela.bss entries, but also
 *              the values in the dynamic symbol table.
 *              Else might fix fprintf(stderr), but not ordinary printf()
*/

#include <global.h>
#include <elf.h>
#include "ElfBinaryFile.h"

typedef struct ELEM
{
    string symName;
    unsigned addr;
};

int main(int argc, char* argv[])
{
    if (argc != 4) {
        cerr << "Usage: changerel <binaryfile> <symbol> <value>\n";
        cerr << "or\n";
        cerr << "changerel <binaryfile> -f <filename>\n";
        cerr << "where <filename> has lines with <symbol> <value> pairs\n";
        cerr <<  "e.g. changrel fibo __iob 1234567\n";
        return 1;
    }

    list<ELEM> elements;
    ELEM e;

    if (strcmp(argv[2], "-f") == 0) {
        // Open the file given in argv[3]
        ifstream is;
        is.open(argv[3]);
        if (!is) {
            cerr << "Could not open list file " << argv[3] << " for reading";
            cerr << endl;
            exit(2);
        }
        while (!is.eof()) {
            is >> e.symName >> e.addr;
            if (is.eof()) break;
            is.ignore(0, '\n');            // Should be end of line
            elements.push_back(e);
        }
        is.close();
    }
    else {
        // Just add one element with argv[2] and argv[3]
        e.symName = argv[2];
        e.addr = atoi(argv[3]);
        elements.push_back(e);
    }

    ElfBinaryFile BF;
    // Open the binaryfile for read/write operation
    if (!BF.Open(argv[1])) {
        cerr << "Could not open binary file " << argv[1] << endl;
        return 3;
    }

    bool rela = true;
    SectionInfo* si = BF.GetSectionInfoByName(".rela.bss");
    if (si == NULL) {
        rela = false;
        si = BF.GetSectionInfoByName(".rel.bss");
    }
    if (si == NULL) {
        cerr << "Could not find section .rel[a].bss in binary file ";
        cerr << argv[1] << endl;
        return 4;
    }

    SectionInfo* dSym = BF.GetSectionInfoByName(".dynsym");
    if (dSym == NULL) {
        cerr << "Could not find section .dynsym in binary file ";
        cerr << argv[1] << endl;
        return 5;
    }
    Elf32_Sym* pDsym = (Elf32_Sym*)dSym->uHostAddr;

    int str = BF.GetSectionIndexByName(".dynstr");
    if (str == -1) {
        cerr << "Could not find section .dynstr in binary file ";
        cerr << argv[1] << endl;
        return 5;
    }

#if 0
    // There may or may not be an ordinary string table (depends on whether the
    // source binary file is stripped).
    SectionInfo* sym = BF.GetSectionInfoByName(".symtab");
    Elf32_Sym* pSym = NULL;
    if (pSym != NULL)
        pSym = (Elf32_Sym*)pSym->uHostAddr;
#endif

    int i;
    int numEnt = si->uSectionSize / si->uSectionEntrySize;
    // it iterates through the elements (given in the -f file);
    // i iterates through the entries in the rel[a].bss section
    list<ELEM>::iterator it;
    for (it = elements.begin(); it != elements.end(); it++) {
        unsigned p = si->uHostAddr;
        for (i=0; i < numEnt; i++) {
            // The ugly p[1] below is because it p might point to an Elf32_Rela
            // struct, or an Elf32_Rel struct
            // Get index into dynamic string table
            int dSym = ELF32_R_SYM(((int*)p)[1]);
            int name = pDsym[dSym].st_name;       // Index into string table
            const char* s = BF.GetStrPtr(str, name);
            if (it->symName == s) {
                // Replace the current value in .rel[a].bss
                ((int*)p)[0] = it->addr;
                // Now also in the dynamic string table
                pDsym[dSym].st_value = it->addr;
                // May be necessary to change the .symtab entry as well
                break;
            }
            if (rela)
                p += sizeof(Elf32_Rela);
            else
                p += sizeof(Elf32_Rel);
        }

        if (i >= numEnt) {
            cerr << "Did not find symbol " << it->symName << " in the "
                "rel[a].bss section of " << argv[1] << endl;
            return 6;
        }
    }

    BF.Close();            // Close the binary file


    return 0;
}

