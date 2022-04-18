
typedef int (*mlist_filter_match)(MINFO *entry, void *data);

MINFO *filter_mlist(MINFO *mlisthead, int flags, mlist_filter_match is_match,
		    void *data)
{
    MINFO *filtered_head = NULL, *filtered_tail, *filtered_children;

    while (mlisthead != NULL) {
	if (flags & FILTER_MLIST_SEARCH_CHILDREN) {
	    filtered_children
		= filter_mlist(mlisthead->first_child, flags,
			       is_match, data);
	} else {
	    filtered_children = NULL;
	}
	/*
	 * filtered_children will be non-NULL if one or more of the children
	 * matches the filter and FILTER_MLIST_SEARCH_CHILDREN was specified,
	 * in which case we need to include the MINFO even if it doesn't
	 * match the filter itself.
	 */
	if (is_match(mlisthead, data) || filtered_children != NULL) {
	    /*
	     * Duplicate the MINFO element and put it on the end of the
	     * filtered list.
	     */
	    if (filtered_head != NULL) {
		filtered_tail->next = minfo_dup(mlisthead);
		filtered_tail = filtered_tail->next;
	    } else {
		filtered_head = filtered_tail = minfo_dup(mlisthead);
	    }
	    if (!is_match(mlisthead, data)) {
		/*
		 * In this case we must have flags &
		 * FILTER_MLIST_SEARCH_CHILDREN.
		 */
		filtered_tail->first_child = filtered_children;
	    } else if ((flags & FILTER_MLIST_EXPANDED) == 0) {
		/* filter the children too */
		filtered_tail->first_child
		    = filter_mlist(filtered_tail->first_child, flags,
				   is_match, data);
	    }
	}
	mlisthead = mlisthead->next;
    }

    if (filtered_head == NULL) {
	/*
	 * No entries match the given module name.
	 */
	return NULL;
    }

    /*
     * The 'next' field will have whatever value the MINFO it was duplicated
     * from had, which wouldn't be NULL unless it was the last entry in the
     * original list.
     */
    filtered_tail->next = NULL;

    return filtered_head;
}

int filter_by_module_name(MINFO *entry, void *data)
{
    return (strcmpi(entry->name, (char *) data) == 0);
}

/*
 * Filter for /MODULE SYSTEM
 */
int filter_system(MINFO *entry, void *data)
{
    return (entry->type == MT_SYSCODE ||
	    entry->type == MT_SYSDATA ||
	    entry->type == MT_KERNEL);
}

/*
 * Filter for /U
 */
int filter_upper(MINFO *entry, void *data)
{
    return (entry->type != MT_NONE
	    && (entry->type < MT_ENV
		|| entry->type == MT_DEVICE));
}

#define dos_in_hma() (dos_in_hma_() & 0x10)

/* print n (unit: 16kb) size as 5+1 char string, for example "1,234M",
 * followed by "(.... bytes)\n"
 */
static void print_normalized_ems_size(unsigned n)
{
    if (n > 624) /* 9984 is the highest "K" value */
	convert("%5sM ", (n + 32) / 64);
    else
	convert("%5sK ", n * 16);
    convert(_(1,2,"(%s bytes)\n"), n * 16384UL);
}

static void print_normal_entry(char *text, unsigned long total,
			       unsigned long used, unsigned long free)
{
    printf("%-17s", text);
    convert("%8sK ", total);
    convert("%9sK ", used);
    convert("%9sK\n", free);
}

/*
 * There are a number of possibilities:
 *
 * - DOS is resident in and hence manging the HMA
 *   - in this case we ask DOS how much space is free in the HMA
 * - DOS is not managing the HMA
 *   - in this case an XMS driver (e.g. HIMEM) may be managing it and we
 *     can find out about it by trying to allocate the HMA
 *     - in this case the HMA may already be in use in which case the
 *	 whole area has been allocated to a program
 *     - the XMS driver may not implement HMA
 *     - the XMS driver may see that the HMA contains a VDISK header
 *     - the XMS driver may report HMA does not exist
 *     - otherwise it is all free
 *	 - there may be a minimum size for TSRs to be allowed HMA
 *   - otherwise (no XMS driver) HMA is not available
 */
static void show_hma_info(int show_hma_free)
{
    unsigned int high, low, test;

    if (dos_in_hma()) {
	if (show_hma_free) {
	    unsigned int hma_free = dos_hma_free();

	    printf("%-38s%3uK", _(2,12,"Available space in High Memory Area"),
		   round_kb(hma_free));
	    convert(_(1,3," (%7s bytes)\n"), hma_free);
	}
	printf(_(2,11,"%s is resident in the high memory area.\n"), get_os());
    } else {
	if (show_hma_free) {
	    if (xms_available() == XMS_AVAILABLE_RESULT) {
		ulong result;

		xms_drv = get_xms_drv();
		/*
		 * Try requesting a small amount of memory so as to
		 * simultaneoulsy check if HMA is available and whether HMAMIN
		 * is set.
		 */
		result = xms_hma_request(1);
		if (XMS_HMA_AX(result) == XMS_HMA_AX_OK) {
		    printf(_(2,13,"HMA is available via the XMS driver\n"));
		} else {
		    switch (XMS_HMA_BL(result))	{
		    case XMS_HMA_BL_NOT_IMPL:
			printf(_(2,14,
"HMA is not available via the XMS driver: not implemented by the driver\n"));
			break;
		    case XMS_HMA_BL_VDISK:
			printf(_(2,15,
"HMA is not available via the XMS driver: a VDISK device is present\n"));
			break;
		    case XMS_HMA_BL_NOT_EXIST:
			printf(_(2,16,
"HMA is not available via the XMS driver: HMA does not exist\n"));
			break;
		    case XMS_HMA_BL_IN_USE:
			printf(_(2,17,
"HMA is not available via the XMS driver: HMA already in use\n"));
			break;
		    case XMS_HMA_BL_HMAMIN:
			/*
			 * HMA is available but HMAMIN is set, so try to
			 * determine what HMAMIN is set to.
			 */
			low = 0;
			high = XMS_HMA_SIZE + 1;
			while (high - low > 1) {
			    test = ((ulong) high + (ulong) low) / 2;
#ifdef DEBUG
			    if (dbghmamin) {
				printf("HMAMIN binary search: low=%5u "
				       "high=%5u test=%5u\n", low, high, test);
			    }
#endif
			    result = xms_hma_request(test);
			    if (XMS_HMA_AX(result) != XMS_HMA_AX_OK) {
				low = test;
			    } else {
				high = test;
				result = xms_hma_release();
				if (XMS_HMA_AX(result) != XMS_HMA_AX_FAILED) {
				    fatal(_(0,7,
"Fatal error: failed to free HMA, error code %02Xh\n"), XMS_HMA_BL(result));
				}
			    }
			}
			if (high == XMS_HMA_SIZE + 1) {
			    printf(_(2,18,
"HMA is not available via the XMS driver: HMAMIN is larger than HMA\n"));
			} else {
			    printf(_(2,19,
"HMA is available via the XMS driver, minimum TSR size (HMAMIN): %u bytes\n"),
				   high);
			}
			break;
		    default:
			printf(_(2,20,
"HMA is not available via the XMS driver: unknown error %02Xh\n"),
			       XMS_HMA_BL(result));
		    }
		}
	    } else {
		printf(_(2,21,
"HMA is not available as no XMS driver is loaded\n"));
	    }
	}
    }
}

