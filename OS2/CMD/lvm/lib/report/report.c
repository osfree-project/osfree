/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "lib.h"
#include "metadata.h"
#include "report.h"
#include "toolcontext.h"
#include "lvm-string.h"
#include "display.h"
#include "activate.h"
#include "segtype.h"
#include "str_list.h"
#include "lvmcache.h"

#include <libdevmapper.h>

struct lvm_report_object {
        struct volume_group *vg;
        struct logical_volume *lv;
        struct physical_volume *pv;
        struct lv_segment *seg;
        struct pv_segment *pvseg;
};

/*
 * For macro use
 */
static union {
        struct physical_volume _pv;
        struct logical_volume _lv;
        struct volume_group _vg;
        struct lv_segment _seg;
        struct pv_segment _pvseg;
} _dummy;

static char _alloc_policy_char(alloc_policy_t alloc)
{
        switch (alloc) {
        case ALLOC_CONTIGUOUS:
                return 'c';
        case ALLOC_CLING:
                return 'l';
        case ALLOC_NORMAL:
                return 'n';
        case ALLOC_ANYWHERE:
                return 'a';
        default:
                return 'i';
        }
}

/*
 * Data-munging functions to prepare each data type for display and sorting
 */
static int _string_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        return dm_report_field_string(rh, field, (const char **) data);
}

static int _dev_name_disp(struct dm_report *rh, struct dm_pool *mem,
                          struct dm_report_field *field,
                          const void *data, void *private)
{
        const char *name = dev_name(*(const struct device **) data);

        return dm_report_field_string(rh, field, &name);
}

static int _devices_disp(struct dm_report *rh, struct dm_pool *mem,
                         struct dm_report_field *field,
                         const void *data, void *private)
{
        const struct lv_segment *seg = (const struct lv_segment *) data;
        unsigned int s;
        const char *name = NULL;
        uint32_t extent = 0;
        char extent_str[32];

        if (!dm_pool_begin_object(mem, 256)) {
                log_error("dm_pool_begin_object failed");
                return 0;
        }

        for (s = 0; s < seg->area_count; s++) {
                switch (seg_type(seg, s)) {
                case AREA_LV:
                        name = seg_lv(seg, s)->name;
                        extent = seg_le(seg, s);
                        break;
                case AREA_PV:
                        name = dev_name(seg_dev(seg, s));
                        extent = seg_pe(seg, s);
                        break;
                case AREA_UNASSIGNED:
                        name = "unassigned";
                        extent = 0;
                }

                if (!dm_pool_grow_object(mem, name, strlen(name))) {
                        log_error("dm_pool_grow_object failed");
                        return 0;
                }

                if (dm_snprintf(extent_str, sizeof(extent_str), "(%" PRIu32
                                 ")", extent) < 0) {
                        log_error("Extent number dm_snprintf failed");
                        return 0;
                }

                if (!dm_pool_grow_object(mem, extent_str, strlen(extent_str))) {
                        log_error("dm_pool_grow_object failed");
                        return 0;
                }

                if ((s != seg->area_count - 1) &&
                    !dm_pool_grow_object(mem, ",", 1)) {
                        log_error("dm_pool_grow_object failed");
                        return 0;
                }
        }

        if (!dm_pool_grow_object(mem, "\0", 1)) {
                log_error("dm_pool_grow_object failed");
                return 0;
        }

        dm_report_field_set_value(field, dm_pool_end_object(mem), NULL);

        return 1;
}

static int _tags_disp(struct dm_report *rh, struct dm_pool *mem,
                      struct dm_report_field *field,
                      const void *data, void *private)
{
        const struct list *tags = (const struct list *) data;
        struct str_list *sl;

        if (!dm_pool_begin_object(mem, 256)) {
                log_error("dm_pool_begin_object failed");
                return 0;
        }

        //list_iterate_items(sl, tags) {
        list_iterate_items(sl, struct str_list, tags) {
                if (!dm_pool_grow_object(mem, sl->str, strlen(sl->str)) ||
                    (sl->list.n != tags && !dm_pool_grow_object(mem, ",", 1))) {
                        log_error("dm_pool_grow_object failed");
                        return 0;
                }
        }

        if (!dm_pool_grow_object(mem, "\0", 1)) {
                log_error("dm_pool_grow_object failed");
                return 0;
        }

        dm_report_field_set_value(field, dm_pool_end_object(mem), NULL);

        return 1;
}

