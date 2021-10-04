#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gkscore.h"


#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#define NUM_CODEPOINTS 283

short *offsets = NULL;

static void init_offsets(void)
{
  const char *path;
  char offsets_path[MAXPATHLEN];

  path = gks_getenv("GKS_FONTPATH");
  if (path == NULL)
    {
      path = gks_getenv("GRDIR");
      if (path == NULL) path = GRDIR;
    }
  strcpy(offsets_path, (char *)path);
#ifndef _WIN32
  strcat(offsets_path, "/fonts/mathtex2_offsets.bin");
#else
  strcat(offsets_path, "\\FONTS\\MATHTEX2_OFFSETS.BIN");
#endif
  FILE *offsets_fp = fopen(offsets_path, "rb");
  if (!offsets_fp)
    {
      return;
    }
  offsets = malloc(sizeof(short) * NUM_CODEPOINTS * NUM_CODEPOINTS);
  if (!offsets)
    {
      fclose(offsets_fp);
      return;
    }
  fread(offsets, sizeof(short), NUM_CODEPOINTS * NUM_CODEPOINTS, offsets_fp);
  fclose(offsets_fp);
}

static int get_index(unsigned int codepoint);

double gr_get_kerning_offset(unsigned int left_codepoint, unsigned int right_codepoint)
{
  int left_index, right_index;
  left_index = get_index(left_codepoint);
  short tmp;
  if (left_index < 0)
    {
      return 0;
    }
  right_index = get_index(right_codepoint);
  if (right_index < 0)
    {
      return 0;
    }
  if (!offsets)
    {
      init_offsets();
    }
  if (!offsets)
    {
      return 0;
    }
  return offsets[left_index * NUM_CODEPOINTS + right_index] / 213.3333;
}

