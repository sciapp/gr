#include "include/render.hxx"
#include "gr.h"
#include <vector>


void simple()
{
  /*!
   * Simple polymarker example
   */
  gr_initgr();
  auto doc = Render::createRender();
  std::shared_ptr<Element> root = doc->createElement("root");

  doc->append(root);

  std::vector<double> x = {0.1, 0.2, 0.8};
  std::vector<double> y = {0.9, 0.2, 0.8};

  auto element = doc->createPolymarker(3, x, y);
  root->append(element);
  doc->render();

  std::cout << "Simple example enter to stop\n";
  std::cin.get();
}

void markerlist()
{
  gr_initgr();
  auto doc = Render::createRender();
  std::shared_ptr<Element> root = doc->createElement("root");

  doc->append(root);

  std::vector<double> x = {0.1, 0.2, 0.8};
  std::vector<double> y = {0.9, 0.2, 0.8};
  std::vector<int> markertype = {2, -32, -16};
  std::vector<double> markersizes = {0.4, 2.3, 8.0};
  std::vector<int> colorinds = {2, 17, 323};

  auto element = doc->createPolymarker(3, x, y);
  doc->setMarkerType(element, markertype);
  doc->setMarkerSize(element, markersizes);
  doc->setMarkerColorInd(element, colorinds);

  root->append(element);

  doc->render();

  std::cout << "\nSimple example enter to stop\n";
  std::cin.get();
}

void markerWindow()
{
  gr_initgr();
  auto doc = Render::createRender();
  std::shared_ptr<Element> root = doc->createElement("root");

  doc->append(root);

  std::vector<double> x = {0.1, 0.2, 0.8};
  std::vector<double> y = {0.9, 0.2, 0.8};

  auto element = doc->createPolymarker(3, x, y);
  element->setAttribute("window", "");
  element->setAttribute("window_xmin", 0.3);
  element->setAttribute("window_xmax", 0.8);
  element->setAttribute("window_ymin", 0.0);
  element->setAttribute("window_ymax", 1.0);

  root->append(element);

  doc->render();

  std::cout << "\nSimple example enter to stop\n";
  std::cin.get();
}

void polylinelist()
{
  gr_initgr();
  auto doc = Render::createRender();
  std::shared_ptr<Element> root = doc->createElement("root");

  doc->append(root);

  std::vector<double> x = {0.1, 0.2, 0.8};
  std::vector<double> y = {0.9, 0.2, 0.8};
  std::vector<int> linetypes = {2, 3};
  std::vector<double> widths = {2.0, 13.4};
  std::vector<int> colorinds = {42, 3};


  auto element = doc->createPolyline(3, x, y);
  doc->setLineType(element, linetypes);
  doc->setLineWidth(element, widths);
  doc->setLineColorInd(element, colorinds);

  root->append(element);

  doc->render();

  std::cout << "\nSimple example enter to stop\n";
  std::cin.get();
}


void textExample()
{
  gr_initgr();
  auto doc = Render::createRender();
  std::shared_ptr<Element> root = doc->createElement("root");

  doc->append(root);

  auto element = doc->createText(0.5, 0.5, "Hello World");

  Render::setTextFontPrec(element, 232, 3);

  root->append(element);
  doc->render();
  std::cout << "\nSimple text example";
  std::cin.get();
}

int main()
{
  simple();
  //    textExample();
  //    markerlist();
  //    polylinelist();
  //    markerWindow();
}
