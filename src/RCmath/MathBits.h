#ifndef RCMATH_MATHBITS_H
#define RCMATH_MATHBITS_H

#include "../RC/Types.h"
#include <cmath>
#include <complex>


#ifdef PI
#undef PI
#endif
#define PI 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798L

#ifdef TWO_PI
#undef TWO_PI
#endif
#define TWO_PI (2*PI)


namespace RCmath {
  template<class T> inline T Abs(T x) { return (((x)<0)?(-(x)):(x)); }
  template<class T> inline i64 Round(T x) {
    // Round 0.5 / -0.5 away from zero.  (neg. safe)
    return ((x)<0?(((u64)((x)-1/(T)2))):((u64)((x)+1/(T)2)));
  }

  template<class T>
  inline T Min(const T& x, const T& y) {
    return ( (y<x) ? y : x );
  }
  template<class T>
  inline T Max(const T& x, const T& y) {
    return ( (x<y) ? y : x );
  }
#ifdef CPP11
  template<class T1, class... Types>
  struct SumType {
    typedef T1 type;
  };
  template<class T1, class T2, class... Types>
  struct SumType<T1, T2, Types...> {
    typedef decltype(T1() + typename SumType<T2, Types...>::type()) type;
  };

  template<class T1>
  inline T1 Min(const T1& t1) {
    return t1;
  }
  template<class T1, class T2, class... Types>
  inline auto Min(const T1& t1, const T2& t2, const Types&... types)
    -> typename SumType<T1,T2,Types...>::type {
    return Min( ((t2<t1) ? t2 : t1), types... );
  }

  template<class T1>
  inline T1 Max(const T1& t1) {
    return t1;
  }
  template<class T1, class T2, class... Types>
  inline auto Max(const T1& t1, const T2& t2, const Types&... types)
    -> typename SumType<T1,T2,Types...>::type {
    return Max( ((t1<t2) ? t2 : t1), types... );
  }


  template<class T>
  class MaxVal {
    protected:
    T val;
    public:
    inline MaxVal() { val = RC::LOW_VAL<T>(); }
    inline MaxVal(const T& x) { val = x; }
    inline MaxVal& operator=(const T& x) { val = x; }
    inline MaxVal& MaxOf(const T& x) {
      val = Max(val, x);
      return *this;
    }
    inline operator T () const { return val; }
  };

  template<class T>
  class MinVal {
    protected:
    T val;
    public:
    inline MinVal() { val = RC::MAX_VAL<T>(); }
    inline MinVal(const T& x) { val = x; }
    inline MinVal& operator=(const T& x) { val = x; }
    inline MinVal& MinOf(const T& x) {
      val = Min(val, x);
      return *this;
    }
    inline operator T () const { return val; }
  };

#endif

