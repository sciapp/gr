#include <iostream>
#include "GR/Document.hxx"
#include "GR/Element.hxx"
#include "GR/Comment.hxx"
#include "GR/util.hxx"

int main()
{
  auto document = GR::createDocument();
  auto root = document->createElement("root");
  root->setAttribute("id", "demo");
  document->append(root);
  root->append(document->createComment("This is a demo"));
  std::cerr << GR::toXML(document) << std::endl;
  return 0;
}
