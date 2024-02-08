#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/Comment.hxx>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/TypeError.hxx>
#include <sstream>
#include <algorithm>
#include <cmath>
#include "grm/utilcpp_int.hxx"

static void nodeToXML(std::stringstream &os, const std::shared_ptr<const GRM::Node> &node,
                      const GRM::SerializerOptions &options, const std::string &indent,
                      std::optional<std::function<bool(const std::string &attribute_name, const GRM::Element &,
                                                       std::optional<std::string> &new_attribute_name)>>
                          attribute_filter);

static void documentToXML(std::stringstream &os, const std::shared_ptr<const GRM::Document> &document,
                          const GRM::SerializerOptions &options, const std::string &indent)
{
  os << indent << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  for (const auto &child_node : document->childNodes())
    {
      nodeToXML(os, child_node, options, indent, std::nullopt);
    }
}

static void
elementToXML(std::stringstream &os, const std::shared_ptr<const GRM::Element> &element,
             const GRM::SerializerOptions &options, const std::string &indent,
             std::optional<std::function<bool(const std::string &attribute_name, const GRM::Element &element,
                                              std::optional<std::string> &new_attribute_name)>>
                 attribute_filter)
{
  os << indent << "<" << element->localName();
  auto attribute_names_set = element->getAttributeNames();
  std::vector<std::string> attribute_names{attribute_names_set.begin(), attribute_names_set.end()};
  std::sort(attribute_names.begin(), attribute_names.end());
  if (element->hasAttribute("name"))
    {
      os << " "
         << "name"
         << "=\"" << (std::string)element->getAttribute("name") << "\"";
    }
  for (const auto &original_attribute_name : attribute_names)
    {
      auto attribute_name = std::ref(original_attribute_name);
      std::optional<std::string> new_attribute_name;
      if (original_attribute_name == "name" ||
          (attribute_filter && !(*attribute_filter)(original_attribute_name, *element, new_attribute_name)))
        continue;
      if (new_attribute_name)
        {
          attribute_name = *new_attribute_name;
        }
      if (!options.show_hidden && starts_with(attribute_name.get(), "_")) continue;

      auto value = (std::string)element->getAttribute(original_attribute_name);
      if (value == "nan")
        {
          os << " " << attribute_name.get() << "=\""
             << "NaN"
             << "\"";
        }
      else if (value == "inf")
        {
          os << " " << attribute_name.get() << "=\""
             << "INF"
             << "\"";
        }
      else if (value == "-inf")
        {
          os << " " << attribute_name.get() << "=\""
             << "-INF"
             << "\"";
        }
      else
        {
          os << " " << attribute_name.get() << "=\"" << value << "\"";
        }
    }
  if (element->hasChildNodes())
    {
      os << ">\n";
      for (const auto &child_node : element->childNodes())
        {
          nodeToXML(os, child_node, options, indent + options.indent, attribute_filter);
        }
      os << indent << "</" << element->localName() << ">\n";
    }
  else
    {
      os << " />\n";
    }
}

static void commentToXML(std::stringstream &os, const std::shared_ptr<const GRM::Comment> &comment,
                         const GRM::SerializerOptions &options, const std::string &indent)
{
  os << indent << "<!--" << comment->data() << "-->\n";
}

static void nodeToXML(std::stringstream &os, const std::shared_ptr<const GRM::Node> &node,
                      const GRM::SerializerOptions &options, const std::string &indent,
                      std::optional<std::function<bool(const std::string &attribute_name, const GRM::Element &,
                                                       std::optional<std::string> &new_attribute_name)>>
                          attribute_filter)
{
  switch (node->nodeType())
    {
    case GRM::Node::Type::DOCUMENT_NODE:
      {
        auto document = std::dynamic_pointer_cast<const GRM::Document>(node);
        documentToXML(os, document, options, indent);
        break;
      }
    case GRM::Node::Type::ELEMENT_NODE:
      {
        auto element = std::dynamic_pointer_cast<const GRM::Element>(node);
        elementToXML(os, element, options, indent, attribute_filter);
        break;
      }
    case GRM::Node::Type::COMMENT_NODE:
      {
        auto comment = std::dynamic_pointer_cast<const GRM::Comment>(node);
        commentToXML(os, comment, options, indent);
        break;
      }
    }
};

std::string GRM::toXML(const std::shared_ptr<const GRM::Node> &node, const GRM::SerializerOptions &options,
                       std::optional<std::function<bool(const std::string &attribute_name, const GRM::Element &element,
                                                        std::optional<std::string> &new_attribute_name)>>
                           attribute_filter)
{
  if (!node)
    {
      throw TypeError("node is null");
    }
  std::stringstream os;
  nodeToXML(os, node, options, "", attribute_filter);
  return os.str();
}

