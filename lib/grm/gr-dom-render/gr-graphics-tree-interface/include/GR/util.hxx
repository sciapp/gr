#ifndef GR_GRAPHICS_TREE_INTERFACE_UTIL_HXX
#define GR_GRAPHICS_TREE_INTERFACE_UTIL_HXX

#include <string>
#include <memory>
#include <map>
#include <vector>

namespace GR
{
class Document;
class Element;
class Node;

struct SerializerOptions
{
  std::string indent;
};
std::string toXML(const std::shared_ptr<const Node> &node, const SerializerOptions &options = {});

std::string tolower(std::string string);
std::string toupper(std::string string);
std::vector<std::string> split(const std::string &string, const std::string &token);
std::string strip(const std::string &string);

class Selector
{
public:
  bool matchElement(const GR::Element &element,
                    std::map<std::tuple<const GR::Element *, const GR::Selector *>, bool> &match_map) const;

protected:
  virtual bool
  doMatchElement(const GR::Element &element,
                 std::map<std::tuple<const GR::Element *, const GR::Selector *>, bool> &match_map) const = 0;
};
std::shared_ptr<GR::Selector> parseSelectors(const std::string &selectors);
} // namespace GR

#endif
