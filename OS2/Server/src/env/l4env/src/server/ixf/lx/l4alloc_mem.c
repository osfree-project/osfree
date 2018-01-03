//
// Memmory allocation work in following logic:
//   1. Get region of memory using l4dm_mem_open
//   2. After memory opened we need to map pages
// Map can be done page by page. Whole region can't be mapped if it large of L4_PAGESIZE
//

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/l4_alloc_mem.h>
#include <os3/cfgparser.h>
#include <os3/io.h>

/* l4env includes */
#include <l4/dm_mem/dm_mem.h>
#include <l4/l4rm/l4rm.h>
#include <l4/env/errno.h>
#include <l4/env/env.h>
#include <l4/util/l4_macros.h>

/* libc includes */
#include <stdio.h>

//extern l4_taskid_t taskid;

/****************************************************************************************/

// #if 0 /*!defined(__OS2__) && !defined(__LINUX__) */
// 
// IN:  o32_base, o32_size, PROT_WRITE | PROT_READ | PROT_EXEC
// OUT: mmap_obj
//         mmap_obj = mmap((void *)lx_obj->o32_base, lx_obj->o32_size,
//                                  PROT_WRITE | PROT_READ | PROT_EXEC  ,       /* | PROT_EXEC */
//                                   MAP_GROWSDOWN | MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 0/*lx_exe_mod->fh*/,
//                                  0 /*lx_exe_mod->lx_head_e32_exe->e32_datapage*/);
// #else
// // Under OS/2 return always unique address
//   #if defined(__LINUX__) || defined(__WIN32__)
//         mmap_obj = malloc(lx_obj->o32_size);
//   #else
//         DosAllocMem(&mmap_obj, lx_obj->o32_size, PAG_COMMIT|PAG_EXECUTE|PAG_READ|PAG_WRITE);
//   #endif
// #endif

/****************************************************************************************/

void l4_test_mem_alloc(void) {
    int ret;
    l4dm_dataspace_t ds2;
    void *addr5;
    l4_threadid_t dm_id = l4env_get_default_dsm();
    
    //Region test
    //  Skapar en region på adress 4096 med storlek 8192.
//     ret = l4rm_area_setup_region(4096 , 81, 
//                        L4RM_DEFAULT_REGION_AREA, 
//                        L4RM_REGION_PAGER, /* L4RM_REGION_PAGER L4RM_REGION_BLOCKED*/
//                        0, /* L4RM_LOG2_ALIGNED | L4RM_LOG2_ALLOC, */
//                        L4_INVALID_ID);
//     if (ret < 0)
//         printf("l4rm_area_setup_region failed: %s\n", l4env_errstr(ret));
//     else
//         printf("got area at 0x%08lx\n", 4096);
//     l4rm_show_region_list();
//     if (ret < 0)
//         enter_kdebug("l4rm_area_setup_region failed");
    /* open new ds 2 */
    ret = l4dm_mem_open(dm_id,8*8192,0,0,"l4rm_test2",&ds2);
    if (ret)
    {
      printf("error allocating dataspace: %d\n",ret);
      enter_kdebug("???");
    }
    printf("ds2 = %d at "l4util_idfmt"\n",ds2.id,l4util_idstr(ds2.manager));
  
  
    l4_addr_t fpage_addr=0;
    l4_size_t fpage_size=0;
    ret=
    l4dm_map_pages(&ds2, 0, 8*8192,     // 0 - 8192 i ds                   
                         0, 16 /*2^13=8192*/, 0,  // 0 - 8192 i rcv window, offset=0 
                    L4DM_RW,  &fpage_addr, &fpage_size);
    if (ret < 0)
        printf("l4dm_map_pages failed: %s\n", l4env_errstr(ret));
    else {
        printf("got area at 0x%08lx\n", fpage_addr);
//        int i;
//        int s=fpage_size;
//        char *p= (char*)fpage_addr;
//        for(i=0; i<s; i++) {
//            printf("i=%d,", i);
//            p[i] = 1;
//        }
    }
    addr5 = (void*)0x10010000;
    ret = l4rm_attach_to_region(&ds2,addr5,1000,0,0);
    if (ret)
    {
      printf("attach dataspace failed %s\n", l4env_errstr(ret));
      enter_kdebug("-");
    }
}

