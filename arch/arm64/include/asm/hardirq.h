/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_HARDIRQ_H
#define __ASM_HARDIRQ_H

#include <linux/cache.h>
#include <linux/threads.h>
#include <asm/irq.h>

#define NR_IPI	7

typedef struct {
	unsigned int __softirq_pending;
	unsigned int ipi_irqs[NR_IPI];
} ____cacheline_aligned irq_cpustat_t;

#include <linux/irq_cpustat.h>	/* Standard mappings for irq_cpustat_t above */

#define __inc_irq_stat(cpu, member)	__IRQ_STAT(cpu, member)++
#define __get_irq_stat(cpu, member)	__IRQ_STAT(cpu, member)

u64 smp_irq_stat_cpu(unsigned int cpu);
#define arch_irq_stat_cpu	smp_irq_stat_cpu

#define __ARCH_IRQ_EXIT_IRQS_DISABLED	1

static inline void ack_bad_irq(unsigned int irq)
{
	extern unsigned long irq_err_count;
	irq_err_count++;
}

#endif /* __ASM_HARDIRQ_H */
