/*----------------------------------------------------------------------------- 
 *
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------- 
 * $Author: $ 
 * $Date: $
 * $Revision: $
 *----------------------------------------------------------------------------*/


#include	<stdio.h>
#include	<errno.h>
#include	<malloc.h>
#include	<fcntl.h>
#include	<string.h>


#include	"whead.h"
#include	"apilist.h"

/* If the path is given, sprintf fails becaus the length of the cmdline string
 * becomes too long for it to handle.  Any better ways(other than strcat())??
 */
#define	GREP	"grep"
#define	SORT	"sort"
#define	AWK		"awk"

extern	int errno;
extern	char *sys_errlist[];

/* Global variables */
char *entry_tbl;
char *header_image;
char *nehdr;
DWORD ne_offset = 0;
int options = 0;
/* A kluge for not printing things that would upset operations which rely on
 * the regularity of the output, for instance in dump_imports()
 */
int	print_level = -1;

/* Error strings */
static char *wh_errlist[] = {
	"No MZ signature in file",
	"No NE signature in file",
	"Not a windows exe",
	"Error during file read",
	"Not enough memory",
	"Invalid relocation source type",
	"Invalid relocation target",
	"No N3 signature",
	"Error during file write",
	"Not a self loading application",
	""
};

/* defines for options */
#define	OPT_HEADER			0x1
#define	OPT_SEGMENTS		0x2
#define	OPT_RELOCATIONS		0x4
#define	OPT_RESIDENTTBL		0x8
#define	OPT_MODULEREFTBL	0x10
#define	OPT_ENTRYTBL		0x20
#define	OPT_NONRESIDENTTBL	0x40
#define	OPT_RESOURCETBL		0x80
#define	OPT_IMPORTTBL		0x100
#define	OPT_NORELOCCHAIN	0x200
#define	OPT_UNMUNGE			0x400
#define	OPT_IMPORTS			0x800
#define	OPT_PRELOADSEGS		0x1000
#define	OPT_PARADOX			0x2000
#define	OPT_SILENT			0x4000
#define	OPT_PRINTSTRUCT		0x8000
#define	OPT_OVERRIDEWINEXE	0x10000
#define	OPT_ALL		OPT_HEADER | OPT_SEGMENTS | OPT_RELOCATIONS | \
					OPT_RESIDENTTBL | OPT_MODULEREFTBL | OPT_ENTRYTBL | \
					OPT_NONRESIDENTTBL | OPT_RESOURCETBL | OPT_IMPORTTBL | \
					OPT_NORELOCCHAIN 

/* and their corresponding switches */
#define	SW_ALL				'a'
#define	SW_HEADER			'b'
#define	SW_SEGMENTS			'c'
#define	SW_RELOCATIONS		'd'
#define	SW_RESIDENTTBL		'e'
#define	SW_MODULEREFTBL		'f'
#define	SW_ENTRYTBL			'g'
#define	SW_NONRESIDENTTBL	'h'
#define	SW_RESOURCETBL		'i'
#define	SW_IMPORTTBL		'j'
#define	SW_NORELOCCHAIN		'k'
#define	SW_IMPORTS			'l'
#define	SW_OVERRIDEWINEXE	'o'
#define	SW_PARADOX			'p'
#define	SW_SILENT			's'
#define	SW_UNMUNGE			'u'
#define	SW_OPT_PRELOADSEGS	'y'
#define	SW_PRINTSTRUCT		'z'

/* defines for print_level */
#define	PRINT_TITLE			0x1	/* Print the pogram name, version etc. */
#define	PRINT_RELOC_MSG		0x2	/* Print nice things along with reloc info */

/*--------------------------------------------------------------------------*/

/* Takes a byte stream and returns a WORD in Little Endian convention */
WORD
get_word(str)
BYTE *str;
{
	WORD ret_val = ((*(str + 1) * 0x100) + (*str));
	return ret_val;
}

/* Takes a byte stream and returns a DWORD in Little Endian convention */
DWORD
get_dword(str)
BYTE *str;
{
	DWORD ret_val = ((get_word(str + 2) * 0x10000) + (get_word(str)));
	return ret_val;
}

/* Places the WORD in character stream in Little Endian order */
void
put_word(str, w)
BYTE *str;
WORD w;
{
	*str = w % 0x100;
	*(str + 1) = w / 0x100;
}

/* Places the DWORD in character stream in Little Endian order */
void
put_dword(str, dw)
BYTE *str;
DWORD dw;
{
	put_word(str, dw % 0x10000);
	put_word(str + 2, dw / 0x10000);
}

/* Converts string to upper case */
char *
str_toupper(lstring)
char	*lstring;
{
	register char	*str = lstring;

	/* A kludge for not converting strings that do not have the second
	 * character < 'Z'. Just a crude way to avoid going through the entire
	 * string when MOST strings are expected to be upper case anyway
	 */
	if (lstring[1] < 'Z')
		return(lstring);

	while (*str != 0) {
		if (isalpha(*str) && islower(*str))
			*str = toupper(*str);
		++str;
	}
	return(lstring);
}

/* Print error string from error number */
void
wh_perror(errnum)
int errnum;
{
	fprintf(stderr, "%s\n\n", wh_errlist[errnum - 1]);
}

/* Print error string from error number and exit*/
void
wh_panic(errnum)
int errnum;
{
	fprintf(stderr, "panic: ");
	wh_perror(errnum);
	exit(-errnum);
}

/* Same as fread but prints message and exits on error */
void
wh_fread(ptr, size, nitems, stream)
char *ptr;
int size, nitems;
FILE *stream;
{
	/* rudimentary error checking */
	if (fread(ptr, size, nitems, stream) == 0) {
		fprintf(stderr, "wh_fread: Error %d\n", errno);
		wh_panic(WHERR_FILEREAD);
	}
}

