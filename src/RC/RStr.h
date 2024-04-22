/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2016, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file RStr.h
/// Provides a robust value-added wrapper for std::string.
/////////////////////////////////////////////////////////////////////

#ifndef RC_RSTR_H
#define RC_RSTR_H

#include "RCconfig.h"
#include "Macros.h"
#include "Types.h"
#include "Errors.h"
#include "RCBits.h"
#include "Data1D.h"
#include "Data2D.h"
#include "Data3D.h"
#include "Iter.h"

#ifdef WIN32
#include <tchar.h>
#endif

#include <algorithm>
#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef CPP11324324
#include <sstream>
#endif

/// @cond PROTECTED
// Use C++11 regex by default, otherwise try Boost.
#ifdef CPP11
#define RC_RSTR_REGEX
#include <regex>
#define RC_REGEX_NS std
#else
#ifdef RC_HAVE_BOOST
#define RC_RSTR_REGEX
#include <boost/regex.hpp>
#define RC_REGEX_NS boost
#endif
#endif
/// @endcond

#ifdef RC_HAVE_QT
#include <QString>
#endif


namespace RC {
  /// The styles in which an integral number can be formatted.
  /** DEC == decimal, HEX == hexadecimal, HEX0x == hexadecimal with "0x"
   *  prepended, OCT == octal, OCT0 == octal with "0" prepended,
   *  BIN == binary, CHAR == treated as a character.
   */
  enum RStr_IntStyle { DEC=100000, HEX, HEX0x, OCT, OCT0, BIN, CHAR };
  /// The styles in which a floating point number can be formatted.
  /** AUTO == determine the format automatically, FIXED == as 0.0000123,
   *  SCI == as 1.23e-5
   *  SCI treats the precision parameter as a count of significant digits.
   *  FIXED treats the precision parameter as digits right of the decimal
   *  point.
   *  AUTO treats the precision parameter as the most significant digits
   *  to show, but trims trailing zeros.
   */
  enum RStr_FloatStyle { AUTO=100000, FIXED, SCI };


  class RStr;
  /// The random-access iterator for RStr begin() and end()
  typedef RAIter<RStr,char> RStrIter;

  /// A bounds-safe string class which provides an identical interface to
  /// std::string plus many convenience functions for string manipulation.
  /** This class checks boundaries on access and sanitizes inputs whenever
   *  possible, throwing ErrorMsgBounds or ErrorMsgNull when a usage problem
   *  arises.  Functions are provided for converting to and from numerical
   *  types with formatting control, as well as high-level functions for
   *  splitting, joining, regular expressions, word-wrapping and alignment,
   *  and working with base64, UTF-8, and CSV. 
   *
   *  Usage example:
   *  \code{.cpp}
   *  u64 val = 0x12ab34cd5;
   *
   *  // 5011360981 in hex is 0x000000012ab34cd5, or 12ab34cd5
   *  cout << val << " in hex is " << RStr(val, HEX0x)
   *       << ", or " << RStr(val, HEX, 1) << endl;
   *
   *  // Its reciprocal is 1.9955e-10
   *  RStr message = "Its reciprocal is " + RStr(1.0/val, SCI, 5) + "\n";
   *  cout << message;
   *
   *  // Pi dollars would be $3.14
   *  cout << "Pi dollars would be $" << RStr(3.14159, FIXED, 2) << endl;
   *
   *  // foo, bar, baz
   *  cout << RStr::Join(RStr("foo:bar:baz").Split(':'), ", ") << endl;
   *  \endcode
   *  \nosubgrouping
   */
  class RStr {
    protected:
    /// @cond PROTECTED

    #ifdef WIN32
    static inline size_t strnlen(const char *s, size_t n) {
      size_t i;
      for (i=0; i<n; i++) {
        if (s[i] == 0) {
          return i;
        }
      }
      return n;
    }
    #endif

    template<class InputIterator>
    inline void FindNullTerm(InputIterator& first, InputIterator& last) {
      for(InputIterator iter = first; iter != last; ++iter) {
        if (0 == char(*iter)) {
          last = iter;
          break;
        }
      }
    }
    /// @endcond
    public:

    /// \name Wrapper methods for std::string
    /// @{


    /// Default constructor, initializes to blank string.
    inline RStr() {
    }

    /// Copy constructor.
    inline RStr(const RStr& other)
      : str (other.str) {
    }

    /// Initializes to str.
    inline RStr(const std::string& str)
      : str (str) {
    }

    /// Creates a new RStr from a segment of s starting at index pos with
    /// length n or until the end.
    /** Throws RC::ErrorMsgBounds if pos is out of bounds. */
    inline RStr(const RStr& s, size_t pos, size_t n = npos) {
      s.AssertPlus(pos);
      str = std::string(s.str, pos, n);
    }

    /// Creates a new RStr from n characters at s, or until null-termination.
    /** Creates an empty string if s is NULL. */
    inline RStr(const char *s, size_t n) {
      if (s != NULL) {
        size_t len = strnlen(s, n);
        if (len < n) { n = len; }
        str = std::string(s, n);
      }
    }

    /// Creates a new RStr from s until null-termination, or empty string if s
    /// is NULL.
    inline RStr(const char *s) {
      if (s != NULL) {
        str = std::string(s);
      }
    }

    /// Initializes to n copies of character c.
    inline RStr(size_t n, char c)
      : str (std::string(n, c)) {
    }

    /// Initializes the string from the values in the iterator range.
    template <class InputIterator>
    inline RStr(InputIterator begin, InputIterator end)
      : str (std::string(begin, end)) {
    }

#ifdef CPP11
    /// Initializes the string with the list of characters.
    inline RStr(const std::initializer_list<char> characters)
      : str (characters) {
    }

    /// Moves other to this RStr.
    inline RStr(RStr&& other) noexcept
      : str (other.str) {
    }
#endif

    /// Copies the contents of s to this RStr.
    inline RStr& operator= (const RStr& s) {
      str = s.str;
      return *this;
    }

    /// Copies the null-terminated characters at s to this RStr, or empties
    /// if s is NULL.
    inline RStr& operator= (const char *s) {
      if (s != NULL) {
        str = s;
      }
      else {
        str.clear();
      }
      return *this;
    }

    /// Sets the RStr to char c.
    inline RStr& operator= (char c) {
      str = c;
      return *this;
    }

#ifdef CPP11
    /// Sets the contents of the RStr to the initializer list.
    inline RStr& operator= (std::initializer_list<char> characters) {
      str = characters;
      return *this;
    }

    /// Moves the contests of other to this RStr.
    inline RStr& operator= (RStr&& other) {
      str = std::move(other.str);
      return *this;
    }
#endif

    /// Returns a bounds-safe iterator to the first element.
    /** The iterator throws ErrorMsgNull if it is accessed after this RStr
     *  is destructed.  The element pointed to by this respects the offset
     *  with SetOffset.
     */
    inline RStrIter begin() {
      return RStrIter(this, 0);
    }

    /// Returns a bounds-safe iterator which points past the last element.
    /** Accessing this iterator before decrementing it will trigger an
     *  ErrorMsgBounds.  Accessing a decremented iterator after this RStr is
     *  destructed will throw ErrorMsgNull.
     */
    inline RStrIter end() {
      return RStrIter(this, str.length());
    }


    /// Returns the length of the string.
    inline size_t size() const { return str.size(); }
    /// Returns the length of the string.
    inline size_t length() const { return str.length(); }
    /// Returns the maximum size of the string.
    inline size_t max_size() const { return str.max_size(); }
    /// Resizes the string, filling with character c if enlarging.
    inline void resize(size_t n, char c) { str.resize(n, c); }
    /// Resizes the string, filling with null characters if enlarging.
    inline void resize(size_t n) { str.resize(n); }
    /// Returns the allocated storage space
    inline size_t capacity() const {
      return str.capacity();
    }
    /// Request a storage capacity increase to capsize.
    inline void reserve( size_t capsize=0 ) { str.reserve(capsize); }
    /// Make the string empty.
    inline void clear() { str.clear(); }
    /// True of the string is empty.
    inline bool empty() const { return str.empty(); }

    /// Returns the char at index pos, or throws ErrorMsgBounds if out of
    /// bounds.
    inline char& operator[] (size_t pos) {
      AssertPlus(pos);
      return str[pos];
    }

    /// Const version of operator[]
    inline const char& operator[] (size_t pos) const {
      AssertPlus(pos);
      return str[pos];
    }

    /// Identical to operator[]
    inline char& at(size_t pos) {
      Assert(pos);
      return str[pos];
    }

    /// Const version of at
    inline const char& at(size_t pos) const {
      Assert(pos);
      return str[pos];
    }

    /// Appends s to this RStr.
    inline RStr& operator+= (const RStr& s) {
      str += s.str;
      return *this;
    }

