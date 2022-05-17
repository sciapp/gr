#ifndef GR_GRAPHICS_TREE_INTERFACE_DOCUMENT_HXX
#define GR_GRAPHICS_TREE_INTERFACE_DOCUMENT_HXX

#ifndef GRM_INCLUDE_PATH_
#define GRM_QUOTE_(x) #x
#ifdef BUILDING_GR
#define GRM_INCLUDE_PATH_(filename) GRM_QUOTE_(filename)
#else
/* clang-format off */
#define GRM_INCLUDE_PATH_(filename) GRM_QUOTE_(grm/filename)
/* clang-format on */
#endif
#endif

#include <memory>
#include <string>

/* clang-format off */
#include GRM_INCLUDE_PATH_(dom_render/graphics_tree/Node.hxx)
/* clang-format on */

namespace GR
{
class Element;
class Comment;

class Document : public Node
{
public:
  static std::shared_ptr<Document> createDocument();

  // Document interface
  std::shared_ptr<Element> documentElement();

  std::shared_ptr<const Element> documentElement() const;

  std::shared_ptr<Element> createElement(const std::string &localName);

  std::shared_ptr<Comment> createComment(const std::string &data);

  std::vector<std::shared_ptr<Element>> getElementsByTagName(const std::string &qualifiedName);

  std::vector<std::shared_ptr<const Element>> getElementsByTagName(const std::string &qualifiedName) const;

  std::vector<std::shared_ptr<Element>> getElementsByClassName(const std::string &classNames);

  std::vector<std::shared_ptr<const Element>> getElementsByClassName(const std::string &classNames) const;

  std::shared_ptr<Element> getElementById(const std::string &id);

  std::shared_ptr<const Element> getElementById(const std::string &id) const;

  std::shared_ptr<Node> adoptNode(std::shared_ptr<Node> node);

  std::shared_ptr<Node> importNode(const std::shared_ptr<Node> &node, bool deep = false);

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

  // virtual functions
  std::string nodeName() const override;

protected:
  Document();

private:
  std::shared_ptr<Node> cloneIndividualNode() override;

  std::shared_ptr<Document> shared();
};

std::shared_ptr<Document> createDocument();
} // namespace GR

#endif
