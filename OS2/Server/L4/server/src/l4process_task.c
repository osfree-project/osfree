

/* \file   l4env/include/system.h
 * \brief  System/Architecture specific definitions */
/pub/L4_Fiasco/tudos2/my_user_build_dir/include/l4/env/system.h

l4/generic_ts/generic_ts.h

/pub/L4_Fiasco/tudos2/my_user_build_dir/include/x86/l4v2/l4/sys/types.h
L4
 l4_threadid_struct_t   
 l4_threadid_t   
 
 typedef l4_threadid_t l4_taskid_t;
 

/**
 * \brief Allocate a task ID.
 * \retval taskid	allocated task ID.
 * \return		0 on success
 *			error code otherwise. */
/* int
   l4ts_allocate_task(l4_taskid_t *taskid); */
int alloc_task() {
    
}

/**
 * \brief Allocate a task ID and become the task's chief.
 * \retval taskid   allocated task ID
 * \return          0 on success
 *                  error code otherwise
 */
int
l4ts_allocate_task2(l4_taskid_t *taskid);

/**
 * \brief Start a previously allocated task.
 *
 * \param taskid	ID of the previos allocated task
 * \param entry		Initial instruction pointer
 * \param stack		Initial stack pointer
 * \param mcp		Maximum controlled priority (see L4-Spec)
 * \param pager		Pager of first thread
 * \param prio		Priority of first thread
 * \param resname	Module name as specified in the RMGR (subject of
 *			future changes)
 * \param flags		(currently unused)
 */
/*
int
l4ts_create_task(l4_taskid_t *taskid, l4_addr_t entry, l4_addr_t stack,
                 l4_uint32_t mcp, const l4_taskid_t *pager, l4_int32_t prio,
                 const char *resname, l4_uint32_t flags);
*/                 