    /// Appends s to this RStr if s is non-NULL.
    inline RStr& operator+= (const char* s) {
      if (s != NULL) {
        str += s;
      }
      return *this;
    }

    /// Appends the char c to this RStr.
    inline RStr& operator+= (char c) {
      str += c;
      return *this;
    }

    /// Appends s to this RStr.
    inline RStr& append(const RStr& s) {
      str.append(s.str);
      return *this;
    }

    /// Appends n characters in s starting at pos to this RStr, or fewer
    /// characters if s is too short.
    inline RStr& append(const RStr& s, size_t pos, size_t n) {
      str.append(s.str, pos, n);
      return *this;
    }

    /// Appends n characters starting at s to this RStr, or fewer if
    /// null-termination is reached in s.
    /** Appends nothing if s is NULL. */
    inline RStr& append(const char* s, size_t n) {
      if (s != NULL) {
        size_t len = strnlen(s, n);
        if (len < n) { n = len; }
        str.append(s, n);
      }
      return *this;
    }

    /// Append s to this RStr if s is non-NULL.
    inline RStr& append(const char* s) {
      if (s != NULL) {
        str.append(s);
      }
      return *this;
    }

    /// Appends n copies of char c.
    inline RStr& append(size_t n, char c) {
      str.append(n, c);
      return *this;
    }

    /// Appends from first to one before last, or until a null character.
    template <class InputIterator>
    inline RStr& append(InputIterator first, InputIterator last) {
      FindNullTerm(first, last);
      str.append(first, last);
      return *this;
    }

#ifdef CPP11
    /// Append the initializer list of characters.
    inline RStr& append (std::initializer_list<char> characters) {
      str.append(characters);
      return *this;
    }
#endif

    /// Appends character c.
    inline void push_back(char c) { str.push_back(c); }

    /// Copies s to this RStr.
    inline RStr& assign(const RStr& s) {
      str.assign(s.str);
      return *this;
    }

    /// Sets this RStr to n characters in s at index pos, or until the end.
    inline RStr& assign(const RStr& s, size_t pos, size_t n) {
      str.assign(s.str, pos, n);
      return *this;
    }

    /// Sets this RStr to n characters starting at s or until null-termination.
    /** The string is cleared if s is NULL. */
    inline RStr& assign(const char* s, size_t n) {
      if (s != NULL) {
        size_t len = strnlen(s, n);
        if (len < n) { n = len; }
        str.assign(s, n);
      }
      else { str.clear(); }
      return *this;
    }

    /// Sets this RStr to the characters at s until null-termination.
    /** The string is cleared if s is NULL. */
    inline RStr& assign(const char* s) {
      if (s != NULL) {
        str.assign(s);
      }
      else { str.clear(); }
      return *this;
    }

    /// Sets this RStr to n copies of character c.
    inline RStr& assign(size_t n, char c) {
      str.assign(n, c);
      return *this;
    }

    /// Sets the RStr to characters from first to one before last, or until
    /// a null character.
    template <class InputIterator>
    inline RStr& assign(InputIterator first, InputIterator last) {
      FindNullTerm(first, last);
      str.assign(first, last);
      return *this;
    }

    /// Inserts s starting at index pos_this of this RStr, or throws
    /// ErrorMsgBounds.
    // Bounds check does not throw out_of_range.  Throws RC::ErrorMsgBounds.
    inline RStr& insert(size_t pos_this, const RStr& s) {
      AssertPlus(pos_this);
      str.insert(pos_this, s.str);
      return *this;
    }

    /// Inserts at index pos_this, n characters from s starting at pos_s.
    /** Throws ErrorMsgBounds if pos_this or pos_s are out of their respective
     *  bounds.  Inserts fewer characters if n is greater than the remaining
     *  characters.
     */
    inline RStr& insert(size_t pos_this, const RStr& s,
                        size_t pos_s, size_t n) {
      AssertPlus(pos_this);
      s.AssertPlus(pos_s);
      str.insert(pos_this, s.str, pos_s, n);
      return *this;
    }

    /// Inserts at index pos_this, n characters from s or until
    /// NULL-termination
    /** If s is NULL, nothing is inserted. */
    inline RStr& insert(size_t pos_this, const char* s, size_t n) {
      if (s != NULL) {
        AssertPlus(pos_this);
        n = strnlen(s, n);
        str.insert(pos_this, s, n);
      }
      return *this;
    }

    /// Inserts s at index pos_this until null-termination, or nothing if s
    /// is NULL.
    inline RStr& insert(size_t pos_this, const char* s) {
      if (s != NULL) {
        AssertPlus(pos_this);
        str.insert(pos_this, s);
      }
      return *this;
    }

    /// Inserts n copies of char c at pos_this.
    inline RStr& insert(size_t pos_this, size_t n, char c) {
      AssertPlus(pos_this);
      str.insert(pos_this, n, c);
      return *this;
    }

    /// Inserts char c at iterator position p.
    inline RStrIter insert(RStrIter p, char c) {
      str.insert(p.GetIndex(), 1, c);
      return p;
    }

    /// Inserts n copies of char c at iterator position p.
    inline void insert(RStrIter p, size_t n, char c) {
      str.insert(p.GetIndex(), n, c);
    }

    /// At iterator position p, inserts characters from first through one
    /// before last or until null-termination.
    template <class InputIterator>
    inline void insert(RStrIter p, InputIterator first, InputIterator last) {
      FindNullTerm(first, last);
      AssertPlus(p.GetIndex());
      str.insert(str.begin()+p.GetIndex(), first, last);
    }

#ifdef CPP11
    /// At iterator position p, inserts list of characters.
    inline RStr& insert(const RStrIter p,
                        std::initializer_list<char> characters) {
      str.insert(p.GetIndex(), characters);
      return *this;
    }
#endif

    /// Erases n characters at index pos, or until the end.
    inline RStr& erase (size_t pos=0, size_t n=npos) {
      AssertPlus(pos);
      str.erase(pos, n);
      return *this;
    }

    /// Erases one character at index pos.
    inline RStrIter erase(const RStrIter pos) {
      erase(pos.GetIndex(), 1);
      return pos;
    }

    /// Erases from index first through one before index last.
    inline RStrIter erase(const RStrIter first, const RStrIter last) {
      size_t first_i = first.GetIndex();
      size_t last_i = last.GetIndex();

      if (last_i > first_i) {
        erase(first_i, last_i - first_i);
      }

      return first;
    }


    /// Replace n_this characters at pos_this with all of s.
    inline RStr& replace(size_t pos_this, size_t n_this, const RStr& s) {
      erase(pos_this, n_this);
      insert(pos_this, s);
      return *this;
    }

    /// Replace from first to one before last with all of s.
    inline RStr& replace(RStrIter first, RStrIter last, const RStr& s) {
      return replace(first.GetIndex(), last-first, s);
    }

    /// Replace n_this characters at pos_this with n_s characters from s
    /// starting at pos_s.
    inline RStr& replace(size_t pos_this, size_t n_this,
                         const RStr& s, size_t pos_s, size_t n_s) {
      erase(pos_this, n_this);
      insert(pos_this, s, pos_s, n_s);
      return *this;
    }

    /// Replace n_this characters at pos_this with n_s characters from s, or
    /// until s's null-termination.
    inline RStr& replace(size_t pos_this, size_t n_this, const char* s,
                         size_t n_s) {
      erase(pos_this, n_this);
      insert(pos_this, s, n_s);
      return *this;
    }

    /// Replace from first through one before last with n_s characters from s,
    /// or until s's null-termination.
    inline RStr& replace(RStrIter first, RStrIter last,
                         const char* s, size_t n_s) {
      return replace(first.GetIndex(), last-first, s, n_s);
    }

    /// Replace n_this characters at pos_this with all of s, or nothing if s
    /// is NULL.
    inline RStr& replace(size_t pos_this, size_t n_this, const char* s) {
      erase(pos_this, n_this);
      insert(pos_this, s);
      return *this;
    }

    /// Replace from first to one before last with all of s, or nothing if s
    /// is NULL.
    inline RStr& replace(RStrIter first, RStrIter last, const char* s) {
      return replace(first.GetIndex(), last-first, s);
    }

    /// Replace n_this characters at pos_this with n_c copies of c.
    inline RStr& replace(size_t pos_this, size_t n_this, size_t n_c, char c) {
      erase(pos_this, n_this);
      insert(pos_this, n_c, c);
      return *this;
    }

    /// Replace from first through one before last with n_c copies of c.
    // Bounds checked.
    inline RStr& replace(RStrIter first, RStrIter last, size_t n_c, char c) {
      return replace(first.GetIndex(), last-first, n_c, c);
    }

    /// Replace from first through one before lsat with in1 through one before
    /// in2, or until null-termination.
    template <class InputIterator>
    inline RStr& replace(RStrIter first, RStrIter last,
                         InputIterator in_first, InputIterator in_last) {
      erase(first, last);
      insert(first, in_first, in_last);
      return *this;
    }