static void int_15_info(void)
{
    ulong result = get_ext_mem_size();

    if (GET_EXT_MEM_SIZE_OK(result)) {
	printf("%-36s", _(2,22,"Memory accessible using Int 15h"));
	convert("%5sK", round_kb(BYTES_PER_KB * GET_EXT_MEM_SIZE_VALUE(result)));
	convert(_(1,3," (%7s bytes)\n"),
		(ulong) GET_EXT_MEM_SIZE_VALUE(result) * BYTES_PER_KB);
    } else {
	printf(_(2,23,"Memory is not accessible using Int 15h (code %02h)\n"),
	       GET_EXT_MEM_SIZE_ERROR(result));
    }
}

static void normal_list(unsigned memfree, UPPERINFO *upper, int show_hma_free,
			int show_int_15)
{
    unsigned memory, memused, largest_executable, reserved;
    unsigned umbfree = 0, umbtotal = 0;
    unsigned long xms_total_k, xms_free_k;
    XMSINFO *xms;
    EMSINFO *ems;

#ifdef DEBUG
    if (dbgcpu)	{
	printf("check_ems\n");
    }
#endif
    ems=check_ems();
#ifdef DEBUG
    if (dbgcpu)	{
	printf("ems=0x%04X\n",ems);
	printf("check_xms\n");
    }
#endif
    xms=check_xms();
#ifdef DEBUG
    if (dbgcpu)	{
	printf("xms=0x%04X\n",xms);
	printf("biosmemory\n");
    }
#endif
    memory=biosmemory();
    memfree=round_seg_kb(memfree);
    memused=memory - memfree;
    printf("\n");
    printf(_(2,0,"Memory Type         Total      Used       Free\n"));
    printf(	 "----------------  --------   --------   --------\n");
    print_normal_entry(_(2,1,"Conventional"), memory, memused, memfree);
    if (upper) {
	umbfree=round_seg_kb(upper->free);
	umbtotal=round_seg_kb(upper->total);
    }
    print_normal_entry(_(2,2,"Upper"), umbtotal, umbtotal-umbfree, umbfree);
    reserved = 1024 - memory - umbtotal;
    print_normal_entry(_(2,3,"Reserved"), reserved, reserved, 0);
    xms_total_k = round_kb(xms->total);
    xms_free_k = round_kb(xms->free);
    print_normal_entry(_(2,4,"Extended (XMS)"), xms_total_k,
        xms_total_k - xms_free_k, xms_free_k);
    printf(      "----------------  --------   --------   --------\n");
    print_normal_entry(_(2,5,"Total memory"), 1024 + xms_total_k,
		       1024 - memfree - umbfree + xms_total_k - xms_free_k,
		       memfree + umbfree + xms_free_k);
    printf("\n");
    print_normal_entry(_(2,6,"Total under 1 MB"), 1024 - reserved,
	   memused + umbtotal - umbfree, memfree + umbfree);
    printf("\n");
    if (ems != NULL) {
	printf("%-36s",_(2,7,"Total Expanded (EMS)"));
	print_normalized_ems_size(ems->size);
	printf("%-36s",_(2,8,"Free Expanded (EMS)"));
	print_normalized_ems_size(ems->free);
	printf("\n");
    }

    /*
     * We only show the amount of memory available via INT 15 if /DEBUG
     * was specified to mimic MS-DOS.
     */
    if (show_int_15) {
	int_15_info();
    }

    largest_executable = mcb_largest();

    printf("%-38s%3uK", _(2,9,"Largest executable program size"), round_seg_kb(largest_executable));
    convert(_(1,3," (%7s bytes)\n"), (ulong)largest_executable
	    * CONV_BYTES_PER_PAGE);
/*
 * FIXME: this is inconsistent with MS-DOS 6.22 which shows 0K if no
 * UMBs; we appear to show nothing if no UMBs.
 */
    if (upper != NULL) {
	printf("%-38s%3uK", _(2,10,"Largest free upper memory block"), round_seg_kb(upper->largest));
	convert(_(1,3," (%7s bytes)\n"), (ulong)upper->largest
		* CONV_BYTES_PER_PAGE);
    }

    show_hma_info(show_hma_free);
}

/*
 * Minimum (top) level is 0, and maximum is 2 when we have a device under a
 * child memory block.
 */
#define MAX_LEVEL 2

/*
 * Number of characters to indent by per level.
 */
#define INDENT_PER_LEVEL 2

/*
 * Maximum number of characters of indent.
 */
#define MAX_INDENT_SIZE (MAX_LEVEL * INDENT_PER_LEVEL)

static void indent_setup(char *pre_indent, char *post_indent,
			 unsigned int level)
{
/* FIXME: assert(level <= MAX_LEVEL);*/
    /* fill buffers with the maximum number of space characters we might
       need */
    memset(pre_indent, ' ', MAX_INDENT_SIZE);
    memset(post_indent, ' ', MAX_INDENT_SIZE);
    /* put the terminating NUL character at the right location */
    pre_indent[level * INDENT_PER_LEVEL] = '\0';
    post_indent[(MAX_LEVEL - level) * INDENT_PER_LEVEL] = '\0';
}

/*
 * Print the specified value in conventional memory pages (of 16
 * bytes) as bytes and kilobytes, e.g "16,384 (16K)".
 *
 * format must be valid for convert() which means it must include a
 * "%s".  On top of the width of that string will be an additional 8
 * characters that are used to show the value in kilobytes, so for a
 * value less than 1000K there will be two spaces of leading padding,
 * e.g. "  (999K)".
 *
 * e.g. print_classify_value("%7s", 999999)
 *	results in: "999,999  (999K)"
 *		    \_____/\______/
 *		       /       \
 *  "%7s": 7 characters always 8 characters wide
 *
 * You should therefore typically pass at least "%7s" and allow 7+8=15
 * characters to allow enough room for a normal conventional memory
 * block's size to be displayed.
 */
static void print_classify_value(const char *format, unsigned n)
{
    char kbuf[8];
    convert(format, n*CONV_BYTES_PER_PAGE);
    sprintf(kbuf, "(%uK)", round_seg_kb(n));
    printf("%8s", kbuf);
}

static void print_minfo_full(MINFO *entry, unsigned int level)
{
    char pre_indent[MAX_INDENT_SIZE + 1];
    char post_indent[MAX_INDENT_SIZE + 1];

    indent_setup(pre_indent, post_indent, level);
#if 0
    printf("  %04X%9lu   %s",
	   entry->seg, (ulong)entry->size * CONV_BYTES_PER_PAGE,
	   pre_indent);
#endif
    printf("  %04X", entry->seg);
    print_classify_value("%11s", (ulong)entry->size);
    printf("  %s", pre_indent);
    entry->print_name(entry);
    printf("%s  ", post_indent);
    entry->print_type(entry);
    printf("\n");
}

static void print_minfo_free(MINFO *entry, unsigned int level)
{
    printf("  %04X", entry->seg);
    print_classify_value("%11s", (ulong)entry->size);
    printf("\n");
}

static void print_devinfo_full(DEVINFO *entry, unsigned int level)
{
    char pre_indent[MAX_INDENT_SIZE + 1];
    char post_indent[MAX_INDENT_SIZE + 1];

    indent_setup(pre_indent, post_indent, level);

/* FIXME: make sure entry->minfo != NULL */

    printf("                             %s%-8s%s",
	   pre_indent, entry->devname, post_indent);
    if (entry->minfo->type == MT_KERNEL) {
	printf(_(4,8,"system device driver\n"));
    } else {
	printf(_(4,9,"installed DEVICE=%s\n"), entry->minfo->name);
    }
}

static void print_classify_entry(char *name, unsigned total_conv, unsigned total_umb)
{
    printf("  %-9s", name);
    print_classify_value("%9s", total_conv + total_umb);
    print_classify_value("%11s", total_conv);
    print_classify_value("%11s", total_umb);
    printf("\n");
}

/*
 * Recursively classify items that belong to "SYSTEM".
 */
