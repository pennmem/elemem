/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2016, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Data1D.h
/// Provides a one-dimensional vector-like structure.
/////////////////////////////////////////////////////////////////////

#ifndef RC_DATA1D_H
#define RC_DATA1D_H

#include "RCconfig.h"
#include "Errors.h"
#include "Iter.h"
#include "Macros.h"
#include "Types.h"
#include <algorithm>
#include <stdlib.h>
#include <iostream>
#ifdef CPP11
#include <type_traits>
#endif


namespace RC {
  /// @cond EXTERNAL
  extern u64 RND_Get_Range(u64);
  extern u64 URand_Get_Range(u64);
  class RND;
  extern RND& RND_Gen();
  class URand;
  extern URand& URand_Gen();
  /// @endcond

  /// A bounds-safe one-dimensional vector-like structure.
  /** It provides efficient resizing and offsets, as well as convenience
   *  functions for assignments and comparisons.  It also provides
   *  bounds-safe iterators.
   *  Note:  Non-POD classes stored in Data1D containers must have a
   *  default constructor with default values or no arguments, which is
   *  necessary for efficient resizing.  This requirement also permits
   *  safe use of offsets and temporary shrinking, because all allocated
   *  space contains defined data with properly constructed objects. 
   *  @see Data2D
   *  @see Data3D
   */
  template <class T>
  class Data1D {
    protected:
    /// @cond PROTECTED

    RevPtr< Data1D<T> > rev_ptr;

    size_t d_alloc;
    T *alloc_data;

    size_t offset;
    size_t d_size;
    T *data;
    bool do_delete;

    inline void DelArray() {
      if ((alloc_data != NULL) && (do_delete)) {
        delete[] (alloc_data);
      }
      alloc_data = NULL;
      data = NULL;
    }

    inline void AllocArray(const size_t newsize) {
      try {
        alloc_data = new T[newsize];
      }
      catch (std::bad_alloc &) {
        Throw_RC_Type(Memory, "Allocation error");
      }
      data = alloc_data + offset;
      d_alloc = newsize;
      do_delete = true;
    }

    void Initialize(size_t new_d_size) {
      offset = 0;
      d_size = new_d_size;

      if (d_size > 0) {
        AllocArray(d_size);
      }
      else {
        d_alloc = 0;
        alloc_data = NULL;
        data = NULL;
      }

      rev_ptr = this;
      rev_ptr.AutoRevoke();
    }

    /// @endcond
    public:

    // Default constructor, which initializes to zero elements.
    inline Data1D() {
      Initialize(0);
    }

    /// Constructor which sets the initial size to d_size.
    /** @param d_size The initial number of elements.
     */
    explicit inline Data1D(size_t d_size) {
      Initialize(d_size);
    }

    /// Copy constructor that copies all elements.
    /** @param copy A source Data1D from which elements should be copied.
     */
    inline Data1D(const Data1D<T>& copy)
      : offset (0),
        d_size (copy.d_size) {
      AllocArray(d_size);
      std::copy(copy.data, copy.data + copy.d_size, data);

      rev_ptr = this;
      rev_ptr.AutoRevoke();
    }

#ifdef CPP11
    /// Initializer list constructor, for initializing with bracketed data.
    /** @param new_data An initialization list of the form {elem1, elem2, ...}.
     */
    inline Data1D(const std::initializer_list<T>& new_data)
      : offset(0),
        d_size(new_data.size()) {
      AllocArray(d_size);
      std::copy(new_data.begin(), new_data.end(), data);

      rev_ptr = this;
      rev_ptr.AutoRevoke();
    }


    /// Move constructor that reassigns all the data to this Data1D.
    /** @param other A source Data1D from which elements should be moved.
     */
    inline Data1D(Data1D<T>&& other)
      : rev_ptr(other.rev_ptr)
      , d_alloc(other.d_alloc)
      , alloc_data(other.alloc_data)
      , offset(other.offset)
      , d_size(other.d_size) 
      , data(other.data)
      , do_delete(other.do_delete) {

      rev_ptr.AutoRevoke();
      other.rev_ptr.AutoRevoke(false);
      other.d_alloc = 0;
      other.alloc_data = NULL;
      other.do_delete = false;
      other.offset = 0;
      other.d_size = 0;
      other.data = NULL;
    }
#endif // CPP11

