#include "grm/dom_render/graphics_tree/graphics_tree_c.h"
#include "grm/dom_render/render.hxx"
#include "grm/dom_render/casts.hxx"
#include "grm/dom_render/context.hxx"
#include "grm/dom_render/creator.hxx"
#include "grm/dom_render/process_attributes.hxx"
#include "grm/dom_render/process_elements.hxx"
#include "grm/dom_render/render_util.hxx"
#include "grm/dom_render/updater.hxx"
#include "grm/dom_render/graphics_tree/comment.hxx"
#include "grm/dom_render/graphics_tree/hierarchy_request_error.hxx"
#include "grm/dom_render/graphics_tree/util.hxx"

const char *grm_comment_data(grm_comment_t *comment)
{
  return reinterpret_cast<GRM::Comment *>(comment)->data().c_str();
}

unsigned long grm_comment_length(grm_comment_t *comment)
{
  return reinterpret_cast<GRM::Comment *>(comment)->length();
}

const char *grm_comment_substring_data(unsigned long offset, unsigned long count, grm_comment_t *comment)
{
  return strdup(reinterpret_cast<GRM::Comment *>(comment)->substringData(offset, count).c_str());
}

void grm_comment_append_data(const char *data, grm_comment_t *comment)
{
  reinterpret_cast<GRM::Comment *>(comment)->appendData(data);
}

void grm_comment_insert_data(unsigned long offset, const char *data, grm_comment_t *comment)
{
  reinterpret_cast<GRM::Comment *>(comment)->insertData(offset, data);
}

void grm_comment_replace_data(unsigned long offset, unsigned long count, const char *data, grm_comment_t *comment)
{
  reinterpret_cast<GRM::Comment *>(comment)->replaceData(offset, count, data);
}

void grm_comment_delete_data(unsigned long offset, unsigned long count, grm_comment_t *comment)
{
  reinterpret_cast<GRM::Comment *>(comment)->deleteData(offset, count);
}

grm_element_t *grm_comment_previous_element_sibling(grm_comment_t *comment)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Comment *>(comment)->previousElementSibling().get());
}

grm_element_t *grm_comment_next_element_sibling(grm_comment_t *comment)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Comment *>(comment)->nextElementSibling().get());
}

const char *grm_comment_node_name(grm_comment_t *comment)
{
  return strdup(reinterpret_cast<GRM::Comment *>(comment)->nodeName().c_str());
}

int grm_comment_is_equal_node(grm_node_t *other_node, grm_comment_t *comment)
{
  auto node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(other_node));
  return reinterpret_cast<GRM::Comment *>(comment)->isEqualNode(node_ptr);
}

/* =============================== document ========================================================================= */

grm_document_t *grm_document_create()
{
  return reinterpret_cast<grm_document_t *>(GRM::createDocument().get());
}

grm_element_t *grm_document_document_element(grm_document_t *document)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Document *>(document)->documentElement().get());
}

const grm_element_t *grm_document_document_element_const(grm_document_t *document)
{
  return reinterpret_cast<const grm_element_t *>(reinterpret_cast<GRM::Document *>(document)->documentElement().get());
}

grm_element_t *grm_document_create_element(const char *local_name, grm_document_t *document)
{
  return reinterpret_cast<grm_element_t *>(
      reinterpret_cast<GRM::Document *>(document)->createElement(local_name).get());
}

grm_comment_t *grm_document_create_comment(const char *data, grm_document_t *document)
{
  return reinterpret_cast<grm_comment_t *>(reinterpret_cast<GRM::Document *>(document)->createComment(data).get());
}

