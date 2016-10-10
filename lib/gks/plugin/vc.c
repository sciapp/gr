#if !defined(NO_AV) && !defined(NO_MUPDF)

#include <stdio.h>

#include "vc.h"

movie_t vc_movie_create(const char *path, int framerate, int bitrate)
{
  movie_t movie = (movie_t) malloc(sizeof(struct movie_t_));

  av_register_all();
  av_log_set_level(AV_LOG_QUIET);

  movie->path = path;

  avformat_alloc_output_context2(&movie->fmt_ctx, NULL, NULL, movie->path);

  movie->out_fmt = movie->fmt_ctx->oformat;
  movie->video_st = NULL;

  if (movie->out_fmt->video_codec == AV_CODEC_ID_NONE) {
    fprintf(stderr, "codec not found from given path\n");
    exit(1);
  }

  movie->cdc = avcodec_find_encoder(movie->out_fmt->video_codec);
  if (!(movie->cdc)) {
    fprintf(stderr, "codec not found\n");
    exit(1);
  }

  movie->video_st = avformat_new_stream(movie->fmt_ctx, movie->cdc);
  if (!movie->video_st) {
    fprintf(stderr, "Could not alloc stream\n");
    exit(1);
  }

  movie->cdc_ctx = movie->video_st->codec;
  avcodec_get_context_defaults3(movie->cdc_ctx, movie->cdc);

  movie->cdc_ctx->codec_id       = movie->out_fmt->video_codec;
  movie->cdc_ctx->bit_rate       = bitrate;
  movie->cdc_ctx->time_base.num  = 1;
  movie->cdc_ctx->time_base.den  = framerate;
  movie->cdc_ctx->gop_size       = 12;
  movie->cdc_ctx->pix_fmt        = PIX_FMT_YUV420P;
  movie->num_frames              = 0;

  return movie;
}