    /// Efficiently wraps in-place C-pointer data in a bounds-safe container.
    /** Note:  If auto_delete=true, new_data must have been acquired with
     *  new[].  delete[] is used internally, so behavior may vary if it was
     *  acquired with new, malloc, or others.
     *  @param d_size The initial number of elements.
     *  @param new_data C-pointer data to wrap.
     *  @param auto_delete Should the Data1D delete new_data upon destruction?
     */
    inline Data1D(size_t d_size, T* new_data, bool auto_delete = false)
      : d_alloc (d_size),
        alloc_data (new_data),
        offset (0),
        d_size (d_size),
        data (new_data),
        do_delete (auto_delete) {

      rev_ptr = this;
      rev_ptr.AutoRevoke();
    }


    /// Delete all the elements and free all allocated memory.
    inline void Delete() {
      d_size = 0;
      d_alloc = 0;
      offset = 0;
      DelArray();
    }


    /// Deletes all contents upon destruction.
    /** Note:  For wrapped pointers, if auto_delete was false no deletion
     *  of the original data occurs.
     */
    inline ~Data1D() {
      Delete();
    }



    /// Returns true if there are no elements / the size is 0.
    inline bool IsEmpty() const {
      return (d_size == 0);
    }


    /// Returns the first index at or after start_at for which the data
    /// equals elem.
    /** @param elem The element to compare to each entry in this Data1D
     *  @param start_at The first index to begin comparing at.
     *  @return The index of the item found, or npos if no matching
     *  data is found.
     */
    template<class T2>
    inline size_t Find(const T2& elem, size_t start_at=0) const {
      for (size_t i=start_at; i<d_size; i++) {
        if (data[i] == elem) {
          return i;
        }
      }
      return npos;
    }

    /// Returns true if at least one entry equals elem.
    template<class T2>
    inline bool Contains(const T2& elem) const {
      return (Find(elem) != npos);
    }


    /// Check if index x is in bounds.
    /** @param x The index to check.
     *  @return True if in bounds.
     */
    inline bool Check(const size_t x) const {
      if (x < d_size) {
        return true;
      }
      else {
        return false;
      }
    }


    protected:
    /// @cond PROTECTED
    inline void OutOfBounds() const {
      Throw_RC_Type(Bounds, "Out of bounds");
    }
    /// @endcond
    public:


    /// Throws an ErrorMsgBounds exception if x is out of bounds.
    /** @param x The index to check.
     */
    inline void Assert(const size_t x) const {
      if ( ! Check(x) ) {
        OutOfBounds();
      }
    }

    /// Reserve storage without resizing the array.
    /** @param reserve_size The number of elements worth of storage to reserve.
     **/
    inline void Reserve(const size_t reserve_size) {
      T *tmp;
      size_t reserve_offset = reserve_size + offset;

      if (reserve_offset <= d_alloc) {
        return;
      }

      tmp = new T[reserve_offset];
#ifdef CPP11
      std::move(alloc_data, alloc_data + d_alloc, tmp);
#else
      std::copy(alloc_data, alloc_data + d_alloc, tmp);
#endif
      d_alloc = reserve_offset;

      DelArray();

      alloc_data = tmp;
      data = alloc_data + offset;
      do_delete = true;
    }


    /// Resize the array, reallocating if necessary.
    /** This may trigger a copy operation upon expansion.  For efficiency it
     *  never reallocates or copies while shrinking or expanding within a
     *  previous size range.  Use Crop if necessary to shrink storage to the
     *  current size.
     *  @param resize_size The new number of elements.
     */
    inline void Resize(const size_t resize_size) {
      Reserve(resize_size);
      d_size = resize_size;
    }


    /// Set a new offset position for index 0.
    /** This also adjusts the size to reach the same last element.
     *  @param new_offset The new absolute offset from the 0th allocated
     *  element.
     *  @see SetRange
     */
    inline void SetOffset(const size_t new_offset) {
      if (new_offset > d_alloc) {
        OutOfBounds();
      }

      if ( (d_size + offset) > new_offset ) {
        d_size += offset - new_offset;
      }
      else {
        d_size = 0;
      }

      offset = new_offset;
      data = alloc_data + offset;
    }


    /// Set a new offset and data size, resizing if necessary.
    /** @param new_offset The new absolute offset from the 0th allocated
     *  element.
     *  @param new_size The new number of elements.
     *  @see SetOffset
     *  @see Resize
     */
    inline void SetRange(const size_t new_offset, const size_t new_size) {
      if (new_offset > d_alloc) {
        OutOfBounds();
      }
      offset = new_offset;
      data = alloc_data + offset;

      Resize(new_size);
    }


