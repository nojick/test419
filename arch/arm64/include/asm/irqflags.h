/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_IRQFLAGS_H
#define __ASM_IRQFLAGS_H

#ifdef __KERNEL__

#include <asm/ptrace.h>

/*
 * Aarch64 has flags for masking: Debug, Asynchronous (serror), Interrupts and
 * FIQ exceptions, in the 'daif' register. We mask and unmask them in 'dai'
 * order:
 * Masking debug exceptions causes all other exceptions to be masked too/
 * Masking SError masks irq, but not debug exceptions. Masking irqs has no
 * side effects for other flags. Keeping to this order makes it easier for
 * entry.S to know which exceptions should be unmasked.
 *
 * FIQ is never expected, but we mask it when we disable debug exceptions, and
 * unmask it at all other times.
 */

/*
 * CPU interrupt mask handling.
 */
static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags;
	asm volatile(
		"mrs	%0, daif		// arch_local_irq_save\n"
		"msr	daifset, #2"
		: "=r" (flags)
		:
		: "memory");
	return flags;
}

static inline void arch_local_irq_enable(void)
{
	asm volatile(
		"msr	daifclr, #2		// arch_local_irq_enable"
		:
		:
		: "memory");
}

static inline void arch_local_irq_disable(void)
{
	asm volatile(
		"msr	daifset, #2		// arch_local_irq_disable"
		:
		:
		: "memory");
}

/*
 * Save the current interrupt enable state.
 */
static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;
	asm volatile(
		"mrs	%0, daif		// arch_local_save_flags"
		: "=r" (flags)
		:
		: "memory");
	return flags;
}

/*
 * restore saved IRQ state
 */
static inline void arch_local_irq_restore(unsigned long flags)
{
	asm volatile(
		"msr	daif, %0		// arch_local_irq_restore"
	:
	: "r" (flags)
	: "memory");
}

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return flags & PSR_I_BIT;
}
#endif
#endif
