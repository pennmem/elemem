#ifndef PALETTE_H
#define PALETTE_H

#define OFF_PALETTE_VAL    (-2e30)
#define PALETTE_VAL_THRESH (-1e30)

#include "RC/RC.h"
#include <math.h>


using namespace std;
using namespace RC;


namespace CML {
  enum PaletteType { BGRSpectrum, Grayscale, InverseBGR, InverseGray };

  class Color {
    public:
      float r;
      float g;
      float b;
  };

  class Palette {
    public:

    Palette(PaletteType pt = BGRSpectrum, int new_size = 2048);
    ~Palette();

    Color GetSpacedColor(u32 index) const;
    Data1D<u8> GetSpacedColorData(u32 index) const;
    u32 GetSpacedColorARGB(u32 index) const;
    inline const Color& GetBG() const { return bg; }
    inline const Color& GetFG() const { return fg; }
    inline const Data1D<u8>& GetBGData() const { return bg_data; }
    inline const Data1D<u8>& GetFGData() const { return fg_data; }
    inline u32 GetBG_ARGB() const { return bg_argb; }
    inline u32 GetFG_ARGB() const { return fg_argb; }
    inline u32 GetBG_ARGB(f32 alpha) const {
      return AddAlphaVal(GetBG_ARGB(), alpha);
    }
    inline u32 GetFG_ARGB(f32 alpha) const {
      return AddAlphaVal(GetFG_ARGB(), alpha);
    }

    inline bool IsBG(float val) const {
      if ( ! (val > PALETTE_VAL_THRESH) ) {
        return true;
      }
      return false;
    }

    inline size_t ValToIndex(float val) const {
      if (val > 1) { return palette_size-1; }
      else if (val >= 0) { return (int)(val * palette_scale); }
      else {
        return 0;
      }
    }


    void SetPalette(PaletteType pt);
    inline PaletteType GetPaletteType() {
      return palette_type;
    }


    inline u32 GetARGBLinear(float val, u8 alpha) const {
      if (IsBG(val)) {
        return bg_argb;
      }
      else {
        return AddAlphaARGB(argb_table[ValToIndex(val)], alpha);
      }
    }


    inline u32 GetARGBSqrt(float val, u8 alpha) const {
      if (IsBG(val)) {
        return bg_argb;
      }
      else {
        return AddAlphaARGB(argb_table_sqrt[ValToIndex(val)], alpha);
      }
    }


    inline u32 GetARGB(float val, u8 alpha=255, bool dosqrt=false) const {
      if (dosqrt) {
        return GetARGBSqrt(val, alpha);
      }
      else {
        return GetARGBLinear(val, alpha);
      }
    }

    // val 0-1, rgb 0-1
    inline const Color& GetRGBSqrt(float val) const {
      if (IsBG(val)) {
        return GetBG();
      }
      else {
        return pal_table_sqrt[ValToIndex(val)];
      }
    }

    // val 0-1, rgb 0-1
    inline const Color& GetRGBLinear(float val) const {
      if (IsBG(val)) {
        return GetBG();
      }
      else {
        return pal_table[ValToIndex(val)];
      }
    }

    // val 0-1, rgb 0-1
    inline Color GetRGB(float val, bool dosqrt=false) const {
      if (dosqrt) {
        return GetRGBSqrt(val);
      }
      else {
        return GetRGBLinear(val);
      }
    }


    // val 0-1, returns 3 byte 0-255 RGB val
    inline const Data1D<u8>& GetColorDataSqrt(float val) const {
      if (IsBG(val)) {
        return GetBGData();
      }
      else {
        return color_data_sqrt[ValToIndex(val)];
      }
    }

    // val 0-1, returns 3 byte 0-255 RGB val
    inline const Data1D<u8>& GetColorDataLinear(float val) const {
      if (IsBG(val)) {
        return GetBGData();
      }
      else {
        return color_data[ValToIndex(val)];
      }
    }

    // val 0-1, returns 3 byte 0-255 RGB val
    inline const Data1D<u8>& GetColorData(float val, bool dosqrt=false) const {
      if (dosqrt) {
        return GetColorDataSqrt(val);
      }
      else {
        return GetColorDataLinear(val);
      }
    }

    inline u32 NormTo256(f32 val) const { return 0xff & (u32)(255.999 * val); }

    inline u32 NormToARGB(f32 r, f32 g, f32 b, f32 a=1) const {
      return (NormTo256(a) << 24) + (NormTo256(r) << 16) +
             (NormTo256(g) << 8) + NormTo256(b);
    }

    inline u32 AddAlphaARGB(u32 val, u8 alpha) const {
      return (val & 0xffffff) + (((u32)alpha) << 24);
    }

    inline u32 AddAlphaVal(u32 val, f32 alpha) const {
      return AddAlphaARGB(val, NormTo256(alpha));
    }


    protected:

    void SetColors();
    void SetBGR();
    void SetGrayscale();
    void SetInverseBGR();
    void SetInverseGray();

    float maxRGBval;

    int palette_size;
    float palette_scale;
    Data1D<Color> pal_table;
    Data1D<Color> pal_table_sqrt;
    Data2D<u8> color_data;
    Data2D<u8> color_data_sqrt;
    Data1D<u32> argb_table;
    Data1D<u32> argb_table_sqrt;
    Color bg;
    Color fg;
    Data1D<u8> bg_data;
    Data1D<u8> fg_data;
    u32 bg_argb;
    u32 fg_argb;
    PaletteType palette_type;
  };
}

#endif // PALETTE_H

