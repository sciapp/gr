#ifndef GROUP_MASK_HXX_INCLUDED
#define GROUP_MASK_HXX_INCLUDED

#include <cassert>
#include <cstdlib>
#include <memory>

namespace GRM
{

class GroupMask
{
public:
  GroupMask() {}
  GroupMask(unsigned int width, unsigned int height, unsigned int **c_mask) { own(width, height, c_mask); }
  virtual ~GroupMask() {}

  void own(unsigned int width, unsigned int height, unsigned int **c_mask)
  {
    mask_ = std::unique_ptr<unsigned int, decltype(&std::free)>(*c_mask, std::free);
    width_ = width;
    height_ = height;
    *c_mask = nullptr;
  }

  unsigned int operator()(unsigned int x, unsigned int y) const
  {
    assert(x < width_ && y < height_);
    // Use `& 0x00FFFFFF` to remove the alpha channel which must always unequal zero in the image, otherwise Qt will
    // use undesired alpha optimizations, effectively breaking the id storage in the mask.
    return mask_.get()[y * width_ + x] & 0x00FFFFFF;
  }

  bool hasPixel(unsigned int x, unsigned int y) const { return this->operator()(x, y) != 0; }

private:
  std::unique_ptr<unsigned int, decltype(&std::free)> mask_{nullptr, &std::free};
  unsigned int width_{0};
  unsigned int height_{0};
};

} // namespace GRM
#endif /* ifndef GROUP_MASK_HXX_INCLUDED */
