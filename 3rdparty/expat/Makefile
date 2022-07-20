ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 2.4.8
EXPAT_EXTRA_CONFIGURE_FLAGS ?=
ifeq ($(shell uname),Darwin)
EXPAT_EXTRA_CONFIGURE_FLAGS += CFLAGS=-mmacosx-version-min=10.14
endif

ifeq ($(DOWNLOAD_CMD),)
ifneq ($(shell curl --version 2>/dev/null),)
DOWNLOAD_CMD := curl -f -k -OL
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

$(PREFIX)/src/expat-$(VERSION).tar.xz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/expat-$(VERSION).tar.xz

$(PREFIX)/src/expat-$(VERSION)/configure: $(PREFIX)/src/expat-$(VERSION).tar.xz
	cd $(PREFIX)/src/ && tar -xf expat-$(VERSION).tar.xz
	touch $@

$(PREFIX)/src/expat-$(VERSION)/Makefile: $(PREFIX)/src/expat-$(VERSION)/configure
	cd $(PREFIX)/src/expat-$(VERSION) && \
	./configure \
		--prefix=$(PREFIX) \
		--libdir=$(PREFIX)/lib \
		--enable-static \
		--disable-shared \
		--with-pic \
		$(EXPAT_EXTRA_CONFIGURE_FLAGS)
	# configure doesn't update the Makefile mtime correctly
	touch $@

$(PREFIX)/lib/libexpat.a: $(PREFIX)/src/expat-$(VERSION)/Makefile
	$(MAKE) -C $(PREFIX)/src/expat-$(VERSION) -j4 && \
	$(MAKE) -C $(PREFIX)/src/expat-$(VERSION) install

install: $(PREFIX)/lib/libexpat.a

.PHONY: default install
