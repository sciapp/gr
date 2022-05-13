#include <catch2/catch.hpp>
#include "Element.hxx"
#include "Document.hxx"
#include "Comment.hxx"
#include "HierarchyRequestError.hxx"

using GR::Comment;
using GR::Document;
using GR::Element;
using GR::HierarchyRequestError;
using GR::Node;
using GR::Value;

TEST_CASE("Element::nodeName", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  REQUIRE(element->nodeName() == element->tagName());
}

TEST_CASE("Element::tagName", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  REQUIRE(element->tagName() == "ELEMENT");
}

TEST_CASE("Element::localName", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("ELEMENT");
  REQUIRE(element->localName() == "element");
}

TEST_CASE("Element::id", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  element->setAttribute("id", "test");
  REQUIRE(element->id() == "test");
}

TEST_CASE("Element::hasAttributes", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  REQUIRE(!element->hasAttributes());
  element->setAttribute("test", "");
  REQUIRE(element->hasAttributes());
}

TEST_CASE("Element::getAttributeNames", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  element->setAttribute("a", "1");
  element->setAttribute("b", "2");
  std::unordered_set<std::string> result = {"a", "b"};
  REQUIRE(element->getAttributeNames() == result);
}


TEST_CASE("Element::getAttribute", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  element->setAttribute("name", "value");
  REQUIRE(element->getAttribute("name").isString());
  REQUIRE((std::string)element->getAttribute("name") == "value");
}

TEST_CASE("Element::setAttribute", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  element->setAttribute("name", "value");
  REQUIRE(element->getAttribute("name").isString());
  REQUIRE((std::string)element->getAttribute("name") == "value");
}

TEST_CASE("Element::removeAttribute", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  element->setAttribute("name", "value");
  element->removeAttribute("name");
  REQUIRE(!element->hasAttribute("name"));
  REQUIRE(!element->hasAttributes());
}

TEST_CASE("Element::toggleAttribute", "[element]")
{
  auto document = Document::createDocument();
  auto element = document->createElement("element");
  REQUIRE(!element->hasAttribute("name"));
  REQUIRE(element->toggleAttribute("name"));
  REQUIRE(element->hasAttribute("name"));
  REQUIRE((std::string)element->getAttribute("name") == "");
  REQUIRE(!element->toggleAttribute("name"));
  REQUIRE(!element->hasAttribute("name"));
  REQUIRE(!element->toggleAttribute("name", false));
  REQUIRE(!element->hasAttribute("name"));
  element->setAttribute("name", "test");
  REQUIRE(!element->toggleAttribute("name", false));
  REQUIRE(!element->hasAttribute("name"));
  REQUIRE(element->toggleAttribute("name", true));
  REQUIRE((std::string)element->getAttribute("name") == "");
  element->setAttribute("name", "test");
  REQUIRE(element->toggleAttribute("name", true));
  REQUIRE((std::string)element->getAttribute("name") == "test");
  REQUIRE(element->toggleAttribute("name", true));
  REQUIRE(element->hasAttribute("name"));
}

TEST_CASE("Element::getElementsByTagName", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto child_element = document->createElement("element");
  child_element->appendChild(document->createElement("element"));
  root_element->appendChild(child_element);
  root_element->appendChild(document->createElement("element"));
  child_element->appendChild(document->createElement("element"));
  child_element->appendChild(document->createElement("x"));
  root_element->appendChild(document->createElement("x"));
  root_element->appendChild(document->createElement("test_node"));
  child_element->appendChild(document->createElement("test_node"));
  REQUIRE(root_element->getElementsByTagName("element").size() == 4);
  REQUIRE(root_element->getElementsByTagName("x").size() == 2);
  child_element->remove();
  REQUIRE(root_element->getElementsByTagName("element").size() == 1);
}

TEST_CASE("Element::replaceWith", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto child_element = document->createElement("element");
  auto other_child_element = document->createElement("element");
  root_element->appendChild(child_element);
  REQUIRE(child_element->parentNode() == root_element);
  REQUIRE(root_element->hasChildNodes());
  REQUIRE(root_element->contains(child_element));
  child_element->replaceWith(other_child_element);
  REQUIRE(other_child_element->parentNode() == root_element);
  REQUIRE(root_element->hasChildNodes());
  REQUIRE(root_element->contains(other_child_element));
  REQUIRE_THROWS_AS(root_element->replaceWith(other_child_element), HierarchyRequestError);
}