    /// Access a raw unprotected C-pointer to the enclosed data.
    /** Warning:  This convenience function bypasses the bounds protections
     *  provided by this class.  The returned pointer is likely to become
     *  invalid after any of the operations on this object which change the
     *  size.
     *  @return C-style pointer to the contents at the offset (0 by default).
     */
    inline T* Raw() const { return data; }

    /// Returns the current number of elements.
    inline size_t size() const { return d_size; }
    /// Returns the size of this Data1D's type.
    inline size_t TypeSize() const { return sizeof(T); }
    /// Returns the current size in bytes.
    inline size_t ByteSize() const { return d_size*sizeof(T); }
    /// Returns the number of elements for which space is reserved.
    inline size_t reserved() const { return d_alloc; }
    /// Returns the current allocated storage in bytes.
    inline size_t ByteReserved() const { return d_alloc*sizeof(T); }
    /// Returns the current offset for index 0.
    /** @see SetOffset
     *  @see SetRange
     **/
    inline size_t GetOffset() const { return offset; }


    /// Creates a duplicate copy of the contents, with up to amnt elements
    /// from pos.
    inline Data1D Copy (const size_t pos=0, const size_t amnt=npos) const {
      Data1D<T> dup;
      dup.CopyFrom(*this, pos, amnt);
      return dup;
    }


    /// Copy data from any type with a compatible assignment operator.
    /** @param other The compatible Data1D from which data should be copied.
     *  @return A reference to this Data1D.
     */
    template <class T2>
    inline Data1D& CopyFrom (const Data1D<T2>& other) {
      if (reinterpret_cast<void*>(data) ==
          reinterpret_cast<void*>(other.Raw())) {
        return *this;
      }

      if (d_alloc < other.size()) {
        DelArray();

        AllocArray(other.size());
      }
      d_size = other.size();
      offset = 0;

      std::copy(other.Raw(), other.Raw() + other.size(), data);

      rev_ptr = this;
      rev_ptr.AutoRevoke();

      return *this;
    }


    /// assignment operator.
    /** Note, to copy data around in the same Data1D, use CopyData.
     *  @param other The compatible Data1D from which data should be copied.
     *  @param pos The offset in other to start copying from.
     *  @param num_elem The number of elements to copy.
     *  @return A reference to this Data1D.
     */
    template <class T2>
    inline Data1D& CopyFrom (const Data1D<T2>& other,
                          size_t pos, size_t num_elem=npos) {
      if (reinterpret_cast<void*>(data) ==
          reinterpret_cast<void*>(other.Raw())) {
        if (pos == 0 && num_elem > d_size) {
          return *this;
        }
        else {
          Throw_RC_Type(Bounds, "CopyFrom on a subset of the same underlying "
              "data is invalid.  Use CopyData.");
        }
      }

      // If out of bounds, but copying no elements, just blank the array.
      // Otherwise, throw exception.
      if ( ! other.Check(pos) ) {
        if (num_elem == 0) {
          d_size = 0;
          offset = 0;
          return *this;
        }
        else {
          other.Assert(pos);
        }
      }

      size_t real_num_elem = num_elem;
      if (num_elem > (other.size()-pos)) {
        real_num_elem = other.size() - pos;
      }

      if (d_alloc < real_num_elem) {
        DelArray();

        AllocArray(real_num_elem);
      }
      d_size = real_num_elem;
      offset = 0;

      std::copy(other.Raw()+pos, other.Raw()+pos+real_num_elem, data);

      rev_ptr = this;
      rev_ptr.AutoRevoke();

      return *this;
    }


    /// Overwrite a range of data using all data from any compatible type,
    /// expanding if necessary.
    /** @param pos The offset in this Data1D to begin overwriting.
     *  @param other The compatible Data1D to copy from.
     */
    template <class T2>
    inline void CopyAt (const size_t pos, const Data1D<T2>& other) {
      if (pos > d_size) {
        OutOfBounds();
      }

      size_t other_size = other.size();  // cache for self-copy
      size_t copy_end = pos + other_size;
      if (d_size < copy_end) {
        Resize(copy_end);
      }

      std::copy(other.Raw(), other.Raw() + other_size, data + pos);
    }


