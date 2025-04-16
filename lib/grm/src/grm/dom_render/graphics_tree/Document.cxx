#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/graphics_tree/Comment.hxx>
#include <grm/dom_render/graphics_tree/HierarchyRequestError.hxx>
#include <grm/dom_render/graphics_tree/NotSupportedError.hxx>

static void (*render)() = nullptr;
static void (*update)(const std::shared_ptr<GRM::Element> &, const std::string &, const std::string &) = nullptr;
static void (*context_update)(const std::shared_ptr<GRM::Element> &, const std::string &, const GRM::Value &) = nullptr;
static void (*context_delete)(const std::shared_ptr<GRM::Element> &) = nullptr;
static void (*element_cleanup)(GRM::Element &) = nullptr;
GRM::Document::Document() : GRM::Node(GRM::Node::Type::DOCUMENT_NODE, nullptr) {}

std::string GRM::Document::nodeName() const
{
  return "#document";
}

std::shared_ptr<GRM::Document> GRM::Document::createDocument()
{
  return std::shared_ptr<GRM::Document>(new GRM::Document());
}

std::shared_ptr<GRM::Element> GRM::Document::documentElement()
{
  return firstChildElement();
}

std::shared_ptr<const GRM::Element> GRM::Document::documentElement() const
{
  return firstChildElement();
}

std::shared_ptr<GRM::Element> GRM::Document::createElement(const std::string &local_name)
{
  return std::shared_ptr<GRM::Element>(new GRM::Element(local_name, shared()));
}

std::shared_ptr<GRM::Comment> GRM::Document::createComment(const std::string &data)
{
  return std::shared_ptr<GRM::Comment>(new GRM::Comment(data, shared()));
}

std::shared_ptr<GRM::Document> GRM::Document::shared()
{
  return std::static_pointer_cast<GRM::Document>(shared_from_this());
}