/* Same as fwrite but prints message and exits on error */
void
wh_fwrite(ptr, size, nitems, stream)
char *ptr;
int size, nitems;
FILE *stream;
{
	/* rudimentary error checking */
	if (fwrite(ptr, size, nitems, stream) == 0) {
		fprintf(stderr, "wh_fwrite: Error %d\n", errno);
		wh_panic(WHERR_FILEWRITE);
	}
}

/* converts a pascal string to a null terminated string. Max of 256 characters */
char *
cnvrt_pascalstr(pstr)
BYTE *pstr;
{
	static char cstr[256];
	int cnt = *pstr;
	
	if (cnt == 0)
		return(NULL);
	memcpy(cstr, (char *)(pstr + 1), cnt);
	cstr[cnt] = 0;
	return(cstr);
}

/* Given a module name and ordinal number, would return modulename.name if
 * available otherwise modulename.number
 */
char *
cnvrt_ordinal(module, ordinal)
char *module;
int ordinal;
{
	register int j;
	register int i;
	static char buffer[128];

	for (i = 0; api_list[i] != 0; i++) {
		if (strcmp(api_list[i][0].name, module))
			continue;
		for (j = 0; api_list[i][j].ordinal != -1; j++)
			if (api_list[i][j].ordinal == ordinal) {
				sprintf(buffer, "%s.%s", module, api_list[i][j].name);
				return (buffer);
			}
	}
	sprintf(buffer, "%s.%d", module, ordinal);
	return (buffer);
}

/* Convert module.name to module.ordinal if information on module is available, else
 * return module.name
 */
char *
cnvrt_module_name(module, name)
char *module;
char *name;
{
	register int j;
	register int i;
	static char buffer[128];

	for (i = 0; api_list[i] != 0; i++) {
		if (strcmp(api_list[i][0].name, module))
			continue;
		for (j = 0; api_list[i][j].ordinal != -1; j++)
			if (strcmp(api_list[i][j].name, name) == 0) {
				sprintf(buffer, "%s.%d", module, api_list[i][j].ordinal);
				return (buffer);
			}
	}
	sprintf(buffer, "%s.%s", module, name);
	return (buffer);
}

/* Dump the contents of the main ne header */
void
dump_header()
{
	int i;

	fprintf(stdout, "Dump of the NE header:\n\n");
	fprintf(stdout, "Linker version number\t\t\t%d.%02d\n",
			nehdr[0x2], nehdr[0x3]);
	fprintf(stdout, "File CRC\t\t\t\t%lx\n", GDWN(0x8));

	i = GWN(HEADER_FLAG);
	fprintf(stdout, "Program flags:\n");
	if (i == HEADER_NOAUTODATA)
		fprintf(stdout, "\tNo Automatic data segment");
	if (i & HEADER_SINGLEDATA)
		fprintf(stdout, "\tSINGLE_DATA");
	if (i & HEADER_MULTIPLEDATA)
		fprintf(stdout, "\tMULTIPLE_DATA");
	if (i & HEADER_REALMODE)
		fprintf(stdout, "\tREAL_MODE_APP");
	if (i & HEADER_PROTMODE)
		fprintf(stdout, "\tPROTECTED_MODE_APP");
	if (i & HEADER_INVALID)
		fprintf(stdout, "\tNON_CONFORMING_APP");
	if (i & HEADER_LIBRARY)
		fprintf(stdout, "\tDLL_MODULE");
	fprintf(stdout, "\n\n");
	i = GWN(MORE_FLAGS);
	if (i & WIN2_WIN3_PROT)
		fprintf(stdout, "\tWin 2.0 app that can run in win 3.0\n");
	if (i & WIN2_PFONTS)
		fprintf(stdout, "\tWin 2.0 app supports proportional fonts\n");
	if (i & FAST_LOAD) {
		fprintf(stdout, "Program has fast load area:\n");
		fprintf(stdout, "\tStart of fast load area : %04x\n",
		        GWN(FAST_LOAD_AREA));
		fprintf(stdout, "\tSize of fast load area  : %04x\n", GWN(FAST_LOAD_SZ));
	}

	fprintf(stdout, "Automatic data segment index\t\t%04x\n",
			GWN(AUTO_DATASEG));
	fprintf(stdout, "Initial heap size\t\t\t%04x\n", GWN(0x10));
	fprintf(stdout, "Initial stack size\t\t\t%04x\n", GWN(0x12));
	fprintf(stdout, "Initial CS:IP\t\t\t\t%04x:%04x\n", GWN(0x16), GWN(0x14));
	fprintf(stdout, "Initial SS:SP\t\t\t\t%04x:%04x\n", GWN(0x1a), GWN(0x18));
	fprintf(stdout, "Entries in segment table\t\t%04x\n", GWN(SEGMENTTBL_SZ));
	fprintf(stdout, "Entries in module reference table\t%04x\n",
			GWN(MODULEREFTBL_SZ));
	fprintf(stdout, "Entry table size\t\t\t%04x\n", GWN(ENTRYTBL_SZ));
	fprintf(stdout, "Non resident table size\t\t\t%04x\n",
			GWN(NONRESIDENTTBL_SZ));
	fprintf(stdout, "\nEntry table offset\t\t:%04x\n",
			GWN(ENTRY_TBL) );
	fprintf(stdout, "Segment table offset\t\t:%04x\n",
			GWN(SEGMENT_TBL) );
	fprintf(stdout, "Resource table offset\t\t:%04x\n",
			GWN(RESOURCE_TBL) );
	fprintf(stdout, "Resident name table offset\t:%04x\n",
			GWN(RESIDENT_TBL) );
	fprintf(stdout, "Module reference table offset\t:%04x\n",
			GWN(MODULEREF_TBL) );
	fprintf(stdout, "Imported names table offset\t:%04x\n",
			GWN(IMPORTED_TBL) );
	fprintf(stdout, "Non-resident name table offset\t:%06lx\n",
			GDWN(NONRESIDENT_TBL));
	fprintf(stdout, "\nMovable entry point count\t\t%04x\n",
			GWN(MOVABLE_ENTRYPT));
	fprintf(stdout, "Shift count of logical sector\t\t%04x\n", GWN(SHIFT_CNT));
	fprintf(stdout, "Target Operating System\t\t\t");
	i = GWN(TARGET_OS);
	if (i & OS2_MASK)
		fprintf(stdout, "OS/2\n");
	else if (i & WIN_MASK) {
		i = GWN(WIN_VER);
		fprintf(stdout, "Windows\n");
		fprintf(stdout, "Windows version required\t\t %d.%d\n", i/0x100, i%0x100);
	}
}

