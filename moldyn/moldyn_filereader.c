#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "moldyn.h"
#include "moldyn_element_infos.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MOLDYN_FORMAT_UNKNOWN 1
#define MOLDYN_FORMAT_XYZ 2
#define MOLDYN_FORMAT_NORMAL 3
#define MOLDYN_FORMAT_UNICHEM 4


char current_title[MOLDYN_MAX_LINE_LENGTH+1] = {0};
char current_file_comment[MOLDYN_MAX_LINE_LENGTH+1] = {0};
float radius_factor = 0.2; /* Todo calculate this value like in moldyn */

int num_atoms = 0;
float *atom_positions = NULL;
float *atom_colors = NULL;
float *atom_radii = NULL;
char *atom_names = NULL;
int *atom_numbers1 = NULL;
int *atom_numbers2 = NULL;

int num_bonds = 0;
float *bond_positions = NULL;
float *bond_directions = NULL;
float *bond_colors = NULL;
float *bond_radii = NULL;
float *bond_lengths = NULL;

float xmin;
float xmax;
float ymin;
float ymax;
float zmin;
float zmax;
float xmean;
float ymean;
float zmean;

static FILE *current_file = NULL;
static int current_file_format = MOLDYN_FORMAT_UNKNOWN;
static int current_frame = 0;
static int num_known_frame_positions = 0;
static long *known_frame_positions = NULL;
static int first_invalid_frame = -1;

static char temp_comment[MOLDYN_MAX_LINE_LENGTH+1] = {0};

static void moldyn_realloc_atom_buffers_(int new_num_atoms);
static void moldyn_realloc_bond_buffers_(int new_num_bonds);
static int  moldyn_check_file_(FILE *fp, int *format, long *first_frame_position);
static int  moldyn_read_frame_(int frame_number);
static int  moldyn_read_frame_xyz_(void);
static int  moldyn_read_frame_normal_(void);
static void moldyn_analyze_frame_(void);
static int atomname2atomnumber(const char *atomname);

void moldyn_init_filereader(void) {
    /* do nothing */
}

int moldyn_open_file(const char *filename) {
    FILE *new_file;
    int format;
    long first_frame_position;
    
    new_file = fopen(filename, "r");
    if (!new_file) {
        moldyn_log("failed to open file!");
        return 1;
    }
    
    if (!moldyn_check_file_(new_file, &format, &first_frame_position)) {
        FILE *old_file = current_file;
        int old_file_format = current_file_format;
        int old_frame = current_frame;
        int old_num_known_frame_positions = num_known_frame_positions;
        long *old_known_frame_positions = known_frame_positions;
        int old_first_invalid_frame = first_invalid_frame;
        if (current_file) {
            fclose(current_file);
        }
        current_file = new_file;
        current_file_format = format;
        num_known_frame_positions = 1;
        known_frame_positions = moldyn_reallocf(known_frame_positions, num_known_frame_positions*sizeof(long));
        known_frame_positions[0] = first_frame_position;
        first_invalid_frame = -1;
        if (!moldyn_first_frame()) {
            if (old_file) {
                fclose(old_file);
                moldyn_reallocf(old_known_frame_positions,0);
            }
            strcpy(current_file_comment, temp_comment);
            return 0;
        } else {
            fclose(current_file);
            current_file = old_file;
            current_file_format = old_file_format;
            current_frame = old_frame;
            moldyn_reallocf(known_frame_positions,0);
            num_known_frame_positions = old_num_known_frame_positions;
            known_frame_positions = old_known_frame_positions;
            first_invalid_frame = old_first_invalid_frame;
            moldyn_log("failed to read the first frame! (maybe the format is unknown to moldyn?)");
            return 1;
        }
    } else {
        fclose(new_file);
        moldyn_log("failed to read file! (maybe the format is unknown to moldyn?)");
        return 1;
    }
}