    /// Swap contents with s.
    inline void swap(RStr& s) {
      str.swap(s.str);
    }

    /// Provides a null-terminated C style string corresponding to RStr.
    inline const char* c_str() const {
      return str.c_str();
    }

    /// Identical to c_str.
    inline const char* data() const {
      return str.c_str();
    }


    /// Get the allocator used for string storage.
    inline std::allocator<char> get_allocator() const {
      return str.get_allocator();
    }


    /// Copies n characters to s, starting from pos_this.
    /** Warning:  If n is greater than the length of s, this routine can
     *  write past the end of s.  Consider instead obtaining a managed
     *  Data1D<char> via ToData(), using Data1D::CopyAt, and accessing the
     *  char* with Data1D::Raw for interfacing with C routines.
     */
    inline size_t copy(char *s, size_t n, size_t pos=0) const {
      Assert(s);
      AssertPlus(pos);
      return str.copy(s, n, pos);
    }


    /// Returns the index at pos or later which matches string s, or npos if
    /// no match.
    inline size_t find(const RStr& s, size_t pos=0) const {
      return str.find(s.str, pos);
    }

    /// Returns the index at pos or later which matches n characters of s, or
    /// npos if no match.
    inline size_t find(const char* s, size_t pos, size_t n) const {
      Assert(s);
      if (strnlen(s, n) < n) {
        return npos;
      }
      return str.find(s, pos, n);
    }

    /// Returns the index at pos or later which matches s until
    /// null-termination, or npos if no match.
    inline size_t find(const char* s, size_t pos=0) const {
      Assert(s);
      return str.find(s, pos);
    }

    /// Returns the index at pos or later which matches character c.
    inline size_t find(char c, size_t pos=0) const {
      return str.find(c, pos);
    }


    /// Returns the index at pos or before which starts a match of string s,
    /// or npos if no match.
    inline size_t rfind(const RStr& s, size_t pos=npos) const {
      return str.rfind(s.str, pos);
    }

    /// Returns the index at pos or before which matches n characters of s, or
    /// npos if no match.
    inline size_t rfind(const char* s, size_t pos, size_t n) const {
      Assert(s);
      if (strnlen(s, n) < n) {
        return npos;
      }
      return str.rfind(s, pos, n);
    }

    /// Returns the index at pos or before which matches s until
    /// null-termination, or npos if no match.
    inline size_t rfind(const char* s, size_t pos=npos) const {
      Assert(s);
      return str.rfind(s, pos);
    }

    /// Returns the index at pos or before which matches character c.
    inline size_t rfind(char c, size_t pos=npos) const {
      return str.rfind(c, pos);
    }


    /// Returns the first index at pos or later which matches a character
    /// in s, or npos if none match.
    inline size_t find_first_of(const RStr& s, size_t pos=0) const {
      return str.find_first_of(s.str, pos);
    }

    /// Returns the first index at pos or later which matches one of the first
    /// n characters in s, or npos if none match.
    inline size_t find_first_of(const char* s, size_t pos, size_t n) const {
      Assert(s);
      size_t len = strnlen(s, n);
      if (len < n) { n = len; }
      return str.find_first_of(s, pos, n);
    }

    /// Returns the first index at pos or later which matches a character
    /// in s, or npos if none match.
    inline size_t find_first_of(const char* s, size_t pos=0) const {
      Assert(s);
      return str.find_first_of(s, pos);
    }

    /// Returns the index at pos or later which matches character c.
    inline size_t find_first_of(char c, size_t pos=0) const {
      return str.find_first_of(c, pos);
    }


    /// Returns the highest index at pos or before which matches a character
    /// in s, or npos if none match.
    inline size_t find_last_of(const RStr& s, size_t pos=npos) const {
      return str.find_last_of(s.str, pos);
    }

    /// Returns the highest index at pos or before which matches the first n
    /// characters in s, or npos if none match.
    inline size_t find_last_of(const char* s, size_t pos, size_t n) const {
      Assert(s);
      size_t len = strnlen(s, n);
      if (len < n) { n = len; }
      return str.find_last_of(s, pos, n);
    }

    /// Returns the highest index at pos or before which matches a character
    /// in s, or npos if none match.
    inline size_t find_last_of(const char* s, size_t pos=npos) const {
      Assert(s);
      return str.find_last_of(s, pos);
    }

    /// Returns the highest index at pos or before which matches character c,
    /// or npos if none match.
    inline size_t find_last_of(char c, size_t pos=npos) const {
      return str.find_last_of(c, pos);
    }


    /// Returns the first index at pos or later which does not match a
    /// character in s, or npos if all match.
    inline size_t find_first_not_of(const RStr& s, size_t pos=0) const {
      return str.find_first_not_of(s.str, pos);
    }

    /// Returns the first index at pos or later which does not match the first
    /// n characters in s, or npos if all match.
    inline size_t find_first_not_of(const char* s, size_t pos, size_t n) const {
      Assert(s);
      size_t len = strnlen(s, n);
      if (len < n) { n = len; }
      return str.find_first_not_of(s, pos, n);
    }

    /// Returns the first index at pos or later which does not match a
    /// character in s, or npos if all match.
    inline size_t find_first_not_of(const char* s, size_t pos=0) const {
      Assert(s);
      return str.find_first_not_of(s, pos);
    }

    /// Returns the first index at pos or later which does not match character
    /// c, or npos if all match.
    inline size_t find_first_not_of(char c, size_t pos=0) const {
      return str.find_first_not_of(c, pos);
    }


    /// Returns the highest index at pos or before which does not match a
    /// character in s, or npos if all match.
    inline size_t find_last_not_of(const RStr& s, size_t pos=npos) const {
      return str.find_last_not_of(s.str, pos);
    }

    /// Returns the highest index at pos or before which does not match the
    /// first n characters in s, or npos if all match.
    inline size_t find_last_not_of(const char* s, size_t pos, size_t n) const {
      Assert(s);
      size_t len = strnlen(s, n);
      if (len < n) { n = len; }
      return str.find_last_not_of(s, pos, n);
    }

    /// Returns the highest index at pos or before which does not match a
    /// character in s, or npos if all match.
    inline size_t find_last_not_of(const char* s, size_t pos=npos) const {
      Assert(s);
      return str.find_last_not_of(s, pos);
    }

    /// Returns the highest index at pos or before which does not match
    /// character c, or npos if all match.
    inline size_t find_last_not_of(char c, size_t pos=npos) const {
      return str.find_last_not_of(c, pos);
    }


    /// Creates a substring from n characters starting at position pos, or
    /// until the end of the string.
    inline RStr substr(size_t pos=0, size_t n=npos) const {
      AssertPlus(pos);
      return RStr(str.substr(pos, n));
    }


    /// Returns negative, 0, or positive if this string is lesser,
    /// equal, or greater than s.
    inline int compare(const RStr& s) const {
      return str.compare(s.str);
    }

    /// Returns negative, 0, or positive if this string is lesser,
    /// equal, or greater than s.
    inline int compare(const char* s) const {
      Assert(s);
      return str.compare(s);
    }

    /// Returns negative, 0, or positive if the n_this characters at pos_this
    /// are lesser, equal, or greater than s.
    inline int compare(size_t pos_this, size_t n_this, const RStr& s) const {
      AssertPlus(pos_this);
      return str.compare(pos_this, n_this, s.str);
    }

    /// Returns negative, 0, or positive if the n_this characters at pos_this
    /// are lesser, equal, or greater than s.
    inline int compare(size_t pos_this, size_t n_this, const char* s) const {
      Assert(s);
      AssertPlus(pos_this);
      return str.compare(pos_this, n_this, s);
    }

    /// Returns negative, 0, or positive if the n_this characters at pos_this
    /// are lesser, equal, or greater than the n_s characters at pos_s in s.
    inline int compare(size_t pos_this, size_t n_this,
                       const RStr& s, size_t pos_s, size_t n_s) const {
      AssertPlus(pos_this);
      s.AssertPlus(pos_s);
      return str.compare(pos_this, n_this, s.str, pos_s, n_s);
    }

    /// Returns negative, 0, or positive if the n_this characters at pos_this
    /// are lesser, equal, or greater than the n_s characters in s.
    inline int compare(size_t pos_this, size_t n_this,
                       const char* s, size_t n_s) const {
      Assert(s);
      AssertPlus(pos_this);
      size_t len = strnlen(s, n_s);
      if (len < n_s) { n_s = len; }
      return str.compare(pos_this, n_this, s, n_s);
    }


    /// @}
    // End wrapper methods for std::string.


