/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Bitfield2D.h
/// Provides a two-dimensional structure of packed bits.
/////////////////////////////////////////////////////////////////////

#ifndef RC_BITFIELD2D_H
#define RC_BITFIELD2D_H


#include "Bitfield.h"
#include "Data1D.h"
#include "Errors.h"
#include <stdlib.h>


namespace RC {
  /// A bounds-safe two-dimensional structure of efficiently packed bits.
  /** Note AxB and BxA dimensions will both take AB bits of storage with the
   *  total rounded up to the nearest 32 bit word.
   *  @see Bitfield
   *  @see Bitfield3D
   */
  class Bitfield2D {
    protected:
    /// @cond UNDOC
    void Initialize(size_t new_b_size1, size_t new_b_size2) {
      b_size1 = new_b_size1;
      b_size2 = new_b_size2;

      data = Bitfield (b_size1 * b_size2);
    }
    /// @endcond

    public:
    /// Default constructor which initializes to size 0, 0.
    Bitfield2D() {
      Initialize(0, 0);
    }

    /// Constructor which sets the initial sizes.
    /** Efficiency note:  The first dimension, set by b_size1, should be the
     *  one which is iterated over most frequently in innermost loops for
     *  caching gains.
     *  @param b_size1 The size of the first dimension.
     *  @param b_size2 The size of the second dimension.
     */
    explicit Bitfield2D(size_t b_size1, size_t b_size2) {
      Initialize(b_size1, b_size2);
    }


    /// Deletes the stored data, resets the sizes to 0, and releases the
    /// memory.
    inline void Delete() {
      b_size1 = 0;
      b_size2 = 0;
      data.Delete();
    }

    
    /// True if the total size of the bits is 0.
    inline bool IsEmpty() const {
      return ( (b_size1 == 0) || (b_size2 == 0) );
    }


    /// Bounds-checked access of the indexed element.
    /** Throws an ErrorMsgBounds exception if x or y is out of the bounds.
     *  @param x The dimension 1 index of the bit to access.
     *  @param y The dimension 2 index of the bit to access.
     *  @return A Bitfield::BitfieldBool structure that casts to a bool or
     *  can be assigned a bool value.
     */
    inline Bitfield::BitfieldBool operator() (size_t x, size_t y) {
      Assert(x, y);
      return data[y*b_size1 + x];
    }

    /// Const version of operator()
    inline Bitfield::BitfieldBoolConst operator() (size_t x, size_t y) const {
      Assert(x, y);
      return data[y*b_size1 + x];
    }

    /// Identical to operator()
    inline Bitfield::BitfieldBool At(size_t x, size_t y) {
      Assert(x, y);
      return data[y*b_size1 + x];
    }

    /// Const version of At()
    inline Bitfield::BitfieldBoolConst At(size_t x, size_t y) const {
      Assert(x, y);
      return data[y*b_size1 + x];
    }


    /// Return the extent in bits of dimension 1.
    inline size_t size1() const { return b_size1; }
    /// Return the extent in bits of dimension 2.
    inline size_t size2() const { return b_size2; }

    /// Set all bits to 0 / false.
    inline void Zero() {
      data.Zero();
    }

    /// Set all bits of dimension 1 to zero at y indexed dimension 2.
    inline void Zero1D(const size_t y) {
      size_t y_start;

      Assert(0, y);

      y_start = y*b_size1;

      data.ZeroRange(y_start, y_start + b_size1 - 1);
    }


    /// True if indices x and y are both in bounds.
    inline bool Check(const size_t x, const size_t y) const {
      if ( ! ((x < b_size1) && (y < b_size2)) ) {
        return false;
      }
      else {
        return true;
      }
    }


    /// Throws ErrorMsgBounds if index x or y is out of bounds.
    inline void Assert(const size_t x, const size_t y) const {
      if ( ! Check(x, y) ) {
        Throw_RC_Type(Bounds, "Out of bounds");
      }
    }


    /// Swap the data in Bitfield2D a and b.
    friend void swap (Bitfield2D &a, Bitfield2D &b) {
      std::swap(a.b_size1, b.b_size1);
      std::swap(a.b_size2, b.b_size2);
      swap(a.data, b.data);
    }


    protected:
    /// @cond PROTECTED

    size_t b_size1;
    size_t b_size2;
    Bitfield data;
    /// @endcond
  };


  /// Outputs data to a stream as a character series of '1's and '0's, with
  /// newlines trailing each entry of dimension 2.
  /** Usage like Bitfield2D bf;  std::cout << bf << std::endl;
   */
  inline std::ostream& operator<< (std::ostream &out, const Bitfield2D &b) {
    size_t i, j;

    for (j=0; j<b.size2(); j++) {
      for (i=0; i<b.size1(); i++) {
        if (b(i,j)) {
          out << '1';
        }
        else {
          out << '0';
        }
      }
      out << '\n';
    }

    return out;
  }
}



#endif // RC_BITFIELD2D_H

