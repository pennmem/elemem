/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2015, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file RND.h
/// Provides random number generator classes.
/////////////////////////////////////////////////////////////////////

#ifndef RC_RND_H
#define RC_RND_H

#include "Macros.h"
#include "Types.h"
#include "RTime.h"
#include "Data1D.h"
#ifdef RC_HAVE_URAND
#include "RStr.h"
#include "File.h"
#endif

#include <stdio.h>

namespace RC {

  /// An abstract class which provides functions for obtaining randomness
  /// in convenient forms.
  /** It fulfills the UniformRandomNumberGenerator concept.
   *  @see RND
   *  @see EntropyRND
   *  @see URand
   */
  class BaseRND {
    /// @cond PROTECTED
    #define RC_BASERND_u32_to_f32_divisor 4294967425.0
    #define RC_BASERND_u64_to_f64_divisor 18446744073709552645.0L
    /// @endcond

    public:

    /// Default constructor.
    BaseRND()
      : u8_offset(32),
        u16_offset(32) {
    }

    /// Implement this for all subclasses.  All other functions get their
    /// randomness from this.
    /** @return A random unsigned 32 bit integer.
     */
    virtual u32 Get_u32() = 0;


    /// Equivalent to Get_u32().
    inline u32 operator()() { return Get_u32(); }

    /// For UniformRandomNumberGenerator compliance.
    typedef u32 result_type;

    /// Returns 0 for UniformRandomNumberGenerator compliance.
    constexpr static inline u32 min() { return 0; }

    /// Returns u32 max for UniformRandomNumberGenerator compliance.
    constexpr static inline u32 max() { return 0xFFFFFFFF; }


    /// Provides random i32 values.
    inline i32 Get_i32() {
      return i32(Get_u32());
    }


    /// Provides random u64 values.
    inline u64 Get_u64() {
      return ( (u64(Get_u32()) << 32) + Get_u32() );
    }


    /// Provides random i64 values.
    inline i64 Get_i64() {
      return i64(Get_u64());
    }


    /// Provides a random f32 in the range [0,1).
    inline f32 Get_f32() {
      return ( f32(Get_u32()) / RC_BASERND_u32_to_f32_divisor );
    }


    /// Provides a random f64 in the range [0,1).
    inline f64 Get_f64() {
      return ( f64(Get_u64()) / RC_BASERND_u64_to_f64_divisor );
    }


    /// Provides random u8 values.
    inline u8 Get_u8() {
      u8 retval;

      if (u8_offset >= 32) {
        u8_rndstore = Get_u32();
        u8_offset = 0;
      }

      retval = u8(u8_rndstore >> u8_offset);
      u8_offset += 8;

      return retval;
    }

    /// Provides random i8 values.
    inline i8 Get_i8() {
      return i8(Get_u8());
    }

    /// Provides random char values.
    inline char Get_char() {
      return char(Get_u8());
    }

    /// Provides random u16 values.
    inline u16 Get_u16() {
      u16 retval;

      if (u16_offset >= 32) {
        u16_rndstore = Get_u32();
        u16_offset = 0;
      }

      retval = u16(u16_rndstore >> u16_offset);
      u16_offset += 16;

      return retval;
    }

    /// Provides random i16 values.
    inline i16 Get_i16() {
      return i16(Get_u16());
    }


    RC_GetT(u8);
    RC_GetT(i8);
    RC_GetT(char);
    RC_GetT(u16);
    RC_GetT(i16);
    RC_GetT(u32);
    RC_GetT(i32);
    RC_GetT(u64);
    RC_GetT(i64);
    RC_GetT(f32);
    RC_GetT(f64);


    /// Returns true with probability, which should be in the range [0,1].
    /** @param probability The chance of returning true.
     *  @return Randomly true according to probability.
     */
    inline bool GetProb(f64 probability) {
      return (Get_f64() < probability);
    }

    /// Returns true with probability, which should be in the range [0,1].
    /** @param probability The chance of returning true.
     *  @return Randomly true according to probability.
     */
    inline bool GetProb(f32 probability) {
      return (Get_f32() < probability);
    }


    /// Returns [0,range)
    /** Note:  The precision comes from f64, so it is slightly less than
     *  full u64 precision.
     *  @param range The number of integers in the range.
     *  @return A value from 0 up to range-1.
     */
    inline u64 GetRange(u64 range) {
      return range*Get_f64();
    }
    /// Like GetRange() but for i64.
    inline i64 GetRange(i64 range) { return i64(GetRange(u64(range))); }

