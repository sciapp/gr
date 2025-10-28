#ifndef GROUP_MASK_HXX_INCLUDED
#define GROUP_MASK_HXX_INCLUDED

#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>

#ifdef QT_VERSION
#include <QColor>
#include <QImage>
#endif

namespace GRM
{

const unsigned int DEFAULT_BOX_SIZE = 8;

class GroupMask
{
public:
  GroupMask() {}
  GroupMask(unsigned int width, unsigned int height, unsigned int **c_mask, unsigned int box_size = DEFAULT_BOX_SIZE)
  {
    own(width, height, c_mask);
  }
  virtual ~GroupMask() {}

  void own(unsigned int width, unsigned int height, unsigned int **c_mask)
  {
    mask_ = std::unique_ptr<unsigned int, decltype(&std::free)>(*c_mask, std::free);
    width_ = width;
    height_ = height;
    *c_mask = nullptr;
  }

  // TODO:
  // - Return image with single id (with image filter)
  // - Cellarray / Images


#ifdef QT_VERSION
  QImage toColoredImage() const
  {
    std::set<unsigned int> used_ids;

    for (unsigned int y = 0; y < height_; y++)
      {
        for (unsigned int x = 0; x < width_; x++)
          {
            used_ids.insert((*this)(x, y));
          }
      }

    float hue_step = 1.0f / used_ids.size();

    QImage img(width_, height_, QImage::Format_ARGB32);
    for (unsigned int y = 0; y < height_; y++)
      {
        for (unsigned int x = 0; x < width_; x++)
          {
            QColor color;
            auto current_id = (*this)(x, y);
            if (current_id != 0)
              {
                auto id_pos = *used_ids.find(current_id);
                color.setHsvF((hue_step + 0.25f) * id_pos - (int)((hue_step + 0.25f) * id_pos), 1.0f, 1.0f);
              }
            else
              {
                color.setRgb(0, 0, 0, 0);
              }
            img.setPixelColor(x, y, color);
          }
      }

    return img;
  }
#endif

  std::set<unsigned int> getObjectsInBox(unsigned int center_x, unsigned int center_y) const
  {
    std::set<unsigned int> object_ids;
    std::cerr << "################################################################################" << std::endl;
    for (int i = std::max<int>(0, center_x - box_size_ / 2); i < std::min<int>(width_, center_x + box_size_ / 2); i++)
      {
        for (int j = std::max<int>(0, center_y - box_size_ / 2); j < std::min<int>(height_, center_y + box_size_ / 2);
             j++)
          {
            unsigned int id;
            std::cerr << "(" << i << ", " << j << ") = " << this->operator()(i, j) << std::endl;
            if ((id = (*this)(i, j)) != 0)
              {
                object_ids.insert(id);
              }
          }
      }
    std::cerr << "################################################################################" << std::endl;
    return object_ids;
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
  unsigned int box_size_{DEFAULT_BOX_SIZE};
};

} // namespace GRM
#endif /* ifndef GROUP_MASK_HXX_INCLUDED */
