#ifndef GRM_GRAPHICS_TREE_INTERFACE_ELEMENT_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_ELEMENT_HXX

#include <unordered_map>
#include <unordered_set>

#include <grm/dom_render/graphics_tree/node.hxx>
#include <grm/dom_render/graphics_tree/value.hxx>
#include <grm/util.h>

namespace GRM
{
class Document;

class GRM_EXPORT Element : public Node
{
public:
  virtual ~Element();

  // Element interface
  std::string localName() const;

  std::string tagName() const;

  std::string id() const;

  bool hasAttributes() const;

  std::unordered_set<std::string> getAttributeNames() const;

  Value getAttribute(const std::string &qualified_name) const;

  void setAttribute(const std::string &qualified_name, const Value &value);

  void setAttribute(const std::string &qualified_name, const std::string &value);

  void setAttribute(const std::string &qualified_name, const double &value);

  void setAttribute(const std::string &qualified_name, const int &value);

  void removeAttribute(const std::string &qualified_name);

  bool toggleAttribute(const std::string &qualified_name);

  bool toggleAttribute(const std::string &qualified_name, bool force);

  bool hasAttribute(const std::string &qualified_name) const;

  std::vector<std::shared_ptr<Element>> getElementsByTagName(const std::string &qualified_name);

  std::vector<std::shared_ptr<const Element>> getElementsByTagName(const std::string &qualified_name) const;

  std::vector<std::shared_ptr<Element>> getElementsByClassName(const std::string &class_names);

  std::vector<std::shared_ptr<const Element>> getElementsByClassName(const std::string &class_names) const;

  // ChildNode interface
  void before(std::shared_ptr<Element> node);

  void after(std::shared_ptr<Element> node);

  void replaceWith(const std::shared_ptr<Element> &node);

  void remove();

  // ParentNode interface
  std::vector<std::shared_ptr<Element>> children();

  std::vector<std::shared_ptr<const Element>> children() const;

  std::shared_ptr<Element> firstChildElement();

  std::shared_ptr<const Element> firstChildElement() const;

  std::shared_ptr<Element> lastChildElement();

  std::shared_ptr<const Element> lastChildElement() const;

  unsigned long childElementCount() const;

  template <class... NodeT> void prepend(std::shared_ptr<NodeT>... nodes)
  {
    prepend(std::vector<std::shared_ptr<Node>>{nodes...});
  }

  void prepend(const std::vector<std::shared_ptr<Node>> &nodes);

  template <class... NodeT> void append(std::shared_ptr<NodeT>... nodes)
  {
    append(std::vector<std::shared_ptr<Node>>{nodes...});
  }

  void append(const std::vector<std::shared_ptr<Node>> &nodes);

  template <class... NodeT> void replaceChildren(std::shared_ptr<NodeT>... nodes)
  {
    replaceChildren(std::vector<std::shared_ptr<Node>>{nodes...});
  }

  void replaceChildren(const std::vector<std::shared_ptr<Node>> &nodes);

  std::vector<std::shared_ptr<Element>> querySelectorsAll(const std::string &selectors);
  std::vector<std::shared_ptr<const Element>> querySelectorsAll(const std::string &selectors) const;
  std::shared_ptr<Element> querySelectors(const std::string &selectors);
  std::shared_ptr<const Element> querySelectors(const std::string &selectors) const;

  // NonDocumentTypeChildNode interface

  std::shared_ptr<Element> previousElementSibling();
  std::shared_ptr<const Element> previousElementSibling() const;

  std::shared_ptr<Element> nextElementSibling();
  std::shared_ptr<const Element> nextElementSibling() const;

  // virtual functions
  std::string nodeName() const override;

  bool isEqualNode(const std::shared_ptr<const Node> &other_node) const override;

private:
  friend class Document;

  Element(std::string local_name, const std::shared_ptr<Document> &owner_document);

  std::shared_ptr<Element> getElementById(const std::string &id);

  std::shared_ptr<const Element> getElementById(const std::string &id) const;

  std::shared_ptr<Element> shared();

  std::shared_ptr<const Element> shared() const;

  std::string m_local_name;
  std::unordered_map<std::string, Value> m_attributes;

  std::shared_ptr<Node> cloneIndividualNode() override;
};
} // namespace GRM

#endif