void vc_movie_append_frame(movie_t movie, frame_t frame)
{
  int                 got_output, ret;
  struct SwsContext   *sws_ctx;
  AVPacket            pkt;
  AVPicture           src_picture;

  if (movie->num_frames == 0) {

    movie->cdc_ctx->width  = frame->width;
    movie->cdc_ctx->height = frame->height;

    if (movie->fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
      movie->cdc_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(movie->cdc_ctx, movie->cdc, NULL) < 0) {
      fprintf(stderr, "could not open video codec\n");
      exit(1);
    }

    movie->frame = avcodec_alloc_frame();
    if (!frame) {
      fprintf(stderr, "could not allocate video frame\n");
      exit(1);
    }

    ret = avpicture_alloc(&movie->dst_picture, movie->cdc_ctx->pix_fmt,
                          movie->cdc_ctx->width, movie->cdc_ctx->height);
    if (ret < 0) {
      fprintf(stderr, "could not allocate picture\n");
      exit(1);
    }

    *((AVPicture *)movie->frame) = movie->dst_picture;

    av_dump_format(movie->fmt_ctx, 0, movie->path, 1);
    if (!(movie->out_fmt->flags & AVFMT_NOFILE)) {
      if (avio_open(&movie->fmt_ctx->pb, movie->path, AVIO_FLAG_WRITE) < 0)
        {
          fprintf(stderr, "could not open '%s'\n", movie->path);
          exit(1);
        }
    }

    if (avformat_write_header(movie->fmt_ctx, NULL) < 0) {
      fprintf(stderr, "error occurred when opening output file\n");
      exit(1);
    }

    movie->frame->pts = 0;
  }

  ret = avpicture_alloc(&src_picture, PIX_FMT_RGB32, movie->cdc_ctx->width,
                        movie->cdc_ctx->height);
  if (ret < 0) {
    fprintf(stderr, "could not allocate temporary picture\n");
    exit(1);
  }

  avpicture_fill(&src_picture, frame->data, PIX_FMT_RGBA, movie->cdc_ctx->width,
                 movie->cdc_ctx->height);
  src_picture.data[0] = frame->data;

  sws_ctx = NULL;
  sws_ctx = sws_getCachedContext(
    sws_ctx, movie->cdc_ctx->width, movie->cdc_ctx->height, PIX_FMT_RGBA,
    movie->cdc_ctx->width, movie->cdc_ctx->height, PIX_FMT_YUV420P, SWS_BICUBIC,
    NULL, NULL, NULL);

  if (!sws_ctx) {
    fprintf(stderr, "could not initialize the conversion context\n");
    exit(1);
  }

  sws_scale(sws_ctx, (const uint8_t * const *) src_picture.data,
            src_picture.linesize, 0, movie->cdc_ctx->height,
            movie->dst_picture.data, movie->dst_picture.linesize);

  av_init_packet(&pkt);

  if (movie->fmt_ctx->oformat->flags & AVFMT_RAWPICTURE) {
    pkt.flags          |= AV_PKT_FLAG_KEY;
    pkt.stream_index    = movie->video_st->index;
    pkt.data            = movie->dst_picture.data[0];
    pkt.size            = sizeof(AVPicture);

    av_write_frame(movie->fmt_ctx, &pkt);
  } else {
    pkt.data = NULL;
    pkt.size = 0;

    ret = avcodec_encode_video2(movie->cdc_ctx, &pkt, movie->frame,
                                &got_output);
    if (ret < 0) {
      fprintf(stderr, "Error encoding video frame\n");
      exit(1);
    }

    if (got_output) {
      if (movie->cdc_ctx->coded_frame->key_frame)
        pkt.flags |= AV_PKT_FLAG_KEY;

      pkt.stream_index = movie->video_st->index;
      ret = av_write_frame(movie->fmt_ctx, &pkt);
    } else {
      ret = 0;
    }

    if (ret != 0) {
      fprintf(stderr, "Error while writing video frame\n");
      exit(1);
    }
  }
  av_free_packet(&pkt);
  movie->frame->pts += av_rescale_q(1, movie->video_st->codec->time_base,
                                    movie->video_st->time_base);
  movie->num_frames++;
}

void vc_frame_free(frame_t frame)
{
  free(frame->data);
  free(frame);
}

void vc_movie_finish(movie_t movie)
{
  unsigned int i;

  av_write_trailer(movie->fmt_ctx);

  if (movie->video_st) {
    avcodec_close(movie->video_st->codec);

    av_free(movie->dst_picture.data[0]);
    avcodec_free_frame(&movie->frame);
  }
  for (i = 0; i < movie->fmt_ctx->nb_streams; i++) {
    av_freep(&movie->fmt_ctx->streams[i]->codec);
    av_freep(&movie->fmt_ctx->streams[i]);
  }

  if (!(movie->fmt_ctx->oformat->flags & AVFMT_NOFILE))
    avio_close(movie->fmt_ctx->pb);

  av_free(movie->fmt_ctx);
}

pdf_t vc_pdf_from_file(const char *path)
{
  pdf_t pdf = (pdf_t) malloc(sizeof(struct pdf_t_));

  pdf->ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  fz_register_document_handlers(pdf->ctx);
  pdf->doc = fz_open_document(pdf->ctx, path);
#ifdef MUPDF_API_VERSION_17
  pdf->num_pages = fz_count_pages(pdf->ctx, pdf->doc);
#else
  pdf->num_pages = fz_count_pages(pdf->doc);
#endif
  pdf->path = path;

  return pdf;
}

pdf_t vc_pdf_from_memory(unsigned char *data, int len)
{
  fz_stream *stream;

  pdf_t pdf = (pdf_t) malloc(sizeof(struct pdf_t_));

  pdf->ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  fz_register_document_handlers(pdf->ctx);
  stream = fz_open_memory(pdf->ctx, data, len);
  pdf->doc = fz_open_document_with_stream(pdf->ctx, "pdf", stream);
#ifdef MUPDF_API_VERSION_17
  pdf->num_pages = fz_count_pages(pdf->ctx, pdf->doc);
#else
  pdf->num_pages = fz_count_pages(pdf->doc);
#endif
  pdf->path = "stream";

  return pdf;
}