/* Partial working memory allocation for L4. Non working because it allocates
   virtual memory from dm_phys (physical mem) and sometimes fail.
      To fix this, use the function l4dm_map_pages, as in the test function
   above this comment.
     l4dm_map_pages has some difficulities, it needs to be run (I guess)
   from the thread it allocates mem to. */
void * l4_alloc_mem(unsigned long long area, int base, int size, int flags, unsigned long PIC, void *ds)
{
    /* L4/Fiasco example*/
    int st;
    //printf("l4_alloc_mem( %d, %d, 0x%x\n", base, size, flags);
    //l4dm_dataspace_t ds;
    void *addr;
    int l4_flags = l4_translate_os2_flags(flags); /* PAG_COMMIT|PAG_EXECUTE|PAG_READ|PAG_WRITE */
                                                  /* L4RM_MAP   L4DM_RW     L4DM_RO or L4DM_RW */
    //printf("l4_translate_os2_flags( 0x%x ) = 0x%x\n", flags, l4_flags);
    /* Allocate mem */
    /*          dsm_id,           size,      align,     flags,       name,          ds
                l4_threadid_t,    l4_size_t, l4_addr_t, l4_uint32_t, const char *,  l4dm_dataspace_t
    */
 /* l4dm_mem_open(L4DM_DEFAULT_DSM, 8192,    0,         0,           "L4 RM Example", &ds); */
    l4dm_mem_open(L4DM_DEFAULT_DSM, size,    4096,      l4_flags,    "L4 RM Example", (l4dm_dataspace_t *)ds);

    /*          ds,               size,      ds_offs,   flags,       addr
                l4dm_dataspace_t, l4_size_t, l4_offs_t, l4_uint32_t, void **
    */
 /* l4rm_attach(&ds,              8192,      0,         0,           &addr) */
 /* l4rm_attach(&ds,              size,      base,      l4_flags,    &addr); */
//    io_log("PIC=%d\n",PIC);
    //if (PIC)
    //{
      //st =  l4rm_attach(ds,             size,      0,      l4_flags,    &addr);
      st = l4rm_area_attach((l4dm_dataspace_t *)ds,   (unsigned long)area,   size,      0,      l4_flags,    &addr);
    //} else {
    //  st = l4rm_attach_to_region(ds,    base,      size,   0,         l4_flags);
    //  addr = base;
    //}

//    l4dm_share(&ds,taskid, L4DM_RW);
/*                        ds,     addr,      size,   ds_offs,   flags
                          const l4dm_dataspace_t *, const void *, l4_size_t, l4_offs_t, l4_uint32_t
    l4rm_attach_to_region(const l4dm_dataspace_t * ds, const void * addr,
                      l4_size_t size, l4_offs_t ds_offs, l4_uint32_t flags);
*/
//    l4dm_ds_show(&ds);
    //printf("l4rm_attach_to_region  ret = 0x%x (%d)\n", st, st);
    if(st < 0) {
      st = st * (-1);
      printf("l4env_strerror '%s'\n", l4env_strerror(st));
      l4rm_show_region_list();
    }

  if (options.debugmemmgr) l4rm_show_region_list();

/* * \return  0 on success (dataspace attached to region at \a addr), error code
 *          otherwise:
 *          - -#L4_EINVAL  invalid dataspace id
 *          - -#L4_EUSED   region already used
 *          - -#L4_ENOMEM  out of memory allocating descriptors
 *          - -#L4_ENOMAP  no region found
 *          - -#L4_EIPC    error calling region mapper */

    /* Dealloc mem */
    /*/l4rm_detach(addr);
    l4dm_close(&ds); */
    //printf("l4_alloc_mem  0x%x\n", addr);
    return addr;
}

/****************************************************************************************/