static void classify_system(MINFO *ml, unsigned *total_conv,
			    unsigned *total_umb)
{
    for (;ml!=NULL;ml=ml->next)	{
	if (ml->owner == _psp || ml->type == MT_DOSDATA
	    || ml->type == MT_RESERVED)
	    /*
	     * We ignore our own block and DOSDATA or RESERVED blocks.
	     */
	    ml->classified = 1;
	else if (ml->type == MT_SYSCODE || ml->type == MT_SYSDATA
		 || ml->type == MT_KERNEL || ml->type == MT_IVT
		 || ml->type == MT_BDA || ml->type >= MT_DEVICE) {
	    int size = ml->size + 1;
	    /*
	     * Device drivers come off our total as they are already included
	     * in their parent MINFO's size but we don't actually include them
	     * in "SYSTEM".
	     */
	    if (ml->type == MT_DEVICE)
		size = -(size - 1);
	    else
		ml->classified = 1;
	    if (ml->seg < last_conv_seg)
		*total_conv += size;
	    else
		*total_umb += size;
	    if (ml->first_child != NULL)
		classify_system(ml->first_child, total_conv, total_umb);
	}
    }
}

static void classify_modules(MINFO *ml)
{
    MINFO *ml2;
    unsigned total_conv, total_umb;

    for (;ml!=NULL;ml=ml->next)	{
	if (ml->type > MT_FREE && !ml->classified) {
	    total_conv = total_umb = 0;
	    for (ml2 = ml; ml2 != NULL; ml2 = ml2->next) {
		if (!ml2->classified && ml2->type > MT_FREE &&
		    ml2->owner == ml->owner) {
		    ml2->classified = 1;
		    if (ml2->seg < last_conv_seg)
			total_conv += ml2->size + 1;
		    else
			total_umb += ml2->size + 1;
		}
	    }
	    print_classify_entry(ml->name, total_conv, total_umb);
	}
	if (ml->first_child != NULL)
	    classify_modules(ml->first_child);
    }
}

static void classify_list(unsigned convmemfree, unsigned umbmemfree)
{
    unsigned total_conv, total_umb;

    printf(_(4,0,
"\nModules using memory below 1 MB:\n\n"));
    printf(_(4,1,
  "  Name            Total          Conventional       Upper Memory\n"));
    printf(
  "  --------  ----------------   ----------------   ----------------\n");
    /* figure out code used by "SYSTEM" */
    total_conv = 0; total_umb = 0;
    classify_system(make_mcb_list(NULL), &total_conv, &total_umb);
    print_classify_entry(_(4,2,"SYSTEM"), total_conv, total_umb);
    /* generate output for other modules */
    classify_modules(make_mcb_list(NULL));
    print_classify_entry(_(4,3,"Free"), convmemfree, umbmemfree);
}

typedef void (*print_header_t)(memory_t memory_type);
typedef void (*print_footer_t)(MINFO *mlisthead);
typedef void (*print_minfo_t)(MINFO *entry, unsigned int level);
typedef void (*print_devinfo_t)(DEVINFO *entry, unsigned int level);

static void generic_list(MINFO *ml, unsigned int level,
			 print_minfo_t print_minfo,
			 print_devinfo_t print_devinfo)
{
    DEVINFO *dl;

    for (;ml!=NULL;ml=ml->next)	{
	if (print_minfo) {
	    print_minfo(ml, level);
	}
	if (print_devinfo) {
	    for (dl=ml->first_dev;dl!=NULL;dl=dl->next_in_minfo) {
		if (print_devinfo != NULL) {
		    print_devinfo(dl, level);
		}
	    }
	}
	if (ml->first_child) {
	    generic_list(ml->first_child, level + 1,
			 print_minfo, print_devinfo);
	}
    }
}

static void print_full_header(void)
{
    printf(_(4,4,"\nSegment        Total           Name           Type\n"));
    printf(	   "-------  ----------------  ------------  -------------\n");
}

static void print_free_header(void)
{
    printf(_(4,6,"\nSegment        Total\n"));
    printf(	   "-------  ----------------\n");
}

static void print_full_footer(ulong total)
{
    printf(	   "         ----------------\n");
    printf(_(4,7,  "Total:"));
    print_classify_value("%11s", total);
    printf("\n");
}

/*
 * Similar to /DEBUG, but doesn't show devices, environment blocks,
 * data blocks, and some other things.
 */
/*
 * FIXME: give complete list of what is skipped in this comment and in
 * the user documentation.
 */
/* FIXME: Can this be renamed from UPPER to U<something else>?	It's
 * not really got anything to do with upper memory.  Currently the
 * user can only access this option via /U and not via /UPPER in
 * anticipation of finding a better name.
 */
static void upper_list(void)
{
    MINFO *mlisthead = make_mcb_list(NULL);

    /* ignore the return value, as we get the DEVINFO entries from mlisthead */
    (void) make_dev_list(mlisthead);

    /*
     * Filter the list to just the entries with the given name.
     */
    mlisthead = filter_mlist(mlisthead, FILTER_MLIST_NO_FLAGS,
			     filter_upper, NULL);

    print_full_header();

    generic_list(mlisthead, 0, print_minfo_full, NULL);
}

static void full_list(void)
{
    print_full_header();
    generic_list(make_mcb_list(NULL), 0, print_minfo_full, NULL);
}

static void device_list(void)
{
    DEVINFO *dl;
    char *progname;

    printf(_(4,5,"\n   Address     Attr     Name       Program\n"));
    printf(	   " -----------  ------ ----------  ----------\n");
	     /*	 XXXX:XXXX    XXXX   XXXXXXXX	 XXXXXXXX */
    for (dl=make_dev_list(make_mcb_list(NULL));dl!=NULL;dl=dl->next) {
	progname = (dl->minfo != NULL) ? dl->minfo->name : "";
	printf("  %p    %04X   %-8s    %-8s\n", dl->addr, dl->attr,
		dl->devname, progname);
    }
}

/*
 * For each memory type, display a header, use generic_list to display
 * that part of the split list, then display a footer.	The header and
 * footer are both optional.  The function used to print the header is
 * passed the memory type (conventional or upper) and the function
 * used to print the footer is passed the MINFO list so it can
 * calculate a memory total for display in the footer.	A blank line
 * is inserted between memory types.  If print_empty is supplied, it
 * is called when one of the split lists is empty instead of any of
 * the other functions.
 */
static void generic_split_list(MINFO **split,
			       print_header_t print_header,
			       print_footer_t print_footer,
			       print_header_t print_empty,
			       print_minfo_t print_minfo,
			       print_devinfo_t print_devinfo)
{
    memory_t memory_type;

    for (memory_type = memory_conv; memory_type < memory_num_types;
	 memory_type++)	{
	if (memory_type > memory_conv) { /* if not the first type */
	    printf("\n");
	}
	if (split[memory_type] == NULL && print_empty != NULL) {
	    print_empty(memory_type);
	} else {
	    if (print_header != NULL) {
		print_header(memory_type);
	    }
	    generic_list(split[memory_type], 0,
			 print_minfo,
			 print_devinfo);
	    if (print_footer != NULL) {
		print_footer(split[memory_type]);
	    }
	}
    }
}

static void print_full_header_with_type(memory_t memory_type)
{
    printf(_(4, 10, "%s Memory Detail:\n"), memory_typename(memory_type));
    print_full_header();
}

static void print_full_empty(memory_t memory_type)
{
    printf(_(1, 9, "%s Memory is not accessible\n"), memory_typename(memory_type));
}

static void debug_list(void)
{
    MINFO *ml;

    ml = make_mcb_list(NULL);
    /* ignore the return value, as we get the DEVINFO entries from ml */
    (void) make_dev_list(ml);

    generic_split_list(split_mlist_conv_upper(ml),
		       print_full_header_with_type, NULL, print_full_empty,
		       print_minfo_full, print_devinfo_full);
}

