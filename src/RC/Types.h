/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2016, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Types.h
/// Provides typedefs and routines for working with primitives.
/////////////////////////////////////////////////////////////////////

#ifndef RC_TYPES_H
#define RC_TYPES_H

#include "RCconfig.h"
#include "Macros.h"

#include <stdint.h>
#include <stddef.h>  // Includes size_t and NULL
#include <limits>
#include <cfloat>
#ifdef CPP11
#include <type_traits>
#endif

typedef int8_t i8;      ///< 8-bit signed integer.
typedef uint8_t u8;     ///< 8-bit unsigned integer.
typedef int16_t i16;    ///< 16-bit signed integer.
typedef uint16_t u16;   ///< 16-bit unsigned integer.
typedef int32_t i32;    ///< 32-bit signed integer.
typedef uint32_t u32;   ///< 32-bit unsigned integer.
typedef int64_t i64;    ///< 64-bit signed integer.
typedef uint64_t u64;   ///< 64-bit unsigned integer.

typedef float f32;      ///< 32-bit float.
typedef double f64;     ///< 64-bit float.
#ifdef unix
#if __LDBL_MAX_EXP__ == 16384
/// Defined if the 80-bit float was found available on this system.
#define RC_HAVE_F80
typedef long double f80;  ///< 80-bit float.  (Note:  Usually takes 16 bytes.)
#endif
#endif
#if (__SIZEOF_FLOAT128__ == 16)
/// Defined if the 128-bit float was found available on this system.
#define RC_HAVE_F128
typedef __float128 f128;  ///< 128-bit float.
#endif

#ifdef RC_HAVE_F128
typedef f128 fBIGGEST;    ///< The biggest float type available.
#else
#ifdef RC_HAVE_F80
typedef f80 fBIGGEST;
#else
typedef f64 fBIGGEST;
#endif
#endif

#ifdef CPP11
/// \def RC_THREAD_LOCAL
/// Provides thread_local if available
#define RC_THREAD_LOCAL thread_local
/// \def RC_NOEXCEPT
/// Provides noexcept if available
#define RC_NOEXCEPT noexcept
#else
#define decltype(x) typeof(x)
#define RC_THREAD_LOCAL
#define RC_NOEXCEPT throw()
#endif

namespace RC {

  /// True if the type is signed.
  /** Use as IsSignedType<u32>() or u32 x; IsSignedType(x); */
  template<class T> inline bool IsSignedType(const T& RC_UNUSED_PARAM(x)=0) {
    return (0 > (T(-1)));
  }
  /// True if the type is an integer type.
  /** Use as IsIntegerType<u32>() or u32 x; IsIntegerType(x); */
  template<class T> inline bool IsIntegerType(const T& RC_UNUSED_PARAM(x)=1) {
    return (1 == (T(1.1)));
  }


  /// @cond UNDOC
  template<class T, class Error, bool> struct TypeCheck {
    static inline void Assert() { }
  };
  template<class T, class Error> struct TypeCheck<T, Error, false> {
    static inline void Assert() { T::__TypeCheckFailed__; }
  };
  /// @endcond


  /// \def RC_TYPE_TRUE_MACRO
  /// For adding a new float or integral type, where StructName is FloatType
  /// or IntegerType
#define RC_TYPE_TRUE_MACRO(StructName, Type) \
  template<> struct StructName<Type> { static const bool value = true; };\
  template<> struct StructName<const Type> { static const bool value = true; };\
  template<> struct StructName<volatile Type> { static const bool value = true; };\
  template<> struct StructName<const volatile Type> { static const bool value = true; };

  /// @cond UNDOC
  template<class T> struct FloatType { static const bool value = false; };
  RC_TYPE_TRUE_MACRO(FloatType, f32)
  RC_TYPE_TRUE_MACRO(FloatType, f64)
#ifdef RC_HAVE_F80
  RC_TYPE_TRUE_MACRO(FloatType, f80)
#endif
#ifdef RC_HAVE_F128
  RC_TYPE_TRUE_MACRO(FloatType, f128)
#endif

