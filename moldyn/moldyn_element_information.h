#ifndef MOLDYN_ELEMENT_INFORMATION_H
#define MOLDYN_ELEMENT_INFORMATION_H

#define NUM_ELEMENTS 118

void reset_element_information(void);

extern float element_radii[118];
extern unsigned char element_colors[118][3];
extern const char element_names[118][4];
extern const float default_element_radii[118];
extern const unsigned char default_element_colors[118][3];

extern const unsigned char all_color_rgb[752][3];
extern const char all_color_names[752][23];

#endif
