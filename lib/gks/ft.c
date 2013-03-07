#include <stdlib.h>
#include <math.h>

#include "gks.h"
#include "gkscore.h"

#ifdef XFT

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_XFREE86_H

const static FT_String *gks_font_list[] = {
  "NimbusRomNo9L-Regu",         /* 1: Times New Roman */
  "NimbusRomNo9L-ReguItal",
  "NimbusRomNo9L-Medi",
  "NimbusRomNo9L-MediItal",
  "NimbusSanL-Regu",            /* 5: Helvetica */
  "NimbusSanL-ReguItal",
  "NimbusSanL-Bold",
  "NimbusSanL-BoldItal",
  "NimbusMonL-Regu",            /* 9: Courier */
  "NimbusMonL-ReguObli",
  "NimbusMonL-Bold",
  "NimbusMonL-BoldObli",
  "StandardSymL",               /* 13: Symbol */
  "URWBookmanL-Ligh",           /* 14: Bookman Light */
  "URWBookmanL-LighItal",
  "URWBookmanL-DemiBold",
  "URWBookmanL-DemiBoldItal",
  "CenturySchL-Roma",           /* 18: New Century Schoolbook Roman */
  "CenturySchL-Ital",
  "CenturySchL-Bold",
  "CenturySchL-BoldItal",
  "URWGothicL-Book",            /* 22: Avant Garde Book */
  "URWGothicL-BookObli",
  "URWGothicL-Demi",
  "URWGothicL-DemiObli",
  "URWPalladioL-Roma",          /* 26: Palatino */
  "URWPalladioL-Ital",
  "URWPalladioL-Bold",
  "URWPalladioL-BoldItal",
  "URWChanceryL-MediItal",      /* 30: Zapf Chancery */
  "Dingbats"                    /* 31: Zapf Dingbats */
};

const static int map[] = {
  22, 9, 5, 14, 18, 26, 13, 1,
  24, 11, 7, 16, 20, 28, 13, 3,
  23, 10, 6, 15, 19, 27, 13, 2,
  25, 12, 8, 17, 21, 29, 13, 4
};

const static float caps[] = {
  0.662, 0.660, 0.681, 0.662,
  0.729, 0.729, 0.729, 0.729,
  0.583, 0.583, 0.583, 0.583,
  0.667,
  0.681, 0.681, 0.681, 0.681,
  0.722, 0.722, 0.722, 0.722,
  0.739, 0.739, 0.739, 0.739,
  0.694, 0.693, 0.683, 0.683,
  0.587, 0.692
};

static FT_Bool init = 0;
static FT_Library library;

static FT_Pointer safe_realloc(FT_Pointer ptr, size_t size);
static FT_Error set_glyph(FT_Face *face, FT_UInt codepoint, FT_UInt *previous,
                          FT_Vector *pen, FT_Bool vertical, FT_Matrix *rotation);
static void convert_text(FT_Bytes str, FT_UInt *unicode_string, int *length);
static FT_Long min(FT_Long a, FT_Long b);
static FT_Long max(FT_Long a, FT_Long b);

static FT_Long min(FT_Long a, FT_Long b) {
  return a > b ? b : a;
}

static FT_Long max(FT_Long a, FT_Long b) {
  return a > b ? a : b;
}

static FT_Pointer safe_realloc(FT_Pointer ptr, size_t size) {
  FT_Pointer tmp;
  if (ptr) {
    tmp = malloc(size);
  } else {
    tmp = realloc(ptr, size);
  }
  if (tmp != NULL) {
    ptr = tmp;
  } else {
    gks_perror("Out of memory");
    ptr = NULL;
  }
  return ptr;
}

/* load a glyph into the slot */
static FT_Error set_glyph(FT_Face *face, FT_UInt codepoint, FT_UInt *previous,
                          FT_Vector *pen, FT_Bool vertical, FT_Matrix *rotation) {
  FT_Error error;
  FT_UInt glyph_index;

  glyph_index = FT_Get_Char_Index(*face, codepoint);
  if (FT_HAS_KERNING((*face)) && *previous && glyph_index) {
    FT_Vector delta;
    FT_Get_Kerning(*face, *previous, glyph_index, FT_KERNING_DEFAULT,
                   &delta);
    FT_Vector_Transform(&delta, rotation);
    if (!vertical) (*pen).x += delta.x;
    (*pen).y += delta.y;
  }
  error = FT_Load_Glyph(*face, glyph_index, vertical ?
                        FT_LOAD_VERTICAL_LAYOUT : FT_LOAD_DEFAULT);
  if (error) {
    gks_perror("Glyph could not be loaded: %c", codepoint);
    return 1;
  }
  error = FT_Render_Glyph((*face)->glyph, FT_RENDER_MODE_NORMAL);
  if (error) {
    gks_perror("Glyph could not be rendered: %c", codepoint);
    return 1;
  }
  *previous = glyph_index;
  return 0;
}

