#include <vector>
#include <map>
#include <iostream>

#include "include/render.hxx"
#include "gr.h"
#include "context.hxx"

void testGetGetif()
{
  GR::Context c;
  std::vector<int> x = {1, 2, 3};

  c["x"] = x;

  const auto j = c["x"];
  auto x2 = GR::get<std::vector<int>>(j);

  std::vector<int> y = {9, 8, 7};
  c["y"] = y;

  auto y2 = GR::get<std::vector<int>>(c["y"]);
  std::cout << y2[0] << "\n";

  try
    {
      GR::get<std::vector<double>>(c["y"]);
    }
  catch (NotFoundError &e)
    {
      std::cout << e.what() << "\n";
    }

  auto in = c["y"];
  if (const std::vector<int> *x_ptr = GR::get_if<std::vector<int>>(in))
    {
      for (auto elem : *x_ptr)
        {
          std::cout << elem << ",\t";
        }
      std::cout << "\n";
    }

  if (const std::vector<int> *x_ptr = GR::get_if<std::vector<int>>(c["b"]))
    {
      for (auto elem : *x_ptr)
        {
          std::cout << elem << ",\t";
        }
    }
  else
    {
      std::cout << "nullptr\n";
    }
}

void testPolymarker()
{
  /*!
   * Simple polymarker example
   */
  gr_initgr();
  auto doc = GR::Render::createRender();
  std::shared_ptr<GR::Element> root = doc->createElement("root");

  doc->append(root);

  std::vector<double> x = {0.1, 0.2, 0.8};
  std::vector<double> y = {0.9, 0.2, 0.8};

  auto element = doc->createPolymarker(3, "x_key", x, "y_key", y);
  root->append(element);
  doc->render();

  std::cout << "Simple example enter to stop\n";
  std::cin.get();
}

void markerlist()
{
  gr_initgr();
  auto doc = GR::Render::createRender();
  std::shared_ptr<GR::Element> root = doc->createElement("root");

  doc->append(root);

  std::vector<double> x = {0.1, 0.2, 0.8};
  std::vector<double> y = {0.9, 0.2, 0.8};
  std::vector<int> markertypes = {2, -32, -16};
  std::vector<double> markersizes = {0.4, 2.3, 8.0};
  std::vector<int> colorinds = {2, 17, 323};

  auto element = doc->createPolymarker(3, "x_key", x, "y_key", y);
  doc->setMarkerType(element, "markertypes", markertypes);
  doc->setMarkerSize(element, "markersizes", markersizes);
  doc->setMarkerColorInd(element, "colorinds", colorinds);

  root->append(element);

  doc->render();

  std::cout << "\nSimple example enter to stop\n";
  std::cin.get();
}

void markerWindow()
{
  gr_initgr();
  auto doc = GR::Render::createRender();
  std::shared_ptr<GR::Element> root = doc->createElement("root");

  doc->append(root);

  std::vector<double> x = {0.1, 0.2, 0.8};
  std::vector<double> y = {0.9, 0.2, 0.8};

  auto element = doc->createPolymarker(3, "x_key", x, "y_key", y);
  element->setAttribute("window", "true");
  element->setAttribute("window_xmin", 0.2);
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
  auto doc = GR::Render::createRender();
  std::shared_ptr<GR::Element> root = doc->createElement("root");

  doc->append(root);

  std::vector<double> x = {0.1, 0.2, 0.8};
  std::vector<double> y = {0.9, 0.2, 0.8};
  std::vector<int> linetypes = {2, 3};
  std::vector<double> widths = {2.0, 13.4};
  std::vector<int> colorinds = {42, 3};


  auto element = doc->createPolyline(3, "x_key", x, "y_key", y);
  doc->setLineType(element, "linetypes", linetypes);
  doc->setLineWidth(element, "widths", widths);
  doc->setLineColorInd(element, "colorinds", colorinds);

  root->append(element);

  doc->render();

  std::cout << "\nSimple example enter to stop\n";
  std::cin.get();
}


void textExample()
{
  gr_initgr();
  auto doc = GR::Render::createRender();
  std::shared_ptr<GR::Element> root = doc->createElement("root");

  doc->append(root);

  auto element = doc->createText(0.5, 0.5, "Hello World");

  doc->setTextFontPrec(element, 232, 3);
  root->append(element);
  doc->render();
  std::cout << "\nSimple text example";
  std::cin.get();
}

void testPlotPolymarker()
{
  gr_initgr();
  auto doc = GR::Render::createRender();
  std::shared_ptr<GR::Element> root = doc->createElement("root");

  doc->append(root);

  auto element = doc->createAxes(0.1, 0.1, 0.0, 0.0, 5, 5, 0.01);
  root->append(element);
  std::vector<double> x = {0.1, 0.2, 0.8};

  element = doc->createPolymarker(3, "x_key", x, "y_key", x);
  root->firstChildElement()->append(element);

  doc->render();
  std::cout << "\n testPlotPolymarker";
  std::cin.get();
}

void testSubPlot()
{
  gr_initgr();
  auto doc = GR::Render::createRender();
  std::shared_ptr<GR::Element> root = doc->createElement("root");
  doc->append(root);

  std::vector<double> x = {0.1, 0.2, 0.8};
  auto element = doc->createPolymarker(3, "x_key", x, "y_key", x);
  doc->setViewport(element, 0.0, 0.5, 0.0, 1.0);
  root->append(element);

  auto group = doc->createGroup();
  doc->setViewport(group, 0.5, 1.0, 0.0, 1.0);

  //    std::vector<double> x2 = {0.1, 0.2, 0.8};
  element = doc->createPolymarker(3, "x_key", std::nullopt, "y_key", std::nullopt);
  group->append(element);
  element = doc->createPolyline(3, "x", x, "y", x);
  group->append(element);

  root->append(group);

  doc->render();
  std::cout << "\n testSubPlot";
  std::cin.get();
}


void testOnePlot()
{
  gr_initgr();
  auto doc = GR::Render::createRender();
  std::shared_ptr<GR::Element> root = doc->createElement("root");
  doc->append(root);

  std::vector<double> x = {0.1, 0.2, 0.8};
  std::vector<double> x2 = {0.1, 0.2, 0.8};

  auto element = doc->createPolymarker(3, "x_key2", x2, "y_key2", x2);
  root->append(element);
  element = doc->createPolyline(3, "x", x, "y", x);
  root->append(element);


  doc->render();
  std::cout << "\n testSubPlot";
  std::cin.get();
}

int main()
{
  //    testGetGetif();
  //    testPlotPolymarker();
  testSubPlot();
  //    testOnePlot();
  //    textExample();
  //    markerlist();
  //    polylinelist();
  //    markerWindow();
}