std::string GRM::tolower(std::string string)
{
  for (char &c : string)
    {
      c = static_cast<char>(std::tolower(c));
    }
  return string;
}

std::string GRM::toupper(std::string string)
{
  for (char &c : string)
    {
      c = static_cast<char>(std::toupper(c));
    }
  return string;
}

std::vector<std::string> GRM::split(const std::string &string, const std::string &token)
{
  std::vector<std::string> results;
  if (string.empty())
    {
      return {};
    }
  if (token.empty())
    {
      for (const auto &character : string)
        {
          if (character)
            {
              results.push_back({character});
            }
        }
      return results;
    }
  std::string::size_type search_start_position = 0;
  do
    {
      auto token_position = string.find(token, search_start_position);
      results.push_back(string.substr(search_start_position, token_position - search_start_position));
      if (token_position == std::string::npos)
        {
          break;
        }
      search_start_position = token_position + token.size();
    }
  while (search_start_position <= string.size());
  return results;
}

std::string GRM::strip(const std::string &string)
{
  if (string.empty())
    {
      return string;
    }
  std::string::size_type left_start;
  std::string::size_type right_start;
  for (left_start = 0; left_start < string.size() && std::isspace(string[left_start]); ++left_start)
    ;
  for (right_start = string.size() - 1; right_start >= left_start && std::isspace(string[right_start]); --right_start)
    ;
  return string.substr(left_start, right_start - left_start + 1);
}

bool GRM::Selector::matchElement(
    const GRM::Element &element,
    std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const
{
  auto element_ptr = std::dynamic_pointer_cast<const Element>(element.shared_from_this()).get();
  if (match_map.find(std::tuple<const GRM::Element *, const GRM::Selector *>{element_ptr, this}) != match_map.end())
    {
      return match_map[std::tuple<const GRM::Element *, const GRM::Selector *>{element_ptr, this}];
    }
  bool result = doMatchElement(element, match_map);
  match_map[std::tuple<const GRM::Element *, const GRM::Selector *>{element_ptr, this}] = result;
  return result;
}

/*!
 * Normalize a given vector of doubles so all values sum up to 1.0
 *
 * \param[in] x A vector that should be normalized
 */
void GRM::normalize_vec(std::vector<double> x, std::vector<double> *normalized_x)
{
  double sum;
  int n = size(x);
  unsigned int i;

  sum = 0.0;
  for (i = 0; i < n; ++i)
    {
      sum += x[i];
    }

  for (i = 0; i < n; ++i)
    {
      normalized_x[0][i] = x[i] / sum;
    }
}

/*!
 * Normalize a vector array of doubles so all values sum up to `sum`.
 * All values are converted to unsigned integers. It is guaranteed that
 * the sum of all values is always `sum` (rounding errors are handled).
 */
void GRM::normalize_vec_int(std::vector<double> x, std::vector<unsigned int> *normalized_x, unsigned int sum)
{
  double sum_x;
  unsigned int actual_sum;
  int rounding_error;
  double normalized_x_without_rounding;
  double current_relative_error;
  double min_relative_error;
  unsigned int min_relative_error_index;
  unsigned int i;
  int n = size(x);

  sum_x = 0.0;
  for (i = 0; i < n; ++i)
    {
      sum_x += x[i];
    }

  for (i = 0; i < n; ++i)
    {
      normalized_x[0][i] = (int)((x[i] * sum / sum_x) + 0.5);
    }

  actual_sum = 0;
  for (i = 0; i < n; ++i)
    {
      actual_sum += normalized_x[0][i];
    }
  rounding_error = sum - actual_sum;

  if (rounding_error != 0)
    {
      /*
       * Find the data value which gets the lowest relative error
       * when the rounding error is added.
       */
      min_relative_error = INFINITY;
      min_relative_error_index = 0;
      for (i = 0; i < n; ++i)
        {
          normalized_x_without_rounding = x[i] * sum / sum_x;
          current_relative_error =
              fabs(normalized_x[0][i] + rounding_error - normalized_x_without_rounding) / normalized_x_without_rounding;
          if (current_relative_error < min_relative_error)
            {
              min_relative_error = current_relative_error;
              min_relative_error_index = i;
            }
        }
      /* Apply the rounding error to the previously found data value */
      normalized_x[0][min_relative_error_index] += rounding_error;
    }
}

namespace GRM
{
class AndCombinedSelector : public Selector
{
public:
  explicit AndCombinedSelector(std::vector<std::shared_ptr<Selector>> part_selectors)
      : m_part_selectors(std::move(part_selectors))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    if (m_part_selectors.empty())
      {
        return false;
      }
    for (const auto &parsed_selector : m_part_selectors)
      {
        if (!parsed_selector->matchElement(element, match_map))
          {
            return false;
          }
      }
    return true;
  }

private:
  std::vector<std::shared_ptr<Selector>> m_part_selectors;
};
class OrCombinedSelector : public Selector
{
public:
  explicit OrCombinedSelector(std::vector<std::shared_ptr<Selector>> part_selectors)
      : m_part_selectors(std::move(part_selectors))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    if (m_part_selectors.empty())
      {
        return false;
      }
    for (const auto &parsed_selector : m_part_selectors)
      {
        if (parsed_selector->matchElement(element, match_map))
          {
            return true;
          }
      }
    return false;
  }

private:
  std::vector<std::shared_ptr<Selector>> m_part_selectors;
};
class AncestorAndLocalSelector : public Selector
{
public:
  explicit AncestorAndLocalSelector(std::shared_ptr<Selector> ancestor_selector,
                                    std::shared_ptr<Selector> local_selector)
      : m_ancestor_selector(std::move(ancestor_selector)), m_local_selector(std::move(local_selector))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    bool conditional_selector_matched = false;
    for (auto ancestor = element.parentElement(); ancestor; ancestor = ancestor->parentElement())
      {
        if (m_ancestor_selector->matchElement(*ancestor, match_map))
          {
            return m_local_selector->matchElement(element, match_map);
          }
      }
    return false;
  }