/* Dump segment information from the segment table */
void
dump_segments()
{
	BYTE *tmpbptr;
	int seg_cnt = GWN(SEGMENTTBL_SZ);
	long code_size = 0;
	long data_size = 0;
	int i;
	WORD tmpw, tmpw1;
	DWORD tmpdw;

	tmpbptr = (BYTE *)(header_image + GWN(SEGMENT_TBL) );
	fprintf(stdout, "\nSegment table at %04x\n\n", GWN(SEGMENT_TBL) );
	for (i = 1; i <= seg_cnt; i++) {
		fprintf(stdout, "\n\n  Segment number: %04x\n", i);

		tmpw1 = get_word(tmpbptr + SEGENTRY_START);
		tmpdw = tmpw1 << GWN(SHIFT_CNT);
		tmpw = get_word(tmpbptr + SEGENTRY_LENGTH);
		if (tmpw == 0)
			tmpw = (WORD) (64 * 1024);
		fprintf(stdout, "\tSector offset: %04x, File length:  %04x, ", tmpw1, tmpw);
		tmpw = get_word(tmpbptr + SEGENTRY_FLAG);
		fprintf(stdout, "Flag:  %04x\n", tmpw);
		tmpw1 = get_word(tmpbptr + SEGENTRY_ALLOCSZ);
		fprintf(stdout, "\tFile Offset: %06x, Alloc length: %04x\n", tmpdw, tmpw1);
		fprintf(stdout, "\tSome attributes: ");

		switch (tmpw & SEG_TYPEMASK) {
			case	SEG_CODE:
					code_size += get_word(tmpbptr + SEGENTRY_LENGTH);
					fprintf(stdout, "CODE");
					break;
			case	SEG_DATA:
					data_size += get_word(tmpbptr + SEGENTRY_LENGTH);
					fprintf(stdout, "DATA");
					break;
		}
		if (tmpw & SEG_MOVABLE)
			fprintf(stdout, " Movable");
		if (tmpw & SEG_PURE)
			fprintf(stdout, " Shareable");
		if (tmpw & SEG_PRELOAD)
			fprintf(stdout, " Preloaded");
		if (tmpw & SEG_ERONLY)
			fprintf(stdout, " Read-only");
		if (tmpw & SEG_RELOCINFO)
			fprintf(stdout, " Relocations");
		if (tmpw & SEG_DISCARD)
			fprintf(stdout, " Discardable");
		tmpbptr += SEGMENT_ENTRYSZ;
	}
	fprintf(stdout, "\n\nBytes in code segments: %8ld (0x%lx)\n",
			code_size, code_size);
	fprintf(stdout, "Bytes in data segments: %8ld (0x%lx)\n\n",
			data_size, data_size);
}

/* Dump the resident name table */
void
dump_residenttable()
{
	WORD offset = GWN(RESIDENT_TBL) ;
	BYTE *tmpbptr = (BYTE *)(header_image + offset);
	char *name;

	fprintf(stdout, "\n\nResident name table at %04x\n", offset);
	fprintf(stdout, "  Module name:");
	while (*tmpbptr != 0) {
		name = cnvrt_pascalstr(tmpbptr);
		fprintf(stdout, "\t\"%s\"\n", name);
		tmpbptr = tmpbptr + strlen(name) + 1;
		tmpbptr += 2;	/* sizeof(WORD) */
	}
}

/* Dump the module reference table */
void
dump_modulereftable()
{
	WORD offset = GWN(MODULEREF_TBL) ;
	BYTE *tmpbptr = (BYTE *)(header_image + offset);
	BYTE *tmpbptr1 = (BYTE *)(header_image + GWN(IMPORTED_TBL) );
	WORD i, j;

	fprintf(stdout, "\n\nModule refernce table at %04x\n", offset);
	for (i = 1; tmpbptr < tmpbptr1 ; i++, tmpbptr += 2) {
		j = get_word(tmpbptr);
		if (j == 0)
			break;
		fprintf(stdout, "  Module %x:\t%s\n", i, cnvrt_pascalstr(tmpbptr1 + j));
	}
}

/* Dump the impotred names table. Is there anyhing different from what we
 * dump in module reference tabele
 */
void
dump_importtable()
{
	WORD offset = GWN(IMPORTED_TBL) ;
	BYTE *tmpbptr1 = (BYTE *)(header_image + offset);
	BYTE *tmpbptr = (BYTE *)(header_image + GWN(MODULEREF_TBL) );
	WORD i, j;

	fprintf(stdout, "\n\nImported names table at %04x\n", offset);
	fprintf(stdout, "\tname\t\t\t\toffset\n");
	for (i = 0; tmpbptr < tmpbptr1 ; i++, tmpbptr += 2) {
		j = get_word(tmpbptr);
		if (j == 0)
			break;
		fprintf(stdout, "\t%s\t\t\t\t%04x\n", cnvrt_pascalstr(tmpbptr1 + j), j);
	}
}

