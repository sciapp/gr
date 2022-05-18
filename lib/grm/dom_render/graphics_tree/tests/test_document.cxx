#include <catch2/catch.hpp>
#include "Document.hxx"
#include "Element.hxx"
#include "Comment.hxx"
#include "NotSupportedError.hxx"
#include "HierarchyRequestError.hxx"

using GR::Comment;
using GR::Document;
using GR::Element;
using GR::HierarchyRequestError;
using GR::Node;
using GR::NotSupportedError;
using GR::Value;

TEST_CASE("createDocument", "[document]")
{
  auto document = GR::createDocument();
  REQUIRE(document != nullptr);
}

TEST_CASE("Document::createElement", "[document]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  REQUIRE(element->localName() == "element");
  REQUIRE(element->ownerDocument() == document);
}

TEST_CASE("Document::createComment", "[document]")
{
  auto document = Document::createDocument();
  auto comment = document->createComment("test");
  REQUIRE(comment->data() == "test");
  REQUIRE(comment->ownerDocument() == document);
}

TEST_CASE("Document::nodeName", "[document]")
{
  auto document = Document::createDocument();
  auto const_document = std::dynamic_pointer_cast<const Document>(document);
  REQUIRE(const_document->nodeName() == "#document");
}

TEST_CASE("Document::documentElement", "[document]")
{
  auto document = Document::createDocument();
  auto const_document = std::dynamic_pointer_cast<const Document>(document);
  auto comment = document->createComment("test");
  auto element = document->createElement("element");
  REQUIRE(document->documentElement() == nullptr);
  REQUIRE(const_document->documentElement() == nullptr);
  document->appendChild(comment);
  REQUIRE(document->documentElement() == nullptr);
  REQUIRE(const_document->documentElement() == nullptr);
  document->appendChild(element);
  REQUIRE(document->documentElement() == element);
  REQUIRE(const_document->documentElement() == element);
}

TEST_CASE("Document::getElementsByTagName", "[document]")
{
  auto document = Document::createDocument();
  auto const_document = std::dynamic_pointer_cast<const Document>(document);
  auto element1 = document->createElement("element");
  auto element2 = document->createElement("element");
  auto element3 = document->createElement("other");
  REQUIRE(document->getElementsByTagName("*").empty());
  REQUIRE(const_document->getElementsByTagName("*").empty());
  document->appendChild(element1);
  element1->appendChild(element2);
  element1->appendChild(element3);
  auto all_elements = document->getElementsByTagName("*");
  REQUIRE(all_elements.size() == 3);
  auto const_all_elements = const_document->getElementsByTagName("*");
  REQUIRE(const_all_elements.size() == 3);
  auto element_elements = document->getElementsByTagName("element");
  REQUIRE(element_elements.size() == 2);
  REQUIRE(std::find(element_elements.begin(), element_elements.end(), element1) != element_elements.end());
  REQUIRE(std::find(element_elements.begin(), element_elements.end(), element2) != element_elements.end());
  REQUIRE(std::find(element_elements.begin(), element_elements.end(), element3) == element_elements.end());
  auto const_element_elements = const_document->getElementsByTagName("element");
  REQUIRE(const_element_elements.size() == 2);
  REQUIRE(std::find(const_element_elements.begin(), const_element_elements.end(), element1) !=
          const_element_elements.end());
  REQUIRE(std::find(const_element_elements.begin(), const_element_elements.end(), element2) !=
          const_element_elements.end());
  REQUIRE(std::find(const_element_elements.begin(), const_element_elements.end(), element3) ==
          const_element_elements.end());
  auto other_elements = document->getElementsByTagName("other");
  REQUIRE(other_elements.size() == 1);
  REQUIRE(std::find(other_elements.begin(), other_elements.end(), element1) == other_elements.end());
  REQUIRE(std::find(other_elements.begin(), other_elements.end(), element2) == other_elements.end());
  REQUIRE(std::find(other_elements.begin(), other_elements.end(), element3) != other_elements.end());
  auto const_other_elements = const_document->getElementsByTagName("other");
  REQUIRE(const_other_elements.size() == 1);
  REQUIRE(std::find(const_other_elements.begin(), const_other_elements.end(), element1) == const_other_elements.end());
  REQUIRE(std::find(const_other_elements.begin(), const_other_elements.end(), element2) == const_other_elements.end());
  REQUIRE(std::find(const_other_elements.begin(), const_other_elements.end(), element3) != const_other_elements.end());
}

TEST_CASE("Document::getElementById", "[document]")
{
  auto document = Document::createDocument();
  auto const_document = std::dynamic_pointer_cast<const Document>(document);
  auto element = document->createElement("element");
  REQUIRE(document->getElementById("test") == nullptr);
  REQUIRE(const_document->getElementById("test") == nullptr);
  document->appendChild(element);
  REQUIRE(document->getElementById("test") == nullptr);
  REQUIRE(const_document->getElementById("test") == nullptr);
  element->setAttribute("id", "test");
  REQUIRE(document->getElementById("test") == element);
  REQUIRE(const_document->getElementById("test") == element);
}

