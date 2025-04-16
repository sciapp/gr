#ifndef GR_MANAGEZINDEX_HXX
#define GR_MANAGEZINDEX_HXX

#include <stack>

class ManageZIndex
{
public:
  void saveState(void);
  void restoreState(void);
  int getZIndex(void);
  void setZIndex(int z_index);

private:
  int current_z_index = 0;
  std::stack<int> state;
};

#endif // GR_MANAGEZINDEX_HXX