static void ems_list(void)
{
    EMSINFO *ems;
    ushort i;
    static char handlename_other[9];
    char *handlename, *handlename_sys;
    static char format[] = "  %-20s";

    ems=check_ems();
    if (ems==NULL)
    {
	printf(_(5,1,"  EMS driver not installed in system.\n"));
    }
    else
    {
        printf("\n");
	printf(format, _(5,2,"EMS driver version"));
	printf("%1u.%1u\n", ems->vermajor, ems->verminor);
	if (ems->frame)	{
	    printf(format, _(5,3,"EMS page frame"));
	    printf("%04X\n", ems->frame);
	}
	printf(format, _(5,4,"Total EMS memory"));
	convert(_(1,1,"%s bytes\n"), ems->size * EMS_BYTES_PER_PAGE);
	printf(format, _(5,5,"Free EMS memory"));
	convert(_(1,1,"%s bytes\n"), ems->free * EMS_BYTES_PER_PAGE);
	printf(format, _(5,6,"Total handles"));
	printf("%u\n", ems->totalhandle);
	printf(format, _(5,7,"Free handles"));
	printf("%u\n", ems->freehandle);

	printf(_(5,8,"\n  Handle   Pages    Size       Name\n"));
	printf(	       " -------- ------  --------   ----------\n");
	handlename_sys = _(4,2,"SYSTEM");
	for (i=0;i<ems->usehandle;i++)
	{
	    handlename = handlename_sys;
	    if (ems->vermajor >= 4)
	    {
		if (ems->handles[i].handle != 0)
		{
		    handlename = handlename_other;
		    memset(handlename, 0, 9);
		    ems_get_handle_name(ems->handles[i].handle, handlename);
		    check_name(handlename, handlename, 8);
		}
	    }

	    printf("%9u%7u%10lu%11s\n", ems->handles[i].handle,
		   ems->handles[i].pages, (ulong)ems->handles[i].pages
		   * EMS_BYTES_PER_PAGE, handlename);
	}
    }
}

static long xms_common(unsigned char al, signed char bl, ushort result)
{
     return (al != 0 ? result : (long)bl << 24);
}

static long xms_handleinfo(ushort handle)
{
     /* Get handle information */
     ulong bx_ax = call_xms_driver_bx_ax(0xe, handle);
     return xms_common((uchar)bx_ax, (uchar)(bx_ax >> 16),
		       (ushort)(bx_ax >> 16));
}

/* Get handle size information */

#define xms_handlesize(handle) ((ushort)(call_xms_driver_dx_bl_al(0xe, handle) >> 16))

static long xms_alloc(ushort kbytes)
{
     /* "Allocate extended memory block" */
     ulong dx_bl_al = call_xms_driver_dx_bl_al(0x9, kbytes);
     return xms_common((uchar)dx_bl_al,
		       (uchar)(dx_bl_al>>8), (ushort)(dx_bl_al>>16));
}

static void xms_free(ushort handle)
{
     /* "Free extended memory block" */
     call_xms_driver_dx_bl_al(0xa, handle);
}

static long xms_query_a20(void)
{
    ulong bx_ax = call_xms_driver_bx_ax(0x7, 0);
    return (((uchar)(bx_ax>>16))!=0 ? ((long)bx_ax<<8) : (uchar)(bx_ax));
}

static void xms_list(void)
{
    UPPERINFO *upper;
    XMSINFO *xms;
    XMS_HANDLE *handle = NULL;
    ushort i;
    long lhandle;
    static char format[] = "%-26s";
    XMS_HANDLE_TABLE far* xmsHanTab;

    xms = check_xms();

    if (xms_drv==NULL)
	{
	printf(_(6,0,"XMS driver not installed in system.\n"));
	return;
	}

    printf(_(6,1,"\nTesting XMS memory ...\n"));

    lhandle = xms_query_a20();
    if (lhandle < 0) {
	printf(_(6,2,"XMS INTERNAL ERROR.\n"));
	return;
    }
    xms->a20 = lhandle & 0xff;

    /*
    // 01/4/27 tom + alain
    //
    // although the 'old' method to search the handle table should be OK,
    // it crashes somehow and for unknown reason under Win98. So, a 'new' method to
    // query all handles was implemented, using INT 2F, AX=4309
       test support for INT2F AX=4309 first */
    xmsHanTab = get_xmsHanTab();
    if (xmsHanTab != NULL    && /* test returned OK */
	xmsHanTab->sizeOfDesc == sizeof(XMS_HANDLE_DESCRIPTOR)) /* assert correct size */
    {
	XMS_HANDLE_DESCRIPTOR far* descr = xmsHanTab->xmsHandleDescr;

	printf(_(6,3,"INT 2F AX=4309 supported\n"));

	for (i=0;i<xmsHanTab->numbOfHandles;i++,descr++)
	{
	    if (descr->flag != 0x01 && /* not free */
		descr->xmsBlkSize != 0)	  /* and takes memory */
	    {
		if (handle==NULL)
		    xms->handles=handle=xmalloc(sizeof(XMS_HANDLE));
		else {
		    handle->next=xmalloc(sizeof(XMS_HANDLE));
		    handle=handle->next;
		}
		handle->handle=FP_OFF(descr);
		handle->size=descr->xmsBlkSize*1024UL;
		handle->locks=descr->locks;
		handle->next=NULL;
	    }
	}
    }
    else
    {
	/* old method */
	/* query all handles 0..0xffff */

	for (i=0;i<65535u;i++)
	{
	    /* Get handle information */
	    if ((lhandle = xms_handleinfo(i)) >= 0) {
		uchar free_handles_tmp = lhandle & 0xff;
		if (handle==NULL)
		    xms->handles=handle=xcalloc(1, sizeof(XMS_HANDLE));
		else {
		    handle->next=xcalloc(1, sizeof(XMS_HANDLE));
		    handle=handle->next;
		}
		handle->handle=i;
		if (xms->vermajor >= 3 && xms->is_386)
		    handle->size=xms_exthandlesize(i);
		if (!handle->size)
		    handle->size=xms_handlesize(i);
		handle->size *= 1024UL;
		handle->locks=lhandle >> 8;
		handle->next=NULL;
		if (xms->freehandle < free_handles_tmp)
		{
		    xms->freehandle = free_handles_tmp;
		}
	    }
	}
    }

    /* First try to get a handle of our own. */
    /* First we try 1kB. I'm not sure if XMS driver
       must support a zero sized allocate. */
    /* "Allocate extended memory block" */
    if (((lhandle = xms_alloc(1)) >= 0) ||
	/* Now try a zero sized allocate just in case there was no free memory. */
	((lhandle = xms_alloc(0)) >= 0))
    {
	long info = xms_handleinfo((ushort)lhandle);
	/* else nothing worked out. Use whatever we got from the loop above. */
	/* We can't do much if the free call fails, so it ends here. */
	if (info >= 0)
	    /* Hey! We got some info. Put it in a safe place. */
	    xms->freehandle = (uchar)info + 1;
	/* Add one for the handle we have allocated. */
	xms_free((ushort)lhandle);
    }

    printf(format, _(6,4,"XMS version"));
    printf("%u.%02u \t\t", xms->vermajor, xms->verminor);
    printf(format, _(6,5,"XMS driver version"));
    printf("%u.%02u\n", xms->drv_vermajor, xms->drv_verminor);
    printf(format, _(6,6,"HMA"));
    printf("%s \t", (xms->hma) ? _(6,7,"exists") : _(6,8,"does not exist"));
    printf(format, _(6,9,"A20 line"));
    printf("%s\n", (xms->a20) ? _(6,10,"enabled") : _(6,11,"disabled"));
    printf(format, _(6,12,"Free XMS memory"));
    convert(_(1,1,"%s bytes\n"), xms->free);
    printf(format, _(6,13,"Largest free XMS block"));
    convert(_(1,1,"%s bytes\n"), xms->largest);
    printf(format, _(6,14,"Free handles"));
    printf("%u\n", xms->freehandle);
    printf("\n");
    if (xms->handles != NULL)
    {
	printf(_(6,15," Block   Handle     Size     Locks\n"));
	printf(	      "------- --------  --------  -------\n");
	for (i=0, handle=xms->handles;handle!=NULL;handle=handle->next, i++)
	    printf("%7u %8u  %8lu  %7u\n", i, handle->handle,
		   handle->size, handle->locks);

    }

    upper = check_upper(make_mcb_list(NULL));
    if (upper != NULL)
    {
	printf(format, _(6,16,"Free upper memory"));
	convert(_(1,1,"%s bytes\n"), upper->free*CONV_BYTES_PER_PAGE);
	printf(format, _(6,17,"Largest upper block"));
	convert(_(1,1,"%s bytes\n"), upper->largest*CONV_BYTES_PER_PAGE);
    }
    else
    {
	printf(_(6,18,"Upper memory not available\n"));
    }

}