private:
  std::shared_ptr<Selector> m_ancestor_selector;
  std::shared_ptr<Selector> m_local_selector;
};
class ParentAndLocalSelector : public Selector
{
public:
  explicit ParentAndLocalSelector(std::shared_ptr<Selector> parent_selector, std::shared_ptr<Selector> local_selector)
      : m_parent_selector(std::move(parent_selector)), m_local_selector(std::move(local_selector))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    bool conditional_selector_matched = false;
    auto parent = element.parentElement();
    if (!parent)
      {
        return false;
      }
    return m_parent_selector->matchElement(*parent, match_map) && m_local_selector->matchElement(element, match_map);
  }

private:
  std::shared_ptr<Selector> m_parent_selector;
  std::shared_ptr<Selector> m_local_selector;
};
class PreviousSiblingAndLocalSelector : public Selector
{
public:
  explicit PreviousSiblingAndLocalSelector(std::shared_ptr<Selector> sibling_selector,
                                           std::shared_ptr<Selector> local_selector)
      : m_sibling_selector(std::move(sibling_selector)), m_local_selector(std::move(local_selector))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    bool conditional_selector_matched = false;
    auto parent = element.parentElement();
    if (!parent)
      {
        return false;
      }
    for (auto sibling = element.previousElementSibling(); sibling; sibling = element.previousElementSibling())
      {
        if (m_sibling_selector->matchElement(*sibling, match_map))
          {
            return m_local_selector->matchElement(element, match_map);
          }
      }
    return false;
  }

private:
  std::shared_ptr<Selector> m_sibling_selector;
  std::shared_ptr<Selector> m_local_selector;
};
class FalseSelector : public Selector
{
protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    (void)element;
    return false;
  }
};
class RootSelector : public Selector
{
protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    return (!element.parentNode() || element.parentNode()->nodeType() == GRM::Node::Type::DOCUMENT_NODE);
  }
};
class EmptySelector : public Selector
{
protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    return (element.childElementCount() == 0);
  }
};
class OnlyChildSelector : public Selector
{
protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    return (element.previousElementSibling() == nullptr && element.nextElementSibling() == nullptr);
  }
};
class OnlyOfTypeSelector : public Selector
{
protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    auto parent_element = element.parentElement();
    std::vector<std::shared_ptr<const Element>> elements;
    if (parent_element)
      {
        elements = parent_element->children();
      }
    else if (element.parentNode() && element.parentNode() == element.ownerDocument())
      {
        elements = element.ownerDocument()->children();
      }
    else
      {
        return {};
      }
    std::string local_name = element.localName();
    for (const auto &other_element : elements)
      {
        if (other_element != element.shared_from_this() && other_element->localName() == local_name)
          {
            return false;
          }
      }
    return true;
  }
};
class NthOfSelector : public Selector
{
public:
  explicit NthOfSelector(long offset, long factor, bool reverse)
      : m_offset(offset), m_factor(factor), m_reverse(reverse)
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    auto elements = this->elements(element);
    if (elements.empty())
      {
        return false;
      }
    auto element_it = std::find(elements.begin(), elements.end(), element.shared_from_this());
    if (element_it == elements.end())
      {
        return false;
      }
    long element_index = static_cast<long>(element_it - elements.begin() + 1);
    if (m_reverse)
      {
        element_index = static_cast<long>(elements.size()) + 1 - element_index;
      }
    if (m_factor > 0)
      {
        return element_index % m_factor == m_offset;
      }
    else if (m_factor < 0)
      {
        return element_index <= m_offset;
      }
    else
      {
        return element_index == m_offset;
      }
  }

  virtual std::vector<std::shared_ptr<const Element>> elements(const GRM::Element &element) const = 0;

