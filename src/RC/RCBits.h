/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2019, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file RCBits.h
/// Provides short convenience classes and functions.
/////////////////////////////////////////////////////////////////////

#ifndef RC_RCBITS_H
#define RC_RCBITS_H

#include "RCconfig.h"
#include "Errors.h"
#ifdef CPP11
#include "Tuple.h"
#endif
#include "Types.h"
#include <iostream>
#include <sstream>
#include <string>

namespace RC {

/// @cond UNDOC
template<class T>
class BetwCompare {
  protected:
  const T& x;
  bool ans;
  public:
  inline explicit BetwCompare(const T& x) : x(x), ans(true) { }
#define RC_BETW_HELP(REL) \
  template<class T2> \
  inline BetwCompare operator REL (const T2& other) { \
    ans &= x REL other; \
    return *this; \
  }
  RC_BETW_HELP(<)
  RC_BETW_HELP(>)
  RC_BETW_HELP(<=)
  RC_BETW_HELP(>=)
  RC_BETW_HELP(==)
  RC_BETW_HELP(!=)
  inline operator bool () const { return ans; }
};

#define RC_BETW_HELP2(REL1, REL2) \
template<class T, class T2> \
inline BetwCompare<T> operator REL1 (const T2& other, BetwCompare<T> betw) { \
  return betw REL2 other; \
}

RC_BETW_HELP2(<, >)
RC_BETW_HELP2(>, <)
RC_BETW_HELP2(<=, >=)
RC_BETW_HELP2(>=, <=)
RC_BETW_HELP2(==, ==)
RC_BETW_HELP2(!=, !=)

/// @endcond

/// Returns a comparator that can be used for range comparisons.
/** Usage exmple:  if (3.14 < Betw(5) <= 7) { cout << "Yes\n"; }
 *  Works for any comparable types, as long as one of the left-most two in
 *  the chain is wrapped in Betw.
 */
template<class T>
BetwCompare<T> Betw(const T& x) { return BetwCompare<T>(x); }

#ifdef CPP11

/// @cond UNDOC
template<class... Args>
class OneOfCompare {
  protected:
  Tuple<Args...> compare;
  public:
  OneOfCompare(Tuple<Args...>&& tup) : compare(tup) { }
  template<class T, class... Args2>
  friend bool operator==(const T& item, const OneOfCompare<Args2...>& list);
  template<class T, class... Args2>
  friend bool operator!=(const T& item, const OneOfCompare<Args2...>& list);
};

template<class T, class... Args>
bool operator== (const T& item, const OneOfCompare<Args...>& list) {
  Data1D<T> arr = list.compare.template AsData<T>();
  for (T& elem : arr) {
    if (item == elem) {
      return true;
    }
  }
  return false;
}
template<class T, class... Args>
bool operator!= (const T& item, const OneOfCompare<Args...>& list) {
  return ! (item == list);
}
/// @endcond

/// Returns a comparator that can be used to check if something is in a list.
/** Each element given as a parameter to OneOf is constructed with the type
 *  of the object left of the == or != operator.  Usage exmple:
 *  RStr str = "foo";  if (str != OneOf("bar", -3.54, "blah")) {
 *  cout << "Not Found\n"; }  int x = 8; if (5 == OneOf(3.14, 5, x)) {
 *  cout << "Found\n"; }
 */
template<class... Args>
OneOfCompare<Args...> OneOf(Args... args) {
  return OneOfCompare<Args...>(Tuple<Args...>(args...));
}
#endif


/// A size_t like integer class which automatically stays within its range.
/** Use this for circular buffers or anywhere modulo arithmetic is needed.
 */
class LoopIndex {
  public:
  /// Defines the range of of the object.  The index is always less than this.
  inline LoopIndex(size_t range) : index(0), range(range) { }
  /// Returns the set range.
  inline size_t Range() const { return range; }
  /// Sets a new range.
  inline void SetRange(size_t new_range) { range = new_range; *this = index; }
  /// Assigns a new index value, forcing it in the range.  Signed indices are
  /// handled properly.
  inline LoopIndex& operator= (size_t new_index) {
    if (new_index > (size_t(-1)/2)) {
      index = range - ((0-new_index) % range);
    }
    else {
      index = new_index % range;
    }
    return *this;
  }
  /// Implicitly returns the size_t index.
  inline operator size_t () const { return index; }
  /// Increment by one, looping within range.
  inline LoopIndex& operator++ () {
    index++;
    if (index>=range) { index=0; }
    return *this;
  }
  /// Postfix increment by one, looping within range.
  inline LoopIndex operator++ (int) {
    LoopIndex tmp(*this);
    ++(*this);
    return tmp;
  }
  /// Decrement by one, looping within range.
  inline LoopIndex& operator-- () {
    if (index==0) { index = range-1; }
    else { index--; }
    return *this;
  }
  /// Postfix decrement by one, looping within range.
  inline LoopIndex operator-- (int) {
    LoopIndex tmp(*this);
    --(*this);
    return tmp;
  }
  /// Increment by offset, looping within range, and handling negative offsets
  /// correctly.
  inline LoopIndex& operator+= (size_t offset) {
    if (offset > (size_t(-1)/2)) {
      size_t negoff = (0-offset)%range;
      if (index < negoff) {
        index = range - (negoff - index);
      }
      else {
        index = index - negoff;
      }
    }
    else {
      index = (index + offset) % range;
    }
    return *this;
  }
  /// Decrement by offset, looping within range, and handling negative offsets
  /// correctly.
  inline LoopIndex& operator-= (size_t offset) {
    return (*this += 0-offset);
  }