void moldyn_terminate_filereader(void) {
    moldyn_realloc_atom_buffers_(0);
    moldyn_realloc_bond_buffers_(0);
    if (current_file) {
        fclose(current_file);
    }
}

int moldyn_first_frame(void) {
    return moldyn_read_frame_(0);
}

int  moldyn_next_frame(void) {
    return moldyn_read_frame_(current_frame+1);
}

int  moldyn_previous_frame(void) {
    if (current_frame > 0) {
        return moldyn_read_frame_(current_frame-1);
    } else {
        moldyn_log("already at the first frame.");
        return 1;
    }
}


static void moldyn_realloc_atom_buffers_(int new_num_atoms) {
    num_atoms = new_num_atoms;
    atom_positions = moldyn_reallocf(atom_positions, num_atoms*3*sizeof(float));
    atom_colors = moldyn_reallocf(atom_colors, num_atoms*3*sizeof(float));
    atom_radii = moldyn_reallocf(atom_radii, num_atoms*1*sizeof(float));
    atom_names = moldyn_reallocf(atom_names, num_atoms*4*sizeof(char));
    atom_numbers1 = moldyn_reallocf(atom_numbers1, num_atoms*1*sizeof(int));
    atom_numbers2 = moldyn_reallocf(atom_numbers2, num_atoms*1*sizeof(int));
}

static void moldyn_realloc_bond_buffers_(int new_num_bonds) {
    num_bonds = new_num_bonds;
    bond_positions = moldyn_reallocf(bond_positions, num_atoms*3*sizeof(float));
    bond_directions = moldyn_reallocf(bond_directions, num_atoms*3*sizeof(float));
    bond_colors = moldyn_reallocf(bond_colors, num_atoms*3*sizeof(float));
    bond_radii = moldyn_reallocf(bond_radii, num_atoms*1*sizeof(float));
    bond_lengths = moldyn_reallocf(bond_lengths, num_atoms*1*sizeof(float));
}

static int moldyn_read_frame_(int frame_number) {
    int err;
    long frame_position;
    if (first_invalid_frame != -1 && frame_number >= first_invalid_frame) {
        moldyn_log("not a valid frame!");
        return 1;
    }
    if (frame_number < num_known_frame_positions) {
        frame_position = known_frame_positions[frame_number];
    } else {
        moldyn_log("unknown frame position! (this is an internal error)");
        return 1;
    }
    if (fseek(current_file, frame_position, SEEK_SET) == -1) {
        moldyn_log("could not set file to frame position! (was the file changed while being opened by moldyn?)");
    }
    switch (current_file_format) {
        case MOLDYN_FORMAT_XYZ:
            err = moldyn_read_frame_xyz_();
            break;
        case MOLDYN_FORMAT_UNICHEM:
            break;
        case MOLDYN_FORMAT_NORMAL:
            err = moldyn_read_frame_normal_();
            break;
        case MOLDYN_FORMAT_UNKNOWN:
        default:
            moldyn_log("unknown format! (this is an internal error)");
            break;
    }
    if (err) {
        if (first_invalid_frame == -1 || first_invalid_frame > frame_number) {
            first_invalid_frame = frame_number;
        }
        return 1;
    }
    moldyn_analyze_frame_();
    
    current_frame = frame_number;
    if (frame_number + 1 == num_known_frame_positions) {
        num_known_frame_positions++;
        known_frame_positions = moldyn_reallocf(known_frame_positions, num_known_frame_positions*sizeof(long));
        known_frame_positions[frame_number+1] = ftell(current_file);
    }
    return 0;
}

