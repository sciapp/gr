BUILD_TYPE = Release
BUILD_DEMOS = OFF
EXPORT_COMPILE_COMMANDS = OFF
GRDIR = /usr/local/gr
UNAME := $(shell uname)
ifneq (,$(filter self,$(MAKECMDGOALS)))
  USE_BUNDLED_LIBRARIES = ON
else
  USE_BUNDLED_LIBRARIES = OFF
endif

PREFERRED_CLANG_FORMAT_VERSION="13"
ifeq ($(shell command -v "clang-format-$(PREFERRED_CLANG_FORMAT_VERSION)"),)
  CLANG_FORMAT="clang-format"
else
  CLANG_FORMAT="clang-format-$(PREFERRED_CLANG_FORMAT_VERSION)"
endif

default: all

all: build

pre-check:
	@lib/Precheck "${GRDIR}"  || \
	( echo "FATAL: Source and target directory are identical"; exit 1 )

configure: pre-check $(subst ON,bundled-libraries,$(filter ON,$(USE_BUNDLED_LIBRARIES)))
	cmake \
	  -DCMAKE_INSTALL_PREFIX="$(GRDIR)" \
	  -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
	  -DGR_BUILD_DEMOS="$(BUILD_DEMOS)" \
	  -DGR_USE_BUNDLED_LIBRARIES="$(USE_BUNDLED_LIBRARIES)" \
	  -DCMAKE_EXPORT_COMPILE_COMMANDS="$(EXPORT_COMPILE_COMMANDS)" \
	  -S . \
	  -B build

build:
	@if [ ! -d build ]; then \
	  $(MAKE) configure \
	    GRDIR="$(GRDIR)" \
	    BUILD_TYPE="$(BUILD_TYPE)" \
	    BUILD_DEMOS="$(BUILD_DEMOS)" \
	    USE_BUNDLED_LIBRARIES="$(USE_BUNDLED_LIBRARIES)" \
	    EXPORT_COMPILE_COMMANDS="$(EXPORT_COMPILE_COMMANDS)"; \
	fi
	cmake --build build -j

install:
	@if [ ! -d build ]; then \
	  $(MAKE) build \
	    GRDIR="$(GRDIR)" \
	    BUILD_TYPE="$(BUILD_TYPE)" \
	    BUILD_DEMOS="$(BUILD_DEMOS)" \
	    USE_BUNDLED_LIBRARIES="$(USE_BUNDLED_LIBRARIES)" \
	    EXPORT_COMPILE_COMMANDS="$(EXPORT_COMPILE_COMMANDS)"; \
	fi
	cmake --install build

clean:
	rm -rf build
	$(MAKE) -C 3rdparty clean
	rm -f gr.pkg

realclean: clean
	$(MAKE) -C 3rdparty realclean
	find packaging -type f \( -name '*.deb' -o -name '*.rpm' \) -exec rm \{\} \;
	rm -rf tmp

bundled-libraries:
	$(MAKE) -C 3rdparty default extras

self: build

osxpkg:
	mkdir -p tmp/bin tmp/gr
	rsync -a /usr/local/gr/ tmp/gr/
	ln -s ../gr/bin/gr tmp/bin/gr
	ln -s ../gr/Applications/glgr.app/Contents/MacOS/glgr tmp/bin/glgr
	ln -s ../gr/Applications/gksqt.app/Contents/MacOS/gksqt tmp/bin/gksqt
	@if [ -e ../gr/Applications/grplot.app/Contents/MacOS/grplot ]; then \
	ln -s ../gr/Applications/grplot.app/Contents/MacOS/grplot tmp/bin/grplot; fi
	sudo chown -R -h root:wheel tmp/
	pkgbuild --identifier de.fz-juelich.gr --root tmp --install-location /usr/local --ownership preserve gr.pkg
	sudo rm -rf tmp

code-format:
ifeq ($(UNAME), Darwin)
	@find -E . -type f \
	           -regex '.*\.(c|cpp|cxx|m|h|hpp|hxx)' \
	         ! -path './3rdparty/*' \
	         ! -path './apps/*' \
	         ! -path './build/*' \
	           -exec "$(CLANG_FORMAT)" -i -verbose -style=file {} \;
	@CMAKE_FORMAT="$$(./.setup_cmakeformat.sh)" && \
	find -E . -type f \
	          -regex '(.*/CMakeLists\.txt)|(.*\.cmake)' \
	        ! -path './3rdparty/*' \
	        ! -path './apps/*' \
	        ! -path './build/*' \
	          -exec echo "Formatting "{} \; \
	          -exec "$${CMAKE_FORMAT}" -i {} \;
else
	@find . -type f \
	        -regextype posix-extended \
	        -regex '.*\.(c|cpp|cxx|m|h|hpp|hxx)' \
	      ! -path './3rdparty/*' \
	      ! -path './apps/*' \
	      ! -path './build/*' \
	        -exec "$(CLANG_FORMAT)" -i -verbose -style=file {} \;
	@CMAKE_FORMAT="$$(./.setup_cmakeformat.sh)" && \
	find . -type f \
	       -regextype posix-extended \
	       -regex '(.*/CMakeLists\.txt)|(.*\.cmake)' \
	     ! -path './3rdparty/*' \
	     ! -path './apps/*' \
	     ! -path './build/*' \
	       -exec echo "Formatting "{} \; \
	       -exec "$${CMAKE_FORMAT}" -i {} \;
endif


.PHONY: default all pre-check configure build install clean realclean bundled-libraries self osxpkg code-format
