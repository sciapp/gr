 TARGETS = jbig2dec jpeg openjpeg png zlib
  EXTRAS = mupdf ffmpeg
     DIR = 

default:
	@for d in $(TARGETS); do make -C $$d; done

extras:
	@for d in $(EXTRAS); do ( cd $$d && ./build.sh $(DIR) ); done
test:
	@echo "make EXTRA_CFLAGS=-I`pwd`/build/include EXTRA_CXXFLAGS=-I`pwd`/build/include EXTRA_LDFLAGS=-L`pwd`/build/lib"

clean:
	@for d in $(TARGETS); do make -C $$d clean; done
	rm -rf ./build
