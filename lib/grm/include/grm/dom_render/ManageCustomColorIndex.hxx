#ifndef GR_MANAGECUSTOMCOLORINDEX_HXX
#define GR_MANAGECUSTOMCOLORINDEX_HXX

#include <stack>
#include <map>

class ManageCustomColorIndex
{
public:
  void saveState(void);
  void restoreState(void);
  void saveContext(int context_id);
  void selectContext(int context_id);

private:
  std::stack<int> state;
  std::map<int, int> context;
};

#endif // GR_MANAGECUSTOMCOLORINDEX_HXX
