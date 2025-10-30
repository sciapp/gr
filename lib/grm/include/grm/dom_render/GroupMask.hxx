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

#include <cmath>
#include <fstream>
#include <cstdint>
#include <array>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/util.h>

GRM_EXPORT std::shared_ptr<GRM::Element> grm_get_document_root(void);


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


  void toPPM(const std::string &filepath, std::optional<std::set<unsigned int>> filter_ids = std::nullopt,
             bool export_elements = false) const
  {
    std::set<unsigned int> ids;

    if (filter_ids)
      {
        ids = *filter_ids;
      }
    else
      {
        for (unsigned int y = 0; y < height_; y++)
          {
            for (unsigned int x = 0; x < width_; x++)
              {
                auto index = (*this)(x, y);
                if (index < 0) continue;
                ids.insert(index);
              }
          }
      }

    float hue_step = 1.0f / ids.size();
    std::unordered_map<int, std::array<uint8_t, 3>> id_to_rgb_map{{-1, {255, 255, 255}}};
    unsigned int index = 0;
    for (auto id : ids)
      {
        std::array<uint8_t, 3> pixel{255, 255, 255};
        /*
         * Formula taken from <https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB> with s = 1 and v = 1
         */
        auto hue = (hue_step + 0.25f) * index - std::floor((hue_step + 0.25f) * index);
        auto hue6 = hue * 6.0f;
        auto color_component = static_cast<uint8_t>(255.0f * (1.0f - std::abs(std::fmod(hue6, 2.0f) - 1.0f)));
        switch (static_cast<int>(hue6))
          {
          case 0:
            pixel = {255, color_component, 0};
            break;
          case 1:
            pixel = {color_component, 255, 0};
            break;
          case 2:
            pixel = {0, 255, color_component};
            break;
          case 3:
            pixel = {0, color_component, 255};
            break;
          case 4:
            pixel = {color_component, 0, 255};
            break;
          default:
            pixel = {255, 0, color_component};
            break;
          }
        id_to_rgb_map[id] = pixel;
        ++index;
      }

    std::ofstream image_file(filepath, std::ios::out | std::ios::binary);
    image_file << "P6\n";
    if (export_elements)
      {
        const auto &document = grm_get_document_root();
        for (auto id : ids)
          {
            std::cerr << "Current id: " << id << std::endl;
            const auto &elem = document->querySelectors("[_bbox_id=\"" + std::to_string(id) + "\"]");
            if (elem == nullptr)
              {
                std::cerr << "Invalid id: " << id << std::endl;
                continue;
              }
            image_file << "# " << id << ": " << elem->localName() << " ";
            for (const auto &attribute_name : elem->getAttributeNames())
              {
                image_file << attribute_name << "=\"" << static_cast<std::string>(elem->getAttribute(attribute_name))
                           << "\" ";
              }
            image_file << "\n";
          }
      }
    image_file << std::to_string(width_) << " " << std::to_string(height_) << "\n255\n";
    for (unsigned int y = 0; y < height_; y++)
      {
        for (unsigned int x = 0; x < width_; x++)
          {
            auto current_id = (*this)(x, y);
            const auto &pixel = id_to_rgb_map[current_id];
            image_file << pixel[0] << pixel[1] << pixel[2];
          }
      }
  }

  std::unordered_set<unsigned int> getObjectsInBox(unsigned int center_x, unsigned int center_y) const
  {
    std::unordered_set<unsigned int> object_ids;
    // std::cerr << "################################################################################" << std::endl;
    for (int i = std::max<int>(0, center_x - box_size_ / 2); i < std::min<int>(width_, center_x + box_size_ / 2); i++)
      {
        for (int j = std::max<int>(0, center_y - box_size_ / 2); j < std::min<int>(height_, center_y + box_size_ / 2);
             j++)
          {
            int id;
            // std::cerr << "(" << i << ", " << j << ") = " << this->operator()(i, j) << std::endl;
            if ((id = (*this)(i, j)) >= 0)
              {
                object_ids.insert(id);
              }
          }
      }
    // std::cerr << "################################################################################" << std::endl;
    return object_ids;
  }

  int operator()(unsigned int x, unsigned int y) const
  {
    assert(x < width_ && y < height_);
    // Use `& 0x00FFFFFF` to remove the alpha channel which must always unequal zero in the image, otherwise Qt will
    // use undesired alpha optimizations, effectively breaking the id storage in the mask.
    auto color = mask_.get()[y * width_ + x];
    auto id = color & 0x00FFFFFF;
    return (color & 0xFF000000) == 0 ? -1 : id;
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
