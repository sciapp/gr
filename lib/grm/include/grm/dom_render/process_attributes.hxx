#ifndef GR_PROCESS_ATTRIBUTES_HXX
#define GR_PROCESS_ATTRIBUTES_HXX

#include <grm/dom_render/graphics_tree/element.hxx>
#include <grm/dom_render/graphics_tree/document.hxx>
#include <grm/dom_render/context.hxx>


/* ========================= functions ============================================================================== */

void processAttributes(const std::shared_ptr<GRM::Element> &element);
void calculateCentralRegionMarginOrDiagFactor(const std::shared_ptr<GRM::Element> &element, double *vp_x_min,
                                              double *vp_x_max, double *vp_y_min, double *vp_y_max,
                                              bool diag_factor = false);
void calculateViewport(const std::shared_ptr<GRM::Element> &element);
void setViewportForSideRegionElements(const std::shared_ptr<GRM::Element> &element, double offset, double width,
                                      bool uniform_data);

void processClipRegion(const std::shared_ptr<GRM::Element> &element);
void processCharExpan(const std::shared_ptr<GRM::Element> &element);
void processCharHeight(const std::shared_ptr<GRM::Element> &element);
void processCharSpace(const std::shared_ptr<GRM::Element> &element);
void processCharUp(const std::shared_ptr<GRM::Element> &element);
void processMarginalHeatmapKind(const std::shared_ptr<GRM::Element> &element);
void processResetRotation(const std::shared_ptr<GRM::Element> &element);
void processLineColorInd(const std::shared_ptr<GRM::Element> &element);
void processLineSpec(const std::shared_ptr<GRM::Element> &element);
void processLineType(const std::shared_ptr<GRM::Element> &element);
void processLineWidth(const std::shared_ptr<GRM::Element> &element);
void processMarkerColorInd(const std::shared_ptr<GRM::Element> &element);
void processMarkerSize(const std::shared_ptr<GRM::Element> &element);
void processMarkerType(const std::shared_ptr<GRM::Element> &element);
void processResampleMethod(const std::shared_ptr<GRM::Element> &element);
void processScale(const std::shared_ptr<GRM::Element> &element);
void processSelectSpecificXform(const std::shared_ptr<GRM::Element> &element);
void processSpace(const std::shared_ptr<GRM::Element> &element);
void processSpace3d(const std::shared_ptr<GRM::Element> &element);
void processTextAlign(const std::shared_ptr<GRM::Element> &element);
void processTextColorInd(const std::shared_ptr<GRM::Element> &element);
void processTextColorForBackground(const std::shared_ptr<GRM::Element> &element);
void processTextEncoding(const std::shared_ptr<GRM::Element> &element);
void processTransparency(const std::shared_ptr<GRM::Element> &element);
void processWSViewport(const std::shared_ptr<GRM::Element> &element);
void processWSWindow(const std::shared_ptr<GRM::Element> &element);
void processViewport(const std::shared_ptr<GRM::Element> &element);
void processBackgroundColor(const std::shared_ptr<GRM::Element> &element);
void processBorderColorInd(const std::shared_ptr<GRM::Element> &element);
void processBorderWidth(const std::shared_ptr<GRM::Element> &element);
void processMarginalHeatmapSidePlot(const std::shared_ptr<GRM::Element> &element);
void processColormap(const std::shared_ptr<GRM::Element> &element);
void processColorRep(const std::shared_ptr<GRM::Element> &element, const std::string &attribute);
void processColorReps(const std::shared_ptr<GRM::Element> &element);
void processFillColorInd(const std::shared_ptr<GRM::Element> &element);
void processFillIntStyle(const std::shared_ptr<GRM::Element> &element);
void processFillStyle(const std::shared_ptr<GRM::Element> &element);
void processFlip(const std::shared_ptr<GRM::Element> &element);
void processFont(const std::shared_ptr<GRM::Element> &element);
void processZIndex(const std::shared_ptr<GRM::Element> &element);
void processRefAxisLocation(const std::shared_ptr<GRM::Element> &element);
void processPrivateTransparency(const std::shared_ptr<GRM::Element> &element);
void plotProcessWsWindowWsViewport(const std::shared_ptr<GRM::Element> &element,
                                   const std::shared_ptr<GRM::Context> &context);

namespace GRM
{
void GRM_EXPORT processLimits(const std::shared_ptr<GRM::Element> &element);
void GRM_EXPORT processWindow(const std::shared_ptr<GRM::Element> &element);
} // namespace GRM

#endif // GR_PROCESS_ATTRIBUTES_HXX
