#
_XDCBUILDCOUNT = 0
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/ti/tirtos_c2000_2_16_01_14/packages;C:/ti/tirtos_c2000_2_16_01_14/products/tidrivers_c2000_2_16_01_13/packages;C:/ti/tirtos_c2000_2_16_01_14/products/bios_6_45_02_31/packages;C:/ti/tirtos_c2000_2_16_01_14/products/ndk_2_25_00_09/packages;C:/ti/tirtos_c2000_2_16_01_14/products/uia_2_00_05_50/packages;C:/ti/ccsv6/ccs_base;C:/Users/Chris/DOCUME~1/BUCKEY~1/BATTER~1/Software/BATTER~1/.config
override XDCROOT = C:/ti/xdctools_3_31_00_24_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/ti/tirtos_c2000_2_16_01_14/packages;C:/ti/tirtos_c2000_2_16_01_14/products/tidrivers_c2000_2_16_01_13/packages;C:/ti/tirtos_c2000_2_16_01_14/products/bios_6_45_02_31/packages;C:/ti/tirtos_c2000_2_16_01_14/products/ndk_2_25_00_09/packages;C:/ti/tirtos_c2000_2_16_01_14/products/uia_2_00_05_50/packages;C:/ti/ccsv6/ccs_base;C:/Users/Chris/DOCUME~1/BUCKEY~1/BATTER~1/Software/BATTER~1/.config;C:/ti/xdctools_3_31_00_24_core/packages;..
HOSTOS = Windows
endif
