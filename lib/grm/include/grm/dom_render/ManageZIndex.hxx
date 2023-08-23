#ifndef GR_MANAGEZINDEX_HXX
#define GR_MANAGEZINDEX_HXX

#include <stack>

class ManageZIndex
{
public:
  void savestate(void);
  void restorestate(void);
  int getZIndex(void);
  void setZIndex(int zIndex);

private:
  int currentZIndex = 0;
  std::stack<int> state;
};

#endif // GR_MANAGEZINDEX_HXX
