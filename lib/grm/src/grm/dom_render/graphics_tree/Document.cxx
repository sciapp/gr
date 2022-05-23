#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/graphics_tree/Comment.hxx>
#include <grm/dom_render/graphics_tree/HierarchyRequestError.hxx>
#include <grm/dom_render/graphics_tree/NotSupportedError.hxx>

GR::Document::Document() : GR::Node(GR::Node::Type::DOCUMENT_NODE, nullptr) {}

std::string GR::Document::nodeName() const
{
  return "#document";
}

std::shared_ptr<GR::Document> GR::Document::createDocument()
{
  return std::shared_ptr<GR::Document>(new GR::Document());
}

std::shared_ptr<GR::Element> GR::Document::documentElement()
{
  return firstChildElement();
}

std::shared_ptr<const GR::Element> GR::Document::documentElement() const
{
  return firstChildElement();
}

std::shared_ptr<GR::Element> GR::Document::createElement(const std::string &localName)
{
  auto result = std::shared_ptr<GR::Element>(new GR::Element(localName, shared()));
  return result;
}

std::shared_ptr<GR::Comment> GR::Document::createComment(const std::string &data)
{
  auto result = std::shared_ptr<GR::Comment>(new GR::Comment(data, shared()));
  return result;
}

std::shared_ptr<GR::Document> GR::Document::shared()
{
  return std::static_pointer_cast<GR::Document>(shared_from_this());
}

template <typename T, typename U>
static std::vector<std::shared_ptr<T>> getElementsByTagName_impl(U &document, const std::string &qualifiedName)
{
  std::string local_name = GR::tolower(qualifiedName);
  auto document_element = document.documentElement();
  if (!document_element)
    {
      return {};
    }
  auto found_elements = document_element->getElementsByTagName(qualifiedName);
  if (local_name == "*" || local_name == document_element->localName())
    {
      found_elements.insert(found_elements.begin(), document_element);
    }
  return found_elements;
}

std::vector<std::shared_ptr<GR::Element>> GR::Document::getElementsByTagName(const std::string &qualifiedName)
{
  return ::getElementsByTagName_impl<GR::Element>(*this, qualifiedName);
}
std::vector<std::shared_ptr<const GR::Element>>
GR::Document::getElementsByTagName(const std::string &qualifiedName) const
{
  return ::getElementsByTagName_impl<const GR::Element>(*this, qualifiedName);
}

std::vector<std::shared_ptr<GR::Element>> GR::Document::getElementsByClassName(const std::string &classNames)
{
  return getElementsByClassName_impl(classNames);
}

std::vector<std::shared_ptr<const GR::Element>>
GR::Document::getElementsByClassName(const std::string &classNames) const
{
  return getElementsByClassName_impl(classNames);
}

std::shared_ptr<GR::Element> GR::Document::getElementById(const std::string &id)
{
  auto document_element = documentElement();
  if (!document_element)
    {
      return {};
    }
  return document_element->getElementById(id);
}

std::shared_ptr<const GR::Element> GR::Document::getElementById(const std::string &id) const
{
  auto document_element = documentElement();
  if (!document_element)
    {
      return {};
    }
  return document_element->getElementById(id);
}

void GR::Document::prepend(const std::vector<std::shared_ptr<GR::Node>> &nodes)
{
  prepend_impl(nodes);
}

void GR::Document::append(const std::vector<std::shared_ptr<GR::Node>> &nodes)
{
  append_impl(nodes);
}

void GR::Document::replaceChildren(const std::vector<std::shared_ptr<GR::Node>> &nodes)
{
  replaceChildren_impl(nodes);
}

std::shared_ptr<GR::Node> GR::Document::cloneIndividualNode()
{
  auto document = GR::Document::createDocument();
  *document = *this;
  return document;
}

template <typename T, typename U> static std::vector<std::shared_ptr<T>> children_impl(U &document)
{
  auto child_element = document.firstChildElement();
  if (child_element)
    {
      return {child_element};
    }
  return {};
}

std::vector<std::shared_ptr<GR::Element>> GR::Document::children()
{
  return ::children_impl<GR::Element>(*this);
}

std::vector<std::shared_ptr<const GR::Element>> GR::Document::children() const
{
  return ::children_impl<const GR::Element>(*this);
}

std::shared_ptr<GR::Element> GR::Document::firstChildElement()
{
  return firstChildElement_impl();
}

std::shared_ptr<const GR::Element> GR::Document::firstChildElement() const
{
  return firstChildElement_impl();
}

std::shared_ptr<GR::Element> GR::Document::lastChildElement()
{
  return firstChildElement();
}

std::shared_ptr<const GR::Element> GR::Document::lastChildElement() const
{
  return firstChildElement();
}

unsigned long GR::Document::childElementCount() const
{
  if (firstChildElement())
    {
      return 1;
    }
  return 0;
}

std::shared_ptr<GR::Node> GR::Document::adoptNode(std::shared_ptr<GR::Node> node)
{
  if (node->nodeType() == Type::DOCUMENT_NODE)
    {
      throw NotSupportedError("node must not be GR::Document node");
    }
  auto old_document = node->ownerDocument();
  auto node_parent = node->parentNode();
  if (node_parent)
    {
      node_parent->removeChild(node);
    }
  if (old_document.get() != this)
    {
      set_owner_document_recursive(node, shared());
    }
  return node;
}

std::shared_ptr<GR::Node> GR::Document::importNode(const std::shared_ptr<GR::Node> &node, bool deep)
{
  auto clone = node->cloneNode(deep);
  adoptNode(clone);
  return clone;
}

std::shared_ptr<GR::Document> GR::createDocument()
{
  return Document::createDocument();
}

std::vector<std::shared_ptr<GR::Element>> GR::Document::querySelectorsAll(const std::string &selectors)
{
  std::vector<std::shared_ptr<GR::Element>> found_elements;
  std::map<std::tuple<const GR::Element *, const GR::Selector *>, bool> match_map;
  querySelectorsAll_impl(parseSelectors(selectors), found_elements, match_map);
  return found_elements;
}

std::vector<std::shared_ptr<const GR::Element>> GR::Document::querySelectorsAll(const std::string &selectors) const
{
  std::vector<std::shared_ptr<const GR::Element>> found_elements;
  std::map<std::tuple<const GR::Element *, const GR::Selector *>, bool> match_map;
  querySelectorsAll_impl(parseSelectors(selectors), found_elements, match_map);
  return found_elements;
}

std::shared_ptr<GR::Element> GR::Document::querySelectors(const std::string &selectors)
{
  std::map<std::tuple<const GR::Element *, const GR::Selector *>, bool> match_map;
  return querySelectors_impl(parseSelectors(selectors), match_map);
}

std::shared_ptr<const GR::Element> GR::Document::querySelectors(const std::string &selectors) const
{
  std::map<std::tuple<const GR::Element *, const GR::Selector *>, bool> match_map;
  return querySelectors_impl(parseSelectors(selectors), match_map);
}
