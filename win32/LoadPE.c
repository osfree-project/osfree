
#include <stdio.h>

#include "windows.h"

#include "peexe.h"

extern int LoadPE(char *);
#include "Log.h"
#include "ctype.h"

static int lf_console = LF_CONSOLE;
static int lf_header  = 0;
static int lf_header2 = 0; 
static int lf_imports = 0;
static int lf_exports = LF_CONSOLE;

/*
 * Load a PE file into memory
 * read headers first...
 * get file ImageBase
 * get file SizeOfImage
 *
 * VirtualAlloc(ImageBase,SizeofImage,...)
 * read section headers
 * for each section read section...
 */

#define RVA(b,rva)	((void *)b + rva)

typedef struct {
	char *modulename;
	PIMAGE_NT_HEADERS pNTHeader;
	void *BaseAddress;
} MODULE_HEADERS;

static MODULE_HEADERS NTModules[256];
static char dirname[256] = "/home/robf/bin32";

void DumpHeader(PIMAGE_FILE_HEADER );
void DumpImportsSection(void *, PIMAGE_NT_HEADERS);
void DumpSectionTable(void *,PIMAGE_SECTION_HEADER , unsigned );
void ExecEntryPoint(void *, PIMAGE_NT_HEADERS,char *);
void DumpOptionalHeader(PIMAGE_OPTIONAL_HEADER );
void LoadImportsSection(void * , PIMAGE_NT_HEADERS ,char *);
void LoadExportsTable(MODULE_HEADERS *,PIMAGE_NT_HEADERS ,char *);

static IMAGE_NT_HEADERS NTHeader[256];

static int usebuiltins;