static int _modules_disp(struct dm_report *rh, struct dm_pool *mem,
                         struct dm_report_field *field,
                         const void *data, void *private)
{
        const struct logical_volume *lv = (const struct logical_volume *) data;
        struct list *modules;

        if (!(modules = str_list_create(mem))) {
                log_error("modules str_list allocation failed");
                return 0;
        }

        if (!list_lv_modules(mem, lv, modules))
                return_0;

        return _tags_disp(rh, mem, field, modules, private);
}

static int _vgfmt_disp(struct dm_report *rh, struct dm_pool *mem,
                       struct dm_report_field *field,
                       const void *data, void *private)
{
        const struct volume_group *vg = (const struct volume_group *) data;

        if (!vg->fid) {
                dm_report_field_set_value(field, "", NULL);
                return 1;
        }

        return _string_disp(rh, mem, field, &vg->fid->fmt->name, private);
}

static int _pvfmt_disp(struct dm_report *rh, struct dm_pool *mem,
                       struct dm_report_field *field,
                       const void *data, void *private)
{
        const struct physical_volume *pv =
            (const struct physical_volume *) data;

        if (!pv->fmt) {
                dm_report_field_set_value(field, "", NULL);
                return 1;
        }

        return _string_disp(rh, mem, field, &pv->fmt->name, private);
}

static int _lvkmaj_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct logical_volume *lv = (const struct logical_volume *) data;
        struct lvinfo info;
        uint64_t minusone = UINT64_C(-1);

        if (lv_info(lv->vg->cmd, lv, &info, 0) && info.exists)
                return dm_report_field_int(rh, field, &info.major);

        return dm_report_field_uint64(rh, field, &minusone);
}

static int _lvkmin_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct logical_volume *lv = (const struct logical_volume *) data;
        struct lvinfo info;
        uint64_t minusone = UINT64_C(-1);

        if (lv_info(lv->vg->cmd, lv, &info, 0) && info.exists)
                return dm_report_field_int(rh, field, &info.minor);

        return dm_report_field_uint64(rh, field, &minusone);
}

