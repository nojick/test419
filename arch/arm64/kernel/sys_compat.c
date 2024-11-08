// SPDX-License-Identifier: GPL-2.0-only
/*
 * Based on arch/arm/kernel/sys_arm.c
 *
 * Copyright (C) People who wrote linux/arch/i386/kernel/sys_i386.c
 * Copyright (C) 1995, 1996 Russell King.
 * Copyright (C) 2012 ARM Ltd.
 */

#include <linux/compat.h>
#include <linux/personality.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>

#include <asm/cacheflush.h>
#include <asm/system_misc.h>
#include <asm/unistd.h>

static long
__do_compat_cache_op(unsigned long start, unsigned long end)
{
	long ret;

	do {
		unsigned long chunk = min(PAGE_SIZE, end - start);

		if (fatal_signal_pending(current))
			return 0;

		ret = __flush_cache_user_range(start, start + chunk);
		if (ret)
			return ret;

		cond_resched();
		start += chunk;
	} while (start < end);

	return 0;
}

static inline long
do_compat_cache_op(unsigned long start, unsigned long end, int flags)
{
	if (end < start || flags)
		return -EINVAL;

	if (!access_ok(VERIFY_READ, (const void __user *)start, end - start))
		return -EFAULT;

	return __do_compat_cache_op(start, end);
}
/*
 * Handle all unrecognised system calls.
 */
long compat_arm_syscall(struct pt_regs *regs)
{
	siginfo_t info;
	unsigned int no = regs->regs[7];

	switch (no) {
	/*
	 * Flush a region from virtual address 'r0' to virtual address 'r1'
	 * _exclusive_.  There is no alignment requirement on either address;
	 * user space does not need to know the hardware cache layout.
	 *
	 * r2 contains flags.  It should ALWAYS be passed as ZERO until it
	 * is defined to be something else.  For now we ignore it, but may
	 * the fires of hell burn in your belly if you break this rule. ;)
	 *
	 * (at a later date, we may want to allow this call to not flush
	 * various aspects of the cache.  Passing '0' will guarantee that
	 * everything necessary gets flushed to maintain consistency in
	 * the specified region).
	 */
	case __ARM_NR_compat_cacheflush:
		return do_compat_cache_op(regs->regs[0], regs->regs[1], regs->regs[2]);

	case __ARM_NR_compat_set_tls:
		current->thread.uw.tp_value = regs->regs[0];

		/*
		 * Protect against register corruption from context switch.
		 * See comment in tls_thread_flush.
		 */
		barrier();
		write_sysreg(regs->regs[0], tpidrro_el0);
		return 0;

	default:
		/*
		 * Calls 9f00xx..9f07ff are defined to return -ENOSYS
		 * if not implemented, rather than raising SIGILL. This
		 * way the calling program can gracefully determine whether
		 * a feature is supported.
		 */
		if ((no & 0xffff) <= 0x7ff)
			return -ENOSYS;
		break;
	}

	clear_siginfo(&info);
	info.si_signo = SIGILL;
	info.si_errno = 0;
	info.si_code  = ILL_ILLTRP;
	info.si_addr  = (void __user *)instruction_pointer(regs) -
			 (compat_thumb_mode(regs) ? 2 : 4);

	arm64_notify_die("Oops - bad compat syscall(2)", regs, &info, no);
	return 0;
}
