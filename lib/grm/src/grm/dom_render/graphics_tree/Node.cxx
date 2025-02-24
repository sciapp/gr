#include <grm/dom_render/graphics_tree/Node.hxx>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/NotFoundError.hxx>
#include <grm/dom_render/graphics_tree/HierarchyRequestError.hxx>
#include <grm/dom_render/graphics_tree/TypeError.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <algorithm>
#include <map>
#include <tuple>

GRM::Node::Node(Type type, const std::shared_ptr<GRM::Document> &owner_document)
    : m_type(type), m_owner_document(owner_document)
{
}

GRM::Node::Type GRM::Node::nodeType() const
{
  return m_type;
}

bool GRM::Node::isConnected() const
{
  return getRootNode()->nodeType() == GRM::Node::Type::DOCUMENT_NODE;
}

std::shared_ptr<GRM::Document> GRM::Node::ownerDocument()
{
  return std::const_pointer_cast<GRM::Document>(const_cast<const GRM::Node *>(this)->ownerDocument());
}

std::shared_ptr<const GRM::Document> GRM::Node::ownerDocument() const
{
  if (nodeType() == Type::DOCUMENT_NODE) return nullptr;
  return m_owner_document.lock();
}

std::shared_ptr<GRM::Document> GRM::Node::nodeDocument()
{
  if (m_type == Type::DOCUMENT_NODE) return std::dynamic_pointer_cast<GRM::Document>(shared_from_this());
  return ownerDocument();
}

std::shared_ptr<const GRM::Document> GRM::Node::nodeDocument() const
{
  if (m_type == Type::DOCUMENT_NODE) return std::dynamic_pointer_cast<const GRM::Document>(shared_from_this());
  return ownerDocument();
}

template <typename T> std::shared_ptr<T> getRootNodeImpl(std::shared_ptr<T> node)
{
  auto parent_node = node->parentNode();
  if (!parent_node) return node;
  return getRootNodeImpl(parent_node);
}

std::shared_ptr<GRM::Node> GRM::Node::getRootNode()
{
  return getRootNodeImpl(shared_from_this());
}

std::shared_ptr<const GRM::Node> GRM::Node::getRootNode() const
{
  return getRootNodeImpl(shared_from_this());
}

std::shared_ptr<GRM::Node> GRM::Node::parentNode()
{
  return m_parent_node.lock();
}

std::shared_ptr<const GRM::Node> GRM::Node::parentNode() const
{
  return m_parent_node.lock();
}

std::shared_ptr<GRM::Element> GRM::Node::parentElement()
{
  return std::dynamic_pointer_cast<GRM::Element>(parentNode());
}

std::shared_ptr<const GRM::Element> GRM::Node::parentElement() const
{
  return std::dynamic_pointer_cast<const GRM::Element>(parentNode());
}

bool GRM::Node::hasChildNodes() const
{
  return !m_child_nodes.empty();
}

std::vector<std::shared_ptr<GRM::Node>> GRM::Node::childNodes()
{
  return {m_child_nodes.cbegin(), m_child_nodes.cend()};
}

std::vector<std::shared_ptr<const GRM::Node>> GRM::Node::childNodes() const
{
  return {m_child_nodes.cbegin(), m_child_nodes.cend()};
}

std::shared_ptr<GRM::Node> GRM::Node::firstChild()
{
  return std::const_pointer_cast<GRM::Node>(const_cast<const GRM::Node *>(this)->firstChild());
}

std::shared_ptr<const GRM::Node> GRM::Node::firstChild() const
{
  if (m_child_nodes.empty()) return nullptr;
  return m_child_nodes.front();
}


std::shared_ptr<GRM::Node> GRM::Node::lastChild()
{
  return std::const_pointer_cast<GRM::Node>(const_cast<const GRM::Node *>(this)->lastChild());
}

std::shared_ptr<const GRM::Node> GRM::Node::lastChild() const
{
  if (m_child_nodes.empty()) return nullptr;
  return m_child_nodes.back();
}