void grm_document_get_elements_by_tag_name(const char *qualified_name, grm_document_t *document,
                                           grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Document *>(document)->getElementsByTagName(qualified_name);
  return_value = static_cast<grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_document_get_elements_by_tag_name_const(const char *qualified_name, grm_document_t *document,
                                                 const grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Document *>(document)->getElementsByTagName(qualified_name);
  return_value =
      static_cast<const grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(const grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<const grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_document_get_elements_by_class_name(const char *class_names, grm_document_t *document,
                                             grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Document *>(document)->getElementsByClassName(class_names);
  return_value = static_cast<grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_document_get_elements_by_class_name_const(const char *class_names, grm_document_t *document,
                                                   const grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Document *>(document)->getElementsByClassName(class_names);
  return_value =
      static_cast<const grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(const grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<const grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

grm_element_t *grm_document_get_element_by_id(const char *id, grm_document_t *document)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Document *>(document)->getElementById(id).get());
}

const grm_element_t *grm_document_get_element_by_id_const(const char *id, grm_document_t *document)
{
  return reinterpret_cast<const grm_element_t *>(reinterpret_cast<GRM::Document *>(document)->getElementById(id).get());
}

grm_node_t *grm_document_adopt_node(grm_node_t *node, grm_document_t *document)
{
  auto node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(node));
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Document *>(document)->adoptNode(node_ptr).get());
}

grm_node_t *grm_document_import_node(grm_node_t *node, int deep, grm_document_t *document)
{
  auto node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(node));
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Document *>(document)->importNode(node_ptr, deep).get());
}

void grm_document_children(grm_document_t *document, grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Document *>(document)->children();
  return_value = static_cast<grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_document_children_const(grm_document_t *document, const grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Document *>(document)->children();
  return_value =
      static_cast<const grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(const grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<const grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

grm_element_t *grm_document_first_child_element(grm_document_t *document)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Document *>(document)->firstChildElement().get());
}

const grm_element_t *grm_document_first_child_element_const(grm_document_t *document)
{
  return reinterpret_cast<const grm_element_t *>(
      reinterpret_cast<GRM::Document *>(document)->firstChildElement().get());
}

grm_element_t *grm_document_last_child_element(grm_document_t *document)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Document *>(document)->lastChildElement().get());
}

const grm_element_t *grm_document_last_child_element_const(grm_document_t *document)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Document *>(document)->lastChildElement().get());
}

unsigned long grm_document_child_element_count(grm_document_t *document)
{
  return reinterpret_cast<GRM::Document *>(document)->childElementCount();
}

void grm_document_prepend_t(grm_node_t *nodes, grm_document_t *document)
{
  auto nodes_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes));
  reinterpret_cast<GRM::Document *>(document)->prepend(nodes_ptr);
}

void grm_document_prepend(grm_node_t **nodes, grm_document_t *document)
{
  auto nodes_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes));
  reinterpret_cast<GRM::Document *>(document)->prepend(nodes_ptr);
}

void grm_document_append_t(grm_node_t *nodes, grm_document_t *document)
{
  auto nodes_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes));
  reinterpret_cast<GRM::Document *>(document)->append(nodes_ptr);
}

void grm_document_append(grm_node_t **nodes, grm_document_t *document)
{
  auto nodes_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes));
  reinterpret_cast<GRM::Document *>(document)->append(nodes_ptr);
}

void grm_document_replace_children_t(grm_node_t *nodes, grm_document_t *document)
{
  auto nodes_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes));
  reinterpret_cast<GRM::Document *>(document)->replaceChildren(nodes_ptr);
}

void grm_document_replace_children(grm_node_t **nodes, grm_document_t *document)
{
  auto nodes_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes));
  reinterpret_cast<GRM::Document *>(document)->replaceChildren(nodes_ptr);
}

