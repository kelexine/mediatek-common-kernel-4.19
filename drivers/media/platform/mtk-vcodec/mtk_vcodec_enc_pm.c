/*
 * Copyright (c) 2016 MediaTek Inc.
 * Author: Tiffany Lin <tiffany.lin@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/pm_runtime.h>

#include "mtk_vcodec_enc_pm.h"
#include "mtk_vcodec_util.h"

int mtk_vcodec_init_enc_pm(struct mtk_vcodec_dev *mtkdev)
{
	struct platform_device *pdev;
	struct mtk_vcodec_pm *pm;
	struct mtk_vcodec_clk *enc_clk;
	struct mtk_vcodec_clk_info *clk_info;
	int ret = 0, i = 0;
	struct device *dev;

	pdev = mtkdev->plat_dev;
	pm = &mtkdev->pm;
	memset(pm, 0, sizeof(struct mtk_vcodec_pm));
	pm->mtkdev = mtkdev;
	pm->dev = &pdev->dev;
	dev = &pdev->dev;
	enc_clk = &pm->venc_clk;
	pdev = mtkdev->plat_dev;
	pm->dev = &pdev->dev;

	enc_clk->clk_num = of_property_count_strings(pdev->dev.of_node,
		"clock-names");
	if (enc_clk->clk_num > 0) {
		enc_clk->clk_info = devm_kcalloc(&pdev->dev,
			enc_clk->clk_num, sizeof(*clk_info),
			GFP_KERNEL);
		if (!enc_clk->clk_info)
			return -ENOMEM;
	} else {
		mtk_v4l2_err("Failed to get venc clock count");
		return -EINVAL;
	}

	for (i = 0; i < enc_clk->clk_num; i++) {
		clk_info = &enc_clk->clk_info[i];
		ret = of_property_read_string_index(pdev->dev.of_node,
			"clock-names", i, &clk_info->clk_name);
		if (ret) {
			mtk_v4l2_err("venc failed to get clk name %d", i);
			return ret;
		}
		clk_info->vcodec_clk = devm_clk_get(&pdev->dev,
			clk_info->clk_name);
		if (IS_ERR(clk_info->vcodec_clk)) {
			mtk_v4l2_err("venc devm_clk_get (%d)%s fail", i,
				clk_info->clk_name);
			return PTR_ERR(clk_info->vcodec_clk);
		}
	}

	pm_runtime_enable(&pdev->dev);
	return ret;
}

void mtk_vcodec_release_enc_pm(struct mtk_vcodec_dev *dev)
{
	pm_runtime_disable(dev->pm.dev);
}


void mtk_vcodec_enc_clock_on(struct mtk_vcodec_pm *pm)
{
	struct mtk_vcodec_clk *enc_clk = &pm->venc_clk;
	int ret, i = 0;

	ret = pm_runtime_get_sync(pm->dev);
	if (ret)
		mtk_v4l2_err("pm_runtime_get_sync fail %d", ret);

	for (i = 0; i < enc_clk->clk_num; i++) {
		ret = clk_prepare_enable(enc_clk->clk_info[i].vcodec_clk);
		if (ret) {
			mtk_v4l2_err("venc clk_prepare_enable %d %s fail %d", i,
				enc_clk->clk_info[i].clk_name, ret);
			goto clkerr;
		}
	}

	return;
clkerr:
	for (i -= 1; i >= 0; i--)
		clk_disable_unprepare(enc_clk->clk_info[i].vcodec_clk);
	return;
}

void mtk_vcodec_enc_clock_off(struct mtk_vcodec_pm *pm)
{
	struct mtk_vcodec_clk *enc_clk = &pm->venc_clk;
	int ret, i = 0;

	ret = pm_runtime_put_sync(pm->dev);
	if (ret)
		mtk_v4l2_err("pm_runtime_put_sync fail %d", ret);

	for (i = enc_clk->clk_num - 1; i >= 0; i--)
		clk_disable_unprepare(enc_clk->clk_info[i].vcodec_clk);
}
