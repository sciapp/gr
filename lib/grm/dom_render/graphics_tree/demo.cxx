#include <iostream>
#include "Document.hxx"
#include "Element.hxx"
#include "Comment.hxx"
#include "util.hxx"

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