void grm_document_query_selectors_all(const char *selectors, grm_document_t *document, grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Document *>(document)->querySelectorsAll(selectors);
  return_value = static_cast<grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_document_query_selectors_all_const(const char *selectors, grm_document_t *document,
                                            const grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Document *>(document)->querySelectorsAll(selectors);
  return_value =
      static_cast<const grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(const grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<const grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

grm_element_t *grm_document_query_selectors(const char *selectors, grm_document_t *document)
{
  return reinterpret_cast<grm_element_t *>(
      reinterpret_cast<GRM::Document *>(document)->querySelectors(selectors).get());
}

const grm_element_t *grm_document_query_selectors_const(const char *selectors, grm_document_t *document)
{
  return reinterpret_cast<const grm_element_t *>(
      reinterpret_cast<GRM::Document *>(document)->querySelectors(selectors).get());
}

const char *grm_document_node_name(grm_document_t *document)
{
  return strdup(reinterpret_cast<GRM::Document *>(document)->nodeName().c_str());
}

grm_document_t *grm_document_create_document()
{
  return reinterpret_cast<grm_document_t *>(GRM::createDocument().get());
}

/* =============================== element ========================================================================== */

void grm_element_delete(grm_element_t *element)
{
  delete reinterpret_cast<GRM::Element *>(element);
}

const char *grm_element_local_name(grm_element_t *element)
{
  return strdup(reinterpret_cast<GRM::Element *>(element)->localName().c_str());
}

const char *grm_element_tag_name(grm_element_t *element)
{
  return strdup(reinterpret_cast<GRM::Element *>(element)->tagName().c_str());
}

const char *grm_element_id(grm_element_t *element)
{
  return strdup(reinterpret_cast<GRM::Element *>(element)->id().c_str());
}

int grm_element_has_attributes(grm_element_t *element)
{
  return reinterpret_cast<GRM::Element *>(element)->hasAttributes();
}

void grm_element_get_attribute_names(grm_element_t *element, const char **return_value)
{
  auto string_set = reinterpret_cast<GRM::Element *>(element)->getAttributeNames();
  auto string_set_iter = string_set.begin();
  return_value = static_cast<const char **>(realloc(return_value, string_set.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_set_iter->c_str();
      return_value++;
      ++string_set_iter;
    }
}

grm_value_t *grm_element_get_attribute(const char *qualified_name, grm_element_t *element)
{
  static auto tmp = reinterpret_cast<GRM::Element *>(element)->getAttribute(qualified_name);
  return reinterpret_cast<grm_value_t *>(&tmp);
}

void grm_element_set_attribute(const char *qualified_name, grm_value_t *value, grm_element_t *element)
{
  auto value_ptr = reinterpret_cast<GRM::Value *>(value);
  reinterpret_cast<GRM::Element *>(element)->setAttribute(qualified_name, *value_ptr);
}

void grm_element_set_attribute_string(const char *qualified_name, const char *value, grm_element_t *element)
{
  reinterpret_cast<GRM::Element *>(element)->setAttribute(qualified_name, value);
}

void grm_element_set_attribute_double(const char *qualified_name, const double value, grm_element_t *element)
{
  reinterpret_cast<GRM::Element *>(element)->setAttribute(qualified_name, value);
}

void grm_element_set_attribute_int(const char *qualified_name, const int value, grm_element_t *element)
{
  reinterpret_cast<GRM::Element *>(element)->setAttribute(qualified_name, value);
}

void grm_element_remove_attribute(const char *qualified_name, grm_element_t *element)
{
  reinterpret_cast<GRM::Element *>(element)->removeAttribute(qualified_name);
}

int grm_element_toggle_attribute(const char *qualified_name, grm_element_t *element)
{
  return reinterpret_cast<GRM::Element *>(element)->toggleAttribute(qualified_name);
}

int grm_element_toggle_attribute_force(const char *qualified_name, int force, grm_element_t *element)
{
  return reinterpret_cast<GRM::Element *>(element)->toggleAttribute(qualified_name, force);
}

int grm_element_has_attribute(const char *qualified_name, grm_element_t *element)
{
  return reinterpret_cast<GRM::Element *>(element)->hasAttribute(qualified_name);
}

void grm_element_get_elements_by_tag_name(const char *qualified_name, grm_element_t *element,
                                          grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Element *>(element)->getElementsByTagName(qualified_name);
  return_value = static_cast<grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_element_get_elements_by_tag_name_const(const char *qualified_name, grm_element_t *element,
                                                const grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Element *>(element)->getElementsByTagName(qualified_name);
  return_value =
      static_cast<const grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(const grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<const grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_element_get_elements_by_class_name(const char *class_names, grm_element_t *element,
                                            grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Element *>(element)->getElementsByClassName(class_names);
  return_value = static_cast<grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_element_get_elements_by_class_name_const(const char *class_names, grm_element_t *element,
                                                  const grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Element *>(element)->getElementsByClassName(class_names);
  return_value =
      static_cast<const grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(const grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<const grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_element_before(grm_element_t *node, grm_element_t *element)
{
  auto node_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(node));
  reinterpret_cast<GRM::Element *>(element)->before(node_ptr);
}

void grm_element_after(grm_element_t *node, grm_element_t *element)
{
  auto node_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(node));
  reinterpret_cast<GRM::Element *>(element)->after(node_ptr);
}

void grm_element_replace_with(grm_element_t *node, grm_element_t *element)
{
  auto node_ptr = std::shared_ptr<GRM::Element>(reinterpret_cast<GRM::Element *>(node));
  reinterpret_cast<GRM::Element *>(element)->replaceWith(node_ptr);
}

void grm_element_remove(grm_element_t *element)
{
  reinterpret_cast<GRM::Element *>(element)->remove();
}

void grm_element_children(grm_element_t *element, grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Element *>(element)->children();
  return_value = static_cast<grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_element_children_const(grm_element_t *element, const grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Element *>(element)->children();
  return_value =
      static_cast<const grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(const grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<const grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

grm_element_t *grm_element_first_child_element(grm_element_t *element)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Element *>(element)->firstChildElement().get());
}

const grm_element_t *grm_element_first_child_element_const(grm_element_t *element)
{
  return reinterpret_cast<const grm_element_t *>(reinterpret_cast<GRM::Element *>(element)->firstChildElement().get());
}

grm_element_t *grm_element_last_child_element(grm_element_t *element)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Element *>(element)->lastChildElement().get());
}

const grm_element_t *grm_element_last_child_element_const(grm_element_t *element)
{
  return reinterpret_cast<const grm_element_t *>(reinterpret_cast<GRM::Element *>(element)->lastChildElement().get());
}

unsigned long grm_element_child_element_count(grm_element_t *element)
{
  return reinterpret_cast<GRM::Element *>(element)->childElementCount();
}

void grm_element_prepend_t(grm_node_t *nodes, grm_element_t *element)
{
  auto node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes));
  reinterpret_cast<GRM::Element *>(element)->prepend(node_ptr);
}

void grm_element_prepend(grm_node_t **nodes, int nodes_length, grm_element_t *element)
{
  std::vector<std::shared_ptr<GRM::Node>> node_vec;
  for (int i = 0; i < nodes_length; i++)
    {
      node_vec.push_back(std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes[i])));
    }
  reinterpret_cast<GRM::Element *>(element)->prepend(node_vec);
}

void grm_element_append_t(grm_node_t *nodes, grm_element_t *element)
{
  auto node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes));
  reinterpret_cast<GRM::Element *>(element)->append(node_ptr);
}

void grm_element_append(grm_node_t **nodes, int nodes_length, grm_element_t *element)
{
  std::vector<std::shared_ptr<GRM::Node>> node_vec;
  for (int i = 0; i < nodes_length; i++)
    {
      node_vec.push_back(std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes[i])));
    }
  reinterpret_cast<GRM::Element *>(element)->append(node_vec);
}