std::shared_ptr<GRM::Node> GRM::Node::previousSibling()
{
  return std::const_pointer_cast<GRM::Node>(const_cast<const GRM::Node *>(this)->previousSibling());
}

std::shared_ptr<const GRM::Node> GRM::Node::previousSibling() const
{
  auto parent_node = parentNode();
  if (!parent_node) return nullptr;
  if (parent_node->m_child_nodes.front().get() == this) return nullptr;
  auto it = std::find(parent_node->m_child_nodes.begin(), parent_node->m_child_nodes.end(), shared_from_this());
  --it;
  return *it;
}

std::shared_ptr<GRM::Node> GRM::Node::nextSibling()
{
  return std::const_pointer_cast<GRM::Node>(const_cast<const GRM::Node *>(this)->nextSibling());
}

std::shared_ptr<const GRM::Node> GRM::Node::nextSibling() const
{
  auto parent_node = parentNode();
  if (!parent_node) return nullptr;
  if (parent_node->m_child_nodes.back().get() == this) return nullptr;
  auto it = std::find(parent_node->m_child_nodes.begin(), parent_node->m_child_nodes.end(), shared_from_this());
  ++it;
  return *it;
}

std::shared_ptr<GRM::Node> GRM::Node::cloneNode()
{
  // hardcoded in overloaded function as virtual functions may not have default arguments
  return cloneNode(false);
}

std::shared_ptr<GRM::Node> GRM::Node::cloneNode(bool deep)
{
  auto clone = cloneIndividualNode();
  clone->m_parent_node = {};
  if (deep)
    {
      clone->m_child_nodes.clear();
      for (auto const &child_node : m_child_nodes)
        {
          clone->appendChild(child_node->cloneNode(deep));
        }
    }
  else
    {
      clone->m_child_nodes.clear();
    }
  return clone;
}

bool GRM::Node::isEqualNode(const std::shared_ptr<const GRM::Node> &other_node) const
{
  return isSameNode(other_node);
}

bool GRM::Node::isSameNode(const std::shared_ptr<const GRM::Node> &other_node) const
{
  return (other_node == shared_from_this());
}

bool GRM::Node::contains(const std::shared_ptr<const GRM::Node> &other_node) const
{
  if (!other_node) return false;
  if (other_node->parentNode().get() == this) return true;
  return contains(other_node->parentNode());
}

std::shared_ptr<GRM::Node> GRM::Node::insertBefore(std::shared_ptr<GRM::Node> node,
                                                   const std::shared_ptr<GRM::Node> &child)
{
  if (nodeType() != Type::DOCUMENT_NODE && nodeType() != Type::ELEMENT_NODE)
    throw HierarchyRequestError("parent must be Document or Element node");
  if (!node) throw TypeError("node is null");
  if (node->nodeType() != Type::ELEMENT_NODE && node->nodeType() != Type::COMMENT_NODE)
    throw HierarchyRequestError("node must be Element or Comment node");
  if (node.get() == this || node->contains(shared_from_this()))
    throw HierarchyRequestError("node must not be an inclusive ancestor of parent");
  if (child && child->parentNode().get() != this) throw NotFoundError("child is not a child of parent");
  if (nodeType() == Type::DOCUMENT_NODE)
    {
      auto this_as_document = dynamic_cast<GRM::Document *>(this);
      if (node->nodeType() == Type::ELEMENT_NODE && this_as_document->childElementCount() != 0)
        throw HierarchyRequestError("parent already has an element node");
    }
  nodeDocument()->adoptNode(node);
  if (child)
    {
      auto it = std::find(m_child_nodes.begin(), m_child_nodes.end(), child);
      m_child_nodes.insert(it, node);
    }
  else
    {
      m_child_nodes.push_back(node);
    }
  node->m_parent_node = shared_from_this();
  return node;
}

std::shared_ptr<GRM::Node> GRM::Node::appendChild(std::shared_ptr<GRM::Node> node)
{
  insertBefore(node, nullptr);
  return node;
}