private:
  long m_offset;
  long m_factor;
  bool m_reverse;
};
class NthChildSelector : public NthOfSelector
{
public:
  explicit NthChildSelector(long offset, long factor, bool reverse) : NthOfSelector(offset, factor, reverse) {}

protected:
  std::vector<std::shared_ptr<const Element>> elements(const GRM::Element &element) const override
  {
    auto parent_element = element.parentElement();
    if (parent_element)
      {
        return parent_element->children();
      }
    else if (element.parentNode() && element.parentNode() == element.ownerDocument())
      {
        return element.ownerDocument()->children();
      }
    else
      {
        return {};
      }
  }
};
class NthOfTypeSelector : public NthOfSelector
{
public:
  explicit NthOfTypeSelector(long offset, long factor, bool reverse) : NthOfSelector(offset, factor, reverse) {}

protected:
  std::vector<std::shared_ptr<const Element>> elements(const GRM::Element &element) const override
  {
    auto parent_element = element.parentElement();
    std::vector<std::shared_ptr<const Element>> elements;
    if (parent_element)
      {
        elements = parent_element->children();
      }
    else if (element.parentNode() && element.parentNode() == element.ownerDocument())
      {
        elements = element.ownerDocument()->children();
      }
    else
      {
        return {};
      }
    std::string local_name = element.localName();
    elements.erase(std::remove_if(elements.begin(), elements.end(),
                                  [&local_name](const std::shared_ptr<const Element> &other_element) {
                                    return other_element->localName() == local_name;
                                  }),
                   elements.end());
    return elements;
  }
};
class TagSelector : public Selector
{
public:
  explicit TagSelector(const std::string &tag_name) : m_local_name(GRM::tolower(tag_name)) {}

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    return !m_local_name.empty() && (m_local_name == "*" || m_local_name == element.localName());
  }

private:
  std::string m_local_name;
};
class HasAttributeSelector : public Selector
{
public:
  explicit HasAttributeSelector(const std::string &attribute_name) : m_attribute_name(GRM::tolower(attribute_name)) {}

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    return !m_attribute_name.empty() && element.hasAttribute(m_attribute_name);
  }

private:
  std::string m_attribute_name;
};
class AttributeEqualsSelector : public Selector
{
public:
  AttributeEqualsSelector(const std::string &attribute_name, std::string attribute_value)
      : m_attribute_name(GRM::tolower(attribute_name)), m_attribute_value(std::move(attribute_value))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    return !m_attribute_name.empty() && (std::string)element.getAttribute(m_attribute_name) == m_attribute_value;
  }

private:
  std::string m_attribute_name;
  std::string m_attribute_value;
};
class AttributeStartsWithSelector : public Selector
{
public:
  AttributeStartsWithSelector(const std::string &attribute_name, std::string attribute_value)
      : m_attribute_name(GRM::tolower(attribute_name)), m_attribute_value(std::move(attribute_value))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    return !m_attribute_name.empty() &&
           (((std::string)element.getAttribute(m_attribute_name)).find(m_attribute_value) == 0);
    ;
  }

private:
  std::string m_attribute_name;
  std::string m_attribute_value;
};
class IDSelector : public Selector
{
public:
  explicit IDSelector(const std::string &id) : m_id(GRM::tolower(id)) {}

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    return !m_id.empty() && m_id == (std::string)element.getAttribute("id");
  }

