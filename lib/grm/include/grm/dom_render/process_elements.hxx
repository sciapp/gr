#ifndef GR_PROCESS_ELEMENTS_HXX
#define GR_PROCESS_ELEMENTS_HXX

#include <grm/dom_render/graphics_tree/element.hxx>
#include <grm/dom_render/graphics_tree/document.hxx>
#include <grm/dom_render/context.hxx>

/* ------------------------------- process high lvl elements ---------------------------------------------------------*/

void processElement(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processSeries(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPlot(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);

/* ------------------------------- pre process elements --------------------------------------------------------------*/

void preBarplot(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void prePolarHistogram(const std::shared_ptr<GRM::Element> &plot_elem, const std::shared_ptr<GRM::Context> &context);

/* ------------------------------- process low lvl elements ----------------------------------------------------------*/

void processAxis(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processRadialAxes(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processAngleLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processThetaAxes(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processDrawRect(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processFillArc(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processFillRect(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processFillArea(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processGrid3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processGridLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processIntegral(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processIntegralGroup(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processArcGridLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processAxes3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processCellArray(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processColorbar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processDrawArc(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processDrawGraphics(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processDrawImage(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processErrorBars(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processErrorBar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processLegend(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processBar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processLayoutGrid(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processNonUniformPolarCellArray(const std::shared_ptr<GRM::Element> &element,
                                     const std::shared_ptr<GRM::Context> &context);
void processNonuniformCellArray(const std::shared_ptr<GRM::Element> &element,
                                const std::shared_ptr<GRM::Context> &context);
void processOverlayElement(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPanzoom(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPolarCellArray(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPolyline(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPolyline3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPolymarker(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPolymarker3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processSidePlotRegion(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processCoordinateSystem(const std::shared_ptr<GRM::Element> &element,
                             const std::shared_ptr<GRM::Context> &context);
void processSideRegion(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPolarBar(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPieSegment(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processText(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processTextRegion(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processTickGroup(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processTick(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processTitles3d(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);

/* ------------------------------- process series elements -----------------------------------------------------------*/

void processHeatmap(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processHexbin(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processBarplot(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processContour(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processContourf(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processIsosurface(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processHistogram(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processQuiver(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPolarLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPolarScatter(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPolarHeatmap(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processWireframe(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processPolarHistogram(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processScatter(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processScatter3(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processStairs(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processStem(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processShade(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processSurface(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processLine(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processMarginalHeatmapPlot(const std::shared_ptr<GRM::Element> &element,
                                const std::shared_ptr<GRM::Context> &context);
void processPie(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processLine3(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processImshow(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processTriContour(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processTriSurface(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);
void processVolume(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context);

#endif // GR_PROCESS_ELEMENTS_HXX