    /// Like GetRange() but for i32.
    /** Note:  The precision comes from f32, so it is slightly less than
     *  full u32 precision.
     */
    inline u32 GetRange(u32 range) {
      return range*Get_f32();
    }
    /// Like GetRange() but for i32.
    inline i32 GetRange(i32 range) { return i32(GetRange(u32(range))); }

    /// Provides a random integer value in the range [low,high].
    template <class T>
    inline T GetRange(T low, T high) { return GetRange(high-low+1)+low;}

    /// Provides a random float in the range [low,high).
    inline f64 GetFRange(f64 low, f64 high) {
      return Get_f64()*(high-low)+low;
    }


    /// For any supported data type, fills the Data1D with random values.
    template <class T>
    inline void Fill(Data1D<T>& data) {
      size_t i;

      for (i=0; i<data.size(); i++) {
        Get(data[i]);
      }
    }

    /// Returns a random element from the Data1D.
    template <class T>
    inline T GetFrom(Data1D<T>& data) {
      return data[GetRange(data.size())];
    }

    private:
    template<class T> inline static T EntAbs(T x) {
      return (((x)<0)?(-(x)):(x));
    }
    inline static void EntRot(u64 &x, u32 left_by) {
      x = (x << left_by) + (x >> (64-left_by));
    }
    public:

    /// Provides 64 bits of environmental entropy.
    /** This function uses the jittering of a tight loop relative to the
     *  ticking of the high precision clock to generate entropy, which is then
     *  mixed with the Rijndael S-box.  It automatically adapts to the system's
     *  clock precision and relative cpu speed.
     *  The randomness has passed dieharder version 3.31.1.
     *  @return 64 bits of environmental entropy.
     */
    inline static u64 GetEntropy() {
      static RC_THREAD_LOCAL u64 entropy = 0;
      u8 sbox_raw[256] = {  // Rijndael S-box
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B,
        0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
        0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, 0xB7, 0xFD, 0x93, 0x26,
        0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2,
        0xEB, 0x27, 0xB2, 0x75, 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
        0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED,
        0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F,
        0x50, 0x3C, 0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
        0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, 0xCD, 0x0C, 0x13, 0xEC,
        0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14,
        0xDE, 0x5E, 0x0B, 0xDB, 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
        0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79, 0xE7, 0xC8, 0x37, 0x6D,
        0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F,
        0x4B, 0xBD, 0x8B, 0x8A, 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
        0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, 0xE1, 0xF8, 0x98, 0x11,
        0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F,
        0xB0, 0x54, 0xBB, 0x16
      };
      Data1D<u8> sbox(256, sbox_raw);

      static RC_THREAD_LOCAL f64 half_prec = 0.5*Time::GetPrecision();
      static RC_THREAD_LOCAL f64 delta = half_prec;
      const u32 empirical_entropy_factor = 32;  // ~20
      u64 loopdiffsum = 0;
      u64 last_loops = 0;
      Data1D<f64> delta_arr;

      for (u32 i=0; i < 64*empirical_entropy_factor; i++) {
        u64 loops = 0;

        f64 start_t = RC::Time::Get();
        while (EntAbs(RC::Time::Get() - start_t) <= delta) { loops++; }
        delta /= 0.98 + 0.04*(loops>0); // Adapt time to 50/50 LSB flip
        delta_arr += delta;
        if (delta < half_prec) { delta = half_prec; }

        if (i > 0) {
          i64 loopdiff = (i64)loops - (i64)last_loops;
          if (loopdiff < 0) { loopdiff = -loopdiff; }
          loopdiffsum += loopdiff;
        }
        last_loops = loops;

        entropy ^= loops;
        EntRot(entropy, 8);

        if ((i&7) == 7) {
          for (u32 b=0; b<8; b++) {
            u32 shift = b*8;
            u64 keep_mask = ~(u64(0xFFL) << shift);
            u8 look_up = u8(entropy >> shift);
            entropy = (entropy & keep_mask) + (u64(sbox[look_up]) << shift);
          }
          EntRot(entropy, 3);
        }

        if ((i&7) == 0 && loopdiffsum > 64*empirical_entropy_factor) {
          break;  // High entropy estimate / slow jittery loop detected.
        }
      }

      return entropy;
    }


