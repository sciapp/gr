ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 1.4.0
VPX_EXTRA_CONFIGURE_FLAGS ?=
ifeq ($(shell uname),Darwin)
VPX_EXTRA_CONFIGURE_FLAGS += --extra-cflags=-mmacosx-version-min=10.11
endif

ifeq ($(DOWNLOAD_CMD),)
ifneq ($(shell curl --version 2>/dev/null),)
DOWNLOAD_CMD := curl -k -OL
endif
endif
ifeq ($(DOWNLOAD_CMD),)
ifneq ($(shell wget --version 2>/dev/null),)
DOWNLOAD_CMD := wget --no-check-certificate
endif
endif
ifeq ($(DOWNLOAD_CMD),)
DOWNLOAD_CMD := echo "Error: Unable to find curl or wget."; exit 1; \#
endif

default: install

$(PREFIX)/src/libvpx-$(VERSION).tar.bz2:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/libvpx-$(VERSION).tar.bz2

$(PREFIX)/src/libvpx-$(VERSION)/configure: $(PREFIX)/src/libvpx-$(VERSION).tar.bz2
	cd $(PREFIX)/src/ && tar -xf libvpx-$(VERSION).tar.bz2
	touch $@

$(PREFIX)/src/libvpx-$(VERSION)/Makefile: $(PREFIX)/src/libvpx-$(VERSION)/configure
	cd $(PREFIX)/src/libvpx-$(VERSION) && \
	./configure \
		--prefix=$(PREFIX) \
		--libdir=$(PREFIX)/lib \
		--enable-static \
		--disable-shared \
		--enable-pic \
		--disable-dependency-tracking \
		--disable-unit-tests \
		--disable-examples \
		--disable-docs \
		--enable-vp8 \
		--disable-vp9 \
		--target=generic-gnu \
		$(VPX_EXTRA_CONFIGURE_FLAGS)

$(PREFIX)/lib/libvpx.a: $(PREFIX)/src/libvpx-$(VERSION)/Makefile
	$(MAKE) -C $(PREFIX)/src/libvpx-$(VERSION) -j4
	$(MAKE) -C $(PREFIX)/src/libvpx-$(VERSION) install

install: $(PREFIX)/lib/libvpx.a


.PHONY: default install
