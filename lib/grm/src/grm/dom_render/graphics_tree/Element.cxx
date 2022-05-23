#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/graphics_tree/HierarchyRequestError.hxx>
#include <iterator>

GR::Element::Element(std::string local_name, const std::shared_ptr<GR::Document> &owner_document)
    : GR::Node(GR::Node::Type::ELEMENT_NODE, owner_document), m_local_name(tolower(std::move(local_name)))
{
}

std::string GR::Element::nodeName() const
{
  return tagName();
}

std::string GR::Element::id() const
{
  return (std::string)getAttribute("id");
}

std::string GR::Element::tagName() const
{
  return GR::toupper(this->m_local_name);
}

std::string GR::Element::localName() const
{
  return this->m_local_name;
}

bool GR::Element::hasAttributes() const
{
  return !this->m_attributes.empty();
}

std::unordered_set<std::string> GR::Element::getAttributeNames() const
{
  std::unordered_set<std::string> keys;
  keys.reserve(this->m_attributes.size());
  for (const auto &key : this->m_attributes)
    {
      keys.insert(key.first);
    }
  return keys;
}

GR::Value GR::Element::getAttribute(const std::string &qualifiedName) const
{
  if (!this->hasAttribute(qualifiedName)) return {};
  return this->m_attributes.at(qualifiedName);
}

void GR::Element::setAttribute(const std::string &qualifiedName, const GR::Value &value)
{
  this->m_attributes[qualifiedName] = value;
}

void GR::Element::setAttribute(const std::string &qualifiedName, const std::string &value)
{
  setAttribute(qualifiedName, GR::Value(value));
}

void GR::Element::setAttribute(const std::string &qualifiedName, const double &value)
{
  setAttribute(qualifiedName, GR::Value(value));
}

void GR::Element::setAttribute(const std::string &qualifiedName, const int &value)
{
  setAttribute(qualifiedName, GR::Value(value));
}

void GR::Element::removeAttribute(const std::string &qualifiedName)
{
  this->m_attributes.erase(qualifiedName);
}

bool GR::Element::toggleAttribute(const std::string &qualifiedName)
{
  bool has_attribute = hasAttribute(qualifiedName);
  if (has_attribute)
    {
      removeAttribute(qualifiedName);
    }
  else
    {
      setAttribute(qualifiedName, "");
    }
  return !has_attribute;
}

bool GR::Element::toggleAttribute(const std::string &qualifiedName, bool force)
{
  bool has_attribute = hasAttribute(qualifiedName);
  if (force)
    {
      if (!has_attribute)
        {
          setAttribute(qualifiedName, "");
        }
    }
  else
    {
      if (has_attribute)
        {
          removeAttribute(qualifiedName);
        }
    }
  return force;
}

bool GR::Element::hasAttribute(const std::string &qualifiedName) const
{
  return this->m_attributes.find(qualifiedName) != GR::Element::m_attributes.end();
}

template <typename T>
static std::vector<std::shared_ptr<T>> getElementsByTagName_impl(T &element, const std::string &qualifiedName)
{
  std::string local_name = GR::tolower(qualifiedName);
  std::vector<std::shared_ptr<T>> found_elements;
  for (const auto &child_element : element.children())
    {
      if (local_name == "*" || child_element->localName() == local_name)
        {
          found_elements.push_back(child_element);
        }
      auto child_found_elements = child_element->getElementsByTagName(qualifiedName);
      found_elements.insert(found_elements.end(), child_found_elements.begin(), child_found_elements.end());
    }
  return found_elements;
}

std::vector<std::shared_ptr<GR::Element>> GR::Element::getElementsByTagName(const std::string &qualifiedName)
{
  return getElementsByTagName_impl(*this, qualifiedName);
}
std::vector<std::shared_ptr<const GR::Element>>
GR::Element::getElementsByTagName(const std::string &qualifiedName) const
{
  return getElementsByTagName_impl(*this, qualifiedName);
}

std::vector<std::shared_ptr<GR::Element>> GR::Element::getElementsByClassName(const std::string &classNames)
{
  return getElementsByClassName_impl(classNames);
}

std::vector<std::shared_ptr<const GR::Element>> GR::Element::getElementsByClassName(const std::string &classNames) const
{
  return getElementsByClassName_impl(classNames);
}


void GR::Element::before(std::shared_ptr<GR::Element> node)
{
  if (!parentNode())
    {
      throw HierarchyRequestError("element is root node");
    }
  parentNode()->insertBefore(node, shared_from_this());
}

void GR::Element::after(std::shared_ptr<GR::Element> node)
{
  if (!parentNode())
    {
      throw HierarchyRequestError("element is root node");
    }
  auto next_sibling = nextSibling();
  if (next_sibling)
    {
      parentNode()->insertBefore(node, next_sibling);
    }
  else
    {
      parentNode()->appendChild(node);
    }
}

void GR::Element::replaceWith(const std::shared_ptr<GR::Element> &node)
{
  if (!parentNode())
    {
      throw HierarchyRequestError("element is root node");
    }
  parentNode()->replaceChild(node, shared_from_this());
}