/* set text string, converting UTF-8 into Unicode */
static void convert_text(FT_Bytes str, FT_UInt *unicode_string, int *length) {
  FT_UInt num_glyphs = 0;
  FT_UInt codepoint;
  FT_Byte following_bytes;
  FT_Byte offset;
  int i, j;

  for (i = 0; i < *length; i++) {
    if (str[i] < 128) {
      offset = 0;
      following_bytes = 0;
    } else if (str[i] < 128+64+32) {
      offset = 128+64;
      following_bytes = 1;
    } else if (str[i] < 128+64+32+16) {
      offset = 128+64+32;
      following_bytes = 2;
    } else if (str[i] < 128+64+32+16+8) {
      offset = 128+64+32+16;
      following_bytes = 3;
    } else {
      gks_perror("Character ignored due to unicode error");
      continue;
    }
    codepoint = str[i] - offset;
    for (j = 0; j < following_bytes; j++) {
      codepoint = codepoint * 64;
      i++;
      if (str[i] < 128 || str[i] >= 128+64) {
        gks_perror("Character ignored due to unicode error");
        continue;
      }
      codepoint += str[i] - 128;
    }
    unicode_string[num_glyphs] = codepoint;
    num_glyphs++;
  }
  unicode_string[num_glyphs] = '\0';

  *length = num_glyphs;
}

int gks_ft_init(void) {
  FT_Error error;
  if (init) return 0;
  error = FT_Init_FreeType(&library);
  if (error) {
    gks_perror("Could not initialize freetype library");
    init = 0;
  } else {
    init = 1;
  }
  return error;
}

/* deallocate memory */
void gks_ft_terminate(void) {
  if (init) {
    FT_Done_FreeType(library);
  }
  init = 0;
}

