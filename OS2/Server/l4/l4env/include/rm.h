#ifndef __OS3_RM_H__
#define __OS3_RM_H__

#include <l4/l4rm/l4rm.h>

enum l4re_rm_flags_t {
  L4RE_RM_READ_ONLY    = 0x01, /**< \brief Region is read-only */
  L4RE_RM_NO_ALIAS     = 0x02, /**< \brief The region contains exclusive memory that is not mapped anywhere else */
  L4RE_RM_PAGER        = 0x04, /**< \brief Region has a pager */
  L4RE_RM_RESERVED     = 0x08, /**< \brief Region is reserved (blocked) */
  L4RE_RM_REGION_FLAGS = 0x0f, /**< \brief Mask of all region flags */

  L4RE_RM_OVERMAP      = 0x10, /**< \brief Unmap memory already mapped in the region */
  L4RE_RM_SEARCH_ADDR  = 0x20, /**< \brief Search for a suitable address range */
  L4RE_RM_IN_AREA      = 0x40, /**< \brief Search only in area, or map into area */
  L4RE_RM_EAGER_MAP    = 0x80, /**< \brief Eagerly map the attached data space in. */
  L4RE_RM_ATTACH_FLAGS = 0xf0, /**< \brief Mask of all attach flags */
};

L4_CV L4_INLINE int
l4re_rm_attach(void **start, unsigned long size, unsigned long flags,
               l4re_ds_t const mem, l4_addr_t offs,
               unsigned char align);

// implementation

L4_CV L4_INLINE int
l4re_rm_attach(void **start, unsigned long size, unsigned long flags,
               l4re_ds_t const mem, l4_addr_t offs,
               unsigned char align)
{
  return l4rm_attach(mem, size, offs, flags, start);
}

#endif
