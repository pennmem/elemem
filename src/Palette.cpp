#include "Palette.h"
#include <math.h>

using namespace std;


namespace CML {
  Palette::Palette(PaletteType pt, int new_size) {
    palette_size = new_size;
    palette_scale = 0.9999999 * palette_size;

    pal_table = Data1D<Color>(palette_size);
    pal_table_sqrt = Data1D<Color>(palette_size);
    color_data = Data2D<u8>(3, palette_size);
    color_data_sqrt = Data2D<u8>(3, palette_size);
    argb_table = Data1D<u32>(palette_size);
    argb_table_sqrt = Data1D<u32>(palette_size);
    bg_data = Data1D<u8>(3);
    fg_data = Data1D<u8>(3);

    // maxRGBval = 255.9999;
    maxRGBval = 1.0;

    palette_type = pt;
    SetColors();
  }


  Palette::~Palette() {
  }


  void Palette::SetPalette(PaletteType pt) {
    if (pt == palette_type) {
      return;
    }

    palette_type = pt;

    SetColors();
  }


  void Palette::SetColors() {
    switch(palette_type) {
      case BGRSpectrum:
        SetBGR();
        break;
      case Grayscale:
        SetGrayscale();
        break;
      case InverseBGR:
        SetInverseBGR();
        break;
      case InverseGray:
        SetInverseGray();
        break;
    }
  }


  void Palette::SetBGR() {
    int i;
    float val;
    float bright, sat, hue;
    float vr, vg, vb;

    bg.r = 0;
    bg.g = 0;
    bg.b = 0;
    fg.r = maxRGBval;
    fg.g = maxRGBval;
    fg.b = maxRGBval;
    bg_data[0] = 0;
    bg_data[1] = 0;
    bg_data[2] = 0;
    fg_data[0] = 255;
    fg_data[1] = 255;
    fg_data[2] = 255;
    bg_argb = NormToARGB(0,0,0);
    fg_argb = NormToARGB(1,1,1);


    for (i=0; i<palette_size; i++) {
      val = i / (palette_size - 1.0);

      bright = 0.5;
      sat = 0.484375;
      hue = 0.66666666*(1-val) * 6.28318530717958;

      vr = (bright + sat*cos(hue));
      vg = (bright + sat*cos(hue + 4.188790204786387));
      vb = (bright + sat*cos(hue + 2.094395102393193));
      pal_table[i].r = maxRGBval * vr;
      pal_table[i].g = maxRGBval * vg;
      pal_table[i].b = maxRGBval * vb;
      color_data[i][0] = 255.999 * vr;
      color_data[i][1] = 255.999 * vg;
      color_data[i][2] = 255.999 * vb;
      argb_table[i] = NormToARGB(vr, vg, vb);

      val = sqrt(val);

      bright = 0.5;
      sat = 0.484375;
      hue = 0.66666666*(1-val) * 6.28318530717958;

      vr = (bright + sat*cos(hue));
      vg = (bright + sat*cos(hue + 4.188790204786387));
      vb = (bright + sat*cos(hue + 2.094395102393193));
      pal_table_sqrt[i].r = maxRGBval * vr;
      pal_table_sqrt[i].g = maxRGBval * vg;
      pal_table_sqrt[i].b = maxRGBval * vb;
      color_data_sqrt[i][0] = 255.999 * vr;
      color_data_sqrt[i][1] = 255.999 * vg;
      color_data_sqrt[i][2] = 255.999 * vb;
      argb_table_sqrt[i] = NormToARGB(vr, vg, vb);
    }
  }


  void Palette::SetGrayscale() {
    int i;
    float val;

    bg.r = 0;
    bg.g = 0;
    bg.b = 0;
    fg.r = maxRGBval;
    fg.g = maxRGBval;
    fg.b = maxRGBval;
    bg_data[0] = 0;
    bg_data[1] = 0;
    bg_data[2] = 0;
    fg_data[0] = 255;
    fg_data[1] = 255;
    fg_data[2] = 255;
    bg_argb = NormToARGB(0,0,0);
    fg_argb = NormToARGB(1,1,1);

    for (i=0; i<palette_size; i++) {
      val = i / (palette_size - 1.0);

      pal_table[i].r = val * maxRGBval;
      pal_table[i].g = val * maxRGBval;
      pal_table[i].b = val * maxRGBval;
      color_data[i][0] = 255.999 * val;
      color_data[i][1] = 255.999 * val;
      color_data[i][2] = 255.999 * val;
      argb_table[i] = NormToARGB(val, val, val);

      val = sqrt(val);

      pal_table_sqrt[i].r = val * maxRGBval;
      pal_table_sqrt[i].g = val * maxRGBval;
      pal_table_sqrt[i].b = val * maxRGBval;
      color_data_sqrt[i][0] = 255.999 * val;
      color_data_sqrt[i][1] = 255.999 * val;
      color_data_sqrt[i][2] = 255.999 * val;
      argb_table_sqrt[i] = NormToARGB(val, val, val);
    }
  }