    protected:
    /// @cond PROTECTED
    template<class T>
    void ParseInt(T x, RStr_IntStyle style=DEC, i32 pad_to=-1,
                  char pad_with='0') {
      Data1D<char> arr;
      arr.Reserve(64);
      char basetable[37] = "0123456789abcdefghijklmnopqrstuvwxyz";
      i32 base = 10;
      switch(style) {
        case BIN:    base = 2;   break;
        case OCT0:
        case OCT:    base = 8;   break;
        case DEC:    base = 10;  break;
        case HEX0x:
        case HEX:    base = 16;  break;
        case CHAR:   str = char(x);  return;
        default:  Throw_RC_Error("Unknown style");
      }
      if (base < 1 || base > 36) {
        Throw_RC_Error("Base out of range");
      }
      // Auto hex precision for type size.
      if (pad_to < 0 && base == 16) {
        pad_to = i32(sizeof(x) << 1);
      }

      bool neg = false;
      if (x<0) {
        neg = true;
        do {
          arr += basetable[-(x%base)];
          x /= base;
        } while (x);
      }
      else {
        do {
          arr += basetable[x%base];
          x /= base;
        } while (x);
      }

      size_t pad_by = 0;
      if (pad_to > 0 && arr.size() < size_t(pad_to)) {
        pad_by = pad_to - arr.size();
      }

      if (neg) {
        str.reserve(str.size() + pad_to + arr.size() + 1);
        str += '-';
      }
      else {
        str.reserve(str.size() + pad_to + arr.size());
      }

      if (style == OCT0) {
        str += "0";
      }
      else if (style == HEX0x) {
        str += "0x";
      }

      for (size_t i=0; i<pad_by; i++) {
        str += pad_with;
      }

      for (size_t i=0; i<arr.size(); i++) {
        str += arr[arr.size()-1-i];
      }
    }

    template<class T>
    void ParseFloat(T x, RC::RStr_FloatStyle style=AUTO,
                    u32 precision=std::numeric_limits<T>::digits10) {
#ifdef WIN32
      unsigned int previous_output_format;
      previous_output_format = _set_output_format(_TWO_DIGIT_EXPONENT);
#endif

      std::stringstream ss;
      if (style == SCI) {
        ss.setf(std::ios::scientific, std::ios::floatfield);
        precision--;
      }
      else if (style == FIXED) {
        ss.setf(std::ios::fixed, std::ios::floatfield);
      }
      ss.precision(precision);
      ss << x;
      ss >> str;

#ifdef WIN32
          _set_output_format(previous_output_format);
#endif
    }

    /// @endcond
    public:

    /// \name Convert to and from other supported types
    /// @{

    /// The default constructor for a char, treating it as a character.
    /** To override, cast to another type or add an RStr_IntStyle parameter.
     */
    inline RStr(char x) {
      str = x;
    }

    /// Formats x as a string in the given style, and with at least
    /// precision 0-padded digits.
    /** Note, the default style for a char is as a character. */
    inline RStr(char x, RStr_IntStyle style, i32 precision=-1) { \
      ParseInt(x, style, precision); \
    }

    /// Internal.
#define RC_RSTR_Int_Input(TYPE) \
    /** \brief Formats x as a string in the given style, and with at least \
        precision 0-padded digits. */ \
    inline RStr(TYPE x, RStr_IntStyle style=DEC, i32 precision=-1) { \
      ParseInt(x, style, precision); \
    }

    RC_RSTR_Int_Input(u8)
    RC_RSTR_Int_Input(i8)
    RC_RSTR_Int_Input(u16)
    RC_RSTR_Int_Input(i16)
    RC_RSTR_Int_Input(u32)
    RC_RSTR_Int_Input(i32)
    RC_RSTR_Int_Input(u64)
    RC_RSTR_Int_Input(i64)
#ifdef MACOS
    RC_RSTR_Int_Input(size_t)
#endif

    /// Internal.
#define RC_RSTR_Float_Input(TYPE) \
    /** \brief Formats x as a string in the given style, and with it rounded
        to precision digits.
        \details For SCI precision is the significant digits to show, for
        AUTO it is the  most signicant digits to show before removing trailing
        zeroes, and for FIXED, precision is digits after the decimal.  AUTO
        has no exponent for numbers in the range (1e-5, 10^Precision). */ \
    inline RStr(TYPE x, RStr_FloatStyle style=AUTO, \
                u32 precision=std::numeric_limits<TYPE>::digits10) { \
      ParseFloat(x, style, precision); \
    }
    RC_RSTR_Float_Input(f32)
    RC_RSTR_Float_Input(f64)
#ifdef RC_HAVE_F80
    RC_RSTR_Float_Input(f80)
#endif


    /// "true" if b is true, or "false" if b is false.
    inline RStr(bool b) {
      str = b ? "true" : "false";
    }


    /// Constructor for displaying a pointer, with 0's prepended if pad0s is
    /// true, and 0x prepended if use0x is true.
    inline RStr(bool pad0s, void *ptr, bool use0x=true) {
      u32 precision = pad0s ? (2*sizeof(ptr)) : 1;

      if (use0x) {
        ParseInt(size_t(ptr), HEX0x, precision);
      }
      else {
        ParseInt(size_t(ptr), HEX, precision);
      }
    }


    /// Initialize the string with the bytes stored in Data1D treated as
    /// raw character data.
    template <class T>
    explicit inline RStr(const Data1D<T>& arr) {
      char *s = reinterpret_cast<char*>(arr.Raw());
      size_t n = arr.size() * arr.TypeSize();
      if (s != NULL) {
        n = strnlen(s, n);
        str = std::string(s, n);
      }
    }


    /// Convert a std::wstring to RStr, discarding the high bits.
    inline RStr(const std::wstring& wstr) {
      str.assign(wstr.begin(), wstr.end());
    }


#ifdef RC_HAVE_QT
    /// Convert a QString to RStr, discarding the high bits.
    inline RStr(const QString &qstr) {
      str = qstr.toStdString();
    }
#endif


    /// Convert the beginning characters of this string to a float.
    /** Note, these detect hexadecimal float numbers formatted like like
     *  0xF.8 = 15.5, but they do not process octal.
     */
    inline f32 Get_f32() const { return strtof(str.c_str(), NULL); }
    /// Convert the beginning characters of this string to a float.
    inline f64 Get_f64() const { return strtod(str.c_str(), NULL); }
#ifdef RC_HAVE_F80
    /// Convert the beginning characters of this string to a float.
    inline f80 Get_f80() const { return strtold(str.c_str(), NULL); }
#endif

    /// Convert the beginning characters of this string to this integer type.
    /** Note, the default parameter for base autodetects according
     *  to decimal, unless 0-leading octal as 013 = 11, or 0x-leading
     *  hexadecimal as 0xF = 15.
     */
    inline u32 Get_u32(int base=0) const
      { return strtoul(str.c_str(), NULL, base); }
    /// Convert the beginning characters of this string to this integer type.
    inline u64 Get_u64(int base=0) const
      { return strtoull(str.c_str(), NULL, base); }
    /// Convert the beginning characters of this string to this integer type.
    inline i32 Get_i32(int base=0) const
      { return strtol(str.c_str(), NULL, base); }
    /// Convert the beginning characters of this string to this integer type.
    inline i64 Get_i64(int base=0) const
      { return strtoll(str.c_str(), NULL, base); }

    /// Convert the beginning characters of this string as a hexadecimal
    /// to this integer type.
    inline u32 Get_hex32() const { return Get_u32(16); }
    /// Convert the beginning characters of this string as a hexadecimal
    /// to this integer type.
    inline u64 Get_hex64() const { return Get_u64(16); }

    /// Returns true if case-insensitive "true", "T", or non-zero.
    inline bool Get_bool() const {
      if (Is_f64() && (Get_f64() != 0)) {
        return true;
      }
      RStr tmp = *this;
      tmp.ToLower();
      if (tmp.str == "true" || tmp.str == "t") {
        return true;
      }
      return false;
    }

    // Provides Get(x) for all the types above.
    RC_GetTc(f32)
    RC_GetTc(f64)
#ifdef RC_HAVE_F80
    RC_GetTc(f80)
#endif
    RC_GetTc(u32)
    RC_GetTc(u64)
    RC_GetTc(i32)
    RC_GetTc(i64)
    RC_GetTc(bool)


    /// True if the start of the string can be converted to an f32.
    /** @param strict Return true only if the full string converts. */
    inline bool Is_f32(bool strict=false) const {
      char *test;
      errno = 0;
      f32 x = strtof(str.c_str(), &test);
      UnusedVar(x);
      bool retval = (errno == 0);
      if (strict) { retval = retval && (test == str.c_str()+str.length()); }
      else { retval = retval && (test != str.c_str()); }
      return retval;
    }

