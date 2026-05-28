#ifndef GR_GRAPHICS_TREE_H
#define GR_GRAPHICS_TREE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "../../util.h"

typedef struct grm_comment grm_comment_t;
typedef struct grm_document grm_document_t;
typedef struct grm_element grm_element_t;
typedef struct grm_hierarchy_request_error grm_hierarchy_request_error_t;
typedef struct grm_node grm_node_t;
typedef struct grm_type grm_type_t;
typedef struct grm_selector grm_selector_t;
typedef struct grm_value grm_value_t;
typedef struct grm_serializer_options grm_serializer_options_t;

/* =============================== comment ========================================================================== */

GRM_EXPORT const char *grm_comment_data(grm_comment_t *comment);
GRM_EXPORT unsigned long grm_comment_length(grm_comment_t *comment);
GRM_EXPORT const char *grm_comment_substring_data(unsigned long offset, unsigned long count, grm_comment_t *comment);
GRM_EXPORT void grm_comment_append_data(const char *data, grm_comment_t *comment);
GRM_EXPORT void grm_comment_insert_data(unsigned long offset, const char *data, grm_comment_t *comment);
GRM_EXPORT void grm_comment_replace_data(unsigned long offset, unsigned long count, const char *data,
                                         grm_comment_t *comment);
GRM_EXPORT void grm_comment_delete_data(unsigned long offset, unsigned long count, grm_comment_t *comment);
GRM_EXPORT grm_element_t *grm_comment_previous_element_sibling(grm_comment_t *comment);
GRM_EXPORT grm_element_t *grm_comment_next_element_sibling(grm_comment_t *comment);
GRM_EXPORT const char *grm_comment_node_name(grm_comment_t *comment);
GRM_EXPORT int grm_comment_is_equal_node(grm_node_t *other_node, grm_comment_t *comment);

/* =============================== document ========================================================================= */

GRM_EXPORT grm_document_t *grm_document_create();
GRM_EXPORT grm_element_t *grm_document_document_element(grm_document_t *document);
GRM_EXPORT const grm_element_t *grm_document_document_element_const(grm_document_t *document);
GRM_EXPORT grm_element_t *grm_document_create_element(const char *local_name, grm_document_t *document);
GRM_EXPORT grm_comment_t *grm_document_create_comment(const char *data, grm_document_t *document);
GRM_EXPORT void grm_document_get_elements_by_tag_name(const char *qualified_name, grm_document_t *document,
                                                      grm_element_t **return_value);
GRM_EXPORT void grm_document_get_elements_by_tag_name_const(const char *qualified_name, grm_document_t *document,
                                                            const grm_element_t **return_value);
GRM_EXPORT void grm_document_get_elements_by_class_name(const char *class_names, grm_document_t *document,
                                                        grm_element_t **return_value);
GRM_EXPORT void grm_document_get_elements_by_class_name_const(const char *class_names, grm_document_t *document,
                                                              const grm_element_t **return_value);
GRM_EXPORT grm_element_t *grm_document_get_element_by_id(const char *id, grm_document_t *document);
GRM_EXPORT const grm_element_t *grm_document_get_element_by_id_const(const char *id, grm_document_t *document);
GRM_EXPORT grm_node_t *grm_document_adopt_node(grm_node_t *node, grm_document_t *document);
GRM_EXPORT grm_node_t *grm_document_import_node(grm_node_t *node, int deep, grm_document_t *document);
GRM_EXPORT void grm_document_children(grm_document_t *document, grm_element_t **return_value);
GRM_EXPORT void grm_document_children_const(grm_document_t *document, const grm_element_t **return_value);
GRM_EXPORT grm_element_t *grm_document_first_child_element(grm_document_t *document);
GRM_EXPORT const grm_element_t *grm_document_first_child_element_const(grm_document_t *document);
GRM_EXPORT grm_element_t *grm_document_last_child_element(grm_document_t *document);
GRM_EXPORT const grm_element_t *grm_document_last_child_element_const(grm_document_t *document);
GRM_EXPORT unsigned long grm_document_child_element_count(grm_document_t *document);
GRM_EXPORT void grm_document_prepend_t(grm_node_t *nodes, grm_document_t *document);
GRM_EXPORT void grm_document_prepend(grm_node_t **nodes, grm_document_t *document);
GRM_EXPORT void grm_document_append_t(grm_node_t *nodes, grm_document_t *document);
GRM_EXPORT void grm_document_append(grm_node_t **nodes, grm_document_t *document);
GRM_EXPORT void grm_document_replace_children_t(grm_node_t *nodes, grm_document_t *document);
GRM_EXPORT void grm_document_replace_children(grm_node_t **nodes, grm_document_t *document);
GRM_EXPORT void grm_document_query_selectors_all(const char *selectors, grm_document_t *document,
                                                 grm_element_t **return_value);
GRM_EXPORT void grm_document_query_selectors_all_const(const char *selectors, grm_document_t *document,
                                                       const grm_element_t **return_value);
