/*
 * File hash.c - generate hash tables for Wine debugger symbols
 *
 * Copyright (C) 1993, Eric Youngdale.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "platform.h"

extern FILE *debug_outfp;

extern char *dladdrtosym(unsigned long addr, char *name_buffer);

struct  name_hash
{
    struct name_hash * next;
    unsigned int * address;
    char * name;
};

#define NR_NAME_HASH 128

static struct name_hash * name_hash_table[NR_NAME_HASH] = {0,};

static  unsigned int name_hash(const char * name)
{
    unsigned int hash = 0;
    const char * p;

    p = name;

    while (*p) 
	hash = (hash << 15) + (hash << 3) + (hash >> 3) + *p++;

    return hash % NR_NAME_HASH;
}


void add_hash(char * name, unsigned int * address)
{
    struct name_hash  * new;
    int hash;

    new = (struct  name_hash *) malloc(sizeof(struct name_hash));
    new->address = address;
    new->name = (char *) strdup(name);
    new->next = NULL;
    hash = name_hash(name);

    /* Now insert into the hash table */
    new->next = name_hash_table[hash];
    name_hash_table[hash] = new;
}

unsigned int * _dl_find_hash(char *, char *, int , char *, int);

unsigned int * find_hash(char * name)
{
    char buffer[256];
    struct name_hash  * nh;
    unsigned int *addr;

#if defined(ELF) && defined(linux)
    addr = (unsigned int *) _dl_find_hash(name, NULL, 0, NULL, 1);
    if (addr)
	return addr;
#endif /* ELF */

    for(nh = name_hash_table[name_hash(name)]; nh; nh = nh->next)
	if(strcmp(nh->name, name) == 0) 
	    return nh->address;

    if(name[0] != '_')
    {
	buffer[0] = '_';
	strcpy(buffer+1, name);
	for(nh = name_hash_table[name_hash(buffer)]; nh; nh = nh->next)
	    if(strcmp(nh->name, buffer) == 0) 
		return nh->address;
    }

    return (unsigned int *) 0xffffffff;
}


static char name_buffer[256];

#if defined(ELF) && defined(linux)
struct name_hash * find_nearest_elf_symbol(unsigned int * address)
{
    static char name[256];
    static struct name_hash elf_nearest;

    if (!dladdrtosym((unsigned long)address, name))
	return NULL;

    elf_nearest.next = NULL;
    elf_nearest.address = find_hash(name);
    elf_nearest.name = name;
    
    return &elf_nearest;
}
#endif /* ELF */

char * find_nearest_symbol(unsigned int * address)
{
    struct name_hash *nearest = NULL;
    struct name_hash start;
    struct name_hash *nh;
    int i;

#if defined(ELF) && defined(linux)
    nearest = find_nearest_elf_symbol(address);
#endif

    if (!nearest)
    {
	nearest = &start;
	start.address = (unsigned int *) 0;
    }
	
    for(i=0; i<NR_NAME_HASH; i++) 
    {
	for(nh = name_hash_table[i]; nh; nh = nh->next)
	    if(nh->address <= address && nh->address > nearest->address)
		nearest = nh;
    }

    if((unsigned int) nearest->address == 0) 
	return NULL;

    sprintf(name_buffer, "%s+0x%x", nearest->name, 
	    ((unsigned int) address) - ((unsigned int) nearest->address));

    return name_buffer;
}