/* Dump the non-resident name table */
void
dump_nonresidenttable()
{
	DWORD offset = GDWN(NONRESIDENT_TBL) - ne_offset;
	BYTE *tmpbptr = (BYTE *)(header_image + offset);
	BYTE *res_name_tbl =  (BYTE *)(header_image  + GWN(RESIDENT_TBL));
	char *name;
	WORD entry;

	if (options & OPT_PRINTSTRUCT) {
		fprintf(stdout, "\n\n/* Header file generated by whead */\n\n");
		name = cnvrt_pascalstr(res_name_tbl);
		fprintf(stdout, "ENTRY %s_api[] = {\n", name);
		fprintf(stdout, "\t{    0, \"%s\"},\n", name);
	} else
		fprintf(stdout, "\n\nNon-resident name table at %04x\n", offset);
	while (*tmpbptr != 0) {
		name = cnvrt_pascalstr(tmpbptr);
		tmpbptr = tmpbptr + strlen(name) + 1;
		entry = get_word(tmpbptr);
		tmpbptr += 2;		/* sizeof(WORD) */
		if (options & OPT_PRINTSTRUCT) {
			if (entry != 0)
				fprintf(stdout, "\t{ %4d, \"%s\"},\n", entry, name);
		} else {
			if (entry == 0)
				fprintf(stdout, "  Module description: \"%s\"\n", name);
			else
				fprintf(stdout, "  Name: %-24sEntry: %04x\n", name, entry);
		}
	}
	if (options & OPT_PRINTSTRUCT) 
		fprintf(stdout, "\t{   -1, \"\"},\n};\n");
}

/* Dump entry table */
void
dump_entrytable()
{
	DWORD offset = GWN(ENTRY_TBL) ;
	BYTE *tmpbptr = (BYTE *)(header_image + offset);
	int entry_cnt = 0;
	WORD w1, w2, w3;
	int i;
	int entry_num = 1;

	fprintf(stdout, "\n\nEntry table at %04x", offset);
	while (*tmpbptr != 0) {
		/* Process thr bundle */
		entry_cnt = *tmpbptr++;
		switch(*tmpbptr) {
			case 0:
				fprintf(stdout, "\n\n  Unused segment records -- 0x%02x entries", entry_cnt);
				++tmpbptr;
				entry_num += entry_cnt;
				break;
			case 0xff:
				fprintf(stdout, "\n\n  Movable segment records -- 0x%02x entries", entry_cnt);
				++tmpbptr;
				for (i = 1; i <= entry_cnt; i++, entry_num++) {
					w1 = *tmpbptr;
					tmpbptr += 3;		/* an int 3f instrn here */
					w2 = *tmpbptr++;
					w3 = get_word(tmpbptr);
					tmpbptr += 2;		/* sizeof(WORD) */
					fprintf(stdout, "\n\tEntry: %.4x  Segment: %02x  Offset: %04x",
							entry_num, w2, w3);
					if (w1 & ENTRYF_EXPORTED)
						fprintf(stdout, "  Exported");
					if (w1 & ENTRYF_SHAREDDATA)
						fprintf(stdout, "  Uses shared data");
				}
				break;
			default:
				fprintf(stdout, "\n\n  Fixed(segment number %04x) segment records -- 0x%02x entries", *tmpbptr, entry_cnt);
				++tmpbptr;
				for (i = 1; i <= entry_cnt; i++, entry_num++) {
					w1 = *tmpbptr++;
					w2 = get_word(tmpbptr);
					tmpbptr += 2;		/* sizeof(WORD) */
					fprintf(stdout, "\n\tEntry: %.4x  Offset: %04x", entry_num, w2);
					if (w1 & ENTRYF_EXPORTED)
						fprintf(stdout, "  Exported");
					if (w1 & ENTRYF_SHAREDDATA)
						fprintf(stdout, "  Uses shared data");
				}
				break;
		}	/* case statement for type of bundle segments */
	}	/* while loop for bundles */
}

/* Scan through the entry table for the nth entry and return the address */
struct addr *
search_entrytable(ordinal)
WORD ordinal;
{
	static struct addr ad;
	DWORD offset = GWN(ENTRY_TBL) ;
	BYTE *tmpbptr = (BYTE *)(header_image + offset);
	int entry_cnt = 0;
	int entry_num = 1;

	while (*tmpbptr != 0) {
		/* For each of the bundles */
		entry_cnt = *tmpbptr++;
		switch(*tmpbptr) {
			case	0:	/* Null entries */
					++tmpbptr;
					entry_num += entry_cnt;
					continue;

			case	0xff:
					/* Movable segment type */
					++tmpbptr;
					while(entry_cnt--) {
						if (entry_num == ordinal) {
							tmpbptr += 3;	/* Skip flag and int 3f instrn */
							ad.segment = *tmpbptr++;
							ad.offset = get_word(tmpbptr);
							return(&ad);
						}
						tmpbptr += MOVABLESEG_ENTRYSZ;
						++entry_num;
					}
					break;

			default:
					/* Fixed segment type */
					ad.segment = *tmpbptr;
					++tmpbptr;
					while(entry_cnt--) {
						if (entry_num == ordinal) {
							tmpbptr++;	/* Skip flag byte */
							ad.offset = get_word(tmpbptr);	
							return(&ad);
						} else
							tmpbptr += FIXEDSEG_ENTRYSZ;
						++entry_num;
					}
					break;
		}	/* switch for bundle type */
	}	/* while for the list of bundles */
	return(NULL);
}