static int get_index(unsigned int codepoint)
{
  /* kerning offsets can be disabled for codepoints by adding them here */
  switch (codepoint)
    {
    case '.':
    case ',':
    case ';':
    case '!':
      return -1;
    }

  switch (codepoint)
    {
    case 35:
      return 0;
    case 36:
      return 1;
    case 37:
      return 2;
    case 39:
      return 3;
    case 40:
      return 4;
    case 41:
      return 5;
    case 42:
      return 6;
    case 43:
      return 7;
    case 44:
      return 8;
    case 45:
      return 9;
    case 46:
      return 10;
    case 48:
      return 11;
    case 49:
      return 12;
    case 50:
      return 13;
    case 51:
      return 14;
    case 52:
      return 15;
    case 53:
      return 16;
    case 54:
      return 17;
    case 55:
      return 18;
    case 56:
      return 19;
    case 57:
      return 20;
    case 58:
      return 21;
    case 60:
      return 22;
    case 61:
      return 23;
    case 63:
      return 24;
    case 64:
      return 25;
    case 65:
      return 26;
    case 66:
      return 27;
    case 67:
      return 28;
    case 68:
      return 29;
    case 69:
      return 30;
    case 70:
      return 31;
    case 71:
      return 32;
    case 72:
      return 33;
    case 73:
      return 34;
    case 74:
      return 35;
    case 75:
      return 36;
    case 76:
      return 37;
    case 77:
      return 38;
    case 78:
      return 39;
    case 79:
      return 40;
    case 80:
      return 41;
    case 81:
      return 42;
    case 82:
      return 43;
    case 83:
      return 44;
    case 84:
      return 45;
    case 85:
      return 46;
    case 86:
      return 47;
    case 87:
      return 48;
    case 88:
      return 49;
    case 89:
      return 50;
    case 90:
      return 51;
    case 91:
      return 52;
    case 92:
      return 53;
    case 93:
      return 54;
    case 95:
      return 55;
    case 96:
      return 56;
    case 97:
      return 57;
    case 98:
      return 58;
    case 99:
      return 59;
    case 100:
      return 60;
    case 101:
      return 61;
    case 102:
      return 62;
    case 103:
      return 63;
    case 104:
      return 64;
    case 105:
      return 65;
    case 106:
      return 66;
    case 107:
      return 67;
    case 108:
      return 68;
    case 109:
      return 69;
    case 110:
      return 70;
    case 111:
      return 71;
    case 112:
      return 72;
    case 113:
      return 73;
    case 114:
      return 74;
    case 115:
      return 75;
    case 116:
      return 76;
    case 117:
      return 77;
    case 118:
      return 78;
    case 119:
      return 79;
    case 120:
      return 80;
    case 121:
      return 81;
    case 122:
      return 82;
    case 124:
      return 83;
    case 161:
      return 84;
    case 167:
      return 85;
    case 169:
      return 86;
    case 172:
      return 87;
    case 177:
      return 88;
    case 182:
      return 89;
    case 183:
      return 90;
    case 191:
      return 91;
    case 197:
      return 92;
    case 198:
      return 93;
    case 215:
      return 94;
    case 216:
      return 95;
    case 223:
      return 96;
    case 230:
      return 97;
    case 247:
      return 98;
    case 305:
      return 99;
    case 321:
      return 100;
    case 338:
      return 101;
    case 339:
      return 102;
    case 567:
      return 103;
    case 824:
      return 104;
    case 915:
      return 105;
    case 916:
      return 106;
    case 920:
      return 107;
    case 923:
      return 108;
    case 926:
      return 109;
    case 928:
      return 110;
    case 931:
      return 111;
    case 933:
      return 112;
    case 934:
      return 113;
    case 936:
      return 114;
    case 937:
      return 115;
    case 945:
      return 116;
    case 946:
      return 117;
    case 947:
      return 118;
    case 948:
      return 119;
    case 949:
      return 120;
    case 950:
      return 121;
    case 951:
      return 122;
    case 952:
      return 123;
    case 953:
      return 124;
    case 954:
      return 125;
    case 956:
      return 126;
    case 957:
      return 127;
    case 958:
      return 128;
    case 960:
      return 129;
    case 961:
      return 130;
    case 962:
      return 131;
    case 963:
      return 132;
    case 964:
      return 133;
    case 965:
      return 134;
    case 966:
      return 135;
    case 967:
      return 136;
    case 968:
      return 137;
    case 969:
      return 138;
    case 977:
      return 139;
    case 981:
      return 140;
    case 982:
      return 141;
    case 1009:
      return 142;
    case 8214:
      return 143;
    case 8216:
      return 144;
    case 8217:
      return 145;
    case 8220:
      return 146;
    case 8221:
      return 147;
    case 8224:
      return 148;
    case 8225:
      return 149;
    case 8230:
      return 150;
    case 8242:
      return 151;
    case 8465:
      return 152;
    case 8467:
      return 153;
    case 8472:
      return 154;
    case 8476:
      return 155;
    case 8501:
      return 156;
    case 8592:
      return 157;
    case 8593:
      return 158;
    case 8594:
      return 159;
    case 8595:
      return 160;
    case 8596:
      return 161;
    case 8597:
      return 162;
    case 8598:
      return 163;
    case 8599:
      return 164;
    case 8600:
      return 165;
    case 8601:
      return 166;
    case 8614:
      return 167;
    case 8617:
      return 168;
    case 8618:
      return 169;
    case 8636:
      return 170;
    case 8637:
      return 171;
    case 8640:
      return 172;
    case 8641:
      return 173;
    case 8652:
      return 174;
    case 8656:
      return 175;
    case 8657:
      return 176;
    case 8658:
      return 177;
    case 8659:
      return 178;
    case 8660:
      return 179;
    case 8661:
      return 180;
    case 8704:
      return 181;
    case 8706:
      return 182;
    case 8707:
      return 183;
    case 8711:
      return 184;
    case 8712:
      return 185;
    case 8713:
      return 186;
    case 8715:
      return 187;
    case 8719:
      return 188;
    case 8720:
      return 189;
    case 8721:
      return 190;
    case 8723:
      return 191;
    case 8725:
      return 192;
    case 8727:
      return 193;
    case 8728:
      return 194;
    case 8729:
      return 195;
    case 8734:
      return 196;
    case 8736:
      return 197;
    case 8739:
      return 198;
    case 8741:
      return 199;
    case 8743:
      return 200;
    case 8744:
      return 201;
    case 8745:
      return 202;
    case 8746:
      return 203;
    case 8747:
      return 204;
    case 8750:
      return 205;
    case 8764:
      return 206;
    case 8768:
      return 207;
    case 8771:
      return 208;
    case 8773:
      return 209;
    case 8776:
      return 210;
    case 8781:
      return 211;
    case 8784:
      return 212;
    case 8800:
      return 213;
    case 8801:
      return 214;
    case 8804:
      return 215;
    case 8805:
      return 216;
    case 8810:
      return 217;
    case 8811:
      return 218;
    case 8826:
      return 219;
    case 8827:
      return 220;
    case 8828:
      return 221;
    case 8829:
      return 222;
    case 8834:
      return 223;
    case 8835:
      return 224;
    case 8838:
      return 225;
    case 8839:
      return 226;
    case 8846:
      return 227;
    case 8849:
      return 228;
    case 8850:
      return 229;
    case 8851:
      return 230;
    case 8852:
      return 231;
    case 8853:
      return 232;
    case 8854:
      return 233;
    case 8855:
      return 234;
    case 8856:
      return 235;
    case 8857:
      return 236;
    case 8866:
      return 237;
    case 8867:
      return 238;
    case 8868:
      return 239;
    case 8869:
      return 240;
    case 8871:
      return 241;
    case 8896:
      return 242;
    case 8897:
      return 243;
    case 8898:
      return 244;
    case 8899:
      return 245;
    case 8900:
      return 246;
    case 8901:
      return 247;
    case 8902:
      return 248;
    case 8904:
      return 249;
    case 8942:
      return 250;
    case 8943:
      return 251;
    case 8945:
      return 252;
    case 8968:
      return 253;
    case 8969:
      return 254;
    case 8970:
      return 255;
    case 8971:
      return 256;
    case 8994:
      return 257;
    case 8995:
      return 258;
    case 9651:
      return 259;
    case 9655:
      return 260;
    case 9661:
      return 261;
    case 9665:
      return 262;
    case 9675:
      return 263;
    case 9824:
      return 264;
    case 9825:
      return 265;
    case 9826:
      return 266;
    case 9827:
      return 267;
    case 9837:
      return 268;
    case 9838:
      return 269;
    case 9839:
      return 270;
    case 10178:
      return 271;
    case 10229:
      return 272;
    case 10230:
      return 273;
    case 10231:
      return 274;
    case 10232:
      return 275;
    case 10233:
      return 276;
    case 10234:
      return 277;
    case 10236:
      return 278;
    case 10752:
      return 279;
    case 10753:
      return 280;
    case 10754:
      return 281;
    case 10756:
      return 282;
    default:
      return -1;
    }
}
