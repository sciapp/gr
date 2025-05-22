# Replace this with a `CMakeLists.txt`

GRDIR = /usr/local/gr
INCLUDES = -I$(GRDIR)/include
CFLAGS = -Wall -g $(INCLUDES)# -fsanitize=address
LIBS = -L$(GRDIR)/lib -lGRM -lm
LDFLAGS = $(LIBS) -Wl,-rpath,$(GRDIR)/lib# -fsanitize=address

all: sender_line sender_stairs sender_lines sender_contour sender_shade sender_scatter sender_multiple_plots sender_multiple_plots_shade_contour sender_event_resize sender_heatmap sender_barplot sender_nonuniformheatmap sender_hist

sender_line: sender_line.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_lines: sender_lines.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_stairs: sender_stairs.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_shade: sender_shade.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_contour: sender_contour.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_scatter: sender_scatter.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_multiple_plots: sender_multiple_plots.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_multiple_plots_shade_contour: sender_multiple_plots_shade_contour.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_event_resize: sender_event_resize.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_heatmap: sender_heatmap.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_nonuniformheatmap: sender_nonuniformheatmap.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_barplot: sender_barplot.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender_hist: sender_hist.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $^

clean:
	rm -f sender_line sender_stairs sender_multiple_plots_shade_contour sender_lines sender_contour sender_shade sender_scatter sender_multiple_plots sender_event_resize sender_heatmap sender_barplot sender_nonuniformheatmap sender_hist *.o *.a *.so

.PHONY: all clean