/* Dump relocation information for a segment */
void
dump_relocation(exefile, outfile, segnum)
FILE *exefile;
FILE *outfile;
int segnum;
{
	BYTE *buff, *buff1;
	char tmpc[4];
	unsigned long file_offset;
	BYTE *reloc_ptr;
	BYTE *segment_ptr;
	BYTE *imp_name_tbl = (BYTE *)(header_image  + GWN(IMPORTED_TBL));
	BYTE *mod_ref_tbl = (BYTE *)(header_image  + GWN(MODULEREF_TBL));
	BYTE *entry = (BYTE *)(header_image + GWN(SEGMENT_TBL));
	int reloc_cnt = 0;
	int reloc_recs;
	int i, len;
	WORD w1, w2;
	BYTE flag = 0;
	char *name;
	struct addr *address;
	char src_type[12];
	char src_offset[16];
	char target[64];
	char alt_target[64];
	struct segentry ent;

	entry += (segnum - 1) * SEGMENT_ENTRYSZ;
	ent.sec_offset = get_word(entry + SEGENTRY_START);
	ent.length = get_word(entry + SEGENTRY_LENGTH);
	ent.flag = get_word(entry + SEGENTRY_FLAG);
	ent.alloc_sz = get_word(entry + SEGENTRY_ALLOCSZ);

	if (!(ent.flag & SEG_RELOCINFO)) {
		if (print_level & PRINT_RELOC_MSG)
			fprintf(outfile, "\n\tNo relocation information for this segment\n\n");
		return;
	}
	if (ent.length == 0)
		len = 64 * 1024;
	else
		len = ent.length;
	file_offset = (ent.sec_offset << GWN(SHIFT_CNT));
	/* Read segment */
	fseek(exefile, file_offset, 0);
	if ((segment_ptr = buff = (BYTE *)malloc(len)) == (BYTE *)NULL)
		wh_panic(WHERR_NOMEM);
	wh_fread(buff, 1, len, exefile);

	/* read the relocation item count */
	wh_fread(tmpc, 2, 1, exefile);
	reloc_recs = get_word(tmpc);

	/* Read relocation records */
	if ((reloc_ptr = buff1 = (BYTE *)malloc(reloc_recs * RELOCITEM_SZ)) == (BYTE *)NULL)
		wh_panic(WHERR_NOMEM);

	wh_fread(buff1, RELOCITEM_SZ, reloc_recs, exefile);

	/* And process 'em all */
	if (print_level & PRINT_RELOC_MSG)
		fprintf(outfile, "\n\tSrc_type  Src_addr\tTarget\t\t\t\tAlt_target\n");
	for (i = 0; i < reloc_recs; i++, reloc_ptr += RELOCITEM_SZ) {
		switch(*(reloc_ptr + RELOCATION_SRC) & RELOCSRC_MASK) {
			case	RELOCSRC_SEGMENT:
					strcpy(src_type, "Segment ");
					break;
			case	RELOCSRC_FARADDR:
					strcpy(src_type, "Faraddr ");
					break;
			case	RELOCSRC_OFFSET:
					strcpy(src_type, "Offset  ");
					break;
			default:
					wh_perror(WHERR_INVRELOCSRC);
		} /* switch for relocation source types */

		flag = *(reloc_ptr + RELOCATION_FLAG);

		switch(flag & RELOCF_TARGETMASK) {
			case	RELOCF_INTERNALREF:
					/* Movable segments need to be taken care of */
					w1 = get_word(reloc_ptr + RELOCIREF_SEGNUM);
					w2 = get_word(reloc_ptr + RELOCIREF_INDEX);
					if (w1 != 0xff) {
						sprintf(target, "%04x:%04x", w1, w2);
						strcpy(alt_target, "Internal (Fixed seg)");
					} else {
						if ((address = search_entrytable(w2)) != NULL) {
							sprintf(target, "%04x:%04x", address->segment,
									address->offset);
							strcpy(alt_target, "Internal (Moveable seg)");
						} else {
							sprintf(target, "Invalid entry: %04x", w2);
							strcpy(alt_target, target);
						}
					}
					break;

			case	RELOCF_IMPORTORDINAL:
					w1 = get_word(reloc_ptr + RELOCIORD_INDEX) - 1;
					w2 = get_word(mod_ref_tbl + 2*w1);
					name = cnvrt_pascalstr(imp_name_tbl + w2);
					str_toupper(name);
					w2 = get_word(reloc_ptr + RELOCIORD_PROCNUM);
					/* fprintf(outfile, "\t%s.%d", name, w2); */
					sprintf(target, "%s", cnvrt_ordinal(name, w2));
					sprintf(alt_target, "%s.%04x", name, w2);
					break;

			case	RELOCF_IMPORTNAME:
					w1 = get_word(reloc_ptr + RELOCINAME_INDEX) - 1;
					w2 = get_word(mod_ref_tbl + 2*w1);
					name = cnvrt_pascalstr(imp_name_tbl + w2);
					str_toupper(name);
					strcpy(target, name);
					w2 = get_word(reloc_ptr + RELOCINAME_OFFSET);
					name = cnvrt_pascalstr(imp_name_tbl + w2);
					sprintf(alt_target, "%s", cnvrt_module_name(target, name));
					strcat(target, name);
					break;

			case	RELOCF_OSFIXUP:
					w1 = get_word(reloc_ptr + RELOCOSFIX_FIXUPNUM);
					sprintf(target, "OSFIXUP_#%04x", w1);
					strcpy(alt_target, "Internal (Soft math)");
					break;
			default:
					/* Cannot come here now that we have RELOCF_OSFIXUP */
					wh_perror(WHERR_INVRELOCTRG);
		} /* switch for target mask */
		/* The source offset */
		w1 = get_word(reloc_ptr + RELOCATION_OFFSET);
		do {
			/* Follow the source chain if NOT Additive relocation
			 * Need to take a grammar course on the abuse of conjunctions in
			 * technical documentation.
			 *	If the ADDITIVE flag is set, then add the target value to the
			 *	source contents instead of (replacing the source and following
			 *	the chain). Obviously the parenthesis was NOT there and I 
			 * followed the chain adding the value in there instead of taking
			 * the word there and going to the next element in the chain!!
		 	 */
			/*			if (flag & RELOCF_ADDITIVE)
							off += w1;
						else
							off = w1;
			*/

			sprintf(src_offset, "%04x:%04x", segnum, w1);

			fprintf(outfile, "\t%s  %s\t%-36s%-16s", src_type, src_offset,
					target, alt_target);
			if (flag & RELOCF_ADDITIVE) {
				fprintf(outfile, "\tAdditive + 0x%04x\n", get_word(segment_ptr + w1));
				break;
			}
			fprintf(outfile, "\n");
			if ((options & OPT_NORELOCCHAIN) ||
				((flag & RELOCF_TARGETMASK) == RELOCF_OSFIXUP))
				break;
			w1 = get_word(segment_ptr + w1);
			++reloc_cnt;
			strcpy(src_type, "SrcChain");
		} while (w1 != 0xffff);
	} /* for loop for relocation entries */
	if (print_level  & PRINT_RELOC_MSG)
		fprintf(outfile,"\n\tRelocation records: %04x\t", reloc_recs);
	if (options & OPT_NORELOCCHAIN)
		fprintf(outfile, "\n");
	else
		fprintf(outfile, "Relocation items: %04x\n", reloc_cnt);
	free(buff);
	free(buff1);
}

