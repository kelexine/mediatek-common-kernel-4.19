/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __MTK_SPM_SYSFS__
#define __MTK_SPM_SYSFS__

#include "mtk_lp_sysfs.h"
#include "mtk_lp_kernfs.h"

/* For legacy definition*/
#define mtk_spm_sysfs_handle	mtk_lp_sysfs_handle
#define mtk_spm_sysfs_op		mtk_lp_sysfs_op

#define mtk_spm_sysfs_entry_func_create	mtk_lp_sysfs_entry_func_create
#define mtk_spm_sysfs_entry_func_node_add	mtk_lp_sysfs_entry_func_node_add
#define mtk_spm_sysfs_entry_create		mtk_spm_sysfs_root_entry_create


/*Get the mtk idle system fs root entry handle*/
int mtk_spm_sysfs_entry_root_get(struct mtk_lp_sysfs_handle **handle);

/*Creat the entry for mtk idle systme fs*/
int mtk_spm_sysfs_entry_group_add(const char *name
		, int mode, struct mtk_lp_sysfs_group *_group
		, struct mtk_lp_sysfs_handle *handle);

/*Add the child file node to mtk idle system*/
int mtk_spm_sysfs_entry_node_add(const char *name, int mode
			, const struct mtk_lp_sysfs_op *op
			, struct mtk_lp_sysfs_handle *node);

int mtk_spm_sysfs_entry_node_remove(
		struct mtk_lp_sysfs_handle *handle);

int mtk_spm_sysfs_root_entry_create(void);

int mtk_spm_sysfs_power_create_group(struct attribute_group *grp);
size_t get_mtk_spm_sysfs_power_bufsz_max(void);

#endif
