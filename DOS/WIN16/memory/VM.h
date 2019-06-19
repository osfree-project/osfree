
typedef struct _vm_map{
	struct _vm_map *next;		/* next element */

	void 	       *base;		/* where it starts */
	void 	       *limit;		/* next unavailable */

	DWORD   	type;		/* type from VirtualAlloc */	
	DWORD 		protect;	/* protect from VirtualAlloc */

	int		flag;		/* our internal flags */
} VM_MAP;

#define PAGEGRAN 	(0x1000*64)
