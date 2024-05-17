#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/graphics_tree/HierarchyRequestError.hxx>
#include <iterator>
#include <grm/utilcpp_int.hxx>
#include <grm/dom_render/graphics_tree/TypeError.hxx>

GRM::Element::Element(std::string local_name, const std::shared_ptr<GRM::Document> &owner_document)
    : GRM::Node(GRM::Node::Type::ELEMENT_NODE, owner_document), m_local_name(std::move(local_name))
{
}

std::string GRM::Element::nodeName() const
{
  return tagName();
}

std::string GRM::Element::id() const
{
  return (std::string)getAttribute("id");
}

std::string GRM::Element::tagName() const
{
  return GRM::toupper(this->m_local_name);
}

std::string GRM::Element::localName() const
{
  return this->m_local_name;
}

bool GRM::Element::hasAttributes() const
{
  return !this->m_attributes.empty();
}

std::unordered_set<std::string> GRM::Element::getAttributeNames() const
{
  std::unordered_set<std::string> keys;
  keys.reserve(this->m_attributes.size());
  for (const auto &key : this->m_attributes)
    {
      keys.insert(key.first);
    }
  return keys;
}

GRM::Value GRM::Element::getAttribute(const std::string &qualifiedName) const
{
  if (!this->hasAttribute(qualifiedName)) return {};
  return this->m_attributes.at(qualifiedName);
}

void GRM::Element::setAttribute(const std::string &qualifiedName, const GRM::Value &value)
{
  GRM::Value old_value;
  void (*render)() = nullptr;
  void (*update)(const std::shared_ptr<GRM::Element> &, const std::string &, const std::string &) = nullptr;
  void (*contextUpdate)(const std::shared_ptr<GRM::Element> &, const std::string &, const GRM::Value &) = nullptr;
  void (*contextDelete)(const std::shared_ptr<GRM::Element> &) = nullptr;
  ownerDocument()->getUpdateFct(&render, &update);
  ownerDocument()->getContextFct(&contextDelete, &contextUpdate);

  if (hasAttribute(qualifiedName)) old_value = this->m_attributes[qualifiedName];

  this->m_attributes[qualifiedName] = value;
  if (value != old_value)
    {
      auto elem_p = std::static_pointer_cast<Element>(shared_from_this());
      if (contextUpdate) contextUpdate(elem_p, qualifiedName, old_value);
      if (qualifiedName == "kind")
        {
          ;
        }
      if (qualifiedName == "viewport_x_min" || qualifiedName == "viewport_x_max" || qualifiedName == "viewport_y_min" ||
          qualifiedName == "viewport_y_max")
        {
          if (update) update(elem_p, qualifiedName, std::to_string(static_cast<double>(old_value)));
        }
      else
        {
          if (update) update(elem_p, qualifiedName, static_cast<std::string>(old_value));
        }
      if (render) render();
    }
}

void GRM::Element::setAttribute(const std::string &qualifiedName, const std::string &value)
{
  setAttribute(qualifiedName, GRM::Value(value));
}

void GRM::Element::setAttribute(const std::string &qualifiedName, const double &value)
{
  setAttribute(qualifiedName, GRM::Value(value));
}

void GRM::Element::setAttribute(const std::string &qualifiedName, const int &value)
{
  setAttribute(qualifiedName, GRM::Value(value));
}

void GRM::Element::removeAttribute(const std::string &qualifiedName)
{
  this->m_attributes.erase(qualifiedName);
}

bool GRM::Element::toggleAttribute(const std::string &qualifiedName)
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

bool GRM::Element::toggleAttribute(const std::string &qualifiedName, bool force)
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

bool GRM::Element::hasAttribute(const std::string &qualifiedName) const
{
  return this->m_attributes.find(qualifiedName) != GRM::Element::m_attributes.end();
}

