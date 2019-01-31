#if !defined(NO_AV)

#include <stdio.h>

#include "vc.h"

movie_t vc_movie_create(const char *path, int framerate, int bitrate)
{
  movie_t movie = (movie_t)malloc(sizeof(struct movie_t_));

  av_register_all();
  av_log_set_level(AV_LOG_QUIET);

  movie->path = path;

  avformat_alloc_output_context2(&movie->fmt_ctx, NULL, NULL, movie->path);

  movie->out_fmt = movie->fmt_ctx->oformat;
  movie->video_st = NULL;

  if (movie->out_fmt->video_codec == AV_CODEC_ID_NONE)
    {
      fprintf(stderr, "codec not found from given path\n");
      exit(1);
    }

  movie->cdc = avcodec_find_encoder(movie->out_fmt->video_codec);
  if (!(movie->cdc))
    {
      fprintf(stderr, "codec not found\n");
      exit(1);
    }

  movie->video_st = avformat_new_stream(movie->fmt_ctx, movie->cdc);
  if (!movie->video_st)
    {
      fprintf(stderr, "Could not alloc stream\n");
      exit(1);
    }

  movie->cdc_ctx = movie->video_st->codec;

  movie->cdc_ctx->codec_id = movie->out_fmt->video_codec;
  movie->cdc_ctx->bit_rate = bitrate;
  movie->cdc_ctx->time_base.num = 1;
  movie->cdc_ctx->time_base.den = framerate;
  movie->cdc_ctx->gop_size = 12;
  movie->cdc_ctx->pix_fmt = PIX_FMT_YUV420P;
  movie->num_frames = 0;

  return movie;
}

void vc_movie_append_frame(movie_t movie, frame_t frame)
{
  int got_output, ret;
  struct SwsContext *sws_ctx;
  AVPacket pkt;
  AVPicture src_picture;

  if (movie->num_frames == 0)
    {

      movie->cdc_ctx->width = frame->width;
      movie->cdc_ctx->height = frame->height;

      if (movie->fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) movie->cdc_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;

      if (avcodec_open2(movie->cdc_ctx, movie->cdc, NULL) < 0)
        {
          fprintf(stderr, "could not open video codec\n");
          exit(1);
        }

      movie->frame = avcodec_alloc_frame();
      if (!movie->frame)
        {
          fprintf(stderr, "could not allocate video frame\n");
          exit(1);
        }

      ret =
          avpicture_alloc(&movie->dst_picture, movie->cdc_ctx->pix_fmt, movie->cdc_ctx->width, movie->cdc_ctx->height);
      if (ret < 0)
        {
          fprintf(stderr, "could not allocate picture\n");
          exit(1);
        }

      *((AVPicture *)movie->frame) = movie->dst_picture;

      av_dump_format(movie->fmt_ctx, 0, movie->path, 1);
      if (!(movie->out_fmt->flags & AVFMT_NOFILE))
        {
          if (avio_open(&movie->fmt_ctx->pb, movie->path, AVIO_FLAG_WRITE) < 0)
            {
              fprintf(stderr, "could not open '%s'\n", movie->path);
              exit(1);
            }
        }

      if (avformat_write_header(movie->fmt_ctx, NULL) < 0)
        {
          fprintf(stderr, "error occurred when opening output file\n");
          exit(1);
        }

      movie->frame->pts = 0;
    }

  avpicture_fill(&src_picture, frame->data, PIX_FMT_RGBA, movie->cdc_ctx->width, movie->cdc_ctx->height);
  src_picture.data[0] = frame->data;

  sws_ctx = sws_getContext(movie->cdc_ctx->width, movie->cdc_ctx->height, PIX_FMT_RGBA, movie->cdc_ctx->width,
                           movie->cdc_ctx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

  if (!sws_ctx)
    {
      fprintf(stderr, "could not initialize the conversion context\n");
      exit(1);
    }

  sws_scale(sws_ctx, (const uint8_t *const *)src_picture.data, src_picture.linesize, 0, movie->cdc_ctx->height,
            movie->dst_picture.data, movie->dst_picture.linesize);

  av_init_packet(&pkt);

  if (movie->fmt_ctx->oformat->flags & AVFMT_RAWPICTURE)
    {
      pkt.flags |= AV_PKT_FLAG_KEY;
      pkt.stream_index = movie->video_st->index;
      pkt.data = movie->dst_picture.data[0];
      pkt.size = sizeof(AVPicture);

      av_write_frame(movie->fmt_ctx, &pkt);
    }
  else
    {
      pkt.data = NULL;
      pkt.size = 0;

      ret = avcodec_encode_video2(movie->cdc_ctx, &pkt, movie->frame, &got_output);
      if (ret < 0)
        {
          fprintf(stderr, "Error encoding video frame\n");
          exit(1);
        }

      if (got_output)
        {
          if (movie->cdc_ctx->coded_frame->key_frame) pkt.flags |= AV_PKT_FLAG_KEY;

          pkt.stream_index = movie->video_st->index;
          ret = av_write_frame(movie->fmt_ctx, &pkt);
        }
      else
        {
          ret = 0;
        }

      if (ret != 0)
        {
          fprintf(stderr, "Error while writing video frame\n");
          exit(1);
        }
    }
  av_free_packet(&pkt);
  sws_freeContext(sws_ctx);
  movie->frame->pts += av_rescale_q(1, movie->video_st->codec->time_base, movie->video_st->time_base);
  movie->num_frames++;
}

void vc_frame_free(frame_t frame)
{
  free(frame->data);
  free(frame);
}

void vc_movie_finish(movie_t movie)
{
  av_write_trailer(movie->fmt_ctx);

  if (movie->video_st)
    {
      avcodec_close(movie->video_st->codec);

      av_free(movie->dst_picture.data[0]);
      avcodec_free_frame(&movie->frame);
    }

  if (!(movie->fmt_ctx->oformat->flags & AVFMT_NOFILE)) avio_close(movie->fmt_ctx->pb);

  avformat_free_context(movie->fmt_ctx);
}

#endif