    /// Overwrite a range of data from any compatible type, expanding if
    /// necessary.
    /** @param pos The offset in this Data1D to begin overwriting.
     *  @param other The compatible Data1D to copy from.
     *  @param other_start The offset in the other Data1D to begin copying
     *  from.
     *  @param amnt The quantity of data to copy (automatically
     *  bounds-capped, defaults to all).
     */
    template <class T2>
    inline void CopyAt (const size_t pos, const Data1D<T2>& other,
                        const size_t other_start, const size_t amnt=npos) {
      if (pos > d_size) {
        OutOfBounds();
      }

      other.Assert(other_start);

      size_t real_amnt = amnt;
      size_t to_end = other.size() - other_start;

      if ( real_amnt > to_end) {
        real_amnt = to_end;
      }

      size_t copy_end = pos + real_amnt;
      if (d_size < copy_end) {
        Resize(copy_end);
      }

      T2* s_first = other.Raw() + other_start;
      T2* s_last = other.Raw() + other_start + real_amnt;
      T* d_first = data + pos;
      if (reinterpret_cast<void*>(d_first) >
          reinterpret_cast<void*>(s_last) ||
          reinterpret_cast<void*>(d_first) <=
          reinterpret_cast<void*>(s_first)) {
        std::copy(s_first, s_last, d_first);
      }
      else {
        T* d_last = data + pos + real_amnt;
        std::copy_backward(s_first, s_last, d_last);
      }
    }


    /// Copy data from any location in this Data1D to another location,
    /// handling overlap automatically.
    /** @param dest The offset in this Data1D to begin overwriting.
     *  @param source The offset in this Data1D to copy from.
     *  @param amnt The quantity of data to copy (automatically
     *  bounds-capped, defaults to all).
     */
    inline void CopyData (const size_t dest, const size_t source,
                          const size_t amnt=npos) {
      CopyAt(dest, *this, source, amnt);
    }


    /// Assignment operator which copies all contents from other, respecting
    /// offsets.
    /** @param other Data1D to copy from, from offset (default 0) to size.
     *  @return This object.
     */
    inline Data1D& operator= (const Data1D& other) {
      return (*this).CopyFrom(other);
    }


#ifdef CPP11
    /// Assignment operator which copies all contents from other, respecting
    /// offsets.
    /** @param other Data1D to copy from, from offset (default 0) to size.
     *  @return This object.
     */
    inline Data1D& operator= (Data1D&& other) {
      Delete();
      swap(*this, other);
      return *this;
    }
#endif


    /// Returns a new array with all the elements of this array assigned to
    /// type T2.
    /** @see CopyFrom
     */
    template<class T2>
    inline Data1D<T2> Cast() {
      Data1D<T2> retval;
      return retval.CopyFrom(*this);
    }

#ifdef CPP11
    /// Returns a new array with all the elements of this array converted
    /// to another type by converter.
    template<class Conv>
    inline auto CastWith(Conv converter) -> Data1D<decltype(converter(*data))> {
      Data1D<decltype(converter(*data))> retval(d_size);
      for (size_t i=0; i<d_size; i++) {
        retval[i] = converter(data[i]);
      }
      return retval;
    }
#endif // CPP11

    /// Returns a new array with the raw data of this Data1D reinterpreted
    /// as type T2.
    /** Warning, this carries the same risks and caveats as C-type casting
     *  the raw data before assignment, and has undefined behavior for
     *  non-POD types in an ambiguous state.  If the data sizes do not fit
     *  to an integral size of the returned type then there will be extra
     *  undefined bytes at the end of the returned data.
     *  @see Put
     *  @see Get
     */
    template<class T2>
    inline Data1D<T2> Reinterpret() const {
      size_t bytesize = d_size * sizeof(T);
      size_t newsize = bytesize / sizeof(T2) +
                       (((bytesize % sizeof(T2))>0)?1:0);
      Data1D<T2> retval(newsize);
      unsigned char *srcptr = reinterpret_cast<unsigned char*>(data);
      unsigned char *destptr = reinterpret_cast<unsigned char*>(retval.Raw());
      std::copy(srcptr, srcptr+bytesize, destptr);
      return retval;
    }


    /// Copy data from any other object of a type with a compatible

    /// Return a bounds-checked random-access iterator starting at offset.
    /** Note that the iterator is revokable, meaning use of it will throw an
     *  exception if this Data1D is deleted.
     *  @return A bounds-checked iterator.
     *  @see end()
     */
    inline RAIter<Data1D<T>,T> begin() {
      return RAIter<Data1D<T>,T> (rev_ptr, 0);
    }
    /// Const version of begin.
    inline const RAIter<Data1D<T>,T> begin() const {
      return RAIter<Data1D<T>,T> (rev_ptr, 0);
    }


