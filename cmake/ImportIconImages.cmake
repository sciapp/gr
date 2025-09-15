set(GRPLOT_ICONS
    use_gr3
    algorithm
    aspect_ratio
    flip
    lim
    location
    log
    orientation
    kind
    polar_with_pan
    text_color_ind
    use_gr3_dark
    algorithm_dark
    aspect_ratio_dark
    flip_dark
    lim_dark
    location_dark
    log_dark
    orientation_dark
    kind_dark
    polar_with_pan_dark
    text_color_ind_dark
)
set(GRPLOT_ICON_QRC "<RCC>\n    <qresource prefix=\"/\">\n")
foreach(icon IN LISTS GRPLOT_ICONS)
  string(APPEND GRPLOT_ICON_QRC "        <file>icons/${icon}.png</file>\n")
endforeach()
string(APPEND GRPLOT_ICON_QRC "   </qresource>\n</RCC>")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/grplot_icon.qrc" "${GRPLOT_ICON_QRC}")

set(GRPLOT_ICONS_IMAGES "${GRPLOT_ICONS}")
list(TRANSFORM GRPLOT_ICONS_IMAGES PREPEND "icons/")
list(TRANSFORM GRPLOT_ICONS_IMAGES APPEND ".png")
add_custom_target(grplot_icon_images DEPENDS ${GRPLOT_ICONS_IMAGES})
add_custom_command(
  OUTPUT ${GRPLOT_ICONS_IMAGES}
  COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/../lib/grm/grplot/icons"
          "${CMAKE_CURRENT_BINARY_DIR}/icons"
)