/* Returns non-zero if self loading application, else 0 */
BOOL
is_selfloading(exefile)
FILE *exefile;
{
	WORD w1;
	BYTE *tmpbptr = (BYTE *)(header_image + GWN(SEGMENT_TBL) );
	char tmpc[4];

	/* Read signature on segment 1 to isolate self-loading apps */
	w1 = get_word(tmpbptr);		/* Sector offset of segment 1 */
	w1 <<= GWN(SHIFT_CNT);
	fseek(exefile, w1, 0);
	wh_fread(tmpc, 2, 1, exefile);
	w1 = get_word(tmpc);
	if (w1 == 0x3041) 		/* A0 */
		return(1);
	else
		return(0);
}

/* Dump relocation entries for all segments */
void
dump_allrelocations(exefile, outfile)
FILE *exefile;
FILE *outfile;
{
	int seg_cnt = GWN(SEGMENTTBL_SZ);
	int i;

	if (is_selfloading(exefile)) {
		fprintf(outfile, "\nSELF LOADING APPLICATION...  ");
		fprintf(outfile, "Processing one segment\n");
		seg_cnt = 1;
	}
	if (print_level  & PRINT_RELOC_MSG)
		fprintf(outfile, "\nSegment relocation records\n");
	for (i = 1; i <= seg_cnt; i++) {
		if (print_level  & PRINT_RELOC_MSG)
			fprintf(outfile, "\n  Segment %04x relocations\n", i);
		dump_relocation(exefile, outfile, i);
	}
}

/* Create a temporary file with relocatable targets which are not internal
 * references and returns the filename 
 */
char *
create_importlist_file(in)
FILE	*in;
{
	static	char awk_cmd[] = "$4!=\"Internal\" {op = sprintf(\"%-40s%s\", $4, $3); print op;}";
	static	char exec_fmt[] = "%s '%s' %s | %s -u";
	FILE *tmpfile;
	FILE *ret_tmpfile;
	FILE *tmppipe;
	char fname[128];
	static	char ret_fname[128];
	char exec_line[256];

	tmpnam(fname);
	tmpnam(ret_fname);
	if ((tmpfile = fopen(fname, "w")) == (FILE*)NULL) {
		fprintf(stderr, "Error opening temp file %s\n", fname);
		return;
	}
	if ((ret_tmpfile = fopen(ret_fname, "w")) == (FILE*)NULL) {
		fprintf(stderr, "Error opening temp file %s\n", ret_fname);
		return;
	}

	/* Having source chains would unnecessarily slow things */
	options |= OPT_NORELOCCHAIN;
	dump_allrelocations(in, tmpfile);
	fclose(tmpfile);

	/* And process the temp file */
	sprintf(exec_line, exec_fmt, AWK, awk_cmd, fname, SORT);
	if ((tmppipe = popen(exec_line, "r")) == NULL) {
		fprintf(stderr, "create_importlist_file: Error opening pipe\n");
		unlink(fname);
		return((char *)NULL);
	}

	while (fgets(exec_line, 256, tmppipe) != (char *)NULL) 
		fprintf(ret_tmpfile, "%s", exec_line);
	fclose(ret_tmpfile);
	pclose(tmppipe);

	/* Get rid of the first temp file */
	unlink(fname);
	return(ret_fname);
}

/* Dump all imported functions by module */
void
dump_imports(in)
FILE	*in;
{
	static	char exec_fmt[] = "%s \"%s\\.\" %s";
	FILE *tmppipe;
	char *implist_fname;
	char *module_name;
	int	save;
	BYTE *tmpbptr = (BYTE *)(header_image + GWN(MODULEREF_TBL) );
	BYTE *tmpbptr1 = (BYTE *)(header_image + GWN(IMPORTED_TBL) );
	WORD i, j;
	int count = 0;
	char exec_line[256];

	if ((implist_fname = create_importlist_file(in)) == (char *)NULL) 
		return;

	/* And process the temp file */
	for (i = 1; tmpbptr < tmpbptr1 ; i++, tmpbptr += 2) {
		j = get_word(tmpbptr);
		if (j == 0)
			break;
		count = 0;
		module_name = cnvrt_pascalstr(tmpbptr1 + j);
		fprintf(stdout, "Functions imported from module %s\n\n", module_name);
		sprintf(exec_line, exec_fmt, GREP, module_name, implist_fname);
		if ((tmppipe = popen(exec_line, "r")) == NULL) {
			fprintf(stderr, "dump_imports: Error opening pipe\n");
		} else {
			while (fgets(exec_line, 256, tmppipe) != (char *)NULL) {
				fprintf(stdout, "%s", exec_line);
				++count;
			}
			pclose(tmppipe);
		}
		fprintf(stdout, "\nCount of ordinals in module %s: %d\n\n",
				module_name, count);
	}

	/* Get rid of the temp file */
	unlink(implist_fname);
}