private:
  std::string m_id;
};
class AttributeEndsWithSelector : public Selector
{
public:
  AttributeEndsWithSelector(const std::string &attribute_name, std::string attribute_value)
      : m_attribute_name(GRM::tolower(attribute_name)), m_attribute_value(std::move(attribute_value))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    if (m_attribute_name.empty())
      {
        return false;
      }
    auto element_attribute_string = (std::string)element.getAttribute(m_attribute_name);
    auto attribute_value_position = element_attribute_string.find(m_attribute_value);
    return (attribute_value_position != std::string::npos) &&
           (attribute_value_position == element_attribute_string.size() - m_attribute_value.size());
  }

private:
  std::string m_attribute_name;
  std::string m_attribute_value;
};
class AttributeContainsSelector : public Selector
{
public:
  AttributeContainsSelector(const std::string &attribute_name, std::string attribute_value)
      : m_attribute_name(GRM::tolower(attribute_name)), m_attribute_value(std::move(attribute_value))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    if (m_attribute_name.empty())
      {
        return false;
      }
    auto element_attribute_string = (std::string)element.getAttribute(m_attribute_name);
    auto attribute_value_position = element_attribute_string.find(m_attribute_value);
    return (attribute_value_position != std::string::npos);
  }

private:
  std::string m_attribute_name;
  std::string m_attribute_value;
};
class AttributeContainsWordSelector : public Selector
{
public:
  AttributeContainsWordSelector(const std::string &attribute_name, std::string attribute_value)
      : m_attribute_name(GRM::tolower(attribute_name)), m_attribute_value(std::move(attribute_value))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    if (m_attribute_name.empty())
      {
        return false;
      }
    auto element_attribute_string = (std::string)element.getAttribute(m_attribute_name);
    auto element_attribute_words = GRM::split(element_attribute_string, " ");
    return (std::find(element_attribute_words.begin(), element_attribute_words.end(), m_attribute_value) !=
            element_attribute_words.end());
  }

private:
  std::string m_attribute_name;
  std::string m_attribute_value;
};
class AttributeContainsPrefixSelector : public Selector
{
public:
  AttributeContainsPrefixSelector(const std::string &attribute_name, std::string attribute_value)
      : m_attribute_name(GRM::tolower(attribute_name)), m_attribute_value(std::move(attribute_value))
  {
  }

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    if (m_attribute_name.empty())
      {
        return false;
      }
    auto element_attribute_string = (std::string)element.getAttribute(m_attribute_name);
    auto element_attribute_words = GRM::split(element_attribute_string, " ");
    for (const auto &word : element_attribute_words)
      {
        // match either a whole word or a hyphenated word starting with the selector value
        if (word == m_attribute_value || (word.find(m_attribute_value) == 0 && word.size() > m_attribute_value.size() &&
                                          word[m_attribute_value.size()] == '-'))
          {
            return true;
          }
      }
    return false;
  }

private:
  std::string m_attribute_name;
  std::string m_attribute_value;
};
class ClassSelector : public Selector
{
public:
  explicit ClassSelector(std::string class_name) : m_class_name(std::move(class_name)) {}

protected:
  bool doMatchElement(const GRM::Element &element,
                      std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> &match_map) const override
  {
    if (!m_class_name.empty())
      {
        auto element_classes_value = element.getAttribute("class");
        auto element_classes = GRM::split((std::string)element_classes_value, " ");
        for (auto &element_class : element_classes)
          {
            element_class = GRM::tolower(GRM::strip(element_class));
          }
        for (const auto &element_class : element_classes)
          {
            if (element_class == m_class_name)
              {
                return true;
              }
          }
      }
    return false;
  }

private:
  std::string m_class_name;
};
} // namespace GRM

