include $(TOPDIR)/rules.mk

PKG_NAME:=device_inhibitor
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/device_inhibitor
	CATEGORY:=Base system
	TITLE:=device_inhibitor
	DEPENDS:=+tuya-iot-core-main +libargp +libubus +libubox +libblobmsg-json
endef

define Package/device_inhibitor/description
	My lil' ol' device inhibitor
endef

define Package/device_inhibitor/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/device_inhibitor $(1)/usr/bin
	$(INSTALL_BIN) ./files/device_inhibitor.init $(1)/etc/init.d/device_inhibitor
	$(INSTALL_CONF) ./files/device_inhibitor.config $(1)/etc/config/device_inhibitor
endef

$(eval $(call BuildPackage,device_inhibitor))