  template<class T> struct IntegerType { static const bool value = false; };
  RC_TYPE_TRUE_MACRO(IntegerType, char)
  RC_TYPE_TRUE_MACRO(IntegerType, u8)
  RC_TYPE_TRUE_MACRO(IntegerType, i8)
  RC_TYPE_TRUE_MACRO(IntegerType, u16)
  RC_TYPE_TRUE_MACRO(IntegerType, i16)
  RC_TYPE_TRUE_MACRO(IntegerType, u32)
  RC_TYPE_TRUE_MACRO(IntegerType, i32)
  RC_TYPE_TRUE_MACRO(IntegerType, u64)
  RC_TYPE_TRUE_MACRO(IntegerType, i64)

  class ThisTypeIsNotAFloat { };
  class ThisTypeIsNotAnInteger { };
  /// @endcond

  /// For generating compilation errors if the type is not a float.
  template<class T> inline void AssertFloat() {
    TypeCheck<T, ThisTypeIsNotAFloat, FloatType<T>::value>::Assert();
  }

  /// For generating compilation errors if the type is not an integer.
  template<class T> inline void AssertInteger() {
    TypeCheck<T, ThisTypeIsNotAnInteger, IntegerType<T>::value>::Assert();
  }

#ifdef CPP11
  /// @cond UNDOC
  template<class T, bool> struct ForceFloatHelper {
    typedef T type;
  };
  template<class T> struct ForceFloatHelper<T, false> {
    typedef f64 type;
  };
  /// @endcond

  /// If T is a float type, return the same type.  Otherwise return it as
  /// an f64.
  template<class T> inline auto ForceFloat(T val)
    -> typename ForceFloatHelper<T, FloatType<T>::value>::type {
    return val;
  }
#endif

#ifdef CPP11
  /// Returns the unsigned equivalent to the given type.
  template<class T> typename std::make_unsigned<T>::type ForceUnsigned(T val) {
    return val;
  }
  /// Returns the signed equivalent to the given type.
  template<class T> typename std::make_signed<T>::type ForceSigned(T val) {
    return val;
  }
#else
#define RC_MAKE_FORCESIGNS(TS) \
  u##TS ForceUnsigned(i##TS val) { return val; } \
  i##TS ForceSigned(u##TS val) { return val; }
  template <class T> T ForceUnsigned(T val) { return val; }
  template <class T> T ForceSigned(T val) { return val; }
  RC_ARGS_EACH(RC_MAKE_FORCESIGNS, 8, 16, 32, 64)
#endif


  /// Provide the minimum value held by this type.  For floats, the
  /// smallest positive value.
  /** Use as MIN_VAL<u32>() or as u32 x = MIN_VAL(x); */
  template<class T> inline T MIN_VAL(const T& RC_UNUSED_PARAM(x)
      = std::numeric_limits<T>::min()) { return std::numeric_limits<T>::min();}
  /// Provide the maximum value held by this type.
  /** Use as MAX_VAL<u32>() or as u32 x = MAX_VAL(x); */
  template<class T> inline T MAX_VAL(const T& RC_UNUSED_PARAM(x)
      = std::numeric_limits<T>::max()) { return std::numeric_limits<T>::max();}
#ifdef CPP11
  /// Provide the lowest value held by this type, for which no value is less.
  /** Use as LOW_VAL<f32>() or as f32 x = LOW_VAL(x); */
  template<class T> inline T LOW_VAL(const T & RC_UNUSED_PARAM(x)
      = std::numeric_limits<T>::lowest()) {
    return std::numeric_limits<T>::lowest();
  }
#else
  template<class T, bool> struct LOW_VAL_Helper {
    static inline T F() { return MIN_VAL<T>(); }
  };
  template<class T> struct LOW_VAL_Helper<T, true> {
    // Usually true.  CPP11 version always true.
    static inline T F() { return -MAX_VAL<T>(); }
  };

  template<class T> inline T LOW_VAL(const T &x
      = LOW_VAL_Helper<T, FloatType<T>::value>::F()) {
    return LOW_VAL_Helper<T, FloatType<T>::value>::F();
  }
#endif