std::shared_ptr<GRM::Node> GRM::Node::replaceChild(std::shared_ptr<GRM::Node> node,
                                                   const std::shared_ptr<GRM::Node> &child)
{
  if (nodeType() != Type::DOCUMENT_NODE && nodeType() != Type::ELEMENT_NODE)
    throw HierarchyRequestError("parent must be Document or Element node");
  if (!node) throw TypeError("node is null");
  if (node->nodeType() != Type::ELEMENT_NODE && node->nodeType() != Type::COMMENT_NODE)
    throw HierarchyRequestError("node must be Element or Comment node");
  if (node->contains(shared_from_this()))
    throw HierarchyRequestError("node must not be an inclusive ancestor of parent");
  if (!child) throw TypeError("child is null");
  if (child->parentNode().get() != this) throw NotFoundError("child is not a child of parent");
  if (nodeType() == Type::DOCUMENT_NODE)
    {
      auto this_as_document = dynamic_cast<GRM::Document *>(this);
      if (node->nodeType() == Type::ELEMENT_NODE && this_as_document->firstChildElement() != child)
        throw HierarchyRequestError("parent already has an element node");
    }
  nodeDocument()->adoptNode(node);
  auto it = std::find(m_child_nodes.begin(), m_child_nodes.end(), child);
  *it = node;
  node->m_parent_node = shared_from_this();
  child->m_parent_node = {};
  return node;
}

std::shared_ptr<GRM::Node> GRM::Node::removeChild(std::shared_ptr<GRM::Node> child)
{
  if (!child) throw TypeError("child is null");
  if (child->parentNode().get() != this) throw NotFoundError("child is not a child of this node");
  m_child_nodes.remove(child);
  child->m_parent_node = {};
  return child;
}

void GRM::Node::prependImpl(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  auto previous_first_child = firstChild();
  for (const auto &node : nodes)
    {
      if (previous_first_child)
        {
          insertBefore(node, previous_first_child);
        }
      else
        {
          appendChild(node);
        }
    }
}

void GRM::Node::appendImpl(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  for (const auto &node : nodes)
    {
      appendChild(node);
    }
}

void GRM::Node::replaceChildrenImpl(const std::vector<std::shared_ptr<GRM::Node>> &nodes)
{
  for (const auto &node : m_child_nodes)
    {
      node->m_parent_node = {};
    }
  m_child_nodes.clear();
  appendImpl(nodes);
}

void GRM::Node::setOwnerDocumentRecursive(const std::shared_ptr<GRM::Node> &node,
                                          const std::shared_ptr<GRM::Document> &document)
{
  node->m_owner_document = document;
  for (const auto &child_node : node->m_child_nodes)
    {
      setOwnerDocumentRecursive(child_node, document);
    }
}

bool GRM::Node::childrenAreEqualRecursive(const std::shared_ptr<const GRM::Node> &left_node,
                                          const std::shared_ptr<const GRM::Node> &right_node)
{
  if (left_node == right_node) return true;
  if (!left_node || !right_node) return false;
  if (left_node->m_child_nodes.size() != right_node->m_child_nodes.size()) return false;
  for (auto left_it = left_node->m_child_nodes.cbegin(), right_it = right_node->m_child_nodes.cbegin();
       left_it != left_node->m_child_nodes.cend() && right_it != right_node->m_child_nodes.cend();
       ++left_it, ++right_it)
    {
      if (!(*left_it)->isEqualNode(*right_it)) return false;
    }
  return true;
}