/* create rgba bitmap and compute position */
int *gks_ft_render(int *x, int *y, int *width, int *height,
                   gks_state_list_t *gkss, const char *text, int length) {
  FT_Face face;                   /* font face */
  FT_Vector pen;                  /* glyph position */
  FT_Long xmin, ymin, xmax, ymax; /* bounding box */
  FT_UInt previous;               /* previous glyph index */
  FT_Vector spacing;              /* amount of additional space between glyphs */
  FT_Vector right;                /* for computing vertical alignment */
  FT_ULong textheight;            /* textheight in FreeType convention */
  FT_Error error;                 /* error code */
  FT_Matrix rotation;             /* text rotation matrix */
  FT_UInt size;                   /* number of pixels of the bitmap */
  FT_String *file;                /* concatenated font path */
  const FT_String *font, *prefix; /* font file name and directory */
  int color[3];                   /* text color */
  FT_UInt *unicode_string;        /* unicode text string */
  FT_Int halign, valign;          /* alignment */
  FT_Byte *mono_bitmap = NULL;    /* target for rendered text */
  FT_Byte *rgba_bitmap = NULL;    /*        for creation of colored text */
  FT_Int num_glyphs;              /* number of glyphs */
  FT_Vector tr, align, point;
  FT_Bitmap ftbitmap;
  FT_UInt codepoint;
  int i, j, k, textfont, dx, dy, value, tmp;
  float red, green, blue, angle;
  const int windowheight = *height;
  const int direction = (gkss->txp <= 3 && gkss->txp >= 0 ? gkss->txp : 0);
  const FT_Bool vertical = (direction == GKS_K_TEXT_PATH_DOWN ||
                            direction == GKS_K_TEXT_PATH_UP);

  if (!init) gks_ft_init();

  num_glyphs = length;
  unicode_string = (FT_UInt *) malloc(length * sizeof(FT_UInt) + 1);
  convert_text((FT_Bytes)text, unicode_string, &num_glyphs);

  if (gkss->txal[0] != GKS_K_TEXT_HALIGN_NORMAL) {
    halign = gkss->txal[0];
  } else if (vertical) {
    halign = GKS_K_TEXT_HALIGN_CENTER;
  } else if (direction == GKS_K_TEXT_PATH_LEFT) {
    halign = GKS_K_TEXT_HALIGN_RIGHT;
  } else {
    halign = GKS_K_TEXT_HALIGN_LEFT;
  }
  valign = gkss->txal[1];
  textfont = abs(gkss->txfont);
  if (textfont >= 101 && textfont <= 131)
    textfont -= 100;
  if (textfont <= 32) {
    font = gks_font_list[map[textfont - 1] - 1];
  } else {
    gks_perror("Invalid font index: %d", gkss->txfont);
    font = gks_font_list[0];
  }

  prefix = gks_getenv("GKS_FONTPATH");
  if (prefix == NULL) {
    prefix = GRDIR;
  }
  file = (FT_String *) malloc(strlen(prefix) + 9 + strlen(font) + 4 + 1);
  strcpy(file, prefix);
#ifndef _WIN32
  strcat(file, "/fonts/");
#else
  strcat(fontdb, "\\FONTS\\");
#endif
  strcat(file, font);
  strcat(file, ".pfb");
  error = FT_New_Face(library, file, 0, &face);
  if (error == FT_Err_Unknown_File_Format) {
    gks_perror("Unknown file format: %s", file);
    return NULL;
  } else if (error) {
    gks_perror("Could not open font file: %s", file);
    return NULL;
  }
  if (strcmp(FT_Get_X11_Font_Format(face), "Type 1") == 0) {
    strcpy(file, prefix);
#ifndef _WIN32
    strcat(file, "/fonts/");
#else
    strcat(fontdb, "\\FONTS\\");
#endif
    strcat(file, font);
    strcat(file, ".afm");
    FT_Attach_File(face, file);
  }
  free(file);

  textheight = gkss->chh * windowheight * 64 / caps[map[textfont-1]-1];
  error = FT_Set_Char_Size(face, textheight * gkss->chxp, textheight, 72, 72);
  if (error) gks_perror("Cannot set text height");

  if (gkss->chup[0] != 0.0 || gkss->chup[1] != 0.0) {
    angle = atan2f(gkss->chup[1], gkss->chup[0]) - M_PI/2;
    rotation.xx =  cosf(angle) * 0x10000L;
    rotation.xy = -sinf(angle) * 0x10000L;
    rotation.yx =  sinf(angle) * 0x10000L;
    rotation.yy =  cosf(angle) * 0x10000L;
    FT_Set_Transform(face, &rotation, NULL);
  } else {
    FT_Set_Transform(face, NULL, NULL);
  }

  spacing.x = spacing.y = 0;
  if (gkss->chsp != 0.0) {
    error = FT_Load_Glyph(face, FT_Get_Char_Index(face, ' '),
                          vertical ? FT_LOAD_VERTICAL_LAYOUT : FT_LOAD_DEFAULT);
    if (!error) {
      spacing.x = face->glyph->advance.x * gkss->chsp;
      spacing.y = face->glyph->advance.y * gkss->chsp;
    } else {
      gks_perror("Cannot apply character spacing");
    }
  }

  xmin = ymin = LONG_MAX;
  xmax = ymax = LONG_MIN;
  pen.x = pen.y = 0;
  right.x = right.y = 0;
  previous = 0;

  for (i = 0; i < num_glyphs; i++) {
    codepoint = unicode_string[direction == GKS_K_TEXT_PATH_LEFT ?
                               (num_glyphs - 1 - i) : i];
    error = set_glyph(&face, codepoint, &previous, &pen, vertical, &rotation);
    if (error) continue;

    /* glyph translation vector for vertical text */
    tr.x = tr.y = 0;
    if (vertical && halign != GKS_K_TEXT_HALIGN_LEFT) {
      tr.x = face->size->metrics.max_advance - face->glyph->metrics.width;
      if (tr.x != 0) FT_Vector_Transform(&tr, &rotation);
      if (halign != GKS_K_TEXT_HALIGN_RIGHT) {
        tr.x /= 2;
        tr.y /= 2;
      }
    }
    /* enlarge bounding box if necessary */
    xmin = min(xmin, pen.x + min(tr.x, 0) + 64*face->glyph->bitmap_left);
    xmax = max(xmax, pen.x + max(tr.x, 0) +
               64*(face->glyph->bitmap_left + face->glyph->bitmap.width));
    ymin = min(ymin, pen.y + min(tr.y, 0) +
               64*(face->glyph->bitmap_top - face->glyph->bitmap.rows));
    ymax = max(ymax, pen.y + max(tr.y, 0) + 64*face->glyph->bitmap_top);
    if (vertical) {
      right.x = max(right.x, face->glyph->metrics.width);
    }
    if (direction == GKS_K_TEXT_PATH_DOWN) {
      pen.x -= face->glyph->advance.x + spacing.x;
      pen.y -= face->glyph->advance.y + spacing.y;
    } else {
      pen.x += face->glyph->advance.x + spacing.x;
      pen.y += face->glyph->advance.y + spacing.y;
    }
  }

  *width = (int)((xmax - xmin) / 64);
  *height = (int)((ymax - ymin) / 64);
  if (xmax <= xmin || ymax <= ymin) return NULL;
  size = *width * *height;
  mono_bitmap = (FT_Byte *) safe_realloc(mono_bitmap, size);
  memset(mono_bitmap, 0, size);

  pen.x = 0;
  pen.y = 0;
  previous = 0;

  for (i = 0; i < num_glyphs; i++) {
    codepoint = unicode_string[direction == GKS_K_TEXT_PATH_LEFT ?
                               (num_glyphs - 1 - i) : i];
    error = set_glyph(&face, codepoint, &previous, &pen, vertical, &rotation);
    if (error) continue;

    tr.x = 0; tr.y = 0;
    if (vertical && halign != GKS_K_TEXT_HALIGN_LEFT) {
      tr.x = right.x - face->glyph->metrics.width;
      if (tr.x != 0) FT_Vector_Transform(&tr, &rotation);
      if (halign != GKS_K_TEXT_HALIGN_RIGHT) {
        tr.x /= 2;
        tr.y /= 2;
      }
    }

    tr.x =  tr.x + pen.x + 64*face->glyph->bitmap_left - xmin;
    tr.y = -tr.y - pen.y - 64*face->glyph->bitmap_top  + ymax;
    ftbitmap = face->glyph->bitmap;
    for (j = 0; j < ftbitmap.rows; j++) {
      for (k = 0; k < ftbitmap.width; k++) {
        dx = k + (tr.x / 64);
        dy = j + (tr.y / 64);
        value = mono_bitmap[dy * *width + dx];
        value += ftbitmap.buffer[j * ftbitmap.pitch + k];
        if (value > 255) {
          value = 255;
        }
        mono_bitmap[dy * *width + dx] = value;
      }
    }

    if (direction == GKS_K_TEXT_PATH_DOWN) {
      pen.x -= face->glyph->advance.x + spacing.x;
      pen.y -= face->glyph->advance.y + spacing.y;
    } else {
      pen.x += face->glyph->advance.x + spacing.x;
      pen.y += face->glyph->advance.y + spacing.y;
    }
  }

  gks_inq_rgb(gkss->txcoli, &red, &green, &blue);
  color[0] = (int)(red * 255);
  color[1] = (int)(green * 255);
  color[2] = (int)(blue * 255);

  rgba_bitmap = (FT_Byte *) safe_realloc(rgba_bitmap, 4 * size);
  memset(rgba_bitmap, 0, 4 * size);
  for (i = 0; i < size; i++) {
    for (j = 0; j < 3; j++) {
      tmp = rgba_bitmap[4*i + j] + color[j] * mono_bitmap[i]/255;
      rgba_bitmap[4*i + j] = (FT_Byte) min(tmp, 255);
    }
    tmp = rgba_bitmap[4*i + 3] + mono_bitmap[i];
    rgba_bitmap[4*i + 3] = (FT_Byte) min(tmp, 255);
  }

  if (right.x!=0 || right.y!=0) {
    FT_Vector_Transform(&right, &rotation);
  }
  if (vertical) {
    pen.x = right.x;
    pen.y = right.y;
  }

  if (halign == GKS_K_TEXT_HALIGN_LEFT) {
    align.x = 0;
    align.y = 0;
  } else if (halign == GKS_K_TEXT_HALIGN_CENTER) {
    align.x = pen.x / 2;
    align.y = pen.y / 2;
  } else { /* right justified */
    align.x = pen.x;
    align.y = pen.y;
  }
  if (valign != GKS_K_TEXT_VALIGN_BASE) {
    point.x = 0;
    point.y = gkss->chh * windowheight * 64;
    FT_Vector_Transform(&point, &rotation);
    if (valign == GKS_K_TEXT_VALIGN_CAP) {
      align.x += point.x;
      align.y += point.y;
    } else if (valign == GKS_K_TEXT_VALIGN_HALF) {
      align.x += point.x * 0.5;
      align.y += point.y * 0.5;
    } else if (valign == GKS_K_TEXT_VALIGN_TOP) {
      align.x += (point.x * 1.2);
      align.y += (point.y * 1.2);
    } else if (valign == GKS_K_TEXT_VALIGN_BOTTOM) {
      align.x -= (point.x * 0.2);
      align.y -= (point.y * 0.2);
    }
  }
  *x += (xmin - align.x) / 64;
  *y += (ymin - align.y) / 64;

  free(mono_bitmap);
  return (int *) rgba_bitmap;
}

#else

static int init = 0;

int gks_ft_init(void)
{
  gks_perror("FreeType support not compiled in");
  init = 1;
  return 1;
}

int *gks_ft_render(int *x, int *y, int *width, int *height,
                   gks_state_list_t *gkss, const char *text, int length)
{
  if (!init) gks_ft_init();
  return NULL;
}

void gks_ft_terminate(void)
{
}

#endif

