/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2015, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Data3D.h
/// Provides a bounds-safe three-dimensional resizeable structure.
/////////////////////////////////////////////////////////////////////

#ifndef RC_DATA3D_H
#define RC_DATA3D_H


#include "Data1D.h"
#include "Data2D.h"
#include "Errors.h"
#include <stdlib.h>


namespace RC {
  template <class T>
      class Data3D;
  template <class T>
      inline std::ostream& operator<< (std::ostream &out, const Data3D<T>& d);

  /// A bounds-safe three-dimensional resizeable structure.
  /** @see Data1D
   *  @see Data2D
   */
  template <class T>
  class Data3D {
    protected:
    /// @cond UNDOC
    void Initialize(size_t new_d_size1, size_t new_d_size2,
        size_t new_d_size3) {
      d_size1 = new_d_size1;
      d_size2 = new_d_size2;
      d_size3 = new_d_size3;

      data = Data1D< Data2D<T> >(d_size3);
      for (size_t i=0; i<d_size3; i++) {
        data[i] = Data2D<T>(d_size1, d_size2);
      }
    }
    /// @endcond

    public:

    /// Default constructor which initializes to size 0, 0, 0.
    Data3D() {
      Initialize(0, 0, 0);
    }

    /// Constructor which sets the initial sizes.
    /** Efficiency note:  The first dimension, set by d_size1, should be the
     *  one which is iterated over most frequently in innermost loops for
     *  caching gains.
     *  Note:  Non-POD classes stored in Data3D containers must have a
     *  default constructor with default values or no arguments.
     *  @param d_size1 The size of the first dimension.
     *  @param d_size2 The size of the second dimension.
     *  @param d_size3 The size of the third dimension.
     */
    explicit Data3D(size_t d_size1, size_t d_size2, size_t d_size3) {
      Initialize(d_size1, d_size2, d_size3);
    }


    /// Copy constructor that copies all elements.
    /** @param copy A source Data3D from which elements should be copied.
     */
    inline Data3D(const Data3D<T>& copy)
      : d_size1 (copy.d_size1),
        d_size2 (copy.d_size2),
        d_size3 (copy.d_size3) {

      data = copy.data;
    }

#ifdef CPP11
    /// Initializer list constructor, initializes with nested brackets.
    inline Data3D(const std::initializer_list<Data2D<T>>& new_data)
      : d_size1(0),
        d_size2(0),
        d_size3(new_data.size()),
        data(new_data.size()) {
      std::copy(new_data.begin(), new_data.end(), data.Raw());
      if (data.size()) {
        d_size2 = data[0].size2();
        d_size1 = data[0].size1();
      }
    }
#endif

    /// Deletes all contents upon destruction.
    ~Data3D() {
      Delete();
    }


    /// Delete all the elements and free all allocated memory.
    inline void Delete() {
      d_size1 = 0;
      d_size2 = 0;
      d_size3 = 0;
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

      for (size_t i=0; i<d_size3; i++) {
        data[i].Crop();
      }

      cptrs.Crop();
    }

    
    /** @return True of there are no elements / at least one size is 0.
     */
    inline bool IsEmpty() const {
      return ( (d_size1 == 0) || (d_size2 == 0) || (d_size3 == 0) );
    }


    /// Assignment operator which copies all contents from other.
    /** @param other Data3D to copy from.
     *  @return This object.
     */
    inline Data3D& operator= (const Data3D& other) {
      if (this != &other) {
        d_size1 = other.d_size1;
        d_size2 = other.d_size2;
        d_size3 = other.d_size3;
        data = other.data;
      }

      return *this;
    }


    /// Bounds-checked access of a Data2D corresponding to the data at
    /// index x in dimension 3.
    /** Throws an ErrorMsgBounds if out of bounds.  Usage note:
     *  Data3D<int> arr(size1, size2, size3);  int val =
     *  arr[x3][x2][x1];
     *  @param x The index of dimension 3.
     *  @return A Data2D of the elements in dimension 1 and 2 at that index.
     */
    inline Data2D<T>& operator[] (size_t x) { return data[x]; }
    /// Identical to Data3D::operator[]
    inline Data2D<T>& operator() (size_t x) { return data[x]; }
    /// Const version of Data3D::operator[]
    inline const Data2D<T>& operator[] (size_t x) const { return data[x]; }
    /// Const version of Data3D::operator[]
    inline const Data2D<T>& operator() (size_t x) const { return data[x]; }
    /// Bounds-checked access of an element.
    /** Throws an ErrorMsgBounds if out of bounds.  Note that for
     *  Data3D<int> arr;  arr(x, y, z) is equivalent to
     *  arr[z][y][x].
     *  @param x The index of dimension 1
     *  @param y The index of dimension 2.
     *  @param z The index of dimension 3.
     *  @return A reference to the indexed element.
     */
    inline T& operator() (size_t x, size_t y, size_t z) {
      return data[z][y][x];
    }
    /// Const version of Data3D::operator()(size_t x, size_t y, size_t z)
    inline const T& operator() (size_t x, size_t y, size_t z) const {
      return data[z][y][x];
    }
    /// Equivalent to Data3D::operator()(size_t x, size_t y, size_t z)
    inline T& At(size_t x, size_t y, size_t z) {
      return data[z][y][x];
    }
    /// Const version of Data3D::operator()(size_t x, size_t y, size_t z))
    inline const T& At(size_t x, size_t y, size_t z) const {
      return data[z][y][x];
    }

