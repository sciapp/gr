#ifndef GR_MANAGEGRCONTEXTIDS_HXX
#define GR_MANAGEGRCONTEXTIDS_HXX

#include <queue>

class ManageGRContextIds
{
public:
  void destroyGRContexts();
  int getUnusedGRContextId();
  void markIdAsUnused(int id);
  void markAllIdsAsUnused();

private:
  std::queue<int> available_gr_context_ids = {};
  int no_currently_allocated_gr_contexts = 0;
};

#endif // GR_MANAGEGRCONTEXTIDS_HXX
