/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __APU_DVFS_H
#define __APU_DVFS_H

#include <linux/delay.h>
#include <linux/devfreq.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm_qos.h>

#include "apusys_power_user.h"

#define MTK_MMDVFS_ENABLE     (0)

#define PM_QOS_VCORE_OPP_DEFAULT_VALUE	(15)
#define PM_QOS_VVPU_OPP_DEFAULT_VALUE	(3)
#define PM_QOS_VMDLA_OPP_DEFAULT_VALUE	(3)
#define	PM_QOS_VCORE_OPP	(0)
#define	PM_QOS_VVPU_OPP		(1)
#define	PM_QOS_VMDLA_OPP	(2)

enum vcore_opp {
	VCORE_OPP_0 = 0,
	VCORE_OPP_1,
	VCORE_OPP_2,
	VCORE_OPP_3,
	VCORE_OPP_4,
	VCORE_OPP_5,
	VCORE_OPP_6,
	VCORE_OPP_7,
	VCORE_OPP_8,
	VCORE_OPP_9,
	VCORE_OPP_10,
	VCORE_OPP_11,
	VCORE_OPP_12,
	VCORE_OPP_13,
	VCORE_OPP_14,
	VCORE_OPP_15,
	VCORE_OPP_NUM,
	VCORE_OPP_UNREQ = PM_QOS_VCORE_OPP_DEFAULT_VALUE,
};

#define VCORE_DVFS_LOW_BOUND	(65000) /* mV x 100 */
#define VVPU_DVFS_VOLT0	 (82500)	/* mV x 100 */
#define VVPU_DVFS_VOLT1	 (72500)	/* mV x 100 */
#define VVPU_DVFS_VOLT2	 (65000)	/* mV x 100 */
#define VVPU_PTPOD_FIX_VOLT	 (80000)	/* mV x 100 */


#define VMDLA_DVFS_VOLT0	 (82500)	/* mV x 100 */
#define VMDLA_DVFS_VOLT1	 (72500)	/* mV x 100 */
#define VMDLA_DVFS_VOLT2	 (65000)	/* mV x 100 */
#define VMDLA_PTPOD_FIX_VOLT	 (80000)	/* mV x 100 */

#define USER_VPU0  0x1
#define USER_VPU1  0x2
#define USER_MDLA  0x4
#define USER_EDMA  0x8

struct vpu_opp_table_info {
	unsigned int vpufreq_khz;
	unsigned int vpufreq_volt;
	unsigned int vpufreq_idx;
};
struct mdla_opp_table_info {
	unsigned int mdlafreq_khz;
	unsigned int mdlafreq_volt;
	unsigned int mdlafreq_idx;
};

struct vpu_ptp_count_info {
	int vpu_ptp_count;
};
struct mdla_ptp_count_info {
	int mdla_ptp_count;
};


struct apu_dvfs {
	struct devfreq		*devfreq;

	bool qos_enabled;
	bool dvfs_enabled;

	void __iomem		*regs;
	void __iomem		*sram_regs;

	struct notifier_block	pm_qos_vvpu_opp_nb;
	struct notifier_block	pm_qos_vmdla_opp_nb;

	struct reg_config	*init_config;
	struct device_node *dvfs_node;
	unsigned int dvfs_irq;

	bool opp_forced;
	char			force_start[20];
	char			force_end[20];
};

enum vvpu_opp {
	VVPU_OPP_0 = 0,
	VVPU_OPP_1,
	VVPU_OPP_2,
	VVPU_OPP_NUM,
	VVPU_OPP_UNREQ = PM_QOS_VVPU_OPP_DEFAULT_VALUE,
};
enum vmdla_opp {
	VMDLA_OPP_0 = 0,
	VMDLA_OPP_1,
	VMDLA_OPP_2,
	VMDLA_OPP_NUM,
	VMDLA_OPP_UNREQ = PM_QOS_VMDLA_OPP_DEFAULT_VALUE,
};
enum APU_SEGMENT {
	SEGMENT_90M = 0,
	SEGMENT_90,
	SEGMENT_95,
	SEGMENT_NUM,
};



extern char *apu_dvfs_dump_reg(char *ptr);
extern void apu_dvfs_reg_config(struct reg_config *config);
extern int apu_dvfs_add_interface(struct device *dev);
extern void apu_dvfs_remove_interface(struct device *dev);
extern int apu_dvfs_platform_init(struct apu_dvfs *dvfs);

extern unsigned int vvpu_get_cur_volt(void);
extern unsigned int vmdla_get_cur_volt(void);
extern unsigned int vvpu_update_volt(unsigned int pmic_volt[]
		, unsigned int array_size);
extern void vvpu_restore_default_volt(void);
extern unsigned int vpu_get_freq_by_idx(unsigned int idx);
extern unsigned int vpu_get_volt_by_idx(unsigned int idx);
extern void vpu_disable_by_ptpod(void);
extern void vpu_enable_by_ptpod(void);
int vvpu_regulator_set_mode(bool enable);
int vmdla_regulator_set_mode(bool enable);
bool vvpu_vmdla_vcore_checker(void);
unsigned int vvpu_update_ptp_count(unsigned int ptp_count[],
		unsigned int array_size);
unsigned int vmdla_update_ptp_count(unsigned int ptp_count[],
		unsigned int array_size);
void mdla_enable_by_ptpod(void);
void mdla_disable_by_ptpod(void);
bool get_vvpu_DVFS_is_paused_by_ptpod(void);
bool get_ready_for_ptpod_check(void);
int apu_dvfs_dump_info(void);
int vpu_get_hw_vvpu_opp(int core);
int mdla_get_hw_vmdla_opp(int core);
extern unsigned int mt_get_ckgen_freq(unsigned int ID);
extern void check_vpu_clk_sts(void);
void apu_get_power_info_internal(void);
void ptpod_is_enabled(bool enable);

int apu_dvfs_init(struct platform_device *pdev);
int apu_dvfs_remove(struct platform_device *pdev);
#ifndef ENABLE_PMQOS
int apusys_pm_vcore(enum DVFS_USER device, int volt);
int apusys_pm_update_request(enum DVFS_USER device, int opp);
int apusys_pm_request_arbiter(void);
#endif
#endif /* __APU_DVFS_H */

