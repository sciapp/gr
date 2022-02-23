#ifndef GRM_H_INCLUDED
#define GRM_H_INCLUDED

/* This is a convenience header which includes all public header files of the GRM library. */

#define QUOTE(x) #x
#ifdef BUILDING_GR
#define INCLUDE_PATH(filename) QUOTE(filename)
#else
#define INCLUDE_PATH(filename) QUOTE(grm / filename)
#endif

#include INCLUDE_PATH(args.h)
#include INCLUDE_PATH(dump.h)
#include INCLUDE_PATH(event.h)
#include INCLUDE_PATH(interaction.h)
#include INCLUDE_PATH(net.h)
#include INCLUDE_PATH(plot.h)
#include INCLUDE_PATH(util.h)

#undef QUOTE
#undef INCLUDE_PATH

#endif /* ifndef GRM_H_INCLUDED */