  protected:
  /// @cond PROTECTED
  size_t index;
  size_t range;
  /// @endcond
};


/// @cond UNDOC

template<class T>
class DerefIncrement {
  protected:
  T val;
  public:
  inline DerefIncrement(const T& val) : val(val) { }
  inline T operator* () const { return val; }
  inline DerefIncrement& operator++ () { ++val; return *this; }
  inline DerefIncrement& operator-- () { --val; return *this; }
  inline bool operator== (const DerefIncrement& other) const {
    return val == other.val;
  }
  inline bool operator!= (const DerefIncrement& other) const {
    return val != other.val;
  }
};

template<class T>
class RangeHelper {
  protected:
  T start;
  T past_the_end;
  public:
  inline RangeHelper(const T& start, const T& past_the_end) :
    start(start), past_the_end(past_the_end) { }
  inline DerefIncrement<T> begin() const { return start; }
  inline DerefIncrement<T> cbegin() const { return start; }
  inline DerefIncrement<T> end() const { return past_the_end; }
  inline DerefIncrement<T> cend() const { return past_the_end; }
  inline DerefIncrement<T> rbegin() const { return past_the_end-1; }
  inline DerefIncrement<T> crbegin() const { return past_the_end-1; }
  inline DerefIncrement<T> rend() const { return start-1; }
  inline DerefIncrement<T> crend() const { return start-1; }
};
/// @endcond

/// Provides an iterator which dereferences to the iterated values
/// [start,past_the_end).
/** Usage example, outputs 5 through 9:
 *  for (auto i : Range(5, 10)) { cout << i << endl; }
 *  This is a generalization, since:
 *  for (auto iter : Range(cont.begin(), cont.end())) { ... }
 *  is equivalent to the canonical:
 *  for (auto iter : cont) { ... }
 */
template<class T>
inline RangeHelper<T> Range(const T& start, const T& past_the_end) {
  return RangeHelper<T>(start, past_the_end);
}

#ifdef CPP11

/// Provides an iterator which dereferences to the indices of cont from
/// [0,cont.size()).
/** Usage example:  for (auto i : IndexOf(cont)) { cout << cont[i]; }
 */
template<class T2>
inline auto IndexOf(const T2& cont) -> RangeHelper<decltype(cont.size())> {
  return RangeHelper<decltype(cont.size())>(0, cont.size());
}

#endif

/// Stores the value of type Hold while providing access via type Provide.
/** @see RStr::ToLPCWSTR()
 */
template<class Hold, class Provide>
class HoldRelated {
  protected:
  /// @cond PROTECTED
  Hold held;
  Provide give;
  /// @endcond
  public:
#ifdef CPP11
  /// Stores held, but implicitly casts to related value give.
  HoldRelated(Hold held, Provide give)
    : held(std::move(held)), give(std::move(give)) { }
  /// Stores held.  Use with Set to set the give value later.
  HoldRelated(Hold held) : held(std::move(held)) { }
  /// Sets the implicitly cast value of type Provide to give.
  void Set(Provide new_give) { give = std::move(new_give); }
#else
  HoldRelated(Hold held, Provide give) : held(held), give(give) { }
  HoldRelated(Hold held) : held(held) { }
  void Set(Provide new_give) { give = new_give; }
#endif
  /// Implicitly casts to the give value set.
  operator Provide() { return give; }
  /// Implicitly casts to the give value set.
  operator const Provide() const { return give; }
  /// Explicitly access the value set as give.
  const Provide Get() const { return give; }
  /// Explicitly access the value set as give.
  Provide Get() { return give; }
  /// Access the full held value.
  const Hold& Held() const { return held; }
  /// Access the full held value.
  Hold& Held() { return held; }
};

/// Inherit this class to add construction, destruction, and assignment
/// output tracking.
/** Set ClassTracking to the class which is inheriting this.  If
 *  stack_trace is true, a stack trace is output at each event.  See
 *  ErrorMsg for stack trace usage details.
 */
template<class ClassTracking, bool stack_trace=false>
class DebugTrack {
  public:
/// @cond UNDOC
#define RC_DEBUG_TRACK_OUT(label) \
    std::string name = typeid(ClassTracking).name(); \
    std::stringstream sstr; \
    sstr << "[" << this << "] -> "; \
    sstr << label; \
    if (stack_trace) { \
      ErrorMsg e(sstr.str().c_str()); \
      sstr.str(std::string()); \
      sstr << e.what(); \
    } \
    sstr << "\n"; \
    RC_DEBOUT_STREAM << sstr.str();