static int moldyn_read_frame_normal_(void) {
    int i;
    int icycle;
    double e0;
    double energy;
    double dum;
    int new_num_atoms;
    long current_position;
    long start_position;
    char line[MOLDYN_MAX_LINE_LENGTH+1] = {0};
    char *tmp;
    /* TODO: use cycle, e0, energy, dum */
    if (!fgets(line, MOLDYN_MAX_LINE_LENGTH+1, current_file)) {
        moldyn_log("could not read any further. (1)");
        return 1;
    }
    if (feof(current_file)) {
        moldyn_log("reached EOF!");
        return 1;
    }
    if (line[0] == '#') {
        if (!fgets(line, MOLDYN_MAX_LINE_LENGTH+1, current_file)) {
            moldyn_log("could not read any further. (2)");
            return 1;
        }
        moldyn_log("a comment was ignored.");
    }
    if (sscanf(line, "%d %lg %lg %lg", &icycle, &e0, &energy, &dum) != 4) {
        moldyn_log("frame is not in normal format!");
        return 1;
    }
    start_position = ftell(current_file);
    new_num_atoms = 0;
    while (TRUE) {
        current_position = ftell(current_file);
        if (!fgets(line, MOLDYN_MAX_LINE_LENGTH+1, current_file)) {
            moldyn_log("could not read any further. (3)");
            return 1;
        }
        if (feof(current_file)) {
            moldyn_log("reached EOF!");
            break;
        }
        strtok(line, MOLDYN_SEPARATORS);
        tmp = strtok(NULL, MOLDYN_SEPARATORS);
        if (strchr(tmp, '.') != NULL) {
            fseek(current_file, current_position, SEEK_SET);
            break;
        } else {
            new_num_atoms++;
        }
    }
    fseek(current_file, start_position, SEEK_SET);
    moldyn_realloc_atom_buffers_(new_num_atoms);
    for (i = 0; i < new_num_atoms; i++) {
        int atomnumber1;
        int atomnumber2;
        float x;
        float y;
        float z;
        if (!fgets(line, MOLDYN_MAX_LINE_LENGTH+1, current_file)) {
            moldyn_log("could not read any further. (4)");
            return 1;
        }
        if (feof(current_file)) {
            moldyn_log("reached EOF!");
            return 1;
        }
        if (sscanf(line, "%d %d %g %g %g", &atomnumber1, &atomnumber2, &x, &y, &z) != 5) {
            moldyn_log("frame is not in normal format!");
            return 1;
        }
        atom_positions[3*i+0] = x;
        atom_positions[3*i+1] = y;
        atom_positions[3*i+2] = -z;
        if (atomnumber1) {
            atom_colors[3*i+0] = f_ptable[atomnumber1-1][0]/255.0;
            atom_colors[3*i+1] = f_ptable[atomnumber1-1][1]/255.0;
            atom_colors[3*i+2] = f_ptable[atomnumber1-1][2]/255.0;
        } else {
            atom_colors[3*i+0] = 1.0;
            atom_colors[3*i+1] = 1.0;
            atom_colors[3*i+2] = 1.0;
        }
        if (atomnumber1) {
            atom_radii[i] = radius_factor * radii[atomnumber1-1];
        }
        atom_names[4*i+0] = 0;
        atom_names[4*i+1] = 0;
        atom_names[4*i+2] = 0;
        atom_names[4*i+3] = 0;
        atom_numbers1[i] = atomnumber1;
        atom_numbers2[i] = atomnumber2;
    }
    current_title[0] = 0;
    return 0;
}