ulong total_mem(MINFO *mlist)
{
    ulong result = 0;

    while (mlist != NULL) {
	result += mlist->size;
	mlist = mlist->next;
    }
    return result;
}

void module_list(char *module_name)
{
    MINFO *mlisthead = make_mcb_list(NULL);

    /*
     * The parser will not indicate /MODULE was specified without setting
     * module_name.
     */
/*FIXME: assert(module_name != NULL);*/

    /* ignore the return value, as we get the DEVINFO entries from mlisthead */
    (void) make_dev_list(mlisthead);

    if (strcmpi(module_name, "SYSTEM") == 0) {
	/*
	 * Filter the list to entries related to the OS which we would
	 * show as "IO" or "DOS" and show under "SYSTEM" in /CLASSIFY.
	 */
	mlisthead = filter_mlist(mlisthead,
				 FILTER_MLIST_EXPANDED |
				 FILTER_MLIST_SEARCH_CHILDREN,
				 filter_system, NULL);
    } else {
	/*
	 * Filter the list to just the entries with the given name.
	 */
	mlisthead = filter_mlist(mlisthead,
				 FILTER_MLIST_EXPANDED |
				 FILTER_MLIST_SEARCH_CHILDREN,
				 filter_by_module_name, module_name);
    }

    if (mlisthead == NULL) {
	printf(_(1,7,"%s is not currently in memory.\n"), module_name);
	exit(2);
    }

    printf(_(1,8,"%s is using the following memory:\n"), module_name);
    print_full_header();

    generic_list(mlisthead, 0, print_minfo_full, print_devinfo_full);

    print_full_footer(total_mem(mlisthead));

/*
 * FIXME: use split list (conventional/upper) for /MODULE?  We should
 * probably include a grand total as well as the conventional total
 * and upper total if we do split it.
 */
}

static void print_free_header_with_type(memory_t memory_type)
{
    printf(_(4, 11, "Free %s Memory:\n"), memory_typename(memory_type));
    print_free_header();
}

static void print_free_empty(memory_t memory_type)
{
    printf(_(1, 6, "No %s Memory is free\n"), memory_typename(memory_type));
}

static void print_free_footer(MINFO *mlisthead)
{
    print_full_footer(total_mem(mlisthead));
}

void free_list(void)
{
    MINFO *mlisthead = make_mcb_list(NULL);

    /* ignore the return value, as we get the DEVINFO entries from ml */
    (void) make_dev_list(mlisthead);

    /*
     * Filter the list to just the entries with the given name.	 Whether or not
     * we pass FILTER_MLIST_EXPANDED is irrelevant as free blocks don't have
     * sub-blocks.
     */
    mlisthead = filter_mlist(mlisthead, FILTER_MLIST_NO_FLAGS,
			     filter_free, NULL);

    /*
     * Pass NULL for print_devinfo as we won't be printing any devices
     * - a free block won't have any devices linked off it.
     */
    generic_split_list(split_mlist_conv_upper(mlisthead),
		       print_free_header_with_type,
		       print_free_footer, print_free_empty,
		       print_minfo_free, NULL);
}

/*
 * FIXME: add support for showing a process listing/tree using this function.
 */
#if 0
void ps_list_from(MINFO *mlisthead, MINFO *root, unsigned int level)
{
    MINFO *mlist;
    char pre_indent[MAX_INDENT_SIZE + 1 + 99/*FIXME*/];
    char post_indent[MAX_INDENT_SIZE + 1 + 99/*FIXME*/];

    indent_setup(pre_indent, post_indent, level);
    for (mlist = mlisthead; mlist != NULL; mlist = mlist->next)	{
	if (mlist->type == MT_PROGRAM && mlist->caller == root)	{
	    printf("%s%s\n", pre_indent, mlist->name);
	    ps_list_from(mlisthead, mlist, level + 1);
	}
    }
}

void ps_list(void)
{
    MINFO *mlisthead = make_mcb_list(NULL);

    ps_list_from(mlisthead, NULL, 0);
}
#endif

/* function to obtain the number of lines on the screen...added by brian reifsnyder.  */
static uchar get_font_info(void)
{
    uchar number_of_lines = *((uchar far *)MK_FP(0x40, 0x84));
    if (number_of_lines == 0)
	number_of_lines = 25;
    else
	number_of_lines++;
    return number_of_lines;
}

int is_switch_char(char c)
{
    return (c == '-' || c == '/');
}

int is_space_char(char c)
{
    /*
     * Don't need to actually list ' ' since the C runtime takes care of
     * splitting up argv[] based on spaces.
     */
    return (c == ':');
}

#define NO_SWITCH_CHAR '\0'

#define F_HELP	     0x00000001UL
#define F_DEVICE     0x00000002UL
#define F_EMS	     0x00000004UL
#define F_FULL	     0x00000008UL
#define F_UPPER	     0x00000010UL
#define F_XMS	     0x00000020UL
#define F_PAGE	     0x00000040UL
#define F_CLASSIFY   0x00000080UL
#define F_DEBUG	     0x00000100UL
#define F_MODULE     0x00000200UL
#define F_FREE	     0x00000400UL
#define F_ALL	     0x00000800UL
#define F_NOSUMMARY  0x00001000UL
#define F_SUMMARY    0x00002000UL
#define F_DBGDEVADDR 0x00004000UL
#define F_DBGHMAMIN  0x00008000UL
#define F_OLD	     0x00010000UL
#define F_F	     0x00020000UL
#define F_D	     0x00040000UL
#define F_DBGCPU     0x00080000UL

typedef unsigned long opt_flag_t;

/*
 * Mask for values of opt_flag_t which cause additional output (on top of the
 * normal summary) to be produced.  If the user specifies /NOSUMMARY without
 * one of these flags it is considered an error because absolutley no output
 * would be generated.
 */
#define OPT_FLAG_MASK_OUTPUT (F_HELP | F_DEVICE | F_EMS | F_FULL |	\
			      F_UPPER | F_XMS | F_CLASSIFY | F_DEBUG |	\
			      F_MODULE | F_FREE)

typedef struct {
    char *s;
    opt_flag_t flag;
    char **value;
} opt_t;

typedef enum {
    GET_OPTS_STATE_SWITCH,
    GET_OPTS_STATE_VALUE
} get_opts_state_t;