std::shared_ptr<GRM::Selector> GRM::parseSelectors(const std::string &selectors)
{
  auto individual_selectors = GRM::split(selectors, ",");
  for (auto &selector : individual_selectors)
    {
      selector = GRM::tolower(GRM::strip(selector));
    }
  std::vector<std::shared_ptr<GRM::Selector>> parsed_individual_selectors;
  for (auto &selector : individual_selectors)
    {
      bool conditional_selector_found = false;
      bool in_quotes = false;
      bool in_bracketed_block = false;
      char quoting_character = '\0';
      for (auto it = selector.rbegin(); it != selector.rend() && !conditional_selector_found; ++it)
        {
          if (!in_bracketed_block && *it == ']')
            {
              in_bracketed_block = true;
            }
          else if (in_bracketed_block && !in_quotes && *it == '[')
            {
              in_bracketed_block = false;
            }
          else if (in_bracketed_block && !in_quotes && (*it == '"' || *it == '\''))
            {
              quoting_character = *it;
              in_quotes = true;
            }
          else if (in_bracketed_block && in_quotes && *it == quoting_character)
            {
              int num_leading_slashes = 0;
              for (auto it2 = it + 1; it2 != selector.rend() && *it2 == '\\'; ++it2)
                {
                  ++num_leading_slashes;
                }
              if (num_leading_slashes % 2 == 0)
                {
                  in_quotes = false;
                }
            }
          if (in_bracketed_block)
            {
              continue;
            }
          switch (*it)
            {
            case ' ':
              {
                char actual_character = ' ';
                for (auto it2 = it; it2 != selector.rend(); ++it2)
                  {
                    if (*it2 == ' ')
                      {
                        continue;
                      }
                    else
                      {
                        actual_character = *it2;
                        break;
                      }
                  }
                if (actual_character == '>' || actual_character == '~')
                  {
                    // this is just whitespace around another symbol
                    continue;
                  }
                auto ancestor_selector = parseSelectors(selector.substr(0, selector.rend() - it - 1));
                auto local_selector = parseSelectors(selector.substr(selector.rend() - it));
                parsed_individual_selectors.push_back(
                    std::make_shared<GRM::AncestorAndLocalSelector>(ancestor_selector, local_selector));
                conditional_selector_found = true;
                break;
              }
            case '>':
              {
                auto parent_selector = parseSelectors(selector.substr(0, selector.rend() - it - 1));
                auto local_selector = parseSelectors(selector.substr(selector.rend() - it));
                parsed_individual_selectors.push_back(
                    std::make_shared<GRM::ParentAndLocalSelector>(parent_selector, local_selector));
                conditional_selector_found = true;
                break;
              }
            case '~':
              {
                auto sibling_selector = parseSelectors(selector.substr(0, selector.rend() - it - 1));
                auto local_selector = parseSelectors(selector.substr(selector.rend() - it));
                parsed_individual_selectors.push_back(
                    std::make_shared<GRM::PreviousSiblingAndLocalSelector>(sibling_selector, local_selector));
                conditional_selector_found = true;
                break;
              }
            default:
              continue;
            }
        }
      if (conditional_selector_found)
        {
          continue;
        }
      std::string local_selector = selector;
      local_selector = GRM::strip(local_selector);
      if (local_selector.empty())
        {
          continue;
        }
      std::vector<std::shared_ptr<GRM::Selector>> parsed_selector_parts;
      in_bracketed_block = false;
      in_quotes = false;
      char previous_character = '\0';
      quoting_character = '\0';
      auto selector_part_begin = local_selector.begin();
      for (auto it = local_selector.begin(); true; ++it)
        {
          if (it == local_selector.end() || (!in_quotes && (*it == '.' || *it == '#' || *it == '[') || *it == ':'))
            {
              auto selector_part =
                  local_selector.substr(selector_part_begin - local_selector.begin(), it - selector_part_begin);
              selector_part_begin = it;
              if (!selector_part.empty())
                {
                  if (selector_part.front() == '.')
                    {
                      auto selector_class = selector_part.substr(1);
                      parsed_selector_parts.push_back(std::make_shared<GRM::ClassSelector>(selector_class));
                    }
                  else if (selector_part.front() == '#')
                    {
                      auto selector_id = selector_part.substr(1);
                      parsed_selector_parts.push_back(std::make_shared<GRM::IDSelector>(selector_id));
                    }
                  else if (selector_part.front() == '[')
                    {
                      auto equals_sign_pos = selector_part.find('=');
                      if (equals_sign_pos == std::string::npos)
                        {
                          auto selector_attribute = selector_part.substr(1, selector_part.size() - 2);
                          parsed_selector_parts.push_back(
                              std::make_shared<GRM::HasAttributeSelector>(selector_attribute));
                        }
                      else if (equals_sign_pos == 0)
                        {
                          parsed_selector_parts.push_back(std::make_shared<GRM::FalseSelector>());
                        }
                      else
                        {
                          auto selector_attribute = selector_part.substr(1, equals_sign_pos - 1);
                          auto selector_value =
                              selector_part.substr(equals_sign_pos + 1, selector_part.size() - equals_sign_pos - 2);
                          std::string selector_comparison = "=";
                          if (std::string{"|~^$*"}.find(selector_attribute.back()) != std::string::npos)
                            {
                              selector_comparison = {selector_attribute.back(), '='};
                              selector_attribute = selector_attribute.substr(0, selector_attribute.size() - 1);
                            }
                          if (selector_value.size() >= 2 &&
                              (selector_value.front() == '"' || selector_value.front() == '\''))
                            {
                              if (selector_value.back() == selector_value.front())
                                {
                                  int num_leading_slashes = 0;
                                  for (auto it2 = selector_value.rbegin() + 1;
                                       it2 != selector_value.rend() && *it2 == '\\'; ++it2)
                                    {
                                      ++num_leading_slashes;
                                    }
                                  if (num_leading_slashes % 2 == 0)
                                    {
                                      selector_value = selector_value.substr(1, selector_value.size() - 2);
                                    }
                                }
                            }
                          bool is_escaping = false;
                          auto escaped_selector_value = selector_value;
                          std::string::size_type i = 0;
                          for (const auto &c : escaped_selector_value)
                            {
                              if (!is_escaping && c == '\\')
                                {
                                  is_escaping = true;
                                }
                              else
                                {
                                  is_escaping = false;
                                  if (i < selector_value.size())
                                    {
                                      selector_value[i] = c;
                                      ++i;
                                    }
                                }
                            }
                          if (i < selector_value.size())
                            {
                              selector_value = selector_value.substr(0, i);
                            }
                          if (selector_comparison == "=")
                            {
                              parsed_selector_parts.push_back(
                                  std::make_shared<GRM::AttributeEqualsSelector>(selector_attribute, selector_value));
                            }
                          else if (selector_comparison == "~=")
                            {
                              parsed_selector_parts.push_back(std::make_shared<GRM::AttributeContainsWordSelector>(
                                  selector_attribute, selector_value));
                            }
                          else if (selector_comparison == "|=")
                            {
                              parsed_selector_parts.push_back(std::make_shared<GRM::AttributeContainsPrefixSelector>(
                                  selector_attribute, selector_value));
                            }
                          else if (selector_comparison == "^=")
                            {
                              parsed_selector_parts.push_back(std::make_shared<GRM::AttributeStartsWithSelector>(
                                  selector_attribute, selector_value));
                            }
                          else if (selector_comparison == "$=")
                            {
                              parsed_selector_parts.push_back(
                                  std::make_shared<GRM::AttributeEndsWithSelector>(selector_attribute, selector_value));
                            }
                          else if (selector_comparison == "*=")
                            {
                              parsed_selector_parts.push_back(
                                  std::make_shared<GRM::AttributeContainsSelector>(selector_attribute, selector_value));
                            }
                          else
                            {
                              // unsupported comparison types
                              parsed_selector_parts.push_back(std::make_shared<GRM::FalseSelector>());
                            }
                        }
                    }
                  else if (selector_part.front() == ':')
                    {
                      if (selector_part == ":nth-child(n)")
                        {
                          // matches every element, so no new selector
                        }
                      else if (selector_part == ":root")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::RootSelector>());
                        }
                      else if (selector_part == ":empty")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::EmptySelector>());
                        }
                      else if (selector_part == ":only-child")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::OnlyChildSelector>());
                        }
                      else if (selector_part == ":only-of-type")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::OnlyOfTypeSelector>());
                        }
                      else if (selector_part == ":first-child" || selector_part == ":nth-child(1)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthChildSelector>(1, 0, false));
                        }
                      else if (selector_part == ":nth-child(even)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthChildSelector>(0, 2, false));
                        }
                      else if (selector_part == ":nth-child(odd)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthChildSelector>(1, 2, false));
                        }
                      else if (selector_part == ":last-child" || selector_part == ":nth-last-child(1)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthChildSelector>(1, 0, true));
                        }
                      else if (selector_part == ":nth-last-child(even)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthChildSelector>(0, 2, true));
                        }
                      else if (selector_part == ":nth-last-child(odd)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthChildSelector>(1, 2, true));
                        }
                      else if (selector_part == ":first-of-type" || selector_part == ":nth-of-type(1)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthOfTypeSelector>(1, 0, false));
                        }
                      else if (selector_part == ":nth-of-type(even)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthOfTypeSelector>(0, 2, false));
                        }
                      else if (selector_part == ":nth-of-type(odd)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthOfTypeSelector>(1, 2, false));
                        }
                      else if (selector_part == ":last-of-type" || selector_part == ":nth-last-child(1)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthOfTypeSelector>(1, 0, true));
                        }
                      else if (selector_part == ":nth-last-of-type(even)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthOfTypeSelector>(0, 2, true));
                        }
                      else if (selector_part == ":nth-last-of-type(odd)")
                        {
                          parsed_individual_selectors.push_back(std::make_shared<GRM::NthOfTypeSelector>(1, 2, true));
                        }
                      else if ((selector_part.find(":nth-child(") == 0 || selector_part.find(":nth-last-child(") == 0 ||
                                selector_part.find(":nth-of-type(") == 0 ||
                                selector_part.find(":nth-last-of-type(") == 0) &&
                               selector_part.back() == ')')
                        {
                          auto parenthesis_position = selector_part.find('(');
                          auto selector_type_string = selector_part.substr(0, parenthesis_position);
                          auto factor_and_offset_string = GRM::strip(selector_part.substr(
                              parenthesis_position + 1, selector_part.size() - parenthesis_position - 2));
                          auto n_position = factor_and_offset_string.find('n');
                          std::string factor_string;
                          std::string offset_string;
                          if (n_position == std::string::npos)
                            {
                              parsed_individual_selectors.push_back(std::make_shared<GRM::FalseSelector>());
                              factor_string = "0";
                              offset_string = factor_and_offset_string;
                            }
                          else
                            {
                              factor_string = factor_and_offset_string.substr(0, n_position);
                              offset_string = factor_and_offset_string.substr(n_position + 1);
                            }
                          bool invalid = false;
                          long offset = 0;
                          long factor = 0;
                          if (factor_string.empty())
                            {
                              invalid = true;
                            }
                          else if (factor_string == "-")
                            {
                              factor = -1;
                            }
                          else
                            {
                              char *end;
                              factor = std::strtol(factor_string.c_str(), &end, 10);
                              if (end - factor_string.c_str() != factor_string.size())
                                {
                                  invalid = true;
                                }
                            }
                          if (offset_string.empty())
                            {
                              invalid = true;
                            }
                          else
                            {
                              bool must_be_positive = false;
                              if (offset_string.front() == '+')
                                {
                                  must_be_positive = true;
                                  offset_string = offset_string.substr(1);
                                }
                              if (offset_string.empty())
                                {
                                  invalid = true;
                                }
                              else
                                {
                                  char *end;
                                  offset = std::strtol(offset_string.c_str(), &end, 10);
                                  if (end - offset_string.c_str() != offset_string.size() ||
                                      (offset < 0 && must_be_positive))
                                    {
                                      invalid = true;
                                    }
                                }
                            }
                          if (invalid)
                            {
                              parsed_individual_selectors.push_back(std::make_shared<GRM::FalseSelector>());
                            }
                          else if (selector_type_string == ":nth-child")
                            {
                              parsed_individual_selectors.push_back(
                                  std::make_shared<GRM::NthChildSelector>(offset, factor, false));
                            }
                          else if (selector_type_string == ":nth-last-child")
                            {
                              parsed_individual_selectors.push_back(
                                  std::make_shared<GRM::NthChildSelector>(offset, factor, true));
                            }
                          else if (selector_type_string == ":nth-of-type")
                            {
                              parsed_individual_selectors.push_back(
                                  std::make_shared<GRM::NthOfTypeSelector>(offset, factor, false));
                            }
                          else if (selector_type_string == ":nth-last-of-type")
                            {
                              parsed_individual_selectors.push_back(
                                  std::make_shared<GRM::NthOfTypeSelector>(offset, factor, true));
                            }
                          else
                            {
                              parsed_individual_selectors.push_back(std::make_shared<GRM::FalseSelector>());
                            }
                        }
                      else
                        {
                          // most pseudo-elements are currently not supported
                          parsed_individual_selectors.push_back(std::make_shared<GRM::FalseSelector>());
                        }
                    }
                  else
                    {
                      parsed_selector_parts.push_back(std::make_shared<GRM::TagSelector>(selector_part));
                    }
                }
              if (in_bracketed_block)
                {
                  in_bracketed_block = false;
                  in_quotes = false;
                  quoting_character = '\0';
                }
            }
          if (it == local_selector.end())
            {
              break;
            }
          else if (*it == '[')
            {
              in_bracketed_block = true;
            }
          else if ((*it == '"' || *it == '\'') && in_bracketed_block && !in_quotes)
            {
              in_quotes = true;
            }
          else if (*it == quoting_character && in_bracketed_block && in_quotes && previous_character != '\\')
            {
              in_quotes = false;
            }
          if (previous_character == '\\')
            {
              previous_character = '\0';
            }
          else
            {
              previous_character = *it;
            }
        }
      if (parsed_selector_parts.size() == 1)
        {
          parsed_individual_selectors.push_back(parsed_selector_parts[0]);
        }
      else if (!parsed_selector_parts.empty())
        {
          parsed_individual_selectors.push_back(std::make_shared<GRM::AndCombinedSelector>(parsed_selector_parts));
        }
    }
  return std::make_shared<GRM::OrCombinedSelector>(parsed_individual_selectors);
}
