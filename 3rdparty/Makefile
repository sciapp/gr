UNAME := $(shell uname)

 TARGETS = freetype jpeg libpng16 zlib qhull bzip2
ifeq ($(UNAME), Darwin)
 TARGETS += zeromq
endif
  EXTRAS = agg tiff ogg theora vpx openh264 ffmpeg glfw zeromq pixman cairo icu xerces-c
     DIR =

default:
	@set -e; for d in $(TARGETS); do $(MAKE) -C $$d PREFIX=$(DIR); done

extras:
	@set -e; for d in $(EXTRAS); do $(MAKE) -C $$d PREFIX=$(DIR); done

clean:
	@set -e; for d in $(TARGETS); do $(MAKE) -C $$d clean; done

realclean: clean
	rm -rf ./build

.PHONY: default extras clean realclean