void
read_symboltable(char * filename)
{
    FILE * symbolfile;
    unsigned int addr;
    int nargs;
    char type;
    char * cpnt;
    char buffer[256];
    char name[256];

    symbolfile = fopen(filename, "r");
    if(!symbolfile) 
    {
	fprintf(debug_outfp,"Unable to open symbol table %s\n", filename);
	return;
    }

    fprintf(debug_outfp,"Reading symbols from file %s\n", filename);

    while (1)
    {
	fgets(buffer, sizeof(buffer),  symbolfile);
	if (feof(symbolfile)) break;
		
	/* Strip any text after a # sign (i.e. comments) */
	cpnt = buffer;
	while(*cpnt)
	{
	    if(*cpnt == '#') {*cpnt = 0; break; };
	    cpnt++;
	}
		
	/* Quietly ignore any lines that have just whitespace */
	cpnt = buffer;
	while(*cpnt)
	{
	    if(*cpnt != ' ' && *cpnt != '\t') 
		break;
	    cpnt++;
	}

	if (!(*cpnt) || *cpnt == '\n') 
	{
	    continue;
	}
		
	nargs = sscanf(buffer, "%x %c %s", &addr, &type, name);
	add_hash(name, (unsigned int *) addr);
    }

    fclose(symbolfile);
}

void find_invoke_native_bounds(unsigned long *startp,
			       unsigned long *endp)
{

#if defined(ELF) && defined(linux)
    *startp = (unsigned long) find_hash("invoke_native");
    *endp   = (unsigned long) find_hash("invoke_code16") - 1;
#else

    struct name_hash *above_nearest = NULL;
    struct name_hash *nh;
    int i;

    *startp = (unsigned long) find_hash("_invoke_native");
	
    for(i=0; i<NR_NAME_HASH; i++) 
    {
	for(nh = name_hash_table[i]; nh; nh = nh->next)
	{
	    if((unsigned long) nh->address > *startp && 
	       (!above_nearest || nh->address < above_nearest->address))
	    {
		above_nearest = nh;
	    }
	}
    }

    *endp   = (unsigned long) above_nearest->address;
#endif
}

#if defined(ELF) && defined(linux)

#include "dlhash.h"

void
dump_init_sections()
{
    struct elf_resolve *erp;

    for (erp = _dl_loaded_modules; erp; erp = erp->next)
    {
	fprintf(stderr, "%s: DT_INIT at %08x\n",
		erp->libname ? erp->libname : "NONE", 
		(erp->dynamic_info[DT_INIT] +
		 (unsigned int)erp->loadaddr));
    }
}

char *dladdrtosym(unsigned long addr, char *name_buffer)
{
    struct elf_resolve *erp;
    struct elf_resolve *best_erp = NULL;
    struct elf32_sym *symtab;
    char *strtab;
    char *name;
    int best_si = 0;
    int best_sv = 0;
    int hn;
    int si;
    unsigned long sv;

    for (erp = _dl_loaded_modules; erp; erp = erp->next)
    {
	/* if this is a candidate address AND we have no best yet, OR */
 	/* this one is better than the previous one, then we have a   */
	/* new best */
	if ((unsigned long) erp->loadaddr < addr &&
	    (!best_erp || erp->loadaddr > best_erp->loadaddr))
	{
	    best_erp = erp;
	}
    }

    /* there is no good candidate */
    if (!best_erp)
	return NULL;
    
    symtab = (struct elf32_sym *) (best_erp->dynamic_info[DT_SYMTAB] +
				   best_erp->loadaddr);
    strtab = (char *) (best_erp->dynamic_info[DT_STRTAB] + best_erp->loadaddr);
    for (hn = 0; hn < best_erp->nbucket; hn++)
    {
	for (si = best_erp->elf_buckets[hn]; si; si = best_erp->chains[si])
	{
	    if (ELF32_ST_TYPE(symtab[si].st_info) == STT_FUNC ||
		ELF32_ST_TYPE(symtab[si].st_info) == STT_NOTYPE ||
		ELF32_ST_TYPE(symtab[si].st_info) == STT_OBJECT)
	    {
		sv = (unsigned long) best_erp->loadaddr + symtab[si].st_value;
		if (sv <= addr && sv > best_sv)
		{
		    best_sv = sv;
		    best_si = si;
		}
	    }
	}
    }

    name = strtab + symtab[best_si].st_name;
/*    _dl_strcpy(name_buffer, name); */
    strcpy(name_buffer, name);
    
    return name_buffer;
}

#endif /* ELF */