/* Dump all non-internal reference targets in the relocation table */
void
dump_imports_paradox(in)
{
	FILE *tmpfile;
	char *implist_fname;
	char inp_line[256];
	char *token;

	if ((implist_fname = create_importlist_file(in)) == (char *)NULL) 
		return;
	if ((tmpfile = fopen(implist_fname, "r")) == (FILE*)NULL) {
		fprintf(stderr, "Error opening imports file %s\n", implist_fname);
		return;
	}

	while (fgets(inp_line, 256, tmpfile) != (char *)NULL) {
		token = strtok(inp_line, " ");
		fprintf(stdout, "%s\n", token);
	}

	fclose(tmpfile);
	unlink(implist_fname);
}

/* If something is wrong with cmd line invocation, this is called */
void
print_usage(progname)
char *progname;
{
	fprintf(stderr, "\nUsage %s {- /}options filename(s)\n", progname);
	fprintf(stderr, "Valid options are:\n");
	fprintf(stderr, "\t%c - Dump all the program can\n", SW_ALL);
	fprintf(stderr, "\t%c - Main header\n", SW_HEADER);
	fprintf(stderr, "\t%c - Segment information\n", SW_SEGMENTS);
	fprintf(stderr, "\t%c - Relocation information\n", SW_RELOCATIONS);
	fprintf(stderr, "\t%c - Resident table\n", SW_RESIDENTTBL);
	fprintf(stderr, "\t%c - Module refernce table\n", SW_MODULEREFTBL);
	fprintf(stderr, "\t%c - Entry table\n", SW_ENTRYTBL);
	fprintf(stderr, "\t%c - Non-resident table\n", SW_NONRESIDENTTBL);
	fprintf(stderr, "\t%c - Resource table\n", SW_RESOURCETBL);
	fprintf(stderr, "\t%c - Imported names table\n", SW_IMPORTTBL);
	fprintf(stderr, "\t%c - Imported function list\n", SW_IMPORTS);
	fprintf(stderr, "\t%c - Do NOT process source chain in relocation\n",
	        SW_NORELOCCHAIN);
	fprintf(stderr, "\t%c - Force this program to be a windows EXE\n", 
		SW_OVERRIDEWINEXE);
	fprintf(stderr, "\t%c - Imported function list for use in Paradox\n", 
	        SW_PARADOX);
	fprintf(stderr, "\t%c - Silent - minimal error/status display\n", SW_SILENT);
	fprintf(stderr, "\t%c - Preload all segments\n", SW_OPT_PRELOADSEGS);
	fprintf(stderr, "\t%c - Print the ENTRY structure for adding to apilib\n\n",
	        SW_PRINTSTRUCT);
}

int
process_one_file(fname)
char *fname;
{
	FILE *in;
	FILE *out;
	WORD tmpw;
	DWORD tmpdw;
	int tmpi;
	char *tmpptr;
	char tmpbuff[128];

	if ((in = fopen(fname, "r")) == (FILE *)NULL) {
		fprintf(stderr, "Error opening %s\n\n", fname);
		return(-1);
	}
	wh_fread(header_image, 1, EXEHDR_SZ, in);
	if (get_word(header_image + MZ_SIGNATURE) != 0x5a4d) {
		wh_perror(WHERR_NOMZHDR);
		return(-1);
	}
	if (get_word(header_image + RELOC_TABLE) < 0x40) {
		wh_perror(WHERR_NOTNEWEXE);
		if (!(options & OPT_OVERRIDEWINEXE))
			return(-1);
	}
	ne_offset = get_dword(header_image + 0x3c);

	fseek(in, ne_offset, 0);
	nehdr = header_image;
	if (fread(header_image, 1, NEWEXEHDR_SZ, in) != NEWEXEHDR_SZ)
		wh_panic(WHERR_NOTNEWEXE);

	if (get_word(nehdr + NE_SIGNATURE) != 0x454e) {
		wh_perror(WHERR_NONEHDR);
	}

	/* Find out the offset of the first segment in the file and read upto that
	 * point into header_image, so that we would not have to do any more read
	 * operations until relocation
	 * Just discovered a catch. There is Gangload area(tdump) which comes
	 * between the nan resident name table and the first segment and is usually
	 * large. So, we would end up reading more than what is needed. 
	 */
	tmpw = GWN(SEGMENT_TBL);
	fseek(in, ne_offset, 0);

	wh_fread(header_image, 1, tmpw + SEGMENT_ENTRYSZ, in);
	tmpw = GWH(tmpw + SEGENTRY_START);
	/* tmpw has the logical sector offset of the first sector */
	tmpdw = tmpw << GWN(SHIFT_CNT);
	header_image = realloc(header_image, tmpdw);

	fseek(in, ne_offset, 0);
	wh_fread(header_image, 1, tmpdw, in);
	nehdr = header_image;

	if (print_level & PRINT_TITLE)
		fprintf(stdout, "\n\n\t\t\tDump of file: %s\n\n", fname);
	/* process various dump options */
	if (options & OPT_HEADER)
		dump_header();
	if (options & OPT_SEGMENTS)
		dump_segments();
	/* Resource table needs to be decoded */
	if (options & OPT_RESOURCETBL)
		dump_restbl(in);
	if (options & OPT_RESIDENTTBL)
		dump_residenttable(); 
	if (options & OPT_MODULEREFTBL)
		dump_modulereftable(); 
	if (options & OPT_IMPORTTBL)
		dump_importtable(); 
	if (options & OPT_ENTRYTBL)
		dump_entrytable(); 
	if ((options & OPT_NONRESIDENTTBL) || (options & OPT_PRINTSTRUCT))
		dump_nonresidenttable(); 
	if (options & OPT_RELOCATIONS)
		dump_allrelocations(in, stdout); 
	if (options & OPT_IMPORTS)
		dump_imports(in); 
	if (options & OPT_PARADOX)
		dump_imports_paradox(in); 
	if (options & OPT_UNMUNGE) {
		if (!is_selfloading(in))
			wh_panic(WHERR_NOTSELFLOADING);
		strcpy(tmpbuff, "ua_");
		strcat(tmpbuff, fname);
		fprintf(stdout, "Generating %s (unmunged version) from %s..\n",
				tmpbuff, fname);
		if ((out = fopen(tmpbuff, "w+")) == (FILE *)NULL) {
			fprintf(stderr, "Error opening %s\n", tmpbuff);
			exit(-1);
		}
		if (unmunge(in, out)) {
			fprintf(stderr, "Removing %s\n", tmpbuff);
			unlink(tmpbuff);
			exit(-1);
		}
		fclose(out);
	}	/* (options & OPT_MUNGE) */

	if (options & OPT_PRELOADSEGS) {
		int cnt;
		int i;

		strcpy(tmpbuff, fname);
		strcat(tmpbuff, ".preload");
		fprintf(stdout, "Generating %s (all segments preloaded) from %s..\n",
				tmpbuff, fname);
		if ((out = fopen(tmpbuff, "w+")) == (FILE *)NULL) {
			fprintf(stderr, "Error opening %s\n", tmpbuff);
			exit(-1);
		}
		if ((tmpptr = (char *)malloc(128 * 1024)) == (char *)NULL) 
			wh_panic(WHERR_NOMEM);
		rewind(in);
		rewind(out);
		/* Make a copy of the file */
		while ((cnt = fread(tmpptr, 1, 128 * 1024, in)) != 0) 
			fwrite(tmpptr, 1, cnt, out);
		fseek(out, ne_offset + GWN(SEGMENT_TBL), 0);

		cnt = GWN(SEGMENTTBL_SZ);
		tmpptr = header_image + GWN(SEGMENT_TBL);
		for (i = 1; i <= cnt; i++, tmpptr += SEGMENT_ENTRYSZ) {
			tmpw = get_word(tmpptr + SEGENTRY_FLAG);
			tmpw |= SEG_PRELOAD;
			put_word(tmpptr + SEGENTRY_FLAG, tmpw);
		}
		tmpptr = header_image + GWN(SEGMENT_TBL);
		fwrite(tmpptr, SEGMENT_ENTRYSZ, cnt, out);
		fclose(out);
	} /* (options & OPT_PRELOADSEGS) */

	fclose(in);
	return(0);
}

