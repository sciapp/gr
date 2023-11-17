SHELL := /bin/bash

ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 3.2.4
XERCES_C_EXTRA_CMAKE_FLAGS ?=
ifeq ($(shell uname),Darwin)
XERCES_C_EXTRA_CMAKE_FLAGS += -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
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

ifeq ($(notdir $(CC)),emcc)
# If building with Emscripten, ICU must not be built or linked explicitly. Instead, patch the Xerces-C++
# `CMakeLists.txt` to include the Emscripten port of ICU (`-s USE_ICU=1`)
ICU_LIBS =
else
ICU_LIBS = $(PREFIX)/lib/libicuuc.a
endif

default: install

$(PREFIX)/src/xerces-c-$(VERSION).tar.gz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/xerces-c-$(VERSION).tar.gz

$(PREFIX)/src/xerces-c-$(VERSION)/CMakeLists.txt: $(PREFIX)/src/xerces-c-$(VERSION).tar.gz
	cd $(PREFIX)/src/ && tar -xf xerces-c-$(VERSION).tar.gz
	cd $(PREFIX)/src/xerces-c-$(VERSION) && \
	( \
	echo 'diff --git a/CMakeLists.txt b/CMakeLists.txt'; \
	echo 'index 33bc40f..5de3dee 100644'; \
	echo '--- a/CMakeLists.txt'; \
	echo '+++ b/CMakeLists.txt'; \
	echo '@@ -164,10 +164,7 @@ install('; \
	echo '   COMPONENT "development")'; \
	echo; \
	echo ' # Process subdirectories'; \
	echo '-add_subdirectory(doc)'; \
	echo ' add_subdirectory(src)'; \
	echo '-add_subdirectory(tests)'; \
	echo '-add_subdirectory(samples)'; \
	echo; \
	echo ' # Display configuration summary'; \
	echo ' message(STATUS "")'; \
	echo 'diff --git a/cmake/XercesNetAccessorSelection.cmake b/cmake/XercesNetAccessorSelection.cmake'; \
	echo 'index 7a63f1f..723c394 100644'; \
	echo '--- a/cmake/XercesNetAccessorSelection.cmake'; \
	echo '+++ b/cmake/XercesNetAccessorSelection.cmake'; \
	echo '@@ -31,7 +31,6 @@ if(network)'; \
	echo ''; \
	echo '   # Requires select() which is UNIX only'; \
	echo '   if(UNIX)'; \
	echo '-    find_package(CURL)'; \
	echo '     if(CURL_FOUND)'; \
	echo '       list(APPEND netaccessors curl)'; \
	echo '     endif()'; \
	echo 'diff --git a/src/CMakeLists.txt b/src/CMakeLists.txt'; \
	echo 'index a168db1..50043ee 100644'; \
	echo '--- a/src/CMakeLists.txt'; \
	echo '+++ b/src/CMakeLists.txt'; \
	echo '@@ -1271,6 +1271,7 @@ set_target_properties(xerces-c-headers PROPERTIES FOLDER "Library")'; \
	echo ' add_library(xerces-c'; \
	echo '   $${libxerces_c_SOURCES}'; \
	echo '   $${libxerces_c_RESOURCES})'; \
	echo '+set_target_properties(xerces-c PROPERTIES POSITION_INDEPENDENT_CODE ON)'; \
	echo ' target_link_libraries(xerces-c PRIVATE $${libxerces_c_DEPS})'; \
	echo ' if(XERCES_USE_NETACCESSOR_CURL)'; \
	echo '   target_include_directories(xerces-c SYSTEM PRIVATE $${CURL_INCLUDE_DIRS})'; \
	) | patch -Np1
