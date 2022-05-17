#ifndef GRM_H_INCLUDED
#define GRM_H_INCLUDED

/* This is a convenience header which includes all public header files of the GRM library. */

#ifndef GRM_INCLUDE_PATH_
#define GRM_QUOTE_(x) #x
#ifdef BUILDING_GR
#define GRM_INCLUDE_PATH_(filename) GRM_QUOTE_(filename)
#else
/* clang-format off */
#define GRM_INCLUDE_PATH_(filename) GRM_QUOTE_(grm/filename)
/* clang-format on */
#endif
#endif

#include GRM_INCLUDE_PATH_(args.h)
#include GRM_INCLUDE_PATH_(dump.h)
#include GRM_INCLUDE_PATH_(event.h)
#include GRM_INCLUDE_PATH_(interaction.h)
#include GRM_INCLUDE_PATH_(layout.h)
#include GRM_INCLUDE_PATH_(net.h)
#include GRM_INCLUDE_PATH_(plot.h)
#include GRM_INCLUDE_PATH_(util.h)

#endif /* ifndef GRM_H_INCLUDED */
