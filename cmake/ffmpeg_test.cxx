#ifdef __cplusplus
extern "C"
{
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

int main(void)
{
  AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_VP8);
  AVCodecContext *ctx = avcodec_alloc_context3(codec);
  avcodec_free_context(&ctx);
  return 0;
}
