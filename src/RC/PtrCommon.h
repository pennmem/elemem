/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2015, Ryan A. Colyer
//
/// \file PtrCommon.h
/// Common components for the *Ptr.h files.  Do not include this directly.
/////////////////////////////////////////////////////////////////////

    public:

    /// Throws RC::ErrorMsgNull if the pointer is null.
    inline void Assert() const {
      if ( IsNull() ) {
        Throw_RC_Type(Null, "Null pointer");
      }
    }

    /// True if the pointer is non-NULL.
    /** @return True if the pointer is non-NULL.
     */
    inline bool IsSet() const {
      return (Raw() != NULL);
    }


    /// True if the pointer is NULL.
    /** @return True if the pointer is NULL.
     */
    inline bool IsNull() const {
      return (Raw() == NULL);
    }


    /// Dereferences the pointer, or throws an exception if null.
    /** The exception thrown is RC::ErrorMsgNull.
     *  @return A reference to the dereferenced object.
     */
    inline T& operator* () { Assert(); return (*Raw()); }
    /// Const version of operator*()
    inline const T& operator* () const { Assert(); return (*Raw()); }
    /// Provides access to the enclosed pointer, or throws RC::ErrorMsgNull
    /// if null.
    /** @return The enclosed pointer.
     */
    inline T* operator-> () { Assert(); return Raw(); }
    /// Const version of operator->()
    inline const T* operator-> () const { Assert(); return Raw(); }

    /// Implicitly casts to the enclosed pointer, without null checking.
    inline operator T* () const { return Raw(); }


    /// Dynamically casts to an RC::Ptr of the type used as a template
    /// parameter on this function.
    /** The Ptr is NULL if the cast fails. */
    template<class Type>
    inline Ptr<Type> Cast() { return dynamic_cast<Type*>(Raw()); }
    /// Const version of Cast()
    template<class Type>
    inline const Ptr<Type> Cast() const { return dynamic_cast<Type*>(Raw()); }

    /// True if it can dynamically cast to this type.
    template<class Type>
    inline bool CanCast() const {
      return Cast<const Type>().IsSet();
    }


    protected:
    /// @cond PROTECTED
    template<class Type, class Self>
    inline static Type& As_Helper(Self& self) {
      self.Assert();
      Type* retptr = dynamic_cast<Type*>(self.Raw());
      if (retptr == NULL) {
        Throw_RC_Type(Cast, "Bad pointer cast");
      }
      return *retptr;
    }
    /// @endcond
    public:

    /// Dynamically casts and dereferences to the type presented as a
    /// template parameter, or throws ErrorMsgCast if it fails.
    template<class Type>
    inline Type& As() { return As_Helper<Type>(*this); }
    /// Const version of As()
    template<class Type>
    inline const Type& As() const { return As_Helper<const Type>(*this); }

    /// Dereference as the default type.
    /** @see operator() */
    inline T& As() { Assert(); return (*Raw()); }
    /// Const version of operator*()
    inline const T& As() const { Assert(); return (*Raw()); }