template <typename T, typename U>
static std::vector<std::shared_ptr<T>> getElementsByTagNameImpl(U &document, const std::string &qualified_name)
{
  std::string local_name = GRM::toLower(qualified_name);
  auto document_element = document.documentElement();
  if (!document_element)
    {
      return {};
    }
  auto found_elements = document_element->getElementsByTagName(qualified_name);
  if (local_name == "*" || local_name == document_element->localName())
    {
      found_elements.insert(found_elements.begin(), document_element);
    }
  return found_elements;
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Document::getElementsByTagName(const std::string &qualified_name)
{
  return ::getElementsByTagNameImpl<GRM::Element>(*this, qualified_name);
}
std::vector<std::shared_ptr<const GRM::Element>>
GRM::Document::getElementsByTagName(const std::string &qualified_name) const
{
  return ::getElementsByTagNameImpl<const GRM::Element>(*this, qualified_name);
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Document::getElementsByClassName(const std::string &class_names)
{
  return getElementsByClassNameImpl(class_names);
}

std::vector<std::shared_ptr<const GRM::Element>>
GRM::Document::getElementsByClassName(const std::string &class_names) const
{
  return getElementsByClassNameImpl(class_names);
}

std::shared_ptr<GRM::Element> GRM::Document::getElementById(const std::string &id)
{
  auto document_element = documentElement();
  if (!document_element) return {};
  return document_element->getElementById(id);
}

std::shared_ptr<const GRM::Element> GRM::Document::getElementById(const std::string &id) const
{
  auto document_element = documentElement();
  if (!document_element) return {};
  return document_element->getElementById(id);
}

void GRM::Document::prepend(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  prependImpl(nodes);
}

void GRM::Document::append(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  appendImpl(nodes);
}

void GRM::Document::replaceChildren(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  replaceChildrenImpl(nodes);
}

std::shared_ptr<GRM::Node> GRM::Document::cloneIndividualNode()
{
  auto document = GRM::Document::createDocument();
  *document = *this;
  return document;
}

template <typename T, typename U> static std::vector<std::shared_ptr<T>> childrenImpl(U &document)
{
  auto child_element = document.firstChildElement();
  if (child_element) return {child_element};
  return {};
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Document::children()
{
  return ::childrenImpl<GRM::Element>(*this);
}

std::vector<std::shared_ptr<const GRM::Element>> GRM::Document::children() const
{
  return ::childrenImpl<const GRM::Element>(*this);
}

std::shared_ptr<GRM::Element> GRM::Document::firstChildElement()
{
  return firstChildElementImpl();
}

std::shared_ptr<const GRM::Element> GRM::Document::firstChildElement() const
{
  return firstChildElementImpl();
}

std::shared_ptr<GRM::Element> GRM::Document::lastChildElement()
{
  return firstChildElement();
}

std::shared_ptr<const GRM::Element> GRM::Document::lastChildElement() const
{
  return firstChildElement();
}

unsigned long GRM::Document::childElementCount() const
{
  if (firstChildElement()) return 1;
  return 0;
}

std::shared_ptr<GRM::Node> GRM::Document::adoptNode(std::shared_ptr<GRM::Node> node)
{
  if (node->nodeType() == Type::DOCUMENT_NODE)
    {
      throw NotSupportedError("node must not be GRM::Document node");
    }
  auto old_document = node->ownerDocument();
  auto node_parent = node->parentNode();
  if (node_parent) node_parent->removeChild(node);
  if (old_document.get() != this) setOwnerDocumentRecursive(node, shared());

  return node;
}

std::shared_ptr<GRM::Node> GRM::Document::importNode(const std::shared_ptr<GRM::Node> &node, bool deep)
{
  auto clone = node->cloneNode(deep);
  adoptNode(clone);
  return clone;
}

std::shared_ptr<GRM::Document> GRM::createDocument()
{
  return Document::createDocument();
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Document::querySelectorsAll(const std::string &selectors)
{
  std::vector<std::shared_ptr<GRM::Element>> found_elements;
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  querySelectorsAllImpl(parseSelectors(selectors), found_elements, match_map);
  return found_elements;
}

std::vector<std::shared_ptr<const GRM::Element>> GRM::Document::querySelectorsAll(const std::string &selectors) const
{
  std::vector<std::shared_ptr<const GRM::Element>> found_elements;
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  querySelectorsAllImpl(parseSelectors(selectors), found_elements, match_map);
  return found_elements;
}

std::shared_ptr<GRM::Element> GRM::Document::querySelectors(const std::string &selectors)
{
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  return querySelectorsImpl(parseSelectors(selectors), match_map);
}

std::shared_ptr<const GRM::Element> GRM::Document::querySelectors(const std::string &selectors) const
{
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  return querySelectorsImpl(parseSelectors(selectors), match_map);
}

void GRM::Document::setUpdateFct(void (*ren)(), void (*upt)(const std::shared_ptr<GRM::Element> &, const std::string &,
                                                            const std::string &))
{
  render = ren;
  update = upt;
}

void GRM::Document::getUpdateFct(void (**ren)(), void (**upt)(const std::shared_ptr<GRM::Element> &,
                                                              const std::string &, const std::string &))
{
  *ren = render;
  *upt = update;
}

void GRM::Document::setContextFct(void (*del)(const std::shared_ptr<GRM::Element> &),
                                  void (*upt)(const std::shared_ptr<GRM::Element> &, const std::string &,
                                              const GRM::Value &))
{
  context_delete = del;
  context_update = upt;
}

void GRM::Document::getContextFct(void (**del)(const std::shared_ptr<GRM::Element> &),
                                  void (**upt)(const std::shared_ptr<GRM::Element> &, const std::string &,
                                               const GRM::Value &))
{
  *del = context_delete;
  *upt = context_update;
}

void GRM::Document::setElementCleanupFct(void (*cleanup)(GRM::Element &))
{
  element_cleanup = cleanup;
}

void (*GRM::Document::getElementCleanupFct())(GRM::Element &)
{
  return element_cleanup;
}
