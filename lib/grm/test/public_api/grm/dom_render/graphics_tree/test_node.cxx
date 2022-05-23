#include <catch2/catch.hpp>
#include "Node.hxx"
#include "Comment.hxx"
#include "Document.hxx"
#include "Element.hxx"
#include "NotFoundError.hxx"
#include "HierarchyRequestError.hxx"
#include "TypeError.hxx"

using GR::Comment;
using GR::Document;
using GR::Element;
using GR::HierarchyRequestError;
using GR::Node;
using GR::NotFoundError;
using GR::TypeError;
using GR::Value;

static std::shared_ptr<Node> createNode()
{
  static auto document = Document::createDocument();
  return document->createElement("test_node");
}

TEST_CASE("root nodes", "[node]")
{
  auto root_node = createNode();
  REQUIRE(root_node->getRootNode() == root_node);
  REQUIRE(root_node->parentNode() == nullptr);
  REQUIRE(!root_node->hasChildNodes());
}

TEST_CASE("Node::ownerDocument", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  REQUIRE(element->ownerDocument() == document);
}

TEST_CASE("Node::firstChild", "[node]")
{
  auto node = createNode();
  auto child_node = createNode();
  auto other_child_node = createNode();
  REQUIRE(node->firstChild() == nullptr);
  node->appendChild(child_node);
  REQUIRE(node->firstChild() == child_node);
  node->appendChild(other_child_node);
  REQUIRE(node->firstChild() == child_node);
}

TEST_CASE("Node::lastChild", "[node]")
{
  auto node = createNode();
  auto child_node = createNode();
  auto other_child_node = createNode();
  REQUIRE(node->lastChild() == nullptr);
  node->appendChild(child_node);
  REQUIRE(node->lastChild() == child_node);
  node->appendChild(other_child_node);
  REQUIRE(node->lastChild() == other_child_node);
}

TEST_CASE("Node::previousSibling", "[node]")
{
  auto node = createNode();
  auto child_node = createNode();
  auto other_child_node = createNode();
  REQUIRE(other_child_node->previousSibling() == nullptr);
  node->appendChild(child_node);
  REQUIRE(child_node->previousSibling() == nullptr);
  REQUIRE(other_child_node->previousSibling() == nullptr);
  node->appendChild(other_child_node);
  REQUIRE(other_child_node->previousSibling() == child_node);
}

TEST_CASE("Node::nextSibling", "[node]")
{
  auto node = createNode();
  auto child_node = createNode();
  auto other_child_node = createNode();
  REQUIRE(child_node->nextSibling() == nullptr);
  node->appendChild(child_node);
  REQUIRE(child_node->nextSibling() == nullptr);
  node->appendChild(other_child_node);
  REQUIRE(child_node->nextSibling() == other_child_node);
}

TEST_CASE("Node::getRootNode", "[node]")
{
  auto node = createNode();
  REQUIRE(node->getRootNode() == node);
  auto child_node = createNode();
  auto const_child_node = std::dynamic_pointer_cast<const Node>(child_node);
  REQUIRE(child_node->getRootNode() == child_node);
  REQUIRE(const_child_node->getRootNode() == const_child_node);
  node->appendChild(child_node);
  REQUIRE(child_node->getRootNode() == node);
  REQUIRE(const_child_node->getRootNode() == node);
}

TEST_CASE("Node::hasChildNodes", "[node]")
{
  auto node = createNode();
  REQUIRE(!node->hasChildNodes());
  auto child_node = createNode();
  node->appendChild(child_node);
  REQUIRE(node->hasChildNodes());
}

TEST_CASE("Node::cloneNode", "[node]")
{
  auto node = createNode();
  auto child_node = createNode();
  node->appendChild(child_node);
  REQUIRE(node->hasChildNodes());
  REQUIRE(!node->cloneNode()->hasChildNodes());
  REQUIRE(node->cloneNode(true)->hasChildNodes());
}

TEST_CASE("Node::isEqualNode", "[node]")
{
  auto node = createNode();
  auto other_node = node->ownerDocument()->createElement("test_node");
  REQUIRE(node->isEqualNode(node));
  REQUIRE(node->isEqualNode(other_node));
  other_node->setAttribute("test", "a");
  REQUIRE(!node->isEqualNode(other_node));
}

TEST_CASE("Node::isSameNode", "[node]")
{
  auto node = createNode();
  auto other_node = createNode();
  REQUIRE(node->isSameNode(node));
  REQUIRE(!node->isSameNode(other_node));
}

TEST_CASE("Node::contains", "[node]")
{
  auto root_node = createNode();
  auto child_node = createNode();
  auto other_node = createNode();
  REQUIRE(!root_node->contains(child_node));
  REQUIRE(!root_node->contains(other_node));
  root_node->appendChild(child_node);
  REQUIRE(root_node->contains(child_node));
  REQUIRE(!root_node->contains(other_node));
  child_node->appendChild(other_node);
  REQUIRE(root_node->contains(child_node));
  REQUIRE(root_node->contains(other_node));
  root_node->replaceChild(other_node, child_node);
  REQUIRE(!root_node->contains(child_node));
  REQUIRE(root_node->contains(other_node));
}

