
ccflags-y += -I$(srctree)/drivers/misc/mediatek/dcm/include/
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include/dcm_v1/$(MTK_PLATFORM)/
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/$(MTK_PLATFORM)/
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/dramc/$(MTK_PLATFORM)/
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/dcm/include/

# Core
obj-$(CONFIG_MTK_DCM_COMMON)	+= mtk_dcm.o

# SoC Drivers
obj-$(CONFIG_MTK_DCM_MT6779) += mt6779_dcm.o
mt6779_dcm-objs := mt6779_dcm_internal.o mt6779_dcm_autogen.o

