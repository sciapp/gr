GRDIR = /usr/local/gr
INCLUDES = -I..
CFLAGS = $(INCLUDES) -std=c89 -Wall -Wextra -Wpedantic -Wno-unused-parameter -g
LIBS = -L$(GRDIR)/lib -lGR -lm
LDFLAGS = $(LIBS) -Wl,-rpath,$(GRDIR)/lib


all: plot receiver sender custom_receiver custom_sender

plot: plot.o
	$(CC) -o $@ $^ $(LDFLAGS)

receiver: receiver.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender: sender.o
	$(CC) -o $@ $^ $(LDFLAGS)

custom_receiver: custom_receiver.o
	$(CC) -o $@ $^ $(LDFLAGS)

custom_sender: custom_sender.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $^

clean:
	rm -f plot receiver sender custom_receiver custom_sender custom_sender.out *.o *.a *.so

.PHONY: all clean
