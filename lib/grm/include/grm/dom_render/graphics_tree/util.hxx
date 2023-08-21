#ifndef GRM_GRAPHICS_TREE_INTERFACE_UTIL_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_UTIL_HXX

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <grm/util.h>

namespace GRM
{
class Document;
class Element;
class Node;

struct EXPORT SerializerOptions
{
  std::string indent;
  bool show_hidden;
};
EXPORT std::string toXML(const std::shared_ptr<const Node> &node, const SerializerOptions &options = {"", false});

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
} // namespace GRM

#endif