TEST_CASE("Element::remove", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto child_element = document->createElement("element");
  root_element->appendChild(child_element);
  REQUIRE(child_element->parentNode() == root_element);
  REQUIRE(root_element->hasChildNodes());
  child_element->remove();
  REQUIRE(!child_element->parentNode());
  REQUIRE(!root_element->hasChildNodes());
  REQUIRE_THROWS_AS(root_element->remove(), HierarchyRequestError);
}

TEST_CASE("Element::prepend", "[Element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto child_node1 = document->createElement("test_node");
  auto child_node2 = document->createElement("test_node");
  auto child_node3 = document->createElement("test_node");
  root_element->prepend(child_node1);
  REQUIRE(root_element->firstChild() == child_node1);
  REQUIRE(root_element->lastChild() == child_node1);
  root_element->prepend(child_node2, child_node3);
  REQUIRE(root_element->firstChild() == child_node2);
  REQUIRE(child_node2->nextSibling() == child_node3);
  REQUIRE(child_node3->nextSibling() == child_node1);
  REQUIRE(root_element->lastChild() == child_node1);
}

TEST_CASE("Element::append", "[Element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto child_node1 = document->createElement("test_node");
  auto child_node2 = document->createElement("test_node");
  auto child_node3 = document->createElement("test_node");
  root_element->append(child_node1);
  REQUIRE(root_element->firstChild() == child_node1);
  REQUIRE(root_element->lastChild() == child_node1);
  root_element->append(child_node2, child_node3);
  REQUIRE(root_element->firstChild() == child_node1);
  REQUIRE(child_node1->nextSibling() == child_node2);
  REQUIRE(child_node2->nextSibling() == child_node3);
  REQUIRE(root_element->lastChild() == child_node3);
}

TEST_CASE("Element::replaceChildren", "[Element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto child_node1a = document->createElement("test_node");
  auto child_node2a = document->createElement("test_node");
  auto child_node1b = document->createElement("test_node");
  auto child_node2b = document->createElement("test_node");
  auto child_node3b = document->createElement("test_node");
  root_element->append(child_node1a, child_node2a);
  REQUIRE(root_element->firstChild() == child_node1a);
  REQUIRE(child_node1a->nextSibling() == child_node2a);
  REQUIRE(child_node2a->nextSibling() == nullptr);
  REQUIRE(root_element->lastChild() == child_node2a);
  root_element->replaceChildren(child_node1b, child_node2b, child_node3b);
  REQUIRE(root_element->firstChild() == child_node1b);
  REQUIRE(child_node1b->nextSibling() == child_node2b);
  REQUIRE(child_node2b->nextSibling() == child_node3b);
  REQUIRE(child_node3b->nextSibling() == nullptr);
  REQUIRE(root_element->lastChild() == child_node3b);
  REQUIRE(child_node1a->parentNode() == nullptr);
  REQUIRE(child_node2a->parentNode() == nullptr);
}

TEST_CASE("Element::cloneNode", "[Element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  root_element->append(document->createElement("test_node1"), document->createElement("test_node2"));

  auto root_clone = root_element->cloneNode(true);
  REQUIRE(root_clone->isEqualNode(root_element));
}

TEST_CASE("Element::isEqualNode", "[Element]")
{
  auto document = Document::createDocument();
  auto element1 = document->createElement("test");
  REQUIRE(!element1->isEqualNode(document->createComment("test")));
  REQUIRE(!element1->isEqualNode(document->createElement("test2")));
  auto element2 = document->createElement("test");
  REQUIRE(element1->isEqualNode(element2));
  element1->setAttribute("test", "1");
  REQUIRE(!element1->isEqualNode(element2));
  element2->setAttribute("test", 1);
  REQUIRE(!element1->isEqualNode(element2));
  element2->setAttribute("test", "1");
  REQUIRE(element1->isEqualNode(element2));
  element2->setAttribute("test2", 1.0);
  REQUIRE(!element1->isEqualNode(element2));
  element1->setAttribute("test3", 1);
  REQUIRE(!element1->isEqualNode(element2));
  element2->setAttribute("test3", 1);
  element1->setAttribute("test2", 1.0);
  REQUIRE(element1->isEqualNode(element2));
  element1->append(document->createElement("test"));
  element1->append(document->createComment("test"));
  REQUIRE(!element1->isEqualNode(element2));
  element2->append(document->createComment("test"));
  element2->append(document->createComment("test"));
  REQUIRE(!element1->isEqualNode(element2));
  element2->replaceChildren(document->createElement("test"));
  REQUIRE(!element1->isEqualNode(element2));
  element2->append(document->createComment("test"));
  REQUIRE(element1->isEqualNode(element2));
}