    /// Return a bounds-checked random-access iterator starting just past
    /// the last element.
    /** Note that the iterator is revokable, meaning use of it will throw an
     *  exception if this Data1D is deleted.
     *  @return A bounds-checked iterator.
     *  @see begin()
     */
    inline RAIter<Data1D<T>,T> end() {
      return RAIter<Data1D<T>,T> (rev_ptr, d_size);
    }
    /// Const version end.
    inline const RAIter<Data1D<T>,T> end() const {
      return RAIter<Data1D<T>,T> (rev_ptr, d_size);
    }


    /// Bounds-checked access of the element at index x from the offset
    /// (default 0).
    /** Throws an ErrorMsgBounds exception if x is out of bounds.
     *  @param x Index to access, where 0 is the element at offset.
     *  @return Reference to the element at index x.
     */
    inline T& operator[] (size_t x) { Assert(x); return data[x]; }
    /// Identical to Data1D::operator[]
    inline T& operator() (size_t x) { Assert(x); return data[x]; }
    /// Const version of Data1D::operator[]
    inline const T& operator[] (size_t x) const { Assert(x); return data[x]; }
    /// Const version of Data1D::operator[]
    inline const T& operator() (size_t x) const { Assert(x); return data[x]; }
    /// Identical to Data1D::operator[]
    inline T& At(size_t x) { Assert(x); return data[x]; }
    /// Const version of Data1D::operator[]
    inline const T& At(size_t x) const { Assert(x); return data[x]; }


    /// Provides the last element.
    /** @return A reference to the last element.
     */
    inline T& Last() { Assert(d_size-1); return data[d_size-1]; }
    /// Const version of last()
    inline const T& Last() const { Assert(d_size-1); return data[d_size-1]; }


    /// Sets all elements equal to 0.
    /** Note: Types for Data1D do not need to have a defined assignment
     *  operator which can receive a 0 unless this function is called
     *  for that Data1D instance.
     */
    inline void Zero() {
      size_t i;

      for (i=0; i<d_size; i++) {
        data[i] = 0;
      }
    }

    /// Like Zero() but operating only between index start and end, inclusive.
    /** If end is unspecified, defaults to the end of the data.  If either is
     *  out of bounds, ErrorMsgBounds is thrown.
     */
    inline void ZeroRange(const size_t start, const size_t end=npos) {
      Assert(start);
      if (end == 0 || end < start) {
        OutOfBounds();
      }
      size_t realend = end;
      if (realend == size_t(-1)) {
        realend = d_size-1;
      }
      Assert(end);
      
      for (size_t i=start; i<=end; i++) {
        data[i] = 0;
      }
    }


    /// Sorts the elements according to T::operator< in N*log2(N) time.
    inline void Sort() {
      std::sort(data, data + d_size);
    }

    /// Sorts the elements according to the binary comparitor comp.
    /** comp should return true if less, and should not modify the elements
     *  passed.
     */
    template <class Comparator>
    inline void Sort(Comparator comp) {
      std::sort(data, data + d_size, comp);
    }


    /// Randomizes the order of all the elements.
    /** Performs a Mersenne Twister random shuffle of the elements,
     *  microsecond time-seeded on the first call.
     */
    inline void Shuffle() {
#ifdef CPP11
      std::shuffle(data, data + d_size, RND_Gen());
#else
      std::random_shuffle(data, data + d_size, RND_Get_Range);
#endif
    }
#ifdef unix
    /// Randomizes the order of all the elements using /dev/urandom.
    /** Uses /dev/urandom, available on many unix implementations, to perform
     *  a shuffle which is reseeded with environmental randomness.
     */
    inline void URandShuffle() {
#ifdef CPP11
      std::shuffle(data, data + d_size, URand_Gen());
#else
      std::random_shuffle(data, data + d_size, URand_Get_Range);
#endif
    }
#endif


    /// Removes ownership of the contained data from the Data1D object.
    /** Note, the array returned becomes the responsibility of the calling
     *  function, and should be delete[]'d externally!  The pointer
     *  returned is as if the offset were set to 0 first.  This Data1D
     *  is empty after calling this.
     */
    inline T* Extract() {
      T* retval = alloc_data;
      alloc_data = NULL;
      data = NULL;
      d_size = 0;
      d_alloc = 0;
      offset = 0;
      return retval;
    }