TEST_CASE("Document::cloneNode", "[document]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  document->appendChild(element);
  auto clone_document = document->cloneNode(true);
  REQUIRE(!document->isSameNode(clone_document));
  REQUIRE(clone_document->hasChildNodes());
  REQUIRE(element->isEqualNode(clone_document->firstChild()));
  REQUIRE(!element->isSameNode(clone_document->firstChild()));
}

TEST_CASE("Document::adoptNode", "[document]")
{
  auto document = Document::createDocument();
  auto other_document = Document::createDocument();
  REQUIRE_THROWS_AS(document->adoptNode(other_document), NotSupportedError);
  auto element = other_document->createElement("test");
  other_document->appendChild(element);
  document->adoptNode(element);
  REQUIRE(element->ownerDocument() == document);
  REQUIRE(element->parentNode() == nullptr);
  REQUIRE(!other_document->hasChildNodes());
}

TEST_CASE("Document::importNode", "[document]")
{
  auto document = Document::createDocument();
  auto other_document = Document::createDocument();
  REQUIRE_THROWS_AS(document->adoptNode(other_document), NotSupportedError);
  auto element = other_document->createElement("test");
  other_document->appendChild(element);
  auto clone_element = document->importNode(element);
  REQUIRE(element->ownerDocument() == other_document);
  REQUIRE(element->parentNode() == other_document);
  REQUIRE(other_document->hasChildNodes());
  REQUIRE(clone_element->ownerDocument() == document);
  REQUIRE(element->isEqualNode(clone_element));
}

TEST_CASE("Document::children", "[document]")
{
  auto document = Document::createDocument();
  auto const_document = std::dynamic_pointer_cast<const Document>(document);
  REQUIRE(document->children().empty());
  REQUIRE(const_document->children().empty());
  auto element = document->createElement("test");
  REQUIRE(document->children().empty());
  REQUIRE(const_document->children().empty());
  document->appendChild(element);
  REQUIRE(document->children().size() == 1);
  REQUIRE(const_document->children().size() == 1);
  REQUIRE(document->children()[0] == element);
  REQUIRE(const_document->children()[0] == element);
}

TEST_CASE("Document::firstChildElement", "[document]")
{
  auto document = Document::createDocument();
  auto const_document = std::dynamic_pointer_cast<const Document>(document);
  REQUIRE(document->firstChildElement() == nullptr);
  REQUIRE(const_document->firstChildElement() == nullptr);
  auto element = document->createElement("test");
  REQUIRE(document->firstChildElement() == nullptr);
  REQUIRE(const_document->firstChildElement() == nullptr);
  document->appendChild(element);
  REQUIRE(document->firstChildElement() == element);
  REQUIRE(const_document->firstChildElement() == element);
}

TEST_CASE("Document::lastChildElement", "[document]")
{
  auto document = Document::createDocument();
  auto const_document = std::dynamic_pointer_cast<const Document>(document);
  REQUIRE(document->lastChildElement() == nullptr);
  REQUIRE(const_document->lastChildElement() == nullptr);
  auto element = document->createElement("test");
  REQUIRE(document->lastChildElement() == nullptr);
  REQUIRE(const_document->lastChildElement() == nullptr);
  document->appendChild(element);
  REQUIRE(document->lastChildElement() == element);
  REQUIRE(const_document->lastChildElement() == element);
}

TEST_CASE("Document::childElementCount", "[document]")
{
  auto document = Document::createDocument();
  auto const_document = std::dynamic_pointer_cast<const Document>(document);
  REQUIRE(document->childElementCount() == 0);
  REQUIRE(const_document->childElementCount() == 0);
  auto element = document->createElement("test");
  REQUIRE(document->childElementCount() == 0);
  REQUIRE(const_document->childElementCount() == 0);
  document->appendChild(element);
  REQUIRE(document->childElementCount() == 1);
  REQUIRE(const_document->childElementCount() == 1);
}

TEST_CASE("Document::append", "[document]")
{
  auto document = Document::createDocument();
  REQUIRE(document->childElementCount() == 0);
  document->append(document->createElement("element"));
  REQUIRE(document->childElementCount() == 1);
  REQUIRE_THROWS_AS(document->append(document->createElement("test1")), HierarchyRequestError);
  REQUIRE(document->childElementCount() == 1);
  REQUIRE(document->firstChildElement()->localName() == "element");
  REQUIRE(document->lastChildElement()->localName() == "element");
  REQUIRE(document->childNodes().size() == 1);
  document->append(document->createComment("test1"), document->createComment("test2"));
  REQUIRE(document->childNodes().size() == 3);
  REQUIRE(document->firstChild()->nodeName() == "ELEMENT");
  REQUIRE(document->lastChild()->nodeName() == "#comment");
  REQUIRE(std::dynamic_pointer_cast<Comment>(document->lastChild())->data() == "test2");
}

