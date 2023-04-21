#ifndef GRM_GRAPHICS_TREE_INTERFACE_COMMENT_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_COMMENT_HXX

#include <grm/dom_render/graphics_tree/Node.hxx>
#include <grm/util.h>

namespace GRM
{
class Document;

class EXPORT Comment : public Node
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
} // namespace GRM

#endif