ifeq ($(notdir $(CC)),emcc)
	cd $(PREFIX)/src/xerces-c-$(VERSION) && \
	( \
	echo 'diff --git a/cmake/XercesICU.cmake b/cmake/XercesICU.cmake'; \
	echo 'index aecde10..dcef358 100644'; \
	echo '--- a/cmake/XercesICU.cmake'; \
	echo '+++ b/cmake/XercesICU.cmake'; \
	echo '@@ -19,4 +19,16 @@'; \
	echo; \
	echo ' # Determine if ICU is available'; \
	echo; \
	echo '-find_package(ICU COMPONENTS uc data)'; \
	echo '+set(USE_FLAGS -s USE_ICU=1)'; \
	echo '+'; \
	echo '+add_library(icu-uc INTERFACE)'; \
	echo '+target_compile_options(icu-uc INTERFACE $${USE_FLAGS})'; \
	echo '+target_link_options(icu-uc INTERFACE $${USE_FLAGS})'; \
	echo '+add_library(ICU::uc ALIAS icu-uc)'; \
	echo '+'; \
	echo '+add_library(icu-data INTERFACE)'; \
	echo '+target_compile_options(icu-data INTERFACE $${USE_FLAGS})'; \
	echo '+target_link_options(icu-data INTERFACE $${USE_FLAGS})'; \
	echo '+add_library(ICU::data ALIAS icu-data)'; \
	echo '+'; \
	echo '+set(ICU_FOUND "1")'; \
	echo 'diff --git a/src/CMakeLists.txt b/src/CMakeLists.txt'; \
	echo 'index a168db1..bfde8bf 100644'; \
	echo '--- a/src/CMakeLists.txt'; \
	echo '+++ b/src/CMakeLists.txt'; \
	echo '@@ -1316,7 +1316,7 @@ else()'; \
	echo '   set(xerces_config_dir "$${CMAKE_INSTALL_LIBDIR}/cmake/XercesC")'; \
	echo ' endif()'; \
	echo; \
	echo '-install(TARGETS xerces-c'; \
	echo '+install(TARGETS xerces-c icu-uc icu-data'; \
	echo '   EXPORT XercesCConfigInternal'; \
	echo '   RUNTIME DESTINATION $${CMAKE_INSTALL_BINDIR}'; \
	echo '   LIBRARY DESTINATION $${CMAKE_INSTALL_LIBDIR}'; \
	) | patch -Np1
endif

$(PREFIX)/build/xerces-c-$(VERSION)/Makefile: $(PREFIX)/src/xerces-c-$(VERSION)/CMakeLists.txt $(CMAKE_DEP) $(ICU_LIBS)
	mkdir -p $(PREFIX)/build/xerces-c-$(VERSION)
	cd $(PREFIX)/build/xerces-c-$(VERSION) && \
	$(CMAKE_CMD) \
		-DCMAKE_INSTALL_PREFIX=${PREFIX} \
		-DCMAKE_INSTALL_LIBDIR=lib \
		-DBUILD_SHARED_LIBS=OFF \
		-DCMAKE_BUILD_TYPE=Release \
		-DICU_ROOT=${PREFIX} \
		$(XERCES_C_EXTRA_CMAKE_FLAGS) \
		$(PREFIX)/src/xerces-c-$(VERSION)/

$(PREFIX)/lib/libxerces-c.a: $(PREFIX)/build/xerces-c-$(VERSION)/Makefile
	$(MAKE) -C $(PREFIX)/build/xerces-c-$(VERSION) -j4
	$(MAKE) -C $(PREFIX)/build/xerces-c-$(VERSION) install
	if [[ -f "$(PREFIX)/lib/cmake/XercesC/XercesCConfigInternal.cmake" ]]; then \
		cd "$(PREFIX)/lib/cmake/XercesC"; \
	elif [[ -f "$(PREFIX)/cmake/XercesCConfigInternal.cmake" ]]; then \
		cd "$(PREFIX)/cmake"; \
	else \
		exit 1; \
	fi; \
	( \
	echo '--- XercesCConfigInternal.cmake 2024-01-04 14:51:30.000000000 +0000'; \
	echo '+++ XercesCConfigInternal.cmake.patched 2024-01-04 15:20:05.000235499 +0000'; \
	echo '@@ -50,6 +50,16 @@'; \
	echo '   set(_IMPORT_PREFIX "")'; \
	echo ' endif()'; \
	echo; \
	echo '+add_library(ICU::uc STATIC IMPORTED)'; \
	echo '+set_target_properties(ICU::uc PROPERTIES'; \
	echo '+  IMPORTED_LOCATION "$${_IMPORT_PREFIX}/lib/libicuuc.a"'; \
	echo '+)'; \
	echo '+add_library(ICU::data STATIC IMPORTED)'; \
	echo '+set_target_properties(ICU::data PROPERTIES'; \
	echo '+  IMPORTED_LOCATION "$${_IMPORT_PREFIX}/lib/libicudata.a"'; \
	echo '+)'; \
	echo '+find_package(Threads)'; \
	echo '+'; \
	echo ' # Create imported target xerces_xerces-c'; \
	echo ' add_library(xerces_xerces-c STATIC IMPORTED)'; \
	echo; \
	) | patch -N

libxerces-c.a: $(PREFIX)/lib/libxerces-c.a
	cp $(PREFIX)/lib/libxerces-c.a libxerces-c.a

clean:
	rm -f libxerces-c.a

install: $(PREFIX)/lib/libxerces-c.a

$(PREFIX)/lib/libicuuc.a:
	$(MAKE) -C ../icu PREFIX="$(PREFIX)"

.PHONY: default clean install
