#ifndef VALITER_H
#define VALITER_H

#include "RC/Types.h"
#include "RC/Macros.h"
#include "RC/RevPtr.h"
#include <iterator>


namespace CML {
  /// A bounds-checked value-wrapping iterator for random-access containers
  /// that knows / when its target was deleted.
  template <class Cont, class T>
  class ValIter

      : public std::iterator<std::random_access_iterator_tag,T,size_t> {
    public:

    /// Default constructor to nothing, by default invalid for use.
    inline ValIter() {
      pos = 0;
      c = NULL;
    }

    /// Creates an iterator to a container starting at index pos.
    /** When the iterator is dereferenced, it corresponds to
     *  (*container)[pos].  Note that the RC::RevPtr for container should be set
     *  to AutoRevoke when the container is deleted.
     * @param container A RC::RevPtr to the container.
     *  @param pos The starting index for this iterator.
     */
    inline ValIter(RC::RevPtr<Cont> container, size_t pos) 
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
    /// Directly access the RC::RevPtr to the container.
    inline RC::RevPtr<Cont> Raw() { return c; }


    /// Dereference the iterator, returning a reference to the corresponding
    /// element.
    inline T operator* () { return (*c)[pos]; }
    /// Const version of operator*
    inline const T operator* () const { return (*c)[pos]; }
    /// Provides access to members of the element this iterator points to.
    inline const T* operator-> () const { return &(*c)[pos]; }

    /// Provides access to the element at offset x from the current index.
    inline T operator[] (size_t x) { return (*c)[pos+x]; }
    /// Const version of operator[]
    inline const T operator[] (size_t x) const { return (*c)[pos+x]; }

    /// Point to the next higher numbered indexed element.
    inline ValIter<Cont,T>& operator++ () { pos++; return (*this); }
    /// Point to the next lower numbered indexed element.
    inline ValIter<Cont,T>& operator-- () { pos--; return (*this); }
    /// Increase the index by x.
    inline ValIter<Cont,T>& operator+= (size_t x) { pos += x; return (*this); }
    /// Decrease the index by x.
    inline ValIter<Cont,T>& operator-= (size_t x) { pos -= x; return (*this); }

    /// Support for iter++, but for efficiency, use ++iter.
    inline ValIter<Cont,T> operator++ (int) {
      ValIter<Cont,T> tmp = *this;
      ++*this;
      return tmp;
    }

    /// Support for iter--, but for efficiency, use --iter.
    inline ValIter<Cont,T> operator-- (int) {
      ValIter<Cont,T> tmp = *this;
      --*this;
      return tmp;
    }

    /// Returns a new iterator with an offset incremented by x.
    inline ValIter<Cont,T> operator+ (size_t x) const {
      ValIter<Cont,T> newiter(*this);
      newiter += x;
      return newiter;
    }

    /// Returns a new iterator with an offset decremented by x.
    inline ValIter<Cont,T> operator- (size_t x) const {
      ValIter<Cont,T> newiter(*this);
      newiter -= x;
      return newiter;
    }

    /// Returns the distance between two iterators.
    inline size_t operator- (const ValIter<Cont,T>& first) const {
      return (GetIndex() - first.GetIndex());
    }


    /// True if the two iterators have the same index.
    inline bool operator== (const ValIter<Cont,T>& other) const {
      return (GetIndex() == other.GetIndex());
    }

    /// True if this iterator is less.
    inline bool operator< (const ValIter<Cont,T>& other) const {
      return (GetIndex() < other.GetIndex());
    }

    RC_DEFAULT_COMPARISON();

    private:

    size_t pos;
    RC::RevPtr<Cont> c;
  };


  /// Returns a new iterator with an offset incremented by x.
  template <class Cont, class T>
  inline ValIter<Cont,T> operator+ (size_t x, const ValIter<Cont,T>& iter) {
    ValIter<Cont,T> newiter = iter;
    newiter += x;
    return newiter;
  }

}

#endif // VALITER_H