    private:

    u32 u8_rndstore;
    u32 u8_offset;
    u32 u16_rndstore;
    u32 u16_offset;
  };



  /// A Mersenne Twister random number generator class with integer, array,
  /// or time-based seeding.
  /** The output of this class is compliant with the mt19937ar reference
   *  implementation by Nishimura & Matsumoto.
   */
  class RND : public BaseRND {
    public:

    /// The reference implementation's recommended seed.
    static const u32 CONST_SEED=5489;

    /// Reseed the random number generator with the value seed.
    inline void Seed(u32 seed) {
      MT[0] = seed;
      
      for (size_t i=1; i<MT.size(); i++) {
        MT[i] = (1812433253 * (MT[i-1] ^ (MT[i-1] >> 30)) + i);
      }
    }


    /// Reseed the generator with up to the first 624 values of init_key.
    inline void Seed(const Data1D<u32>& init_key) {
      Seed(19650218);

      if (init_key.size() == 0) {
        return;
      }

      int i, k, end;
      size_t j;
      i=1;
      j=0;
      end = (624 > init_key.size()) ? 624 : init_key.size();

      for (k=0; k<end; k++) {
        MT[i] = (MT[i] ^ ((MT[i-1]) ^ (MT[i-1] >> 30)) * 1664525)
                + init_key[j] + j;
        MT[i] &= 0xFFFFFFFF;

        i++;
        if (i >= 624) {
          MT[0] = MT[623];
          i=1;
        }

        j++;
        if (j >= init_key.size()) {
          j=0;
        }
      }

      for (k=0; k<623; k++) {
        MT[i] = (MT[i] ^ ((MT[i-1] ^ (MT[i-1] >> 30)) * 1566083941)) - i;
        MT[i] &= 0xFFFFFFFF;

        i++;
        if (i >= 624) {
          MT[0] = MT[623];
          i=1;
        }
      }

      MT[0] = 0x80000000;
    }


    /// Construct the generator with the initial seed given.
    inline RND(u32 seed) 
      : MT (Data1D<u32>(624)),
        u32_index(624) {

      Seed(seed);
    }


    /// Construct the generator with the initial seed being the first 624
    /// values of init_key.
    inline RND(const Data1D<u32>& init_key)
      : MT (Data1D<u32>(624)),
        u32_index(624) {

      Seed(init_key);
    }


    /// Default constructor, which seeds using the system time and 64
    /// bits of environmental entropy.
    inline RND() 
      : MT (Data1D<u32>(624)), 
        u32_index(624) {

      f64 time;
      time = Time::Get();
      u64 entropy = GetEntropy();

      Data1D<u32> time_seed(4);
      time_seed[0] = u32(time*1e9);
      time_seed[1] = u32(time);
      time_seed[2] = u32(entropy);
      time_seed[3] = u32(entropy>>32);

      Seed(time_seed);
    }


#ifdef RC_HAVE_URAND
    /// Seed the genederator by reading 624 u32's from /dev/urandom.
    inline void Seed_urandom() {
      FileRead urand(RC_DEV_URANDOM);
      Data1D<u32> init_key(624);
      urand.Read(init_key);
      Seed(init_key);
    }
#endif


    private:

    inline void UpdateMT() {
      int i;
      u32 val;
      u32 xorarr[2] = {0, 0x9908b0df};

      // Partially unrolled to remove modulos.  Significant performance gain.
      for (i=0; i<227; i++) {
        val = (MT[i] & 0x80000000) + (MT[i+1] & 0x7FFFFFFF);
        MT[i] = MT[i+397] ^ (val>>1);
        MT[i] ^= xorarr[val & 1];
      }
      for (; i<623; i++) {
        val = (MT[i] & 0x80000000) + (MT[i+1] & 0x7FFFFFFF);
        MT[i] = MT[i-227] ^ (val>>1);
        MT[i] ^= xorarr[val & 1];
      }
      val = (MT[i] & 0x80000000) + (MT[0] & 0x7FFFFFFF);
      MT[i] = MT[396] ^ (val>>1);
      MT[i] ^= xorarr[val & 1];
    }

    public:

