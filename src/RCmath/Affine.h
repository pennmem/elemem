#ifndef RCMATH_AFFINE_H
#define RCMATH_AFFINE_H

#include "../RC/Types.h"
#include "../RC/RStr.h"
#include "MathBits.h"
#include <iostream>

namespace RCmath {
  template <class T>
  class Affine {
    public:
    inline Affine() : a00(1), a01(0), a02(0), a10(0), a11(1), a12(0) {}

    T a00, a01, a02;
    T a10, a11, a12;

    inline Affine& I() {
      a00 = 1;  a01 = 0;  a02 = 0;
      a10 = 0;  a11 = 1;  a12 = 0;
      return *this;
    }


    // Note:  This left multiplies other.  (*this) = other * (*this);
    inline Affine& operator*= (const Affine& other) {
      T t00, t01, t02;
      t00 = other.a00*a00 + other.a01*a10;
      t01 = other.a00*a01 + other.a01*a11;
      t02 = other.a00*a02 + other.a01*a12 + other.a02;
      a10 = other.a10*a00 + other.a11*a10;
      a11 = other.a10*a01 + other.a11*a11;
      a12 = other.a10*a02 + other.a11*a12 + other.a12;
      a00 = t00;
      a01 = t01;
      a02 = t02;
      return (*this);
    }

    inline Affine operator* (const Affine& other) const {
      Affine tmp = other;
      tmp *= (*this);
      return tmp;
    }

    inline Affine& Rot(const T& angle) {
      Affine a;
      a.a00 = Cos(angle);  a.a01 = -Sin(angle);
      a.a10 = Sin(angle);  a.a11 = Cos(angle);
      return (*this *= a);
    }

    inline Affine& Shift(const T& x, const T& y) {
      Affine a;
      a.a00 = 1;              a.a02 = x;
                  a.a11 = 1;  a.a12 = y;
      return (*this *= a);
    }

    inline Affine& ShiftX(const T& x) {
      return Shift(x, 0);
    }

    inline Affine& ShiftY(const T& y) {
      return Shift(0, y);
    }

    inline Affine& ShearX(const T& shift_x_per_y) {
      Affine a;
      a.a00 = 1;  a.a01 = shift_x_per_y;
                  a.a11 = 1;
      return (*this *= a);
    }

    inline Affine& ShearY(const T& shift_y_per_x) {
      Affine a;
      a.a00 = 1;
      a.a10 = shift_y_per_x;  a.a11 = 1;
      return (*this *= a);
    }

    inline Affine& Scale(const T& x_factor, const T& y_factor) {
      Affine a;
      a.a00 = x_factor;
      a.a11 = y_factor;
      return (*this *= a);
    }

    inline Affine& ScaleX(const T& x_factor) {
      return Scale(x_factor, 1);
    }

    inline Affine& ScaleY(const T& y_factor) {
      return Scale(1, y_factor);
    }

    inline Affine& Scale(const T& factor) {
      return Scale(factor, factor);
    }


    inline RC::RStr ToRStr() const {
      size_t max0, max1, max2;
      RC::RStr r00(a00), r01(a01), r02(a02);
      RC::RStr r10(a10), r11(a11), r12(a12);
      max0 = Max(r00.size(), r10.size());
      max1 = Max(r01.size(), r11.size());
      max2 = Max(r02.size(), r12.size());
      r00.PadCenter(max0+2);
      r10.PadCenter(max0+2);
      r01.PadCenter(max1+2);
      r11.PadCenter(max1+2);
      r02.PadCenter(max2+2);
      r12.PadCenter(max2+2);
      return RC::RStr("/")+r00+r01+r02+"\\\n"+
                  "\\"+r10+r11+r12+"/";
    }
  };


  template<class T>
  inline std::ostream& operator<< (std::ostream &out, const Affine<T>& c) {
    out << c.ToRStr();
    return out;
  }
}


#endif // RCMATH_AFFINE_H

