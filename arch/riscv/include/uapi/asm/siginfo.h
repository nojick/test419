/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Ltd.
 * Copyright (C) 2015 Regents of the University of California
 */
#ifndef __ASM_SIGINFO_H
#define __ASM_SIGINFO_H

#define __ARCH_SI_PREAMBLE_SIZE	(__SIZEOF_POINTER__ == 4 ? 12 : 16)

#include <asm-generic/siginfo.h>

#endif
