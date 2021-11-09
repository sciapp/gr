#ifndef LAYOUT_H_INCLUDED
#define LAYOUT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### public interface ####################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- layout --------------------------------------------------------------------------------- */

struct _layout_t;
typedef struct _layout_t layout_t;


/* ========================= methods ================================================================================ */

/* ------------------------- layout --------------------------------------------------------------------------------- */

layout_t *layout_new(void);
void layout_delete(layout_t *layout);

void layout_print_test(const layout_t *layout);

#ifdef __cplusplus
}
#endif
#endif /* ifndef LAYOUT_H_INCLUDED */
