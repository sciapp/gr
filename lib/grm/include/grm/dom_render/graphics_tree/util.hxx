#ifndef GRM_GRAPHICS_TREE_INTERFACE_UTIL_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_UTIL_HXX

#include <functional>
#include <string>
#include <memory>
#include <map>
#include <optional>
#include <vector>
#include <grm/util.h>

namespace GRM
{
class Document;
class Element;
class Node;

struct GRM_EXPORT SerializerOptions
{
  enum class InternalAttributesFormat
  {
    NONE,       // Hide internal attributes
    PLAIN,      // Export internal attributes like public attributes
    OBFUSCATED, // Collect internal attributes and store them into a special `internal` attribute, BSON+Base64 encoded
  };

  std::string indent = "";
  InternalAttributesFormat internal_attribute_format = InternalAttributesFormat::NONE;
};
GRM_EXPORT std::string toXML(const std::shared_ptr<const Node> &node,
                             const SerializerOptions &options = {"", SerializerOptions::InternalAttributesFormat::NONE},
                             std::optional<std::function<bool(const std::string &attribute_name, const Element &element,
                                                              std::optional<std::string> &new_attribute_name)>>
                                 attribute_filter = std::nullopt);
GRM_EXPORT std::string toLower(std::string string);
GRM_EXPORT std::string toUpper(std::string string);
GRM_EXPORT std::vector<std::string> split(const std::string &string, const std::string &token);
GRM_EXPORT std::string strip(const std::string &string);

GRM_EXPORT void normalizeVec(std::vector<double> x, std::vector<double> *normalized_x);
GRM_EXPORT void normalizeVecInt(std::vector<double> x, std::vector<unsigned int> *normalized_x, unsigned int sum);

class GRM_EXPORT Selector
{
public:
  bool matchElement(const Element &element,
                    std::map<std::tuple<const Element *, const Selector *>, bool> &match_map) const;

protected:
  virtual bool doMatchElement(const Element &element,
                              std::map<std::tuple<const Element *, const Selector *>, bool> &match_map) const = 0;
};
GRM_EXPORT std::shared_ptr<Selector> parseSelectors(const std::string &selectors);

// `overloaded` utility taken from <https://en.cppreference.com/w/cpp/utility/variant/visit>
template <class... Ts> struct Overloaded : Ts...
{
  using Ts::operator()...;
};
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;
} // namespace GRM

#endif
