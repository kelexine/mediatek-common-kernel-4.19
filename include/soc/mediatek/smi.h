/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015-2016 MediaTek Inc.
 * Author: Yong Wu <yong.wu@mediatek.com>
 */
#ifndef MTK_IOMMU_SMI_H
#define MTK_IOMMU_SMI_H

#include <linux/bitops.h>
#include <linux/device.h>

#if IS_ENABLED(CONFIG_MTK_SMI)

#define MTK_LARB_NR_MAX		16

#define MTK_SMI_MMU_EN(port)	BIT(port)

struct mtk_smi_larb_iommu {
	struct device *dev;
	unsigned int   mmu;
};

struct mtk_smi_iommu {
	struct mtk_smi_larb_iommu larb_imu[MTK_LARB_NR_MAX];
};

void mtk_smi_common_bw_set(struct device *dev, const u32 port, const u32 val);
void mtk_smi_larb_bw_set(struct device *dev, const u32 port, const u32 val);

#else

static inline void
mtk_smi_common_bw_set(struct device *dev, const u32 port, const u32 val) { }
static inline void
mtk_smi_larb_bw_set(struct device *dev, const u32 port, const u32 val) { }

#endif

#endif