/*
 * Each element of argv[] may contain one or more substrings.  Each of
 * these substrings may be an option, which starts with a character
 * for which is_switch_char() returns TRUE, a value which corresponds
 * to the option before it, or a sequence of whitespace-equivalent
 * characters for which is_space_char() returns TRUE.
 *
 * The beginning of an argv[] element is always the beginning of a
 * substring and the end of an argv[] element is always the end of a
 * substring.  A substring ends before a switch character, so an
 * argv[] element of "/FOO/BAR" would be parsed as two substrings
 * "/FOO" and "/BAR".  A substring consisting of whitespace-equivalent
 * characters serves only to split two substrings, so "/FOO:BAR" or
 * "/FOO::BAR", etc. would be parsed as substrings "/FOO", a sequence
 * of colons which are ignored, and "BAR".
 *
 * If a substring begins with a switch character, it is interpreted as
 * an option.  The part of the substring after the leading switch
 * character is compared to the 's' member of each entry in the array
 * 'opts'.  If the substring is a complete case-insensitive match for
 * exactly one 's' member then the corresponding 'flag' will be set in
 * the return value of the function.  If it is an exact match for more
 * than one 's' member then an error message is shown; you should not
 * set up 'opts' such that there are multiple array members with the
 * same 's' value.
 *
 * If the substring isn't an exact match for any 's' member, the
 * following holds: If the substring is an initial substring, i.e. a
 * prefix, of exactly one 's' member, e.g. the substring after removal
 * of the switch characer is "F" and an 's' member is equal to "FOO",
 * then the corresponding 'flag' will be set in the return value of
 * the function.  If it is a prefix for more than one 's' member,
 * e.g. the substring is "B" and it is a substring of both "BAR" and
 * "BAZ", then the user will be shown an error message indicating that
 * the option they specified is ambiguous and matches two or more
 * possible options.  The user needs to specify more characters of the
 * option name to avoid this.
 *
 * If the substring is not a prefix of any 's' member, then the
 * following holds: if the value of 's' is a prefix of the substring,
 * e.g. the substring is "FOOBAR" and 's' is "FOO", AND the
 * corresponding 'value' is not NULL (i.e. 's' is the text of an
 * option that requires a value), then the corresponding 'flag' will
 * be set in the return value of the function and the part of the
 * substring following 's' in the substring will be treated as a
 * separate substring for the purposes of setting 'value' as described
 * below.  If the multiple values of 's' are prefixes of the
 * substring, then we ignore all but the one with the longest string
 * length, e.g. if 's' values of "F" and "FOO" are seen, then if the
 * substring is "FOOBAR", we will ignore the fact that "F" is a prefix
 * of "FOOBAR" and act as if "FOO" was the only option that was a
 * prefix of "FOOBAR".	If there is more than one value of 's' that is
 * a prefix of the substring with the same length, this indicates that
 * 'opts' contains duplicate options and an error will be shown.
 *
 * If the substring is not an exact match for any 's' member, is not a
 * prefix of any 's' member, and does not have a value of 's' for an
 * option that takes a value as its prefix, then the user is given an
 * error message indicating that they specified an unrecognized
 * option.
 *
 * If the return value of the function is being set based on the
 * 'flag' member for an array member, then if the 'value' member for
 * the option is not NULL this indicates that the option requires a
 * value.  The pointer that 'value' points to will be set to point to
 * the substring.
 */
