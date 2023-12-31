// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/pm_qos.h>

static struct pm_qos_request ddr_opp_req;

void secure_perf_init(void)
{
	pm_qos_add_request(&ddr_opp_req, PM_QOS_DDR_OPP,
		PM_QOS_DDR_OPP_DEFAULT_VALUE);
}

void secure_perf_remove(void)
{
	pm_qos_remove_request(&ddr_opp_req);
}

void secure_perf_raise(void)
{
	// TODO
}

void secure_perf_restore(void)
{
	// TODO
}
