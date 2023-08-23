#include <iostream>
#include <grm/dom_render/graphics_tree/Document.hxx>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/graphics_tree/Comment.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>

int main()
{
  auto document = GRM::createDocument();
  auto root = document->createElement("root");
  root->setAttribute("id", "demo");
  document->append(root);
  root->append(document->createComment("This is a demo"));
  std::cerr << GRM::toXML(document) << std::endl;
  return 0;
}