opt_flag_t get_opts (char *argv[], opt_t *opts, int opt_count)
{
    int arg_index = 1, opt_index, matched_index;
    int partial_match_count, exact_match_count;
    int prefix_match_len, new_prefix_match_len;
    char *char_ptr = argv[arg_index];
    char *switch_start, *switch_text_start, old_switch_char;
    opt_flag_t flags = 0;
    get_opts_state_t state = GET_OPTS_STATE_SWITCH;

    while (char_ptr != NULL && (flags & F_HELP)==0) {
	/*
	 * If state == GET_OPTS_STATE_SWITCH, we're at the start of a
	 * switch, which might be the start of argv[i] or somewhere
	 * inside it.  Otherwise we're at the start of a value for a
	 * switch, which is always at the start of argv[i].  */
	switch_start = char_ptr;

	/*
	 * A switch should start with a switch character.
	 */
	if (state == GET_OPTS_STATE_SWITCH) {
	    if (!is_switch_char(*char_ptr)) {
		fatal(_(0,8,"unknown option (expected a '/'): %s\n%s"),
		      switch_start, _(0,4,"Use /? for help\n"));
	    }
	    char_ptr++;
	} else { /* state == GET_OPTS_STATE_VALUE */
	    if (is_switch_char(*char_ptr)) {
		fatal(_(0,9,
"Expected a value after /%s, not another switch\n%s"), opts[matched_index].s,
		      _(0,4,"Use /? for help\n"));
	    }
	}

	/*
	 * Now get the text of the switch or value - scan for the end of the
	 * switch.
	 */
	switch_text_start = char_ptr;
	while (*char_ptr != '\0'
	       && !is_switch_char(*char_ptr)
	       && !is_space_char(*char_ptr)) {
	    char_ptr++;
	}

	/*
	 * If we're not at the end of the current argv[] element, we
	 * need to put a '\0' character at char_ptr so that string
	 * operations don't see the following argument.
	 */
	if (is_switch_char(*char_ptr) || is_space_char(*char_ptr)) {
	    old_switch_char = *char_ptr;
	    *char_ptr = '\0';
	} else {
	    old_switch_char = NO_SWITCH_CHAR;
	}

	prefix_match_len = 0;
	exact_match_count = 0;
	partial_match_count = 0;
	if (state == GET_OPTS_STATE_SWITCH) {
	    if (strlen(switch_text_start) == 0)	{
		fatal(_(0, 14,
"Invalid option '%s': you must specify at least one letter of the\n"
"option name"), switch_start);
	    }
	    strupr(switch_text_start);
#ifdef DEBUG_PARSER
	    printf("%s: argument is [%s]\n", __FUNCTION__, switch_text_start);
#endif

	    for (opt_index = 0; opt_index < opt_count; opt_index++) {
		if (strstr(opts[opt_index].s, switch_text_start)
		    == opts[opt_index].s) {
		    /*
		     * opts[opt_index].s begins with switch_text_start, so
		     * we check if we have an exact match (the string
		     * lengths are the same) or a partial one.
		     * matched_index needs to contain the index into
		     * opts[] of the matching switch, or one of them if
		     * there is ambiguity.  An exact match has precedence
		     * over a partial one, i.e. we set matched_index on an
		     * exact match regardless of whether there are any
		     * partial matches, and we only set it on a partial
		     * match if there haven't been any exact ones.
		     */
		    if (strlen(opts[opt_index].s)
			== strlen(switch_text_start)) {
			exact_match_count++;
			matched_index = opt_index;
#ifdef DEBUG_PARSER
			printf("%s: argument is exact match (number %u) for "
			       "[%s]\n", __FUNCTION__, exact_match_count,
			       opts[opt_index].s);
#endif
		    } else {
			partial_match_count++;
			if (exact_match_count == 0) {
			    matched_index = opt_index;
			}
#ifdef DEBUG_PARSER
			printf("%s: argument is partial match (number %u) for "
			       "[%s]\n", __FUNCTION__, partial_match_count,
			       opts[opt_index].s);
#endif
		    }
		} else if (strstr(switch_text_start, opts[opt_index].s)
			   == switch_text_start
			   && opts[opt_index].value != NULL) {
		    /*
		     * opts[opt_index].s does not begin with
		     * switch_text_start (nor are they equal), but
		     * switch_text_start begins with opts[opt_index].s
		     * and opts[opt_index] is an option that takes a
		     * value.  Note that we only set matched_index
		     * (i.e. treat this as actually being the option
		     * the user wanted to select) if (a) we've not
		     * seen an exact match [yet], (b) we've not seen a
		     * partial match [yet] and (c) we've not seen a
		     * longer match [yet].  If we see any of those
		     * three later, we will overwrite matched_index.
		     */
#ifdef DEBUG_PARSER
		    printf("%s: argument begins with [%s]\n", __FUNCTION__,
			   opts[opt_index].s);
#endif
		    if (exact_match_count == 0 && partial_match_count == 0) {
			new_prefix_match_len = strlen(opts[opt_index].s);
			if (new_prefix_match_len > prefix_match_len) {
			    prefix_match_len = new_prefix_match_len;
			    matched_index = opt_index;
			} else if (new_prefix_match_len == prefix_match_len) {
			    /*
			     * If there is another opts[].s which is the same
			     * length which is also a prefix of
			     * switch_text_start, we must have duplicate
			     * options.
			     */
			    fatal(_(0,10,"Internal error: option '%s' has "
				    "'%s' as a prefix\nplus another equal-"
				    "length prefix"),
				  switch_start, opts[opt_index].s);
			}
		    }
		}
	    }

	    /*
	     * Now figure out if we had ambiguity or no match at all.
	     * The normal cases are exact_match_count == 1 (in which
	     * case we don't care if there were any partial matches or
	     * prefixes) or exact_match_count == 0 &&
	     * partial_match_count == 1 (i.e. the user's input was a
	     * partial match for only one switch) or exact_match_count
	     * == 0 && partial_match_count == 0 && prefix_match_len !=
	     * 0 (i.e. the user's input had a switch as its prefix).
	     */
	    if (exact_match_count > 1) {
		/*
		 * This is a bug - there shouldn't be two options with the
		 * same text in opts[]!
		 */
		fatal(_(0,11,
"Internal error: option '%s' was an exact match for two\n"
"different switches\n"), switch_start);
	    } else if (exact_match_count == 0) {
		if (partial_match_count > 1) {
		    fatal(_(0,12,
"Error: option '%s' is ambiguous - it is a partial match for two\n"
"or more different options\n%s"), switch_start, _(0,4,"Use /? for help\n"));
		}
		if (partial_match_count == 0 && prefix_match_len == 0) {
		    fatal(_(0,5,"unknown option: %s\n%s"),
			  switch_start, _(0,4,"Use /? for help\n"));
		}
	    }
	    /*
	     * We had (a) only one exact match, (b) no exact match but
	     * one partial match, or (c) no exact match, no partial
	     * match, but one or more prefix matches (of which we
	     * would have ignored all but the longest).	 In any
	     * case, matched_index will be set.
	     */
#ifdef DEBUG_PARSER
	    printf("%s: resolved to: [%s]\n", __FUNCTION__,
		   opts[matched_index].s);
#endif
	    flags |= opts[matched_index].flag;

	    /*
	     * If value is not NULL, then on the next iteration through this
	     * loop we need to get a value for the switch.
	     */
	    if (opts[matched_index].value != NULL) {
#ifdef DEBUG_PARSER
		printf("%s: expecting a value for this switch\n",
		       __FUNCTION__);
#endif
		state = GET_OPTS_STATE_VALUE;
	    }
	} else { /* state == GET_OPTS_STATE_VALUE */
#ifdef DEBUG_PARSER
	    printf("%s: got switch value [%s]\n", __FUNCTION__, switch_start);
#endif
	    *(opts[matched_index].value) = xstrdup(switch_start);
	    state = GET_OPTS_STATE_SWITCH;
	}

	if (old_switch_char != NO_SWITCH_CHAR) {
	    /*
	     * *char_ptr was originally a switch or space character,
	     * so restore it and continue through the loop to process
	     * the next switch in this argv[] element.
	     */
	    *char_ptr = old_switch_char;
	}

	if (exact_match_count == 0 && partial_match_count == 0
	    && prefix_match_len != 0) {
	    /*
	     * We just processed an option which takes a value, and
	     * the option and value are concatenated.  char_ptr points
	     * to the first character after the value, but we need to
	     * make it point to the first character after the option
	     * (i.e. the first character of the value) so that we can
	     * parse the value in the next loop iteration.  Note that
	     * the following code which handles trailing
	     * whitespace-like characters and the end of the argv[]
	     * element will do nothing since char_ptr will not be
	     * pointing to one of those types of characters.  */
	    char_ptr = switch_text_start + prefix_match_len;
	}

	/*
	 * Skip over any whitespace-like characters that came after the flag.
	 * If we're already at the end of the string,
	 * is_space_char(*char_ptr) will return FALSE.
	 */
	while (is_space_char(*char_ptr)) {
	    char_ptr++;
	}
	/*
	 * If we're at the end of the argv[] element, possibly after skipping
	 * over whitespace-like characters, move to next argv[] element.
	 */
	if (*char_ptr == '\0') {
	    arg_index++;
	    char_ptr = argv[arg_index];
	}
    }

    /*
     * If we are still in this state, it means we failed to get the required
     * value.
     */
    if (state == GET_OPTS_STATE_VALUE) {
	fatal(_(0,13,"Expected a value after /%s\n%s"),
	      opts[matched_index].s, _(0,4,"Use /? for help\n"));
    }

    return (flags);
}

/*
 * Return the help string for the given command-line flag/option.  If
 * there is no help string for the option, NULL is returned.
 */
char *help_for_flag(opt_flag_t opt)
{
    switch (opt) {
    case F_HELP:       return (_(7, 10,
"/?           Displays this help message"));
    case F_DEVICE:     return (_(7, 6,
"/DEV[ICE]    List of device drivers currently in memory"));
    case F_EMS:	       return (_(7, 3,
"/E[MS]       Reports all information about Expanded Memory"));
    case F_FULL:       return (_(7, 4,
"/FU[LL]      Full list of memory blocks"));
    case F_UPPER:      return (_(7, 7,
"/U           List of programs in conventional and upper memory"));
    case F_XMS:	       return (_(7, 8,
"/X[MS]       Reports all information about Extended Memory"));
    case F_PAGE:       return (_(7, 9,
"/P[AGE]      Pauses after each screenful of information"));
    case F_CLASSIFY:   return (_(7, 5,
"/C[LASSIFY]  Classify modules using memory below 1 MB"));
    case F_DEBUG:      return (_(7, 11,
"/DEB[UG]     Show programs and devices in conventional and upper memory"));
    case F_MODULE:     return (_(7, 12,
"/M <name> | /MODULE <name>\n"
"             Show memory used by the given program or driver"));
    case F_FREE:       return (_(7, 13,
"/FR[EE]      Show free conventional and upper memory blocks"));
    case F_ALL:	       return (_(7, 14,
"/A[LL]       Show all details of high memory area (HMA)"));
    case F_NOSUMMARY:  return (_(7, 15,
"/N[OSUMMARY] Do not show the summary normally displayed when no other\n"
"             options are specified"));
    case F_SUMMARY:    return (_(7, 16,
"/S[UMMARY]   Negates the /NOSUMMARY option"));
    case F_OLD:	       return (_(7, 18,
"/O[LD]       Compatability with FreeDOS MEM 1.7 beta"));
    case F_D:	       return (_(7, 19,
"/D           Same as /DEBUG by default, same as /DEVICE if /OLD used"));
    case F_F:	       return (_(7, 20,
"/F           Same as /FREE by default, same as /FULL if /OLD used"));
    default:	       return (NULL);
    }
}