GRM_EXPORT grm_element_t *grm_document_query_selectors(const char *selectors, grm_document_t *document);
GRM_EXPORT const grm_element_t *grm_document_query_selectors_const(const char *selectors, grm_document_t *document);
GRM_EXPORT const char *grm_document_node_name(grm_document_t *document);
GRM_EXPORT grm_document_t *grm_document_create_document();

/* =============================== element ========================================================================== */

GRM_EXPORT void grm_element_delete(grm_element_t *element);
GRM_EXPORT const char *grm_element_local_name(grm_element_t *element);
GRM_EXPORT const char *grm_element_tag_name(grm_element_t *element);
GRM_EXPORT const char *grm_element_id(grm_element_t *element);
GRM_EXPORT int grm_element_has_attributes(grm_element_t *element);
GRM_EXPORT void grm_element_get_attribute_names(grm_element_t *element, const char **return_value);
GRM_EXPORT grm_value_t *grm_element_get_attribute(const char *qualified_name, grm_element_t *element);
GRM_EXPORT void grm_element_set_attribute(const char *qualified_name, grm_value_t *value, grm_element_t *element);
GRM_EXPORT void grm_element_set_attribute_string(const char *qualified_name, const char *value, grm_element_t *element);
GRM_EXPORT void grm_element_set_attribute_double(const char *qualified_name, const double value,
                                                 grm_element_t *element);
GRM_EXPORT void grm_element_set_attribute_int(const char *qualified_name, const int value, grm_element_t *element);
GRM_EXPORT void grm_element_remove_attribute(const char *qualified_name, grm_element_t *element);
GRM_EXPORT int grm_element_toggle_attribute(const char *qualified_name, grm_element_t *element);
GRM_EXPORT int grm_element_toggle_attribute_force(const char *qualified_name, int force, grm_element_t *element);
GRM_EXPORT int grm_element_has_attribute(const char *qualified_name, grm_element_t *element);
GRM_EXPORT void grm_element_get_elements_by_tag_name(const char *qualified_name, grm_element_t *element,
                                                     grm_element_t **return_value);
GRM_EXPORT void grm_element_get_elements_by_tag_name_const(const char *qualified_name, grm_element_t *element,
                                                           const grm_element_t **return_value);
GRM_EXPORT void grm_element_get_elements_by_class_name_const(const char *class_names, grm_element_t *element,
                                                             grm_element_t **return_value);
GRM_EXPORT void grm_element_get_elements_by_class_name(const char *class_names, grm_element_t *element,
                                                       const grm_element_t **return_value);
GRM_EXPORT void grm_element_before(grm_element_t *node, grm_element_t *element);
GRM_EXPORT void grm_element_after(grm_element_t *node, grm_element_t *element);
GRM_EXPORT void grm_element_replace_with(grm_element_t *node, grm_element_t *element);
GRM_EXPORT void grm_element_remove(grm_element_t *element);
GRM_EXPORT void grm_element_children(grm_element_t *element, grm_element_t **return_value);
GRM_EXPORT void grm_element_children_const(grm_element_t *element, const grm_element_t **return_value);
GRM_EXPORT grm_element_t *grm_element_first_child_element(grm_element_t *element);
GRM_EXPORT const grm_element_t *grm_element_first_child_element_const(grm_element_t *element);
GRM_EXPORT grm_element_t *grm_element_last_child_element(grm_element_t *element);
GRM_EXPORT const grm_element_t *grm_element_last_child_element_const(grm_element_t *element);
GRM_EXPORT unsigned long grm_element_child_element_count(grm_element_t *element);
GRM_EXPORT void grm_element_prepend_t(grm_node_t *nodes, grm_element_t *element);
GRM_EXPORT void grm_element_prepend(grm_node_t **nodes, int nodes_length, grm_element_t *element);
GRM_EXPORT void grm_element_append_t(grm_node_t *nodes, grm_element_t *element);
GRM_EXPORT void grm_element_append(grm_node_t **nodes, int nodes_length, grm_element_t *element);
GRM_EXPORT void grm_element_replace_children_t(grm_node_t *nodes, grm_element_t *element);
GRM_EXPORT void grm_element_replace_children(grm_node_t **nodes, int nodes_length, grm_element_t *element);
GRM_EXPORT void grm_element_query_selectors_all(const char *selectors, grm_element_t *element,
                                                grm_element_t **return_value);
GRM_EXPORT void grm_element_query_selectors_all_const(const char *selectors, grm_element_t *element,
                                                      const grm_element_t **return_value);
GRM_EXPORT grm_element_t *grm_element_query_selectors(const char *selectors, grm_element_t *element);
GRM_EXPORT const grm_element_t *grm_element_query_selectors_const(const char *selectors, grm_element_t *element);
GRM_EXPORT grm_element_t *grm_element_previous_element_sibling(grm_element_t *element);
GRM_EXPORT const grm_element_t *grm_element_previous_element_sibling_const(grm_element_t *element);
GRM_EXPORT grm_element_t *grm_element_next_element_sibling(grm_element_t *element);
GRM_EXPORT const grm_element_t *grm_element_next_element_sibling_const(grm_element_t *element);
GRM_EXPORT const char *grm_element_node_name(grm_element_t *element);
GRM_EXPORT int grm_element_is_equal_node(grm_node_t *other_node, grm_element_t *element);

