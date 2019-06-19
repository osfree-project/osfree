/* bp.h
 *
 * Copyright 1995, Willows Software, Inc.
 */

#ifndef BP_H
#define BP_H

typedef struct bp_regs bp_regs_t;
typedef unsigned long bp_id_t;

struct bp_regs
{
    unsigned int write_mask;
    unsigned int debug_regs[8];
};

#define BPIOCTL_GETID		_IOR('B', 0, bp_id_t)
#define BPIOCTL_READREGS	_IOR('B', 1, bp_regs_t)
#define BPIOCTL_WRITEREGS	_IOW('B', 2, bp_regs_t)

#define BP_ID			(('B' << 24) | ('P' << 16) | ('b' << 8) | 'p')

#endif /* BP_H */
