#
# Copyright (C) 2015 MediaTek Inc.
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

ccflags-y += -I$(srctree)/drivers/gpu/mediatek/gpufreq/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/thermal/mt6779/inc/mach/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/power_throttling/

obj-$(CONFIG_MTK_GPU_SUPPORT) += mtk_gpufreq_mt6779.o

mtk_gpufreq_mt6779-y += mtk_gpufreq_core.o