TEST_CASE("Node::insertBefore", "[node]")
{
  auto root_node = createNode();
  auto child_node = createNode();
  root_node->insertBefore(child_node, nullptr);
  REQUIRE(child_node->getRootNode() == root_node);
  REQUIRE(child_node->parentNode() == root_node);
  REQUIRE(root_node->hasChildNodes());
  REQUIRE(root_node->firstChild() == child_node);
  REQUIRE(root_node->lastChild() == child_node);
  auto other_child_node = createNode();
  root_node->insertBefore(other_child_node, child_node);
  REQUIRE(other_child_node->getRootNode() == root_node);
  REQUIRE(other_child_node->parentNode() == root_node);
  REQUIRE(root_node->firstChild() == other_child_node);
  REQUIRE(root_node->lastChild() == child_node);
  auto document = root_node->ownerDocument();
  auto comment = document->createComment("test");
  REQUIRE_THROWS_AS(comment->insertBefore(child_node, nullptr), HierarchyRequestError);
  REQUIRE_THROWS_AS(root_node->insertBefore(document, nullptr), HierarchyRequestError);
  REQUIRE_THROWS_AS(root_node->insertBefore(root_node, nullptr), HierarchyRequestError);
  REQUIRE_THROWS_AS(root_node->insertBefore(comment, root_node), NotFoundError);
  REQUIRE_THROWS_AS(root_node->insertBefore(nullptr, comment), TypeError);
}

TEST_CASE("Node::appendChild", "[node]")
{
  auto root_node = createNode();
  auto child_node = createNode();
  root_node->appendChild(child_node);
  REQUIRE(child_node->getRootNode() == root_node);
  REQUIRE(child_node->parentNode() == root_node);
  REQUIRE(root_node->hasChildNodes());
  REQUIRE(root_node->firstChild() == child_node);
  REQUIRE(root_node->lastChild() == child_node);
  auto other_child_node = createNode();
  root_node->appendChild(other_child_node);
  REQUIRE(other_child_node->getRootNode() == root_node);
  REQUIRE(other_child_node->parentNode() == root_node);
  REQUIRE(root_node->firstChild() == child_node);
  REQUIRE(root_node->lastChild() == other_child_node);
}

TEST_CASE("Node::replaceChild", "[node]")
{
  auto root_node = createNode();
  auto child_node = createNode();
  root_node->appendChild(child_node);
  auto other_child_node = createNode();
  root_node->replaceChild(other_child_node, child_node);
  REQUIRE(root_node->firstChild() == other_child_node);
  REQUIRE(root_node->lastChild() == other_child_node);
  REQUIRE(other_child_node->parentNode() == root_node);
  auto document = root_node->ownerDocument();
  auto comment = document->createComment("");
  REQUIRE_THROWS_AS(comment->replaceChild(child_node, other_child_node), HierarchyRequestError);
  REQUIRE_THROWS_AS(root_node->replaceChild(document, other_child_node), HierarchyRequestError);
  other_child_node->appendChild(child_node);
  REQUIRE_THROWS_AS(other_child_node->replaceChild(root_node, child_node), HierarchyRequestError);
  REQUIRE_THROWS_AS(other_child_node->replaceChild(child_node, comment), NotFoundError);
  REQUIRE_THROWS_AS(other_child_node->replaceChild(child_node, nullptr), TypeError);
  REQUIRE_THROWS_AS(other_child_node->replaceChild(nullptr, child_node), TypeError);
}

TEST_CASE("Node::removeChild", "[node]")
{
  auto root_node = createNode();
  auto child_node = createNode();
  root_node->appendChild(child_node);
  auto other_child_node = createNode();
  root_node->appendChild(other_child_node);
  root_node->removeChild(other_child_node);
  REQUIRE(root_node->hasChildNodes());
  REQUIRE(root_node->firstChild() == child_node);
  REQUIRE(root_node->lastChild() == child_node);
  REQUIRE(other_child_node->getRootNode() == other_child_node);
  REQUIRE(other_child_node->parentNode() == nullptr);
  REQUIRE_THROWS_AS(other_child_node->removeChild(root_node), NotFoundError);
  REQUIRE_THROWS_AS(root_node->removeChild(nullptr), TypeError);
}

TEST_CASE("Node::parentElement", "[node]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test");
  auto const_comment = std::dynamic_pointer_cast<const Node>(comment);
  REQUIRE(comment->parentElement() == nullptr);
  REQUIRE(const_comment->parentElement() == nullptr);
  document->append(comment);
  REQUIRE(comment->parentElement() == nullptr);
  REQUIRE(const_comment->parentElement() == nullptr);
  auto element = document->createElement("test");
  REQUIRE(comment->parentElement() == nullptr);
  REQUIRE(const_comment->parentElement() == nullptr);
  element->append(comment);
  REQUIRE(comment->parentElement() == element);
  REQUIRE(const_comment->parentElement() == element);
}

TEST_CASE("Node::isConnected", "[node]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test");
  REQUIRE(!comment->isConnected());
  document->appendChild(comment);
  REQUIRE(comment->isConnected());
}
