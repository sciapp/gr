#include <stdlib.h>
#include <math.h>

#include "gks.h"
#include "gkscore.h"

#ifndef NO_FT

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_XFREE86_H

#define nint(a) ((int)(a + 0.5))

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

static
FT_Face font_face_cache[] = {
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

const static int map[] = {
  22, 9, 5, 14, 18, 26, 13, 1,
  24, 11, 7, 16, 20, 28, 13, 3,
  23, 10, 6, 15, 19, 27, 13, 2,
  25, 12, 8, 17, 21, 29, 13, 4
};

const static double caps[] = {
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
static FT_Error set_glyph(FT_Face face, FT_UInt codepoint, FT_UInt *previous,
                          FT_Vector *pen, FT_Bool vertical, FT_Matrix *rotation,
                          FT_Vector *bearing, FT_Int halign);
static void utf_to_unicode(FT_Bytes str, FT_UInt *unicode_string, int *length);
static FT_Long ft_min(FT_Long a, FT_Long b);
static FT_Long ft_max(FT_Long a, FT_Long b);

static FT_Long ft_min(FT_Long a, FT_Long b) {
  return a > b ? b : a;
}

static FT_Long ft_max(FT_Long a, FT_Long b) {
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
    gks_perror("out of memory");
    ptr = NULL;
  }
  return ptr;
}

/* load a glyph into the slot and compute bearing */
static FT_Error set_glyph(FT_Face face, FT_UInt codepoint, FT_UInt *previous,
                          FT_Vector *pen, FT_Bool vertical, FT_Matrix *rotation,
                          FT_Vector *bearing, FT_Int halign) {
  FT_Error error;
  FT_UInt glyph_index;

  glyph_index = FT_Get_Char_Index(face, codepoint);
  if (FT_HAS_KERNING(face) && *previous && !vertical && glyph_index) {
    FT_Vector delta;
    FT_Get_Kerning(face, *previous, glyph_index, FT_KERNING_DEFAULT,
                   &delta);
    FT_Vector_Transform(&delta, rotation);
    pen->x += delta.x;
    pen->y += delta.y;
  }
  error = FT_Load_Glyph(face, glyph_index, vertical ?
                        FT_LOAD_VERTICAL_LAYOUT : FT_LOAD_DEFAULT);
  if (error) {
    gks_perror("glyph could not be loaded: %c", codepoint);
    return 1;
  }
  error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
  if (error) {
    gks_perror("glyph could not be rendered: %c", codepoint);
    return 1;
  }
  *previous = glyph_index;

  bearing->x = face->glyph->metrics.horiBearingX;
  bearing->y = 0;
  if (vertical) {
    if (halign == GKS_K_TEXT_HALIGN_RIGHT) {
      bearing->x += face->glyph->metrics.width;
    } else if (halign == GKS_K_TEXT_HALIGN_CENTER) {
      bearing->x += face->glyph->metrics.width / 2;
    }
    if (bearing->x != 0) FT_Vector_Transform(bearing, rotation);
    bearing->x = 64 * face->glyph->bitmap_left - bearing->x;
    bearing->y = 64 * face->glyph->bitmap_top  - bearing->y;
  } else {
    if (bearing->x != 0) FT_Vector_Transform(bearing, rotation);
    pen->x -= bearing->x;
    pen->y -= bearing->y;
    bearing->x = 64 * face->glyph->bitmap_left;
    bearing->y = 64 * face->glyph->bitmap_top;
  }
  return 0;
}


static
void symbol_to_unicode(FT_Bytes str, FT_UInt *unicode_string, int length)
{
  static int symbol2utf[256] = {
    0,     1,     2,     3,     4,     5,     6,     7,
    8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    18,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,  8704,    35,  8707,    37,    38,  8715,
    40,    41,    42,    43,    44,    45,    46,    47,
    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,
    8773,   913,   914,   935,   916,   917,   934,   915,
    919,   921,   977,   922,   923,   924,   925,   927,
    928,   920,   929,   931,   932,   933,   962,   937,
    926,   936,   918,    91,  8756,    93,  8869,    95,
    8254,   945,   946,   967,   948,   949,   966,   947,
    951,   953,   981,   954,   955,   956,   957,   959,
    960,   952,   961,   963,   964,   965,   982,   969,
    958,   968,   950,   123,   124,   125,   126,   127,
    128,   129,   130,   131,   132,   133,   134,   135,
    136,   137,   138,   139,   140,   141,   142,   143,
    144,   145,   146,   147,   148,   149,   150,   151,
    152,   153,   154,   155,   156,   157,   158,   159,
    160,   978,  8242,  8804,  8260,  8734,   402,  9827,
    9830,  9829,  9824,  8596,  8592,  8593,  8594,  8595,
    176,   177,  8243,  8805,   215,  8733,  8706,  8226,
    247,  8800,  8801,  8776,  8230,  9116,  9135,  8629,
    8501,  8465,  8476,  8472,  8855,  8853,  8709,  8745,
    8746,  8835,  8839,  8836,  8834,  8838,  8712,  8713,
    8736,  8711,   174,   169,  8482,  8719,  8730,   183,
    172,  8743,  8744,  8660,  8656,  8657,  8658,  8659,
    9674, 12296,   174,   169,  8482,  8721,  9115,  9116,
    9117,  9121,  9116,  9123,  9127,  9128,  9129,  9116,
    240, 12297,  8747,  9127,  9116,  9133,  9131,  9130,
    9120,  9124,  9130,  9126,  9131,  9132,  9133,   255
  };
  int i;

  for (i = 0; i < length; i++)
    unicode_string[i] = symbol2utf[str[i]];
}

/* set text string, converting UTF-8 into Unicode */
static
void utf_to_unicode(FT_Bytes str, FT_UInt *unicode_string, int *length) {
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
      gks_perror("character ignored due to unicode error");
      continue;
    }
    codepoint = str[i] - offset;
    for (j = 0; j < following_bytes; j++) {
      codepoint = codepoint * 64;
      i++;
      if (str[i] < 128 || str[i] >= 128+64) {
        gks_perror("character ignored due to unicode error");
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
    gks_perror("could not initialize freetype library");
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

unsigned char *gks_ft_get_bitmap(int *x, int *y, int *width, int *height,
                                 gks_state_list_t *gkss, const char *text,
                                 int length) {
  FT_Face face;                   /* font face */
  FT_Vector pen;                  /* glyph position */
  FT_BBox bb;                     /* bounding box */
  FT_Vector bearing;              /* individual glyph translation */
  FT_UInt previous;               /* previous glyph index */
  FT_Vector spacing;              /* amount of additional space between glyphs */
  FT_ULong textheight;            /* textheight in FreeType convention */
  FT_Error error;                 /* error code */
  FT_Matrix rotation;             /* text rotation matrix */
  FT_UInt size;                   /* number of pixels of the bitmap */
  FT_String *file;                /* concatenated font path */
  const FT_String *font, *prefix; /* font file name and directory */
  FT_UInt *unicode_string;        /* unicode text string */
  FT_Int halign, valign;          /* alignment */
  FT_Byte *mono_bitmap = NULL;    /* target for rendered text */
  FT_Int num_glyphs;              /* number of glyphs */
  FT_Vector align;
  FT_Bitmap ftbitmap;
  FT_UInt codepoint;
  int i, textfont, dx, dy, value, pos_x, pos_y;
  unsigned int j, k;
  double angle;
  const int windowwidth = *width;
  const int direction = (gkss->txp <= 3 && gkss->txp >= 0 ? gkss->txp : 0);
  const FT_Bool vertical = (direction == GKS_K_TEXT_PATH_DOWN ||
                            direction == GKS_K_TEXT_PATH_UP);
  const FT_String *suffix_type1 = ".afm";

  if (!init) gks_ft_init();

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
  if (valign != GKS_K_TEXT_VALIGN_NORMAL) {
    valign = gkss->txal[1];
  } else {
    valign = GKS_K_TEXT_VALIGN_BASE;
  }

  textfont = abs(gkss->txfont);
  if (textfont >= 101 && textfont <= 131)
    textfont -= 100;
  else if (textfont > 1 && textfont <= 32)
    textfont = map[textfont - 1];
  else
    textfont = 9;

  textfont = textfont - 1;
  font = gks_font_list[textfont];

  if (font_face_cache[textfont] == NULL) {
    prefix = gks_getenv("GKS_FONTPATH");
    if (prefix == NULL) {
      prefix = gks_getenv("GRDIR");
      if (prefix == NULL)
        prefix = GRDIR;
    }
    file = (FT_String *) malloc(strlen(prefix) + 7 + strlen(font) + 4 + 1);
    strcpy(file, prefix);
#ifndef _WIN32
    strcat(file, "/fonts/");
#else
    strcat(file, "\\FONTS\\");
#endif
    strcat(file, font);
    strcat(file, ".pfb");
    error = FT_New_Face(library, file, 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
      gks_perror("unknown file format: %s", file);
      return NULL;
    } else if (error) {
      gks_perror("could not open font file: %s", file);
      return NULL;
    }
    if (strcmp(FT_Get_X11_Font_Format(face), "Type 1") == 0) {
      strcpy(file, prefix);
#ifndef _WIN32
      strcat(file, "/fonts/");
#else
      strcat(file, "\\FONTS\\");
#endif
      strcat(file, font);
      strcat(file, suffix_type1);
      FT_Attach_File(face, file);
    }
    free(file);
    font_face_cache[textfont] = face;
  } else {
    face = font_face_cache[textfont];
  }

  num_glyphs = length;
  unicode_string = (FT_UInt *) malloc(length * sizeof(FT_UInt) + 1);
  if (textfont + 1 == 13) {
    symbol_to_unicode((FT_Bytes)text, unicode_string, num_glyphs);
  } else {
    utf_to_unicode((FT_Bytes)text, unicode_string, &num_glyphs);
  }

  textheight = nint(gkss->chh * windowwidth * 64 / caps[textfont]);
  error = FT_Set_Char_Size(face, nint(textheight * gkss->chxp), textheight,
                           72, 72);
  if (error) gks_perror("cannot set text height");

  if (gkss->chup[0] != 0.0 || gkss->chup[1] != 0.0) {
    angle = atan2f(gkss->chup[1], gkss->chup[0]) - M_PI / 2;
    rotation.xx = nint( cosf(angle) * 0x10000L);
    rotation.xy = nint(-sinf(angle) * 0x10000L);
    rotation.yx = nint( sinf(angle) * 0x10000L);
    rotation.yy = nint( cosf(angle) * 0x10000L);
    FT_Set_Transform(face, &rotation, NULL);
  } else {
    FT_Set_Transform(face, NULL, NULL);
  }

  spacing.x = spacing.y = 0;
  if (gkss->chsp != 0.0) {
    error = FT_Load_Glyph(face, FT_Get_Char_Index(face, ' '),
                          vertical ? FT_LOAD_VERTICAL_LAYOUT : FT_LOAD_DEFAULT);
    if (!error) {
      spacing.x = nint(face->glyph->advance.x * gkss->chsp);
      spacing.y = nint(face->glyph->advance.y * gkss->chsp);
    } else {
      gks_perror("cannot apply character spacing");
    }
  }

  bb.xMin = bb.yMin = LONG_MAX;
  bb.xMax = bb.yMax = LONG_MIN;
  pen.x = pen.y = 0;
  previous = 0;

  for (i = 0; i < num_glyphs; i++) {
    codepoint = unicode_string[direction == GKS_K_TEXT_PATH_LEFT ?
                               (num_glyphs - 1 - i) : i];
    error = set_glyph(face, codepoint, &previous, &pen, vertical, &rotation,
                      &bearing, halign);
    if (error) continue;

    bb.xMin = ft_min(bb.xMin, pen.x + bearing.x);
    bb.xMax = ft_max(bb.xMax, pen.x + bearing.x + 64*face->glyph->bitmap.width);
    bb.yMin = ft_min(bb.yMin, pen.y + bearing.y - 64*face->glyph->bitmap.rows);
    bb.yMax = ft_max(bb.yMax, pen.y + bearing.y);

    if (direction == GKS_K_TEXT_PATH_DOWN) {
      pen.x -= face->glyph->advance.x + spacing.x;
      pen.y -= face->glyph->advance.y + spacing.y;
    } else {
      pen.x += face->glyph->advance.x + spacing.x;
      pen.y += face->glyph->advance.y + spacing.y;
    }
  }
  *width  = (int)((bb.xMax - bb.xMin) / 64);
  *height = (int)((bb.yMax - bb.yMin) / 64);
  if (bb.xMax <= bb.xMin || bb.yMax <= bb.yMin) {
    gks_perror("invalid bitmap size");
    free(unicode_string);
    return NULL;
  }
  size = *width * *height;
  mono_bitmap = (FT_Byte *) safe_realloc(mono_bitmap, size);
  memset(mono_bitmap, 0, size);

  pen.x = 0;
  pen.y = 0;
  previous = 0;

  for (i = 0; i < num_glyphs; i++) {
    bearing.x = bearing.y = 0;
    codepoint = unicode_string[direction == GKS_K_TEXT_PATH_LEFT ?
                               (num_glyphs - 1 - i) : i];
    error = set_glyph(face, codepoint, &previous, &pen, vertical, &rotation,
                      &bearing, halign);
    if (error) continue;

    pos_x = ( pen.x + bearing.x - bb.xMin) / 64;
    pos_y = (-pen.y - bearing.y + bb.yMax) / 64;
    ftbitmap = face->glyph->bitmap;
    for (j = 0; j < (unsigned int) ftbitmap.rows; j++) {
      for (k = 0; k < (unsigned int) ftbitmap.width; k++) {
        dx = k + pos_x;
        dy = j + pos_y;
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
  free(unicode_string);

  /* Alignment */
  if (direction == GKS_K_TEXT_PATH_DOWN) {
    pen.x += spacing.x;
    pen.y += spacing.y;
  } else {
    pen.x -= spacing.x;
    pen.y -= spacing.y;
  }

  align.x = align.y = 0;
  if (valign != GKS_K_TEXT_VALIGN_BASE) {
    align.y = nint(gkss->chh * windowwidth * 64);
    FT_Vector_Transform(&align, &rotation);
    if (valign == GKS_K_TEXT_VALIGN_HALF) {
      align.x = nint(0.5 * align.x);
      align.y = nint(0.5 * align.y);
    } else if (valign == GKS_K_TEXT_VALIGN_TOP) {
      align.x = nint(1.2 * align.x);
      align.y = nint(1.2 * align.y);
    } else if (valign == GKS_K_TEXT_VALIGN_BOTTOM) {
      align.x = nint(-0.2 * align.x);
      align.y = nint(-0.2 * align.y);
    }
  }

  if (!vertical && halign != GKS_K_TEXT_HALIGN_LEFT) {
    FT_Vector right;
    right.x = face->glyph->metrics.width + face->glyph->metrics.horiBearingX;
    right.y = 0;
    if (right.x != 0) {
      FT_Vector_Transform(&right, &rotation);
    }
    pen.x += right.x - face->glyph->advance.x;
    pen.y += right.y - face->glyph->advance.y;
    if (halign == GKS_K_TEXT_HALIGN_CENTER) {
      align.x += pen.x / 2;
      align.y += pen.y / 2;
    } else if (halign == GKS_K_TEXT_HALIGN_RIGHT) {
      align.x += pen.x;
      align.y += pen.y;
    }
  }

  *x += (bb.xMin - align.x) / 64;
  *y += (bb.yMin - align.y) / 64;
  return mono_bitmap;
}

int *gks_ft_render(int *x, int *y, int *width, int *height,
                   gks_state_list_t *gkss, const char *text, int length) {
  FT_Byte *rgba_bitmap = NULL;
  double red, green, blue;
  int tmp, size, i, j;
  int color[4];
  unsigned char *mono_bitmap;

  mono_bitmap = gks_ft_get_bitmap(x, y, width, height, gkss, text, length);
  gks_inq_rgb(gkss->txcoli, &red, &green, &blue);
  color[0] = (int)(red * 255);
  color[1] = (int)(green * 255);
  color[2] = (int)(blue * 255);
  color[3] = (int)(gkss->alpha * 255);

  size = *width * *height;
  rgba_bitmap = (FT_Byte *) safe_realloc(rgba_bitmap, 4 * size);
  memset(rgba_bitmap, 0, 4 * size);
  for (i = 0; i < size; i++) {
    for (j = 0; j < 4; j++) {
      tmp = rgba_bitmap[4*i + j] + color[j] * mono_bitmap[i] / 255;
      rgba_bitmap[4*i + j] = (FT_Byte) ft_min(tmp, 255);
    }
  }

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

