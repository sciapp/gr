GRDIR = /usr/local/gr
INCLUDES = -I..
CFLAGS = $(INCLUDES) -std=c89 -Wall -Wextra -Wpedantic -Wno-unused-parameter -g
LIBS = -L$(GRDIR)/lib -lGR
LDFLAGS = $(LIBS) -Wl,-rpath,$(GRDIR)/lib


all: receiver sender

receiver: receiver.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender: sender.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $^

clean:
	rm -f receiver sender *.o *.a *.so

.PHONY: all clean
