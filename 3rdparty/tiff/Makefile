ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 4.0.10
TIFF_EXTRA_CONFIGURE_FLAGS ?=
ifeq ($(shell uname),Darwin)
 TIFF_EXTRA_CONFIGURE_FLAGS += CFLAGS=-mmacosx-version-min=10.11
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

$(PREFIX)/src/tiff-$(VERSION).tar.gz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/tiff-$(VERSION).tar.gz

$(PREFIX)/src/tiff-$(VERSION)/configure: $(PREFIX)/src/tiff-$(VERSION).tar.gz
	cd $(PREFIX)/src/ && tar -xf tiff-$(VERSION).tar.gz
	touch $@

$(PREFIX)/src/tiff-$(VERSION)/Makefile: $(PREFIX)/src/tiff-$(VERSION)/configure
	cd $(PREFIX)/src/tiff-$(VERSION) && \
	./configure \
		--prefix=$(PREFIX) \
		--libdir=$(PREFIX)/lib \
		--enable-static \
		--disable-shared \
		--with-pic \
		--disable-dependency-tracking \
		--with-zlib-include-dir=$(PREFIX)/include/ \
		--with-zlib-lib-dir=$(PREFIX)/lib/ \
		--disable-largefile \
		--disable-ccitt \
		--disable-packbits \
		--disable-lzw \
		--disable-thunder \
		--disable-next \
		--disable-logluv \
		--disable-mdi \
		--disable-pixarlog \
		--disable-jpeg \
		--disable-old-jpeg \
		--disable-jbig \
		--disable-lzma \
		--disable-zstd \
		--disable-webp \
		$(TIFF_EXTRA_CONFIGURE_FLAGS)

$(PREFIX)/lib/libtiff.a: $(PREFIX)/src/tiff-$(VERSION)/Makefile
	$(MAKE) -C $(PREFIX)/src/tiff-$(VERSION) -j4
	$(MAKE) -C $(PREFIX)/src/tiff-$(VERSION) install

install: $(PREFIX)/lib/libtiff.a

.PHONY: default install
