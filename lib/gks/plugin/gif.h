#ifndef GIF_H
#define GIF_H

#define FORMAT_RGB (3)
#define FORMAT_RGBA (4)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  FILE *fp;
  int width;
  int height;
} gif_writer;


/* Open a GIF file for writing. */
void gif_open(gif_writer *gwp, const char *file_name);
/* Write the image rgb_image with the given width, height and format (either FORMAT_RGB or FORMAT_RGBA) into the GIF file and wait delay centiseconds before showing the next frame. */
void gif_write(gif_writer *gwp, const unsigned char *rgb_image, unsigned short width, unsigned short height, int format, int delay);
/* Close the GIF file. */
void gif_close(gif_writer *gwp);

#ifdef __cplusplus
  }
#endif

#endif