MODULE_HEADERS *
ExecPE(char *lpszName)
{
	static void *BaseAddress;

	IMAGE_DOS_HEADER DosHeader;
 	PIMAGE_SECTION_HEADER pSectionHeaders;
	PIMAGE_NT_HEADERS pNTHeader;
	static int nNTHeader;
	int i,len;
	int index = nNTHeader;
	char *bp;
	int ret;
	HFILE hFile;

	bp = lpszName;
	while(*bp) {
		*bp = tolower(*bp);
		bp++;
	}
	for(i=0;i<nNTHeader;i++) {
		if(strcmp(NTModules[i].modulename,lpszName) == 0) {
			return &NTModules[i];
		}
	}

    	hFile = _lopen(lpszName,READ);
	if(hFile == -1) {
		char lpszFileName[256];

		strcpy(lpszFileName,dirname);	
		strcat(lpszFileName,"/");	
		strcat(lpszFileName,lpszName);	

		hFile = _lopen(lpszFileName,READ);

		if(hFile == -1) {
			logstr(LF_ERROR,"cannot open file %s\n",lpszFileName);
			return 0;
		}		
	}

	/* read the dos image header first */
        ret = _lread(hFile,&DosHeader,sizeof(IMAGE_DOS_HEADER));

	if(DosHeader.e_magic == IMAGE_DOS_SIGNATURE)	{

		/* now read in the nt header */
    		_llseek(hFile,DosHeader.e_lfanew,0);

		pNTHeader = &NTHeader[nNTHeader];

		ret = _lread(hFile,pNTHeader, sizeof(IMAGE_NT_HEADERS));

		/* yes, it is a win32 header */
		if (pNTHeader->Signature != IMAGE_NT_SIGNATURE) {
			_lclose(hFile);
			return 0;
		}
		

		bp = strrchr(lpszName,'/');
		if(bp)
			bp++;
		else 
			bp = lpszName;

		BaseAddress = VirtualAlloc(
			(void *) pNTHeader->OptionalHeader.ImageBase,
			pNTHeader->OptionalHeader.SizeOfImage,
			MEM_COMMIT,
			PAGE_EXECUTE_READWRITE);

		logstr(lf_console,"Load File: %s %p\n",lpszName,BaseAddress);

		NTModules[nNTHeader].modulename = bp; 		
		NTModules[nNTHeader].pNTHeader = pNTHeader; 		
		NTModules[nNTHeader].BaseAddress = BaseAddress; 		
		nNTHeader++;

		if (nNTHeader == 1 && usebuiltins) {
			NTModules[nNTHeader++].modulename = "user32.dll";
			NTModules[nNTHeader++].modulename = "gdi32.dll";
			NTModules[nNTHeader++].modulename = "kernel32.dll";
			NTModules[nNTHeader++].modulename = "shell32.dll";
			NTModules[nNTHeader++].modulename = "comctl32.dll";
			NTModules[nNTHeader++].modulename = "comdlg32.dll";
			NTModules[nNTHeader++].modulename = "rpcrt4.dll";
			NTModules[nNTHeader++].modulename = "advapi32.dll";
		}

		/* show the NT header */
	 	//if (index == 0)
		   DumpHeader(&pNTHeader->FileHeader);

		/* show the Optional header */
	 	//if (index == 0)
		   DumpOptionalHeader((PIMAGE_OPTIONAL_HEADER) 
			&pNTHeader->OptionalHeader);

		pSectionHeaders = (PIMAGE_SECTION_HEADER)((void *)BaseAddress + sizeof(IMAGE_NT_HEADERS));

		/* now read the section headers */
		ret = _lread( hFile, pSectionHeaders, sizeof(IMAGE_SECTION_HEADER)*
			pNTHeader->FileHeader.NumberOfSections);

		for(i=0; i < pNTHeader->FileHeader.NumberOfSections; i++) {
			void *LoadAddress;

			LoadAddress = 
				RVA(BaseAddress,pSectionHeaders->VirtualAddress);
			//if (index == 0) 
			{
			   DumpSectionTable( LoadAddress,pSectionHeaders,i);
			}

			/* load only non-BSS segments */
			if(!(pSectionHeaders->Characteristics &
				IMAGE_SCN_CNT_UNINITIALIZED_DATA)) 
		        {
			    _llseek(hFile,pSectionHeaders->PointerToRawData,SEEK_SET);
			    len = _lread(hFile,(char*) LoadAddress, 
				pSectionHeaders->SizeOfRawData);

			    if( len != pSectionHeaders->SizeOfRawData)
			    {
				logstr(LF_ERROR,"Failed to load section %x %x\n", i,len);
				exit(0);
			    }
			    pSectionHeaders++;
			}
			
			/* not needed, memory is zero */
			if(strcmp(pSectionHeaders[i].Name, ".bss") == 0)
			    memset((void *)LoadAddress, 0,
				   pSectionHeaders[i].Misc.VirtualSize ?
				   pSectionHeaders[i].Misc.VirtualSize :
				   pSectionHeaders[i].SizeOfRawData);
		}

		_lclose(hFile);

		// we are dependent on other modules, go get and load those
		//if (index == 0)
		   LoadImportsSection(BaseAddress, pNTHeader,lpszName);

		if (index == 0) 
		{
			logstr(lf_header,"   %32s   PE Header  BaseAddress\n",
				"FileName");
			for(i=0;i<nNTHeader;i++) {
			   logstr(lf_header,"%.4d: %32s %p %p\n",
				i,
				NTModules[i].modulename,
				NTModules[i].pNTHeader,
				NTModules[i].BaseAddress);
			}	
        		
		}

		if (index == 0)
		   LoadExportsTable(&NTModules[0],pNTHeader,lpszName);

		if (index == 0)
		   ExecEntryPoint(
			NTModules[0].BaseAddress, 
			NTModules[0].pNTHeader, 
			lpszName);

		return &NTModules[index]; 		
	}
	return 0;
}

typedef struct
{
        WORD    flag;
        LPSTR   name;
} WORD_FLAG_DESCRIPTIONS;

typedef struct
{
        DWORD   flag;
        LPSTR   name;
} DWORD_FLAG_DESCRIPTIONS;
#define IMAGE_FILE_DLL                  0x2000

// Bitfield values and names for the IMAGE_FILE_HEADER flags
WORD_FLAG_DESCRIPTIONS ImageFileHeaderCharacteristics[] =
{
{ IMAGE_FILE_RELOCS_STRIPPED, "RELOCS_STRIPPED" },
{ IMAGE_FILE_EXECUTABLE_IMAGE, "EXECUTABLE_IMAGE" },
{ IMAGE_FILE_LINE_NUMS_STRIPPED, "LINE_NUMS_STRIPPED" },
{ IMAGE_FILE_LOCAL_SYMS_STRIPPED, "LOCAL_SYMS_STRIPPED" },
{ IMAGE_FILE_MINIMAL_OBJECT, "MINIMAL_OBJECT" },
{ IMAGE_FILE_UPDATE_OBJECT, "UPDATE_OBJECT" },
{ IMAGE_FILE_BYTES_REVERSED_LO, "BYTES_REVERSED_LO" },
{ IMAGE_FILE_32BIT_MACHINE, "32BIT_MACHINE" },
{ IMAGE_FILE_PATCH, "PATCH" },
{ IMAGE_FILE_SYSTEM, "SYSTEM" },
{ IMAGE_FILE_DLL, "DLL" },
{ IMAGE_FILE_BYTES_REVERSED_HI, "BYTES_REVERSED_HI" },
{ IMAGE_FILE_DEBUG_STRIPPED,"DEBUGGING INFO STRIPPED" }
};

