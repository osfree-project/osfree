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
#include "lvm1-label.h"
#include "disk-rep.h"
#include "label.h"
#include "metadata.h"
#include "xlate.h"
#include "lvmcache.h"

#include <libdevmapper.h>

#include <sys/stat.h>
#include <fcntl.h>

static void _not_supported(const char *op)
{
	log_err("The '%s' operation is not supported for the lvm1 labeller.",
		op);
}

static int _lvm1_can_handle(struct labeller *l, void *buf, uint64_t sector)
{
	struct pv_disk *pvd = (struct pv_disk *) buf;
	uint32_t version;

	/* LVM1 label must always be in first sector */
	if (sector)
		return 0;

	version = xlate16(pvd->version);

	if (pvd->id[0] == 'H' && pvd->id[1] == 'M' &&
	    (version == 1 || version == 2))
		return 1;

	return 0;
}

static int _lvm1_write(struct label *label, void *buf)
{
	_not_supported("write");
	return 0;
}

static int _lvm1_read(struct labeller *l, struct device *dev, void *buf,
		 struct label **label)
{
	struct pv_disk *pvd = (struct pv_disk *) buf;
	struct vg_disk vgd;
	struct lvmcache_info *info;
	const char *vgid = NULL;
	unsigned exported = 0;

	munge_pvd(dev, pvd);

	if (*pvd->vg_name && read_vgd(dev, &vgd, pvd)) {
		vgid = (char *) vgd.vg_uuid;
		exported = pvd->pv_status & VG_EXPORTED;
	}

	if (!(info = lvmcache_add(l, (char *)pvd->pv_uuid, dev, (char *)pvd->vg_name, vgid,
				  exported))) {
		stack;
		return 0;
	}
	*label = info->label;

	info->device_size = xlate32(pvd->pv_size) << SECTOR_SHIFT;
	list_init(&info->mdas);

	info->status &= ~CACHE_INVALID;

	return 1;
}

static int _lvm1_initialise_label(struct labeller *l, struct label *label)
{
	strcpy(label->type, "LVM1");

	return 1;
}

static void _lvm1_destroy_label(struct labeller *l, struct label *label)
{
	return;
}

static void _lvm1_destroy(struct labeller *l)
{
	dm_free(l);
}

struct label_ops _lvm1_ops = {
	.can_handle = _lvm1_can_handle,
	.write = _lvm1_write,
	.read = _lvm1_read,
	.verify = _lvm1_can_handle,
	.initialise_label = _lvm1_initialise_label,
	.destroy_label = _lvm1_destroy_label,
	.destroy = _lvm1_destroy,
};

struct labeller *lvm1_labeller_create(struct format_type *fmt)
{
	struct labeller *l;

	if (!(l = dm_malloc(sizeof(*l)))) {
		log_err("Couldn't allocate labeller object.");
		return NULL;
	}

	l->ops = &_lvm1_ops;
	l->private = (const void *) fmt;

	return l;
}