main(argc, argv)
int argc;
char *argv[];
{
	static char rcs_id[] = "$Revision: 1.29 $ $Date: 92/12/11 11:28:53 $";
	char *tmpptr;
	int i;

	tmpptr = rcs_id;
	while (*tmpptr)
		if (*tmpptr == '$')
			*tmpptr++ = ' ';
		else
			tmpptr++;
	if (argc < 3) {
		print_usage(argv[0]);
		exit(-1);
	}
	/* Scan argv[1] for valid options and set 'em */
	tmpptr = argv[1];
	while (*tmpptr) {
		switch(*tmpptr) {
			case	SW_ALL:
					options = OPT_ALL;
					break;
			case	SW_HEADER:
					options |= OPT_HEADER;
					break;
			case	SW_SEGMENTS:
					options |= OPT_SEGMENTS;
					break;
			case	SW_RELOCATIONS:
					options |= OPT_RELOCATIONS;
					break;
			case	SW_RESIDENTTBL:
					options |= OPT_RESIDENTTBL;
					break;
			case	SW_MODULEREFTBL:
					options |= OPT_MODULEREFTBL;
					break;
			case	SW_ENTRYTBL:
					options |= OPT_ENTRYTBL;
					break;
			case	SW_NONRESIDENTTBL:
					options |= OPT_NONRESIDENTTBL;
					break;
			case	SW_RESOURCETBL:
					options |= OPT_RESOURCETBL;
					break;
			case	SW_IMPORTTBL:
					options |= OPT_IMPORTTBL;
					break;
			case	SW_NORELOCCHAIN:
					options |= OPT_NORELOCCHAIN;
					break;
			case	SW_UNMUNGE:
					options |= OPT_UNMUNGE;
					break;
			case	SW_OPT_PRELOADSEGS:
					options |= OPT_PRELOADSEGS;
					break;
			case	SW_PRINTSTRUCT:
					options |= OPT_PRINTSTRUCT;
					break;
			case	SW_IMPORTS:
					options |= OPT_IMPORTS;
					print_level &= ~PRINT_RELOC_MSG;
					break;
			case	SW_PARADOX:
					options |= OPT_PARADOX;
					print_level &= ~PRINT_RELOC_MSG;
					break;
			case	SW_OVERRIDEWINEXE:
					options |= OPT_OVERRIDEWINEXE;
					break;
			case	SW_SILENT:
					options |= OPT_SILENT;
					print_level &= ~PRINT_TITLE;
					break;
			case	'/':
			case	'-':
					break;
			default:
					fprintf(stderr, "Invalid option %c\n", *tmpptr);
					break;
		}
		++tmpptr;
	}
	/* Allocate space to read in header */
	if ((header_image = (char *)malloc(1 * 1024)) == (char *)NULL)
		wh_panic(WHERR_NOMEM);

	if (print_level & PRINT_TITLE) {
		fprintf(stderr, "Windows exe header processing utility\n");
		fprintf(stderr, "%s\n", rcs_id);
	}
	for (i = 2; i < argc; i++) {
		if (print_level & PRINT_TITLE)
			fprintf(stderr, "\n%s:", argv[i]);
		process_one_file(argv[i]);
	}
	return(0);
}