static int _lvstatus_disp(struct dm_report *rh, struct dm_pool *mem,
                          struct dm_report_field *field,
                          const void *data, void *private)
{
        const struct logical_volume *lv = (const struct logical_volume *) data;
        struct lvinfo info;
        char *repstr;
        float snap_percent;

        if (!(repstr = dm_pool_zalloc(mem, 7))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        if (lv->status & PVMOVE)
                repstr[0] = 'p';
        else if (lv->status & MIRRORED) {
                if (lv->status & MIRROR_NOTSYNCED)
                        repstr[0] = 'M';
                else
                        repstr[0] = 'm';
        }else if (lv->status & MIRROR_IMAGE)
                repstr[0] = 'i';
        else if (lv->status & MIRROR_LOG)
                repstr[0] = 'l';
        else if (lv->status & VIRTUAL)
                repstr[0] = 'v';
        else if (lv_is_origin(lv))
                repstr[0] = 'o';
        else if (lv_is_cow(lv))
                repstr[0] = 's';
        else
                repstr[0] = '-';

        if (lv->status & PVMOVE)
                repstr[1] = '-';
        else if (lv->status & LVM_WRITE)
                repstr[1] = 'w';
        else
                repstr[1] = 'r';

        repstr[2] = _alloc_policy_char(lv->alloc);

        if (lv->status & LOCKED)
                repstr[2] = toupper(repstr[2]);

        if (lv->status & FIXED_MINOR)
                repstr[3] = 'm';        /* Fixed Minor */
        else
                repstr[3] = '-';

        if (lv_info(lv->vg->cmd, lv, &info, 1) && info.exists) {
                if (info.suspended)
                        repstr[4] = 's';        /* Suspended */
                else if (info.live_table)
                        repstr[4] = 'a';        /* Active */
                else if (info.inactive_table)
                        repstr[4] = 'i';        /* Inactive with table */
                else
                        repstr[4] = 'd';        /* Inactive without table */

                /* Snapshot dropped? */
                if (info.live_table && lv_is_cow(lv) &&
                    (!lv_snapshot_percent(lv, &snap_percent) ||
                     snap_percent < 0 || snap_percent >= 100)) {
                        repstr[0] = toupper(repstr[0]);
                        if (info.suspended)
                                repstr[4] = 'S'; /* Susp Inv snapshot */
                        else
                                repstr[4] = 'I'; /* Invalid snapshot */
                }

                if (info.open_count)
                        repstr[5] = 'o';        /* Open */
                else
                        repstr[5] = '-';
        } else {
                repstr[4] = '-';
                repstr[5] = '-';
        }

        dm_report_field_set_value(field, repstr, NULL);
        return 1;
}

static int _pvstatus_disp(struct dm_report *rh, struct dm_pool *mem,
                          struct dm_report_field *field,
                          const void *data, void *private)
{
        const uint32_t status = *(const uint32_t *) data;
        char *repstr;

        if (!(repstr = dm_pool_zalloc(mem, 4))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        if (status & ALLOCATABLE_PV)
                repstr[0] = 'a';
        else
                repstr[0] = '-';

        if (status & EXPORTED_VG)
                repstr[1] = 'x';
        else
                repstr[1] = '-';

        dm_report_field_set_value(field, repstr, NULL);
        return 1;
}

static int _vgstatus_disp(struct dm_report *rh, struct dm_pool *mem,
                          struct dm_report_field *field,
                          const void *data, void *private)
{
        const struct volume_group *vg = (const struct volume_group *) data;
        char *repstr;

        if (!(repstr = dm_pool_zalloc(mem, 7))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        if (vg->status & LVM_WRITE)
                repstr[0] = 'w';
        else
                repstr[0] = 'r';

        if (vg->status & RESIZEABLE_VG)
                repstr[1] = 'z';
        else
                repstr[1] = '-';

        if (vg->status & EXPORTED_VG)
                repstr[2] = 'x';
        else
                repstr[2] = '-';

        if (vg->status & PARTIAL_VG)
                repstr[3] = 'p';
        else
                repstr[3] = '-';

        repstr[4] = _alloc_policy_char(vg->alloc);

        if (vg->status & CLUSTERED)
                repstr[5] = 'c';
        else
                repstr[5] = '-';

        dm_report_field_set_value(field, repstr, NULL);
        return 1;
}

static int _segtype_disp(struct dm_report *rh, // __attribute((unused)),
                         struct dm_pool *mem,
                         struct dm_report_field *field,
                         const void *data, void *private)
{
        const struct lv_segment *seg = (const struct lv_segment *) data;

        if (seg->area_count == 1) {
                dm_report_field_set_value(field, "linear", NULL);
                return 1;
        }

        dm_report_field_set_value(field, seg->segtype->ops->name(seg), NULL);
        return 1;
}

static int _origin_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct logical_volume *lv = (const struct logical_volume *) data;

        if (lv_is_cow(lv))
                return dm_report_field_string(rh, field,
                                              (const char **) &origin_from_cow(lv)->name);

        dm_report_field_set_value(field, "", NULL);
        return 1;
}

static int _loglv_disp(struct dm_report *rh, struct dm_pool *mem,
                       struct dm_report_field *field,
                       const void *data, void *private)
{
        const struct logical_volume *lv = (const struct logical_volume *) data;
        struct lv_segment *seg;

        //list_iterate_items(seg, &lv->segments) {
        list_iterate_items(seg, struct lv_segment, &lv->segments) {
                if (!seg_is_mirrored(seg) || !seg->log_lv)
                        continue;
                return dm_report_field_string(rh, field,
                                              (const char **) &seg->log_lv->name);
        }

        dm_report_field_set_value(field, "", NULL);
        return 1;
}

static int _lvname_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct logical_volume *lv = (const struct logical_volume *) data;
        char *repstr, *lvname;
        size_t len;

        if (lv_is_visible(lv)) {
                repstr = lv->name;
                return dm_report_field_string(rh, field, (const char **) &repstr);
        }

        len = strlen(lv->name) + 3;
        if (!(repstr = dm_pool_zalloc(mem, len))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        if (dm_snprintf(repstr, len, "[%s]", lv->name) < 0) {
                log_error("lvname snprintf failed");
                return 0;
        }

        if (!(lvname = dm_pool_strdup(mem, lv->name))) {
                log_error("dm_pool_strdup failed");
                return 0;
        }

        dm_report_field_set_value(field, repstr, lvname);

        return 1;
}

static int _movepv_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct logical_volume *lv = (const struct logical_volume *) data;
        const char *name;
        struct lv_segment *seg;

        //list_iterate_items(seg, &lv->segments) {
        list_iterate_items(seg, struct lv_segment, &lv->segments) {
                if (!(seg->status & PVMOVE))
                        continue;
                name = dev_name(seg_dev(seg, 0));
                return dm_report_field_string(rh, field, &name);
        }

        dm_report_field_set_value(field, "", NULL);
        return 1;
}

static int _size32_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const uint32_t size = *(const uint32_t *) data;
        const char *disp, *repstr;
        uint64_t *sortval;

        if (!*(disp = display_size_units(private, (uint64_t) size))) {
                stack;
                return 0;
        }

        if (!(repstr = dm_pool_strdup(mem, disp))) {
                log_error("dm_pool_strdup failed");
                return 0;
        }

        if (!(sortval = dm_pool_alloc(mem, sizeof(uint64_t)))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        *sortval = (const uint64_t) size;

        dm_report_field_set_value(field, repstr, sortval);

        return 1;
}

static int _size64_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const uint64_t size = *(const uint64_t *) data;
        const char *disp, *repstr;
        uint64_t *sortval;

        if (!*(disp = display_size_units(private, size))) {
                stack;
                return 0;
        }

        if (!(repstr = dm_pool_strdup(mem, disp))) {
                log_error("dm_pool_strdup failed");
                return 0;
        }

        if (!(sortval = dm_pool_alloc(mem, sizeof(uint64_t)))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        *sortval = size;
        dm_report_field_set_value(field, repstr, sortval);

        return 1;
}

static int _vgsize_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct volume_group *vg = (const struct volume_group *) data;
        uint64_t size;

        size = (uint64_t) vg->extent_count * vg->extent_size;

        return _size64_disp(rh, mem, field, &size, private);
}

