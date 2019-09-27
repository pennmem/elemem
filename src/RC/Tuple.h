/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2015, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Tuple.h
/// Provides a Tuple class which can apply its contents as function
/// parameters.
/////////////////////////////////////////////////////////////////////

#ifndef RC_TUPLE_H
#define RC_TUPLE_H

#ifdef CPP11
  
#include "RCconfig.h"
#include "Types.h"
#include "Data1D.h"

#include <tuple>

namespace RC {
  /// @cond PROTECTED

  // Template tool for generating the sequence S... of 0 to one less
  // than the value passed in as the first parameter of SequenceGenerator

  template<size_t...> struct TemplateSequence { };
  template<size_t N, size_t... S> struct SequenceGenerator
   : SequenceGenerator<N-1, N-1, S...> { };
  template<size_t... S> struct SequenceGenerator<0, S...> {
    typedef TemplateSequence<S...> Sequence;
  };

  template<size_t N, class... Types>
  class TupleBucket { };

  template<size_t N, class Type1, class... Types>
  class TupleBucket<N, Type1, Types...>
    : public TupleBucket<N+1, Types...> {

    public:
    TupleBucket() { }

    TupleBucket(Type1& type1, Types&... types)
      : TupleBucket<N+1, Types...>(types...)
      , type1(type1) {
    }

    inline Type1& Get() { return type1; }
    inline const Type1& Get() const { return type1; }

    protected:
    Type1 type1;
  };

  /// @endcond

  /// An efficient Tuple class with Set, Get, and an Apply function
  /// to pass the tuple contents on to any function.
  template<class... Types>
  class Tuple : public TupleBucket<0, Types...> {
    public:
    /// Default constructor, elements use default constructors.
    Tuple() { }

    /// Construct by const reference.
    template<int ResolveAmbiguity=0>
    Tuple(const typename std::add_lvalue_reference<
                typename std::remove_reference<Types>::type>::type... types)
      : TupleBucket<0, Types...>(types...) { }

    /// Construct by moving elements in.
    template<bool ResolveAmbiguity=0>
    Tuple(typename std::add_rvalue_reference<
          typename std::remove_reference<Types>::type>::type... types)
      : TupleBucket<0, Types...>(types...) { }
    
    /// Returns a std::tuple of the same contents.
    std::tuple<Types...> GetStdTuple() {
      return Apply(std::tie<Types...>);
    }

    protected:
    /// @cond PROTECTED
    template<size_t N, class Type1, class... GTypes>
    inline Type1& GetHelper(TupleBucket<N, Type1, GTypes...>& tup) {
      return tup.Get();
    }
    template<size_t N, class Type1, class... GTypes>
    inline const Type1& GetHelper(
        const TupleBucket<N, Type1, GTypes...>& tup) const {
      return tup.Get();
    }
    /// @endcond

    public:
    /// Return element N in the Tuple.
    template<size_t N>
    inline auto Get() -> decltype(this->GetHelper<N>(*this)) {
      return GetHelper<N>(*this);
    }
    /// Return element N in the Tuple.
    template<size_t N>
    inline auto Get() const -> decltype(this->GetHelper<N>(*this)) {
      return GetHelper<N>(*this);
    }

    /// Set element N in the Tuple.
    template<size_t N, class Val>
    inline void Set(Val val) {
      GetHelper<N>(*this) = val;
    }

    protected:
    /// @cond PROTECTED
    template<class Func, size_t... Indices>
    inline auto ApplyHelper(Func f, TemplateSequence<Indices...>)
      -> decltype(f(this->Get<Indices>()...)) {
      return f(Get<Indices>()...);
    }

    template<class T, size_t... Indices>
    inline Data1D<T> AsDataHelper(TemplateSequence<Indices...>) const {
      return Data1D<T>{T(Get<Indices>())...};
    }
    /// @endcond

    public:
    /// Call function f with each element of the Tuple as an argument.
    /** @return The return value of function f for the given arguments.
     */
    template<class Func>
    inline auto Apply(Func f)
      -> decltype(this->ApplyHelper(f,
                  (typename SequenceGenerator<sizeof...(Types)>::Sequence()))) {
      return ApplyHelper(f,
          (typename SequenceGenerator<sizeof...(Types)>::Sequence()));
    }

    /// Return a Data1D array with each element constructed as type T.
    template<class T>
    inline Data1D<T> AsData() const {
      return AsDataHelper<T>(
          (typename SequenceGenerator<sizeof...(Types)>::Sequence()));
    }

    /// Get all elements at once, by reference.
    inline void Get(Types&... types) {
      std::tie(types...) = GetStdTuple();
    }

    protected:
    /// @cond PROTECTED
    template<class... OtherTypes, size_t... Indices, size_t... OtherIndices>
    inline Tuple<Types..., OtherTypes...> ConcatHelper
         (Tuple<OtherTypes...>& other, TemplateSequence<Indices...>,
          TemplateSequence<OtherIndices...>) {
      return Tuple<Types..., OtherTypes...>(Get<Indices>()...,
             other.template Get<OtherIndices>()...);
    }
    /// @endcond

    public:
    /// Forms a new Tuple by concatenating two Tuples.
    template<class... OtherTypes>
    inline Tuple<Types..., OtherTypes...> operator+
         (Tuple<OtherTypes...>& other) {
      return ConcatHelper(other,
          typename SequenceGenerator<sizeof...(Types)>::Sequence(),
          typename SequenceGenerator<sizeof...(OtherTypes)>::Sequence());
    }
  };


  /// A convenience generator to make a Tuple from the given elements with
  /// type inference.
  template<class... Types>
  Tuple<Types...> MakeTuple(Types... types) {
    return Tuple<Types...>(types...);
  }

}

#endif // CPP11

#endif // RC_TUPLE_H

