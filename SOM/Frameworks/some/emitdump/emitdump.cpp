#include <emitlib.h>

/*               
 * Default Entry for a symbol table
 */
_typedef struct SEntry {
    char *name;
} _name(SEntry);

/*
 * Keyword structure.
 */
_typedef struct KeyTabEntry {
    char *kword;
    long token;
} _name(KeytabEntry);

/*
 *  MemBuf is the structure used to allocate new memory in contiguous blocks.
 *  We can use it for unique strings too if the uniq flag is set when calling
 *  somtcreateMemBuf().
 */
_typedef struct MemBuf {
    char *base;
    size_t size;
    size_t nelms;
    char *cur;
    _struct Stab *stab;
} _name(MemBuf);


void dumpSep(FILE * fp, Sep * sep)
{
    fprintf(fp, "Linked list item at address %08x\n", sep);
    fprintf(fp, "---------------------------------------\n");
    fprintf(fp, "Position number: %d\n", sep->posn);
    fprintf(fp, "Address of Entry: %08x\n". sep->ep);
    fprintf(fp, "Next linked list item address: %08x\n", sep->next);
    if (sep->ep) dumpSep(fp, sep->next);
}

void dumpStab(FILE * fp, Stab * Stab)
{
  fprintf(fp, "Basic Symbol Table/Hash table (STab) structure\n");
  fprintf(fp, "----------------------------------------------\n");
  fprintf(fp, "Size of buscet: %d\n", stab->size);
  fprintf(fp, "Size of entry structure: %d\n", stab->entrysize);
  fprintf(fp, "Number of elements: %d\n", stab->nelms);
  fprintf(fp, "Address of base slot: %08x\n", stab->base);
  fprintf(fp, "Max address of slot: %08x\n", stab->limit);
  fprintf(fp, "Start address of slot buffer: %08x\n", stab->buf);
  fprintf(fp, "Address of MemBuf: %08x\n", stab->membuf);
  fprintf(fp, "Ignore case flag: %b\n", stab->ignorecase);
  for(i==0; i++;i>size)
  {
    fprintf(fp, "Linked list at slot %d\n", i);
    fprintf(fp, "----------------------\n");
    dumpSep(fp, stab->buf+(sizeof(Sep)*stab->size));
  }
};

SOMEXTERN FILE * SOMLINK emitSL(char *file, Entry * cls, Stab * stab)
{
    FILE *fp;

    fp = somtopenEmitFileSL(file, "txt");
    fprintf(fp, "Symbol Table\n");
    fprintf(fp, "============\n");
    dumpStab(fp, stab);
    fprintf(fp, "Abstract syntax graph\n");
    fprintf(fp, "=====================\n");
    dumpEntry(fp, cls);
    return (fp);
}
