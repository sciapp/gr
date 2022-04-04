#include <catch2/catch.hpp>
#include "GR/Comment.hxx"
#include "GR/Element.hxx"
#include "GR/Document.hxx"
#include "GR/IndexSizeError.hxx"

using GR::Comment;
using GR::Document;
using GR::Element;
using GR::IndexSizeError;
using GR::Node;
using GR::Value;

TEST_CASE("Comment::nodeName", "[comment]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test");
  REQUIRE(comment->nodeName() == "#comment");
}

TEST_CASE("Comment::cloneNode", "[comment]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test");
  auto comment2 = std::dynamic_pointer_cast<Comment>(comment->cloneNode());
  REQUIRE(comment2->data() == "test");
  REQUIRE(comment2->ownerDocument() == document);
}

TEST_CASE("Comment::isEqualNode", "[comment]")
{
  auto document = Document::createDocument();
  auto comment1 = document->createComment("test");
  auto comment2 = document->createComment("test");
  auto comment3 = document->createComment("test2");
  auto element = document->createElement("test");
  REQUIRE(comment1->isEqualNode(comment2));
  REQUIRE(!comment1->isEqualNode(comment3));
  REQUIRE(!comment1->isEqualNode(element));
}

TEST_CASE("Comment::length", "[comment]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test");
  REQUIRE(comment->length() == 4);
}

TEST_CASE("Comment::substringData", "[comment]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test");
  REQUIRE(comment->substringData(0, 4) == "test");
  REQUIRE(comment->substringData(1, 4) == "est");
  REQUIRE(comment->substringData(1, 2) == "es");
}

TEST_CASE("Comment::appendData", "[comment]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test");
  REQUIRE(comment->data() == "test");
  comment->appendData("");
  REQUIRE(comment->data() == "test");
  comment->appendData("_append");
  REQUIRE(comment->data() == "test_append");
}

TEST_CASE("Comment::insertData", "[comment]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test");
  REQUIRE(comment->data() == "test");
  comment->insertData(0, "");
  REQUIRE(comment->data() == "test");
  comment->insertData(0, "abc");
  REQUIRE(comment->data() == "abctest");
  comment->insertData(4, "123");
  REQUIRE(comment->data() == "abct123est");
  comment->insertData(comment->length(), ".");
  REQUIRE(comment->data() == "abct123est.");
  REQUIRE_THROWS_AS(comment->insertData(comment->length() + 1, "abc"), IndexSizeError);
}

TEST_CASE("Comment::replaceData", "[comment]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test");
  REQUIRE(comment->data() == "test");
  comment->replaceData(1, 2, "abc");
  REQUIRE(comment->data() == "tabct");
  comment->replaceData(5, 0, "123");
  REQUIRE(comment->data() == "tabct123");
  REQUIRE_THROWS_AS(comment->replaceData(comment->length() + 1, 0, "abc"), IndexSizeError);
}

TEST_CASE("Comment::deleteData", "[comment]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test123");
  REQUIRE(comment->data() == "test123");
  comment->deleteData(1, 1);
  REQUIRE(comment->data() == "tst123");
  comment->deleteData(0, 3);
  REQUIRE(comment->data() == "123");
  comment->deleteData(0, 0);
  REQUIRE(comment->data() == "123");
  REQUIRE_THROWS_AS(comment->deleteData(comment->length() + 1, 0), IndexSizeError);
}

TEST_CASE("Comment::previousElementSibling", "[comment]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto element = root_element->appendChild(document->createElement("test"));
  root_element->appendChild(document->createComment(""));
  auto comment = std::dynamic_pointer_cast<Comment>(root_element->appendChild(document->createComment("test")));
  auto const_comment = std::dynamic_pointer_cast<const Comment>(comment);
  REQUIRE(comment->previousElementSibling() == element);
  REQUIRE(const_comment->previousElementSibling() == element);
  root_element->removeChild(element);
  REQUIRE(comment->previousElementSibling() == nullptr);
  REQUIRE(const_comment->previousElementSibling() == nullptr);
}

TEST_CASE("Comment::nextElementSibling", "[comment]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto comment = std::dynamic_pointer_cast<Comment>(root_element->appendChild(document->createComment("test")));
  auto const_comment = std::dynamic_pointer_cast<const Comment>(comment);
  root_element->appendChild(document->createComment(""));
  auto element = root_element->appendChild(document->createElement("test"));
  REQUIRE(comment->nextElementSibling() == element);
  REQUIRE(const_comment->nextElementSibling() == element);
  root_element->removeChild(element);
  REQUIRE(comment->nextElementSibling() == nullptr);
  REQUIRE(const_comment->nextElementSibling() == nullptr);
}
