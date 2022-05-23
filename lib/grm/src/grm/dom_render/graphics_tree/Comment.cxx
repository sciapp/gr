#include <grm/dom_render/graphics_tree/Comment.hxx>
#include <grm/dom_render/graphics_tree/IndexSizeError.hxx>

GR::Comment::Comment(std::string data, const std::shared_ptr<GR::Document> &owner_document)
    : GR::Node(GR::Node::Type::COMMENT_NODE, owner_document), m_data(std::move(data))
{
}

std::string GR::Comment::nodeName() const
{
  return "#comment";
}

const std::string &GR::Comment::data() const
{
  return m_data;
}

std::shared_ptr<GR::Node> GR::Comment::cloneIndividualNode()
{
  auto comment = std::shared_ptr<Comment>(new GR::Comment(m_data, ownerDocument()));
  *comment = *this;
  return comment;
}

bool GR::Comment::isEqualNode(const std::shared_ptr<const GR::Node> &otherNode) const
{
  auto other_node_as_comment = std::dynamic_pointer_cast<const GR::Comment>(otherNode);
  if (!other_node_as_comment)
    {
      return false;
    }
  return (other_node_as_comment->data() == data());
}

unsigned long GR::Comment::length() const
{
  return static_cast<unsigned long>(m_data.size());
}

std::string GR::Comment::substringData(unsigned long offset, unsigned long count) const
{
  return m_data.substr(offset, count);
}

void GR::Comment::appendData(const std::string &data)
{
  m_data += data;
}

void GR::Comment::insertData(unsigned long offset, const std::string &data)
{
  if (offset > length())
    {
      throw IndexSizeError("offset greater than length");
    }
  m_data.insert(offset, data);
}

void GR::Comment::replaceData(unsigned long offset, unsigned long count, const std::string &data)
{
  if (offset > length())
    {
      throw IndexSizeError("offset greater than length");
    }
  m_data.replace(offset, count, data);
}

void GR::Comment::deleteData(unsigned long offset, unsigned long count)
{
  replaceData(offset, count, "");
}

std::shared_ptr<GR::Element> GR::Comment::previousElementSibling()
{
  return previousElementSibling_impl();
}

std::shared_ptr<const GR::Element> GR::Comment::previousElementSibling() const
{
  return previousElementSibling_impl();
}

std::shared_ptr<GR::Element> GR::Comment::nextElementSibling()
{
  return nextElementSibling_impl();
}
std::shared_ptr<const GR::Element> GR::Comment::nextElementSibling() const
{
  return nextElementSibling_impl();
}