static int _segstart_disp(struct dm_report *rh, struct dm_pool *mem,
                          struct dm_report_field *field,
                          const void *data, void *private)
{
        const struct lv_segment *seg = (const struct lv_segment *) data;
        uint64_t start;

        start = (uint64_t) seg->le * seg->lv->vg->extent_size;

        return _size64_disp(rh, mem, field, &start, private);
}

static int _segsize_disp(struct dm_report *rh, struct dm_pool *mem,
                         struct dm_report_field *field,
                         const void *data, void *private)
{
        const struct lv_segment *seg = (const struct lv_segment *) data;
        uint64_t size;

        size = (uint64_t) seg->len * seg->lv->vg->extent_size;

        return _size64_disp(rh, mem, field, &size, private);
}

static int _chunksize_disp(struct dm_report *rh, struct dm_pool *mem,
                           struct dm_report_field *field,
                           const void *data, void *private)
{
        const struct lv_segment *seg = (const struct lv_segment *) data;
        uint64_t size;

        if (lv_is_cow(seg->lv))
                size = (uint64_t) find_cow(seg->lv)->chunk_size;
        else
                size = 0;

        return _size64_disp(rh, mem, field, &size, private);
}

static int _pvused_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct physical_volume *pv =
            (const struct physical_volume *) data;
        uint64_t used;

        if (!pv->pe_count)
                used = 0LL;
        else
                used = (uint64_t) pv->pe_alloc_count * pv->pe_size;

        return _size64_disp(rh, mem, field, &used, private);
}

static int _pvfree_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct physical_volume *pv =
            (const struct physical_volume *) data;
        uint64_t freespace;

        if (!pv->pe_count)
                freespace = pv->size;
        else
                freespace = (uint64_t) (pv->pe_count - pv->pe_alloc_count) * pv->pe_size;

        return _size64_disp(rh, mem, field, &freespace, private);
}

