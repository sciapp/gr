#ifndef GRM_GRAPHICS_TREE_INTERFACE_NODE_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_NODE_HXX

#include <memory>
#include <list>
#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <grm/util.h>

namespace GRM
{
class Element;
class Document;
class Selector;

class EXPORT Node : public std::enable_shared_from_this<Node>
{
public:
  enum class Type
  {
    ELEMENT_NODE = 1,
    COMMENT_NODE = 8,
    DOCUMENT_NODE = 9
  };

  virtual ~Node() = default;

  Type nodeType() const;

  virtual std::string nodeName() const = 0;

  bool isConnected() const;

  std::shared_ptr<Document> ownerDocument();

  std::shared_ptr<const Document> ownerDocument() const;

  std::shared_ptr<Node> getRootNode();

  std::shared_ptr<const Node> getRootNode() const;

  std::shared_ptr<Node> parentNode();

  std::shared_ptr<const Node> parentNode() const;

  std::shared_ptr<Element> parentElement();

  std::shared_ptr<const Element> parentElement() const;

  bool hasChildNodes() const;

  std::vector<std::shared_ptr<Node>> childNodes();

  std::vector<std::shared_ptr<const Node>> childNodes() const;

  std::shared_ptr<Node> firstChild();

  std::shared_ptr<const Node> firstChild() const;

  std::shared_ptr<Node> lastChild();

  std::shared_ptr<const Node> lastChild() const;

  std::shared_ptr<Node> previousSibling();

  std::shared_ptr<const Node> previousSibling() const;

  std::shared_ptr<Node> nextSibling();

  std::shared_ptr<const Node> nextSibling() const;

  virtual std::shared_ptr<Node> cloneNode();

  virtual std::shared_ptr<Node> cloneNode(bool deep);

  virtual bool isEqualNode(const std::shared_ptr<const Node> &otherNode) const;

  bool isSameNode(const std::shared_ptr<const Node> &otherNode) const;

  bool contains(const std::shared_ptr<const Node> &otherNode) const;

  std::shared_ptr<Node> insertBefore(std::shared_ptr<Node> node, const std::shared_ptr<Node> &child);

  std::shared_ptr<Node> appendChild(std::shared_ptr<Node> node);

  std::shared_ptr<Node> replaceChild(std::shared_ptr<Node> node, const std::shared_ptr<Node> &child);

  std::shared_ptr<Node> removeChild(std::shared_ptr<Node> child);

protected:
  Node(Type type, const std::shared_ptr<Document> &owner_document);

  std::vector<std::shared_ptr<Element>> getElementsByClassName_impl(const std::string &classNames);

  std::vector<std::shared_ptr<const Element>> getElementsByClassName_impl(const std::string &classNames) const;

  /* ParentNode interface implementations */
  std::vector<std::shared_ptr<Element>> children_impl();

  std::vector<std::shared_ptr<const Element>> children_impl() const;

  std::shared_ptr<Element> firstChildElement_impl();

  std::shared_ptr<const Element> firstChildElement_impl() const;

  std::shared_ptr<Element> lastChildElement_impl();

  std::shared_ptr<const Element> lastChildElement_impl() const;

  unsigned long childElementCount_impl() const;

  void prepend_impl(const std::vector<std::shared_ptr<Node>> &nodes);

  void append_impl(const std::vector<std::shared_ptr<Node>> &nodes);

  void replaceChildren_impl(const std::vector<std::shared_ptr<Node>> &nodes);

  void querySelectorsAll_impl(const std::shared_ptr<GRM::Selector> &selector,
                              std::vector<std::shared_ptr<GRM::Element>> &found_elements,
                              std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map);
  void querySelectorsAll_impl(const std::shared_ptr<GRM::Selector> &selector,
                              std::vector<std::shared_ptr<const GRM::Element>> &found_elements,
                              std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const;
  std::shared_ptr<Element>
  querySelectors_impl(const std::shared_ptr<GRM::Selector> &selector,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map);
  std::shared_ptr<const Element>
  querySelectors_impl(const std::shared_ptr<GRM::Selector> &selector,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const;

  // NonDocumentTypeChildNode interface implementations

  std::shared_ptr<Element> previousElementSibling_impl();
  std::shared_ptr<const Element> previousElementSibling_impl() const;

  std::shared_ptr<Element> nextElementSibling_impl();
  std::shared_ptr<const Element> nextElementSibling_impl() const;

  // virtual functions

  virtual std::shared_ptr<Node> cloneIndividualNode() = 0;

  // static functions

  static void set_owner_document_recursive(const std::shared_ptr<Node> &node,
                                           const std::shared_ptr<Document> &document);

  static bool children_are_equal_recursive(const std::shared_ptr<const Node> &left_node,
                                           const std::shared_ptr<const Node> &right_node);

private:
  std::shared_ptr<Document> nodeDocument();

  std::shared_ptr<const Document> nodeDocument() const;
  bool matchSelector(const std::shared_ptr<GRM::Selector> &selector,
                     std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const;

  Type m_type;
  std::weak_ptr<Document> m_owner_document;
  std::weak_ptr<Node> m_parent_node;
  std::list<std::shared_ptr<Node>> m_child_nodes;
};
} // namespace GRM

#endif