#define NUMBER_IMAGE_HEADER_FLAGS (sizeof(ImageFileHeaderCharacteristics) / sizeof(WORD_FLAG_DESCRIPTIONS))

void DumpHeader(PIMAGE_FILE_HEADER pImageFileHeader)
{
        UINT headerFieldWidth = 30;
        UINT i;
        char *szMachine;

        logstr(lf_header,"File Header\n");

        switch( pImageFileHeader->Machine )
        {
                case IMAGE_FILE_MACHINE_I386:   szMachine = "i386"; break;
                case IMAGE_FILE_MACHINE_I860:   szMachine = "i860"; break;
                case IMAGE_FILE_MACHINE_R3000:  szMachine = "R3000"; break;
                case IMAGE_FILE_MACHINE_R4000:  szMachine = "R4000"; break;
                case IMAGE_FILE_MACHINE_ALPHA:  szMachine = "alpha"; break;
                default:    szMachine = "unknown"; break;
        }

        logstr(lf_header,"  %-*s%04X (%s)\n", headerFieldWidth, "Machine:",
                                pImageFileHeader->Machine, szMachine);
        logstr(lf_header,"  %-*s%04X\n", headerFieldWidth, "Number of Sections:",
                                pImageFileHeader->NumberOfSections);
        logstr(lf_header2,"  %-*s%08X\n", headerFieldWidth, "TimeDateStamp:",
                                pImageFileHeader->TimeDateStamp);
        logstr(lf_header2,"  %-*s%08X\n", headerFieldWidth, "PointerToSymbolTable:",
                                pImageFileHeader->PointerToSymbolTable);
        logstr(lf_header2,"  %-*s%08X\n", headerFieldWidth, "NumberOfSymbols:",
                                pImageFileHeader->NumberOfSymbols);
        logstr(lf_header2,"  %-*s%04X\n", headerFieldWidth, "SizeOfOptionalHeader:",
                                pImageFileHeader->SizeOfOptionalHeader);

        logstr(lf_header,"  %-*s%04X\n", headerFieldWidth, "Characteristics:",
                                pImageFileHeader->Characteristics);

        for ( i=0; i < NUMBER_IMAGE_HEADER_FLAGS; i++ )
        {
                if ( pImageFileHeader->Characteristics &
                         ImageFileHeaderCharacteristics[i].flag )
                        logstr(lf_header,"    %s\n", ImageFileHeaderCharacteristics[i].name );
        }
	logstr(lf_header,"\n");
}

// Bitfield values and names for the DllCharacteritics flags
WORD_FLAG_DESCRIPTIONS DllCharacteristics[] =
{
{ IMAGE_LIBRARY_PROCESS_INIT, "PROCESS_INIT" },
{ IMAGE_LIBRARY_PROCESS_TERM, "PROCESS_TERM" },
{ IMAGE_LIBRARY_THREAD_INIT, "THREAD_INIT" },
{ IMAGE_LIBRARY_THREAD_TERM, "THREAD_TERM" },
};
#define NUMBER_DLL_CHARACTERISTICS (sizeof(DllCharacteristics) / sizeof(WORD_FLAG_DESCRIPTIONS))

// Bitfield values and names for the LoaderFlags flags
DWORD_FLAG_DESCRIPTIONS LoaderFlags[] =
{
{ IMAGE_LOADER_FLAGS_BREAK_ON_LOAD, "BREAK_ON_LOAD" },
{ IMAGE_LOADER_FLAGS_DEBUG_ON_LOAD, "DEBUG_ON_LOAD" }
};
#define NUMBER_LOADER_FLAGS (sizeof(LoaderFlags) / sizeof(DWORD_FLAG_DESCRIPTIONS))

