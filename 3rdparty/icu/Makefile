SHELL := /bin/bash

ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 74.2
ICU_EXTRA_CONFIGURE_FLAGS ?=
ifeq ($(shell uname),Darwin)
ICU_EXTRA_CONFIGURE_FLAGS += CFLAGS=-mmacosx-version-min=10.15
ICU_EXTRA_CONFIGURE_FLAGS += CXXFLAGS=-mmacosx-version-min=10.15
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

ifeq ($(shell [[ "$(ARCH)" == "x86_64" || "$(ARCHS)" == "-arch x86_64" ]] && echo macos_cross),macos_cross)
default: run_macos_x86_64_cross_makefile
else ifeq ($(shell [[ "$(CC)" =~ -mingw ]] && echo mingw),mingw)
default: run_mingw_makefile
else
default: install
endif

run_macos_x86_64_cross_makefile:
	$(MAKE) -f Makefile.macos_x86_64_cross

run_mingw_makefile:
	$(MAKE) -f Makefile.mingw

$(PREFIX)/src/icu4c-$(VERSION).tar.gz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/icu4c-$(VERSION).tar.gz

$(PREFIX)/src/icu/source/configure: $(PREFIX)/src/icu4c-$(VERSION).tar.gz
	cd $(PREFIX)/src/ && tar -xf icu4c-$(VERSION).tar.gz

$(PREFIX)/src/icu/source/Makefile: $(PREFIX)/src/icu/source/configure
	cd $(PREFIX)/src/icu/source && \
	CFLAGS="-fPIC" \
	CXXFLAGS="-fPIC" \
	./configure \
		--prefix=$(PREFIX) \
		--enable-static \
		--disable-shared \
		$(ICU_EXTRA_CONFIGURE_FLAGS)
	touch $@

$(PREFIX)/lib/libicuuc.a: $(PREFIX)/src/icu/source/Makefile
	$(MAKE) -C $(PREFIX)/src/icu/source -j4
	$(MAKE) -C $(PREFIX)/src/icu/source install

libicuuc.a: $(PREFIX)/lib/libicuuc.a
	cp $(PREFIX)/lib/libicuuc.a libicuuc.a
	cp $(PREFIX)/lib/libicudata.a libicudata.a

clean:
	rm -f libicuuc.a
	rm -f libicudata.a

install: $(PREFIX)/lib/libicuuc.a

.PHONY: default clean install run_macos_x86_64_cross_makefile run_mingw_makefile
