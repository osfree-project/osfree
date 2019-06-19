

/*  
 * Old MZ header for DOS programs.
 * We check only the magic and the e_lfanew offset to the new executable
 * header.
 */
typedef struct
{
        WORD    e_magic;        /* MZ Header signature */
        WORD    e_cblp;         /* Bytes on last page of file */
        WORD    e_cp;           /* Pages in file */
        WORD    e_crlc;         /* Relocations */
        WORD    e_cparhdr;      /* Size of header in paragraphs */
        WORD    e_minalloc;     /* Minimum extra paragraphs needed */
        WORD    e_maxalloc;     /* Maximum extra paragraphs needed */
        WORD    e_ss;           /* Initial (relative) SS value */
        WORD    e_sp;           /* Initial SP value */
        WORD    e_csum;         /* Checksum */
        WORD    e_ip;           /* Initial IP value */
        WORD    e_cs;           /* Initial (relative) CS value */
        WORD    e_lfarlc;       /* File address of relocation table */
        WORD    e_ovno;         /* Overlay number */    
        WORD    e_res[4];       /* Reserved words */    
        WORD    e_oemid;        /* OEM identifier (for e_oeminfo) */
        WORD    e_oeminfo;      /* OEM information; e_oemid specific */
        WORD    e_res2[10];     /* Reserved words */
        WORD    e_lfanew;       /* Offset to extended header */
} IMAGE_DOS_HEADER,*PIMAGE_DOS_HEADER;  

#define IMAGE_DOS_SIGNATURE     0x5A4D          /* MZ */ 
#define IMAGE_OS2_SIGNATURE     0x454E          /* NE */
#define IMAGE_OS2_SIGNATURE_LE  0x454C          /* LE */
#define IMAGE_VXD_SIGNATURE     0x454C          /* LE */
#define IMAGE_NT_SIGNATURE      0x00004550      /* PE00 */