void grm_element_replace_children_t(grm_node_t *nodes, grm_element_t *element)
{
  auto node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes));
  reinterpret_cast<GRM::Element *>(element)->replaceChildren(node_ptr);
}

void grm_element_replace_children(grm_node_t **nodes, int nodes_length, grm_element_t *element)
{
  std::vector<std::shared_ptr<GRM::Node>> node_vec;
  for (int i = 0; i < nodes_length; i++)
    {
      node_vec.push_back(std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(nodes[i])));
    }
  reinterpret_cast<GRM::Element *>(element)->replaceChildren(node_vec);
}

void grm_element_query_selectors_all(const char *selectors, grm_element_t *element, grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Element *>(element)->querySelectorsAll(selectors);
  return_value = static_cast<grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

void grm_element_query_selectors_all_const(const char *selectors, grm_element_t *element,
                                           const grm_element_t **return_value)
{
  int i = 0;
  auto element_vec = reinterpret_cast<GRM::Element *>(element)->querySelectorsAll(selectors);
  return_value =
      static_cast<const grm_element_t **>(realloc(return_value, element_vec.size() * sizeof(const grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<const grm_element_t *>(element_vec[i++].get());
      return_value++;
    }
}

grm_element_t *grm_element_query_selectors(const char *selectors, grm_element_t *element)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Element *>(element)->querySelectors(selectors).get());
}

const grm_element_t *grm_element_query_selectors_const(const char *selectors, grm_element_t *element)
{
  return reinterpret_cast<const grm_element_t *>(
      reinterpret_cast<GRM::Element *>(element)->querySelectors(selectors).get());
}

grm_element_t *grm_element_previous_element_sibling(grm_element_t *element)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Element *>(element)->previousElementSibling().get());
}

