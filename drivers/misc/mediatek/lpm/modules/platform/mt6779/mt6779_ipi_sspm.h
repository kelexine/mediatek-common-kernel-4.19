/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __MT6779_IPI_SSPM__
#define __MT6779_IPI_SSPM__

int mt6779_sspm_notify_enter(unsigned int cmd);

int mt6779_sspm_notify_leave(unsigned int cmd);

int mt6779_sspm_notify_ansyc_enter(unsigned int cmd);

int mt6779_sspm_notify_ansyc_enter_respone(void);

int mt6779_sspm_notify_ansyc_leave(unsigned int cmd);

int mt6779_sspm_notify_ansyc_leave_respone(void);




#endif

