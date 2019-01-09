#
# Copyright (C) 2015-2016 Allwinner
#
# This is free software, licensed under the GNU General Public License v2.
# See /build/LICENSE for more information.

define KernelPackage/touchscreen-hyn_cst2xx
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=touchscreen hyn_cst2xx support
  DEPENDS:=+kmod-input-core
  FILES:=$(LINUX_DIR)/drivers/input/touchscreen/hyn_cst2xx/hyn_cst2xx.ko
  AUTOLOAD:=$(call AutoProbe,hyn_cst2xx)
endef

define KernelPackage/touchscreen-hyn_cst2xx/description
 Kernel modules for support touchscreen hyn_cst2xx
endef

$(eval $(call KernelPackage,touchscreen-hyn_cst2xx))

define KernelPackage/gsensor-da380
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=gsensor da380 support
  DEPENDS:=+kmod-input-core
  FILES:=$(LINUX_DIR)/drivers/hwmon/da380/da380.ko
  AUTOLOAD:=$(call AutoLoad,60,da380)
endef

define KernelPackage/gsensor-da380/description
 Kernel modules for support da380 gsensor
endef

$(eval $(call KernelPackage,gsensor-da380))

define KernelPackage/net-esp8089
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=esp8089 support
  FILES:=$(LINUX_DIR)/drivers/net/wireless/esp_prealloc/esp_prealloc.ko
  FILES+=$(LINUX_DIR)/drivers/net/wireless/esp8089/esp8089.ko
  AUTOLOAD:=$(call AutoLoad,50,esp_prealloc esp8089)
endef

define KernelPackage/net-esp8089/description
 Kernel modules for esp8089 support
endef

$(eval $(call KernelPackage,net-esp8089))