const grm_element_t *grm_element_previous_element_sibling_const(grm_element_t *element)
{
  return reinterpret_cast<const grm_element_t *>(
      reinterpret_cast<GRM::Element *>(element)->previousElementSibling().get());
}

grm_element_t *grm_element_next_element_sibling(grm_element_t *element)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Element *>(element)->nextElementSibling().get());
}

const grm_element_t *grm_element_next_element_sibling_const(grm_element_t *element)
{
  return reinterpret_cast<const grm_element_t *>(reinterpret_cast<GRM::Element *>(element)->nextElementSibling().get());
}

const char *grm_element_node_name(grm_element_t *element)
{
  return strdup(reinterpret_cast<GRM::Element *>(element)->nodeName().c_str());
}

int grm_element_is_equal_node(grm_node_t *other_node, grm_element_t *element)
{
  auto other_node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(other_node));
  return reinterpret_cast<GRM::Element *>(element)->isEqualNode(other_node_ptr);
}

/* =============================== hierarchy request error ========================================================== */

void grm_hierarchy_request_error_new(const char *what_arg, grm_hierarchy_request_error_t **a_error)
{
  *a_error = reinterpret_cast<grm_hierarchy_request_error_t *>(new GRM::HierarchyRequestError(what_arg));
}

/* =============================== node ============================================================================= */

void grm_node_delete(grm_node_t *node)
{
  delete reinterpret_cast<GRM::Node *>(node);
}

grm_type_t *grm_node_type(grm_node_t *node)
{
  return reinterpret_cast<grm_type_t *>(reinterpret_cast<GRM::Node *>(node)->nodeType());
}

const char *grm_node_name(grm_node_t *node)
{
  return strdup(reinterpret_cast<GRM::Node *>(node)->nodeName().c_str());
}

int grm_node_is_connected(grm_node_t *node)
{
  return reinterpret_cast<GRM::Node *>(node)->isConnected();
}

grm_document_t *grm_node_owner_document(grm_node_t *node)
{
  return reinterpret_cast<grm_document_t *>(reinterpret_cast<GRM::Node *>(node)->ownerDocument().get());
}

const grm_document_t *grm_node_owner_document_const(grm_node_t *node)
{
  return reinterpret_cast<const grm_document_t *>(reinterpret_cast<GRM::Node *>(node)->ownerDocument().get());
}

grm_node_t *grm_node_get_root(grm_node_t *node)
{
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->getRootNode().get());
}

const grm_node_t *grm_node_get_root_const(grm_node_t *node)
{
  return reinterpret_cast<const grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->getRootNode().get());
}

grm_node_t *grm_node_parent(grm_node_t *node)
{
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->parentNode().get());
}

const grm_node_t *grm_node_parent_const(grm_node_t *node)
{
  return reinterpret_cast<const grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->parentNode().get());
}

grm_element_t *grm_node_parent_element(grm_node_t *node)
{
  return reinterpret_cast<grm_element_t *>(reinterpret_cast<GRM::Node *>(node)->parentElement().get());
}

const grm_element_t *grm_node_parent_element_const(grm_node_t *node)
{
  return reinterpret_cast<const grm_element_t *>(reinterpret_cast<GRM::Node *>(node)->parentElement().get());
}

int grm_node_has_child_nodes(grm_node_t *node)
{
  return reinterpret_cast<GRM::Node *>(node)->hasChildNodes();
}

