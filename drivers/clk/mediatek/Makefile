# SPDX-License-Identifier: GPL-2.0

ifeq ($(CONFIG_MTK_GCOV_KERNEL),y)
GCOV_PROFILE := y
endif

obj-$(CONFIG_COMMON_CLK_MEDIATEK) += clk-mtk.o clk-pll.o clk-gate.o clk-apmixed.o clk-cpumux.o reset.o clk-mux.o clkchk.o
obj-$(CONFIG_COMMON_CLK_MT6779) += clk-mt6779.o
obj-$(CONFIG_COMMON_CLK_MT6779_MMSYS) += clk-mt6779-mm.o
obj-$(CONFIG_COMMON_CLK_MT6779_IMGSYS) += clk-mt6779-img.o
obj-$(CONFIG_COMMON_CLK_MT6779_IPESYS) += clk-mt6779-ipe.o
obj-$(CONFIG_COMMON_CLK_MT6779_CAMSYS) += clk-mt6779-cam.o
obj-$(CONFIG_COMMON_CLK_MT6779_VDECSYS) += clk-mt6779-vdec.o
obj-$(CONFIG_COMMON_CLK_MT6779_VENCSYS) += clk-mt6779-venc.o
obj-$(CONFIG_COMMON_CLK_MT6779_MFGCFG) += clk-mt6779-mfg.o
obj-$(CONFIG_COMMON_CLK_MT6779_AUDSYS) += clk-mt6779-aud.o
obj-$(CONFIG_COMMON_CLK_MT6779_APUVCORESYS) += clk-mt6779-apuvcore.o
obj-$(CONFIG_COMMON_CLK_MT6779_APUCONNSYS) += clk-mt6779-apuconn.o
obj-$(CONFIG_COMMON_CLK_MT6779_APU0SYS) += clk-mt6779-apu0.o
obj-$(CONFIG_COMMON_CLK_MT6779_APU1SYS) += clk-mt6779-apu1.o
obj-$(CONFIG_COMMON_CLK_MT6779_MDLASYS) += clk-mt6779-mdla.o
obj-$(CONFIG_COMMON_CLK_MT6797) += clk-mt6797.o
obj-$(CONFIG_COMMON_CLK_MT6797_IMGSYS) += clk-mt6797-img.o
obj-$(CONFIG_COMMON_CLK_MT6797_MMSYS) += clk-mt6797-mm.o
obj-$(CONFIG_COMMON_CLK_MT6797_VDECSYS) += clk-mt6797-vdec.o
obj-$(CONFIG_COMMON_CLK_MT6797_VENCSYS) += clk-mt6797-venc.o
obj-$(CONFIG_COMMON_CLK_MT2701) += clk-mt2701.o
obj-$(CONFIG_COMMON_CLK_MT2701_AUDSYS) += clk-mt2701-aud.o
obj-$(CONFIG_COMMON_CLK_MT2701_BDPSYS) += clk-mt2701-bdp.o
obj-$(CONFIG_COMMON_CLK_MT2701_ETHSYS) += clk-mt2701-eth.o
obj-$(CONFIG_COMMON_CLK_MT2701_G3DSYS) += clk-mt2701-g3d.o
obj-$(CONFIG_COMMON_CLK_MT2701_HIFSYS) += clk-mt2701-hif.o
obj-$(CONFIG_COMMON_CLK_MT2701_IMGSYS) += clk-mt2701-img.o
obj-$(CONFIG_COMMON_CLK_MT2701_MMSYS) += clk-mt2701-mm.o
obj-$(CONFIG_COMMON_CLK_MT2701_VDECSYS) += clk-mt2701-vdec.o
obj-$(CONFIG_COMMON_CLK_MT2712) += clk-mt2712.o clkchk-mt2712.o
obj-$(CONFIG_COMMON_CLK_MT2712_BDPSYS) += clk-mt2712-bdp.o
obj-$(CONFIG_COMMON_CLK_MT2712_IMGSYS) += clk-mt2712-img.o
obj-$(CONFIG_COMMON_CLK_MT2712_JPGDECSYS) += clk-mt2712-jpgdec.o
obj-$(CONFIG_COMMON_CLK_MT2712_MFGCFG) += clk-mt2712-mfg.o
obj-$(CONFIG_COMMON_CLK_MT2712_MMSYS) += clk-mt2712-mm.o
obj-$(CONFIG_COMMON_CLK_MT2712_VDECSYS) += clk-mt2712-vdec.o
obj-$(CONFIG_COMMON_CLK_MT2712_VENCSYS) += clk-mt2712-venc.o
obj-$(CONFIG_COMMON_CLK_MT7622) += clk-mt7622.o
obj-$(CONFIG_COMMON_CLK_MT7622_ETHSYS) += clk-mt7622-eth.o
obj-$(CONFIG_COMMON_CLK_MT7622_HIFSYS) += clk-mt7622-hif.o
obj-$(CONFIG_COMMON_CLK_MT7622_AUDSYS) += clk-mt7622-aud.o
obj-$(CONFIG_COMMON_CLK_MT7629) += clk-mt7629.o
obj-$(CONFIG_COMMON_CLK_MT7629_ETHSYS) += clk-mt7629-eth.o
obj-$(CONFIG_COMMON_CLK_MT7629_HIFSYS) += clk-mt7629-hif.o
obj-$(CONFIG_COMMON_CLK_MT8135) += clk-mt8135.o
obj-$(CONFIG_COMMON_CLK_MT8168) += clk-mt8168.o clk-fixup-div.o clkchk-mt8168.o
obj-$(CONFIG_COMMON_CLK_MT8168_APU) += clk-mt8168-apu.o
obj-$(CONFIG_COMMON_CLK_MT8168_CAMSYS) += clk-mt8168-cam.o
obj-$(CONFIG_COMMON_CLK_MT8168_MFGCFG) += clk-mt8168-mfg.o
obj-$(CONFIG_COMMON_CLK_MT8168_MMSYS) += clk-mt8168-mm.o
obj-$(CONFIG_COMMON_CLK_MT8168_VDECSYS) += clk-mt8168-vdec.o
obj-$(CONFIG_COMMON_CLK_MT8168_VENCSYS) += clk-mt8168-venc.o
obj-$(CONFIG_COMMON_CLK_MT8173) += clk-mt8173.o
ifeq ($(CONFIG_DEBUG_FS), y)
obj-$(CONFIG_COMMON_CLK_MEDIATEK) += clkdbg.o
obj-$(CONFIG_COMMON_CLK_MT2712) += clkdbg-mt2712.o
obj-$(CONFIG_COMMON_CLK_MT8168) += clkdbg-mt8168.o
endif
obj-$(CONFIG_COMMON_CLK_MTK_FREQ_HOPPING) += fhctl.o
fhctl-y += clk-fhctl.o
fhctl-$(CONFIG_COMMON_CLK_MTK_FREQ_HOPPING_DEBUG) += clk-fhctl-debug.o
ifneq ($(CONFIG_COMMON_CLK_MTK_FREQ_HOPPING_SUBSYS),y)
fhctl-y += clk-fhctl-ap.o
else
fhctl-$(CONFIG_COMMON_CLK_MTK_FREQ_HOPPING_SSPM) += clk-fhctl-sspm.o
endif
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sspm/