void GR::Element::remove()
{
  if (!parentNode())
    {
      throw HierarchyRequestError("element is root node");
    }
  parentNode()->removeChild(shared_from_this());
}

std::vector<std::shared_ptr<GR::Element>> GR::Element::children()
{
  return children_impl();
}

std::vector<std::shared_ptr<const GR::Element>> GR::Element::children() const
{
  return children_impl();
}

std::shared_ptr<GR::Element> GR::Element::firstChildElement()
{
  return firstChildElement_impl();
}

std::shared_ptr<const GR::Element> GR::Element::firstChildElement() const
{
  return firstChildElement_impl();
}

std::shared_ptr<GR::Element> GR::Element::lastChildElement()
{
  return lastChildElement_impl();
}

std::shared_ptr<const GR::Element> GR::Element::lastChildElement() const
{
  return lastChildElement_impl();
}

unsigned long GR::Element::childElementCount() const
{
  return childElementCount_impl();
}

void GR::Element::prepend(const std::vector<std::shared_ptr<GR::Node>> &nodes)
{
  prepend_impl(nodes);
}

void GR::Element::append(const std::vector<std::shared_ptr<GR::Node>> &nodes)
{
  append_impl(nodes);
}

void GR::Element::replaceChildren(const std::vector<std::shared_ptr<GR::Node>> &nodes)
{
  replaceChildren_impl(nodes);
}

template <typename T> static std::shared_ptr<T> getElementById_impl(std::shared_ptr<T> element, const std::string &id)
{
  if (element->id() == id)
    {
      return element;
    }
  for (const auto &child_element : element->children())
    {
      auto child_found_element = getElementById_impl(child_element, id);
      if (child_found_element)
        {
          return child_found_element;
        }
    }
  return nullptr;
}

std::shared_ptr<GR::Element> GR::Element::getElementById(const std::string &id)
{
  return getElementById_impl(shared(), id);
}
std::shared_ptr<const GR::Element> GR::Element::getElementById(const std::string &id) const
{
  return getElementById_impl(shared(), id);
}

std::shared_ptr<GR::Element> GR::Element::shared()
{
  return std::static_pointer_cast<GR::Element>(shared_from_this());
}

std::shared_ptr<const GR::Element> GR::Element::shared() const
{
  return std::static_pointer_cast<const GR::Element>(shared_from_this());
}

std::shared_ptr<GR::Node> GR::Element::cloneIndividualNode()
{
  auto element = std::shared_ptr<GR::Element>(new GR::Element(m_local_name, ownerDocument()));
  *element = *this;
  return element;
}

bool GR::Element::isEqualNode(const std::shared_ptr<const GR::Node> &otherNode) const
{
  auto other_node_as_element = std::dynamic_pointer_cast<const GR::Element>(otherNode);
  if (!other_node_as_element)
    {
      return false;
    }
  if (other_node_as_element->localName() != localName())
    {
      return false;
    }
  if (other_node_as_element->m_attributes.size() != m_attributes.size())
    {
      return false;
    }
  for (const auto &attribute_entry : other_node_as_element->m_attributes)
    {
      if (m_attributes.find(attribute_entry.first) == m_attributes.end())
        {
          return false;
        }
      if (m_attributes.at(attribute_entry.first) != attribute_entry.second)
        {
          return false;
        }
    }
  return GR::Node::children_are_equal_recursive(shared_from_this(), otherNode);
}

std::shared_ptr<GR::Element> GR::Element::previousElementSibling()
{
  return previousElementSibling_impl();
}

std::shared_ptr<const GR::Element> GR::Element::previousElementSibling() const
{
  return previousElementSibling_impl();
}

std::shared_ptr<GR::Element> GR::Element::nextElementSibling()
{
  return nextElementSibling_impl();
}

std::shared_ptr<const GR::Element> GR::Element::nextElementSibling() const
{
  return nextElementSibling_impl();
}

std::vector<std::shared_ptr<GR::Element>> GR::Element::querySelectorsAll(const std::string &selectors)
{
  std::vector<std::shared_ptr<GR::Element>> found_elements;
  std::map<std::tuple<const GR::Element *, const GR::Selector *>, bool> match_map;
  querySelectorsAll_impl(parseSelectors(selectors), found_elements, match_map);
  return found_elements;
}

std::vector<std::shared_ptr<const GR::Element>> GR::Element::querySelectorsAll(const std::string &selectors) const
{
  std::vector<std::shared_ptr<const GR::Element>> found_elements;
  std::map<std::tuple<const GR::Element *, const GR::Selector *>, bool> match_map;
  querySelectorsAll_impl(parseSelectors(selectors), found_elements, match_map);
  return found_elements;
}

std::shared_ptr<GR::Element> GR::Element::querySelectors(const std::string &selectors)
{
  std::map<std::tuple<const GR::Element *, const GR::Selector *>, bool> match_map;
  return querySelectors_impl(parseSelectors(selectors), match_map);
}

std::shared_ptr<const GR::Element> GR::Element::querySelectors(const std::string &selectors) const
{
  std::map<std::tuple<const GR::Element *, const GR::Selector *>, bool> match_map;
  return querySelectors_impl(parseSelectors(selectors), match_map);
}
