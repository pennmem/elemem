/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Caller.h
/// Provides a set of generalized functors for calling functions and
/// methods.
/////////////////////////////////////////////////////////////////////


#ifndef RC_CALLER_H
#define RC_CALLER_H

#include "RCconfig.h"

#ifdef CPP11

#include "APtr.h"
#include "RevPtr.h"
#include <functional>

namespace RC {
  /// @cond PROTECTED

  template<class Ret=void, class... Params>
  class Caller;


  template<class Ret, class... Params>
  class CallerBase {
    public:
    virtual ~CallerBase() { }
    virtual CallerBase* Copy() const = 0;
    virtual Ret operator()(Params... params) const = 0;
    Caller<Ret, Params...> ToCaller() {
      return Caller<Ret, Params...>(*this);
    }
  };
  /// @endcond


  /// The base class of Caller without return type or parameters specified.
  /** For managed casting to the correct Caller type use DynCaller */
  class UntypedCaller {
    public:
    virtual ~UntypedCaller() { }
  };


  /// @cond PROTECTED
  template<class C, class Ret=void, class... Params>
  class MemberCaller;

  template<class Functor, class Ret=void, class... Params>
  class FunctorCaller;

  template<class Ret=void, class... Params>
  class StaticCaller;
  /// @endcond


  /// A general purpose function class which can refer to any static method,
  /// member method, functor, or lambda function.
  /** Template parameters are specified as the return type followed by a list
   *  of the argument types.
   *  If access is attempted on an undefined Caller, ErrorMsgNull is thrown.
   *  If a member method Caller is constructed with the object as the first
   *  parameter, then the call is assigned to that object, which can be used
   *  for passing handlers.
   *  The convenience functions MakeCaller and MakeFunctor can be used
   *  for automatic type inference.
   *  Strict compile time type-checking is performed upon assignment or
   *  construction.  To explicitly wrap with loose type-checking, use
   *  MakeFunctor on a Caller.
   */
  template<class Ret, class... Params>
  class Caller : public UntypedCaller {
    protected:
    /// @cond PROTECTED
    RC::APtr<CallerBase<Ret, Params...>> ptr;
    /// @endcond
    public:
    /// Default constructor, refers to no function.
    Caller() { }
    /// Copy constructor.
    Caller(const Caller& other) {
      ptr = other.ptr;
    }

    /// @cond PROTECTED
    // This template exists to override Functor
    template<class Ret2, class... Params2>
    Caller(const Caller<Ret2, Params2...>& other) {
      static_assert(std::is_same<Caller<Ret, Params...>,
                                 Caller<Ret2, Params2...>>::value,
          "Attempted to copy-construct Callers with mismatched parameters");
      ptr = other.ptr;  // This should fail, giving compile error!
    }
    /// @endcond

    /// Construct a Caller to a member function of a specific object.
    template<class C>
    Caller(C* object, Ret (C::*func)(Params...)) {
      *this = MemberCaller<C, Ret, Params...>(object, func);
    }

    /// Construct a Caller to a member function of a specific object.
    template<class C>
    Caller(C& object, Ret (C::*func)(Params...)) {
      *this = MemberCaller<C, Ret, Params...>(&object, func);
    }
    
    /// Construct a Caller to a member function, where the inserted first
    /// parameter is a reference to the object.
    /** Usage example:  class A { void F(int x) {} };
     *  Caller<void, A&, int> c(&A::F);  c(5);
     */
    template<class C, class... OneFewerParams>
    Caller(Ret (C::*func)(OneFewerParams...)) {
      *this = Caller<Ret, C&, OneFewerParams...>(std::mem_fn(func));
    }
    
    /// Construct a Caller to any general functor.
    /** Note, for a Caller functor of a slightly different type, use
     *  MakeFunctor to avoid the intentional type-checking compile error.
     */
    template<class Functor>
    Caller(Functor func) {
      *this = FunctorCaller<Functor, Ret, Params...>(func);
    }

    /// Destructor.
    virtual ~Caller() { }

    /// @cond PROTECTED
    Caller(const CallerBase<Ret, Params...>& caller_base) {
      ptr = caller_base.Copy();
    }

    Caller& operator=(const CallerBase<Ret, Params...>& caller_base) {
      ptr = caller_base.Copy();
      return *this;
    }
    /// @endcond

    /// Assign a Caller of identical type.
    Caller& operator=(const Caller<Ret, Params...>& other) {
      ptr = other.ptr;
      return *this;
    }

    /// Call the referenced function.
    virtual Ret operator()(Params... params) const { return (*ptr)(params...); }

    /// True if a function was set.
    bool IsSet() { return ptr.IsSet(); }

    /// Call the referenced function with the parameters given as RC::Tuple
    /// tup.
    template<class TupleType>
    inline Ret Use(TupleType tup) { return tup.Apply(*this); }

    /// Return a functor with arguments bound using syntax identical to
    /// std::bind.
    /** The return type of this is an unspecified functor, but it can
     *  be wrapped in a MakeFunctor with the corresponding types.
     */
    template<class... Args>
    auto Bind(Args... args) -> decltype(std::bind(*this, args...)) {
      return std::bind(*this, args...);
    }
  };


  /// @cond PROTECTED
  template<class C, class Ret, class... Params>
  class MemberCaller : public CallerBase<Ret, Params...> {
    public:
    MemberCaller(C* obj, Ret (C::*func)(Params...)) : obj(obj), func(func) { }
    virtual Ret operator()(Params... params) const {
      return (obj->*func)(params...);
    }
    virtual CallerBase<Ret, Params...>* Copy() const {
      return new MemberCaller<C, Ret, Params...>(obj, func);
    }

