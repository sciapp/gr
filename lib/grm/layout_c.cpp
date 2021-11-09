/* ######################### includes ############################################################################### */

#include "layout.h"
#include "layout.hpp"


/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- layout --------------------------------------------------------------------------------- */

layout_t *layout_new(void)
{
  Layout *layout = new Layout();

  return reinterpret_cast<layout_t *>(layout);
}

void layout_delete(layout_t *layout)
{
  Layout *layout_ = reinterpret_cast<Layout *>(layout);

  delete layout_;
}

void layout_print_test(const layout_t *layout)
{
  const Layout *layout_ = reinterpret_cast<const Layout *>(layout);

  layout_->print_test();
}
