/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2012-2015, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Macros.h
/// Provides a set of convenience macros for metaprogramming
/// and debugging.
/** Note:  RC_EMPTY, RC_ARGS_NUM, RC_ARGS_EACH, RC_ARGS_BET,
 *  RC_ARGS_LIST, and RC_DEBOUT require each of their parameters to
 *  begin with a letter, number, or underscore.
 */
/////////////////////////////////////////////////////////////////////

#ifndef RC_MACROS_H
#define RC_MACROS_H

#include "RCconfig.h"
#ifdef unix
#include <dlfcn.h>
#elif defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

namespace RC {

/// \def RC_NOP
/// Evaluates to whatever is passed in.  Does no operation.
#define RC_NOP(...) __VA_ARGS__

/// @cond UNDOC
#define RC_CAT_HELP(a,b) a ## b
/// @endcond
/// \def RC_CAT
/// Concatenates two tokens.
#define RC_CAT(a,b) RC_CAT_HELP(a,b)

/// @cond UNDOC
#define RC_EMPTY_CHECK2(a,b,...) b
#define RC_EMPTY_CHECKb(...) RC_EMPTY_CHECK2(__VA_ARGS__)
#define RC_EMPTY_CHECK(...) RC_EMPTY_CHECKb(__VA_ARGS__ (),0)
#define RC_EMPTY_F1RC_EMPTY_F0() ~,1,
#define RC_EMPTY_HELP3(a,b,...) a ## b
#define RC_EMPTY_HELP2(...) RC_EMPTY_HELP3(RC_EMPTY_F1, __VA_ARGS__ RC_EMPTY_F0)
#define RC_EMPTY_HELPb(x,...) RC_EMPTY_CHECK(x)
#define RC_EMPTY_HELP(...) RC_EMPTY_HELPb(__VA_ARGS__)
/// @endcond

/// \def RC_EMPTY
/// Returns 1 if no arguments passed, 0 if arguments are passed.
#define RC_EMPTY(...) RC_EMPTY_HELP(RC_EMPTY_HELP2(__VA_ARGS__))


/// @cond UNDOC
#define RC_ARGNUMCAT_HELPER(a, b) a ## b
#define RC_ARGNUMCAT(a, b) RC_ARGNUMCAT_HELPER(a, b)
#define RC_ARGS_NUM_HELPER(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,N,...) N
#define RC_ARGS_EVAL1(...) 0
#define RC_ARGS_EVAL0(...) RC_ARGS_NUM_HELPER(__VA_ARGS__,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
/// @endcond

/// \def RC_ARGS_NUM
/// Returns the number of arguments passed to the macro, from 0 to 63.
#define RC_ARGS_NUM(...) RC_ARGNUMCAT(RC_ARGS_EVAL,RC_EMPTY(__VA_ARGS__))(__VA_ARGS__)


/// @cond UNDOC
#define RC_EV(...)  RC_EV1(RC_EV1(RC_EV1(RC_EV1(__VA_ARGS__))))
#define RC_EV1(...) RC_EV2(RC_EV2(RC_EV2(RC_EV2(__VA_ARGS__))))
#define RC_EV2(...) RC_EV3(RC_EV3(RC_EV3(RC_EV3(__VA_ARGS__))))
#define RC_EV3(...) RC_EV4(RC_EV4(RC_EV4(RC_EV4(__VA_ARGS__))))
#define RC_EV4(...) __VA_ARGS__

#define RC_ARGS_EAT(...)
#define RC_ARGS_EACH_HELP_1 RC_ARGS_EAT
#define RC_ARGS_EACH_HELP_0 RC_ARGS_EACH_LOOP2 RC_ARGS_EAT() ()
#define RC_ARGS_EACH_HELP2(x) RC_ARGS_EACH_HELP_##x
#define RC_ARGS_EACH_HELP(x) RC_ARGS_EACH_HELP2(x)
#define RC_ARGS_EACH_LOOP(func,x,...) func(x)RC_ARGS_EACH_HELP(RC_EMPTY(__VA_ARGS__)) (func,__VA_ARGS__)
#define RC_ARGS_EACH_LOOP2() RC_ARGS_EACH_LOOP
/// @endcond

/// \def RC_ARGS_EACH
/// Does RC_ARGS_EACH(Macro,a,b,c) --> Macro(a)Macro(b)Macro(c), for any
/// number of parameters up to 342 or compiler limits.
#define RC_ARGS_EACH(Macro,...) RC_EV(RC_ARGS_EACH_HELP(RC_EMPTY(__VA_ARGS__))(Macro,__VA_ARGS__))

/// @cond UNDOC
#define RC_ARGS_BET_HELP0(func,bet,x,...) func(x)RC_ARGS_EACH(bet func,__VA_ARGS__)
#define RC_ARGS_BET_HELP1(func,bet,...)
#define RC_ARGS_BET_CAT2(a,b) a##b
#define RC_ARGS_BET_CAT(a,b) RC_ARGS_BET_CAT2(a,b)
#define RC_ARGS_BET_HELP(func,bet,...) RC_ARGS_BET_CAT(RC_ARGS_BET_HELP,RC_EMPTY(__VA_ARGS__))(func,bet,__VA_ARGS__)
/// @endcond

/// \def RC_ARGS_BET
/// Does RC_ARGS_BET(Func,Between,a,b,c) --> Func(a)Between Func(b)Between
/// Func(c)
/** For example:  cout RC_ARGS_BET(<< int, << ", ", 1.4, 2.5, 3.6); --> 
 *  cout << int(1.4) << ", " << int(2.5) << ", " << int(3.5);
 *  @see RC_ARGS_EACH
 */
#define RC_ARGS_BET(func,bet,...) RC_ARGS_BET_HELP(func,bet,__VA_ARGS__)

/// @cond UNDOC
#define RC_ARGS_LIST_FIRST_1 RC_ARGS_EAT
#define RC_ARGS_LIST_FIRST_0 RC_ARGS_LIST_LPFIRST2 RC_ARGS_EAT() ()
#define RC_ARGS_LIST_FIRST2(x) RC_ARGS_LIST_FIRST_##x
#define RC_ARGS_LIST_FIRST(x) RC_ARGS_LIST_FIRST2(x)
#define RC_ARGS_LIST_LPFIRST(func,x,...) func(x)RC_ARGS_LIST_HELP(RC_EMPTY(__VA_ARGS__)) (func,__VA_ARGS__)
#define RC_ARGS_LIST_LPFIRST2() RC_ARGS_LIST_LPFIRST
#define RC_ARGS_LIST_HELP_1 RC_ARGS_EAT
#define RC_ARGS_LIST_HELP_0 RC_ARGS_LIST_LOOP2 RC_ARGS_EAT() ()
#define RC_ARGS_LIST_HELP2(x) RC_ARGS_LIST_HELP_##x
#define RC_ARGS_LIST_HELP(x) RC_ARGS_LIST_HELP2(x)
#define RC_ARGS_LIST_LOOP(func,x,...) ,func(x)RC_ARGS_LIST_HELP(RC_EMPTY(__VA_ARGS__)) (func,__VA_ARGS__)
#define RC_ARGS_LIST_LOOP2() RC_ARGS_LIST_LOOP
/// @endcond

/// \def RC_ARGS_LIST
/// Does RC_ARGS_LIST(Func,a,b,c) --> Func(a),Func(b),Func(c)
/** @see RC_ARGS_EACH
 */
#define RC_ARGS_LIST(func,...) RC_EV(RC_ARGS_LIST_FIRST(RC_EMPTY(__VA_ARGS__))(func,__VA_ARGS__))

/// @cond UNDOC
#define RC_ARGS_PAIR_FIRST_1 RC_ARGS_EAT
#define RC_ARGS_PAIR_FIRST_0 RC_ARGS_PAIR_LPFIRST2 RC_ARGS_EAT() ()
#define RC_ARGS_PAIR_FIRST2(x) RC_ARGS_PAIR_FIRST_##x
#define RC_ARGS_PAIR_FIRST(x) RC_ARGS_PAIR_FIRST2(x)
#define RC_ARGS_PAIR_LPFIRST(odd,even,x,y,...) odd(x)even(y)RC_ARGS_PAIR_HELP(RC_EMPTY(__VA_ARGS__)) (odd,even,__VA_ARGS__)
#define RC_ARGS_PAIR_LPFIRST2() RC_ARGS_PAIR_LPFIRST
#define RC_ARGS_PAIR_HELP_1 RC_ARGS_EAT
#define RC_ARGS_PAIR_HELP_0 RC_ARGS_PAIR_LOOP2 RC_ARGS_EAT() ()
#define RC_ARGS_PAIR_HELP2(x) RC_ARGS_PAIR_HELP_##x
#define RC_ARGS_PAIR_HELP(x) RC_ARGS_PAIR_HELP2(x)
#define RC_ARGS_PAIR_LOOP(odd,even,x,y,...) ,odd(x)even(y)RC_ARGS_PAIR_HELP(RC_EMPTY(__VA_ARGS__)) (odd,even,__VA_ARGS__)
#define RC_ARGS_PAIR_LOOP2() RC_ARGS_PAIR_LOOP
/// @endcond

/// \def RC_ARGS_PAIR
/// Does RC_ARGS_PAIR(Odd,Even,a,b,c,d) --> Odd(a)Even(b),Odd(c)Even(d)
/** @see RC_ARGS_LIST
 */
#define RC_ARGS_PAIR(odd,even,...) RC_EV(RC_ARGS_PAIR_FIRST(RC_EMPTY(__VA_ARGS__))(odd,even,__VA_ARGS__))

/// \def RC_ARGS_DECLIST
/// Does "RC_ARGS_DECLIST(int,a,int,b)" --> "int a,int b"
#define RC_ARGS_DECLIST(...) RC_ARGS_PAIR(RC_NOP,RC_NOP,__VA_ARGS__)
/// \def RC_ARGS_PARAMLIST
/// Does "RC_ARGS_PARAMLIST(int,a,int,b)" --> "a,b"
#define RC_ARGS_PARAMLIST(...) RC_ARGS_PAIR(RC_ARGS_EAT,RC_NOP,__VA_ARGS__)

/// @cond UNDOC
#define RC_DEBOUT_HELP(v) << ", " << #v << " = " << v
/// @endcond

/// \def RC_DEBOUT
/// Use RC_DEBOUT(var1, ..., varN) for a flushed stderr debug printout of
/// variables.
/** This also prints the file name and line number, and can be used to mark
 *  when execution reaches a point with no variables as RC_DEBOUT().  The
 *  default destination stream of std::cerr can be changed by defining
 *  RC_DEBOUT_STREAM in RCconfig.h or earlier.
 */
#define RC_DEBOUT(...) RC_DEBOUT_STREAM << __FILE__ << ":" << __LINE__ RC_ARGS_EACH(RC_DEBOUT_HELP,__VA_ARGS__) << std::endl;