// Names of the data directory elements that are defined
char *ImageDirectoryNames[] = {
        "EXPORT", "IMPORT", "RESOURCE", "EXCEPTION", "SECURITY", "BASERELOC",
        "DEBUG", "COPYRIGHT", "GLOBALPTR", "TLS", "LOAD_CONFIG" };

#define NUMBER_IMAGE_DIRECTORY_ENTRYS (sizeof(ImageDirectoryNames)/sizeof(char *))

//
// Dump the IMAGE_OPTIONAL_HEADER from a PE file
//
void DumpOptionalHeader(PIMAGE_OPTIONAL_HEADER optionalHeader)
{
        UINT width = 30;
        char *s;
        UINT i;

        logstr(lf_header,"Optional Header\n");

        logstr(lf_header2,"  %-*s%04X\n", width, "Magic", 
		optionalHeader->Magic);
        logstr(lf_header2,"  %-*s%u.%02u\n", width, "linker version",
                optionalHeader->MajorLinkerVersion,
                optionalHeader->MinorLinkerVersion);

        logstr(lf_header,"  %-*s%X\n", width, "size of code", optionalHeader->SizeOfCode);
        logstr(lf_header,"  %-*s%X\n", width, "size of initialized data",
                optionalHeader->SizeOfInitializedData);
        logstr(lf_header,"  %-*s%X\n", width, "size of uninitialized data",
                optionalHeader->SizeOfUninitializedData);
        logstr(lf_header,"  %-*s%X\n", width, "entrypoint RVA",
                optionalHeader->AddressOfEntryPoint);
        logstr(lf_header,"  %-*s%X\n", width, "base of code", 
		optionalHeader->BaseOfCode);
        logstr(lf_header,"  %-*s%X\n", width, "base of data", 
		optionalHeader->BaseOfData);
        logstr(lf_header,"  %-*s%X\n", width, "image base", 
		optionalHeader->ImageBase);

        logstr(lf_header2,"  %-*s%X\n", width, "section align",
                optionalHeader->SectionAlignment);
        logstr(lf_header2,"  %-*s%X\n", width, "file align", 
		optionalHeader->FileAlignment);
        logstr(lf_header2,"  %-*s%u.%02u\n", width, "required OS version",
                optionalHeader->MajorOperatingSystemVersion,
                optionalHeader->MinorOperatingSystemVersion);
        logstr(lf_header2,"  %-*s%u.%02u\n", width, "image version",
                optionalHeader->MajorImageVersion,
                optionalHeader->MinorImageVersion);
        logstr(lf_header2,"  %-*s%u.%02u\n", width, "subsystem version",
                optionalHeader->MajorSubsystemVersion,
                optionalHeader->MinorSubsystemVersion);
        logstr(lf_header2,"  %-*s%X\n", width, "Reserved1", 
		optionalHeader->Reserved1);

        logstr(lf_header,"  %-*s%X\n", width, "size of image", 
		optionalHeader->SizeOfImage);
        logstr(lf_header,"  %-*s%X\n", width, "size of headers",
                optionalHeader->SizeOfHeaders);

        switch( optionalHeader->Subsystem )
        {
                case IMAGE_SUBSYSTEM_NATIVE: s = "Native"; break;
                case IMAGE_SUBSYSTEM_WINDOWS_GUI: s = "Windows GUI"; break;
                case IMAGE_SUBSYSTEM_WINDOWS_CUI: s = "Windows character"; break;
                case IMAGE_SUBSYSTEM_OS2_CUI: s = "OS/2 character"; break;
                case IMAGE_SUBSYSTEM_POSIX_CUI: s = "Posix character"; break;
                default: s = "unknown";
        }
        logstr(lf_header,"  %-*s%04X (%s)\n", width, "Subsystem",
                        optionalHeader->Subsystem, s);

        logstr(lf_header,"  %-*s%04X\n", width, "DLL flags",
                        optionalHeader->DllCharacteristics);
        for ( i=0; i < NUMBER_DLL_CHARACTERISTICS; i++ )
        {
                if ( optionalHeader->DllCharacteristics &
                         DllCharacteristics[i].flag )
                        logstr(lf_header,"  %s\n", 
				DllCharacteristics[i].name );
        }
        if ( optionalHeader->DllCharacteristics )
                logstr(lf_header,"\n");

        logstr(lf_header2,"  %-*s%X\n", width, "stack reserve size",
                optionalHeader->SizeOfStackReserve);
        logstr(lf_header2,"  %-*s%X\n", width, "stack commit size",
                optionalHeader->SizeOfStackCommit);
        logstr(lf_header2,"  %-*s%X\n", width, "heap reserve size",
                optionalHeader->SizeOfHeapReserve);
        logstr(lf_header2,"  %-*s%X\n", width, "heap commit size",
                optionalHeader->SizeOfHeapCommit);

        logstr(lf_header2,"  %-*s%08X\n", width, "loader flags",
                optionalHeader->LoaderFlags);

        for ( i=0; i < NUMBER_LOADER_FLAGS; i++ )
        {
                if ( optionalHeader->LoaderFlags &
                         LoaderFlags[i].flag )
                        logstr(lf_header,"  %s", LoaderFlags[i].name );
        }

        if ( optionalHeader->LoaderFlags )
                logstr(lf_header,"\n");

        logstr(lf_header,"  %-*s%X\n", width, "RVAs & sizes",
                optionalHeader->NumberOfRvaAndSizes);

        logstr(lf_header,"Data Directory\n");
        for ( i=0; i < optionalHeader->NumberOfRvaAndSizes; i++)
        {
                logstr(lf_header,"  %-12s rva: %08X  size: %08X\n",
                        (i >= NUMBER_IMAGE_DIRECTORY_ENTRYS)
                                ? "unused" : ImageDirectoryNames[i],
                        optionalHeader->DataDirectory[i].VirtualAddress,
                        optionalHeader->DataDirectory[i].Size);
        }
}


