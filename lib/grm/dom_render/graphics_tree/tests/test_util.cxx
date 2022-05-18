#include <catch2/catch.hpp>
#include "util.hxx"
#include "Document.hxx"
#include "Element.hxx"
#include "Comment.hxx"
#include "TypeError.hxx"

using GR::Comment;
using GR::Document;
using GR::Element;
using GR::Node;
using GR::TypeError;
using GR::Value;

TEST_CASE("toXML", "[util]")
{
  auto document = Document::createDocument();
  document->appendChild(document->createComment("comment 1"));
  auto root_element = document->createElement("root");
  root_element->setAttribute("id", "root");
  root_element->setAttribute("int-attr", 1);
  root_element->setAttribute("double-attr", 1.0);
  document->appendChild(root_element);
  root_element->appendChild(document->createComment("comment 2"));
  document->appendChild(document->createComment("comment 4"));
  root_element->appendChild(document->createElement("child"));
  root_element->firstChildElement()->setAttribute("id", "child");
  document->getElementById("child")->appendChild(document->createComment("comment 3"));
  REQUIRE(GR::toXML(document) == ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                  "<!--comment 1-->\n"
                                  "<root double-attr=\"1.000000\" id=\"root\" int-attr=\"1\">\n"
                                  "<!--comment 2-->\n"
                                  "<child id=\"child\">\n"
                                  "<!--comment 3-->\n"
                                  "</child>\n"
                                  "</root>\n"
                                  "<!--comment 4-->\n"));
  REQUIRE(GR::toXML(document, {"\t"}) == ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                          "<!--comment 1-->\n"
                                          "<root double-attr=\"1.000000\" id=\"root\" int-attr=\"1\">\n"
                                          "\t<!--comment 2-->\n"
                                          "\t<child id=\"child\">\n"
                                          "\t\t<!--comment 3-->\n"
                                          "\t</child>\n"
                                          "</root>\n"
                                          "<!--comment 4-->\n"));
  REQUIRE_THROWS_AS(GR::toXML(nullptr), TypeError);
}

TEST_CASE("tolower", "[util]")
{
  REQUIRE(GR::tolower("test") == "test");
  REQUIRE(GR::tolower("Test") == "test");
  REQUIRE(GR::tolower("tEST") == "test");
  REQUIRE(GR::tolower("ABC123?") == "abc123?");
}

TEST_CASE("toupper", "[util]")
{
  REQUIRE(GR::toupper("TEST") == "TEST");
  REQUIRE(GR::toupper("Test") == "TEST");
  REQUIRE(GR::toupper("tEST") == "TEST");
  REQUIRE(GR::toupper("abc123?") == "ABC123?");
}

TEST_CASE("split", "[util]")
{
  REQUIRE(GR::split("", "").empty());
  REQUIRE(GR::split("abc", "").size() == 3);
  REQUIRE(GR::split("abc", "")[0] == "a");
  REQUIRE(GR::split("abc", "")[1] == "b");
  REQUIRE(GR::split("abc", "")[2] == "c");
  REQUIRE(GR::split("abc", "123").size() == 1);
  REQUIRE(GR::split("abc", "123")[0] == "abc");
  REQUIRE(GR::split("abc 123", "123").size() == 2);
  REQUIRE(GR::split("abc 123", "123")[0] == "abc ");
  REQUIRE(GR::split("abc 123", "123")[1] == "");
  REQUIRE(GR::split("abc,123,", ",").size() == 3);
  REQUIRE(GR::split("abc,123,", ",")[0] == "abc");
  REQUIRE(GR::split("abc,123,", ",")[1] == "123");
  REQUIRE(GR::split("abc,123,", ",")[2] == "");
}

TEST_CASE("strip", "[util]")
{
  REQUIRE(GR::strip("") == "");
  REQUIRE(GR::strip(" \t\n") == "");
  REQUIRE(GR::strip("test") == "test");
  REQUIRE(GR::strip("  test   ") == "test");
  REQUIRE(GR::strip("\t\ntest\t\n") == "test");
}
