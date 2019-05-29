/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2017 Linaro Ltd.
 */
#ifndef __VENUS_FIRMWARE_H__
#define __VENUS_FIRMWARE_H__

struct device;

int venus_boot(struct device *dev, const char *fwname);
int venus_shutdown(struct device *dev);

#endif