    /// Reduces memory consumption to only that necessary for size() elements.
    /** After calling, offset is 0 and points to the element indexed as 0
     *  before calling.  Efficiency note:  This function may copy all elements
     *  if necessary for reallocating.
     */
    inline void Crop() {
      T *tmp;

      if ((offset == 0) && (d_alloc <= d_size)) {
        return;
      }

      d_alloc = d_size;

      if (d_alloc == 0) {
        Delete();
        return;
      }

      tmp = new T[d_alloc];

#ifdef CPP11
      std::move(data, data + d_size, tmp);
#else
      std::copy(data, data + d_size, tmp);
#endif

      DelArray();

      alloc_data = tmp;
      offset = 0;
      data = alloc_data;
      do_delete = true;
    }


    /// Identical to Delete().
    inline void Clear() {
      Delete();
    }


    /// Add an element to the end, expanding if necessary.
    /** Efficiency note:  Expands by doubling for linear efficiency.
     *  @param newT The new element to append to the end.
     *  @see Reserve
     */
    inline void Append(const T& newT) {
      size_t growto;
      size_t offset_size = d_size + offset;

      if (d_alloc <= offset_size) {
        growto = d_size << 1;
        if (growto <= d_size) {
          growto = d_size + 1;
        }
        Reserve(growto);
      }

      data[d_size++] = newT;
    }


    /// Append data from any type with a compatible assignment operator.
    /** @param other The compatible Data1D from which data should be copied.
     */
    template <class T2>
    inline void AppendFrom (const Data1D<T2>& other) {
      size_t last_d_size = d_size;
      size_t other_size = other.size();

      Resize(d_size + other_size);

      std::copy(other.Raw(), other.Raw() + other_size, data + last_d_size);
    }


    /// Appends all elements in other to the end, expanding if necessary.
    /** @param other The Data1D to append to the end.
     */
    inline void Append(const Data1D& other) {
      AppendFrom(other);
    }


    /// Assign newT to index pos, expanding if necessary to reach that index.
    /** @param pos The index to which newT should be assigned.
     *  @param newT The new element to assign.
     */
    inline void ExpandSet(size_t pos, const T& newT) {
      if (!Check(pos)) {
        Resize(pos+1);
      }
      data[pos] = newT;
    }


    /// Inserts newT at position pos, shifting the other elements.
    /** Efficiency note:  For random positions, on average O(N).
     *  @param pos The index at which newT should be inserted.
     *  @param newT The new element to insert.
     */
    inline void Insert(size_t pos, const T& newT) {
      Assert(pos);

      Resize(d_size + 1);

      std::copy_backward(data+pos, data+size(), data+size()+1);
      data[pos] = newT;
    }


    /// Removes the element at pos from the array, shrinking the size.
    /** Efficiency note:  For random positions, on average O(N).
     *  @param pos The index at which an element should be removed.
     */
    inline void Remove(size_t pos) {
      Assert(pos);
      if (Check(pos+1)) {
#ifdef CPP11
        std::move(data+pos+1, data+size(), data+pos);
#else
        std::copy(data+pos+1, data+size(), data+pos);
#endif
      }
      Resize(d_size - 1);
    }


    /// Add an element to the end, expanding if necessary.
    /** Efficiency note:  Expands by doubling for linear efficiency.
     *  @param newT The new element to append to the end.
     *  @return A reference to this Data1D.
     *  @see Reserve
     */
    inline Data1D<T>& operator+= (const T& newT) {
      Append(newT);
      return (*this);
    }


    /// Appends all elements in other to the end, expanding if necessary.
    /** @param other The Data1D to append to the end.
     *  @return A reference to this Data1D.
     */
    inline Data1D<T>& operator+= (const Data1D& other) {
      Append(other);
      return (*this);
    }


    /// Create a new array from this Data1D concatenated with other after it.
    inline Data1D<T> operator+ (const Data1D& other) const {
      Data1D<T> retval(size()+other.size());
      retval.CopyAt(0, *this);
      retval.CopyAt(size(), other);
      return retval;
    }


    /// Convert endianness of all elements if needed, for supported types.
    inline void ToLilEndian() {
      if (Endian::IsBig()) {
        for (size_t i=0; i<d_size; i++) {
          data[i] = Endian::ToLittle(data[i]);
        }
      }
    }
    /// Convert endianness of all elements if needed, for supported types.
    inline void FromLilEndian() { ToLilEndian(); }

