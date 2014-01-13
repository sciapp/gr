
#ifndef _VC_H_
#define _VC_H_

#ifdef __cplusplus
extern "C" {
#endif

#define __STDC_CONSTANT_MACROS
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <mupdf/fitz.h>

typedef struct movie_t_ *movie_t;
typedef struct frame_t_ *frame_t;
typedef struct pdf_t_ *pdf_t;

struct frame_t_ {
  unsigned char *data;
  int           width, height;
};

struct movie_t_ {
  AVCodec         *cdc;
  AVCodecContext  *cdc_ctx;
  AVFormatContext *fmt_ctx;
  AVOutputFormat  *out_fmt;
  AVStream        *video_st;
  AVFrame         *frame;
  int             num_frames;
  AVPicture       dst_picture;
  const char      *path;
};

struct pdf_t_ {
  fz_context   *ctx;
  fz_document  *doc;
  const char   *path;
  int          num_pages;
};

movie_t vc_movie_create(const char *path, int framerate, int bitrate);
void vc_movie_append_frame(movie_t movie, frame_t frame); 
void vc_movie_finish(movie_t movie); 
void vc_movie_free(movie_t);
void vc_frame_free(frame_t);

pdf_t vc_pdf_from_file(const char *path);
pdf_t vc_pdf_from_memory(unsigned char *data, int len);
int vc_pdf_get_number_of_pages(pdf_t pdf);
frame_t *vc_pdf_to_frames(pdf_t pdf, int width, int height);
void vc_pdf_close(pdf_t pdf);

#ifdef __cplusplus
}
#endif

#endif