    /// True if the start of the string can be converted to an f64.
    /** @param strict Return true only if the full string converts. */
    inline bool Is_f64(bool strict=false) const {
      char *test;
      errno = 0;
      f64 x = strtod(str.c_str(), &test);
      UnusedVar(x);
      bool retval = (errno == 0);
      if (strict) { retval = retval && (test == str.c_str()+str.length()); }
      else { retval = retval && (test != str.c_str()); }
      return retval;
    }

#ifdef RC_HAVE_F80
    /// True if the start of the string can be converted to an f80.
    /** @param strict Return true only if the full string converts. */
    inline bool Is_f80(bool strict=false) const {
      char *test;
      errno = 0;
      f80 x = strtold(str.c_str(), &test);
      UnusedVar(x);
      bool retval = (errno == 0);
      if (strict) { retval = retval && (test == str.c_str()+str.length()); }
      else { retval = retval && (test != str.c_str()); }
      return retval;
    }
#endif

    /// True if the start of the string can be converted to an u32.
    /** @param base The conversion base to test (0 is automatic).
     *  @param strict Return true only if the full string converts.
     */
    inline bool Is_u32(int base=0, bool strict=false) const {
      char *test;
      errno = 0;
      u32 x = strtoul(str.c_str(), &test, base);
      UnusedVar(x);
      bool retval = (errno == 0);
      if (strict) {
        retval = retval && (test == str.c_str()+str.length())
                 && (Get_f32() >= 0);
      }
      else { retval = retval && (test != str.c_str()); }
      return retval;
    }

    /// True if the start of the can be converted to an u64.
    /** @param base The conversion base to test (0 is automatic).
     *  @param strict Return true only if the full string converts.
     */
    inline bool Is_u64(int base=0, bool strict=false) const {
      char *test;
      errno = 0;
      u64 x = strtoull(str.c_str(), &test, base);
      UnusedVar(x);
      bool retval = (errno == 0);
      if (strict) {
        retval = retval && (test == str.c_str()+str.length())
                 && (Get_f32() >= 0);
      }
      else { retval = retval && (test != str.c_str()); }
      return retval;
    }

    /// True if the start of the string can be converted to an i32.
    /** @param base The conversion base to test (0 is automatic).
     *  @param strict Return true only if the full string converts.
     */
    inline bool Is_i32(int base=0, bool strict=false) const {
      char *test;
      errno = 0;
      i32 x = strtol(str.c_str(), &test, base);
      UnusedVar(x);
      bool retval = (errno == 0);
      if (strict) { retval = retval && (test == str.c_str()+str.length()); }
      else { retval = retval && (test != str.c_str()); }
      return retval;
    }

    /// True if the start of the string can be converted to an i64.
    /** @param base The conversion base to test (0 is automatic).
     *  @param strict Return true only if the full string converts.
     */
    inline bool Is_i64(int base=0, bool strict=false) const {
      char *test;
      errno = 0;
      i64 x = strtoll(str.c_str(), &test, base);
      UnusedVar(x);
      bool retval = (errno == 0);
      if (strict) { retval = retval && (test == str.c_str()+str.length()); }
      else { retval = retval && (test != str.c_str()); }
      return retval;
    }


    /// True if the start of the string can be converted as hexadecimal to
    /// a u32.
    /** @param strict Return true only if the full string converts. */
    inline bool Is_hex32(bool strict=false) const {
      return Is_u32(16, strict);
    }
    /// True if the start of the string can be converted as hexadecimal to
    /// a u64.
    /** @param strict Return true only if the full string converts. */
    inline bool Is_hex64(bool strict=false) const {
      return Is_u64(16, strict);
    }

    /// True if the string is a "0", "1", or case insensitive "true",
    /// "false", "T", or "F".
    inline bool Is_bool() const {
      if (str == "0" || str == "1") {
        return true;
      }
      if (size() <= 4) {
        RStr tmp = *this;
        tmp.ToLower();
        if (tmp.str == "true" || tmp.str == "false" ||
            tmp.str == "t" || tmp.str == "f") {
          return true;
        }
      }
      return false;
    }

#ifdef RC_HAVE_QT
    /// Convert the RStr to a QString.
    inline QString ToQString() const {
      return QString::fromStdString(str);
    }
#endif

    /// Convert the RStr to a std::wstring.
    inline std::wstring wstring() {
      std::wstring wstr;
      wstr.assign(str.begin(), str.end());
      return wstr;
    }

#ifdef WIN32
    /// For Win32, provides an LPCSTR to the string
    inline const char* ToLPCSTR() const {
      return c_str();
    }

    /// For Win32, provides an LPCWSTR to the string
    /** Return type implicitly casts to const whcar_t*, or with Get().
     */
    inline HoldRelated<std::wstring, const wchar_t*> ToLPCWSTR() {
      HoldRelated<std::wstring, const wchar_t*> hr(wstring());
      hr.Set(hr.Held().c_str());
      return hr;
    }
#ifdef UNICODE
    inline HoldRelated<std::wstring, const wchar_t*> ToLPCTSTR() {
      return ToLPCWSTR();
    }
#else
    /// For Win32, provides an LPCTSTR to the string.
    /** See ToLPCWSTR() for the return type if UNICODE is defined.
     */
    inline const char* ToLPCTSTR() {
      return ToLPCSTR();
    }
#endif
#endif


    /// Provides raw access to the std::string this RStr wraps.
    inline std::string& Raw() { return str; }

    /// Provides const raw access to the std::string this RStr wraps.
    inline const std::string& Raw() const { return str; }

    /// Returns a Data1D<char> corresponding to the character data in the
    /// string.
    inline Data1D<char> ToData() const {
      char *copy = new char[size()];
      UnusedVar(strncpy(copy, c_str(), size()));
      return Data1D<char>(size(), copy, true);
    }

    /// @} Basic types


    /// \name Bounds-checking
    /// @{

    /// True if the index pos is in bounds.
    inline bool Check(size_t pos) const {
      return (pos < str.length());
    }

    /// True if the index pos is in bounds, permitting the null.
    inline bool CheckPlus(size_t pos) const {
      return (pos <= str.length());
    }

    /// Throws ErrorMsgBounds if the index pos is out of bounds.
    inline void Assert(size_t pos) const {
      if ( ! Check(pos) ) {
        Throw_RC_Type(Bounds, "Out of bounds");
      }
    }

    /// Throws ErrorMsgBounds if the index pos is out of bounds, permitting
    /// the null.
    inline void AssertPlus(size_t pos) const {
      if ( ! CheckPlus(pos) ) {
        Throw_RC_Type(Bounds, "Out of bounds");
      }
    }


    /// Throws ErrorMsgNull if ptr is null.
    inline void Assert(const char* ptr) const {
      if (ptr == NULL) {
        Throw_RC_Type(Null, "NULL pointer");
      }
    }

    /// @}  Bounds-checking


    /// \name Text alignment and arrangement functions
    /// @{

    /// If the length is less than pad_to, add pad_width to the left until it
    /// reaches that size.
    inline RStr& PadLeft(const size_t pad_to, const char pad_with=' ') {
      if (pad_to > length()) {
        RStr padded;
        padded = RStr(pad_to-length(), pad_with);
        padded += *this;
        *this = padded;
      }
      return *this;
    }


    /// If the length is less than pad_to, add pad_width to the right until it
    /// reaches that size.
    inline RStr& PadRight(const size_t pad_to, const char pad_with=' ') {
      if (pad_to > length()) {
        *this += RStr(pad_to-length(), pad_with);
      }
      return *this;
    }


    /// If the length is less than pad_to, add pad_width evenly to the right
    /// and left until it reaches that size.
    inline RStr& PadCenter(const size_t pad_to, const char pad_with=' ') {
      if (pad_to > length()) {
        size_t diff = pad_to - length();
        size_t half = diff/2;
        PadLeft(pad_to - diff + half, pad_with);
        PadRight(pad_to, pad_with);
      }
      return *this;
    }


    /// Wraps each line in this RStr to be no longer than width.
    inline Data1D<RStr> Wrap(const size_t width) {
      Data1D<RStr> wrapped;
      size_t pos = 0;

      do {
        size_t this_width, end_pos;
        size_t search_end = pos + width;
        if (search_end > length()) { search_end = length(); }
        for(end_pos=pos; end_pos < search_end; end_pos++) {
          if (str[end_pos] == '\n') { break; }
        }
        if (end_pos < search_end) {
          // newline found
          this_width = end_pos - pos;
          if (end_pos > 0 && str[end_pos-1] == '\r') {
            this_width--;
          }
          wrapped += substr(pos, this_width);
          pos = end_pos+1;
        }
        else {
          wrapped += substr(pos, width);
          pos += width;
        }
      } while(pos < length());

      return wrapped;
    }


