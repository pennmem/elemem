/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file APtr.h
/// A reference counting ptr that is auto-deleted as the last copy
/// leaves scope.
/////////////////////////////////////////////////////////////////////

#ifndef RC_APTR_H
#define RC_APTR_H

#include "RCconfig.h"
#include "Types.h"
#include "Macros.h"
#include "Errors.h"
#include <iostream>

#ifdef CPP11
#include <atomic>
#endif


namespace RC {
  /// @cond EXTERNAL
  template <class T> class Ptr;
  /// @endcond

  /// A reference counting ptr that auto-deletes what it points to when the
  /// last copy leaves scope or is otherwise destructed.
  /** Use this for scope-controlled garbage collection.  Like Ptr,
   *  dereferencing a null APtr throws ErrorMsgNull.  For C++11 the reference
   *  counting is thread safe.
   *  @see Ptr
   *  @see RevPtr
   */
  template <class T>
  class APtr {
    public:

    /// Default constructor assigning the value of the pointer.
    /** @param t_ptr The new pointer value.
     */
    inline APtr(T* t_ptr = NULL) {
      helper = new PtrHelper(t_ptr, true);
    }

    /// Copy constructor.
    /** @param other The APtr to copy.
     */
    inline APtr(const APtr<T>& other) {
      other.helper->Add();
      helper = other.helper;
    }

    /// A conversion constructor which creates a new APtr from a Ptr of the
    /// same or a derived type.
    /** @param other The Ptr from which the pointer should be used to make
     *  this APtr.
     */
    template <class Tderived>
    inline APtr(const Ptr<Tderived>& other) {
      helper = new PtrHelper(other, true);
    }

    /// An assignment operator, which increases the shared reference count with
    /// the source APtr.
    /** @param other The source APtr to assign here.
     */
    inline APtr& operator= (const APtr<T> &other) {
      other.helper->Add();

      PtrHelper* loc_helper = helper;
      helper = other.helper;
      loc_helper->Del();
      
      return (*this);
    }


    /// Destructor which decreases the shared reference count.
    /** The object is deleted if the shared reference count reaches 0.
     */
    inline ~APtr() {
      helper->Del();
    }


    /// Delete the object pointed to and set the shared pointer for all linked
    /// APtr's to NULL.
    /** Note:  While this NULLs all the linked APtr's, it cannot affect
     *  raw pointers which have already been extracted or are in use when
     *  this is called.  Be mindful of this in threading architecture design.
     */
    inline void Delete() {
      helper->Delete();
    }


    /// Extract APtr<const T> and revokes ownership from this object.
    inline APtr<const T> ExtractConst() {
      APtr<const T> retptr(helper->t_ptr);
      helper->Revoke();
      return retptr;
    }

#include "PtrSharedCommon.h"
#include "PtrCommon.h"


    protected:
    /// @cond PROTECTED

    PtrHelper* helper;
    /// @endcond
  };


  RC_STREAM_RAWWRAP(APtr);


  /// Create a new APtr<T> constructed with the given arguments.
  template<class T, class... Args>
  APtr<T> MakeAPtr(Args&&... args) {
    return APtr<T>(new T(std::forward<Args>(args)...));
  }
}


#endif // RC_APTR_H

