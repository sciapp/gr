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

struct EXPORT SerializerOptions
{
  enum class InternalAttributesFormat
  {
    None,       // Hide internal attributes
    Plain,      // Export internal attributes like public attributes
    Obfuscated, // Collect internal attributes and store them into a special `internal` attribute, BSON+Base64 encoded
  };

  std::string indent = "";
  InternalAttributesFormat internal_attribute_format = InternalAttributesFormat::None;
};
EXPORT std::string
toXML(const std::shared_ptr<const Node> &node,
      const SerializerOptions &options = {"", SerializerOptions::InternalAttributesFormat::None},
      std::optional<std::function<bool(const std::string &attribute_name, const GRM::Element &element,
                                       std::optional<std::string> &new_attribute_name)>>
          attribute_filter = std::nullopt);
EXPORT std::string tolower(std::string string);
EXPORT std::string toupper(std::string string);
EXPORT std::vector<std::string> split(const std::string &string, const std::string &token);
EXPORT std::string strip(const std::string &string);

EXPORT void normalize_vec(std::vector<double> x, std::vector<double> *normalized_x);
EXPORT void normalize_vec_int(std::vector<double> x, std::vector<unsigned int> *normalized_x, unsigned int sum);

class EXPORT Selector
{
public:
  bool matchElement(const GRM::Element &element,
                    std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const;

protected:
  virtual bool
  doMatchElement(const GRM::Element &element,
                 std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const = 0;
};
EXPORT std::shared_ptr<GRM::Selector> parseSelectors(const std::string &selectors);

// `overloaded` utility taken from <https://en.cppreference.com/w/cpp/utility/variant/visit>
template <class... Ts> struct overloaded : Ts...
{
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
} // namespace GRM

#endif
