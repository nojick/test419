// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copied from arch/arm64/kernel/cpufeature.c
 *
 * Copyright (C) 2015 ARM Ltd.
 * Copyright (C) 2017 SiFive
 */

#include <linux/of.h>
#include <asm/processor.h>
#include <asm/hwcap.h>

unsigned long elf_hwcap __read_mostly;

void riscv_fill_hwcap(void)
{
	struct device_node *node;
	const char *isa;
	size_t i;
	static unsigned long isa2hwcap[256] = {0};

	isa2hwcap['i'] = isa2hwcap['I'] = COMPAT_HWCAP_ISA_I;
	isa2hwcap['m'] = isa2hwcap['M'] = COMPAT_HWCAP_ISA_M;
	isa2hwcap['a'] = isa2hwcap['A'] = COMPAT_HWCAP_ISA_A;
	isa2hwcap['f'] = isa2hwcap['F'] = COMPAT_HWCAP_ISA_F;
	isa2hwcap['d'] = isa2hwcap['D'] = COMPAT_HWCAP_ISA_D;
	isa2hwcap['c'] = isa2hwcap['C'] = COMPAT_HWCAP_ISA_C;

	elf_hwcap = 0;

	/*
	 * We don't support running Linux on hertergenous ISA systems.  For
	 * now, we just check the ISA of the first processor.
	 */
	node = of_find_node_by_type(NULL, "cpu");
	if (!node) {
		pr_warning("Unable to find \"cpu\" devicetree entry");
		return;
	}

	if (of_property_read_string(node, "riscv,isa", &isa)) {
		pr_warning("Unable to find \"riscv,isa\" devicetree entry");
		return;
	}

	for (i = 0; i < strlen(isa); ++i)
		elf_hwcap |= isa2hwcap[(unsigned char)(isa[i])];

	pr_info("elf_hwcap is 0x%lx", elf_hwcap);
}
