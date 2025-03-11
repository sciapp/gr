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

GRM::Element::~Element()
{
  auto cleanup_element = ownerDocument()->getElementCleanupFct();
  if (cleanup_element) cleanup_element(*this);
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
  return GRM::toUpper(this->m_local_name);
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

GRM::Value GRM::Element::getAttribute(const std::string &qualified_name) const
{
  if (!this->hasAttribute(qualified_name)) return {};
  return this->m_attributes.at(qualified_name);
}

void GRM::Element::setAttribute(const std::string &qualified_name, const GRM::Value &value)
{
  GRM::Value old_value;
  void (*render)() = nullptr;
  void (*update)(const std::shared_ptr<GRM::Element> &, const std::string &, const std::string &) = nullptr;
  void (*context_update)(const std::shared_ptr<GRM::Element> &, const std::string &, const GRM::Value &) = nullptr;
  void (*context_delete)(const std::shared_ptr<GRM::Element> &) = nullptr;
  ownerDocument()->getUpdateFct(&render, &update);
  ownerDocument()->getContextFct(&context_delete, &context_update);

  if (hasAttribute(qualified_name)) old_value = this->m_attributes[qualified_name];

  this->m_attributes[qualified_name] = value;
  if (value != old_value)
    {
      auto elem_p = std::static_pointer_cast<Element>(shared_from_this());
      if (context_update) context_update(elem_p, qualified_name, old_value);
      if (qualified_name == "kind")
        {
          ;
        }
      if (qualified_name == "viewport_x_min" || qualified_name == "viewport_x_max" ||
          qualified_name == "viewport_y_min" || qualified_name == "viewport_y_max")
        {
          if (update) update(elem_p, qualified_name, std::to_string(static_cast<double>(old_value)));
        }
      else
        {
          if (update) update(elem_p, qualified_name, static_cast<std::string>(old_value));
        }
      if (render) render();
    }
}

void GRM::Element::setAttribute(const std::string &qualified_name, const std::string &value)
{
  setAttribute(qualified_name, GRM::Value(value));
}

void GRM::Element::setAttribute(const std::string &qualified_name, const double &value)
{
  setAttribute(qualified_name, GRM::Value(value));
}

void GRM::Element::setAttribute(const std::string &qualified_name, const int &value)
{
  setAttribute(qualified_name, GRM::Value(value));
}

void GRM::Element::removeAttribute(const std::string &qualified_name)
{
  this->m_attributes.erase(qualified_name);
}

bool GRM::Element::toggleAttribute(const std::string &qualified_name)
{
  bool has_attribute = hasAttribute(qualified_name);
  if (has_attribute)
    {
      removeAttribute(qualified_name);
    }
  else
    {
      setAttribute(qualified_name, "");
    }
  return !has_attribute;
}

bool GRM::Element::toggleAttribute(const std::string &qualified_name, bool force)
{
  bool has_attribute = hasAttribute(qualified_name);
  if (force)
    {
      if (!has_attribute) setAttribute(qualified_name, "");
    }
  else
    {
      if (has_attribute) removeAttribute(qualified_name);
    }
  return force;
}

bool GRM::Element::hasAttribute(const std::string &qualified_name) const
{
  return this->m_attributes.find(qualified_name) != m_attributes.end();
}

template <typename T>
static std::vector<std::shared_ptr<T>> getElementsByTagNameImpl(T &element, const std::string &qualified_name)
{
  std::string local_name = qualified_name;
  std::vector<std::shared_ptr<T>> found_elements;
  for (const auto &child_element : element.children())
    {
      if (local_name == "*" || child_element->localName() == local_name) found_elements.push_back(child_element);
      auto child_found_elements = child_element->getElementsByTagName(qualified_name);
      found_elements.insert(found_elements.end(), child_found_elements.begin(), child_found_elements.end());
    }
  return found_elements;
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Element::getElementsByTagName(const std::string &qualified_name)
{
  return getElementsByTagNameImpl(*this, qualified_name);
}
std::vector<std::shared_ptr<const GRM::Element>>
GRM::Element::getElementsByTagName(const std::string &qualified_name) const
{
  return getElementsByTagNameImpl(*this, qualified_name);
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Element::getElementsByClassName(const std::string &class_names)
{
  return getElementsByClassNameImpl(class_names);
}

std::vector<std::shared_ptr<const GRM::Element>>
GRM::Element::getElementsByClassName(const std::string &class_names) const
{
  return getElementsByClassNameImpl(class_names);
}

void GRM::Element::before(std::shared_ptr<GRM::Element> node)
{
  if (!parentNode()) throw HierarchyRequestError("element is root node");
  parentNode()->insertBefore(node, shared_from_this());
}

void GRM::Element::after(std::shared_ptr<GRM::Element> node)
{
  if (!parentNode()) throw HierarchyRequestError("element is root node");
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
  if (!parentNode()) throw HierarchyRequestError("element is root node");
  parentNode()->replaceChild(node, shared_from_this());
}

void GRM::Element::remove()
{
  void (*context_update)(const std::shared_ptr<GRM::Element> &, const std::string &, const GRM::Value &) = nullptr;
  void (*context_delete)(const std::shared_ptr<GRM::Element> &) = nullptr;
  ownerDocument()->getContextFct(&context_delete, &context_update);

  if (!parentNode()) throw HierarchyRequestError("element is root node");

  auto elem_p = std::static_pointer_cast<Element>(shared_from_this());
  context_delete(elem_p);

  parentNode()->removeChild(shared_from_this());
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Element::children()
{
  return childrenImpl();
}

std::vector<std::shared_ptr<const GRM::Element>> GRM::Element::children() const
{
  return childrenImpl();
}

std::shared_ptr<GRM::Element> GRM::Element::firstChildElement()
{
  return firstChildElementImpl();
}

std::shared_ptr<const GRM::Element> GRM::Element::firstChildElement() const
{
  return firstChildElementImpl();
}

std::shared_ptr<GRM::Element> GRM::Element::lastChildElement()
{
  return lastChildElementImpl();
}

std::shared_ptr<const GRM::Element> GRM::Element::lastChildElement() const
{
  return lastChildElementImpl();
}

unsigned long GRM::Element::childElementCount() const
{
  return childElementCountImpl();
}

void GRM::Element::prepend(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  prependImpl(nodes);
}

void GRM::Element::append(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  appendImpl(nodes);
}

void GRM::Element::replaceChildren(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  replaceChildrenImpl(nodes);
}

template <typename T> static std::shared_ptr<T> getElementByIdImpl(std::shared_ptr<T> element, const std::string &id)
{
  if (element->id() == id) return element;
  for (const auto &child_element : element->children())
    {
      auto child_found_element = getElementByIdImpl(child_element, id);
      if (child_found_element) return child_found_element;
    }
  return nullptr;
}

std::shared_ptr<GRM::Element> GRM::Element::getElementById(const std::string &id)
{
  return getElementByIdImpl(shared(), id);
}
std::shared_ptr<const GRM::Element> GRM::Element::getElementById(const std::string &id) const
{
  return getElementByIdImpl(shared(), id);
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

bool GRM::Element::isEqualNode(const std::shared_ptr<const GRM::Node> &other_node) const
{
  auto other_node_as_element = std::dynamic_pointer_cast<const GRM::Element>(other_node);
  if (!other_node_as_element) return false;
  if (other_node_as_element->localName() != localName()) return false;
  if (other_node_as_element->m_attributes.size() != m_attributes.size()) return false;
  for (const auto &attribute_entry : other_node_as_element->m_attributes)
    {
      if (m_attributes.find(attribute_entry.first) == m_attributes.end()) return false;
      if (m_attributes.at(attribute_entry.first) != attribute_entry.second) return false;
    }
  return GRM::Node::childrenAreEqualRecursive(shared_from_this(), other_node);
}

std::shared_ptr<GRM::Element> GRM::Element::previousElementSibling()
{
  return previousElementSiblingImpl();
}

std::shared_ptr<const GRM::Element> GRM::Element::previousElementSibling() const
{
  return previousElementSiblingImpl();
}

std::shared_ptr<GRM::Element> GRM::Element::nextElementSibling()
{
  return nextElementSiblingImpl();
}

std::shared_ptr<const GRM::Element> GRM::Element::nextElementSibling() const
{
  return nextElementSiblingImpl();
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Element::querySelectorsAll(const std::string &selectors)
{
  std::vector<std::shared_ptr<GRM::Element>> found_elements;
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  querySelectorsAllImpl(parseSelectors(selectors), found_elements, match_map);
  return found_elements;
}

std::vector<std::shared_ptr<const GRM::Element>> GRM::Element::querySelectorsAll(const std::string &selectors) const
{
  std::vector<std::shared_ptr<const GRM::Element>> found_elements;
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  querySelectorsAllImpl(parseSelectors(selectors), found_elements, match_map);
  return found_elements;
}

std::shared_ptr<GRM::Element> GRM::Element::querySelectors(const std::string &selectors)
{
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  return querySelectorsImpl(parseSelectors(selectors), match_map);
}

std::shared_ptr<const GRM::Element> GRM::Element::querySelectors(const std::string &selectors) const
{
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  return querySelectorsImpl(parseSelectors(selectors), match_map);
}
