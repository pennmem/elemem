/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Bitfield.h
/// Provides a one-dimensional vector-like structure of packed bits.
/////////////////////////////////////////////////////////////////////

#ifndef RC_BITFIELD_H
#define RC_BITFIELD_H


#include "Data1D.h"
#include "Errors.h"
#include <iostream>


namespace RC {
  /// A bounds-safe one-dimensional vector-like structure of efficiently
  /// packed bits.
  /** It provides efficient resizing and append operations, and bounds-safe
   *  array access.
   *  @see Bitfield2D
   *  @see Bitfield3D
   */
  class Bitfield {
    friend class Bitfield2D;
    friend class Bitfield3D;

    protected:
    /// @cond UNDOC
    inline void Initialize(size_t new_b_size) {
      b_size = new_b_size;

      size_t u32size;

      if (b_size > 0) {
        u32size = b_size >> 5;
        if (b_size & 31) {
          u32size++;
        }
        data = Data1D<u32>(u32size);
      }
    }
    /// @endcond

    public:
    /// Default constructor which initializes to size 0.
    inline Bitfield() {
      Initialize(0);
    }

    /// Constructor which sets the initial size to b_size bits.
    /** @param b_size The initial number of bits.
     */
    explicit inline Bitfield(size_t b_size) {
      Initialize(b_size);
    }


    /// Deletes the stored data, resets the size to 0, and releases the
    /// memory.
    inline void Delete() {
      data.Delete();
      b_size = 0;
    }


    /// True if the size is 0.
    inline bool IsEmpty() const {
      return (b_size == 0);
    }


    protected:
    /// A temporary return-type that serves as an interface to specific
    /// bit values.
    class BitfieldBool {
      friend class Bitfield;

      public:

      /// @cond PROTECTED
      inline BitfieldBool(Bitfield *bf, size_t index)
        : bf (bf) {
        bf->Assert(index);
        index_u32 = index >> 5;
        index_mask = 1 << (index & 31);
      }
      /// @endcond

      /// Implicit conversion to bool.
      inline operator bool () const {
        return (bf->data[index_u32] & index_mask);
      }

      /// Set the bit to true / 1.
      inline void Set() {
        bf->data[index_u32] |= index_mask;
      }

      /// Set the bit to false / 0.
      inline void Clear() {
        bf->data[index_u32] &= ~index_mask;
      }

      /// Assign the bool value of set to this bit.
      inline BitfieldBool& operator= (const bool set) {
        if (set) {
          Set();
        }
        else {
          Clear();
        }

        return *this;
      }


      protected:
      /// @cond PROTECTED

      Bitfield *bf;
      size_t index_u32;
      u32 index_mask;
      /// @endcond
    };

    /// A temporary return-type that serves as a const interface to specific
    /// bit values.
    class BitfieldBoolConst {
      friend class Bitfield;

      public:

      /// @cond PROTECTED
      inline BitfieldBoolConst(const Bitfield *bf, size_t index)
        : bf (bf) {
        bf->Assert(index);
        index_u32 = index >> 5;
        index_mask = 1 << (index & 31);
      }
      /// @endcond

      /// Implicit conversion to bool.
      inline operator bool () const {
        return (bf->data[index_u32] & index_mask);
      }

      protected:
      /// @cond PROTECTED

      const Bitfield *bf;
      size_t index_u32;
      u32 index_mask;
      /// @endcond
    };
    public:


    /// Bounds-checked access of the element at index x.
    /** Throws an ErrorMsgBounds exception if x is out of the bounds.
     *  @param x Index of the bit to access.
     *  @return A BitfieldBool structure that casts to a bool or can be
     *  assigned a bool value.
     */
    inline BitfieldBool operator[] (size_t x) {
      return BitfieldBool(this, x);
    }
    /// Identical to operator[]
    inline BitfieldBool operator() (size_t x) {
      return BitfieldBool(this, x);
    }
    /// Const version of operator[]
    inline BitfieldBoolConst operator[] (size_t x) const {
      return BitfieldBoolConst(this, x);
    }
    /// Const version of operator[]
    inline BitfieldBoolConst operator() (size_t x) const {
      return BitfieldBoolConst(this, x);
    }
    /// Const version of At()
    inline BitfieldBool At(size_t x) {
      return BitfieldBool(this, x);
    }
    /// Identical to operator[]
    inline BitfieldBoolConst At(size_t x) const {
      return BitfieldBoolConst(this, x);
    }


    /// Return the current number of bits.
    inline size_t size() const { return b_size; }
    /// Return the number of bits for which space is reserved.
    inline size_t reserved() const { return (data.reserved() << 5); }

    /// Set all bits to zero.
    inline void Zero() {
      data.Zero();
    }


