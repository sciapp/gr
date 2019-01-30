#ifndef _VC_H_
#define _VC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define __STDC_CONSTANT_MACROS
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

  struct frame_t_
  {
    unsigned char *data;
    int width, height;
  };

  struct movie_t_
  {
    AVCodec *cdc;
    AVCodecContext *cdc_ctx;
    AVFormatContext *fmt_ctx;
    AVOutputFormat *out_fmt;
    AVStream *video_st;
    AVFrame *frame;
    int num_frames;
    AVPicture dst_picture;
    const char *path;
  };

  typedef struct movie_t_ *movie_t;
  typedef struct frame_t_ *frame_t;

  movie_t vc_movie_create(const char *path, int framerate, int bitrate);
  void vc_movie_append_frame(movie_t movie, frame_t frame);
  void vc_movie_finish(movie_t movie);
  void vc_movie_free(movie_t);
  void vc_frame_free(frame_t);

#ifdef __cplusplus
}
#endif

#endif
