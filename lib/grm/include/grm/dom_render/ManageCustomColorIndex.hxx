#ifndef GR_MANAGECUSTOMCOLORINDEX_HXX
#define GR_MANAGECUSTOMCOLORINDEX_HXX

#include <stack>
#include <map>

class ManageCustomColorIndex
{
public:
  void savestate(void);
  void restorestate(void);
  void savecontext(int contextId);
  void selectcontext(int contextId);

private:
  std::stack<int> state;
  std::map<int, int> context;
};

#endif // GR_MANAGECUSTOMCOLORINDEX_HXX