    /// Wraps each line in this RStr to be no longer than width, but tries
    /// to keep whole words together.
    inline Data1D<RStr> WordWrap(const size_t width) {
      Data1D<RStr> wrapped;
      size_t pos = 0;
      size_t this_width;

      do {
        size_t end_pos;
        size_t search_end = pos + width;
        if (search_end > length()) {
          wrapped += substr(pos);
          break;
        }
        for(end_pos=pos; end_pos < search_end; end_pos++) {
          if (str[end_pos] == '\n') { break; }
        }
        if (end_pos < search_end) {
          // newline found
          this_width = end_pos - pos;
          if (end_pos > 0 && str[end_pos-1] == '\r') {
            this_width--;
          }
          wrapped += substr(pos, this_width);
          pos = end_pos+1;
        }
        else {
          // No newline found.  Check for space.
          size_t try_pos = pos+width;
          try_pos = (try_pos < length()) ? try_pos : length()-1;
          end_pos = find_last_of(" ", try_pos);
          if (end_pos == npos || end_pos < pos) {
            this_width = width;
            end_pos = pos+width;
          }
          else {
            this_width = end_pos - pos;
            end_pos = find_first_not_of(" ", end_pos);
          }
          wrapped += substr(pos, this_width);
          pos = end_pos;
        }
      } while(pos < length());

      return wrapped;
    }

    /// @}  alignment/arrangement


#ifdef RC_RSTR_REGEX
    /// \name Regular expression support
    /// These use C++11 regex by default, but can fall back on boost regex if
    /// C++11 is missing and RC_HAVE_BOOST is defined in RCconfig.h.
    /// @{

    /// True if the regular expression reg matches this string.
    inline bool Match(const RC_REGEX_NS::regex& reg) {
      return RC_REGEX_NS::regex_search(str, reg);
    }
    /// True if the regular expression regstr matches this string.
    inline bool Match(const RStr& regstr) {
      RC_REGEX_NS::regex reg(regstr.c_str());
      return Match(reg);
    }


    /// True if the regular expression reg matches this string, and returns
    /// the matches in matches.
    inline bool Match(const RC_REGEX_NS::regex& reg, Data1D<RStr>& matches) {
      bool result;
      RStr tmp;

      RC_REGEX_NS::cmatch matcharr;

      result = RC_REGEX_NS::regex_search(str.c_str(), matcharr, reg);

      matches.Resize(0);

      if (result) {
        for (size_t i=1; i<matcharr.size(); i++) {
          tmp.assign(matcharr[i].first, matcharr[i].second);
          matches.Append(tmp);
        }
      }

      return result;
    }
    /// True if the regular expression regstr matches this string, and returns
    /// the matches in matches.
    inline bool Match(const RStr& regstr, Data1D<RStr>& matches) {
      RC_REGEX_NS::regex reg(regstr.c_str());
      return Match(reg, matches);
    }


    /// Substitutes regular expression reg with subst in this string.
    inline void Subst(const RC_REGEX_NS::regex& reg, const RStr& subst) {
      str = RC_REGEX_NS::regex_replace(str, reg, subst.str);
    }
    /// Substitutes regular expression regstr with subst in this string.
    inline void Subst(const RStr& regstr, const RStr& subst) {
      RC_REGEX_NS::regex reg(regstr.c_str());
      Subst(reg, subst);
    }
    /// @}  regex
#endif // RC_RSTR_REGEX


    /// \name Text manipulation routines
    /// @{

    /// Returns true if pos or later which matches string s.
    inline bool Contains(const RStr& s, size_t pos=0) const {
      return npos != find(s, pos);
    }

    /// Returns the index after the first instance of string s at pos or
    /// later, or npos if no match.
    inline size_t After(const RStr& s, size_t pos=0) const {
      size_t res = find(s, pos);
      return (res == npos ? npos : res + s.size());
    }

    /// Remove all trailing newline characters, or provided char set.
    inline RStr& Chomp(const RStr& chomp_chars = "\r\n") {
      size_t end;

      if (length() > 0) {
        end = str.find_last_not_of(chomp_chars.str);
        if (end == npos) {
          str.clear();
        }
        else {
          str.resize(end+1);
        }
      }
      return *this;
    }


    /// Remove all leading and trailing whitespace, or provided char set.
    inline RStr& Trim(const RStr& trim_chars = " \t\r\n") {
      size_t start, end;

      if (length() > 0) {
        end = str.find_last_not_of(trim_chars.str);
        if (end == npos) {
          str = "";
        }
        else {
          start = str.find_first_not_of(trim_chars.str);
          str = str.substr(start, end-start+1);
        }
      }
      return *this;
    }


    /// Make sure the string is no longer than max_size.
    inline RStr& Truncate(const size_t max_size) {
      if (max_size < length()) {
        resize(max_size);
      }
      return *this;
    }



    /// Make this RStr lowercase.
    inline RStr& ToLower() {
      size_t i;
      for (i=0; i<str.length(); i++) {
        str[i] = tolower(str[i]);
      }
      return *this;
    }


    /// Make this RStr uppercase.
    inline RStr& ToUpper() {
      size_t i;
      for (i=0; i<str.length(); i++) {
        str[i] = toupper(str[i]);
      }
      return *this;
    }


    /// Return an array of strings split at each instance of c.
    inline Data1D<RStr> Split(char c) const {
      Data1D<RStr> retval;
      size_t start, end;

      start = 0;

      while (start <= str.length()) {
        end = str.find_first_of(c, start);
        if (end == npos) {
          retval.Append(str.substr(start, npos));
          break;
        }
        else {
          retval.Append(str.substr(start, end-start));
        }
        start = end+1;
      }

      return retval;
    }


    /// Return an array of 2 strings split at the first character from
    /// dividers.
    inline Data1D<RStr> SplitFirst(const RStr& dividers) const {
      Data1D<RStr> retval(2);
      size_t mid;

      mid = str.find_first_of(dividers.str, 0);
      if (mid == npos) {
        retval[0] = str.substr(0, npos);
      }
      else {
        retval[0] = str.substr(0, mid);
        if ((mid+1) < str.length()) {
          retval[1] = str.substr(mid+1, npos);
        }
      }

      return retval;
    }


    /// Return an array of 2 strings split at the last character from dividers.
    inline Data1D<RStr> SplitLast(const RStr& dividers) const {
      Data1D<RStr> retval(2);
      size_t mid;

      mid = str.find_last_of(dividers.str, npos);
      if (mid == npos) {
        retval[0] = str.substr(0, npos);
      }
      else {
        retval[0] = str.substr(0, mid);
        if ((mid+1) < str.length()) {
          retval[1] = str.substr(mid+1, npos);
        }
      }

      return retval;
    }


    /// Return an array of strings split by any characters in dividers.
    /** Always returns at least one element.
     */
    inline Data1D<RStr> SplitAny(const RStr& dividers) const {
      Data1D<RStr> retval;
      size_t start, end;

      end = 0;

      while (end < str.length()) {
        start = str.find_first_not_of(dividers.str, end);
        if (start == npos) {
          break;
        }
        end = str.find_first_of(dividers.str, start);
        if (end == npos) {
          end = str.length();
        }
        retval.Append(str.substr(start, end-start));
      }

      if (retval.size() == 0) { retval += ""; }
      return retval;
    }


    /// Return an array of one or more whitespace separated words.
    inline Data1D<RStr> SplitWords() const {
      return SplitAny(" \t\r\n");
    }


    /// Takes a Data1D<T> array and joins them as one string with spacer
    /// between each element while applying the function to each element.
    /** Note:  The default func converts each element to a string with its
     *  RStr(T) constructor, and thus only works on types for which a
     *  constructor exists.
     */
    template <class T>
    static inline RStr Join(const Data1D<T>& str_arr,
                            const RStr& spacer="",
                            RStr(*func)(const T&)=nullptr) {
      RStr retval;
      size_t i;

      if (str_arr.size() == 0) {
        retval.str = "";
        return retval;
      }

      if (func == nullptr) {
        func = [](const T& in) { return RStr(in); };
      }

      retval = func(str_arr[0]);

      for (i=1; i<str_arr.size(); i++) {
        retval += spacer;
        retval += func(str_arr[i]);
      }

      return retval;
    }

    /// @} Text manipulation


    /// \name Metrics
    /// @{

    /// Compute the Levenshtein distance to other.
    inline size_t Distance(const RStr& other) const {
      size_t thislen = length();
      size_t otherlen = other.length();

      Data2D<size_t> count(thislen+1, 2);

      u32 prev_ind = 0;
      u32 curr_ind = 1;
      size_t i, j;
      size_t subst_cnt, insert_cnt;
      for (i=0; i<count.size1(); i++) {
        count[0][i] = i;
      }

      for (i=0; i<otherlen; i++) {
        count[curr_ind][0] = i+1;
        char ch = other[i];
        for (j=0; j<thislen; j++) {
          insert_cnt = std::min(1+count[curr_ind][j], 1+count[prev_ind][1+j]);
          subst_cnt = count[prev_ind][j] + ((str[j] == ch) ? 0 : 1);
          count[curr_ind][j+1] = std::min(insert_cnt, subst_cnt);
        }

        prev_ind ^= 1;
        curr_ind ^= 1;
      }

      return count[prev_ind][thislen];
    }

