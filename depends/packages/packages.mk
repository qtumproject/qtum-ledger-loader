packages:=

qt_packages = zlib

qrencode_packages = 

qt_linux_packages:=qt expat libxcb xcb_proto libXau xproto freetype fontconfig
qt_android_packages=qt

qt_darwin_packages=qt
qt_mingw32_packages=qt

bdb_packages=
sqlite_packages=

zmq_packages=

upnp_packages=

multiprocess_packages = 
multiprocess_native_packages = 

darwin_native_packages = native_biplist native_ds_store native_mac_alias

ifneq ($(build_os),darwin)
darwin_native_packages += native_cctools native_cdrkit native_libdmg-hfsplus
endif