  // This section provides overloads for many of the math.h function calls,
  // in a manner more friendly for templated functions.
  // Many of these are rendered obsolete by the new C++11 support for the same.
#define FDLDMathFunc(Name, name) \
  inline float Name(float x) { return name##f(x); } \
  inline double Name(double x) { return name(x); } \
  template<class T> \
  inline long double Name(T x) { return name##l((long double)x); }
#define FDLDMathFunc2(Name, name) \
  inline float Name(float x, float y) { return name##f(x,y); } \
  template<class T> \
  inline float Name(float x, T y) { return name##f(x,y); } \
  template<class T> \
  inline float Name(T x, float y) { return name##f(x,y); } \
  inline double Name(double x, double y) { return name(x,y); } \
  inline double Name(double x, float y) { return name(x,y); } \
  inline double Name(float x, double y) { return name(x,y); } \
  template<class T> \
  inline double Name(double x, T y) { return name(x,y); } \
  template<class T> \
  inline double Name(T x, double y) { return name(x,y); } \
  inline long double Name(long double x, long double y) {return name##l(x,y);}\
  inline long double Name(long double x, double y) {return name##l(x,y);} \
  inline long double Name(double x, long double y) {return name##l(x,y);} \
  inline long double Name(long double x, float y) {return name##l(x,y);} \
  inline long double Name(float x, long double y) {return name##l(x,y);} \
  template<class T> \
  inline long double Name(long double x, T y) {return name##l(x,y);} \
  template<class T> \
  inline long double Name(T x, long double y) {return name##l(x,y);} \
  template<class T1, class T2> \
  inline long double Name(T1 x, T2 y) { \
    return name##l((long double)x, (long double)y); \
  }

  FDLDMathFunc(Log, log);
  FDLDMathFunc(Log2, log2);
  FDLDMathFunc(Sqrt, sqrt);
  FDLDMathFunc(Sin, sin);
  FDLDMathFunc(Cos, cos);
  FDLDMathFunc(Tan, tan);
  FDLDMathFunc(Asin, asin);
  FDLDMathFunc(Acos, acos);
  FDLDMathFunc(Atan, atan);
  FDLDMathFunc(Sinh, sinh);
  FDLDMathFunc(Cosh, cosh);
  FDLDMathFunc(Tanh, tanh);
  FDLDMathFunc(Asinh, asinh);
  FDLDMathFunc(Acosh, acosh);
  FDLDMathFunc(Atanh, atanh);
  FDLDMathFunc(Exp, exp);
  FDLDMathFunc(Gamma, tgamma);
  FDLDMathFunc(Erf, erf);
  FDLDMathFunc(Floor, floor);
  FDLDMathFunc(Ceil, ceil);
  FDLDMathFunc2(Pow, pow);
  FDLDMathFunc2(FMod, fmod);
  FDLDMathFunc2(Atan2, atan2);


#ifdef CPP11
#define FLOATSTATEWRAP(Name, name, defval) \
  inline int Name(float x) { return name(x); } \
  inline int Name(double x) { return name(x); } \
  inline int Name(long double x) { return name(x); } \
  template<class T> inline int Name(T x) { return defval; }

  FLOATSTATEWRAP(IsNormal, std::isnormal, 1);
  FLOATSTATEWRAP(IsFinite, std::isfinite, 1);
  FLOATSTATEWRAP(IsNan, std::isnan, 0);
  FLOATSTATEWRAP(IsInf, std::isinf, 0);
  FLOATSTATEWRAP(FPClassify, std::fpclassify, FP_NORMAL);
#endif

  template<class T>
  inline T Epsilon(const T& x=0) {
    RC::UnusedVar(x);
#ifdef CPP11
    return std::numeric_limits<T>::epsilon();
#else
    return 0;
#endif
  }
#ifndef CPP11
  inline f32 Epsilon(const f32& x=0) { RC::UnusedVar(x); return FLT_EPSILON; }
  inline f64 Epsilon(const f64& x=0) { RC::UnusedVar(x); return DBL_EPSILON; }
#ifdef RC_HAVE_F80
  inline f80 Epsilon(const f80& x=0) { RC::UnusedVar(x); return LDBL_EPSILON; }
#endif
#endif


  // Floating point comparison, robust against calculation approaches,
  // combining float sizes, and normal/non-normal floats.
  template<class T1, class T2, class T3>
  inline bool Equal(T1 x, T2 y, T3 eps) {
    T1 absx = std::abs(x);
    T2 absy = std::abs(y);
    return std::abs(x-y) < ((absx < absy ? absy : absx) * eps)
                           + RC::MIN_POS(eps);
  }
  template<class T1, class T2>
  inline bool Equal(T1 x, T2 y) {
    T1 epsx = Epsilon(x);
    T2 epsy = Epsilon(y);
    if (epsx < epsy) {
      return Equal(x, y, 16*epsy);
    }
    else {
      return Equal(x, y, 16*epsx);
    }
  }


#ifdef CPP11
  template<class T> inline auto Deg2Rad(T x) -> decltype(RC::ForceFloat(x)) {
    return ((x)*(PI/180.0L));
  }
  template<class T> inline auto Rad2Deg(T x) -> decltype(RC::ForceFloat(x)) {
    return ((x)*(180.0L/PI));
  }
#else
  template<class T> inline f64 Deg2Rad(T x) { return ((x)*(PI/180.0L)); }
  template<class T> inline f64 Rad2Deg(T x) { return ((x)*(180.0L/PI)); }
#endif

#define RCMATH_DEGTRIG(TRIGFUNC) \
  template<class T> T TRIGFUNC##d(T x) {return TRIGFUNC(Deg2Rad(x));}
#define RCMATH_INVDEGTRIG(TRIGFUNC) \
  template<class T> T TRIGFUNC##d(T x) {return Rad2Deg(TRIGFUNC(x));}
  RCMATH_DEGTRIG(Sin);
  RCMATH_DEGTRIG(Cos);
  RCMATH_DEGTRIG(Tan);
  RCMATH_INVDEGTRIG(Asin);
  RCMATH_INVDEGTRIG(Acos);
  RCMATH_INVDEGTRIG(Atan);
#ifdef CPP11
  template<class T1, class T2> auto Atan2d(T1 y, T2 x)
    -> decltype(Atan2(y, x)) {
    return Rad2Deg(Atan2(y, x));
  }
#endif

  // Map x to offset within periodic boundaries of size y.  0>=x>y  (neg. safe)
#ifdef CPP11
  template<class T1, class T2>
  inline auto FWrap(T1 x, T2 y) -> decltype(x+y) {
    return ( (x<0) ? (y - FMod(-x,y)) : FMod(x,y) );
  }
#else
  template<class T>
  inline T FWrap(T x, T y) {
    return ( (x<0) ? (y - FMod(-x,y)) : FMod(x,y) );
  }
#endif

  // Map integer x to offset within periodic boundaries of size y.
  // 0>=x>y  (neg. safe)
  template<class T>
  inline T IWrap(T x, T y) {
    RC::AssertInteger<T>();
    return ( (x<0) ? (y + x%y) : (x%y) );
  }

  inline double Sqrt(u64 x) { return sqrt((double)x); }

  // x ^ y
  template<class T>
  inline T IPow(T x, T y) {
    RC::AssertInteger<T>();

    if (y < 0) {
      switch(x) {
        case -1:  return (y&1) ? -1 : 1;
        case  0:  return RC::MAX_VAL<T>();
        case  1:  return x;
        default:  return 0;
      }
    }

    T val = 1;
    while (y) {
      if (y & 1) { val *= x; }
      y >>= 1;
      x *= x;
    }

    return val;
  }

#ifdef CPP11
  template<class T1, class T2>
  inline auto IPow(T1 x, T2 y) -> decltype(x+y) {
    RC::AssertInteger<decltype(x+y)>();
    return IPow((decltype(x+y))x, (decltype(x+y))y);
  }
#endif


  // Log base 2, rounded down.  Takes any integer type.
  template<class T>
  inline T ILog2(T val) {
    RC::AssertInteger<T>();

    switch(val) {
      case 0:   return -1;
      case 1:   return 0;
      default:  break;
    }

    T check, slide;

    check = sizeof(val) << 2;
    slide = sizeof(val) << 1;

    while (slide) {
      switch(val >> check) {
        case 0:
          check -= slide;
          slide >>= 1;
          break;
        case 1:
          return(check);  // exact
        default:
          check += slide;
          slide >>= 1;
      }
    }

    return check;
  }


  /// Bit rotates x by amnt bits to the left, for any value of amnt.
  template<class T>
  inline T RotLeft(T x, int amnt) {
    RC::AssertInteger<T>();
    amnt &= sizeof(x)*8 - 1;
    return amnt ? (RC::ForceUnsigned(x) << amnt) |
                  (RC::ForceUnsigned(x) >> (sizeof(x)*8-amnt)) : x;
  }

  /// Bit rotates x by amnt bits to the right, for any value of amnt.
  template<class T>
  inline T RotRight(T x, int amnt) {
    RC::AssertInteger<T>();
    amnt &= sizeof(x)*8 - 1;
    return amnt ? (RC::ForceUnsigned(x) >> amnt) |
                  (RC::ForceUnsigned(x) << (sizeof(x)*8-amnt)) : x;
  }
}


#endif // RCMATH_MATHBITS_H

