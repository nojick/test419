/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 * Author: Rob Clark <rob@ti.com>
 */

#ifndef __OMAPDRM_DRV_H__
#define __OMAPDRM_DRV_H__

#include <linux/module.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#include <drm/drmP.h>
#include <drm/drm_gem.h>
#include <drm/omap_drm.h>

#include "dss/omapdss.h"

#include "omap_connector.h"
#include "omap_crtc.h"
#include "omap_encoder.h"
#include "omap_fb.h"
#include "omap_fbdev.h"
#include "omap_gem.h"
#include "omap_irq.h"
#include "omap_plane.h"

#define DBG(fmt, ...) DRM_DEBUG(fmt"\n", ##__VA_ARGS__)
#define VERB(fmt, ...) if (0) DRM_DEBUG(fmt, ##__VA_ARGS__) /* verbose debug */

#define MODULE_NAME     "omapdrm"

struct omap_drm_usergart;

struct omap_drm_private {
	struct drm_device *ddev;
	struct device *dev;
	u32 omaprev;

	struct dss_device *dss;
	struct dispc_device *dispc;
	const struct dispc_ops *dispc_ops;

	unsigned int num_crtcs;
	struct drm_crtc *crtcs[8];

	unsigned int num_planes;
	struct drm_plane *planes[8];

	unsigned int num_encoders;
	struct drm_encoder *encoders[8];

	unsigned int num_connectors;
	struct drm_connector *connectors[8];

	struct drm_fb_helper *fbdev;

	struct workqueue_struct *wq;

	/* lock for obj_list below */
	struct mutex list_lock;

	/* list of GEM objects: */
	struct list_head obj_list;

	struct omap_drm_usergart *usergart;
	bool has_dmm;

	/* properties: */
	struct drm_property *zorder_prop;

	/* irq handling: */
	spinlock_t wait_lock;		/* protects the wait_list */
	struct list_head wait_list;	/* list of omap_irq_wait */
	u32 irq_mask;			/* enabled irqs in addition to wait_list */

	/* memory bandwidth limit if it is needed on the platform */
	unsigned int max_bandwidth;
};


int omap_debugfs_init(struct drm_minor *minor);

#endif /* __OMAPDRM_DRV_H__ */
