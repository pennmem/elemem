/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file RevPtr.h
/// A reference counting pointer that revokes (NULLs) all copies when
/// one set to AutoRevoke(true) leaves scope.
/////////////////////////////////////////////////////////////////////

#ifndef RC_REVPTR_H
#define RC_REVPTR_H


#include "Types.h"
#include "Errors.h"
#include "Macros.h"
#include <iostream>

#ifdef CPP11
#include <atomic>
#endif


namespace RC {
  /// @cond EXTERNAL
  template <class T> class Ptr;
  template <class T> class APtr;
  /// @endcond

  /// A reference counting pointer that revokes (NULLs) all copies when
  /// one set to AutoRevoke(true) leaves scope.
  /** This pointer class does not automatically delete the object pointed to,
   *  so that it can be used for self-referential purposes.  For automatic
   *  deletion, use an APtr, and create a RevPtr family that refers to the
   *  Ptr of the same scope.  Like Ptr and APtr, dereferencing a null
   *  RevPtr throws ErrorMsgNull.
   *  Note:  Revocation is thread safe, but a previously extracted raw ptr
   *  could remain in use after revocation.  Be mindful of this in threading
   *  architecture design.
   *  @see Ptr
   *  @see APtr
   */
  template <class T>
  class RevPtr {
    public:

    /// Default constructor assigning the value of the pointer.
    /** @param t_ptr The new pointer value.
     */
    RevPtr(T* t_ptr = NULL) {
      helper = new PtrHelper(t_ptr, false);
      auto_revoke = false;
    }

    /// Copy constructor.
    /** @param other The RevPtr to copy.
     */
    inline RevPtr(const RevPtr<T>& other) {
      other.helper->Add();
      helper = other.helper;
      auto_revoke = false;
    }

    /// A conversion constructor which creates a new RevPtr from a Ptr of the
    /// same or a derived type.
    /** @param other The Ptr from which the pointer should be used to make
     *  this RevPtr
     */
    inline RevPtr(const Ptr<T>& other) {
      helper = new PtrHelper(other, false);
      auto_revoke = false;
    }

    /// A conversion constructor which creates a new RevPtr from an APtr of
    /// the same or a derived type.
    /** @param other The APtr from which the pointer should be used to make
     *  this RevPtr
     */
    inline RevPtr(const APtr<T>& other) {
      helper = new PtrHelper(other, false);
      auto_revoke = false;
    }


    /// An assignment operator which joins this RevPtr to the other RevPtr.
    /** @param other The source RevPtr to assign here.
     */
    inline RevPtr& operator= (const RevPtr<T> &other) {
      helper->Del();

      other.helper->Add();
      helper = other.helper;
      auto_revoke = false;

      return *this;
    }


    /// Destructor which revokes the pointer only if AutoRevoke() was used
    /// on this object.
    ~RevPtr() {
      if (auto_revoke) {
        helper->Revoke();
      }

      helper->Del();
    }


    /// Manually delete the object pointed to by this RevPtr, and revoke
    /// the pointer for all shared RevPtr's.
    /** Note:  While this NULLs all the linked RevPtr's, it cannot affect
     *  raw pointers which have already been extracted or are in use when
     *  this is called.  Be mindful of this in threading architecture design.
     */
    inline void Delete() {
      helper->Delete();
    }


    /// Manually revoke the pointer, setting it equal to NULL for all shared
    /// RevPtr objects.
    /** This does not delete the pointer.
     */
    inline void Revoke() {
      helper->Revoke();
    }


    /// Set this RevPtr to revoke the pointer upon deletion or leaving scope.
    /** @param new_auto_revoke True if this RevPtr should automatically
     *  revoke.
     *  The recommended way to use this feature is to place the primary RevPtr
     *  in scope with the object to which it points so that they are deleted
     *  together, and then call AutoRevoke() on that RevPtr.  Then all other
     *  RevPtr's which have been assigned or copy constructed from the primary
     *  RevPtr will be automatically revoked when it is destructed, and they
     *  will all return true for IsNull or throw exceptions if dereferencing
     *  is attempted.
     */
    inline void AutoRevoke(bool new_auto_revoke=true) {
      auto_revoke = new_auto_revoke;
    }


#include "PtrSharedCommon.h"
#include "PtrCommon.h"


    protected:
    /// @cond PROTECTED

    PtrHelper *helper;
    bool auto_revoke;
    /// @endcond
  };


  RC_STREAM_RAWWRAP(RevPtr);

  /// Create a new RevPtr<T> constructed with the given arguments.
  template<class T, class... Args>
  RevPtr<T> MakeRevPtr(Args&&... args) {
    return RevPtr<T>(new T(std::forward<Args>(args)...));
  }
}


#endif // RC_REVPTR_H

