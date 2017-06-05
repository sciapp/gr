GRDIR = /usr/local/gr
INCLUDES = -I..
CFLAGS = $(INCLUDES) -std=c89 -Wall -Wextra -Wno-unused-parameter -g
LIBS = -L$(GRDIR)/lib -lGR
LDFLAGS = $(LIBS) -Wl,-rpath,$(GRDIR)/lib


all: main

main: main.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $^

clean:
	rm -f main *.o *.a *.so

.PHONY: all clean
