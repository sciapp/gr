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

static const char *program_name = NULL;
static char *current_filename = NULL;
static int option_use_glut = TRUE;

#define OPTIONS_T_DEFAULT {FALSE, -1, TRUE, FALSE, -1, 0, 0}
typedef struct {
    int show_box;
    float box_size;
    int show_bonds;
    int bond_chain;
    float delta;
    float rot;
    float tilt;
} options_t;
static options_t start_options = OPTIONS_T_DEFAULT;
static options_t current_options;


static int moldyn_parse_options_(options_t *options, int argc, char *argv[]);
static int moldyn_parse_arguments_(int argc, char *argv[]);

static int moldyn_parse_options_(options_t *options, int argc, char *argv[]) {
    while (argc > 0) {
        if (!strcmp(argv[0],"-box")) {
            /* -box (yes|no|<float>) */
            argv++; argc--;
            if (!argc) {
                moldyn_log("missing parameter for option '-box'!");
                return 1;
            } else if (!strcmp(argv[0],"yes")) {
                options->show_box = TRUE;
                options->box_size = -1;
            } else if (!strcmp(argv[0],"no")) {
                options->show_box = FALSE;
                options->box_size = -1;
            } else if (sscanf(argv[0],"%f",&options->box_size) == 1 && options->box_size > 0) {
                options->show_box = TRUE;
            } else {
                moldyn_log("unknown parameter for option '-box'!");
                return 1;
            }
        } else if (!strcmp(argv[0],"-bonds")) {
            /* -bonds (yes|no|chain) */
            argv++; argc--;
            if (!argc) {
                moldyn_log("missing parameter for option '-bonds'!");
                return 1;
            } else if (!strcmp(argv[0],"yes")) {
                options->show_bonds = TRUE;
                options->bond_chain = FALSE;
            } else if (!strcmp(argv[0],"no")) {
                options->show_bonds = FALSE;
                options->bond_chain = FALSE;
            } else if (!strcmp(argv[0],"chain")) {
                options->show_bonds = TRUE;
                options->bond_chain = TRUE;
            } else {
                moldyn_log("unknown parameter for option '-bonds'!");
                return 1;
            }
        } else if (!strcmp(argv[0],"-delta")) {
            /* -delta <float> */
            argv++; argc--;
            if (!argc) {
                moldyn_log("missing parameter for option '-delta'!");
                return 1;
            } else if (sscanf(argv[0],"%f",&options->delta) == 1 && options->delta > 0) {
            } else {
                moldyn_log("unknown parameter for option '-delta'!");
                return 1;
            }
        } else if (!strcmp(argv[0],"-rot")) {
            /* -rot <float> */
            argv++; argc--;
            if (!argc) {
                moldyn_log("missing parameter for option '-rot'!");
                return 1;
            } else if (sscanf(argv[0],"%f",&options->rot) == 1 && options->rot > 0) {
            } else {
                moldyn_log("unknown parameter for option '-rot'!");
                return 1;
            }
        } else if (!strcmp(argv[0],"-tilt")) {
            /* -tilt <float> */
            argv++; argc--;
            if (!argc) {
                moldyn_log("missing parameter for option '-tilt'!");
                return 1;
            } else if (sscanf(argv[0],"%f",&options->tilt) == 1 && options->tilt > 0) {
            } else {
                moldyn_log("unknown parameter for option '-tilt'!");
                return 1;
            }
        } else {
            moldyn_log("unknown option:");
            moldyn_log(argv[0]);
            return 1;
        }
        
        argv++; argc--;
    }
   return 0;
}

static int moldyn_parse_options_from_comment_(options_t *options, const char *comment) {
    int argc = 0;
    char **argv;
    char *strings;
    int len_strings = 0;
    int len_comment = strlen(comment);
    int err;
    if (comment[0] != '#') {
        return 1;
    }
    comment++;
    len_comment--;
    while (comment[0] != 0) {
        while (isspace(comment[0])) {
            comment++;
        }
        if (comment[0] == 0) {
            break;
        }
        argc++;
        len_strings++;
        while (comment[0] != 0 && !isspace(comment[0])) {
            comment++;
            len_strings++;
        }
    }
    comment -= len_comment;
    argv = malloc(argc*sizeof(char *));
    strings = malloc(len_strings);
    argc = 0;
    len_strings = 0;
    while (comment[0] != 0) {
        while (isspace(comment[0])) {
            comment++;
        }
        if (comment[0] == 0) {
            break;
        }
        argv[argc] = strings+len_strings;
        argc++;
        while (comment[0] != 0 && !isspace(comment[0])) {
            strings[len_strings] = comment[0];
            comment++;
            len_strings++;
        }
        strings[len_strings] = 0;
        len_strings++;
    }
    err = moldyn_parse_options_(options,argc, argv);
    free(argv);
    free(strings);
    return err;
}
static int moldyn_parse_arguments_(int argc, char *argv[]) {
    program_name = argv[0];
    argv++; argc--;
    if (argc > 0) {
        int file_name_len = strlen(argv[0]);
        current_filename = moldyn_reallocf(current_filename, file_name_len+1);
        strcpy(current_filename, argv[0]);
        argv++; argc--;
        /* TODO: parse arguments */
        moldyn_parse_options_(&start_options, argc, argv);
        return 0;
    } else {
        return 1;
    }
}

int moldyn_init(int argc, char *argv[]) {
    if (!moldyn_parse_arguments_(argc, argv)) {
        moldyn_init_filereader();
        if (!moldyn_open_file(current_filename)) {
            options_t file_options = start_options;
            fprintf(stderr, "moldyn: current filename = %s;\n", current_filename);
            fprintf(stderr, "moldyn: current comment = %s;\n", current_file_comment);
            if (moldyn_parse_options_from_comment_(&file_options,current_file_comment)) {
                moldyn_log("file comment was no valid options string!");
                current_options = start_options;
            } else {
                current_options = file_options;
            }
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