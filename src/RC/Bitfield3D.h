/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Bitfield3D.h
/// Provides a three-dimensional structure of packed bits.
/////////////////////////////////////////////////////////////////////

#ifndef RC_BITFIELD3D_H
#define RC_BITFIELD3D_H


#include "Bitfield.h"
#include "Bitfield2D.h"
#include "Data1D.h"
#include "Errors.h"
#include <stdlib.h>


namespace RC {
  /// A bounds-safe three-dimensional structure of efficiently packed bits.
  /** Note AxBxC dimensions take ABC bits of storage with the total rounded
   *  up to the nearest 32 bit word.
   *  @see Bitfield
   *  @see Bitfield2D
   */
  class Bitfield3D {
    protected:
    /// @cond UNDOC
    void Initialize(size_t new_b_size1, size_t new_b_size2,
        size_t new_b_size3) {
      b_size1 = new_b_size1;
      b_size2 = new_b_size2;
      b_size3 = new_b_size3;

      data = Bitfield (b_size1 * b_size2 * b_size3);
    }
    /// @endcond

    public:
    /// Default constructor which initializes to size 0, 0, 0.
    Bitfield3D() {
      Initialize(0, 0, 0);
    }

    /// Cosntructor which sets the initial sizes.
    /** Efficiency note:  the first dimension, set by b_size1, should be the
     *  one which is iterated over most frequently in innermost loops for
     *  caching gains.
     *  @param b_size1 The size of the first dimension.
     *  @param b_size2 The size of the second dimension.
     *  @param b_size3 The size of the third dimension.
     */
    explicit Bitfield3D(size_t b_size1, size_t b_size2, size_t b_size3) {
      Initialize(b_size1, b_size2, b_size3);
    }


    /// Deletes the stored data, resets the sizes to 0, and releases the
    /// memory.
    inline void Delete() {
      b_size1 = 0;
      b_size2 = 0;
      b_size3 = 0;
      data.Delete();
    }

    
    /// True of the total size of the bits is 0.
    inline bool IsEmpty() const {
      return ( (b_size1 == 0) || (b_size2 == 0) || (b_size3 == 0) );
    }


    /// Bounds-checked access of the indexed element.
    /** Throws an ErrorMsgBounds exception if x, y, or z is out of the bounds.
     *  @param x The dimension 1 index of the bit to access.
     *  @param y The dimension 2 index of the bit to access.
     *  @param z The dimension 3 index of the bit to access.
     *  @return A Bitfield::BitfieldBool structure that casts to a bool or
     *  can be assigned a bool value.
     */
    inline Bitfield::BitfieldBool operator() (size_t x, size_t y, size_t z) {
      Assert(x, y, z);
      return data[z*b_size2*b_size1 + y*b_size1 + x];
    }

    /// Const version of operator()
    inline Bitfield::BitfieldBoolConst operator()
                                       (size_t x, size_t y, size_t z) const {
      Assert(x, y, z);
      return data[z*b_size2*b_size1 + y*b_size1 + x];
    }

    /// Identical to operator()
    inline Bitfield::BitfieldBool At(size_t x, size_t y, size_t z) {
      Assert(x, y, z);
      return data[z*b_size2*b_size1 + y*b_size1 + x];
    }

    /// Const version of At()
    inline Bitfield::BitfieldBoolConst At(size_t x, size_t y, size_t z) const {
      Assert(x, y, z);
      return data[z*b_size2*b_size1 + y*b_size1 + x];
    }


    /// Return the extent in bits of dimension 1.
    inline size_t size1() const { return b_size1; }
    /// Return the extent in bits of dimension 2.
    inline size_t size2() const { return b_size2; }
    /// Return the extent in bits of dimension 3.
    inline size_t size3() const { return b_size3; }


    /// Set all bits to 0 / false.
    inline void Zero() {
      data.Zero();
    }

    /// Set all bits of dimensions 1 and 2 to zero at z indexed dimension 3.
    inline void Zero2D(const size_t z) {
      size_t z_start, z_fact;

      Assert(0, 0, z);

      z_fact = b_size2*b_size1;
      z_start = z * z_fact;

      data.ZeroRange(z_start, z_start+z_fact-1);
    }

    /// Set all bits of dimension 1 to zero at y and z indexed dimensions 2
    /// and 3.
    inline void Zero1D(const size_t y, const size_t z) {
      size_t y_start;

      Assert(0, y, z);

      y_start = z * b_size2 * b_size1 + y * b_size1;

      data.ZeroRange(y_start, y_start + b_size1 - 1);
    }


    /// True of indices x, y, and z are all in bounds.
    inline bool Check(const size_t x, const size_t y, const size_t z) const {
      if ( ! ((x < b_size1) && (y < b_size2) && (z < b_size3)) ) {
        return false;
      }
      else {
        return true;
      }
    }


    /// Throws ErrorMsgBounds if index x, y, or z is out of bounds.
    inline void Assert(const size_t x, const size_t y, const size_t z) const {
      if ( ! Check(x, y, z) ) {
        Throw_RC_Type(Bounds, "Out of bounds");
      }
    }


    /// Swap the data in Bitfield3D a and b.
    friend void swap (Bitfield3D &a, Bitfield3D &b) {
      std::swap(a.b_size1, b.b_size1);
      std::swap(a.b_size2, b.b_size2);
      std::swap(a.b_size3, b.b_size3);
      swap(a.data, b.data);
    }


    protected:
    /// @cond PROTECTED

    size_t b_size1;
    size_t b_size2;
    size_t b_size3;
    Bitfield data;
    /// @endcond
  };


  /// Outputs data to a stream as a character series of '1's and '0's, with
  /// newlines trailing each entry of dimension 2, and double-spaces after
  /// entries of dimension 3.
  /** Usage like Bitfield3D bf;  std::cout << bf << std::endl;
   */
  inline std::ostream& operator<< (std::ostream &out, const Bitfield3D &b) {
    size_t i, j, k;

    for (k=0; k<b.size3(); k++) {
      for (j=0; j<b.size2(); j++) {
        for (i=0; i<b.size1(); i++) {
          if (b(i,j,k)) {
            out << '1';
          }
          else {
            out << '0';
          }
        }
        out << '\n';
      }
      out << '\n';
    }

    return out;
  }
}


#endif // RC_BITFIELD3D_H

