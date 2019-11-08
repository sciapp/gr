ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 2.0.0
OPENH264_EXTRA_MAKE_FLAGS ?=
ifeq ($(shell uname),Darwin)
OPENH264_EXTRA_MAKE_FLAGS += CFLAGS=-mmacosx-version-min=10.11
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

ifneq ($(shell nasm --version 2>/dev/null),)
ASM_OPTS := USE_ASM=Yes
else
ASM_OPTS := USE_ASM=No
endif

default: install

$(PREFIX)/src/libopenh264-$(VERSION).tar.gz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/libopenh264-$(VERSION).tar.gz

$(PREFIX)/src/libopenh264-$(VERSION)/Makefile: $(PREFIX)/src/libopenh264-$(VERSION).tar.gz
	cd $(PREFIX)/src/ && tar -xf libopenh264-$(VERSION).tar.gz && mv openh264-$(VERSION) libopenh264-$(VERSION)
	touch $@

$(PREFIX)/lib/libopenh264.a: $(PREFIX)/src/libopenh264-$(VERSION)/Makefile
	$(MAKE) -j4 -C $(PREFIX)/src/libopenh264-$(VERSION) install-static $(ASM_OPTS) PREFIX=$(PREFIX) $(OPENH264_EXTRA_MAKE_FLAGS)

install: $(PREFIX)/lib/libopenh264.a

.PHONY: default install
