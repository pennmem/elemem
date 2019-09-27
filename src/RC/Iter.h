/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Iter.h
/// Provides a bounds-checked iterator that knows when its target
/// was deleted.
/////////////////////////////////////////////////////////////////////

#ifndef RC_ITER_H
#define RC_ITER_H

#include "Types.h"
#include "Macros.h"
#include "RevPtr.h"
#include <iterator>


namespace RC {
  /// A bounds-checked iterator for random-access containers that knows
  /// when its target was deleted.
  /** See Data1D.h for a usage example.
   */
  template <class Cont, class T>
  class RAIter
      : public std::iterator<std::random_access_iterator_tag,T,size_t> {
    public:

    /// Default constructor to nothing, by default invalid for use.
    inline RAIter() {
      pos = 0;
      c = NULL;
    }

    /// Creates an iterator to a container starting at index pos.
    /** When the iterator is dereferenced, it corresponds to
     *  (*container)[pos].  Note that the RevPtr for container should be set
     *  to AutoRevoke when the container is deleted.
     * @param container A RevPtr to the container.
     *  @param pos The starting index for this iterator.
     */
    inline RAIter(RevPtr<Cont> container, size_t pos) 
      : pos (pos),
        c (container) {
    }


    /// True if the iterator still points to an existing container and
    /// remains in bounds.
    inline bool IsValid() const {
      if ( c.IsSet() && (pos < c->size()) ) {
        return true;
      }
      else {
        return false;
      }
    }


    /// Return the index of the container which this iterator corresponds to.
    inline size_t GetIndex() const { return pos; }
    /// Directly access the RevPtr to the container.
    inline RevPtr<Cont> Raw() { return c; }


    /// Dereference the iterator, returning a reference to the corresponding
    /// element.
    inline T& operator* () { return (*c)[pos]; }
    /// Const version of operator*
    inline const T& operator* () const { return (*c)[pos]; }
    /// Provides access to members of the element this iterator points to.
    inline const T* operator-> () const { return &(*c)[pos]; }

    /// Provides access to the element at offset x from the current index.
    inline T& operator[] (size_t x) { return (*c)[pos+x]; }
    /// Const version of operator[]
    inline const T& operator[] (size_t x) const { return (*c)[pos+x]; }

    /// Point to the next higher numbered indexed element.
    inline RAIter<Cont,T>& operator++ () { pos++; return (*this); }
    /// Point to the next lower numbered indexed element.
    inline RAIter<Cont,T>& operator-- () { pos--; return (*this); }
    /// Increase the index by x.
    inline RAIter<Cont,T>& operator+= (size_t x) { pos += x; return (*this); }
    /// Decrease the index by x.
    inline RAIter<Cont,T>& operator-= (size_t x) { pos -= x; return (*this); }

    /// Support for iter++, but for efficiency, use ++iter.
    inline RAIter<Cont,T> operator++ (int) {
      RAIter<Cont,T> tmp = *this;
      ++*this;
      return tmp;
    }

    /// Support for iter--, but for efficiency, use --iter.
    inline RAIter<Cont,T> operator-- (int) {
      RAIter<Cont,T> tmp = *this;
      --*this;
      return tmp;
    }

    /// Returns a new iterator with an offset incremented by x.
    inline RAIter<Cont,T> operator+ (size_t x) const {
      RAIter<Cont,T> newiter(*this);
      newiter += x;
      return newiter;
    }

    /// Returns a new iterator with an offset decremented by x.
    inline RAIter<Cont,T> operator- (size_t x) const {
      RAIter<Cont,T> newiter(*this);
      newiter -= x;
      return newiter;
    }

    /// Returns the distance between two iterators.
    inline size_t operator- (const RAIter<Cont,T>& first) const {
      return (GetIndex() - first.GetIndex());
    }


    /// True if the two iterators have the same index.
    inline bool operator== (const RAIter<Cont,T>& other) const {
      return (GetIndex() == other.GetIndex());
    }

    /// True if this iterator is less.
    inline bool operator< (const RAIter<Cont,T>& other) const {
      return (GetIndex() < other.GetIndex());
    }

    RC_DEFAULT_COMPARISON();

    private:

    size_t pos;
    RevPtr<Cont> c;
  };


  /// Returns a new iterator with an offset incremented by x.
  template <class Cont, class T>
  inline RAIter<Cont,T> operator+ (size_t x, const RAIter<Cont,T>& iter) {
    RAIter<Cont,T> newiter = iter;
    newiter += x;
    return newiter;
  }

}

#endif // RC_ITER_H