    /// Set all bits to zero between index start and end, inclusive.
    inline void ZeroRange(const size_t start, const size_t end) {
      size_t start_u32;
      size_t start_u32_border;
      size_t end_u32;
      size_t end_u32_border;
      size_t b;

      start_u32 = (start + 31) >> 5;
      start_u32_border = start_u32 << 5;

      if (start_u32_border >= end) {
        for (b=start; b<end; b++) {
          (*this)[b].Clear();
        }
      }

      else {
        end_u32 = (end + 1) >> 5;
        end_u32_border = end_u32 << 5;

        if (end_u32 > start_u32) {
          data.ZeroRange(start_u32, end_u32-1);
        }

        for (b=start; b<start_u32_border; b++) {
          (*this)[b].Clear();
        }

        for (b=end_u32_border; b<=end; b++) {
          (*this)[b].Clear();
        }
      }
    }


    /// Provides raw access to the Data1D<u32> in which the bits are packed.
    /** The bits are packed in order, least significant bit first.
     */
    inline Data1D<u32>& Raw() { return data; }


    /// Reserve storage without resizing the Bitfield.
    /** @param reserve_size The number of bits for which storage should be
     *  reserved.
     */
    inline void Reserve(const size_t reserve_size) {
      size_t u32size;

      u32size = reserve_size >> 5;
      if (reserve_size & 31) {
        u32size++;
      }

      data.Resize(u32size);
    }


    /// Resize the array, reallocating if necessary.
    /** See Data1D::Resize for efficiency details.
     */
    inline void Resize(const size_t resize_size) {
      Reserve(resize_size);
      b_size = resize_size;
    }


    /// Reduce memory consumption to only that necessary for size() bits.
    inline void Crop() {
      data.Crop();
    }


    /// Identical to Delete()
    inline void Clear() {
      Delete();
    }


    /// Add a bit to the end, expanding if necessary.
    /** Efficiency note:  Expands by doubling for linear efficiency.
     *  @param new_bit The new bit to append to the end.
     *  @see Reserve
     */
    inline void Append(const bool new_bit) {
      size_t last;

      if (b_size >= reserved()) {
        data.Append(0);
      }
      
      last = b_size;
      b_size++;
      (*this)[last] = new_bit;
    }


    /// Append bits from another Bitfield to this Bitfield.
    /** @param other The Bitfield from which bits should be copied over.
     */
    inline void Append(const Bitfield& other) {
      size_t i;
      for (i=0; i<other.b_size; i++) {
        Append(other[i]);
      }
    }


    /// Assign new_bit to index pos, expanding if necessary to reach that
    /// index.
    inline void ExpandSet(size_t pos, const bool new_bit) {
      if (!Check(pos)) {
        Resize(pos+1);
      }
      (*this)[pos] = new_bit;
    }


    /// Appends new_bit, expanding as necessary.
    /** @see Append
     */
    inline Bitfield& operator+= (const bool new_bit) {
      Append(new_bit);
      return (*this);
    }

    
    /// Appends the bits from other, expanding as necessary.
    /** @see Append
     */
    inline Bitfield& operator+= (const Bitfield& other) {
      Append(other);
      return (*this);
    }


    /// True if index x is within the Bitfield.
    inline bool Check(const size_t x) const {
      if ( ! (x < b_size) ) {
        return false;
      }
      else {
        return true;
      }
    }


    /// Throws ErrorMsgBounds if index x is out of bounds for this Bitfield.
    inline void Assert(const size_t x) const {
      if ( ! Check(x) ) {
        Throw_RC_Type(Bounds, "Out of bounds");
      }
    }


    /// Efficiently determines the total number of true / 1 values in the
    /// Bitfield.
    inline size_t CountOnes() const {
      size_t count;
      size_t i;
      u32 word;

      count = 0;
      for (i=0; i<data.size(); i++) {
        word = data[i];
        if ((i == data.size()-1) && (b_size & 31)) {
          word &= (u64(1) << (b_size & 31)) - 1;  // trim overflow
        }
        // Algorithm from Software Optimization Guide for AMD64, sec. 8.6
        word = word - ((word >>1) & 0x55555555);
        word = (word & 0x33333333) + ((word >> 2) & 0x33333333);
        count += (((word + (word >> 4)) & 0xF0F0F0F) * 0x01010101) >> 24;
      }

      return count;
    }

    /// Efficiently determines the total number of false / 0 values in the
    /// Bitfield.
    inline size_t CountZeroes() const {
      return b_size - CountOnes();
    }

    /// Swap the data in Bitfield a and b.
    friend void swap(Bitfield &a, Bitfield &b) {
      std::swap(a.b_size, b.b_size);
      swap(a.data, b.data);
    }


    protected:
    /// @cond PROTECTED

    size_t b_size;
    Data1D<u32> data;
    /// @endcond
  };

  /// Outputs data to a stream as a character series of '1's and '0's.
  /** Usage like Bitfield bf;  std::cout << bf << std::endl;
   */
  inline std::ostream& operator<< (std::ostream &out, const Bitfield &b) {
    size_t i;
    for (i=0; i<b.size(); i++) {
      if (b[i]) {
        out << '1';
      }
      else {
        out << '0';
      }
    }
    return out;
  }
}


#endif // RC_BITFIELD_H