    /// Determine the length of the contents treated as a UTF-8 string.
    /** Throws ErrorMsg "Invalid UTF-8" if the string cannot be parsed as a
     *  UTF-8 string.
     */
    inline size_t Length8() const {
      size_t len = 0;
      for (size_t i=0; i<str.length(); i++) {
        u32 cnt = 0;
        u8 ch = str[i];
        while (0x80 & ch) {
          cnt++;
          ch = ch << 1;
        }
        if (cnt == 1) {
          Throw_RC_Error("Invalid UTF-8");
        }
        for (u32 parts=1; (parts<cnt); parts++) {
          i++;
          if (i >= str.length() || (((u8(str[i])) & 0xC0)!=0x80)) {
            Throw_RC_Error("Invalid UTF-8");
          }
        }
        len++;
      }
      return len;
    }

    /// @}  Metrics


    /// \name Interacting with standard formats
    /// @{


    /// Return comma-separated values with whitespace trimmed.
    inline Data1D<RStr> SplitCSV(char divider=',') const {
      Data1D<RStr> retval;
      size_t i;

      retval = Split(divider);

      for (i=0; i<retval.size(); i++) {
        retval[i].Trim();
      }

      return retval;
    }


    /// Makes a comma-separated values string out of a Data1D<T> arr.
    /** The type must have an RStr(T) constructor.  The elements are
     *  separated by ", ".
     */
    template <class T>
    static inline RStr MakeCSV(const Data1D<T>& arr,
        const RStr& divider=", ") {
      RStr retval;
      size_t i;

      if (arr.size() == 0) {
        retval.str = "";
        return retval;
      }

      retval = RStr(arr[0]);
      
      for (i=1; i<arr.size(); i++) {
        retval += divider;
        retval += RStr(arr[i]);
      }

      return retval;
    }

    /// Makes a comma-separated values string out of a Data2D<T> arr.
    /** The elements are separated by ", " in the first dimension, and
     *  followed by "\n" in the second dimension.
     *  @see MakeCSV
     */
    template <class T>
    static inline RStr MakeCSV(const Data2D<T>& arr,
        const RStr& divider1=", ", const RStr& divider2="\n") {
      RStr retval;
      size_t i;

      for (i=0; i<arr.size2(); i++) {
        retval += MakeCSV(arr[i], divider1);
        retval += divider2;
      }

      return retval;
    }

    /// Makes a comma-separated values string out of a Data3D<T> arr.
    /** The elements are separated by ", " in the first dimension,
     *  followed by "\n" in the second dimension, and separated by an
     *  additional "\n" in the third dimension.
     *  @see MakeCSV
     */
    template <class T>
    static inline RStr MakeCSV(const Data3D<T>& arr,
        const RStr& divider1=", ", const RStr& divider2="\n",
        const RStr& divider3="\n") {
      RStr retval;
      size_t i;

      if (arr.size3() == 0) {
        retval.str = "";
        return retval;
      }

      retval = MakeCSV(arr[0], divider1, divider2);

      for (i=1; i<arr.size3(); i++) {
        retval += divider3;
        retval += MakeCSV(arr[i], divider1, divider2);
      }

      return retval;
    }


    /// Treat this string as ISO-8859-1 and return a UTF8 string.
    /** This supports the windows-1252 extension as required by HTML5. */
    inline RStr ISOtoUTF8() const {
      RStr utf8;
      for (size_t i=0; i<str.length(); i++) {
        if ((u8(str[i])) < 0x80) {
          utf8 += str[i];
        }
        else {
          if ((u8(str[i])) < 0xA0) {
            // Windows-1252 extension
            u8 win1252[4*0x20] = {0xe2,0x82,0xac,0,0x81,0,0,0,
              0xe2,0x80,0x9a,0,0xc6,0x92,0,0,0xe2,0x80,0x9e,0,
              0xe2,0x80,0xa6,0,0xe2,0x80,0xa0,0,0xe2,0x80,0xa1,0,
              0xcb,0x86,0,0,0xe2,0x80,0xb0,0,0xc5,0xa0,0,0,
              0xe2,0x80,0xb9,0,0xc5,0x92,0,0,0x8d,0,0,0,0xc5,0xbd,0,0,
              0x8f,0,0,0,0x90,0,0,0,0xe2,0x80,0x98,0,0xe2,0x80,0x99,0,
              0xe2,0x80,0x9c,0,0xe2,0x80,0x9d,0,0xe2,0x80,0xa2,0,
              0xe2,0x80,0x93,0,0xe2,0x80,0x94,0,0xcb,0x9c,0,0,
              0xe2,0x84,0xa2,0,0xc5,0xa1,0,0,0xe2,0x80,0xba,0,
              0xc5,0x93,0,0,0x9d,0,0,0,0xc5,0xbe,0,0,0xc5,0xb8,0,0};
            utf8 += RStr(reinterpret_cast<char*>(
                  &win1252[4*(u8(str[i])-0x80)]));
          }
          else {
            // Standard ISO-8859-1 extension
            utf8 += (str[i] & 0x40) ? 0xc3 : 0xc2;
            utf8 += str[i] & 0xbf;
          }
        }
      }
      return utf8;
    }
    

    /// Treat the contents as a UTF-8 string, and return corresponding
    /// UTF-32 data.
    /** Throws ErrorMsg "Invalid UTF-8" if the string cannot be parsed as a
     *  UTF-8 string.
     */
    inline Data1D<u32> UTF8toUTF32() const {
      Data1D<u32> utf32;  utf32.Reserve(Length8());  // Validates
      for (size_t i=0; i<str.length(); i++) {
        u32 cnt = 0;
        u8 ch = str[i];
        while (0x80 & ch) {
          cnt++;
          ch = ch << 1;
        }
        u32 val = (u8(str[i])) & (0x7F >> cnt);
        for (u32 parts=1; (parts<cnt) && ((i+1)<str.length()); parts++) {
          i++;
          val = (val << 6) + ((u8(str[i])) & 0x3F);
        }
        utf32 += val;
      }
      return utf32;
    }