    /// Provides random u32 values.
    inline virtual u32 Get_u32() {
      u32 val;

      if (u32_index >= MT.size()) {
        UpdateMT();
        u32_index = 0;
      }

      val = MT[u32_index];

      val ^= (val >> 11);
      val ^= ((val << 7) & 0x9d2c5680);
      val ^= ((val << 15) & 0xefc60000);
      val ^= (val >> 18);

      u32_index++;

      return val;
    }


    private:

    Data1D<u32> MT;
    u16 u32_index;
  };


  /// Provides true random numbers sourced from environmental noise.
  /** See the entropy source BaseRND::GetEntropy() for details on the method.
   *  These numbers should be cryptographically strong.
   */
  class EntropyRND : public BaseRND {
    public:

    /// Default constructor.
    EntropyRND() : u32_rndstore(0), u32_offset(64) { }

    /// Provides random u32 values.
    inline virtual u32 Get_u32() {
      u32 retval;

      if (u32_offset >= 64) {
        u32_rndstore = GetEntropy();
        u32_offset = 0;
      }

      retval = u32(u32_rndstore >> u32_offset);
      u32_offset += 32;

      return retval;
    }

    private:

    u64 u32_rndstore;
    u32 u32_offset;
  };

  /// @cond UNDOC
#ifdef __MINGW32__
#define RC_MINGW_2014_BUGFIX
#else
#define RC_MINGW_2014_BUGFIX RC_THREAD_LOCAL
#endif
  /// @endcond


  /// \def RC_MAKE_GET_RANGE
  /// Generates a family of RND_Get_Range functions.
#define RC_MAKE_GET_RANGE(classname) \
  /** \brief Uses classname as a RandomNumberGenerator, e.g. for random_shuffle 
      @param range The range of values that can be returned, starting with 0.
      @return A random value from 0 up to and including range-1.
   */\
  inline u64 classname##_Get_Range(u64 range) {\
    static RC_MINGW_2014_BUGFIX classname rng;\
    return rng.GetRange(range);\
  }

  RC_MAKE_GET_RANGE(RND);
  RC_MAKE_GET_RANGE(EntropyRND);

#ifdef RC_HAVE_URAND
  /// Cryptographically strong RNG, uses /dev/urandom
  /** The interface is identical to RND.h except there are no seed functions.
   */
  class URand : public BaseRND {
    public:

    /// Default constructor.
    /** @param buf_size The number of u32 values to read at a time.
     */
    inline URand(const size_t buf_size=128)
      : urand_buf (Data1D<u32>(buf_size)),
        urand_fr (RC_DEV_URANDOM),
        index (urand_buf.size()) {
    }


    /// Provides random u32 values.
    inline virtual u32 Get_u32() {
      if (index >= urand_buf.size()) {
        urand_fr.Read(urand_buf);
        index = 0;
      }

      return urand_buf[index++];
    }

    private:

    Data1D<u32> urand_buf;
    FileRead urand_fr;
    size_t index;
  };


  RC_MAKE_GET_RANGE(URand);

#endif // RC_HAVE_URAND


  /// \def RC_MAKE_RND_GEN
  /// Generates a family of random generator singletons.
  #define RC_MAKE_RND_GEN(classname) \
  /** \brief This returns a singleton of classname. */\
  inline classname& classname##_Gen() {\
    static RC_MINGW_2014_BUGFIX classname rng;\
    return rng;\
  }

  RC_MAKE_RND_GEN(RND);
  RC_MAKE_RND_GEN(EntropyRND);
#ifdef RC_HAVE_URAND
  RC_MAKE_RND_GEN(URand);
#endif


  /// @cond UNDOC
  class RC_BaseRND_Float_Bounds_Checker {
    protected:
    /// If this array gives a compile error, then Get_f32() can possibly
    /// return a 1.0!
    /** Either raise the value of the divisor to fix this, or comment out
     *  this check if you do not care.
     */
    char RC_Get_f32_Never_Return_One_Check[int((1-0xFFFFFFFF/RC_BASERND_u32_to_f32_divisor)*1e10)];

    /// If this array gives a compile error, then Get_f64() can possibly
    /// return a 1.0!
    /** Either raise the value of the divisor to fix this, or comment out
     *  this check if you do not care.
     */
    char RC_Get_f64_Never_Return_One_Check[int((1-0xFFFFFFFFFFFFFFFFull/RC_BASERND_u64_to_f64_divisor)*1e20)];
  }; 
  /// @endcond
}

#endif // RC_RND_H

