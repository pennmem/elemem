/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Ptr.h
/// Provides a safe pointer class which throws exceptions.
/////////////////////////////////////////////////////////////////////

#ifndef RC_PTR_H
#define RC_PTR_H


#include "Errors.h"
#include "Macros.h"
#include <iostream>


namespace RC {
  /// @cond EXTERNAL
  template <class T> class APtr;
  template <class T> class RevPtr;
  /// @endcond

  /// A safe pointer class that throws an RC::ErrorMsgNull if a null
  /// dereference is attempted.
  /** @see APtr
   *  @see RevPtr
   */
  template <class T>
  class Ptr {
    public:

    /// Default constructor assigning the value of the pointer.
    /** @param t_ptr The new pointer value.
     */
    inline Ptr(T* t_ptr = NULL)
      : t_ptr (t_ptr) {
    }

    /// A constructor which obtains a non-reference-counted copy of an APtr.
    /** @param other The APtr from which a pointer should be extracted.
     */
    template <class Tderived>
    inline Ptr(const APtr<Tderived>& other) {
      t_ptr = other;
    }

    /// A constructor which obtains a non-revokable copy of a RevPtr.
    /** @param other The RevPtr from which a pointer should be extracted.
     */
    template <class Tderived>
    inline Ptr(const RevPtr<Tderived>& other) {
      t_ptr = other;
    }

    /// Copy constructor.
    /** @param other The Ptr to copy.
     */
    template <class Tderived>
    inline Ptr(const Ptr<Tderived>& other) {
      t_ptr = other;
    }


    /// Deletes the object being pointed to and nulls the pointer.
    inline void Delete() {
      if (t_ptr != NULL) {
        delete(t_ptr);
        t_ptr = NULL;
      }
    }

    /// Returns a direct reference to the enclosed pointer.
    /** The reference is read/write, so it can be used as a function
     *  parameter which updates the value of this pointer.
     *  @return A reference to the enclosed pointer.
     */
    inline T* Raw() const { return t_ptr; }


#include "PtrCommon.h"


    protected:
    /// @cond PROTECTED

    T *t_ptr;
    /// @endcond
  };


  RC_STREAM_RAWWRAP(Ptr);

  /// Create a new Ptr<T> constructed with the given arguments.
  template<class T, class... Args>
  Ptr<T> MakePtr(Args&&... args) {
    return Ptr<T>(new T(std::forward<Args>(args)...));
  }

}


#endif // RC_PTR_H

