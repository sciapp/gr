#ifndef GR_GRAPHICS_TREE_INTERFACE_COMMENT_HXX
#define GR_GRAPHICS_TREE_INTERFACE_COMMENT_HXX

#ifndef GRM_INCLUDE_PATH_
#define GRM_QUOTE_(x) #x
#ifdef BUILDING_GR
#define GRM_INCLUDE_PATH_(filename) GRM_QUOTE_(filename)
#else
/* clang-format off */
#define GRM_INCLUDE_PATH_(filename) GRM_QUOTE_(grm/filename)
/* clang-format on */
#endif
#endif

/* clang-format off */
#include GRM_INCLUDE_PATH_(dom_render/graphics_tree/Node.hxx)
/* clang-format on */

namespace GR
{
class Document;

class Comment : public Node
{
public:
  // Comment interface
  const std::string &data() const;

  // CharacterData interface
  unsigned long length() const;

  std::string substringData(unsigned long offset, unsigned long count) const;

  void appendData(const std::string &data);

  void insertData(unsigned long offset, const std::string &data);

  void replaceData(unsigned long offset, unsigned long count, const std::string &data);

  void deleteData(unsigned long offset, unsigned long count);

  // NonDocumentTypeChildNode interface

  std::shared_ptr<Element> previousElementSibling();
  std::shared_ptr<const Element> previousElementSibling() const;

  std::shared_ptr<Element> nextElementSibling();
  std::shared_ptr<const Element> nextElementSibling() const;

  // virtual functions
  std::string nodeName() const override;

  bool isEqualNode(const std::shared_ptr<const Node> &otherNode) const override;

private:
  friend class Document;

  Comment(std::string data, const std::shared_ptr<Document> &owner_document);

  std::shared_ptr<Node> cloneIndividualNode() override;

  std::string m_data;
};
} // namespace GR

#endif