void show_help(opt_t *opts, int opt_count)
{
    int opt_index;
    opt_flag_t displayed_flags = 0;
    opt_flag_t flag;
    char *help_str;

    printf(_(7, 0, "FreeDOS MEM version %s"), MEM_VERSION);
#ifdef DEBUG
    printf(" DEBUG");
#endif
    printf(" [%s %s", __DATE__, __TIME__);
#ifdef __WATCOMC__
    printf(" Watcom C %u.%u", __WATCOMC__ / 100, __WATCOMC__ % 100);
#endif
#ifdef __TURBOC__
    printf(" Turbo C 0x%04X", __TURBOC__);
#endif
    printf("]\n");

    printf("%s\n\n%s\n",
	   _(7, 1, "Displays the amount of used and free memory in your system."),
	   _(7, 2, "Syntax: MEM [zero or more of the options shown below]"));

    for (opt_index = 0; opt_index < opt_count; opt_index++) {
	flag = opts[opt_index].flag;
	if (displayed_flags & flag) {
	    /*
	     * We've already displayed the provided help string for
	     * this flag so don't re-display it.
	     */
	    continue;
	}
	help_str = help_for_flag(flag);
	if (help_str != NULL) {
	    printf("%s\n", help_str);
	    /*
	     * Make sure we don't display this string again - the
	     * string returned by help_for_flag() should have covered
	     * both options.
	     */
	    displayed_flags |= flag;
	} else {
	    printf(_(7, 17, "/%-10s No help is available for this option\n"),
		   opts[opt_index].s);
	    /*
	     * If there is another option with the same flag, we'll
	     * need to display that one too.
	     */
	}
    }

    exit(1);
}

int main(int argc, char *argv[])
{
    opt_flag_t flags;
    UPPERINFO *upper;
    unsigned memfree;
    static char *module_name = NULL;
    static opt_t opts[] =
    {
      { "?",		F_HELP,		NULL },
      { "ALL",		F_ALL,		NULL },
      { "C",		F_CLASSIFY,	NULL },
      { "CLASSIFY",	F_CLASSIFY,	NULL },
      { "D",		F_D,		NULL },
#ifdef DEBUG
      { "DBGCPU",	F_DBGCPU,	NULL },
      { "DBGDEVADDR",	F_DBGDEVADDR,	NULL },
      { "DBGHMAMIN",	F_DBGHMAMIN,	NULL },
#endif
      { "DEBUG",	F_DEBUG,	NULL },
      { "DEVICE",	F_DEVICE,	NULL },
      { "EMS",		F_EMS,		NULL },
      { "F",		F_F,		NULL },
      { "FREE",		F_FREE,		NULL },
      { "FULL",		F_FULL,		NULL },
      { "HELP",		F_HELP,		NULL },
      /*
       * Specify both /M and /MODULE in this array so the parser handles
       * /M<module_name>, i.e. the option without a separator before
       * the module name.
       */
      { "M",		F_MODULE,	&module_name },
      { "MODULE",	F_MODULE,	&module_name },
      { "NOSUMMARY",	F_NOSUMMARY,	NULL },
      { "OLD",		F_OLD,		NULL },
      { "PAGE",		F_PAGE,		NULL },
      { "SUMMARY",	F_SUMMARY,	NULL },
      { "U",		F_UPPER,	NULL },
      { "XMS",		F_XMS,		NULL }
    };

    setup_globals();

    /* avoid unused argument warning? */
    argc = argc;

    kittenopen("mem");

    flags = get_opts(argv, &opts, sizeof(opts) / sizeof(opts[0]));

#ifdef DEBUG
    /*
     * Set up global boolean flags used to enable debugging based on
     * the command-line options.
     */
    dbgcpu = ((flags & F_DBGCPU) == F_DBGCPU);
    dbgdevaddr = ((flags & F_DBGDEVADDR) == F_DBGDEVADDR);
    dbghmamin = ((flags & F_DBGHMAMIN) == F_DBGHMAMIN);
#endif

    /*
     * The /OLD flag selects the old meanings of /D and /F vs. the (new)
     * MS-DOS-like meanings:
     * /D => /OLD ? /DEVICE : /DEBUG
     * /F => /OLD ? /FULL : /FREE */
    if (flags & F_D) {
	if (flags & F_OLD) {
	    flags |= F_DEVICE;
	} else {
	    flags |= F_DEBUG;
	}
    }
    if (flags & F_F) {
	if (flags & F_OLD) {
	    flags |= F_FULL;
	} else {
	    flags |= F_FREE;
	}
    }

    /*
     * /SUMMARY cancels /NOSUMMARY and has a higher precedence.
     */
    if ((flags & F_SUMMARY) && (flags & F_NOSUMMARY)) {
	flags &= ~(F_SUMMARY | F_NOSUMMARY);
    }

    /*
     * If the user specified /NOSUMMARY but not one of the flags in
     * OPT_FLAG_MASK_OUTPUT then no output would be produced.
     */
    if ((flags & F_NOSUMMARY) && !(flags & OPT_FLAG_MASK_OUTPUT)) {
	fatal(_(0,6,
"The /NOSUMMARY option was specified, but no other output-producing options\n"
"were specified, so no output is being produced.\n%s"),
	      _(0,4,"Use /? for help\n"));
    }

    /*
     * /? or /HELP takes precedence over all other options and they
     * are ignored if it is specified.
     */
    if (flags & F_HELP)
      {
	show_help(&opts, sizeof(opts) / sizeof(opts[0]));
      }

    upper=check_upper(make_mcb_list(&memfree));

    if (flags & F_PAGE)	  num_lines=get_font_info();

/*
 * FIXME: In MS-DOS, when /MODULE or /FREE are specified, nothing else
 * is shown.  We need to make sure no other flags are specified!  Or
 * should we allow those options plus something else?
 */
/*
 * FIXME: Perhaps when /MODULE is specified, the /DEBUG flag should
 * determine whether subtrees of devices are included?
 */
    if (flags & F_MODULE) {
	module_list(module_name);
	return 0;
    }
    if (flags & F_FREE)	{
	free_list();
	return 0;
    }

    if (flags & F_DEBUG)  debug_list();
    if (flags & F_DEVICE) device_list();
    if (flags & F_EMS)	  ems_list();
    if (flags & F_FULL)	  full_list();
    if (flags & F_UPPER)  upper_list();
    if (flags & F_XMS)	  xms_list();
    if (flags & F_CLASSIFY)    classify_list(memfree, upper ? upper->free : 0);

    if (!(flags & F_NOSUMMARY))	{
	normal_list(memfree, upper, flags & F_ALL, flags & F_DEBUG);
    }
    return 0;
}

/*
	TE - some size optimizations for __TURBOC__

	as printf() is redefined in PRF.C to use no stream functions,
	rather calls DOS directly, these Stream operations are nowhere used,
	but happen to be in the executable.

	so we define some dummy functions here to save some precious bytes :-)

	this is in no way necessary, but saves us some 1500 bytes */

#ifdef __TURBOC__

#define UNREFERENCED_PARAMETER(x) if (x);

int _Cdecl flushall(void){return 0;}

int _Cdecl fprintf(FILE *__stream, const char *__format, ...)
{ UNREFERENCED_PARAMETER (__stream);
 UNREFERENCED_PARAMETER ( __format);	return 0;}
int _Cdecl fseek(FILE *__stream, long __offset, int __whence)
{ UNREFERENCED_PARAMETER (__stream);
 UNREFERENCED_PARAMETER (__offset);
 UNREFERENCED_PARAMETER ( __whence);
 return 0;}

int _Cdecl setvbuf(FILE *__stream, char *__buf, int __type, size_t __size)
{ UNREFERENCED_PARAMETER (__stream);
 UNREFERENCED_PARAMETER ( __buf);
 UNREFERENCED_PARAMETER ( __type);
 UNREFERENCED_PARAMETER ( __size);   return 0;}

void _Cdecl _xfflush (void){}
void _Cdecl _setupio (void){}

#endif

/*
 * FIXME: should the sizes shown to the user include sizeof(MCB)?  it
 * seems that we are not consistent in including the size of the MCB
 * between /CLASSIFY and /FULL; in MS-DOS we don't have inconsistency
 * between /C and /F but don't know what they DO do.
 */