// Bitfield values and names for the IMAGE_SECTION_HEADER flags
DWORD_FLAG_DESCRIPTIONS SectionCharacteristics[] = 
{
{ IMAGE_SCN_CNT_CODE, "CODE" },
{ IMAGE_SCN_CNT_INITIALIZED_DATA, "INITIALIZED_DATA" },
{ IMAGE_SCN_CNT_UNINITIALIZED_DATA, "UNINITIALIZED_DATA" },
{ IMAGE_SCN_LNK_INFO, "LNK_INFO" },
{ IMAGE_SCN_LNK_OVERLAY, "LNK_OVERLAY" },
{ IMAGE_SCN_LNK_REMOVE, "LNK_REMOVE" },
{ IMAGE_SCN_LNK_COMDAT, "LNK_COMDAT" },
{ IMAGE_SCN_MEM_DISCARDABLE, "MEM_DISCARDABLE" },
{ IMAGE_SCN_MEM_NOT_CACHED, "MEM_NOT_CACHED" },
{ IMAGE_SCN_MEM_NOT_PAGED, "MEM_NOT_PAGED" },
{ IMAGE_SCN_MEM_SHARED, "MEM_SHARED" },
{ IMAGE_SCN_MEM_EXECUTE, "MEM_EXECUTE" },
{ IMAGE_SCN_MEM_READ, "MEM_READ" },
{ IMAGE_SCN_MEM_WRITE, "MEM_WRITE" },
};

#define NUMBER_SECTION_CHARACTERISTICS  (sizeof(SectionCharacteristics) / sizeof(DWORD_FLAG_DESCRIPTIONS))

//
// Dump the section table from a PE file or an OBJ
//

void 
DumpSectionTable(void *LoadAddress,PIMAGE_SECTION_HEADER section, unsigned cSections)
{
	unsigned j;

	logstr(lf_header,"Section Table %x %d\n",LoadAddress,cSections);
	
	logstr(lf_header,"  %02X %-8.8s  %s: %08X  VirtAddr:  %08X\n",
			cSections, section->Name, "VirtSize",
			section->Misc.VirtualSize, section->VirtualAddress);
	logstr(lf_header,"    raw data offs:   %08X  raw data size: %08X\n",
			section->PointerToRawData, section->SizeOfRawData );
	logstr(lf_header,"    relocation offs: %08X  relocations:   %08X\n",
			section->PointerToRelocations, section->NumberOfRelocations);
	logstr(lf_header,"    line # offs:     %08X  line #'s:      %08X\n",
			section->PointerToLinenumbers, section->NumberOfLinenumbers );
	logstr(lf_header,"    characteristics: %08X\n", section->Characteristics);

	for ( j=0; j < NUMBER_SECTION_CHARACTERISTICS; j++ )
	{
		if ( section->Characteristics & 
			SectionCharacteristics[j].flag )
			logstr(lf_header,"      %s\n", 
				SectionCharacteristics[j].name );
	}
}