    /// Get the size of dimension 1.
    inline size_t size1() const { return d_size1; }
    /// Get the size of dimension 2.
    inline size_t size2() const { return d_size2; }
    /// Get the size of dimension 3.
    inline size_t size3() const { return d_size3; }

    /// Returns sizeof(T).
    inline size_t TypeSize() const { return sizeof(T); }


    /// Sets all elements equal to 0.
    /** @see Data1D::Zero() */
    inline void Zero() {
      int y;

      for (y=0; y<d_size3; y++) {
        data[y].Zero();
      }
    }


    /// Check if the indices x, y, and z are in bounds.
    /** @param x The index for dimension 1.
     *  @param y The index for dimension 2.
     *  @param z The index for dimension 3.
     *  @return True of in bounds.
     */
    inline bool Check(const size_t x, const size_t y, const size_t z) const {
      if ( ! ((x < d_size1) && (y < d_size2) && (z < d_size3)) ) {
        return false;
      }
      else {
        return true;
      }
    }


    /// Throw an ErrorMsgBounds exception if either x, y, or z is
    /// out of bounds.
    /** @param x The index for dimension 1.
     *  @param y The index for dimension 2.
     *  @param z The index for dimension 3.
     */
    inline void Assert(const size_t x, const size_t y, const size_t z) const {
      if ( ! Check(x, y, z) ) {
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
     *  @param resize_size3 The new size for dimension 2.
     */
    inline void Resize(const size_t resize_size1,
                       const size_t resize_size2,
                       const size_t resize_size3) {
      data.Resize(resize_size3);
      for (size_t i=0; i<resize_size2; i++) {
        data[i].Resize(resize_size1, resize_size2);
      }
      d_size3 = resize_size3;
      d_size2 = resize_size2;
      d_size1 = resize_size1;

      cptrs.Resize(0);  // Invalidates
    }


    /// Access a raw unprotected 3-dimensional C-array for the enclosed data.
    /** Warning:  This convenience function bypasses the bounds protections
     *  provided by this class.
     *  @return C-style pointer to the contents.
     */
    inline T*** Raw() {
      if (cptrs.size() != d_size3) {
        cptrs.Resize(d_size3);
        for (size_t i=0; i<d_size3; i++) {
          cptrs[i] = data[i].Raw();
        }
      }
      return cptrs.Raw();
    }


    /// Access the underlying nested Data1D/Data2D structure for this object.
    /** Attempts should not be made to resize the underlying data accessed
     *  with this convenience function.
     *  @return The nested Data1D/Data2D contained within.
     */
    inline const Data1D< Data2D<T> >& RawData() const {
      return data;
    }

    /// Const version of RawData().
    inline Data1D< Data2D<T> >& RawData() {
      return data;
    }


    /// Convert endianness of all elements if needed, for supported types.
    inline void ToLilEndian() {
      if (Endian::IsBig()) {
        for (size_t z=0; z<d_size3; z++) {
          for (size_t y=0; y<d_size2; y++) {
            for (size_t x=0; x<d_size1; x++) {
              data[z][y][x] = Endian::ToLittle(data[z][y][x]);
            }
          }
        }
      }
    }
    /// Convert endianness of all elements if needed, for supported types.
    inline void FromLilEndian() { ToLilEndian(); }
    
    /// Convert endianness of all elements if needed, for supported types.
    inline void ToBigEndian() {
      if (Endian::IsLittle()) {
        for (size_t z=0; z<d_size3; z++) {
          for (size_t y=0; y<d_size2; y++) {
            for (size_t x=0; x<d_size1; x++) {
              data[z][y][x] = Endian::ToBig(data[z][y][x]);
            }
          }
        }
      }
    }
    /// Convert endianness of all elements if needed, for supported types.
    inline void FromBigEndian() { ToBigEndian(); }


    /// Efficiently swap all the contents of a and b.
    template <class T2> friend void swap (Data3D<T2> &a, Data3D<T2> &b);


    protected:
    /// @cond PROTECTED

    size_t d_size1;
    size_t d_size2;
    size_t d_size3;
    Data1D< Data2D<T> > data;
    Data1D< T** > cptrs;
    /// @endcond
  };


  /// Efficiently swap all the contents of a and b.
  template <class T2>
  void swap (Data3D<T2> &a, Data3D<T2> &b) {
    std::swap(a.d_size1, b.d_size1);
    std::swap(a.d_size2, b.d_size2);
    std::swap(a.d_size3, b.d_size3);
    swap(a.data, b.data);
  }


  /// Outputs data to a stream.
  /** For d_x_y_z, formatted as { { { d_0_0_0, d_1_0_0, ...},
   *  { d_0_1_0, d_1_1_0, ...}, ... }, { { d_0_0_1, d_1_0_1, ...},
   * { d_0_1_1, d_1_1_1, ...}, ... }, ... }
   * Usage like:  Data3D<RStr> data;  std::cout << data << std::endl;
   */
  template <class T>
  inline std::ostream& operator<< (std::ostream &out, const Data3D<T>& d) {
    size_t i;

    if (d.size3() == 0) {
      out << "{ { { } } }";
    }
    else {
      out << "{ " << d[0];
      for (i=1; i<d.size3(); i++) {
        out << ", " << d[i];
      }
      out << " }";
    }

    return out;
  }
}


#endif // RC_DATA3D_H

