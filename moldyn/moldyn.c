#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "moldyn.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

const char *program_name = NULL;
char *current_filename = NULL;
static int option_use_glut = TRUE;

#define OPTIONS_T_DEFAULT {FALSE, -1, TRUE, FALSE, -1, 0, 0}
options_t start_options = OPTIONS_T_DEFAULT;
options_t current_options;

int moldyn_init(int argc, char *argv[]) {
    if (!moldyn_parse_arguments(argc, argv)) {
        moldyn_init_filereader();
        if (!moldyn_open_file(current_filename)) {
            fprintf(stderr, "moldyn: current filename = %s;\n", current_filename);
            fprintf(stderr, "moldyn: current comment = %s;\n", current_file_comment);
            if (current_title[0]) {
                fprintf(stderr, "moldyn: current title = %s;\n", current_title);
            }
            if (num_atoms > 0) {
                fprintf(stderr, "moldyn: atom_position[0] = {%f,%f,%f};\n", atom_positions[0], atom_positions[1], atom_positions[2]);
            }
            if (option_use_glut) {
                moldyn_init_glut();
            }
            moldyn_init_gr3();
            moldyn_update_scene();
            if (option_use_glut) {
                moldyn_start_glut_mainloop();
            }
            moldyn_terminate();
            return 0;
        } else {
            moldyn_terminate_filereader();
            return 1;
        }
    } else {
        moldyn_print_usage();
        return 1;
    }
}

void moldyn_terminate(void) {
    if (option_use_glut) {
        moldyn_terminate_glut();
    }
    moldyn_terminate_gr3();
    moldyn_terminate_filereader();
}

void moldyn_exit(int return_code) {
    moldyn_terminate();
    exit(return_code);
}

void moldyn_print_usage(void) {
    if (!program_name) {
        program_name = "moldyn";
    }
    printf("usage: %s file [-options]\n", program_name);
}

int main(int argc, char *argv[]) {
    return moldyn_init(argc, argv);
}