TEST_CASE("Element::after", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("root");
  auto element1 = document->createElement("element");
  auto element2 = document->createElement("test");
  root_element->append(element1);
  element1->after(element2);
  REQUIRE(root_element->firstChildElement() == element1);
  REQUIRE(root_element->lastChildElement() == element2);
  element1->after(element2);
  REQUIRE(root_element->firstChildElement() == element1);
  REQUIRE(root_element->lastChildElement() == element2);
  REQUIRE_THROWS_AS(root_element->after(element1), HierarchyRequestError);
}

TEST_CASE("Element::before", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("root");
  auto element1 = document->createElement("element");
  auto element2 = document->createElement("test");
  root_element->append(element1);
  element1->before(element2);
  REQUIRE(root_element->firstChildElement() == element2);
  REQUIRE(root_element->lastChildElement() == element1);
  REQUIRE_THROWS_AS(root_element->before(element1), HierarchyRequestError);
}


TEST_CASE("Element::children", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("root");
  auto const_root_element = std::dynamic_pointer_cast<const Element>(root_element);
  REQUIRE(root_element->children().empty());
  REQUIRE(const_root_element->children().empty());
  auto element = document->createElement("test");
  REQUIRE(root_element->children().empty());
  REQUIRE(const_root_element->children().empty());
  root_element->appendChild(element);
  REQUIRE(root_element->children().size() == 1);
  REQUIRE(const_root_element->children().size() == 1);
  REQUIRE(root_element->children()[0] == element);
  REQUIRE(const_root_element->children()[0] == element);
}

TEST_CASE("Element::firstChildElement", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("root");
  auto const_root_element = std::dynamic_pointer_cast<const Element>(root_element);
  REQUIRE(root_element->firstChildElement() == nullptr);
  REQUIRE(const_root_element->firstChildElement() == nullptr);
  auto element = document->createElement("test");
  REQUIRE(root_element->firstChildElement() == nullptr);
  REQUIRE(const_root_element->firstChildElement() == nullptr);
  root_element->appendChild(element);
  REQUIRE(root_element->firstChildElement() == element);
  REQUIRE(const_root_element->firstChildElement() == element);
}

TEST_CASE("Element::lastChildElement", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("root");
  auto const_root_element = std::dynamic_pointer_cast<const Element>(root_element);
  REQUIRE(root_element->lastChildElement() == nullptr);
  REQUIRE(const_root_element->lastChildElement() == nullptr);
  auto element = document->createElement("test");
  REQUIRE(root_element->lastChildElement() == nullptr);
  REQUIRE(const_root_element->lastChildElement() == nullptr);
  root_element->appendChild(element);
  REQUIRE(root_element->lastChildElement() == element);
  REQUIRE(const_root_element->lastChildElement() == element);
}

TEST_CASE("Element::childElementCount", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("root");
  auto const_root_element = std::dynamic_pointer_cast<const Element>(root_element);
  REQUIRE(root_element->childElementCount() == 0);
  REQUIRE(const_root_element->childElementCount() == 0);
  auto element = document->createElement("test");
  REQUIRE(root_element->childElementCount() == 0);
  REQUIRE(const_root_element->childElementCount() == 0);
  root_element->appendChild(element);
  REQUIRE(root_element->childElementCount() == 1);
  REQUIRE(const_root_element->childElementCount() == 1);
}

TEST_CASE("Element::getElementsByClassName", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto const_root_element = std::dynamic_pointer_cast<const Element>(root_element);
  auto child_element = document->createElement("element");
  root_element->appendChild(child_element);
  REQUIRE(root_element->getElementsByClassName("testA").empty());
  child_element->setAttribute("class", "testa testb");
  REQUIRE(root_element->getElementsByClassName("testA").size() == 1);
  REQUIRE(root_element->getElementsByClassName("testA")[0] == child_element);
  root_element->setAttribute("class", "testa");
  REQUIRE(root_element->getElementsByClassName("testa").size() == 1);
  REQUIRE(root_element->getElementsByClassName("testa")[0] == child_element);
  REQUIRE(root_element->getElementsByClassName("testc testa").empty());
  REQUIRE(root_element->getElementsByClassName("testb testa").size() == 1);
  REQUIRE(root_element->getElementsByClassName("testb testa")[0] == child_element);
  REQUIRE(const_root_element->getElementsByClassName("testb testa").size() == 1);
  REQUIRE(const_root_element->getElementsByClassName("testb testa")[0] == child_element);
}

