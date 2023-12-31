/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 */
enum MD_REG_ID {
	MD_REG_AP_MDSRC_REQ = 0,
	MD_REG_PC_MONITOR,
	MD_REG_PLL_REG,
	MD_REG_BUS,
	MD_REG_MDMCU_BUSMON,
	MD_REG_MDINFRA_BUSMON,
	MD_REG_ECT,
	MD_REG_TOPSM_REG,
	MD_REG_MD_RGU_REG,
	MD_REG_OST_STATUS,
	MD_REG_CSC_REG,
	MD_REG_ELM_REG,
	MD_REG_USIP,
	MD_REG_AP_MDSRC_ACK,
	MD_REG_AP_MDSRC_SETTLE,
};

void internal_md_dump_debug_register(unsigned int md_index);
