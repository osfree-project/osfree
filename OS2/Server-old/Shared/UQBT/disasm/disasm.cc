/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       disasm.cc
 * OVERVIEW:   Driver for a disassembler program. Machine specific
 *              subdirectories turn this into a particular machine disassembler
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

/* $Revision: 1.7 $
 *
 *    Apr 01 - Cristina: Created
 * 07 May 01 - Mike: Added labels; make slightly more general (enough to drive
 *              the hppa as well as sparc disassemblers)
 * 11 May 01 - Cristina: Fixed usage message. 
 */

#include "BinaryFile.h"
#include "global.h"
#include "decoder.h"

// Since we are using the NJMCTK's decoder mode, the following 
// globals need to be defined.
  char _assembly[1024];


int main(int argc, char* argv[])
{
    bool bPrintCtor = false;

    if ((argc > 2) && (strcmp(argv[1], "-c") == 0))
        bPrintCtor = true;
    else if (argc != 2) {
        printf("Usage: %s [-c] <filename>\n", argv[0]);
        printf("       -c prints SLED constructor names after each instruction\n");
		printf ("%s disassembles all code sections of the given executable file\n", argv[0]); 
        return 1;
    }

    BinaryFile *pbf;
    pbf = BinaryFile::Load(argv[argc-1]);

    // Disassemble the CODE section, or at least the same section as main
    NJMCDecoder decoder(pbf);
    ADDRESS uAddr = pbf->GetMainEntryPoint();
    if( !uAddr ) uAddr = pbf->GetEntryPoint();
    if (uAddr) {

        SectionInfo* pSect = pbf->GetSectionInfoByAddr(uAddr);

        assert (pSect);
        ADDRESS hiAddress = pSect->uNativeAddr + pSect->uSectionSize;
        int delta = pSect->uHostAddr - pSect->uNativeAddr;
        int numBytes;

        // Start at the start of the code section
        uAddr = pSect->uNativeAddr;
        while (uAddr < hiAddress) {

            decoder.constrName[0] = '\0';       // Clear string

            // Check for a symbol
            const char* sym = pbf->SymbolByAddress(uAddr);
            if (sym)
                printf("%s:\n", sym);

            numBytes = decoder.decodeAssemblyInstruction(uAddr, delta);
            printf("%s\n", _assembly);

            if (bPrintCtor) {
                // Print the constructor names
                printf("                     %s\n", decoder.constrName);
            }

            uAddr += numBytes;
        }
    }
    printf("\n");

    pbf->UnLoad();
    return 0;

#if 0
    // Find the entry point and disassemble the text section

    NJMCDecoder decoder;
    ADDRESS uAddr = pbf->GetMainEntryPoint();
    if (uAddr) 
	{
		SectionInfo* pSect = pbf->GetSectionInfoByAddr(uAddr);
		assert (pSect);
		ADDRESS hiAddress = pSect->uNativeAddr + pSect->uSectionSize;
		int delta = pSect->uHostAddr - pSect->uNativeAddr; 
		int numBytes;

		// Decode instructions one at a time, assume all the 
		// bytes in the text area are instructions (i.e. decode 
		// instructions sequentially).

		while (uAddr < hiAddress) 
		{
            numBytes = decoder.decodeAssemblyInstruction (uAddr, delta);
            printf("%s\n", _assembly);
            uAddr += numBytes;
		}
	}

    pbf->UnLoad();
    return 0;
#endif
}


// Alternate constructor for the NJMC decoder class, which saves a copy of the
// pointer to the BinaryFile object that loaded the source image
NJMCDecoder::NJMCDecoder(BinaryFile* pbf)
  : pBF(pbf)
{}
