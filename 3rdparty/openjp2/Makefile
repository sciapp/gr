ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 2.0.0
OPENJP2_EXTRA_CMAKE_FLAGS ?=

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

ifeq ($(CMAKE_CMD),)
ifneq ($(shell cmake --version 2>/dev/null),)
CMAKE_CMD := cmake
CMAKE_DEP :=
endif
endif
ifeq ($(CMAKE_CMD),)
CMAKE_CMD := $(PREFIX)/bin/cmake
CMAKE_DEP := $(CMAKE_CMD)
endif

default: install

$(PREFIX)/src/openjpeg-$(VERSION).tar.gz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/openjpeg-$(VERSION).tar.gz

$(PREFIX)/src/openjpeg-$(VERSION)/CMakeLists.txt: $(PREFIX)/src/openjpeg-$(VERSION).tar.gz
	cd $(PREFIX)/src/ && tar -xf openjpeg-$(VERSION).tar.gz
	touch $@

$(PREFIX)/build/openjpeg-$(VERSION)/Makefile: $(PREFIX)/src/openjpeg-$(VERSION)/CMakeLists.txt $(CMAKE_DEP)
	mkdir -p $(PREFIX)/build/openjpeg-$(VERSION)
	cd $(PREFIX)/build/openjpeg-$(VERSION) && \
	$(CMAKE_CMD) \
		-DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} \
		-DBUILD_SHARED_LIBS=OFF \
		-DBUILD_CODEC=OFF \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
		$(OPENJP2_EXTRA_CMAKE_FLAGS) \
		$(PREFIX)/src/openjpeg-$(VERSION)/

$(PREFIX)/lib/libopenjp2.a: $(PREFIX)/build/openjpeg-$(VERSION)/Makefile
	make -C $(PREFIX)/build/openjpeg-$(VERSION) -j4
	make -C $(PREFIX)/build/openjpeg-$(VERSION) install

install: $(PREFIX)/lib/libopenjp2.a


$(PREFIX)/bin/cmake:
	make -C ../cmake PREFIX="$(PREFIX)" install

.PHONY: default install
