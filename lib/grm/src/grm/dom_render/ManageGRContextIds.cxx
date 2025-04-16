#include "grm/dom_render/ManageGRContextIds.hxx"
#include "gr.h"
#include "grm/dom_render/GRMaxContextReachedError.hxx"
#include <string>

void ManageGRContextIds::destroyGRContexts()
{
  for (int id = 1; id <= no_currently_allocated_gr_contexts; ++id) gr_destroycontext(id);
  available_gr_context_ids = {};
  no_currently_allocated_gr_contexts = 0;
}

int ManageGRContextIds::getUnusedGRContextId()
{
  if (available_gr_context_ids.empty())
    {
      if (no_currently_allocated_gr_contexts + 1 > GR_MAX_CONTEXT)
        throw GRMaxContextReachedError("Internal Error: All gr context ids are being used");
      available_gr_context_ids.push(++no_currently_allocated_gr_contexts);
    }
  int context_id = available_gr_context_ids.front();
  available_gr_context_ids.pop();
  return context_id;
}

void ManageGRContextIds::markIdAsUnused(int id)
{
  if (id <= no_currently_allocated_gr_contexts) available_gr_context_ids.push(id);
}

void ManageGRContextIds::markAllIdsAsUnused()
{
  available_gr_context_ids = {};
  for (int id = 1; id <= no_currently_allocated_gr_contexts; ++id) available_gr_context_ids.push(id);
}