template <typename T>
static std::vector<std::shared_ptr<T>> getElementsByTagName_impl(T &element, const std::string &qualifiedName)
{
  std::string local_name = qualifiedName;
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

std::vector<std::shared_ptr<GRM::Element>> GRM::Element::getElementsByTagName(const std::string &qualifiedName)
{
  return getElementsByTagName_impl(*this, qualifiedName);
}
std::vector<std::shared_ptr<const GRM::Element>>
GRM::Element::getElementsByTagName(const std::string &qualifiedName) const
{
  return getElementsByTagName_impl(*this, qualifiedName);
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Element::getElementsByClassName(const std::string &classNames)
{
  return getElementsByClassName_impl(classNames);
}

std::vector<std::shared_ptr<const GRM::Element>>
GRM::Element::getElementsByClassName(const std::string &classNames) const
{
  return getElementsByClassName_impl(classNames);
}

void GRM::Element::before(std::shared_ptr<GRM::Element> node)
{
  if (!parentNode())
    {
      throw HierarchyRequestError("element is root node");
    }
  parentNode()->insertBefore(node, shared_from_this());
}

void GRM::Element::after(std::shared_ptr<GRM::Element> node)
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

void GRM::Element::replaceWith(const std::shared_ptr<GRM::Element> &node)
{
  if (!parentNode())
    {
      throw HierarchyRequestError("element is root node");
    }
  parentNode()->replaceChild(node, shared_from_this());
}

void GRM::Element::remove()
{
  void (*contextUpdate)(const std::shared_ptr<GRM::Element> &, const std::string &, const GRM::Value &) = nullptr;
  void (*contextDelete)(const std::shared_ptr<GRM::Element> &) = nullptr;
  ownerDocument()->getContextFct(&contextDelete, &contextUpdate);

  if (!parentNode())
    {
      throw HierarchyRequestError("element is root node");
    }

  auto elem_p = std::static_pointer_cast<Element>(shared_from_this());
  contextDelete(elem_p);

  parentNode()->removeChild(shared_from_this());
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Element::children()
{
  return children_impl();
}

std::vector<std::shared_ptr<const GRM::Element>> GRM::Element::children() const
{
  return children_impl();
}

std::shared_ptr<GRM::Element> GRM::Element::firstChildElement()
{
  return firstChildElement_impl();
}

std::shared_ptr<const GRM::Element> GRM::Element::firstChildElement() const
{
  return firstChildElement_impl();
}

std::shared_ptr<GRM::Element> GRM::Element::lastChildElement()
{
  return lastChildElement_impl();
}

std::shared_ptr<const GRM::Element> GRM::Element::lastChildElement() const
{
  return lastChildElement_impl();
}

unsigned long GRM::Element::childElementCount() const
{
  return childElementCount_impl();
}

void GRM::Element::prepend(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  prepend_impl(nodes);
}

void GRM::Element::append(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  append_impl(nodes);
}

void GRM::Element::replaceChildren(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
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

std::shared_ptr<GRM::Element> GRM::Element::getElementById(const std::string &id)
{
  return getElementById_impl(shared(), id);
}
std::shared_ptr<const GRM::Element> GRM::Element::getElementById(const std::string &id) const
{
  return getElementById_impl(shared(), id);
}

std::shared_ptr<GRM::Element> GRM::Element::shared()
{
  return std::static_pointer_cast<GRM::Element>(shared_from_this());
}

std::shared_ptr<const GRM::Element> GRM::Element::shared() const
{
  return std::static_pointer_cast<const GRM::Element>(shared_from_this());
}

std::shared_ptr<GRM::Node> GRM::Element::cloneIndividualNode()
{
  auto element = std::shared_ptr<GRM::Element>(new GRM::Element(m_local_name, ownerDocument()));
  *element = *this;
  return element;
}

bool GRM::Element::isEqualNode(const std::shared_ptr<const GRM::Node> &otherNode) const
{
  auto other_node_as_element = std::dynamic_pointer_cast<const GRM::Element>(otherNode);
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
  return GRM::Node::children_are_equal_recursive(shared_from_this(), otherNode);
}

std::shared_ptr<GRM::Element> GRM::Element::previousElementSibling()
{
  return previousElementSibling_impl();
}

std::shared_ptr<const GRM::Element> GRM::Element::previousElementSibling() const
{
  return previousElementSibling_impl();
}

std::shared_ptr<GRM::Element> GRM::Element::nextElementSibling()
{
  return nextElementSibling_impl();
}

std::shared_ptr<const GRM::Element> GRM::Element::nextElementSibling() const
{
  return nextElementSibling_impl();
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Element::querySelectorsAll(const std::string &selectors)
{
  std::vector<std::shared_ptr<GRM::Element>> found_elements;
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  querySelectorsAll_impl(parseSelectors(selectors), found_elements, match_map);
  return found_elements;
}

std::vector<std::shared_ptr<const GRM::Element>> GRM::Element::querySelectorsAll(const std::string &selectors) const
{
  std::vector<std::shared_ptr<const GRM::Element>> found_elements;
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  querySelectorsAll_impl(parseSelectors(selectors), found_elements, match_map);
  return found_elements;
}

std::shared_ptr<GRM::Element> GRM::Element::querySelectors(const std::string &selectors)
{
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  return querySelectors_impl(parseSelectors(selectors), match_map);
}

std::shared_ptr<const GRM::Element> GRM::Element::querySelectors(const std::string &selectors) const
{
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  return querySelectors_impl(parseSelectors(selectors), match_map);
}
