
# Copyright (C) 2017 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

ccflags-y += \
	-I$(srctree)/drivers/misc/mediatek/include/mt-plat/	\
	-I$(srctree)/drivers/misc/mediatek/include/mt-plat/upower_v2	\
	-I$(srctree)/drivers/misc/mediatek/base/power/include/	\
	-I$(srctree)/drivers/misc/mediatek/base/power/qos/$(CONFIG_MTK_PLATFORM)	\
	-I$(srctree)/drivers/misc/mediatek/base/power/include/upower_v2	\
	-I$(srctree)/drivers/misc/mediatek/eem_v2/$(CONFIG_MTK_PLATFORM)/inc	\
	-I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(CONFIG_MTK_PLATFORM)/include/mach	\

ifeq ($(CONFIG_MTK_TINYSYS_SSPM_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sspm/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sspm/$(CONFIG_MTK_PLATFORM)
ccflags-y += -I$(srctree)/drivers/devfreq
endif

obj-y += mtk_unified_power.o
obj-y += mtk_unified_power_internal.o
obj-y += mtk_unified_power_api.o
obj-y += mtk_unified_power_platform.o

obj-$(CONFIG_MACH_MT6758) += mtk_unified_power_mt6758.o
obj-$(CONFIG_MACH_MT6765) += mtk_unified_power_mt6765.o
obj-$(CONFIG_MACH_MT6761) += mtk_unified_power_mt6761.o
obj-$(CONFIG_MACH_MT3967) += mtk_unified_power_mt3967.o
obj-$(CONFIG_MACH_MT6779) += mtk_unified_power_mt6779.o
obj-$(CONFIG_MACH_MT6768) += mtk_unified_power_mt6768.o
obj-$(CONFIG_MACH_MT6785) += mtk_unified_power_mt6785.o