TEST_CASE("Document::prepend", "[document]")
{
  auto document = Document::createDocument();
  REQUIRE(document->childElementCount() == 0);
  document->prepend(document->createElement("element"));
  REQUIRE(document->childElementCount() == 1);
  REQUIRE_THROWS_AS(document->prepend(document->createElement("test1")), HierarchyRequestError);
  REQUIRE(document->childElementCount() == 1);
  REQUIRE(document->firstChildElement()->localName() == "element");
  REQUIRE(document->lastChildElement()->localName() == "element");
  REQUIRE(document->childNodes().size() == 1);
  document->prepend(document->createComment("test1"), document->createComment("test2"));
  REQUIRE(document->childNodes().size() == 3);
  REQUIRE(document->firstChild()->nodeName() == "#comment");
  REQUIRE(document->lastChild()->nodeName() == "ELEMENT");
  REQUIRE(std::dynamic_pointer_cast<Comment>(document->firstChild())->data() == "test1");
}

TEST_CASE("Document::replaceChildren", "[document]")
{
  auto document = Document::createDocument();
  REQUIRE(document->childElementCount() == 0);
  document->replaceChildren(document->createElement("element"));
  REQUIRE(document->childElementCount() == 1);
  document->replaceChildren(document->createElement("test1"), document->createComment("test2"));
  REQUIRE(document->childElementCount() == 1);
  REQUIRE(document->firstChildElement()->localName() == "test1");
  REQUIRE(document->lastChildElement()->localName() == "test1");
  REQUIRE(document->childNodes().size() == 2);
}

TEST_CASE("Document::getElementsByClassName", "[document]")
{
  auto document = Document::createDocument();
  auto const_document = std::dynamic_pointer_cast<const Document>(document);
  auto root_element = document->createElement("element");
  auto child_element = document->createElement("element");
  document->appendChild(root_element);
  root_element->appendChild(child_element);
  REQUIRE(document->getElementsByClassName("testA").empty());
  child_element->setAttribute("class", "testa testb");
  REQUIRE(document->getElementsByClassName("testA").size() == 1);
  REQUIRE(document->getElementsByClassName("testA")[0] == child_element);
  REQUIRE(document->getElementsByClassName("testc testa").empty());
  REQUIRE(document->getElementsByClassName("testb testa").size() == 1);
  REQUIRE(document->getElementsByClassName("testb testa")[0] == child_element);
  REQUIRE(const_document->getElementsByClassName("testb testa").size() == 1);
  REQUIRE(const_document->getElementsByClassName("testb testa")[0] == child_element);
}

TEST_CASE("Document::querySelectors", "[document]")
{
  auto document = Document::createDocument();
  REQUIRE(document->querySelectors("*") == nullptr);
  auto root_element = document->createElement("root_element");
  document->appendChild(root_element);
  auto tag_element = root_element->appendChild(document->createElement("example_tag"));
  auto child_tag_element =
      std::dynamic_pointer_cast<Element>(tag_element->appendChild(document->createElement("example_tag")));
  child_tag_element->setAttribute("id", "example");
  child_tag_element->setAttribute("class", "testa testb testc-example");
  auto other_child_tag_element =
      std::dynamic_pointer_cast<Element>(tag_element->appendChild(document->createElement("example_tag")));
  other_child_tag_element->setAttribute("class", "testb testc");
  std::string selectors[] = {
      "example_tag",
      "#example",
      ".testa",
      "[class|=\"testc\"]",
      "[class~=\"testc\"]",
      "example_tag #example",
      "example_tag > * ~ #example",
  };
  for (const auto &selector : selectors)
    {
      REQUIRE(document->querySelectors(selector) == root_element->querySelectors(selector));
    }
}

TEST_CASE("Document::querySelectorsAll", "[document]")
{
  auto document = Document::createDocument();
  REQUIRE(document->querySelectorsAll("*").empty());
  auto root_element = document->createElement("root_element");
  document->appendChild(root_element);
  auto tag_element = root_element->appendChild(document->createElement("example_tag"));
  auto child_tag_element =
      std::dynamic_pointer_cast<Element>(tag_element->appendChild(document->createElement("example_tag")));
  child_tag_element->setAttribute("id", "example");
  child_tag_element->setAttribute("class", "testa testb testc-example");
  auto other_child_tag_element =
      std::dynamic_pointer_cast<Element>(tag_element->appendChild(document->createElement("example_tag")));
  other_child_tag_element->setAttribute("class", "testb testc");
  std::string selectors[] = {
      "example_tag",
      "#example",
      ".testa",
      "[class|=\"testc\"]",
      "[class~=\"testc\"]",
      "example_tag #example",
      "example_tag > * ~ #example",
  };
  for (const auto &selector : selectors)
    {
      auto document_results = document->querySelectorsAll(selector);
      auto element_results = root_element->querySelectorsAll(selector);
      REQUIRE(document_results.size() == element_results.size());
      for (decltype(document_results)::size_type i = 0; i < document_results.size(); ++i)
        {
          REQUIRE(document_results[i] == element_results[i]);
        }
    }
}
