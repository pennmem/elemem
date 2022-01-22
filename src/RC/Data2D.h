/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2016, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Data2D.h
/// Provides a bounds-safe two-dimensional resizeable structure.
/////////////////////////////////////////////////////////////////////

#ifndef RC_DATA2D_H
#define RC_DATA2D_H


#include "Data1D.h"
#include "Errors.h"
#include <stdlib.h>


namespace RC {
  template <class T>
      class Data2D;
  template <class T>
      inline std::ostream& operator<< (std::ostream &out, const Data2D<T>& d);

  /// A bounds-safe two-dimensional resizeable structure.
  /** Note:  Non-POD classes stored in Data2D containers must have a
   *  default constructor with default values or no arguments.
   *  @see Data1D
   *  @see Data3D
   */
  template <class T>
  class Data2D {
    protected:
    /// @cond UNDOC
    void Initialize(size_t new_d_size1, size_t new_d_size2) {
      d_size1 = new_d_size1;
      d_size2 = new_d_size2;

      data = Data1D< Data1D<T> >(d_size2);
      for (size_t i=0; i<d_size2; i++) {
        data[i] = Data1D<T>(d_size1);
      }
    }
    /// @endcond

    public:

    /// Default constructor which initializes to size 0.
    Data2D() {
      Initialize(0, 0);
    }

    /// Constructor which sets the initial sizes.
    /** Efficiency note:  The first dimension, set by d_size1, should be the
     *  one which is iterated over most frequently in innermost loops for
     *  caching gains.
     *  @param d_size1 The size of the first dimension.
     *  @param d_size2 The size of the second dimension.
     */
    explicit Data2D(size_t d_size1, size_t d_size2) {
      Initialize(d_size1, d_size2);
    }


    /// Copy constructor that copies all elements.
    /** @param copy A source Data2D from which elements should be copied.
     */
    inline Data2D(const Data2D<T>& copy)
      : d_size1 (copy.d_size1),
        d_size2 (copy.d_size2) {

      data = copy.data;
    }

#ifdef CPP11
    /// Initializer list constructor, initializes with nested brackets.
    inline Data2D(const std::initializer_list<Data1D<T>>& new_data)
      : d_size1(0),
        d_size2(new_data.size()),
        data(new_data.size()) {
        std::copy(new_data.begin(), new_data.end(), data.Raw());
        if (data.size()) {
          d_size1 = data[0].size();
        }
      }
#endif

    /// Deletes all contents upon destruction.
    ~Data2D() {
      Delete();
    }


    /// Delete all the elements and free all allocated memory.
    inline void Delete() {
      d_size1 = 0;
      d_size2 = 0;
      data.Delete();
      cptrs.Delete();
    }


    /// Identical to Delete().
    inline void Clear() {
      Delete();
    }


    /// Reduces memory consumption to only that necessary for the current
    /// size.
    /** Efficiency note:  This function may copy all elements if necessary
     *  for reallocating.
     */
    inline void Crop() {
      data.Crop();

      for (size_t i=0; i<d_size2; i++) {
        data[i].Crop();
      }

      cptrs.Crop();
    }


    /** @return True if there are no elements / at least one size is 0.
     */
    inline bool IsEmpty() const {
      return ( (d_size1 == 0) || (d_size2 == 0) );
    }


    /// Assignment operator which copies all contents from other.
    /** @param other Data2D to copy from.
     *  @return This object.
     */
    inline Data2D& operator= (const Data2D& other) {
      if (this != &other) {
        d_size1 = other.d_size1;
        d_size2 = other.d_size2;
        data = other.data;
      }

      return *this;
    }


    /// Copy data from any other object of a type with a compatible

    /// Return a bounds-checked random-access iterator starting at offset.
    /** Note that the iterator is revokable, meaning use of it will throw an
     *  exception if this Data2D is deleted.
     *  @return A bounds-checked iterator.
     *  @see end()
     */
    inline RAIter< Data1D< Data1D<T> >, Data1D<T> > begin() {
      return data.begin();
    }
    /// Const version of begin.
    inline const RAIter< Data1D< Data1D<T> >, Data1D<T> > begin() const {
      return data.begin();
    }


    /// Return a bounds-checked random-access iterator starting just past
    /// the last element.
    /** Note that the iterator is revokable, meaning use of it will throw an
     *  exception if this Data2D is deleted.
     *  @return A bounds-checked iterator.
     *  @see begin()
     */
    inline RAIter< Data1D< Data1D<T> >, Data1D<T> > end() {
      return data.end();
    }
    /// Const version end.
    inline RAIter< Data1D< Data1D<T> >, Data1D<T> > end() const {
      return data.end();
    }


    /// Bounds-checked access of a Data1D corresponding to the data at
    /// index x in dimension 2.
    /** Throws an ErrorMsgBounds if out of bounds.  Usage note:
     *  Data2D<int> arr(size1, size2);  int val = arr[x2][x1];
     *  @param x The index of dimension 2.
     *  @return A Data1D of all elements in dimension 1 at that index.
     */
    inline Data1D<T>& operator[] (size_t x) { return data[x]; }
    /// Identical to Data2D::operator[]
    inline Data1D<T>& operator() (size_t x) { return data[x]; }
    /// Const version of Data2D::operator[]
    inline const Data1D<T>& operator[] (size_t x) const { return data[x]; }
    /// Const version of Data2D::operator[]
    inline const Data1D<T>& operator() (size_t x) const { return data[x]; }
    /// Bounds-checked access of an element.
    /** Throws an ErrorMsgBounds if out of bounds.  Note that for
     *  Data2D<int> arr;  arr(x, y) is equivalent to arr[y][x].
     *  @param x The index of dimension 1
     *  @param y The index of dimension 2.
     *  @return A reference to the indexed element.
     */
    inline T& operator() (size_t x, size_t y) { return data[y][x]; }
    /// Const version of Data2D::operator()(size_t x, size_t y)
    inline const T& operator() (size_t x, size_t y) const {
      return data[y][x];
    }
    /// Equivalent to Data2D::operator()(size_t x, size_t y)
    inline T& At(size_t x, size_t y) { return data[y][x]; }
    /// Const version of Data2D::operator()(size_t x, size_t y)
    inline const T& At(size_t x, size_t y) const {
      return data[y][x];
    }

