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

MTK_PLATFORM:= $(subst ",,$(CONFIG_MTK_DISP_PLATFORM))
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/$(MTK_PLATFORM)
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/aee/mrdump
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/aee/aed
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/aee
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/include
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/mmp/
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/log_store/
ifdef CONFIG_MTK_ION
subdir-ccflags-y += -I$(srctree)/drivers/staging/android/ion
endif

obj-$(CONFIG_MTK_AEE_FEATURE)	+= aed/
obj-$(CONFIG_MTK_AEE_FEATURE)	+= mrdump/

