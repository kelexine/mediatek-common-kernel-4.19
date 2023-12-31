/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */


#ifndef __MTK_RESOURCE_CONSTRAINT_V1_H__
#define __MTK_RESOURCE_CONSTRAINT_V1_H__

/* Please refer platform mt_spm.h at atf code for constraint id definition */
#define MT_RM_CONSTRAINT_ID_BUS26M	(0)
#define MT_RM_CONSTRAINT_ID_SYSPLL	(1)
#define MT_RM_CONSTRAINT_ID_DRAM	(2)

#define MT_RM_CONSTRAINT_INVALID        (0x0)
#define MT_RM_CONSTRAINT_SW_VALID       (1<<0L)
#define MT_RM_CONSTRAINT_FW_VALID	(1<<1L)

/* MT_RM_CONSTRAINT_SW_VALID | MT_RM_CONSTRAINT_FW_VALID */
#define MT_RM_CONSTRAINT_VALID          (0x11)

/* Please refer mt_lp_rm.h at atf code */
#define MT_RM_CONSTRAINT_ALLOW_CPU_BUCK		(1<<0L)
#define MT_RM_CONSTRAINT_ALLOW_DRAM_EVENT	(1<<1L)
#define MT_RM_CONSTRAINT_ALLOW_SYSPLL_EVENT	(1<<2L)
#define MT_RM_CONSTRAINT_ALLOW_INFRA_EVENT	(1<<3L)
#define MT_RM_CONSTRAINT_ALLOW_26M_EVENT	(1<<4L)
#define MT_RM_CONSTRAINT_ALLOW_VCORE_LP		(1<<5L)
#define MT_RM_CONSTRAINT_ALLOW_INFRA_PDN	(1<<6L)
#define MT_RM_CONSTRAINT_ALLOW_26M_OFF		(1<<7L)

/*
 * 1. 26m clk	[OFF]
 * 2. infra pwr	[ON]
 * 3. mainpll	[OFF]
 * 4. vcore low power mode
 */
#define VCORE_LP_CLK_26M_OFF (\
	MT_RM_CONSTRAINT_ALLOW_VCORE_LP\
	| MT_RM_CONSTRAINT_ALLOW_26M_OFF)

/*
 * 1. 26m clk	[ON]
 * 2. infra pwr	[ON]
 * 3. mainpll	[OFF]
 * 4. vcore low power mode
 */
#define VCORE_LP_CLK_26M_ON (\
	MT_RM_CONSTRAINT_ALLOW_VCORE_LP\
	| MT_RM_CONSTRAINT_ALLOW_26M_EVENT)

/*
 * 1. 26m clk	[OFF]
 * 2. mainpll	[OFF]
 * 3. infra pwr	[OFF]
 * 4. vcore low power mode
 */
#define VCORE_LP_CLK_26M_INFRA_PDN (\
	MT_RM_CONSTRAINT_ALLOW_VCORE_LP\
	| MT_RM_CONSTRAINT_ALLOW_INFRA_PDN\
	| MT_RM_CONSTRAINT_ALLOW_26M_OFF)


/* mainpll	[OFF] */
#define MAINPLL_OFF (\
	MT_RM_CONSTRAINT_ALLOW_SYSPLL_EVENT)


#define DRAM_OFF (\
	MT_RM_CONSTRAINT_ALLOW_DRAM_EVENT)


#define CPU_BUCK_OFF (\
	MT_RM_CONSTRAINT_ALLOW_CPU_BUCK)


#define MT_RM_STATUS_CHECK(status, flag)\
				((status & flag) == flag)


#endif
