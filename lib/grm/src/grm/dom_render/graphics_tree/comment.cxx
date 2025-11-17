#include <grm/dom_render/graphics_tree/comment.hxx>
#include <grm/dom_render/graphics_tree/index_size_error.hxx>

GRM::Comment::Comment(std::string data, const std::shared_ptr<GRM::Document> &owner_document)
    : GRM::Node(GRM::Node::Type::COMMENT_NODE, owner_document), m_data(std::move(data))
{
}

std::string GRM::Comment::nodeName() const
{
  return "#comment";
}

const std::string &GRM::Comment::data() const
{
  return m_data;
}

std::shared_ptr<GRM::Node> GRM::Comment::cloneIndividualNode()
{
  auto comment = std::shared_ptr<Comment>(new GRM::Comment(m_data, ownerDocument()));
  *comment = *this;
  return comment;
}

bool GRM::Comment::isEqualNode(const std::shared_ptr<const GRM::Node> &other_node) const
{
  auto other_node_as_comment = std::dynamic_pointer_cast<const GRM::Comment>(other_node);
  if (!other_node_as_comment) return false;
  return (other_node_as_comment->data() == data());
}

unsigned long GRM::Comment::length() const
{
  return static_cast<unsigned long>(m_data.size());
}

std::string GRM::Comment::substringData(unsigned long offset, unsigned long count) const
{
  return m_data.substr(offset, count);
}

void GRM::Comment::appendData(const std::string &data)
{
  m_data += data;
}

void GRM::Comment::insertData(unsigned long offset, const std::string &data)
{
  if (offset > length())
    {
      throw IndexSizeError("offset greater than length");
    }
  m_data.insert(offset, data);
}

void GRM::Comment::replaceData(unsigned long offset, unsigned long count, const std::string &data)
{
  if (offset > length())
    {
      throw IndexSizeError("offset greater than length");
    }
  m_data.replace(offset, count, data);
}

void GRM::Comment::deleteData(unsigned long offset, unsigned long count)
{
  replaceData(offset, count, "");
}

std::shared_ptr<GRM::Element> GRM::Comment::previousElementSibling()
{
  return previousElementSiblingImpl();
}

std::shared_ptr<const GRM::Element> GRM::Comment::previousElementSibling() const
{
  return previousElementSiblingImpl();
}

std::shared_ptr<GRM::Element> GRM::Comment::nextElementSibling()
{
  return nextElementSiblingImpl();
}
std::shared_ptr<const GRM::Element> GRM::Comment::nextElementSibling() const
{
  return nextElementSiblingImpl();
}