static int moldyn_read_frame_xyz_(void) {
    int i;
    int new_num_atoms;
    char line[MOLDYN_MAX_LINE_LENGTH+1] = {0};
    char title[MOLDYN_MAX_LINE_LENGTH+1] = {0};
    
    if (!fgets(line, MOLDYN_MAX_LINE_LENGTH+1, current_file)) {
        moldyn_log("could not read any further. (1)");
        return 1;
    }
    if (feof(current_file)) {
        moldyn_log("reached EOF!");
        return 1;
    }
    if (line[0] == '#') {
        if (!fgets(line, MOLDYN_MAX_LINE_LENGTH+1, current_file)) {
            moldyn_log("could not read any further. (2)");
            return 1;
        }
        moldyn_log("a comment was ignored.");
    }
    if (sscanf(line, "%d", &new_num_atoms) != 1 || new_num_atoms < 0) {
        moldyn_log("frame is not in xyz format!");
        return 1;
    }
    if (!fgets(line, MOLDYN_MAX_LINE_LENGTH+1, current_file)) {
        moldyn_log("could not read any further. (3)");
        return 1;
    }
    strtok(line, "\n");
    strcpy(title, line);
    moldyn_realloc_atom_buffers_(new_num_atoms);
    for (i = 0; i < new_num_atoms; i++) {
        int atomnumber;
        char atomname[4] = {0};
        float x;
        float y;
        float z;
        char *line_start_ptr = line;
        if (!fgets(line, MOLDYN_MAX_LINE_LENGTH+1, current_file)) {
            moldyn_log("could not read any further. (4)");
            return 1;
        }
        if (feof(current_file)) {
            moldyn_log("reached EOF!");
            return 1;
        }
        while (isspace(*line_start_ptr)) {
            line_start_ptr++;
        }
        if (sscanf(line_start_ptr, "%3s %g %g %g", atomname, &x, &y, &z) != 4) {
            moldyn_log("frame is not in xyz format!");
            return 1;
        }
        atomnumber = atomname2atomnumber(atomname);
        atom_positions[3*i+0] = x;
        atom_positions[3*i+1] = y;
        atom_positions[3*i+2] = -z;
        if (atomnumber) {
            atom_colors[3*i+0] = f_ptable[atomnumber-1][0]/255.0;
            atom_colors[3*i+1] = f_ptable[atomnumber-1][1]/255.0;
            atom_colors[3*i+2] = f_ptable[atomnumber-1][2]/255.0;
        } else {
            atom_colors[3*i+0] = 1.0;
            atom_colors[3*i+1] = 1.0;
            atom_colors[3*i+2] = 1.0;
        }
        if (atomnumber) {
            atom_radii[i] = radius_factor * radii[atomnumber-1];
        }
        atom_names[4*i+0] = atomname[0];
        atom_names[4*i+1] = atomname[1];
        atom_names[4*i+2] = atomname[2];
        atom_names[4*i+3] = atomname[3];
        atom_numbers1[i] = atomnumber;
        atom_numbers2[i] = 1;
    }
    strcpy(current_title, title);
    return 0;
}

static void moldyn_analyze_frame_(void) {
    int i, j;
    int dim;
    float scale;
    
    xmin = atom_positions[0];
    xmax = atom_positions[0];
    ymin = atom_positions[1];
    ymax = atom_positions[1];
    zmin = atom_positions[2];
    zmax = atom_positions[2];
    
    for (i = 0; i < num_atoms; i++) {
        if (atom_positions[3*i+0] < xmin) {
            xmin = atom_positions[3*i+0];
        } else if (atom_positions[3*i+0] > xmax) {
            xmax = atom_positions[3*i+0];
        }
        if (atom_positions[3*i+1] < ymin) {
            ymin = atom_positions[3*i+1];
        } else if (atom_positions[3*i+1] > ymax) {
            ymax = atom_positions[3*i+1];
        }
        if (atom_positions[3*i+2] < zmin) {
            zmin = atom_positions[3*i+2];
        } else if (atom_positions[3*i+2] > zmax) {
            zmax = atom_positions[3*i+2];
        }
    }
    
    xmean = (xmin + xmax)/2;
    ymean = (ymin + ymax)/2;
    zmean = (zmin + zmax)/2;
    xmin -= xmean;
    xmax -= xmean;
    ymin -= ymean;
    ymax -= ymean;
    zmin -= zmean;
    zmax -= zmean;
    
    if (current_options.box_size > 0) {
        xmin = -current_options.box_size;
        xmax = current_options.box_size;
        ymin = -current_options.box_size;
        ymax = current_options.box_size;
        zmin = -current_options.box_size;
        zmax = current_options.box_size;
    }
    
    if (xmax == xmin || ymax == ymin || zmax == zmin) {
        dim = 2;
    } else {
        dim = 3;
    }
    
    scale = (xmax - xmin + ymax - ymin + zmax - zmin) / dim;
    while (scale < (zmax - zmin) / 2) {
        scale = scale * 1.5;
    }
    /* TODO: add dist */
    if (current_options.delta <= 0) {
        float delta;
        if (current_options.show_bonds && current_options.delta < 0) {
            delta = 0;
            for (i = 0; i + 1 < num_atoms; i++) {
                float *pos1 = atom_positions + 3*i;
                float *pos2 = atom_positions + 3*(i+1);
                delta += sqrt((pos1[0]-pos2[0])*(pos1[0]-pos2[0])
                              +(pos1[1]-pos2[1])*(pos1[1]-pos2[1])
                              +(pos1[2]-pos2[2])*(pos1[2]-pos2[2]));
            }
            delta = 1.125 * delta / (num_atoms - 1);
        } else {
            delta = 2.25 * scale / sqrt(num_atoms);
        }
        
        if (current_options.show_bonds) {
            while (TRUE) {
                num_bonds = 0;
                for (i = 0; i < num_atoms; i++) {
                    for (j = i + 1; j < num_atoms; j++) {
                        float *pos1 = atom_positions + 3*i;
                        float *pos2 = atom_positions + 3*j;
                        if ( delta > sqrt((pos1[0]-pos2[0])*(pos1[0]-pos2[0])
                                         +(pos1[1]-pos2[1])*(pos1[1]-pos2[1])
                                         +(pos1[2]-pos2[2])*(pos1[2]-pos2[2]))) {
                            num_bonds++;
                        }
                    }
                }
                if (num_bonds > 3 * num_atoms) {
                    delta *= 0.75;
                } else {
                    break;
                }
            }
        } 
    }
}