frame_t vc_frame_from_pdf(pdf_t pdf, int page, int width, int height)
{
  double transx, transy, zoom;
  fz_matrix transform, scale_mat, transl_mat;
  fz_rect rect;
  fz_irect bbox;
  fz_pixmap *pix;
  fz_device *dev;
  fz_page *page_o;
  unsigned char *data;

  frame_t frame = (frame_t) malloc(sizeof(struct frame_t_));

#ifdef MUPDF_API_VERSION_17
  page_o = fz_load_page(pdf->ctx, pdf->doc, page - 1);
#else
  page_o = fz_load_page(pdf->doc, page - 1);
#endif

  transx = 0;
  transy = 0;
  zoom = 1.0;
  fz_scale(&scale_mat, zoom, zoom);
  fz_translate(&transl_mat, transx, transy);
  fz_concat(&transform, &scale_mat, &transl_mat);

  /*
   * Take the page bounds and transform them by the same matrix that
   * we will use to render the page.
   */

#ifdef MUPDF_API_VERSION_17
  fz_bound_page(pdf->ctx, page_o, &rect);
#else
  fz_bound_page(pdf->doc, page_o, &rect);
#endif
  fz_transform_rect(&rect, &transform);
  fz_round_rect(&bbox, &rect);

  /*
   * Create a blank pixmap to hold the result of rendering. The
   * pixmap bounds used here are the same as the transformed page
   * bounds, so it will contain the entire page.
   */

  pix = fz_new_pixmap(pdf->ctx, fz_device_rgb(pdf->ctx), width, height);
  fz_clear_pixmap_with_value(pdf->ctx, pix, 0xff);

  /*
   * Create a draw device with the pixmap as its target.
   * Run the page with the transform.
   */

  dev = fz_new_draw_device(pdf->ctx, pix);
#ifdef MUPDF_API_VERSION_17
  fz_run_page(pdf->ctx, page_o, dev, &transform, NULL);
#else
  fz_run_page(pdf->doc, page_o, dev, &transform, NULL);
#endif

  frame->data = (unsigned char *) malloc(width * height * 4 * sizeof(unsigned char));
  data = fz_pixmap_samples(pdf->ctx, pix);
  memmove(frame->data, data, width * height * 4 * sizeof(unsigned char));
  frame->width  = width;
  frame->height = height;

#ifdef MUPDF_API_VERSION_17
  fz_drop_device(pdf->ctx, dev);
  fz_drop_pixmap(pdf->ctx, pix);
  fz_drop_page(pdf->ctx, page_o);
#else
  fz_free_device(dev);
  fz_drop_pixmap(pdf->ctx, pix);
  fz_free_page(pdf->doc, page_o);
#endif

  return frame;
}

frame_t *vc_pdf_to_frames(pdf_t pdf, int width, int height)
{
  int i;
  frame_t *frames;

  frames = (frame_t *) malloc(sizeof(frame_t) * pdf->num_pages);
  for (i = 0; i < pdf->num_pages; i++) {
    frames[i] = vc_frame_from_pdf(pdf, i + 1, width, height);
  }

  return frames;
}

int vc_pdf_get_number_of_pages(pdf_t pdf)
{
  return pdf->num_pages;
}

void vc_pdf_close(pdf_t pdf)
{
#ifdef MUPDF_API_VERSION_17
  fz_drop_document(pdf->ctx, pdf->doc);
  fz_drop_context(pdf->ctx);
#else
  fz_close_document(pdf->doc);
  fz_free_context(pdf->ctx);
#endif
  pdf->num_pages = -1;
}

#endif
