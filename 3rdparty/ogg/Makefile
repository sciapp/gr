ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 1.3.2
OGG_EXTRA_CONFIGURE_FLAGS ?=

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

$(PREFIX)/src/libogg-$(VERSION).tar.gz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/libogg-$(VERSION).tar.gz

$(PREFIX)/src/libogg-$(VERSION)/configure: $(PREFIX)/src/libogg-$(VERSION).tar.gz
	cd $(PREFIX)/src/ && tar -xf libogg-$(VERSION).tar.gz
	touch $@

$(PREFIX)/src/libogg-$(VERSION)/Makefile: $(PREFIX)/src/libogg-$(VERSION)/configure
	cd $(PREFIX)/src/libogg-$(VERSION) && \
	./configure \
		--prefix=$(PREFIX) \
		--libdir=$(PREFIX)/lib \
		--enable-static \
		--disable-shared \
		--with-pic \
		--disable-dependency-tracking \
		$(OGG_EXTRA_CONFIGURE_FLAGS)

$(PREFIX)/lib/libogg.a: $(PREFIX)/src/libogg-$(VERSION)/Makefile
	make -C $(PREFIX)/src/libogg-$(VERSION) -j4
	make -C $(PREFIX)/src/libogg-$(VERSION) install

install: $(PREFIX)/lib/libogg.a

.PHONY: default install