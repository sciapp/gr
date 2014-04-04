  DIRS = jbig2dec jpeg openjpeg png zlib

default:
	@for d in $(DIRS); do make -C $$d; done
	
clean:
	@for d in $(DIRS); do make -C $$d clean; done
