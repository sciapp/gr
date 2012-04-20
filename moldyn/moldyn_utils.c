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

void *moldyn_reallocf(void *ptr, size_t size) {
    void *new_ptr;
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        free(ptr);
        moldyn_log("failed to allocate memory!");
        moldyn_exit(1);
    }
    return new_ptr;
}

void moldyn_log(const char *log_message) {
    fprintf(stderr, "moldyn: %s\n", log_message);
}

int moldyn_parse_options(options_t *options, int argc, char *argv[]) {
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

int moldyn_parse_options_from_comment(options_t *options, const char *comment) {
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
    err = moldyn_parse_options(options,argc, argv);
    free(argv);
    free(strings);
    return err;
}

int moldyn_parse_arguments(int argc, char *argv[]) {
    program_name = argv[0];
    argv++; argc--;
    if (argc > 0) {
        int file_name_len = strlen(argv[0]);
        current_filename = moldyn_reallocf(current_filename, file_name_len+1);
        strcpy(current_filename, argv[0]);
        argv++; argc--;
        /* TODO: parse arguments */
        moldyn_parse_options(&start_options, argc, argv);
        return 0;
    } else {
        return 1;
    }
}