TEST_CASE("Element::previousElementSibling", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto element1 = root_element->appendChild(document->createElement("test"));
  root_element->appendChild(document->createComment(""));
  auto element2 = std::dynamic_pointer_cast<Element>(root_element->appendChild(document->createElement("test")));
  auto const_element2 = std::dynamic_pointer_cast<const Element>(element2);
  REQUIRE(element2->previousElementSibling() == element1);
  REQUIRE(const_element2->previousElementSibling() == element1);
  root_element->removeChild(element1);
  REQUIRE(element2->previousElementSibling() == nullptr);
  REQUIRE(const_element2->previousElementSibling() == nullptr);
}

TEST_CASE("Element::nextElementSibling", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("element");
  auto element1 = std::dynamic_pointer_cast<Element>(root_element->appendChild(document->createElement("test")));
  auto const_element1 = std::dynamic_pointer_cast<const Element>(element1);
  root_element->appendChild(document->createComment(""));
  auto element2 = root_element->appendChild(document->createElement("test"));
  REQUIRE(element1->nextElementSibling() == element2);
  REQUIRE(const_element1->nextElementSibling() == element2);
  root_element->removeChild(element2);
  REQUIRE(element1->nextElementSibling() == nullptr);
  REQUIRE(const_element1->nextElementSibling() == nullptr);
}

TEST_CASE("Element::querySelectors", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("root_element");
  REQUIRE(root_element->querySelectors("example_tag") == nullptr);
  auto tag_element = root_element->appendChild(document->createElement("example_tag"));
  REQUIRE(root_element->querySelectors("example_tag") == tag_element);
  REQUIRE(root_element->querySelectors("root_element example_tag") == tag_element);
  REQUIRE(root_element->querySelectors("root_element > example_tag") == tag_element);
  auto child_tag_element =
      std::dynamic_pointer_cast<Element>(tag_element->appendChild(document->createElement("example_tag")));
  REQUIRE(root_element->querySelectors("example_tag") == tag_element);
  REQUIRE(root_element->querySelectors("example_tag example_tag") == child_tag_element);
  REQUIRE(root_element->querySelectors("example_tag > example_tag") == child_tag_element);
  auto other_child_tag_element =
      std::dynamic_pointer_cast<Element>(tag_element->appendChild(document->createElement("example_tag")));
  REQUIRE(root_element->querySelectors("example_tag") == tag_element);
  REQUIRE(root_element->querySelectors("example_tag example_tag") == child_tag_element);
  REQUIRE(root_element->querySelectors("example_tag > example_tag") == child_tag_element);
  REQUIRE(root_element->querySelectors("example_tag ~ *") == other_child_tag_element);
  REQUIRE(root_element->querySelectors("#example") == nullptr);
  other_child_tag_element->setAttribute("id", "example");
  REQUIRE(root_element->querySelectors("#example") == other_child_tag_element);
  child_tag_element->setAttribute("class", "testa");
  REQUIRE(root_element->querySelectors(".testa") == child_tag_element);
  other_child_tag_element->setAttribute("class", "testa testb testc-example");
  REQUIRE(root_element->querySelectors(".testa") == child_tag_element);
  REQUIRE(root_element->querySelectors("example_tag .testa.testb") == other_child_tag_element);
  REQUIRE(root_element->querySelectors("example_tag.testa.testb#example") == other_child_tag_element);
  REQUIRE(root_element->querySelectors("[class]") == child_tag_element);
  REQUIRE(root_element->querySelectors("[id]") == other_child_tag_element);
  child_tag_element->setAttribute("id", "test");
  REQUIRE(root_element->querySelectors("[id]") == child_tag_element);
  REQUIRE(root_element->querySelectors("[id=\"example\"]") == other_child_tag_element);
  REQUIRE(root_element->querySelectors("[id=\"example2\"]") == nullptr);
  REQUIRE(root_element->querySelectors("[id*=\"xampl\"]") == other_child_tag_element);
  REQUIRE(root_element->querySelectors("[id*=\"xample2\"]") == nullptr);
  REQUIRE(root_element->querySelectors("[id^=\"exampl\"]") == other_child_tag_element);
  REQUIRE(root_element->querySelectors("[id^=\"texampl\"]") == nullptr);
  REQUIRE(root_element->querySelectors("[id$=\"ample\"]") == other_child_tag_element);
  REQUIRE(root_element->querySelectors("[id$=\"ample2\"]") == nullptr);
  REQUIRE(root_element->querySelectors("[class~=\"testa\"]") == child_tag_element);
  REQUIRE(root_element->querySelectors("[class~=\"testb\"]") == other_child_tag_element);
  REQUIRE(root_element->querySelectors("[class~=\"testc\"]") == nullptr);
  REQUIRE(root_element->querySelectors("[class|=\"testa\"]") == child_tag_element);
  REQUIRE(root_element->querySelectors("[class|=\"testb\"]") == other_child_tag_element);
  REQUIRE(root_element->querySelectors("[class|=\"testc\"]") == other_child_tag_element);
  REQUIRE(root_element->querySelectors("[class|=\"testd\"]") == nullptr);
  REQUIRE(root_element->querySelectors(":root") == root_element);
}

