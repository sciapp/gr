#ifndef GRM_GRAPHICS_TREE_INTERFACE_DOCUMENT_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_DOCUMENT_HXX

#include <memory>
#include <string>

#include <grm/dom_render/graphics_tree/node.hxx>
#include <grm/dom_render/graphics_tree/value.hxx>
#include <grm/util.h>

namespace GRM
{
class Element;
class Comment;

class GRM_EXPORT Document : public Node
{
public:
  static std::shared_ptr<Document> createDocument();

  // Document interface
  std::shared_ptr<Element> documentElement();

  std::shared_ptr<const Element> documentElement() const;

  std::shared_ptr<Element> createElement(const std::string &local_name);

  std::shared_ptr<Comment> createComment(const std::string &data);

  std::vector<std::shared_ptr<Element>> getElementsByTagName(const std::string &qualified_name);

  std::vector<std::shared_ptr<const Element>> getElementsByTagName(const std::string &qualified_name) const;

  std::vector<std::shared_ptr<Element>> getElementsByClassName(const std::string &class_names);

  std::vector<std::shared_ptr<const Element>> getElementsByClassName(const std::string &class_names) const;

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

  void setUpdateFct(void (*ren)(),
                    void (*upt)(const std::shared_ptr<Element> &, const std::string &, const std::string &));
  void getUpdateFct(void (**ren)(),
                    void (**upt)(const std::shared_ptr<Element> &, const std::string &, const std::string &));

  void setContextFct(void (*del)(const std::shared_ptr<Element> &),
                     void (*upt)(const std::shared_ptr<Element> &, const std::string &, const Value &));
  void getContextFct(void (**del)(const std::shared_ptr<Element> &),
                     void (**upt)(const std::shared_ptr<Element> &, const std::string &, const Value &));

  void setElementCleanupFct(void (*cleanup)(Element &));
  void (*getElementCleanupFct())(Element &);

protected:
  Document();

private:
  std::shared_ptr<Node> cloneIndividualNode() override;

  std::shared_ptr<Document> shared();
};

GRM_EXPORT std::shared_ptr<Document> createDocument();
} // namespace GRM

#endif
