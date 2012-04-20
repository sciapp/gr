#ifndef MOLDYN_H
#define MOLDYN_H

#define MOLDYN_MAX_LINE_LENGTH 255
#define MOLDYN_SEPARATORS " \t"

typedef struct {
    int show_box;
    float box_size;
    int show_bonds;
    int bond_chain;
    float delta;
    float rot;
    float tilt;
} options_t;

extern options_t current_options;
extern options_t start_options;
extern const char *program_name;
extern char *current_filename;

extern char current_title[MOLDYN_MAX_LINE_LENGTH+1];
extern char current_file_comment[MOLDYN_MAX_LINE_LENGTH+1];

extern int num_atoms;
extern float *atom_positions;
extern float *atom_colors;
extern float *atom_radii;

extern int num_bonds;
extern float *bond_positions;
extern float *bond_directions;
extern float *bond_colors;
extern float *bond_radii;
extern float *bond_lengths;

/* Main application */
int moldyn_init(int argc, char *argv[]);
void moldyn_terminate(void);
void moldyn_exit(int return_code);

void moldyn_print_usage(void);

/* Navigation */
void moldyn_rotate_horizontally(double radians);
void moldyn_rotate_vertically(double radians);
void moldyn_rotate_left(void);
void moldyn_rotate_right(void);
void moldyn_rotate_up(void);
void moldyn_rotate_down(void);

void moldyn_translate(double x, double y, double z);
void moldyn_translate_horizontally(double units);
void moldyn_translate_vertically(double units);
void moldyn_translate_left(void);
void moldyn_translate_right(void);
void moldyn_translate_up(void);
void moldyn_translate_down(void);

void moldyn_zoom(double units);
void moldyn_zoom_in(void);
void moldyn_zoom_out(void);

void moldyn_reset_camera(void);

/* File handling */
void moldyn_init_filereader(void);
void moldyn_terminate_filereader(void);

int  moldyn_open_file(const char *filename);
int  moldyn_next_frame(void);
int  moldyn_previous_frame(void);
int  moldyn_first_frame(void);

/* GLUT */
void moldyn_init_glut(void);
void moldyn_terminate_glut(void);
void moldyn_init_glut_menu(void);
void moldyn_terminate_glut_menu(void);
void moldyn_init_glut_key_bindings(void);
void moldyn_terminate_glut_key_bindings(void);

void moldyn_start_glut_mainloop(void);
void moldyn_show_help_overlay(void);
void moldyn_start_animation(void);
void moldyn_stop_animation(void);
void moldyn_toggle_animation(void);

/* GR3 */
void moldyn_init_gr3(void);
void moldyn_terminate_gr3(void);

void moldyn_update_scene(void);
void moldyn_export(const char *filename, int width, int height);

#define MOLDYN_WINDOW_OPENGL 1
int  moldyn_drawimage(float xmin, float xmax, float ymin, float ymax, int width, int height, int window);

#define MOLDYN_QUALITY_OPENGL_NO_SSAA 0
#define MOLDYN_QUALITY_OPENGL_2X_SSAA 1
#define MOLDYN_QUALITY_OPENGL_4X_SSAA 2
#define MOLDYN_QUALITY_OPENGL_8X_SSAA 3
#define MOLDYN_QUALITY_OPENGL_16X_SSAA 4
#define MOLDYN_QUALITY_POVRAY_NO_SSAA 5
#define MOLDYN_QUALITY_POVRAY_2X_SSAA 6
#define MOLDYN_QUALITY_POVRAY_4X_SSAA 7
#define MOLDYN_QUALITY_POVRAY_8X_SSAA 8
#define MOLDYN_QUALITY_POVRAY_16X_SSAA 9
int  moldyn_set_export_quality(int quality);
int  moldyn_set_normal_quality(int quality);

/* Utility functions */
void *moldyn_reallocf(void *ptr, size_t size);
void moldyn_log(const char *log_message);
int moldyn_parse_options(options_t *options, int argc, char *argv[]);
int moldyn_parse_options_from_comment(options_t *options, const char *comment);
int moldyn_parse_arguments(int argc, char *argv[]);
#endif