  DebugTrack() {
    RC_DEBUG_TRACK_OUT(name + "()  [Construct]");
  }
  ~DebugTrack() {
    RC_DEBUG_TRACK_OUT("~" + name + "()  [Destruct]");
  }
  DebugTrack(const DebugTrack&) {
    RC_DEBUG_TRACK_OUT(name + "(const "+name+"&)  [Copy Construct]");
  }
  DebugTrack& operator=(const DebugTrack&) {
    RC_DEBUG_TRACK_OUT(name+"& operator=(const "+name+"&)  [Copy Assignment]");
    return *this;
  }
#ifdef CPP11
  DebugTrack(DebugTrack&&) {
    RC_DEBUG_TRACK_OUT(name + "(" + name + "&&)  [Move Construct]");
  }
  DebugTrack& operator=(DebugTrack&&) {
    RC_DEBUG_TRACK_OUT(name + "& operator=("+name+"&&)  [Move Assignment]");
    return *this;
  }
#endif
/// @endcond
};


/// Opens a dynamic library filename and stores a handle to it.
/// Use as second parameter for RC_DYNAMIC_LOAD_FUNC(FuncName,DynLib)
/** ErrorMsgFile is thrown in the event of a loading error.
 */
class DynamicLibrary {
  public:
#ifdef unix
  /// Direct access to the loaded library.
  void* library;
#elif defined(WIN32)
  HMODULE library;
#endif
  /// Loads the library of the given name.
  template<size_t N>
  DynamicLibrary(const char (&libname)[N]) {
#ifdef unix
    library = dlopen(libname, RTLD_LAZY | RTLD_GLOBAL);
#elif defined(WIN32)
    library = LoadLibrary(libname);
#endif
    if (!library) {
      Throw_RC_Type(File, libname);
    }
  }
};


/// @cond UNDOC
template<class T>
class DynamicFunction {
  T func;
  public:
  template<size_t N>
  DynamicFunction(DynamicLibrary& dyn_lib, const char (&funcname)[N]) {
#ifdef unix
    func = T(dlsym(dyn_lib.library, funcname));
#elif defined(WIN32)
    func = T(GetProcAddress(dyn_lib.library, funcname));
#endif
    if (!func) {
      Throw_RC_Type(File, funcname);
    }
  }

	template<class... Args>
	auto operator()(Args... args) -> decltype((*func)(args...)) {
    return (*func)(args...);
  }
};
/// @endcond


/// \def RC_DYNAMIC_LOAD_FUNC
/// Use as RC_DYNAMIC_LOAD_FUNC(FuncName,DynLib) to load FuncName from the
/// dynamic library file opened by a constructor of the DynamicLibrary class.
/** This creates a static function object named FuncName in-place to provide
 *  access to the loaded library.  As it overrides FuncName, the previous
 *  declaration of FuncName MUST be in an outer scope (outside of the function
 *  where this is called).  For example, include the original definition from a
 *  header, and use this inside of a function or namespace.  After this macro,
 *  call FuncName with the same syntax as you would the  previously declared
 *  FuncName.  This macro uses classes defined for both Linux and Windows.
 *  ErrorMsgFile is thrown in the event of a loading error.
 */
#define RC_DYNAMIC_LOAD_FUNC(FuncName,DynLib) \
  static RC::DynamicFunction<decltype(FuncName)> FuncName{DynLib, #FuncName};


#ifdef CPP11
/// For integer and floating point types, Throws RC::ErrorMsgBounds if
/// the value of x does not fit into T.  Use as CheckedCast<T>(x)
template<class T, class T2>
inline T CheckedCast(const T2 &x) {
  bool out_of_bounds = false;

  if (IsIntegerType<T>()) {
    if (intmax_t(x) < intmax_t(LOW_VAL<T>())) {
      out_of_bounds = true;
    } 
    else if (x > 0 && uintmax_t(x) > uintmax_t(MAX_VAL<T>())) {
      out_of_bounds = true;
    }
  }
  else {
    if (fBIGGEST(x) < fBIGGEST(LOW_VAL<T>())) {
      out_of_bounds = true;
    }
    else if (fBIGGEST(x) > fBIGGEST(MAX_VAL<T>())) {
      out_of_bounds = true;
    }
  }
  if (out_of_bounds) {
    Throw_RC_Type(Bounds, "Cast out of bounds");
  }
  return T(x);
}

/// For integer and floating point types, caps the cast value of x to
/// be within bounds of type T.  Use as CappedCast<T>(x)
template<class T, class T2>
inline T CappedCast(const T2 &x) {
  if (IsIntegerType<T>()) {
    if (intmax_t(x) < intmax_t(LOW_VAL<T>())) {
      return LOW_VAL<T>();
    } 
    if (x > 0 && uintmax_t(x) > uintmax_t(MAX_VAL<T>())) {
      return MAX_VAL<T>();
    }
  }
  else {
    if (fBIGGEST(x) < fBIGGEST(LOW_VAL<T>())) {
      return LOW_VAL<T>();
    }
    if (fBIGGEST(x) > fBIGGEST(MAX_VAL<T>())) {
      return MAX_VAL<T>();
    }
  }
  return T(x);
}
#endif

}

#endif // RC_RCBITS_H

