/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef EARA_THRM_PB_H
#define EARA_THRM_PB_H

struct thrm_pb_ratio {
	int ratio;
	int vpu_power;
	int mdla_power;
};

extern struct ppm_cobra_data *ppm_cobra_pass_tbl(void);
extern void eara_pass_perf_first_hint(int enable);

extern void (*eara_thrm_gblock_bypass_fp)(int pid, int bypass);
extern void (*eara_thrm_frame_start_fp)(int pid,
	int cpu_time, int vpu_time, int mdla_time,
	int cpu_cap, int vpu_cap, int mdla_cap,
	int queuefps, unsigned long long q2q_time,
	int AI_cross_vpu, int AI_cross_mdla, int AI_bg_vpu,
	int AI_bg_mdla, ktime_t cur_time);
extern void (*eara_thrm_enqueue_end_fp)(int pid, int gpu_time, int gpu_freq,
	unsigned long long enq);

void eara_thrm_update_gpu_info(int *input_opp_num, int *in_max_opp_idx,
		struct mt_gpufreq_power_table_info **gpu_tbl,
		struct thrm_pb_ratio **opp_ratio);
int eara_thrm_get_vpu_core_num(void);
int eara_thrm_get_mdla_core_num(void);

#endif