    protected:
    C* obj;
    Ret (C::*func)(Params...);
  };


  template<class Functor, class Ret, class... Params>
  class FunctorCaller : public CallerBase<Ret, Params...> {
    public:
    FunctorCaller(Functor func) : func(func) { }

    virtual Ret operator()(Params... params) const { return func(params...); }
    virtual CallerBase<Ret, Params...>* Copy() const {
      return new FunctorCaller<Functor, Ret, Params...>(func);
    }

    protected:
    Functor func;
  };


  // This either obtains a StaticCaller, or generates errors during
  // implicit casting of static function pointers to a FunctorCaller
  // with parameters of an implicitly castable type.
  template<class Ret, class... Params, class Ret2, class... Params2>
  class FunctorCaller<Ret (*)(Params...), Ret2, Params2...>
    : public StaticCaller<Ret2, Params2...> {
    public:
    FunctorCaller(Ret (*func)(Params...))
      : StaticCaller<Ret2, Params2...>(func) { }
  };


  template<class Ret, class... Params>
  class StaticCaller : public CallerBase<Ret, Params...> {
    public:
    StaticCaller(Ret (*func)(Params...)) : func(func) { }
    virtual Ret operator()(Params... params) const {
      return (*func)(params...);
    }
    virtual CallerBase<Ret, Params...>* Copy() const {
      return new StaticCaller<Ret, Params...>(func);
    }

    protected:
    Ret (*func)(Params...);
  };
  /// @endcond


  /// Generates a Caller for the member function of the given object, with
  /// type inference.
  template<class C, class Ret, class... Params>
  Caller<Ret, Params...> MakeCaller(C* obj, Ret (C::*func)(Params...)) {
    return MemberCaller<C, Ret, Params...>(obj, func);
  }

  /// Generates a Caller for the member function of the given object, with
  /// type inference.
  template<class C, class Ret, class... Params>
  Caller<Ret, Params...> MakeCaller(C& obj, Ret (C::*func)(Params...)) {
    return MemberCaller<C, Ret, Params...>(&obj, func);
  }

  /// Generates a Caller for the member function with type inference,
  /// inserting a first parameter with a reference to the object.
  template<class C, class Ret, class... Params>
  Caller<Ret, C&, Params...> MakeCaller(Ret (C::*func)(Params...)) {
    //return FunctorCaller<Ret (C::*)(Params...), Ret, C&, Params...>(std::mem_fn(func));
    return Caller<Ret, C&, Params...>(std::mem_fn(func));
  }

  /// Generates a Caller for the specified static function, with
  /// type inference.
  template<class Ret, class... Params>
  Caller<Ret, Params...> MakeCaller(Ret (*func)(Params...)) {
    return StaticCaller<Ret, Params...>(func);
  }


  /// A special generator for functors, which requires specifying the return
  /// type and arguments.
  /** Template parameters are specified as the return type followed by a list
   *  of the arguments.
   */
  template<class Ret, class... Params, class Functor>
  Caller<Ret, Params...> MakeFunctor(Functor func) {
    return FunctorCaller<Functor, Ret, Params...>(func);
  }


  /// Generates a Caller for the member function of the given object, with
  /// type inference.
  template<class C, class MemberFunc>
  auto MakeCaller(Ptr<C> obj, MemberFunc func)
    -> decltype(MakeCaller(obj.Raw(), func)) {

    return MakeCaller(obj.Raw(), func);
  }

  /// Generates a Caller for the member function of the given object, with
  /// type inference.
  template<class C, class MemberFunc>
  auto MakeCaller(APtr<C> obj, MemberFunc func)
    -> decltype(MakeCaller(obj.Raw(), func)) {

    return MakeCaller(obj.Raw(), func);
  }

  /// Generates a Caller for the member function of the given object, with
  /// type inference.
  template<class C, class MemberFunc>
  auto MakeCaller(RevPtr<C> obj, MemberFunc func)
    -> decltype(MakeCaller(obj.Raw(), func)) {

    return MakeCaller(obj.Raw(), func);
  }


  /// A typeless container for a Caller, which has methods for dynamically
  /// casting it to the correct type.
  class DynCaller {
    protected:
    /// @cond PROTECTED
    APtr<UntypedCaller> caller;
    /// @endcond
    public:
    /// Default constructor, contains no Caller.
    DynCaller() {}

    /// Construct a DynCaller which wraps new_caller.
    template<class... Types>
    DynCaller(Caller<Types...> new_caller)
      : caller(new Caller<Types...>(new_caller)) {
    }

    /// Creates a Caller referring to the given static function, and then
    /// wraps it.
    template<class Ret, class... Params>
    DynCaller(Ret (*func)(Params...)) {
      caller = new decltype(MakeCaller(func))(func);
    }


    /// Dynamically casts to the Caller of the specified template parameters,
    /// or throws ErrorMsgCast if it fails.
    template<class... Types>
    inline Caller<Types...>& As() {
      return caller.As<Caller<Types...>>();
    }
    /// Const version of As().
    template<class... Types>
    inline const Caller<Types...>& As() const {
      return caller.As<Caller<Types...>>();
    }

    /// True if this can cast to a Caller with the specified template
    /// parameters.
    template<class... Types>
    inline bool CanCast() const {
      return caller.CanCast<Caller<Types...>>();
    }
  };
}

#endif // CPP11

#endif // RC_CALLER_H