    /// Convert endianness of all elements if needed, for supported types.
    inline void ToBigEndian() {
      if (Endian::IsLittle()) {
        for (size_t i=0; i<d_size; i++) {
          data[i] = Endian::ToBig(data[i]);
        }
      }
    }
    /// Convert endianness of all elements if needed, for supported types.
    inline void FromBigEndian() { ToBigEndian(); }


    /// Comparison operator for all elements.
    /** Also returns false if they are differently sized.  Works for
     *  AnyValidType with operator== defined with T.
     *  @param other The Data1D to compare all elements with.
     *  @return True if all elements are equal.
     */
    template<class AnyValidType>
    inline bool operator== (const Data1D<AnyValidType>& other) const {
      size_t i;

      if (d_size != other.d_size) {
        return false;
      }

      if (data == other.data) {
        return true;  // No need to compare equivalent memory.
      }

      for (i=0; i<d_size; i++) {
        if ( ! (data[i] == other.data[i]) ) {
          return false;
        }
      }

      return true;
    }


    /// Returns true if the first different element for this is less than for
    /// other.
    /** If they are differently sized and identical through the common length,
     *  then the shorter one is considered less.  Works for AnyValidType with
     *  defined operator< in both directions with T.
     *  @param other The Data1D to compare all elements with.
     *  @return True if the first non-equal element for this is less than for
     *  other.
     */
    template<class AnyValidType>
    inline bool operator< (const Data1D<AnyValidType>& other) const {
      size_t i, minsize;
      bool left_smaller;

      if (d_size < other.d_size) {
        minsize = d_size;
        left_smaller = true;
      }
      else {
        if (data == other.data && d_size == other.d_size) {
          return false;  // No need to compare equivalent memory.
        }
        minsize = other.d_size;
        left_smaller = false;
      }


      for (i=0; i<minsize; i++) {
        if (data[i] < other.data[i]) {
          return true;
        }
        else if (other.data[i] < data[i]) {
          return false;
        }
      }


      return left_smaller;
    }

    RC_DEFAULT_COMPARISON()


    /// Direct raw data extraction of type T2 at index.
    /** Bounds are checked.  But if the raw data at the index is invalid
     *  for an assignment operation of type T2 (e.g., non-POD in ambiguous
     *  state), then behavior is undefined.
     *  @param store_at The reference to which data is written.
     *  @param index_T The index from which the data is extracted.
     */
    template<class T2>
    inline void Get(T2& store_at, size_t index_T) const {
      size_t end_index = index_T + sizeof(T2) / sizeof(T) +
                         (((sizeof(T2)%sizeof(T)) != 0) ? 1 : 0);
      if (end_index>0) { end_index--; }
      Assert(index_T);  Assert(end_index);
      store_at = *reinterpret_cast<T2*>(data+index_T);
    }
    /// Identical to the Get which receives store_at, but this returns the
    /// extracted value.
    template<class T2>
    inline T2 Get(size_t index_T=0) const {
      T2 retval;
      Get(retval, index_T);
      return retval;
    }
    /// Fills the array store_at to its current size with raw assigned
    /// data of type T2 from this Data1D.
    /** Usage example:  Data1D<u64> x(1);  Data1D<u8> y(8);  x.Get(y,0);
     *  y.Get(x,0);
     *  If store_at has size 0, it resizes and grabs all available data.
     */
    template<class T2>
    inline void Get(Data1D<T2>& store_at, size_t index_T=0) const {
      if (store_at.size() == 0) {
        store_at.Resize(((size()-index_T)*sizeof(T))/sizeof(T2));
      }
      size_t T2_fullsize = sizeof(T2)*store_at.size();
      size_t end_index = index_T + T2_fullsize / sizeof(T) +
                         (((T2_fullsize%sizeof(T)) != 0) ? 1 : 0);
      if (end_index>0) { end_index--; }
      Assert(index_T);  Assert(end_index);
      T2* T2_data_ptr = reinterpret_cast<T2*>(data+index_T);
      std::copy(T2_data_ptr, T2_data_ptr + store_at.size(), store_at.Raw());
    }