DWORD 
PEGetProcAddressByName( MODULE_HEADERS *mp, PIMAGE_IMPORT_BY_NAME pOrdinalName)
{
	PIMAGE_EXPORT_DIRECTORY exportDir;
	DWORD i;
	PDWORD functions;
	PWORD ordinals;
	PSTR *name;
	LPSTR fname;
        IMAGE_DATA_DIRECTORY    dir;
	DWORD	faddress = 0;

	if (mp->pNTHeader == 0) {
	   return 0xffffffff;
	}

	// get the image_data_directory for exported symbols
	dir = mp->pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

	// convert to pointer
	exportDir = MAKEPTR((PIMAGE_EXPORT_DIRECTORY),
		mp->BaseAddress,
		dir.VirtualAddress);

	// any functions at all?
	if (exportDir->NumberOfFunctions == 0)
	   return 0;

	// create array of function addresses
	functions = MAKEPTR((PDWORD),mp->BaseAddress,
		exportDir->AddressOfFunctions);

	// create array of function ordinals
	ordinals =  MAKEPTR((PWORD),mp->BaseAddress,
		exportDir->AddressOfNameOrdinals);

	// create array of RVA's to function names
	name =      MAKEPTR((PSTR *),mp->BaseAddress,
		exportDir->AddressOfNames);
	
	// before we do linear search through name table
	// use quick lookup first, using the hint... 
	// if we have valid hint, get pointer to underlying name
	if (pOrdinalName->Hint < exportDir->NumberOfNames) {

		fname = MAKEPTR((LPSTR),mp->BaseAddress,name[pOrdinalName->Hint]);
		// have we matched the strings?
		if (strcmp((char *)fname,pOrdinalName->Name) == 0) {
			return MAKEPTR((DWORD),mp->BaseAddress,
				functions[pOrdinalName->Hint]);
		}
			
		// no, so do linear search, bummer its wrong???
		logstr(lf_header,"requested f=%s ord=%x found %s ord=%x\n",
			(char *)pOrdinalName->Name,
			(int)pOrdinalName->Hint,
			MAKEPTR((LPSTR),mp->BaseAddress,name[pOrdinalName->Hint]),
			ordinals[pOrdinalName->Hint]);
	}

	// resort to straight line linear search
	// should/could we ignore comparing strings, until we
	// are in range?
	for ( i=0; i < exportDir->NumberOfNames; i++ )
	{
		// get the pointer to the name
		fname = MAKEPTR((LPSTR),mp->BaseAddress,*name);

		// does this one match what we want...
		if (strcmp((char *)fname,pOrdinalName->Name) == 0)
			faddress = MAKEPTR((DWORD),mp->BaseAddress,*functions);

		// done with search, break out...
		// if(faddress)
		//   break;

		// bump all the pointers
		functions++;
		ordinals++;
		name++;		
	}

	// the name is not in the module...
	return faddress;
}