    /// Get the size of dimension 1.
    inline size_t size1() const { return d_size1; }
    /// Get the size of dimension 2.
    inline size_t size2() const { return d_size2; }

    /// Returns sizeof(T).
    inline size_t TypeSize() const { return sizeof(T); }


    /// Sets all elements equal to 0.
    /** @see Data1D::Zero() */
    inline void Zero() {
      for (size_t y=0; y<d_size2; y++) {
        data[y].Zero();
      }
    }


    /// Check if the indices x and y are in bounds.
    /** @param x The index for dimension 1.
     *  @param y The index for dimension 2.
     *  @return True if in bounds.
     */
    inline bool Check(const size_t x, const size_t y) const {
      if ( ! ((x < d_size1) && (y < d_size2)) ) {
        return false;
      }
      else {
        return true;
      }
    }


    /// Throw an ErrorMsgBounds exception if either x or y is out
    /// of bounds
    /** @param x The index for dimension 1.
     *  @param y The index for dimension 2.
     */
    inline void Assert(const size_t x, const size_t y) const {
      if ( ! Check(x, y) ) {
        Throw_RC_Type(Bounds, "Out of bounds");
      }
    }


    /// Resize the array, reallocating if necessary.
    /** This may trigger a copy operation upon expansion.  For efficiency, it
     *  never reallocates or copies while shrinking or expanding within a
     *  previous size range.  Use Crop if necessary to shrink storage to the
     *  current size.
     *  @param resize_size1 The new size for dimension 1.
     *  @param resize_size2 The new size for dimension 2.
     */
    inline void Resize(const size_t resize_size1, const size_t resize_size2) {
      data.Resize(resize_size2);
      for (size_t i=0; i<resize_size2; i++) {
        data[i].Resize(resize_size1);
      }
      d_size2 = resize_size2;
      d_size1 = resize_size1;

      cptrs.Resize(0);  // Invalidates
    }


    /// Access a raw unprotected 2-dimensional C-array for the enclosed data.
    /** Warning:  This convenience function bypasses the bounds protections
     *  provided by this class.  Also the C-array becomes invalid if this
     *  object is resized, deleted, or cropped.
     *  @return C-style pointer to the contents.
     */
    inline T** Raw() {
      if (cptrs.size() != d_size2) {
        cptrs.Resize(d_size2);
        for (size_t i=0; i<d_size2; i++) {
          cptrs[i] = data[i].Raw();
        }
      }
      return cptrs.Raw();
    }


    /// Access the underlying nested Data1D structure for this object.
    /** Attempts should not be made to resize the underlying data accessed
     *  with this convenience function.
     *  @return The nested Data1D contained within.
     */
    inline Data1D< Data1D<T> >& RawData() {
      return data;
    }
    /// Const version of RawData().
    inline const Data1D< Data1D<T> >& RawData() const {
      return data;
    }



    /// Convert endianness of all elements if needed, for supported types.
    inline void ToLilEndian() {
      if (Endian::IsBig()) {
        for (size_t y=0; y<d_size2; y++) {
          for (size_t x=0; x<d_size1; x++) {
            data[y][x] = Endian::ToLittle(data[y][x]);
          }
        }
      }
    }
    /// Convert endianness of all elements if needed, for supported types.
    inline void FromLilEndian() { ToLilEndian(); }

    /// Convert endianness of all elements if needed, for supported types.
    inline void ToBigEndian() {
      if (Endian::IsLittle()) {
        for (size_t y=0; y<d_size2; y++) {
          for (size_t x=0; x<d_size1; x++) {
            data[y][x] = Endian::ToBig(data[y][x]);
          }
        }
      }
    }
    /// Convert endianness of all elements if needed, for supported types.
    inline void FromBigEndian() { ToBigEndian(); }


    /// Efficiently swap all the contents of a and b.
    template <class T2> friend void swap (Data2D<T2> &a, Data2D<T2> &b);


    protected:
    /// @cond PROTECTED

    size_t d_size1;
    size_t d_size2;
    Data1D< Data1D<T> > data;
    Data1D< T* > cptrs;
    /// @endcond
  };


  /// Efficiently swap all the contents of a and b.
  template <class T2>
  void swap (Data2D<T2> &a, Data2D<T2> &b) {
    std::swap(a.d_size1, b.d_size1);
    std::swap(a.d_size2, b.d_size2);
    swap(a.data, b.data);
  }


  /// Outputs data to a stream as { { x_0_0, x_1_0, ...},
  /// { x_0_1, x_1_1, ...}, ... }
  /** Usage like:  Data2D<RStr> data;  std::cout << data << std::endl;
   */
  template <class T>
  inline std::ostream& operator<< (std::ostream &out, const Data2D<T>& d) {
    size_t i;

    if (d.size2() == 0) {
      out << "{ { } }";
    }
    else {
      out << "{ " << d[0];
      for (i=1; i<d.size2(); i++) {
        out << ", " << d[i];
      }
      out << " }";
    }

    return out;
  }
}


#endif // RC_DATA2D_H

