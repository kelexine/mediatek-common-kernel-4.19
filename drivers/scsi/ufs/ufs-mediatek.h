/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 MediaTek Inc.
 */

#ifndef _UFS_MEDIATEK_H
#define _UFS_MEDIATEK_H

/*
 * Vendor specific UFSHCI Registers
 */
#define REG_UFS_ADDR_XOUFS_ST       0x144

/* UFS_ADDR_XOUFS_ST 0x144 */
#define XOUFS_RELEASE               0x0
#define XOUFS_REQUEST               0x1
#define XOUFS_ACK                   0x2

/*
 * Vendor specific pre-defined parameters
 */
#define UFS_MTK_LIMIT_NUM_LANES_RX  1
#define UFS_MTK_LIMIT_NUM_LANES_TX  1
#define UFS_MTK_LIMIT_HSGEAR_RX     UFS_HS_G3
#define UFS_MTK_LIMIT_HSGEAR_TX     UFS_HS_G3
#define UFS_MTK_LIMIT_PWMGEAR_RX    UFS_PWM_G4
#define UFS_MTK_LIMIT_PWMGEAR_TX    UFS_PWM_G4
#define UFS_MTK_LIMIT_RX_PWR_PWM    SLOW_MODE
#define UFS_MTK_LIMIT_TX_PWR_PWM    SLOW_MODE
#define UFS_MTK_LIMIT_RX_PWR_HS     FAST_MODE
#define UFS_MTK_LIMIT_TX_PWR_HS     FAST_MODE
#define UFS_MTK_LIMIT_HS_RATE       PA_HS_MODE_B
#define UFS_MTK_LIMIT_DESIRED_MODE  UFS_HS_MODE

/*
 * Other attributes
 */
#define VS_DEBUGCLOCKENABLE         0xD0A1
#define VS_SAVEPOWERCONTROL         0xD0A6
#define VS_UNIPROPOWERDOWNCONTROL   0xD0A8

/*
 * VS_DEBUGCLOCKENABLE
 */
enum {
	TX_SYMBOL_CLK_REQ_FORCE = 5,
};

/*
 * VS_SAVEPOWERCONTROL
 */
enum {
	RX_SYMBOL_CLK_GATE_EN   = 0,
	SYS_CLK_GATE_EN         = 2,
	TX_CLK_GATE_EN          = 3,
};

/*
 * Ref-clk control mode
 */
enum {
	REF_CLK_SW_MODE         = 0,
	REF_CLK_HALF_HW_MODE    = 1,
	REF_CLK_HW_MODE         = 2
};

/* HW ref-clk control timeout value */
#define REF_CLK_CTRL_TOUT_MS    3

struct ufs_mtk_host {
	struct ufs_hba *hba;
	struct phy *mphy;
	u32 refclk_ctrl;
};

#endif /* !_UFS_MEDIATEK_H */