  /// Mark an unused variable to suppress warnings.
  template<class T> inline void UnusedVar(const T&) { }
/// \def RC_UNUSED_PARAM
/// Use in function headers as void func(int RC_UNUSED_PARAM(x)) to suppress
/// warnings about non-use.
#ifdef __GNUC__
#define RC_UNUSED_PARAM(v) v __attribute__((unused))
#elif defined(__LCLINT__)
#define RC_UNUSED_PARAM(v) /*@unused@*/ v
#else
#define RC_UNUSED_PARAM(v) v
#endif

/// \def RC_DYNAMIC_LOAD_FUNC_RAW
/// Use as RC_DYNAMIC_LOAD_FUNC_RAW(FuncName,Library) to load FuncName from the
/// dynamic library file Library.
/** This creates a static function pointer named FuncName in-place to provide
 *  access to the loaded library.  As it overrides FuncName, the previous
 *  declaration of FuncName MUST be in an outer scope (outside of the function
 *  where this is called).  For example, include the original definition from a
 *  header, and use this inside of a function or namespace.  After this macro,
 *  call FuncName with the same syntax as you would the  previously declared
 *  FuncName.  This macro is defined for both Linux and Windows.  In the event
 *  of an error, FuncName is null.  This must be checked in the current version
 *  of this macro.  A safer version with exception throws is
 *  RC_DYNAMIC_LOAD_FUNC.
 */
#ifdef unix
#define RC_DYNAMIC_LOAD_FUNC_RAW(FuncName,Library) \
  void* FuncName##_library = dlopen(Library, RTLD_LAZY | RTLD_GLOBAL); \
  static decltype(&FuncName) FuncName = (FuncName##_library) ? \
    reinterpret_cast<decltype(FuncName)> \
      (dlsym(FuncName##_library,#FuncName)): 0;
#elif defined(WIN32)
#define RC_DYNAMIC_LOAD_FUNC_RAW(FuncName,Library) \
  HMODULE FuncName##_library = LoadLibrary(Library); \
  static decltype(&FuncName) FuncName = (FuncName##_library) ? \
    reinterpret_cast<decltype(FuncName)> \
      (GetProcAddress(FuncName##_library,#FuncName)): 0;
#endif


/// @cond UNDOC
#define RC_CONSTWRAP_HELP2(Ret,Func,Line,...) \
  inline Ret Func(RC_ARGS_DECLIST(__VA_ARGS__)) { return RC_CONSTWRAP_Helper__##Line<Ret>(RC_ARGS_PARAMLIST(,*this,__VA_ARGS__)); } \
  /** \brief Const version of Func */ \
  inline const Ret Func(RC_ARGS_DECLIST(__VA_ARGS__)) const { return RC_CONSTWRAP_Helper__##Line<const Ret>(RC_ARGS_PARAMLIST(,*this,__VA_ARGS__)); } \
  template<class FHRet, class Self> \
  /** \private */ \
  inline static FHRet RC_CONSTWRAP_Helper__##Line(RC_ARGS_DECLIST(Self&, self, __VA_ARGS__))
#define RC_CONSTWRAP_HELP(Ret,Func,Line,...) RC_CONSTWRAP_HELP2(Ret,Func,Line,__VA_ARGS__)
/// @endcond
/// \def RC_CONSTWRAP
/// Use to metaprogrammatically generate paired const and non-const getters.
/** The first parameter is the return type, and the second the function name.
 *  Subsequent parameters are pairs of type and variable name for function
 *  parameters.  Access member variable x with "self.x".
 *  E.g.:  RC_CONSTWRAP(int&, MyFunc, size_t, index) {
 *  return self.array[index]; }
 */
#define RC_CONSTWRAP(Ret,Func,...) RC_CONSTWRAP_HELP(Ret,Func,__LINE__,__VA_ARGS__)


/// \def RC_GetTc
/// Generates wrappers for const Get_T functions.
#define RC_GetTc(T) \
    /** \brief Overloaded function to extract type T from this class.
        @param x The reference to which the value will be assigned. */ \
    inline void Get(T &x) const { x = Get_##T(); }
/// \def RC_GetT
/// Generates wrappers for Get_T functions.
#define RC_GetT(T) \
    /** \brief Overloaded function to extract type T from this class.
        @param x The reference to which the value will be assigned. */ \
    inline void Get(T &x) { x = Get_##T(); }


/// \def RC_DEFAULT_COMPARISON
/// Given == and <, generates !=, >, <=, and >=
#define RC_DEFAULT_COMPARISON() \
    /** \brief True if not equal. */ \
    template<class AnyValidType> \
    inline bool operator!= (const AnyValidType& other) const { \
      return !((*this) == other); \
    } \
    /** \brief True if other is less than this object. */ \
    template<class AnyValidType> \
    inline bool operator> (const AnyValidType& other) const { \
      return other < (*this); \
    } \
    /** \brief True if other is not less than this object. */ \
    template<class AnyValidType> \
    inline bool operator<= (const AnyValidType& other) const { \
      return !(other < (*this)); \
    } \
    /** \brief True if this object is not less than other. */ \
    template<class AnyValidType> \
    inline bool operator>= (const AnyValidType& other) const { \
      return !((*this) < other); \
    }

    /// \def RC_STREAM_RAWWRAP
    /// Provides stream output for a class via Raw.
#define RC_STREAM_RAWWRAP(Type) \
  /** \brief A convenience stream output for displaying the enclosed \
      object. */ \
  template <class T> \
  inline std::ostream& operator<< (std::ostream &out, Type<T> obj) { \
    return (out << obj.Raw()); \
  }

  /// \def RC_DefaultCopyMove
  /// Provides explicit default copy and move constructors and assignment.
#ifdef CPP11
  #define RC_DefaultCopyMove(ClassName) \
  /** \brief Default copy constructor. */ \
  ClassName(const ClassName&) = default; \
  /** \brief Default move constructor. */ \
  ClassName(ClassName&&) = default; \
  /** \brief Default copy assignment operator. */ \
  ClassName& operator=(const ClassName&) & = default; \
  /** \brief Default move assignment operator. */ \
  ClassName& operator=(ClassName&&) & = default;
#else
  #define RC_DefaultCopyMove(ClassName)
#endif

/// \def RC_ForIndex(i, cont)
/// Creates i of type equivalent to cont.size(), and loops over [0,cont.size).
/// Use like:  RC_ForIndex(i, cont) { cout << cont[i]; }
#define RC_ForIndex(i, cont) for (decltype(cont.size()) i = 0; i < cont.size(); ++i)

/// \def RC_ForEach(e, cont)
/// Provides element auto&& e derived from the cont operator[], and loops over
/// [0,cont.size).  Like a range-based for loop, but using operator[].
/// Use like:  RC_ForEach(e, cont) { cout << e; }
#define RC_ForEach(e, cont) for(decltype(cont.size()) RC_ForEach_i=0; RC_ForEach_i<cont.size(); ++RC_ForEach_i)if(bool RC_ForEach_b=false);else for(auto&& e=cont[RC_ForEach_i];RC_ForEach_b^=1;)

/// \def RC_ForRange(i, start, past_end)
/// Creates a for loop over i with type deduction from the past_end.
#define RC_ForRange(i, start, past_end) for (decltype(past_end) i = start; i < past_end; ++i)

/// \def RC_Repeat(times)
/// Repeats the following block the specified times, with automatic type
/// deduction.
#define RC_Repeat(times) for (decltype(times) RC_Repeat_i = 0; RC_Repeat_i < times; ++RC_Repeat_i)
 
}

#endif // MY_MACROS_H

