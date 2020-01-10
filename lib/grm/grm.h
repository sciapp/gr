#ifndef _GRM_H_
#define _GRM_H_

#include <stdio.h>

#ifdef _WIN32
#define HAVE_BOOLEAN
#include <windows.h> /* required for all Windows applications */
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define GR_SENDER 0
#define GR_RECEIVER 1

  typedef struct _gr_meta_args_t gr_meta_args_t;

  typedef enum
  {
    GR_META_EVENT_NEW_PLOT,
    GR_META_EVENT_UPDATE_PLOT,
    GR_META_EVENT_SIZE,
    GR_META_EVENT_MERGE_END,
    _GR_META_EVENT_TYPE_COUNT /* helper entry to store how many different event types exist */
  } gr_meta_event_type_t;

  typedef struct
  {
    gr_meta_event_type_t type;
    int plot_id;
  } gr_meta_new_plot_event_t;

  typedef struct
  {
    gr_meta_event_type_t type;
    int plot_id;
  } gr_meta_update_plot_event_t;

  typedef struct
  {
    gr_meta_event_type_t type;
    int plot_id;
    int width;
    int height;
  } gr_meta_size_event_t;

  typedef struct
  {
    gr_meta_event_type_t type;
    const char *identificator;
  } gr_meta_merge_end_event_t;

  typedef union
  {
    gr_meta_new_plot_event_t new_plot_event;
    gr_meta_size_event_t size_event;
    gr_meta_update_plot_event_t update_plot_event;
    gr_meta_merge_end_event_t merge_end_event;
  } gr_meta_event_t;

  typedef void (*gr_meta_event_callback_t)(const gr_meta_event_t *);

  DLLEXPORT gr_meta_args_t *gr_newmeta(void);
  DLLEXPORT void gr_deletemeta(gr_meta_args_t *);
  DLLEXPORT void gr_finalizemeta(void);
  DLLEXPORT int gr_meta_args_push(gr_meta_args_t *, const char *, const char *, ...);
  DLLEXPORT int gr_meta_args_push_buf(gr_meta_args_t *, const char *, const char *, const void *, int);
  DLLEXPORT int gr_meta_args_contains(const gr_meta_args_t *, const char *);
  DLLEXPORT void gr_meta_args_clear(gr_meta_args_t *);
  DLLEXPORT void gr_meta_args_remove(gr_meta_args_t *, const char *);
  DLLEXPORT int gr_meta_get_box(const int, const int, const int, const int, const int, int *, int *, int *, int *);
  DLLEXPORT void *gr_openmeta(int, const char *, unsigned int, const char *(*)(const char *, unsigned int),
                              int (*)(const char *, unsigned int, const char *));
  DLLEXPORT gr_meta_args_t *gr_recvmeta(const void *p, gr_meta_args_t *);
  DLLEXPORT int gr_sendmeta(const void *, const char *, ...);
  DLLEXPORT int gr_sendmeta_buf(const void *, const char *, const void *, int);
  DLLEXPORT int gr_sendmeta_ref(const void *, const char *, char, const void *, int);
  DLLEXPORT int gr_sendmeta_args(const void *p, const gr_meta_args_t *);
  DLLEXPORT void gr_closemeta(const void *);
  DLLEXPORT int gr_clearmeta(void);
  DLLEXPORT int gr_inputmeta(const gr_meta_args_t *);
  DLLEXPORT int gr_mergemeta(const gr_meta_args_t *);
  DLLEXPORT int gr_mergemeta_named(const gr_meta_args_t *, const char *identificator);
  DLLEXPORT int gr_plotmeta(const gr_meta_args_t *);
  DLLEXPORT int gr_readmeta(gr_meta_args_t *, const char *);
  DLLEXPORT int gr_switchmeta(unsigned int id);
  DLLEXPORT int gr_registermeta(gr_meta_event_type_t, gr_meta_event_callback_t);
  DLLEXPORT int gr_unregistermeta(gr_meta_event_type_t);
  DLLEXPORT unsigned int gr_meta_max_plotid(void);
#ifndef NDEBUG
  DLLEXPORT void gr_dumpmeta(const gr_meta_args_t *, FILE *);
  DLLEXPORT void gr_dumpmeta_json(const gr_meta_args_t *, FILE *);
#endif
  DLLEXPORT int gr_load_from_str(const char *);
  DLLEXPORT char *gr_dumpmeta_json_str(void);

#ifdef __cplusplus
}
#endif

#endif