  /// @cond UNDOC
  template<class T, bool> struct MIN_POS_Helper {
    static inline T F() { return 1; }
  };
  template<class T> struct MIN_POS_Helper<T, true> {
    static inline T F() { return MIN_VAL<T>(); }
  };
  /// @endcond
  /// Provide the minimum positive value held by this type.
  /** Use as MIN_POS<f32>() or as f32 x = MIN_POS(x); */
  template<class T> inline T MIN_POS(const T& RC_UNUSED_PARAM(x)
      = MIN_POS_Helper<T, FloatType<T>::value>::F()) {
    return MIN_POS_Helper<T, FloatType<T>::value>::F();
  }


#ifdef CPP11
  /// Clean C-style function pointers:  FuncPtr<void(int)> f = \&func;  f(4);
  template<class FT> using FuncPtr = FT*;

  /// Clean function pointer syntax for member function.
  /** MemFuncPtr<A, void(int)> f = &A::func;  A a;  (a.*f)(4);  */
  template<class C, class FT> using MemFuncPtr = FT(C::*);
#endif


  /// @cond UNDOC
  class RC_Float_Size_Checker {
    protected:

    // Asserts the sizes for the float types.  If it fails, correct or
    // comment out.  f80 varies between 12 and 16, and is not checked.
#ifdef CPP11
    static_assert(sizeof(f32)==4, "f32 is not 4 bytes");
    static_assert(sizeof(f64)==8, "f64 is not 8 bytes");
#ifdef RC_HAVE_F128
    static_assert(sizeof(f128)==16, "f128 is not 16 bytes");
#endif
#else
    char RC_f32_Is_Not_4_bytes[0-(sizeof(f32)!=4)];
    char RC_f64_Is_Not_8_bytes[0-2*(sizeof(f64)!=8)];
#ifdef RC_HAVE_F128
    char RC_f128_Is_Not_16_bytes[0-2*(sizeof(f128)!=16)];
#endif
#endif
  };
  /// @endcond


  /// Auto-detects the endianness of the compilation target, and provides
  /// automatic endian conversion features.
  class Endian {
    public:

    /// Reverse the byte order.
    static inline u16 Swap(u16 x) {
      return ( (x >> 8) | (x << 8) );
    }

    /// Reverse the byte order.
    static inline u32 Swap(u32 x) {
      return ( (x >> 24)
             | ((x >> 8) & 0x0000FF00)
             | ((x << 8) & 0x00FF0000)
             | (x << 24) );
    }

    /// Reverse the byte order.
    static inline u64 Swap(u64 x) {
      return ( (x >> 56)
             | ((x >> 40) & 0x000000000000FF00ull)
             | ((x >> 24) & 0x0000000000FF0000ull)
             | ((x >> 8)  & 0x00000000FF000000ull)
             | ((x << 8)  & 0x000000FF00000000ull)
             | ((x << 24) & 0x0000FF0000000000ull)
             | ((x << 40) & 0x00FF000000000000ull)
             | (x << 56) );
    }


    /// Do nothing.  (For consistency.)
    static inline u8 Swap(u8 x) { return x; }
    /// Do nothing.  (For consistency.)
    static inline i8 Swap(i8 x) { return x; }
    /// Reverse the byte order.
    static inline i16 Swap(i16 x) { return Swap(u16(x)); }
    /// Reverse the byte order.
    static inline i32 Swap(i32 x) { return Swap(u32(x)); }
    /// Reverse the byte order.
    static inline i64 Swap(i64 x) { return Swap(u64(x)); }


    /// Converts x to little-endian.
    template<class T>
    static inline T ToLittle(const T& x) {
      if (IsLittle()) {
        return x;
      }
      else {
        return Swap(x);
      }
    }

    /// Converts x from little-endian.
    template<class T>
    static inline T FromLittle(const T& x) {
      return ToLittle(x);
    }

    /// Converts x to big-endian.
    template<class T>
    static inline T ToBig(const T& x) {
      if (IsLittle()) {
        return Swap(x);
      }
      else {
        return x;
      }
    }

    /// Converts x from big-endian.
    template<class T>
    static inline T FromBig(const T& x) {
      return ToBig(x);
    }

    /// True if this system is little-endian.
    static inline bool IsLittle() {
      u16 test = 0x0001;
      return (*reinterpret_cast<char *>(&test) == 1);
    }

    /// True if this system is big-endian.
    static inline bool IsBig() {
      return ( ! IsLittle() );
    }
  };

}

#endif // RC_TYPES_H