  void Palette::SetInverseGray() {
    int i;
    float val;

    bg.r = maxRGBval;
    bg.g = maxRGBval;
    bg.b = maxRGBval;
    fg.r = 0;
    fg.g = 0;
    fg.b = 0;
    bg_data[0] = 255;
    bg_data[1] = 255;
    bg_data[2] = 255;
    fg_data[0] = 0;
    fg_data[1] = 0;
    fg_data[2] = 0;
    bg_argb = NormToARGB(1,1,1);
    fg_argb = NormToARGB(0,0,0);

    for (i=0; i<palette_size; i++) {
      val = 1 - i / (palette_size - 1.0);

      pal_table[i].r = val * maxRGBval;
      pal_table[i].g = val * maxRGBval;
      pal_table[i].b = val * maxRGBval;
      color_data[i][0] = 255.999 * val;
      color_data[i][1] = 255.999 * val;
      color_data[i][2] = 255.999 * val;
      argb_table[i] = NormToARGB(val, val, val);

      val = 1-sqrt(i / (palette_size - 1.0));

      pal_table_sqrt[i].r = val * maxRGBval;
      pal_table_sqrt[i].g = val * maxRGBval;
      pal_table_sqrt[i].b = val * maxRGBval;
      color_data_sqrt[i][0] = 255.999 * val;
      color_data_sqrt[i][1] = 255.999 * val;
      color_data_sqrt[i][2] = 255.999 * val;
      argb_table_sqrt[i] = NormToARGB(val, val, val);
    }
  }


  void Palette::SetInverseBGR() {
    int i;
    float val;
    float bright, sat, hue;
    float vr, vg, vb;

    bg.r = maxRGBval;
    bg.g = maxRGBval;
    bg.b = maxRGBval;
    fg.r = 0;
    fg.g = 0;
    fg.b = 0;
    bg_data[0] = 255;
    bg_data[1] = 255;
    bg_data[2] = 255;
    fg_data[0] = 0;
    fg_data[1] = 0;
    fg_data[2] = 0;
    bg_argb = NormToARGB(1,1,1);
    fg_argb = NormToARGB(0,0,0);

    for (i=0; i<palette_size; i++) {
      val = i / (palette_size - 1.0);

      bright = 0.5;
      sat = 0.484375;
      hue = 0.66666666*(1-val) * 6.28318530717958;

      vr = (bright + sat*cos(hue));
      vg = (bright + sat*cos(hue + 4.188790204786387));
      vb = (bright + sat*cos(hue + 2.094395102393193));
      pal_table[i].r = maxRGBval * vr;
      pal_table[i].g = maxRGBval * vg;
      pal_table[i].b = maxRGBval * vb;
      color_data[i][0] = 255.999 * vr;
      color_data[i][1] = 255.999 * vg;
      color_data[i][2] = 255.999 * vb;
      argb_table[i] = NormToARGB(vr, vg, vb);

      val = sqrt(val);

      bright = 0.5;
      sat = 0.484375;
      hue = 0.66666666*(1-val) * 6.28318530717958;

      vr = (bright + sat*cos(hue));
      vg = (bright + sat*cos(hue + 4.188790204786387));
      vb = (bright + sat*cos(hue + 2.094395102393193));
      pal_table_sqrt[i].r = maxRGBval * vr;
      pal_table_sqrt[i].g = maxRGBval * vg;
      pal_table_sqrt[i].b = maxRGBval * vb;
      color_data_sqrt[i][0] = 255.999 * vr;
      color_data_sqrt[i][1] = 255.999 * vg;
      color_data_sqrt[i][2] = 255.999 * vb;
      argb_table_sqrt[i] = NormToARGB(vr, vg, vb);
    }
  }


