/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * omap_connector.h -- OMAP DRM Connector
 *
 * Copyright (C) 2011 Texas Instruments
 * Author: Rob Clark <rob@ti.com>
 */

#ifndef __OMAPDRM_CONNECTOR_H__
#define __OMAPDRM_CONNECTOR_H__

#include <linux/types.h>

struct drm_connector;
struct drm_device;
struct drm_encoder;
struct omap_dss_device;

struct drm_connector *omap_connector_init(struct drm_device *dev,
		int connector_type, struct omap_dss_device *dssdev,
		struct drm_encoder *encoder);
struct drm_encoder *omap_connector_attached_encoder(
		struct drm_connector *connector);
bool omap_connector_get_hdmi_mode(struct drm_connector *connector);

#endif /* __OMAPDRM_CONNECTOR_H__ */
