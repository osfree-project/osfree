/*static char *SCCSID = "@(#)sas.h      6.2 92/01/17";*/
/*
 * static char *SCCSID = "@(#)sas.h     6.2 92/01/17"
 *
 * System Anchor Segment Data Structure
 */

#define SAS_SIG         "SAS "
#define SAS_CBSIG       4

/* Base section */
struct SAS {
    unsigned char  SAS_signature[SAS_CBSIG]; /* "SAS " */
    unsigned short SAS_tables_data;  /* offset to tables section */
    unsigned short SAS_flat_sel;     /* FLAT selector for kernel data */
    unsigned short SAS_config_data;  /* offset to configuration section */
    unsigned short SAS_dd_data;      /* offset to device driver section */
    unsigned short SAS_vm_data;      /* offset to Virtual Memory section */
    unsigned short SAS_task_data;    /* offset to Tasking section */
    unsigned short SAS_RAS_data;     /* offset to RAS section */
    unsigned short SAS_file_data;    /* offset to File System section */
    unsigned short SAS_info_data;    /* offset to infoseg section */
};

/* Protected Mode Tables section */
struct SAS_tables_section {
    unsigned short SAS_tbl_GDT;         /* selector for GDT */
    unsigned short SAS_tbl_LDT;         /* selector for LDT */
    unsigned short SAS_tbl_IDT;         /* selector for IDT */
    unsigned short SAS_tbl_GDTPOOL;     /* selector for GDTPOOL */
};

/* Configuration section */
struct SAS_config_section {
    unsigned short SAS_config_table;    /* offset for Device Configuration */
                                        /* Table (DevConfigTbl) */
};

/* Device Driver section */
struct SAS_dd_section {
    unsigned short SAS_dd_bimodal_chain; /* offset for the first bimodal */
                                         /* device driver's device header */
    unsigned short SAS_dd_real_chain;    /* offset for the address of the */
                                         /* first real mode device */
                                         /* driver's device header */
    unsigned short SAS_dd_DPB_segment;   /* selector for Drive Parameter */
                                         /* Block (DPB) segment */
    unsigned short SAS_dd_CDA_anchor_p;  /* selector for ABIOS protected */
                                         /* mode Common Data Area */
    unsigned short SAS_dd_CDA_anchor_r;  /* segment for ABIOS real mode */
                                         /* Common Data Area */
    unsigned short SAS_dd_FSC;           /* selector for FSC */
};

/* Virtual Memory Management section */
struct SAS_vm_section {
    unsigned long SAS_vm_arena;     /* Flat offset of arena records */
    unsigned long SAS_vm_object;    /* Flat offset of object records */
    unsigned long SAS_vm_context;   /* Flat offset of context records */
    unsigned long SAS_vm_krnl_mte;  /* Flat offset of kernel MTE records */
    unsigned long SAS_vm_glbl_mte;  /* Flat offset of global MTE linked list. */
                                    /* Note this field points into the chain  */
                                    /* to pick up global MTEs only. Use       */
                                    /* SAS_vm_all_mte to find all the MTEs.   */
    unsigned long SAS_vm_pft;       /* Flat offset of page frame table */
    unsigned long SAS_vm_prt;       /* Flat offset of page range table */
    unsigned long SAS_vm_swap;      /* Pointer to flat offset of swapper */
                                    /*   disk frame bit map followed by */
                                    /*   the size of the bit map in bits */
                                    /* WARNING: the bit map offset and */
                                    /*   size are volatile */
    unsigned long SAS_vm_idle_head; /* Flat offset of Idle Head */
    unsigned long SAS_vm_free_head; /* Flat offset of Free Head */
    unsigned long SAS_vm_heap_info; /* Flat offset of Heap Array */
    unsigned long SAS_vm_all_mte;   /* Flat offset of all MTEs linked list */
};

/* Tasking section */
struct SAS_task_section {
    unsigned short SAS_task_PTDA;       /* selector for current PTDA */
    unsigned long SAS_task_ptdaptrs;    /* FLAT offset for process tree head */
    unsigned long SAS_task_threadptrs;  /* FLAT address for TCB address array */
    unsigned long SAS_task_tasknumber;  /* offset for current TCB number */
    unsigned long SAS_task_threadcount; /* offset for ThreadCount */
};

/* RAS section */
struct SAS_RAS_section {
    unsigned short SAS_RAS_STDA_p;      /* selector for System Trace Data */
                                        /* Area (STDA) */
    unsigned short SAS_RAS_STDA_r;      /* segment for System Trace Data */
                                        /* Area (STDA) */
    unsigned short SAS_RAS_event_mask;  /* offset for trace event mask */
};

/* File System section */
struct SAS_file_section {
    unsigned long SAS_file_MFT; /* handle to MFT PTree */
    unsigned short SAS_file_SFT;        /* selector for System File Table */
                                        /* (SFT) segment */
    unsigned short SAS_file_VPB;        /* selector for Volume Parameter */
                                        /* Block (VPB) segment */
    unsigned short SAS_file_CDS;        /* selector for Current Directory */
                                        /* Structure (CDS) segment */
    unsigned short SAS_file_buffers;    /* selector for buffer segment */
};

/* Information Segment section */
struct SAS_info_section {
    unsigned short SAS_info_global;     /* selector for global info seg */
    unsigned long SAS_info_local;       /* address of curtask local infoseg */
    unsigned long SAS_info_localRM;     /* address of DOS task's infoseg */
    unsigned short SAS_info_CDIB;       /* selector for Codepage Data */
                                        /* Information Block (CDIB) */
};

/*
 * GDT selector used to reference the System Anchor Segment.
 * THIS IS OBSOLETE.  Should be imported by name as SAS_SEL
 * and translated to an ordinal by linking with os2?86p.lib.
 * (NOTE: This is a READ-ONLY ring 0 data segment)
 */
/* ASM
SAS_selector                  equ 70h
*/
