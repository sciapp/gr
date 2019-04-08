ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 2.8.12.2
CMAKE_EXTRA_BOOTSTRAP_FLAGS ?=

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

$(PREFIX)/src/cmake-$(VERSION).tar.gz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/cmake-$(VERSION).tar.gz

$(PREFIX)/src/cmake-$(VERSION)/bootstrap: $(PREFIX)/src/cmake-$(VERSION).tar.gz
	cd $(PREFIX)/src/ && tar -xf cmake-$(VERSION).tar.gz
	touch $@

$(PREFIX)/src/cmake-$(VERSION)/Makefile: $(PREFIX)/src/cmake-$(VERSION)/bootstrap
	cd $(PREFIX)/src/cmake-$(VERSION) && \
	./bootstrap \
		--prefix=$(PREFIX) \
		--parallel=4 \
		$(CMAKE_EXTRA_BOOTSTRAP_FLAGS)

$(PREFIX)/bin/cmake: $(PREFIX)/src/cmake-$(VERSION)/Makefile
	$(MAKE) -C $(PREFIX)/src/cmake-$(VERSION) -j4
	$(MAKE) -C $(PREFIX)/src/cmake-$(VERSION) install

install: $(PREFIX)/bin/cmake

.PHONY: default install