static int _pvsize_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct physical_volume *pv =
            (const struct physical_volume *) data;
        uint64_t size;

        if (!pv->pe_count)
                size = pv->size;
        else
                size = (uint64_t) pv->pe_count * pv->pe_size;

        return _size64_disp(rh, mem, field, &size, private);
}

static int _devsize_disp(struct dm_report *rh, struct dm_pool *mem,
                         struct dm_report_field *field,
                         const void *data, void *private)
{
        const struct device *dev = *(const struct device **) data;
        uint64_t size;

        if (!dev_get_size(dev, &size))
                size = 0;

        return _size64_disp(rh, mem, field, &size, private);
}

static int _vgfree_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct volume_group *vg = (const struct volume_group *) data;
        uint64_t freespace;

        freespace = (uint64_t) vg->free_count * vg->extent_size;

        return _size64_disp(rh, mem, field, &freespace, private);
}

static int _uuid_disp(struct dm_report *rh, struct dm_pool *mem,
                      struct dm_report_field *field,
                      const void *data, void *private)
{
        char *repstr = NULL;

        if (!(repstr = dm_pool_alloc(mem, 40))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        if (!id_write_format((const struct id *) data, repstr, 40)) {
                stack;
                return 0;
        }

        dm_report_field_set_value(field, repstr, NULL);
        return 1;
}

static int _uint32_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        return dm_report_field_uint32(rh, field, data);
}

static int _int32_disp(struct dm_report *rh, struct dm_pool *mem,
                       struct dm_report_field *field,
                       const void *data, void *private)
{
        return dm_report_field_int32(rh, field, data);
}

static int _pvmdas_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        struct lvmcache_info *info;
        uint32_t count;
        const char *pvid = (const char *)(&((struct id *) data)->uuid);

        info = info_from_pvid(pvid);
        count = info ? list_size(&info->mdas) : 0;

        return _uint32_disp(rh, mem, field, &count, private);
}

static int _vgmdas_disp(struct dm_report *rh, struct dm_pool *mem,
                        struct dm_report_field *field,
                        const void *data, void *private)
{
        const struct volume_group *vg = (const struct volume_group *) data;
        uint32_t count;

        count = list_size(&vg->fid->metadata_areas);

        return _uint32_disp(rh, mem, field, &count, private);
}

static int _lvsegcount_disp(struct dm_report *rh, struct dm_pool *mem,
                            struct dm_report_field *field,
                            const void *data, void *private)
{
        const struct logical_volume *lv = (const struct logical_volume *) data;
        uint32_t count;

        count = list_size(&lv->segments);

        return _uint32_disp(rh, mem, field, &count, private);
}