  // Returns a color by index number which has a good hue distance from
  // all the other colors of lesser index value.
  Color Palette::GetSpacedColor(u32 index) const {
    float bright, sat, hue;

#define HUE_TABLE_SIZE 256
    float hue_table[HUE_TABLE_SIZE] = { 0.000000, 3.141593, 1.570796, 4.712389,
      0.785398, 3.926991, 2.356194, 5.497787, 0.392699, 3.534292, 1.963495,
      5.105088, 1.178097, 4.319690, 2.748894, 5.890486, 0.196350, 3.337942,
      1.767146, 4.908739, 0.981748, 4.123340, 2.552544, 5.694137, 0.589049,
      3.730641, 2.159845, 5.301438, 1.374447, 4.516039, 2.945243, 6.086836,
      0.098175, 3.239767, 1.668971, 4.810564, 0.883573, 4.025166, 2.454369,
      5.595962, 0.490874, 3.632467, 2.061670, 5.203263, 1.276272, 4.417865,
      2.847068, 5.988661, 0.294524, 3.436117, 1.865321, 5.006913, 1.079922,
      4.221515, 2.650719, 5.792311, 0.687223, 3.828816, 2.258020, 5.399612,
      1.472622, 4.614214, 3.043418, 6.185011, 0.049087, 3.190680, 1.619884,
      4.761476, 0.834486, 3.976078, 2.405282, 5.546875, 0.441786, 3.583379,
      2.012583, 5.154175, 1.227185, 4.368777, 2.797981, 5.939574, 0.245437,
      3.387030, 1.816233, 4.957826, 1.030835, 4.172428, 2.601631, 5.743224,
      0.638136, 3.779729, 2.208932, 5.350525, 1.423534, 4.565127, 2.994330,
      6.135923, 0.147262, 3.288855, 1.718058, 4.859651, 0.932660, 4.074253,
      2.503457, 5.645049, 0.539961, 3.681554, 2.110758, 5.252350, 1.325359,
      4.466952, 2.896156, 6.037748, 0.343612, 3.485204, 1.914408, 5.056001,
      1.129010, 4.270603, 2.699806, 5.841399, 0.736311, 3.877903, 2.307107,
      5.448700, 1.521709, 4.663302, 3.092505, 6.234098, 0.024544, 3.166136,
      1.595340, 4.736933, 0.809942, 3.951535, 2.380738, 5.522331, 0.417243,
      3.558835, 1.988039, 5.129632, 1.202641, 4.344234, 2.773437, 5.915030,
      0.220893, 3.362486, 1.791690, 4.933282, 1.006291, 4.147884, 2.577088,
      5.718680, 0.613592, 3.755185, 2.184389, 5.325981, 1.398990, 4.540583,
      2.969787, 6.111379, 0.122718, 3.264311, 1.693515, 4.835107, 0.908117,
      4.049709, 2.478913, 5.620506, 0.515418, 3.657010, 2.086214, 5.227807,
      1.300816, 4.442408, 2.871612, 6.013205, 0.319068, 3.460661, 1.889864,
      5.031457, 1.104466, 4.246059, 2.675262, 5.816855, 0.711767, 3.853360,
      2.282563, 5.424156, 1.497165, 4.638758, 3.067962, 6.209554, 0.073631,
      3.215224, 1.644427, 4.786020, 0.859029, 4.000622, 2.429826, 5.571418,
      0.466330, 3.607923, 2.037126, 5.178719, 1.251728, 4.393321, 2.822525,
      5.964117, 0.269981, 3.411573, 1.840777, 4.982370, 1.055379, 4.196971,
      2.626175, 5.767768, 0.662680, 3.804272, 2.233476, 5.375069, 1.448078,
      4.589671, 3.018874, 6.160467, 0.171806, 3.313399, 1.742602, 4.884195,
      0.957204, 4.098797, 2.528000, 5.669593, 0.564505, 3.706098, 2.135301,
      5.276894, 1.349903, 4.491496, 2.920699, 6.062292, 0.368155, 3.509748,
      1.938952, 5.080544, 1.153554, 4.295146, 2.724350, 5.865943, 0.760854,
      3.902447, 2.331651, 5.473243, 1.546253, 4.687845, 3.117049, 6.258642 };

    bright = 0.5;
    sat = 0.484375;

    hue = hue_table[index % HUE_TABLE_SIZE];

    Color c;
    c.r = maxRGBval * (bright + sat * cos(hue));
    c.g = maxRGBval * (bright + sat * cos(hue + 4.188790204786387));
    c.b = maxRGBval * (bright + sat * cos(hue + 2.094395102393193));
    return c;
  }

  Data1D<u8> Palette::GetSpacedColorData(u32 index) const {
    Color c;
    c = GetSpacedColor(index);

    Data1D<u8> retval(3);
    retval[0] = 255.999 * c.r / maxRGBval;
    retval[1] = 255.999 * c.g / maxRGBval;
    retval[2] = 255.999 * c.b / maxRGBval;

    return retval;
  }

  u32 Palette::GetSpacedColorARGB(u32 index) const {
    Color c;
    c = GetSpacedColor(index);

    return NormToARGB(c.r, c.g, c.b);
  }
}