    protected:
    /// @cond PROTECTED
    enum B64Type { B64STANDARD, B64URL };
    static inline void B64DoWrap(size_t wrap, RStr& encoded, size_t& s_cnt) {
      if (wrap && s_cnt && (s_cnt % wrap == 0)) {
        encoded += NewLine();
      }
      s_cnt++;
    }
    static inline void B64Error() { Throw_RC_Error("Invalid Base64 Data"); }
    static inline void B64DoEnc(u32& val, RStr& encoded, B64Type type) {
      const char b64enc[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "abcdefghijklmnopqrstuvwxyz"
                              "0123456789+/";
      const char b64encurl[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz"
                                 "0123456789-_";
      if (type == B64STANDARD) {
        encoded += b64enc[val >> 18];
      }
      else {
        encoded += b64encurl[val >> 18];
      }
      val = (val << 6) & 0x00FFFFFF;
    }
    static inline void B64DoDec(u8 inp, u32& val) {
      const char b64dec[257] =
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!n!n!odefghijklm!!!!!!"
        "!0123456789:;<=>?@ABCDEFGHI!!!!o!JKLMNOPQRSTUVWXYZ[\\]^_`abc!!!!!"
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
      u32 x = b64dec[inp];
      if (x == '!') { B64Error(); }
      val = (val << 6) + (x - '0');
    }
    static inline void B64DoPack(u32& val, Data1D<u8>& decoded) {
      decoded += val >> 16;
      val = (val << 8) & 0x00FFFFFF;
    }

    static inline RStr ToBase64Type(const Data1D<u8>& input, size_t wrap,
                                    B64Type type) {
      RStr encoded;

      size_t i;
      size_t s_cnt = 0;
      u32 val = 0;
      for(i=0; i<input.size(); i++) {
        val = (val << 8) + input[i];
        if ((i % 3) == 2) {
          for (u32 s=0; s<4; s++) {
            B64DoWrap(wrap, encoded, s_cnt);
            B64DoEnc(val, encoded, type);
          }
        }
      }

      u32 resid = i%3;
      if (resid > 0) { resid++; }
      u32 pad = (4-resid) & 3;
      val <<= 8*pad;

      for (u32 s=0; s<resid; s++) {
        B64DoWrap(wrap, encoded, s_cnt);
        B64DoEnc(val, encoded, type);
      }
      if (type == B64STANDARD) {
        for (u32 s=0; s<pad; s++) {
          B64DoWrap(wrap, encoded, s_cnt);
          encoded += "=";
        }
      }

      return encoded;
    }

    /// @endcond
    public:

    /// Encodes input to standard base64 in compliance with RFC 4648.
    static inline RStr ToBase64(const Data1D<u8>& input, size_t wrap=0) {
      return ToBase64Type(input, wrap, B64STANDARD);
    }

    /// Encodes input to base64url in compliance with RFC 4648, safe for url
    /// and filename use.
    static inline RStr ToBase64URL(const Data1D<u8>& input, size_t wrap=0) {
      return ToBase64Type(input, wrap, B64URL);
    }

    /// Decodes input from base64 in compliance with RFC 4648, standard or
    /// URL encoding.
    static inline Data1D<u8> FromBase64(const RStr& input) {
      Data1D<u8> decoded;
      size_t icnt = 0;
      u32 val = 0;
      for (size_t i=0; i<input.size(); i++) {
        if (input[i] == '\n' || input[i] == '\r') { continue; }
        if (input[i] == '=') { break; }
        B64DoDec(input[i], val);
        if ((icnt&3) == 3) {
          for (size_t d=0; d<3; d++) {
            B64DoPack(val, decoded);
          }
        }
        icnt++;
      }
      u32 resid = 0;
      switch(icnt&3) {
        case 0:  return decoded;
        case 2:  resid = 1;  break;
        case 3:  resid = 2;  break;
        default:  B64Error();
      }
      val <<= (4-(icnt&3))*6;
      for (u32 d=0; d<resid; d++) {
        B64DoPack(val, decoded);
      }
      return decoded;
    }


    /// Converts hexstr to a byte array, like "a4c208" -> { 0xa4, 0xc2, 0x08 }
    static inline Data1D<u8> FromHexStr(const RStr& hexstr) {
      Data1D<u8> bytes;
      RStr parse;
      for (size_t i=0; i<hexstr.size(); i++) {
        parse += hexstr[i];
        if (i&1) {
          if (!parse.Is_hex32()) { Throw_RC_Error("Invalid hex string"); }
          bytes += parse.Get_hex32();
          parse.clear();
        }
      }
      return bytes;
    }


    /// Converts rawdata to a hex string, like { 0xa4, 0xc2, 0x08 } -> "a4c208"
    template<class T>
    static inline RStr ToHexStr(const Data1D<T>& rawdata) {
      RStr hexstr;
      u8 *ptr = reinterpret_cast<u8*>(rawdata.Raw());
      size_t len = rawdata.size() * sizeof(T);
      for (size_t i=0; i<len; i++) {
        hexstr += RStr(ptr[i], HEX);
      }
      return hexstr;
    }


    /// Converts standard arguments to "int main" into a Data1D<RStr>.
    /** Index 0 contains the program name, corresponding to argv[0],
     *  and index 1 on up are the parameters.  Thus size()-1 of the returned
     *  value is the number of command-line parameters received.
     *  If called with no arguments, the previous arglist is returned.
     */
    static inline Data1D<RStr> Args(int argc=0, char *argv[]=NULL) {
      static Data1D<RStr> arglist;

      if (argc==0) {
        return arglist;
      }

      if (argv == NULL) {
        Throw_RC_Type(Null, "NULL pointer");
      }

      arglist.Resize(argc);
      
      int i;
      for (i=0; i<argc; i++) {
        arglist[i] = argv[i];
      }

      return arglist;
    }


    /// Returns a thread-safe error string corresponding to err_num.
    static inline RStr Errno(int err_num) {
      const size_t bufsize = 1024;
      char buf[bufsize];  buf[0] = '\0';
#ifdef WIN32
      UnusedVar(strerror_s(buf, bufsize, err_num));
#else
      UnusedVar(strerror_r(err_num, buf, bufsize));
#endif
      return RStr(buf);
    }
    /// Returns a thread-safe error string corresponding to errno.
    static inline RStr Errno() { return Errno(errno); }

    /// @}  Standard formats.


    /// \name Stream interfacing
    /// @{

    /// Set the string equal to the next line from istream in, up to
    /// character delim.
    /** Usage example:  RStr line; while(line.GetLine()) {
     *  cout << line << endl; }
     *  Note:  Due to buffering, it can be more efficient to use:
     *  FileRead fr(stdin); while(fr.Get(line)) { ... }  But GetLine is
     *  the better choice to avoid reading ahead, or for interactive input.
     *  @return True if input was received.
     */
    inline bool GetLine(std::istream& in = std::cin, char delim='\n') {
      if (in.eof() || (!in.good())) { return false; }
      std::getline(in, str, delim);

      // Transparently handle windows style newlines.
      size_t lastch = str.size()-1;
      if (delim=='\n' && str.size()>0 && str[lastch]=='\r') {
        str.erase(lastch);
      }

      return !in.fail() && (in.good() || in.eof());
    }

    friend std::istream& operator>> (std::istream &in, RStr &rstr);
    friend std::ostream& operator<< (std::ostream &out, const RStr &rstr);
    /// @}

    /// \name Constants
    /// @{
    
    /// The largest possible value of size_t.
    static const size_t npos = -1;
    /// Provides all the standard whitespace characters.
    static inline const RStr Whitespace() { return " \t\r\n"; }
    /// Provides the OS-specific newline string.
    static inline const RStr NewLine() {
#ifdef WIN32
      return "\r\n";
#else
      return "\n";
#endif
    }

    /// @}


    protected:
    /// @cond PROTECTED

    std::string str;
    /// @endcond
  };


  /// Concatenates two RStr'ings.
  inline RStr operator+ (const RStr &lhs, const RStr &rhs) {
    RStr retval = lhs;
    retval += rhs;
    return retval;
  }


  /// Input text from a std::istream (e.g., std::cin) into an RStr.
  inline std::istream& operator>> (std::istream &in, RStr &rstr) {
    return (in >> rstr.str);
  }

  /// Output text from an RStr to a std::ostream (e.g., std::cout).
  inline std::ostream& operator<< (std::ostream &out, const RStr &rstr) {
    return (out << rstr.str);
  }


  /// Swaps two RStr'ings.
  inline void swap(RStr& lhs, RStr& rhs) {
    lhs.swap(rhs);
  }


  /// Sets str equal to one line from the input stream, up to end of file or
  /// the delim, which is discarded.
  inline std::istream& getline(std::istream& is, RStr& str, char delim='\n') {
    return getline(is, str.Raw(), delim);
  }

  
  /// True if lhs equals rhs.
  inline bool operator== (const RStr& lhs, const RStr& rhs) {
    return (lhs.compare(rhs) == 0);
  }

  /// True if lhs does not equal rhs.
  inline bool operator!= (const RStr& lhs, const RStr& rhs) {
    return (lhs.compare(rhs) != 0);
  }

  /// True if lhs is less than rhs.
  inline bool operator< (const RStr& lhs, const RStr& rhs) {
    return (lhs.compare(rhs) < 0);
  }

  /// True if lhs is greater than rhs.
  inline bool operator> (const RStr& lhs, const RStr& rhs) {
    return (lhs.compare(rhs) > 0);
  }

  /// True if lhs is less than or equal to rhs.
  inline bool operator<= (const RStr& lhs, const RStr& rhs) {
    return (lhs.compare(rhs) <= 0);
  }

  /// True if lhs is greater than or equal to rhs.
  inline bool operator>= (const RStr& lhs, const RStr& rhs) {
    return (lhs.compare(rhs) >= 0);
  }

#ifdef RC_RSTR_REGEX
  /// A regular expression component to match a floating point number.
  const RStr REG_FLT("[-+]?[0-9]*\\.?[0-9]+[eE][-+]?[0-9]+|[-+]?[0-9]*\\.?[0-9]+");
  /// A regular expression component to match and return a floating point
  /// number.
  const RStr REG_FLTP("([-+]?[0-9]*\\.?[0-9]+[eE][-+]?[0-9]+|[-+]?[0-9]*\\.?[0-9]+)");
#endif

  /// Provides number-based singular/plural string management.
  /** Example usage:  PluralStr cat("cat", "cats");  cout << cat.Count(5)
   *  << " plus one " << cat.For(1) << " is 6 " << cat.For(6);  Produces:
   *  "5 cats plus one cat is 6 cats"
   */
  class PluralStr {
    public:
    /// Initialize with the singular and plural version of a string.
    inline PluralStr(const RStr& singular, const RStr& plural)
      : singular(singular), plural(plural) { }
    /// Provides the singular form if arg is 1, or the plural version
    /// otherwise.
    template<class T>
    inline RStr For(T arg) const {
      return (arg==1) ? singular : plural;
    }
    /// For any number arg, provides the number, followed by a space,
    /// followed by the correct singular or plural form.
    template<class T>
    inline RStr Count(T arg) const {
      return RStr(arg) + " " + For(arg);
    }
    protected:
    /// @cond PROTECTED
    RStr singular, plural;
    /// @endcond
  };
}

/// @cond PROTECTED
#ifdef CPP11
#include <functional>
namespace std {
  template<> struct hash<RC::RStr> {
    std::size_t operator()(const RC::RStr& s) const {
      return h(s.Raw());
    }
    private:
    std::hash<std::string> h;
  };
}
#endif
/// @endcond

#endif // RC_RSTR_H

