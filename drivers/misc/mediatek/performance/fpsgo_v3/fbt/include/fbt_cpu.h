/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __FBT_CPU_H__
#define __FBT_CPU_H__

#include "fpsgo_base.h"

#if defined(CONFIG_MTK_FPSGO) || defined(CONFIG_MTK_FPSGO_V3)
void fpsgo_ctrl2fbt_dfrc_fps(int fps_limit);
void fpsgo_ctrl2fbt_cpufreq_cb(int cid, unsigned long freq);
void fpsgo_ctrl2fbt_vsync(unsigned long long ts);
void fpsgo_comp2fbt_frame_start(struct render_info *thr,
		unsigned long long ts);
void fpsgo_comp2fbt_deq_end(struct render_info *thr,
		unsigned long long ts);

void fpsgo_base2fbt_node_init(struct render_info *obj);
void fpsgo_base2fbt_item_del(struct fbt_thread_loading *obj,
		struct fbt_thread_blc *pblc,
		struct fpsgo_loading *pdep,
		struct render_info *thr);
int fpsgo_base2fbt_get_max_blc_pid(void);
void fpsgo_comp2fbt_bypass_enq(void);
void fpsgo_comp2fbt_bypass_disconnect(void);
void fpsgo_base2fbt_set_bypass(int has_bypass);
void fpsgo_base2fbt_check_max_blc(void);
void fpsgo_base2fbt_no_one_render(void);
void fpsgo_base2fbt_only_bypass(void);
void fpsgo_fbt_set_min_cap(struct render_info *thr, int min_cap, int check);

int __init fbt_cpu_init(void);
void __exit fbt_cpu_exit(void);

int fpsgo_ctrl2fbt_switch_fbt(int enable);
int fbt_switch_ceiling(int value);

#else
static inline void fpsgo_ctrl2fbt_dfrc_fps(int fps_limit) { }
static inline void fpsgo_ctrl2fbt_cpufreq_cb(int cid,
		unsigned long freq) { }
static inline void fpsgo_ctrl2fbt_vsync(unsigned long long ts) { }
static inline void fpsgo_comp2fbt_frame_start(struct render_info *thr,
	unsigned long long ts) { }
static inline void fpsgo_comp2fbt_deq_end(struct render_info *thr,
		unsigned long long ts) { }

static inline int fbt_cpu_init(void) { return 0; }
static inline void fbt_cpu_exit(void) { }

static inline int fpsgo_ctrl2fbt_switch_fbt(int enable) { return 0; }

static inline void fpsgo_base2fbt_node_init(struct render_info *obj) { }
static inline void fpsgo_base2fbt_item_del(
		struct fbt_thread_loading *obj, struct fbt_thread_blc *pblc,
		struct fpsgo_loading *pdep,
		struct render_info *thr) { }
static inline int fpsgo_base2fbt_get_max_blc_pid(void) { return 0; }
static inline void fpsgo_comp2fbt_bypass_enq(void) { }
static inline void fpsgo_comp2fbt_bypass_disconnect(void) { }
static inline void fpsgo_base2fbt_set_bypass(int has_bypass) { }
static inline void fpsgo_base2fbt_check_max_blc(void) { }
static inline void fpsgo_base2fbt_no_one_render(void) { }
static inline void fpsgo_base2fbt_only_bypass(void) { }
static inline int fbt_switch_ceiling(int en) { return 0; }
static inline void fpsgo_fbt_set_min_cap(struct render_info *thr,
				int min_cap, int check) { }

#endif

#endif
