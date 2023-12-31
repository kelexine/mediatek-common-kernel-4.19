// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/cpufreq.h>

#include "mtk_ppm_platform.h"
#include "mtk_ppm_internal.h"


static int __init ppm_power_data_init(void)
{

	struct cpufreq_policy *policy;

	policy = cpufreq_cpu_get(6);

	if (!policy) {
		ppm_info("PPM not support!\n");
		return 0;
	}
	cpufreq_cpu_put(policy);

	ppm_lock(&ppm_main_info.lock);

	mt_ppm_set_dvfs_table(0, NULL, 0, 0);

	ppm_cobra_init();

	ppm_platform_init();

#ifdef PPM_SSPM_SUPPORT
	ppm_ipi_init(0, PPM_COBRA_TBL_SRAM_ADDR);
#endif

	ppm_unlock(&ppm_main_info.lock);

	ppm_info("power data init done!\n");

	return 0;
}

/* should be run after cpufreq and upower init */
late_initcall(ppm_power_data_init);