template <typename T, typename U> static std::vector<std::shared_ptr<T>> childrenImpl(U &child_nodes)
{
  std::vector<std::shared_ptr<T>> child_elements;
  for (const auto &child_node : child_nodes)
    {
      if (child_node->nodeType() == GRM::Node::Type::ELEMENT_NODE)
        child_elements.push_back(std::dynamic_pointer_cast<T>(child_node));
    }
  return child_elements;
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Node::childrenImpl()
{
  return ::childrenImpl<GRM::Element>(m_child_nodes);
}

std::vector<std::shared_ptr<const GRM::Element>> GRM::Node::childrenImpl() const
{
  return ::childrenImpl<const GRM::Element>(m_child_nodes);
}

template <typename T, typename U> static std::shared_ptr<T> firstChildElementImpl(U &child_nodes)
{
  for (const auto &child_node : child_nodes)
    {
      if (child_node->nodeType() == GRM::Node::Type::ELEMENT_NODE) return std::dynamic_pointer_cast<T>(child_node);
    }
  return nullptr;
}

std::shared_ptr<GRM::Element> GRM::Node::firstChildElementImpl()
{
  return ::firstChildElementImpl<GRM::Element>(m_child_nodes);
}

std::shared_ptr<const GRM::Element> GRM::Node::firstChildElementImpl() const
{
  return ::firstChildElementImpl<const GRM::Element>(m_child_nodes);
}

template <typename T, typename U> static std::shared_ptr<T> lastChildElementImpl(U &child_nodes)
{
  for (auto node_it = child_nodes.rbegin(); node_it != child_nodes.rend(); ++node_it)
    {
      const auto &child_node = *node_it;
      if (child_node->nodeType() == GRM::Node::Type::ELEMENT_NODE) return std::dynamic_pointer_cast<T>(child_node);
    }
  return nullptr;
}

std::shared_ptr<GRM::Element> GRM::Node::lastChildElementImpl()
{
  return ::lastChildElementImpl<GRM::Element>(m_child_nodes);
}

std::shared_ptr<const GRM::Element> GRM::Node::lastChildElementImpl() const
{
  return ::lastChildElementImpl<const GRM::Element>(m_child_nodes);
}

unsigned long GRM::Node::childElementCountImpl() const
{
  unsigned long count = 0;
  for (const auto &node : m_child_nodes)
    {
      if (node->nodeType() == GRM::Node::Type::ELEMENT_NODE) count++;
    }
  return count;
}

std::shared_ptr<GRM::Element> GRM::Node::previousElementSiblingImpl()
{
  auto sibling = previousSibling();
  while (sibling && sibling->nodeType() != GRM::Node::Type::ELEMENT_NODE)
    {
      sibling = sibling->previousSibling();
    }
  return std::dynamic_pointer_cast<Element>(sibling);
}

std::shared_ptr<const GRM::Element> GRM::Node::previousElementSiblingImpl() const
{
  auto sibling = previousSibling();
  while (sibling && sibling->nodeType() != GRM::Node::Type::ELEMENT_NODE)
    {
      sibling = sibling->previousSibling();
    }
  return std::dynamic_pointer_cast<const Element>(sibling);
}

std::shared_ptr<GRM::Element> GRM::Node::nextElementSiblingImpl()
{
  auto sibling = nextSibling();
  while (sibling && sibling->nodeType() != GRM::Node::Type::ELEMENT_NODE)
    {
      sibling = sibling->nextSibling();
    }
  return std::dynamic_pointer_cast<Element>(sibling);
}

std::shared_ptr<const GRM::Element> GRM::Node::nextElementSiblingImpl() const
{
  auto sibling = nextSibling();
  while (sibling && sibling->nodeType() != GRM::Node::Type::ELEMENT_NODE)
    {
      sibling = sibling->nextSibling();
    }
  return std::dynamic_pointer_cast<const Element>(sibling);
}

template <typename T, typename U>
static std::vector<std::shared_ptr<T>> getElementsByClassNameImpl(U &node, const std::string &class_names)
{
  if (class_names.empty()) return {};
  auto class_names_vec = GRM::split(GRM::toLower(class_names), " ");
  for (auto &class_name : class_names_vec)
    {
      class_name = GRM::toLower(GRM::strip(class_name));
    }
  for (auto class_name_it = class_names_vec.begin(); class_name_it != class_names_vec.end();)
    {
      if ((*class_name_it).empty())
        {
          class_name_it = class_names_vec.erase(class_name_it);
        }
      else
        {
          ++class_name_it;
        }
    }
  if (class_names_vec.empty()) return {};
  std::vector<std::shared_ptr<T>> found_elements;
  for (const auto &child_node : node.childNodes())
    {
      if (child_node->nodeType() != GRM::Node::Type::ELEMENT_NODE) continue;
      const auto &child_element = std::dynamic_pointer_cast<T>(child_node);
      if (child_element)
        {
          auto child_class_names_value = child_element->getAttribute("class");
          if (child_class_names_value.isString())
            {
              auto child_class_names = GRM::split((std::string)child_class_names_value, " ");
              if (!child_class_names.empty())
                {
                  for (auto &child_class_name : child_class_names)
                    {
                      child_class_name = GRM::toLower(GRM::strip(child_class_name));
                    }

                  bool missing_a_class = false;
                  for (const auto &class_name : class_names_vec)
                    {
                      if (std::find(child_class_names.begin(), child_class_names.end(), class_name) ==
                          child_class_names.end())
                        {
                          missing_a_class = true;
                          break;
                        }
                    }
                  if (!missing_a_class) found_elements.push_back(child_element);
                }
            }
          auto child_found_elements = child_element->getElementsByClassName(class_names);
          found_elements.insert(found_elements.end(), child_found_elements.begin(), child_found_elements.end());
        }
    }
  return found_elements;
}

std::vector<std::shared_ptr<GRM::Element>> GRM::Node::getElementsByClassNameImpl(const std::string &class_names)
{
  return ::getElementsByClassNameImpl<GRM::Element>(*this, class_names);
}

std::vector<std::shared_ptr<const GRM::Element>>
GRM::Node::getElementsByClassNameImpl(const std::string &class_names) const
{
  return ::getElementsByClassNameImpl<const GRM::Element>(*this, class_names);
}

bool GRM::Node::matchSelector(const std::shared_ptr<GRM::Selector> &selector,
                              std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const
{
  auto element = dynamic_cast<const GRM::Element *>(this);
  if (nodeType() != GRM::Node::Type::ELEMENT_NODE || !element) return false;
  return selector->matchElement(*element, match_map);
}

void GRM::Node::querySelectorsAllImpl(
    const std::shared_ptr<GRM::Selector> &selector, std::vector<std::shared_ptr<GRM::Element>> &found_elements,
    std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map)
{
  if (matchSelector(selector, match_map))
    found_elements.push_back(std::dynamic_pointer_cast<Element>(shared_from_this()));
  for (auto &child_node : m_child_nodes)
    {
      child_node->querySelectorsAllImpl(selector, found_elements, match_map);
    }
}

void GRM::Node::querySelectorsAllImpl(
    const std::shared_ptr<GRM::Selector> &selector, std::vector<std::shared_ptr<const GRM::Element>> &found_elements,
    std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const
{
  if (matchSelector(selector, match_map))
    found_elements.push_back(std::dynamic_pointer_cast<const Element>(shared_from_this()));
  for (auto &child_node : m_child_nodes)
    {
      child_node->querySelectorsAllImpl(selector, found_elements, match_map);
    }
}

std::shared_ptr<GRM::Element>
GRM::Node::querySelectorsImpl(const std::shared_ptr<GRM::Selector> &selector,
                              std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map)
{
  if (matchSelector(selector, match_map)) return std::dynamic_pointer_cast<Element>(shared_from_this());
  for (auto &child_node : m_child_nodes)
    {
      auto result = child_node->querySelectorsImpl(selector, match_map);
      if (result)
        {
          return result;
        }
    }
  return nullptr;
}

std::shared_ptr<const GRM::Element>
GRM::Node::querySelectorsImpl(const std::shared_ptr<GRM::Selector> &selector,
                              std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const
{
  if (matchSelector(selector, match_map)) return std::dynamic_pointer_cast<const Element>(shared_from_this());
  for (auto &child_node : m_child_nodes)
    {
      auto result = child_node->querySelectorsImpl(selector, match_map);
      if (result) return result;
    }
  return nullptr;
}