void grm_node_child_nodes(grm_node_t *node, grm_element_t **return_value)
{
  int i = 0;
  auto node_vec = reinterpret_cast<GRM::Node *>(node)->childNodes();
  return_value = static_cast<grm_element_t **>(realloc(return_value, node_vec.size() * sizeof(grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<grm_element_t *>(node_vec[i++].get());
      return_value++;
    }
}

void grm_node_child_nodes_const(grm_node_t *node, const grm_element_t **return_value)
{
  int i = 0;
  auto node_vec = reinterpret_cast<GRM::Node *>(node)->childNodes();
  return_value =
      static_cast<const grm_element_t **>(realloc(return_value, node_vec.size() * sizeof(const grm_element_t *)));
  while (return_value != nullptr)
    {
      *return_value = reinterpret_cast<const grm_element_t *>(node_vec[i++].get());
      return_value++;
    }
}

grm_node_t *grm_node_first_child(grm_node_t *node)
{
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->firstChild().get());
}

const grm_node_t *grm_node_first_child_const(grm_node_t *node)
{
  return reinterpret_cast<const grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->firstChild().get());
}

grm_node_t *grm_node_last_child(grm_node_t *node)
{
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->lastChild().get());
}

const grm_node_t *grm_node_last_child_const(grm_node_t *node)
{
  return reinterpret_cast<const grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->lastChild().get());
}

grm_node_t *grm_node_previous_sibling(grm_node_t *node)
{
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->previousSibling().get());
}

const grm_node_t *grm_node_previous_sibling_const(grm_node_t *node)
{
  return reinterpret_cast<const grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->previousSibling().get());
}

grm_node_t *grm_node_next_sibling(grm_node_t *node)
{
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->nextSibling().get());
}

const grm_node_t *grm_node_next_sibling_const(grm_node_t *node)
{
  return reinterpret_cast<const grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->nextSibling().get());
}

grm_node_t *grm_node_clone(grm_node_t *node)
{
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->cloneNode().get());
}

const grm_node_t *grm_node_clone_deep(int deep, int clear_bbox, grm_node_t *node)
{
  return reinterpret_cast<const grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->cloneNode(deep, clear_bbox).get());
}

int grm_node_is_equal_node(grm_node_t *other_node, grm_node_t *node)
{
  auto other_node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(other_node));
  return reinterpret_cast<GRM::Node *>(node)->isEqualNode(other_node_ptr);
}

int grm_node_is_same_node(grm_node_t *other_node, grm_node_t *node)
{
  auto other_node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(other_node));
  return reinterpret_cast<GRM::Node *>(node)->isSameNode(other_node_ptr);
}

int grm_node_contains(grm_node_t *other_node, grm_node_t *node)
{
  auto other_node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(other_node));
  return reinterpret_cast<GRM::Node *>(node)->contains(other_node_ptr);
}

grm_node_t *grm_node_insert_before(grm_node_t *node, grm_node_t *child, grm_node_t *a_node)
{
  auto child_node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(child));
  auto node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(node));
  return reinterpret_cast<grm_node_t *>(
      reinterpret_cast<GRM::Node *>(a_node)->insertBefore(node_ptr, child_node_ptr).get());
}

grm_node_t *grm_node_append_child(grm_node_t *node, grm_node_t *a_node)
{
  auto node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(node));
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Node *>(a_node)->appendChild(node_ptr).get());
}

grm_node_t *grm_node_replace_child(grm_node_t *node, grm_node_t *child, grm_node_t *a_node)
{
  auto node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(node));
  auto child_node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(child));
  return reinterpret_cast<grm_node_t *>(
      reinterpret_cast<GRM::Node *>(a_node)->replaceChild(node_ptr, child_node_ptr).get());
}

grm_node_t *grm_node_remove_child(grm_node_t *child, grm_node_t *node)
{
  auto child_node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(child));
  return reinterpret_cast<grm_node_t *>(reinterpret_cast<GRM::Node *>(node)->removeChild(child_node_ptr).get());
}

/* =============================== util ============================================================================= */

const char *grm_to_xml(grm_node_t *node, grm_serializer_options_t *options)
{
  auto node_ptr = std::shared_ptr<GRM::Node>(reinterpret_cast<GRM::Node *>(node));
  return strdup(GRM::toXML(node_ptr, *reinterpret_cast<GRM::SerializerOptions *>(options), nullptr).c_str());
}

const char *grm_to_lower(const char *string)
{
  return strdup(GRM::toLower(string).c_str());
}