    /// Direct raw data insertion of type T2 at index.
    /** Bounds are checked.  But if the raw data at the index is invalid
     *  for an assignment operation of type T2 (e.g., not-POD in ambiguous
     *  state), then behavior is undefined.
     *  @param read_from The reference from which data is read.
     *  @param index_T The index at which data is placed.
     *  @return *this
     */
    template<class T2>
    inline Data1D<T>& Put(const T2& read_from, size_t index_T) {
      size_t end_index = index_T + sizeof(T2) / sizeof(T) +
                         (((sizeof(T2)%sizeof(T)) != 0) ? 1 : 0);
      if (end_index>0) { end_index--; }
      Assert(index_T);  Assert(end_index);
      *reinterpret_cast<T2*>(data+index_T) = read_from;
      return *this;
    }
    /// Like Get for a T2 type, but places sequential packed data.
    /** Copies the full array read_from with raw assigned data of type T2,
     *  to this Data1D starting at index, resizing as needed to include all
     *  of read_from.
     *  Usage example:  Data1D<u64> x(1);  Data1D<u8> y(8);  x.Put(y,0);
     *  y.Put(x,0);
     *  @return *this
     */
    template<class T2>
    inline Data1D<T>& Put(const Data1D<T2>& read_from, size_t index_T=0) {
      size_t T2_fullsize = sizeof(T2)*read_from.size();
      size_t T1_sizeneeded = index_T + T2_fullsize / sizeof(T) +
                             (((T2_fullsize%sizeof(T)) != 0) ? 1 : 0);
      if (d_size < T1_sizeneeded) {
        Resize(T1_sizeneeded);
      }
      T2* T2_data_ptr = reinterpret_cast<T2*>(data+index_T);
      std::copy(read_from.Raw(), read_from.Raw()+read_from.size(), T2_data_ptr);
      return *this;
    }


    /// Efficiently swap all the contents of a and b.
    template <class T2> friend void swap (Data1D<T2> &a, Data1D<T2> &b);

    /// The largest possible value of size_t.
    static const size_t npos = size_t(-1);
  };


  /// Outputs all chars to a stream without terminating at null.
  /** Usage like:  Data1D<char> data;  std::cout << data << std::endl;
   */
  inline std::ostream& operator<< (std::ostream &out, const Data1D<char>& d) {
    size_t i;
    for (i=0; i<d.size(); i++) {
      out << d[i];
    }
    return out;
  }

  /// Outputs data to a stream as { 61, 62, ... }
  /** Usage like:  Data1D<u8> data;  std::cout << data << std::endl;
   */
  inline std::ostream& operator<< (std::ostream &out, const Data1D<u8>& d) {
    size_t i;

    if (d.size() == 0) {
      out << "{ }";
    }
    else {
      out << "{ " << u16(d[0]);
      for (i=1; i<d.size(); i++) {
        out << ", " << u16(d[i]);
      }
      out << " }";
    }

    return out;
  }

  /// Outputs data to a stream as { 61, -42, ... }
  /** Usage like:  Data1D<i8> data;  std::cout << data << std::endl;
   */
  inline std::ostream& operator<< (std::ostream &out, const Data1D<i8>& d) {
    size_t i;

    if (d.size() == 0) {
      out << "{ }";
    }
    else {
      out << "{ " << i16(d[0]);
      for (i=1; i<d.size(); i++) {
        out << ", " << i16(d[i]);
      }
      out << " }";
    }

    return out;
  }


  /// Outputs data to a stream as { elem0, elem1, ... }
  /** Usage like:  Data1D<RStr> data;  std::cout << data << std::endl;
   */
  template <class T>
  inline std::ostream& operator<< (std::ostream &out, const Data1D<T>& d) {
    size_t i;

    if (d.size() == 0) {
      out << "{ }";
    }
    else {
      out << "{ " << d[0];
      for (i=1; i<d.size(); i++) {
        out << ", " << d[i];
      }
      out << " }";
    }

    return out;
  }


  /// Efficiently swap all the contents of a and b.
  template <class T>
  void swap (RC::Data1D<T> &a, RC::Data1D<T> &b) {
    std::swap(a.do_delete, b.do_delete);
    std::swap(a.d_size, b.d_size);
    std::swap(a.d_alloc, b.d_alloc);
    std::swap(a.offset, b.offset);
    std::swap(a.data, b.data);
    std::swap(a.alloc_data, b.alloc_data);
  }

#ifdef CPP11
  /// Convenience generator for safely converting C-style arrays.
  template<class T, size_t N> auto MakeData1D(T (&arr)[N]) -> RC::Data1D<T> {
    return RC::Data1D<T>(N,arr);
  }
#endif

}



#endif // RC_DATA1D_H

