/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __APUSYS_USER_H__
#define __APUSYS_USER_H__

/*
 * apusys user record all user related information
 *     for processing and debug
 * 1. process related:
 *      id: user id, struct's kernel address
 *      open_pid: process id which open dev node
 *      open_tgid: thread group which open dev node
 *
 * 2. cmd related:
 *      cmd_list: cmd linked list which record
 *          all working cmd user run from this fd
 *      cmd_mtx: mutex which cmd related operations use
 *
 * 3. memory related: record memory user allocate from this fd
 *      mem_list: mem linked list which record
 *          all memory user allocate from this fd
 *      mem_mtx: mutex which mem related operations use
 */

#include "apusys_drv.h"
#include "apusys_device.h"

struct apusys_user {
	/* basic info */
	uint64_t id;
	pid_t open_pid;
	pid_t open_tgid;

	//struct completion comp;

	/* relate cmd */
	struct list_head cmd_list;
	struct mutex cmd_mtx;

	/* memory mgt */
	struct list_head mem_list;
	struct mutex mem_mtx;

	/* acquired dev */
	struct list_head dev_list;
	struct mutex dev_mtx;

	/* secure acquired dev */
	struct list_head secdev_list;
	struct mutex secdev_mtx;

	/* for driver management only */
	struct list_head list;
};

void apusys_user_dump(void *s_file);
void apusys_user_show_log(void *s_file);
void apusys_user_record_log(void);
int apusys_user_insert_cmd(struct apusys_user *user, void *icmd);
int apusys_user_delete_cmd(struct apusys_user *user, void *icmd);
int apusys_user_get_cmd(struct apusys_user *user, void **icmd, uint64_t cmd_id);
int apusys_user_insert_dev(struct apusys_user *user, void *idev);
int apusys_user_delete_dev(struct apusys_user *user, void *idev);
struct apusys_dev_info *apusys_user_get_dev
	(struct apusys_user *user, uint64_t hnd);
int apusys_user_insert_secdev(struct apusys_user *user, void *idev_info);
int apusys_user_delete_secdev(struct apusys_user *user, void *idev_info);
int apusys_user_delete_sectype(struct apusys_user *u, int dev_type);
int apusys_user_insert_mem(struct apusys_user *user, struct apusys_kmem *mem);
int apusys_user_delete_mem(struct apusys_user *user, struct apusys_kmem *mem);
int apusys_create_user(struct apusys_user **user);
int apusys_delete_user(struct apusys_user *user);

int apusys_user_init(void);
void apusys_user_destroy(void);

#endif
