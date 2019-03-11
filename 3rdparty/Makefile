 TARGETS = freetype jpeg libpng16 zlib qhull
  EXTRAS = jbig2dec cmake openjp2 mupdf ogg theora vpx ffmpeg glfw zeromq pixman cairo
     DIR = 

default:
	@set -e; for d in $(TARGETS); do make -C $$d; done

extras:
	@set -e; for d in $(EXTRAS); do ( cd $$d && ./build.sh $(DIR) ); done

clean:
	@set -e; for d in $(TARGETS); do make -C $$d clean; done

realclean: clean
	rm -rf ./build