static int _snpercent_disp(struct dm_report *rh, struct dm_pool *mem,
                           struct dm_report_field *field,
                           const void *data, void *private)
{
        const struct logical_volume *lv = (const struct logical_volume *) data;
        struct lvinfo info;
        float snap_percent;
        uint64_t *sortval;
        char *repstr;

        /* Suppress snapshot percentage if not using driver */
        if (!activation()) {
                dm_report_field_set_value(field, "", NULL);
                return 1;
        }

        if (!(sortval = dm_pool_alloc(mem, sizeof(uint64_t)))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        if (!lv_is_cow(lv) ||
            (lv_info(lv->vg->cmd, lv, &info, 0) && !info.exists)) {
                *sortval = UINT64_C(0);
                dm_report_field_set_value(field, "", sortval);
                return 1;
        }

        if (!lv_snapshot_percent(lv, &snap_percent) || snap_percent < 0) {
                *sortval = UINT64_C(100);
                dm_report_field_set_value(field, "100.00", sortval);
                return 1;
        }

        if (!(repstr = dm_pool_zalloc(mem, 8))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        if (dm_snprintf(repstr, 7, "%.2f", snap_percent) < 0) {
                log_error("snapshot percentage too large");
                return 0;
        }

        *sortval = snap_percent * UINT64_C(1000);
        dm_report_field_set_value(field, repstr, sortval);

        return 1;
}

static int _copypercent_disp(struct dm_report *rh, struct dm_pool *mem,
                             struct dm_report_field *field,
                             const void *data, void *private)
{
        struct logical_volume *lv = (struct logical_volume *) data;
        float percent;
        uint64_t *sortval;
        char *repstr;

        if (!(sortval = dm_pool_alloc(mem, sizeof(uint64_t)))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        if ((!(lv->status & PVMOVE) && !(lv->status & MIRRORED)) ||
            !lv_mirror_percent(lv->vg->cmd, lv, 0, &percent, NULL)) {
                *sortval = UINT64_C(0);
                dm_report_field_set_value(field, "", sortval);
                return 1;
        }

        percent = copy_percent(lv);

        if (!(repstr = dm_pool_zalloc(mem, 8))) {
                log_error("dm_pool_alloc failed");
                return 0;
        }

        if (dm_snprintf(repstr, 7, "%.2f", percent) < 0) {
                log_error("copy percentage too large");
                return 0;
        }

        *sortval = percent * UINT64_C(1000);
        dm_report_field_set_value(field, repstr, sortval);

        return 1;
}

/* Report object types */

/* necessary for displaying something for PVs not belonging to VG */
static struct volume_group _dummy_vg = {
        .name = (char *) ORPHAN,
};

static void *_obj_get_vg(void *obj)
{
        struct volume_group *vg = ((struct lvm_report_object *)obj)->vg;

        return vg ? vg : &_dummy_vg;
}

static void *_obj_get_lv(void *obj)
{
        return ((struct lvm_report_object *)obj)->lv;
}

static void *_obj_get_pv(void *obj)
{
        return ((struct lvm_report_object *)obj)->pv;
}

static void *_obj_get_seg(void *obj)
{
        return ((struct lvm_report_object *)obj)->seg;
}

static void *_obj_get_pvseg(void *obj)
{
        return ((struct lvm_report_object *)obj)->pvseg;
}

#ifdef DEVMAPPER_SUPPORT

static const struct dm_report_object_type _report_types[] = {
        { VGS, "Volume Group", "vg_", _obj_get_vg },
        { LVS, "Logical Volume", "lv_", _obj_get_lv },
        { PVS, "Physical Volume", "pv_", _obj_get_pv },
        { SEGS, "Logical Volume Segment", "seg_", _obj_get_seg },
        { PVSEGS, "Physical Volume Segment", "pvseg_", _obj_get_pvseg },
        { 0, "", "", NULL },
};

/*
 * Import column definitions
 */

#define STR DM_REPORT_FIELD_TYPE_STRING
#define NUM DM_REPORT_FIELD_TYPE_NUMBER
#define FIELD(type, strct, sorttype, head, field, width, func, id, desc) {type, sorttype, (off_t)((void *)&_dummy._ ## strct.field - (void *)&_dummy._ ## strct), width, id, head, &_ ## func ## _disp, desc},

static struct dm_report_field_type _fields[] = {
#include "columns.h"
{0, 0, 0, 0, "", "", NULL, NULL},
};

#undef STR
#undef NUM
#undef FIELD

void *report_init(struct cmd_context *cmd, const char *format, const char *keys,
                  report_type_t *report_type, const char *separator,
                  int aligned, int buffered, int headings)
{
        uint32_t report_flags = 0;

        if (aligned)
                report_flags |= DM_REPORT_OUTPUT_ALIGNED;

        if (buffered)
                report_flags |= DM_REPORT_OUTPUT_BUFFERED;

        if (headings)
                report_flags |= DM_REPORT_OUTPUT_HEADINGS;

        return dm_report_init(report_type, _report_types, _fields, format,
                              separator, report_flags, keys, cmd);
}

/*
 * Create a row of data for an object
 */
int report_object(void *handle, struct volume_group *vg,
                  struct logical_volume *lv, struct physical_volume *pv,
                  struct lv_segment *seg, struct pv_segment *pvseg)
{
        struct lvm_report_object obj;

        obj.vg = vg;
        obj.lv = lv;
        obj.pv = pv;
        obj.seg = seg;
        obj.pvseg = pvseg;

        return dm_report_object(handle, &obj);
}

#else

#endif