int l4_translate_os2_flags(int flags) { /* PAG_COMMIT|PAG_EXECUTE|PAG_READ|PAG_WRITE */
                                        /* L4RM_MAP   L4DM_READ   L4DM_READ or L4DM_WRITE */
   int l4flags=0;
   if((flags & PAG_COMMIT)==PAG_COMMIT) {
     l4flags = l4flags | L4RM_MAP;
    }
   if((flags & PAG_EXECUTE)==PAG_EXECUTE) {
     l4flags = l4flags | L4DM_READ;
    }

   if( ((flags & PAG_READ)==PAG_READ) && 
       (!((flags & PAG_WRITE)==PAG_WRITE)) ) {
     l4flags = l4flags | L4DM_WRITE;
    }

   if((flags & PAG_WRITE)==PAG_WRITE) {
     l4flags = l4flags | L4DM_WRITE;
    }


   return l4flags;
}

/*****************************************************************************/
/**
 * \brief   Setup VM area  -  l4rm_area_setup_region
 * \ingroup api_vm
 *
 * \param   addr         Region start address
 * \param   size         Region size
 * \param   area         Area id, set to #L4RM_DEFAULT_REGION_AREA to use
 *                       default area (i.e. an area not reserved)
 * \param   type         Region type:
 *                       - #L4RM_REGION_PAGER      region with external pager,
 *                                                 \a pager must contain the id
 *                                                 of the external pager
 *                       - #L4RM_REGION_EXCEPTION  region with exception forward
 *                       - #L4RM_REGION_BLOCKED    blocked (unavailable) region
 * \param   flags        Flags:
 *                       - #L4RM_LOG2_ALIGNED
 *                         reserve a 2^(log2(size) + 1) aligned region
 *                       - #L4RM_SUPERPAGE_ALIGNED
 *                         reserve a superpage aligned region
 *                       - #L4RM_LOG2_ALLOC
 *                         reserve the whole 2^(log2(size) + 1) sized region
 * \param   pager        External pager (if type is #L4RM_REGION_PAGER), if set
 *                       to L4_INVALID_ID, the pager of the region mapper thread
 *                       is used
 *
 * \return  0 on success (setup region), error code otherwise:
 *          - -#L4_ENOMEM     out of memory allocation region descriptor
 *          - -#L4_EUSED      address region already used
 *          - -#L4_EINVAL     invalid area / type
 *          - -#L4_EIPC       error calling region mapper
 */
/*****************************************************************************/
/*
L4_INLINE int
l4rm_area_setup_region(l4_addr_t addr, l4_size_t size, l4_uint32_t area,
                       int type, l4_uint32_t flags, l4_threadid_t pager);
*/

/*****************************************************************************/
/**
 * \brief   Attach dataspace to specified region.
 * \ingroup api_attach
 *
 * \param   ds           Dataspace id
 * \param   addr         Start address, must be page aligned
 * \param   size         Size
 * \param   ds_offs      Offset in dataspace
 * \param   flags        Flags:
 *                       - #L4DM_RO   attach read-only
 *                       - #L4DM_RW   attach read/write
 *                       - #L4RM_MAP  immediately map attached dataspace area
 *
 * \return  0 on success (dataspace attached to region at \a addr), error code
 *          otherwise:
 *          - -#L4_EINVAL  invalid dataspace id
 *          - -#L4_EUSED   region already used
 *          - -#L4_ENOMEM  out of memory allocating descriptors
 *          - -#L4_ENOMAP  no region found
 *          - -#L4_EIPC    error calling region mapper
 *
 * Attach dataspace to region at \a addr.
 */
/*****************************************************************************/
/* L4_INLINE int
l4rm_attach_to_region(const l4dm_dataspace_t * ds, const void * addr,
                      l4_size_t size, l4_offs_t ds_offs, l4_uint32_t flags);
*/

/****************************************************************************************/

/**  l4rm_attach()
 * \brief   Attach dataspace. 
 * \ingroup api_attach
 *
 * \param   ds           Dataspace id
 * \param   size         Size
 * \param   ds_offs      Offset in dataspace
 * \param   flags        Flags:
 *                       - #L4DM_RO           attach read-only
 *                       - #L4DM_RW           attach read/write
 *                       - #L4RM_LOG2_ALIGNED find a
 *                                            \f$2^{(log_2(size) + 1)}\f$
 *                                            aligned region
 *                       - #L4RM_SUPERPAGE_ALIGNED find a
 *                                            superpage aligned region
 *                       - #L4RM_LOG2_ALLOC   allocate the whole
 *                                            \f$2^{(log_2(size) + 1)}\f$
 *                                            sized area
 *                       - #L4RM_MAP          immediately map attached dataspace
 *                                            area
 * \retval  addr          Start address
 *
 * \return  0 on success (dataspace attached to region at address \a addr),
 *          error code otherwise:
 *          - -#L4_EINVAL  invalid dataspace id
 *          - -#L4_ENOMEM  out of memory allocating descriptors
 *          - -#L4_ENOMAP  no region found
 *          - -#L4_EIPC    error calling region mapper
 *
 * Find an unused map region and attach dataspace area
 * (\a ds_offs, \a ds_offs + \a size) to that region.
 */
