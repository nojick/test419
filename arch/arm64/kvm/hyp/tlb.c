// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2015 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 */

#include <asm/kvm_hyp.h>
#include <asm/kvm_mmu.h>
#include <asm/tlbflush.h>

static void __hyp_text __tlb_switch_to_guest_vhe(struct kvm *kvm)
{
	u64 val;

	/*
	 * With VHE enabled, we have HCR_EL2.{E2H,TGE} = {1,1}, and
	 * most TLB operations target EL2/EL0. In order to affect the
	 * guest TLBs (EL1/EL0), we need to change one of these two
	 * bits. Changing E2H is impossible (goodbye TTBR1_EL2), so
	 * let's flip TGE before executing the TLB operation.
	 */
	write_sysreg(kvm->arch.vttbr, vttbr_el2);
	val = read_sysreg(hcr_el2);
	val &= ~HCR_TGE;
	write_sysreg(val, hcr_el2);
	isb();
}

static void __hyp_text __tlb_switch_to_guest_nvhe(struct kvm *kvm)
{
	write_sysreg(kvm->arch.vttbr, vttbr_el2);
	isb();
}

static hyp_alternate_select(__tlb_switch_to_guest,
			    __tlb_switch_to_guest_nvhe,
			    __tlb_switch_to_guest_vhe,
			    ARM64_HAS_VIRT_HOST_EXTN);

static void __hyp_text __tlb_switch_to_host_vhe(struct kvm *kvm)
{
	/*
	 * We're done with the TLB operation, let's restore the host's
	 * view of HCR_EL2.
	 */
	write_sysreg(0, vttbr_el2);
	write_sysreg(HCR_HOST_VHE_FLAGS, hcr_el2);
}

static void __hyp_text __tlb_switch_to_host_nvhe(struct kvm *kvm)
{
	write_sysreg(0, vttbr_el2);
}

static hyp_alternate_select(__tlb_switch_to_host,
			    __tlb_switch_to_host_nvhe,
			    __tlb_switch_to_host_vhe,
			    ARM64_HAS_VIRT_HOST_EXTN);

void __hyp_text __kvm_tlb_flush_vmid_ipa(struct kvm *kvm, phys_addr_t ipa)
{
	dsb(ishst);

	/* Switch to requested VMID */
	kvm = kern_hyp_va(kvm);
	__tlb_switch_to_guest()(kvm);

	/*
	 * We could do so much better if we had the VA as well.
	 * Instead, we invalidate Stage-2 for this IPA, and the
	 * whole of Stage-1. Weep...
	 */
	ipa >>= 12;
	__tlbi(ipas2e1is, ipa);

	/*
	 * We have to ensure completion of the invalidation at Stage-2,
	 * since a table walk on another CPU could refill a TLB with a
	 * complete (S1 + S2) walk based on the old Stage-2 mapping if
	 * the Stage-1 invalidation happened first.
	 */
	dsb(ish);
	__tlbi(vmalle1is);
	dsb(ish);
	isb();

	/*
	 * If the host is running at EL1 and we have a VPIPT I-cache,
	 * then we must perform I-cache maintenance at EL2 in order for
	 * it to have an effect on the guest. Since the guest cannot hit
	 * I-cache lines allocated with a different VMID, we don't need
	 * to worry about junk out of guest reset (we nuke the I-cache on
	 * VMID rollover), but we do need to be careful when remapping
	 * executable pages for the same guest. This can happen when KSM
	 * takes a CoW fault on an executable page, copies the page into
	 * a page that was previously mapped in the guest and then needs
	 * to invalidate the guest view of the I-cache for that page
	 * from EL1. To solve this, we invalidate the entire I-cache when
	 * unmapping a page from a guest if we have a VPIPT I-cache but
	 * the host is running at EL1. As above, we could do better if
	 * we had the VA.
	 *
	 * The moral of this story is: if you have a VPIPT I-cache, then
	 * you should be running with VHE enabled.
	 */
	if (!has_vhe() && icache_is_vpipt())
		__flush_icache_all();

	__tlb_switch_to_host()(kvm);
}

void __hyp_text __kvm_tlb_flush_vmid(struct kvm *kvm)
{
	dsb(ishst);

	/* Switch to requested VMID */
	kvm = kern_hyp_va(kvm);
	__tlb_switch_to_guest()(kvm);

	__tlbi(vmalls12e1is);
	dsb(ish);
	isb();

	__tlb_switch_to_host()(kvm);
}

void __hyp_text __kvm_tlb_flush_local_vmid(struct kvm_vcpu *vcpu)
{
	struct kvm *kvm = kern_hyp_va(kern_hyp_va(vcpu)->kvm);

	/* Switch to requested VMID */
	__tlb_switch_to_guest()(kvm);

	__tlbi(vmalle1);
	dsb(nsh);
	isb();

	__tlb_switch_to_host()(kvm);
}

void __hyp_text __kvm_flush_vm_context(void)
{
	dsb(ishst);
	__tlbi(alle1is);
	asm volatile("ic ialluis" : : );
	dsb(ish);
}