/* =============================== hierarchy request error ========================================================== */

GRM_EXPORT void grm_hierarchy_request_error_new(const char *what_arg, grm_hierarchy_request_error_t **a_error);

/* =============================== node ============================================================================= */

GRM_EXPORT void grm_node_delete(grm_node_t *node);
GRM_EXPORT grm_type_t *grm_node_type(grm_node_t *node);
GRM_EXPORT const char *grm_node_name(grm_node_t *node);
GRM_EXPORT int grm_node_is_connected(grm_node_t *node);
GRM_EXPORT grm_document_t *grm_node_owner_document(grm_node_t *node);
GRM_EXPORT const grm_document_t *grm_node_owner_document_const(grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_get_root(grm_node_t *node);
GRM_EXPORT const grm_node_t *grm_node_get_root_const(grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_parent(grm_node_t *node);
GRM_EXPORT const grm_node_t *grm_node_parent_const(grm_node_t *node);
GRM_EXPORT grm_element_t *grm_node_parent_element(grm_node_t *node);
GRM_EXPORT const grm_element_t *grm_node_parent_element_const(grm_node_t *node);
GRM_EXPORT int grm_node_has_child_nodes(grm_node_t *node);
GRM_EXPORT grm_node_t **grm_node_child_nodes(grm_node_t *node);
GRM_EXPORT const grm_node_t **grm_node_child_nodes_const(grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_first_child(grm_node_t *node);
GRM_EXPORT const grm_node_t *grm_node_first_child_const(grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_last_child(grm_node_t *node);
GRM_EXPORT const grm_node_t *grm_node_last_child_const(grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_previous_sibling(grm_node_t *node);
GRM_EXPORT const grm_node_t *grm_node_previous_sibling_const(grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_next_sibling(grm_node_t *node);
GRM_EXPORT const grm_node_t *grm_node_next_sibling_const(grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_clone(grm_node_t *node);
GRM_EXPORT const grm_node_t *grm_node_clone_deep(int deep, int clear_bbox, grm_node_t *node);
GRM_EXPORT int grm_node_is_equal_node(grm_node_t *other_node, grm_node_t *node);
GRM_EXPORT int grm_node_is_same_node(grm_node_t *other_node, grm_node_t *node);
GRM_EXPORT int grm_node_contains(grm_node_t *other_node, grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_insert_before(grm_node_t *new_node, grm_node_t *child, grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_append_child(grm_node_t *new_node, grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_replace_child(grm_node_t *new_node, grm_node_t *child, grm_node_t *node);
GRM_EXPORT grm_node_t *grm_node_remove_child(grm_node_t *child, grm_node_t *node);

/* =============================== util ============================================================================= */

GRM_EXPORT const char *grm_to_xml(grm_node_t *node, grm_serializer_options_t *options);
GRM_EXPORT const char *grm_to_lower(const char *string);
GRM_EXPORT const char *grm_to_upper(const char *string);
GRM_EXPORT void grm_split(const char *string, const char *token, const char **result);
GRM_EXPORT const char *grm_strip(const char *string);
GRM_EXPORT void grm_normalize_vec(double *x, int x_length, double **normalized_x);
GRM_EXPORT void grm_normalize_vec_int(double *x, int x_length, unsigned int **normalized_x, unsigned int sum);
GRM_EXPORT int grm_match_element(grm_element_t *element, grm_element_t **match_map_element,
                                 grm_selector_t **match_map_selector, int *match_map_bool, int match_map_length,
                                 grm_selector_t *selector);
GRM_EXPORT grm_selector_t *grm_parse_selectors(const char *selectors);

/* =============================== value ============================================================================ */

GRM_EXPORT void grm_value_new(grm_value_t **a_value);
GRM_EXPORT void grm_value_new_int(int value, grm_value_t **a_value);
GRM_EXPORT void grm_value_new_double(double value, grm_value_t **a_value);
GRM_EXPORT void grm_value_new_string(const char *value, grm_value_t **a_value);
GRM_EXPORT int grm_value_is_type(grm_type_t *type, grm_value_t *value);
GRM_EXPORT int grm_value_is_undefined(grm_value_t *value);
GRM_EXPORT int grm_value_is_int(grm_value_t *value);
GRM_EXPORT int grm_value_is_double(grm_value_t *value);
GRM_EXPORT int grm_value_is_string(grm_value_t *value);
GRM_EXPORT grm_type_t *grm_value_type(grm_value_t *value);

#ifdef __cplusplus
}
#endif
#endif // GR_GRAPHICS_TREE_H