/*****************************************************************************/
/* L4_INLINE int
l4rm_attach(const l4dm_dataspace_t * ds, l4_size_t size, l4_offs_t ds_offs,
	    l4_uint32_t flags, void ** addr); */
        


/** l4dm_mem_open()
 * \brief   Create new dataspace 
 * \ingroup api_open
 *
 * \param   dsm_id       Dataspace manager id, set to #L4DM_DEFAULT_DSM
 *                       to use default dataspace manager provided by the
 *                       L4 environment
 * \param   size         Dataspace size
 * \param   align        Alignment
 * \param   flags        Flags:
 *                       - #L4DM_CONTIGUOUS allocate dataspace on phys.
 *                         contiguous memory
 *                       - #L4DM_PINNED allocate "pinned" memory, there will
 *                         be no pagefaults once the dataspace is mapped
 * \param   name         Dataspace name
 * \retval  ds           Dataspace id
 *
 * \return  0 on success, error code otherwise:
 *          - -#L4_EIPC    IPC error calling dataspace manager
 *          - -#L4_ENOMEM  out of memory
 *          - -#L4_ENODM   no dataspace manager found
 *
 * Call dataspace manager \a dsm_id to create a new memory dataspace.
 */
/*****************************************************************************/ 
/* int
l4dm_mem_open(l4_threadid_t dsm_id, l4_size_t size, l4_addr_t align,
	      l4_uint32_t flags, const char * name,
	      l4dm_dataspace_t * ds); */



/*****************************************************************************/
/*  ds  offs, size
          ^
         |
    rcv rcv_sddr, rcv_size2, rcv_offs

l4_addr_t fpage_addr=0;
l4_size_t fpage_size=0;
l4dm_map_pages( &ds, 0, 8192,     // 0 - 8192 i ds                   
                     0, 8192, 0,  // 0 - 8192 i rcv window, offset=0 
                L4DM_RW,  &fpage_addr, &fpage_size);
*/
/**
 * \brief Map dataspace region (IDL wrapper) - l4dm_map_pages
 *
 * \param  ds            Dataspace descriptor
 * \param  offs          Offset in dataspace
 * \param  size          Region size
 * \param  rcv_addr      Receive window address
 * \param  rcv_size2     Receive window size (log2)
 * \param  rcv_offs      Offset in receive window
 * \param  flags         Flags:
 *                       - #L4DM_RO          map read-only
 *                       - #L4DM_RW          map read/write
 *                       - #L4DM_MAP_PARTIAL allow partial mappings
 *                       - #L4DM_MAP_MORE    if possible, map more than the
 *                                           specified dataspace region
 * \retval fpage_addr    Map address of receive fpage
 * \retval fpage_size    Size of receive fpage
 *
 * \return 0 on success (got fpage), error code otherwise:
 *         - -#L4_EIPC         IPC error calling dataspace manager
 *         - -#L4_EINVAL       invalid dataspace id or map / receive window
 *                             size
 *         - -#L4_EINVAL_OFFS  invalid dataspace / receive window offset
 *         - -#L4_EPERM        permission denied
 *
 * For a detailed description of #L4DM_MAP_PARTIAL and #L4DM_MAP_MORE
 * see l4dm_map_pages().
 */
/*****************************************************************************/
/*
int
l4dm_map_pages(const l4dm_dataspace_t * ds, l4_offs_t offs, l4_size_t size,
               l4_addr_t rcv_addr, int rcv_size2, l4_offs_t rcv_offs,
               l4_uint32_t flags, l4_addr_t * fpage_addr,
               l4_size_t * fpage_size)
*/