//
// Dump the imports table (the .idata section) of a PE file
//
void LoadImportsSection(void * BaseAddress, PIMAGE_NT_HEADERS pNTHeader,char *ModuleName)
{
	PIMAGE_IMPORT_DESCRIPTOR importDesc;
	PIMAGE_THUNK_DATA thunk;
	PIMAGE_IMPORT_BY_NAME pOrdinalName;
        IMAGE_DATA_DIRECTORY    dir;
	char *libname;
	MODULE_HEADERS *mp;
	DWORD faddr;
	DWORD *ip;
		
	dir=pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	importDesc = MAKEPTR((PIMAGE_IMPORT_DESCRIPTOR),
		BaseAddress,
		dir.VirtualAddress);

 	logstr(lf_imports,"  DLL Imports Table: %s\n", ModuleName);

	while ( 1 )
	{
		// See if we've reached an empty IMAGE_IMPORT_DESCRIPTOR
		if ( (importDesc->TimeDateStamp==0 ) && (importDesc->Name==0) )
			break;

		libname = (PBYTE)(importDesc->Name) + (int)BaseAddress;

		mp = ExecPE((PBYTE)(importDesc->Name) + (int)BaseAddress);

		logstr(lf_header,"  Library: %s\n",libname); 

		logstr(lf_header2,"  Hint/Name Table: %08X\n", 
			importDesc->u.Characteristics);
 		logstr(lf_header2,"  TimeDateStamp:   %08X\n", 
			importDesc->TimeDateStamp);
 		logstr(lf_header2,"  ForwarderChain:  %08X\n", 
			importDesc->ForwarderChain);
 		logstr(lf_header2,"  First thunk RVA: %08X\n", 
			importDesc->FirstThunk);
	
		thunk = MAKEPTR((PIMAGE_THUNK_DATA),BaseAddress,importDesc->u.Characteristics);
		ip    = MAKEPTR((DWORD *),BaseAddress,importDesc->FirstThunk);

		while (1) {
			if (thunk->u1.AddressOfData == 0)
			   break;

			pOrdinalName = thunk->u1.AddressOfData;

			thunk++;	

			if((int) pOrdinalName & 0x80000000) {
				logstr(lf_imports,"    import %4.4p\n", 
					pOrdinalName);
			} else 
			{

			     pOrdinalName = (PIMAGE_IMPORT_BY_NAME) 
				     ((PBYTE) pOrdinalName + (int)BaseAddress);

			     faddr = PEGetProcAddressByName(mp,pOrdinalName);

			     logstr(lf_imports,
				"    import %-32s %4.4x %8.8x\n",
				pOrdinalName->Name, 
				pOrdinalName->Hint,
				faddr);

			     *ip = faddr;
			}
			ip++;
		}

		importDesc++;	// advance to next IMAGE_IMPORT_DESCRIPTOR
	}
}

void 
LoadExportsTable(MODULE_HEADERS *mp,PIMAGE_NT_HEADERS pNTHeader,char *ModuleName)
{
	PIMAGE_EXPORT_DIRECTORY exportDir;
	DWORD i;
	PDWORD functions;
	PWORD ordinals;
	PSTR *name;
	LPSTR fname;
        IMAGE_DATA_DIRECTORY    dir;

	logstr(lf_exports,"Exports of %s\n",ModuleName);

	// get the image_data_directory for exported symbols
	dir = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

	// convert to pointer
	exportDir = MAKEPTR((PIMAGE_EXPORT_DIRECTORY),
		mp->BaseAddress,
		dir.VirtualAddress);

	// any functions at all?
	if (exportDir->NumberOfFunctions == 0)
	   return ;

	// create array of function addresses
	functions = MAKEPTR((PDWORD),mp->BaseAddress,
		exportDir->AddressOfFunctions);

	// create array of function ordinals
	ordinals =  MAKEPTR((PWORD),mp->BaseAddress,
		exportDir->AddressOfNameOrdinals);

	// create array of RVA's to function names
	name =      MAKEPTR((PSTR *),mp->BaseAddress,
		exportDir->AddressOfNames);
	
	// resort to straight line linear search
	// should/could we ignore comparing strings, until we
	// are in range?
	for ( i=0; i < exportDir->NumberOfNames; i++ )
	{
		// get the pointer to the name
		fname = MAKEPTR((LPSTR),mp->BaseAddress,*name);

	        logstr(lf_exports,"export %x %s\n",
			*ordinals,fname);

		// bump all the pointers
		functions++;	// function address
		ordinals++;	// function ordinal
		name++;		// function name
	}

	// the name is not in the module...
	return ;
}

void
ExecEntryPoint(void *BaseAddress, PIMAGE_NT_HEADERS pNTHeader,char *ModuleName)
{
	int *fp;
	fp = MAKEPTR((int *), BaseAddress, 
			pNTHeader->OptionalHeader.AddressOfEntryPoint);
	logstr(lf_header,"execute: %s %x %x\n",
		ModuleName,pNTHeader->OptionalHeader.AddressOfEntryPoint, fp);
}

int
WEP()
{
return 0;
}

int
IsPEFormat()
{
return 0;
}

int
GetPEExeInfo()
{
return 0;
}

int
GetW32SysVersion()
{
return 0;
}

int
LoadPEResource()
{
return 0;
}

int
GetPEResourceTable()
{
return 0;
}

int
ExecPEEx()
{
return 0;
}

int
W32SError()
{
return 0;
}

int
ItsMe()
{
	return 0;
}

int
SegmentTableW32SYS()
{
	return 0;
}