const char *grm_to_upper(const char *string)
{
  return strdup(GRM::toUpper(string).c_str());
}

void grm_split(const char *string, const char *token, const char **return_value)
{
  int i = 0;
  auto string_vec = GRM::split(string, token);
  return_value = static_cast<const char **>(realloc(return_value, string_vec.size() * sizeof(const char *)));
  while (return_value != nullptr)
    {
      *return_value = string_vec[i++].c_str();
      return_value++;
    }
}

const char *grm_strip(const char *string)
{
  return strdup(GRM::strip(string).c_str());
}

void grm_normalize_vec(double *x, int x_length, double **normalized_x)
{
  std::vector<double> x_vec, x_normalized_vec;
  for (int i = 0; i < x_length; i++)
    {
      x_vec.push_back(x[i]);
    }
  GRM::normalizeVec(x_vec, &x_normalized_vec);
  for (int i = 0; i < x_length; i++)
    {
      *normalized_x[i] = x_normalized_vec[i];
    }
}

void grm_normalize_vec_int(double *x, int x_length, unsigned int **normalized_x, unsigned int sum)
{
  std::vector<double> x_vec;
  std::vector<unsigned int> x_normalized_vec;
  for (int i = 0; i < x_length; i++)
    {
      x_vec.push_back(x[i]);
    }
  GRM::normalizeVecInt(x_vec, &x_normalized_vec, sum);
  for (int i = 0; i < x_length; i++)
    {
      *normalized_x[i] = x_normalized_vec[i];
    }
}

int grm_match_element(grm_element_t *element, grm_element_t **match_map_element, grm_selector_t **match_map_selector,
                      int *match_map_bool, int match_map_length, grm_selector_t *selector)
{
  std::map<std::tuple<const GRM::Element *, const GRM::Selector *>, bool> match_map;
  auto map_element = reinterpret_cast<GRM::Element **>(match_map_element);
  auto map_selector = reinterpret_cast<GRM::Selector **>(match_map_selector);
  for (int i = 0; i < match_map_length; i++)
    {
      std::tuple<const GRM::Element *, const GRM::Selector *> a{map_element[i], map_selector[i]};
      match_map.emplace(a, static_cast<bool>(match_map_bool[i]));
    }

  return reinterpret_cast<GRM::Selector *>(selector)->matchElement(*reinterpret_cast<GRM::Element *>(element),
                                                                   match_map);
}

grm_selector_t *grm_parse_selectors(const char *selectors)
{
  return reinterpret_cast<grm_selector_t *>(GRM::parseSelectors(selectors).get());
}

/* =============================== value ============================================================================ */

void grm_value_new(grm_value_t **a_value)
{
  *a_value = reinterpret_cast<grm_value_t *>(new GRM::Value);
}

void grm_value_new_int(int value, grm_value_t **a_value)
{
  *a_value = reinterpret_cast<grm_value_t *>(new GRM::Value(value));
}

void grm_value_new_double(double value, grm_value_t **a_value)
{
  *a_value = reinterpret_cast<grm_value_t *>(new GRM::Value(value));
}

void grm_value_new_string(const char *value, grm_value_t **a_value)
{
  *a_value = reinterpret_cast<grm_value_t *>(new GRM::Value(value));
}

int grm_value_is_type(grm_type_t *type, grm_value_t *value)
{
  return reinterpret_cast<GRM::Value *>(value)->isType(*reinterpret_cast<GRM::Value::Type *>(type));
}

int grm_value_is_undefined(grm_value_t *value)
{
  return reinterpret_cast<GRM::Value *>(value)->isUndefined();
}

int grm_value_is_int(grm_value_t *value)
{
  return reinterpret_cast<GRM::Value *>(value)->isInt();
}

int grm_value_is_double(grm_value_t *value)
{
  return reinterpret_cast<GRM::Value *>(value)->isDouble();
}

int grm_value_is_string(grm_value_t *value)
{
  return reinterpret_cast<GRM::Value *>(value)->isString();
}

grm_type_t *grm_value_type(grm_value_t *value)
{
  return reinterpret_cast<grm_type_t *>(reinterpret_cast<GRM::Value *>(value)->type());
}