TEST_CASE("Element::querySelectorsAll", "[element]")
{
  auto document = Document::createDocument();
  auto root_element = document->createElement("root_element");
  REQUIRE(root_element->querySelectorsAll("example_tag").empty());
  auto tag_element = root_element->appendChild(document->createElement("example_tag"));
  REQUIRE(root_element->querySelectorsAll("example_tag").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag")[0] == tag_element);
  REQUIRE(root_element->querySelectorsAll("root_element example_tag").size() == 1);
  REQUIRE(root_element->querySelectorsAll("root_element example_tag")[0] == tag_element);
  REQUIRE(root_element->querySelectorsAll("root_element > example_tag").size() == 1);
  REQUIRE(root_element->querySelectorsAll("root_element > example_tag")[0] == tag_element);
  auto child_tag_element =
      std::dynamic_pointer_cast<Element>(tag_element->appendChild(document->createElement("example_tag")));
  REQUIRE(root_element->querySelectorsAll("example_tag").size() == 2);
  REQUIRE(root_element->querySelectorsAll("example_tag")[0] == tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag")[1] == child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag example_tag").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag example_tag")[0] == child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag > example_tag").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag > example_tag")[0] == child_tag_element);
  auto other_child_tag_element =
      std::dynamic_pointer_cast<Element>(tag_element->appendChild(document->createElement("example_tag")));
  REQUIRE(root_element->querySelectorsAll("example_tag").size() == 3);
  REQUIRE(root_element->querySelectorsAll("example_tag")[0] == tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag")[1] == child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag")[2] == other_child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag example_tag").size() == 2);
  REQUIRE(root_element->querySelectorsAll("example_tag example_tag")[0] == child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag example_tag")[1] == other_child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag > example_tag").size() == 2);
  REQUIRE(root_element->querySelectorsAll("example_tag > example_tag")[0] == child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag > example_tag")[1] == other_child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag ~ *").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag ~ *")[0] == other_child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag > :first-child").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag > :first-child")[0] == child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag > :last-child").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag > :last-child")[0] == other_child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag > :nth-child(even)").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag > :nth-child(even)")[0] == other_child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag > :nth-child(odd)").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag > :nth-child(odd)")[0] == child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag > :nth-child(2n1)").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag > :nth-child(2n1)")[0] == child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag > :nth-child(2)").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag > :nth-child(2)")[0] == other_child_tag_element);
  REQUIRE(root_element->querySelectorsAll("example_tag > :nth-last-child(2n0)").size() == 1);
  REQUIRE(root_element->querySelectorsAll("example_tag > :nth-last-child(2n0)")[0] == child_tag_element);
  REQUIRE(root_element->querySelectorsAll(":only-child").size() == 2);
  REQUIRE(root_element->querySelectorsAll(":only-child")[0] == root_element);
  REQUIRE(root_element->querySelectorsAll(":only-child")[1] == tag_element);
  REQUIRE(root_element->querySelectorsAll("element_tag:only-of-type").size() == 1);
  REQUIRE(root_element->querySelectorsAll("element_tag:only-of-type")[0] == tag_element);
  REQUIRE(root_element->querySelectorsAll(":empty").size() == 2);
  REQUIRE(root_element->querySelectorsAll(":empty")[0] == child_tag_element);
  REQUIRE(root_element->querySelectorsAll(":empty")[1] == other_child_tag_element);
  child_tag_element->append(document->createComment("test"));
  REQUIRE(root_element->querySelectorsAll(":empty").size() == 2);
}