static int moldyn_check_file_(FILE *fp, int *format, long *first_frame_position) {
    char line[MOLDYN_MAX_LINE_LENGTH+1] = {0};
    int i_dummy;
    double d_dummy1;
    double d_dummy2;
    double d_dummy3;
    
    *format = MOLDYN_FORMAT_UNKNOWN;
    *first_frame_position = ftell(fp);
    if (!fgets(line, MOLDYN_MAX_LINE_LENGTH+1, fp)) {
        return 1;
    }
    
    if (*line == '#') {
        options_t file_options = start_options;
        strcpy(temp_comment, line);
        if (moldyn_parse_options_from_comment(&file_options,temp_comment)) {
            moldyn_log("file comment was no valid options string!");
            current_options = start_options; 
        } else {
            current_options = file_options;
        }
        *first_frame_position = ftell(fp);
        if (!fgets(line, MOLDYN_MAX_LINE_LENGTH+1, fp)) {
            return 1;
        }
    }
    if (sscanf(line, "%d %lg %lg %lg", &i_dummy, &d_dummy1, &d_dummy2, &d_dummy3) >= 4) {
        *format = MOLDYN_FORMAT_NORMAL;
        moldyn_log("file is in normal format");
    } else if (sscanf(line, "%d", &i_dummy) == 1) {
        *format = MOLDYN_FORMAT_XYZ;
        moldyn_log("file is in xyz format");
    } else if (fscanf(fp, "%d", &i_dummy) == 1) {
        *format = MOLDYN_FORMAT_UNICHEM;
        moldyn_log("file is in unichem format");
    } else {
        return 1;
    }
    rewind(fp);
    return 0;
}

static int atomname2atomnumber(const char *atomname) {
    int i, l;
    char str[4];
    
    l = strlen(atomname);
    if (l > 3) {
        moldyn_log("atomname longer than three characters! (this is an internal error)");
        l = 3;
    }
    for (i = 0; i < l; i++) {
        str[i] = toupper(atomname[i]);
    }
    str[l] = '\0';
    for (i = 0; i < 118; i++) {
        if (!strcmp(ptable[i], str)) {
            return i + 1;
        }
    }
    moldyn_log("unknown atomname");
    return 0;
}
