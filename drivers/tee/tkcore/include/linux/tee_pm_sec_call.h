/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2015-2019 TrustKernel Incorporated
 */

#ifndef TEE_PM_SEC_CALL_H_
#define TEE_PM_SEC_CALL_H_

#ifdef CONFIG_ARM

#define TKCORE_SET_NS_BOOT_ADDR	(0xBF000103U)
#define TKCORE_PREPARE_CPU_OFF	(0xBF000104U)
#define TKCORE_ERRATA_802022	(0xBF000105U)

#define TKCORE_SET_WDT_KADDR    (0xBF000300U)

static void tee_pm_sec_call(u32 cmd, u32 p0, u32 p1, u32 p2)
{
	__asm__ __volatile__(
		".arch_extension sec\n"
		"mov r0, %0\n"
		"mov r1, %1\n"
		"mov r2, %2\n"
		"mov r3, %3\n"
		"smc #0\n" :
		: "r"(cmd), "r"(p0), "r"(p1), "r"(p2)
		: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
}

#endif

#endif
