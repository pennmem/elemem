/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2015, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Errors.h
/// Provides informative exception handling.
/////////////////////////////////////////////////////////////////////

#ifndef RC_ERRORS_H
#define RC_ERRORS_H

#include "RCconfig.h"
#include "Types.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>


#ifndef RC_NO_STACKTRACE
#ifdef unix
#include <execinfo.h>
#elif defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#include <dbghelp.h>
#include <tchar.h>
#endif
#endif // RC_NO_STACKTRACE


namespace RC {
  /// The maximum size of the char array returned by ErrorMsg::what() and
  /// ErrorMsg::GetError(), including the null.
  const size_t ErrorMsg_text_size = 4095;
  /// @cond UNDOC
  const size_t ErrorMsg_text_bufsize = 4096;
  const size_t ErrorMsg_what_bufsize = 4096+1024;
  /// @endcond
  /// The maximum size of the char array returned by ErrorMsg::GetType(),
  /// including the null.
  const size_t ErrorMsg_type_bufsize = 256;

  /// An exception class that records where the exception was thrown and
  /// provides a stack trace.
  /** Use with the macro Throw_RC_Error("Reason"); to automatically pick up
   *  the source filename and line number.  This exception should be used
   *  liberally any time an error would be informative for resolving a bug.
   *
   *  Stack tracing is implemented for Linux and Windows.  For Linux with g++
   *  or clang compile with -rdynamic for full symbols in the stack trace
   *  output from what().  For Windows symbols cannot be automatically provided
   *  with mingw, but the addresses provided in what() can be resolved by
   *  providing them as stdin to "addr2line -pfe myprogram.exe" for any program
   *  compiled with -ggdb.
   */
  class ErrorMsg : virtual public std::exception {
    size_t AmountWritten(size_t size, size_t written) {
      return (size <= written) ? size-1 : written;
    }
    // Returns amount actually written.
    /// @cond UNDOC
    #define R_Safe_snprintf(str, size, ...) \
      AmountWritten(size, size_t(snprintf(str, size, __VA_ARGS__)));
    /// @endcond

    public:

    /// The default constructor.
    /** Use the convenience macro Throw_RC_Error("Reason"); which uses
     *  __FILE__ and __LINE__ to extract the location of the throw.
     *  @param new_err_msg The reason for the exception.
     *  @param filename The source code file that the exception was triggered
     *  in.
     *  @param line_number The source code line number that the exception was
     *  triggered on.
     */
    ErrorMsg(const char* new_err_msg=NULL, const char* filename=NULL,
             int line_number = 0) RC_NOEXCEPT {
      size_t offset = 0;
      char stacktrace_txt[ErrorMsg_text_bufsize] = {0};

      type_msg[0] = '\0';
      if (new_err_msg == NULL) {
        err_msg[0] = '\0';
      }
      else {
        strncpy(err_msg, new_err_msg, ErrorMsg_text_size);
        err_msg[ErrorMsg_text_bufsize-1] = '\0';
      }

#ifndef RC_NO_STACKTRACE
      // Get the stack trace if we can.
      int backtrace_cnt = 0;
      const int backtrace_size = 128;
      void *bt_buffer[backtrace_size];
#ifdef unix
      char **backtrace_symb;
      backtrace_cnt = backtrace(bt_buffer, backtrace_size);
      backtrace_symb = backtrace_symbols(bt_buffer, backtrace_cnt);

      if (backtrace_symb != NULL) {
        for (int i=0; i<backtrace_cnt; i++) {
          offset += R_Safe_snprintf(stacktrace_txt+offset,
            ErrorMsg_text_size-offset, "\n\t%s",
            backtrace_symb[backtrace_cnt-i-1]);
        }
        offset += R_Safe_snprintf(stacktrace_txt+offset,
          ErrorMsg_text_size-offset, "\n");

        SafeFree(backtrace_symb);
      }

// end unix
#elif defined(WIN32)
      RC_DYNAMIC_LOAD_FUNC_RAW(SymInitialize, TEXT("Dbghelp.dll"));
      RC_DYNAMIC_LOAD_FUNC_RAW(SymFromAddr, TEXT("Dbghelp.dll"));
      RC_DYNAMIC_LOAD_FUNC_RAW(SymCleanup, TEXT("Dbghelp.dll"));

      if (SymInitialize && SymFromAddr && SymCleanup) {
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, true);

        // Allocate this variable length SYMBOL_INFO structure on stack.
        const size_t symbolnamelen = 255;
        char symboldata[sizeof(SYMBOL_INFO)+(symbolnamelen+1)];
        SYMBOL_INFO *symbol = reinterpret_cast<SYMBOL_INFO*>(symboldata);
        symbol->MaxNameLen = symbolnamelen;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        backtrace_cnt = CaptureStackBackTrace(0, backtrace_size,
                                              bt_buffer, NULL);

        for(int i=0; i<backtrace_cnt; i++) {
          offset += R_Safe_snprintf(stacktrace_txt+offset,
            ErrorMsg_text_size-offset, "\r\n\t0x%lx : ",
              static_cast<long unsigned int>(size_t(bt_buffer[i])));
          if (SymFromAddr(process, DWORD64(bt_buffer[i]), 0, symbol)) {
            offset += R_Safe_snprintf(stacktrace_txt+offset,
              ErrorMsg_text_size-offset, "%s [0x%lx]", symbol->Name,
              static_cast<long unsigned int>(size_t(symbol->Address)));
          }
          else {
            offset += R_Safe_snprintf(stacktrace_txt+offset,
              ErrorMsg_text_size-offset, "_ [0x0000]");
          }
        }
        offset += R_Safe_snprintf(stacktrace_txt+offset,
            ErrorMsg_text_size-offset, "\r\n");

        SymCleanup(process);
      }
#endif // WIN32
#endif // RC_NO_STACKTRACE

      int written = 0;
      if (filename != NULL && filename[0] != '\0') {
        written = snprintf(what_msg, ErrorMsg_what_bufsize,
          "%s, %s, line %d%s", err_msg, filename, line_number,
          stacktrace_txt);
      }
      else {
        written = snprintf(what_msg, ErrorMsg_what_bufsize, "%s%s", err_msg,
          stacktrace_txt);
      }
      if (written >= int(ErrorMsg_what_bufsize)) {
        for (size_t i=ErrorMsg_what_bufsize-4; i<ErrorMsg_what_bufsize-1;
             i++) {
          what_msg[i] = '.';
        }
        what_msg[ErrorMsg_what_bufsize-1] = '\0';
      }
    }

    #undef R_Safe_snprintf


    /// Provides the reason given for the error.
    /** @return The reason for the exception being thrown.
     */
    virtual const char* GetError() const RC_NOEXCEPT {
      return err_msg;
    }


    /// Provides the type of the exception if this is a subclass.
    /** An empty string is returned for the base class.
     *  @return The exception type defined in a subclass.
     */
    virtual const char* GetType() const RC_NOEXCEPT {
      return type_msg;
    }


    /// Return true if the the reason for the exception matches test_err.
    /** @param test_err An error message to compare with the reason given
     *  in the constructor.
     *  @return True if the error messages are identical.
     */
    virtual bool IsError(const char* test_err) const RC_NOEXCEPT {
      return (0 == strncmp(err_msg, test_err, ErrorMsg_text_bufsize));
    }


    /// Returns a full descriptive error message with reason, source location,
    /// and stack trace.
    /** Note:  The stack trace is only available if on a supported system,
     *  and if the RC_NO_STACKTRACE option was not given in RCconfig.h
     */
    virtual const char* what() const RC_NOEXCEPT {
      return (what_msg);
    }


    private:
    template<class T>
    inline void SafeFree(T*& p) RC_NOEXCEPT {
      free(p);
      p = NULL;
    }


    char err_msg[ErrorMsg_text_bufsize];
    char what_msg[ErrorMsg_what_bufsize];
    protected:
    /// @cond PROTECTED
    char type_msg[ErrorMsg_type_bufsize];
    /// @endcond
  };

/// \def RC_MAKE_ERROR_TYPE
/// Creates new RC::ErrorMsg subtypes.
/** Use as:  "namespace RC { RC_MAKE_ERROR_TYPE(NewType) }" to make
 *  RC::ErrorMsgNewType
 */
#define RC_MAKE_ERROR_TYPE(Type) \
  class ErrorMsg##Type : virtual public RC::ErrorMsg {\
    public:\
    /** \brief The default constructor.  */ \
    /** Use the convenience macro Throw_RC_Type(Type, "Reason"); */ \
    ErrorMsg##Type(const char* new_err_msg, const char* filename = "",\
                   int line_number = 0)\
      : RC::ErrorMsg(new_err_msg, filename, line_number) {\
      snprintf(type_msg, ErrorMsg_type_bufsize, #Type);\
    }\
  };
      

  /// A subtype of RC::ErrorMsg for Fatal errors.
  RC_MAKE_ERROR_TYPE(Fatal);
  /// A subtype of RC::ErrorMsg for Null errors.
  RC_MAKE_ERROR_TYPE(Null);
  /// A subtype of RC::ErrorMsg for Bounds errors.
  RC_MAKE_ERROR_TYPE(Bounds);
  /// A subtype of RC::ErrorMsg for Memory errors.
  RC_MAKE_ERROR_TYPE(Memory);
  /// A subtype of RC::ErrorMsg for Bad Cast errors.
  RC_MAKE_ERROR_TYPE(Cast);
  /// A subtype of RC::ErrorMsg for File related errors.
  RC_MAKE_ERROR_TYPE(File);
  /// A subtype of RC::ErrorMsg for Networking related errors.
  RC_MAKE_ERROR_TYPE(Net);
}


/// \def Throw_RC_Error
/// Use this to throw an RC:ErrorMsg exception.
/** It automatically adds the source file name and line number.
 *  @param err (const char*) The reason for the exception.
 */
#define Throw_RC_Error(err) throw RC::ErrorMsg(err, __FILE__, __LINE__)
/// \def Throw_RC_Type
/// Use this to throw an RC:ErrorMsg subtype exception.
/** It automatically adds the source file name and line number.
 *  @param Type The subtype.  e.g. Throw_RC_Type(Null, "Reason")
 *  for ErrorMsgNull.
 *  @param err (const char*) The reason for the exception.
 */
#define Throw_RC_Type(Type, err) throw RC::ErrorMsg##Type(err, __FILE__, __LINE__)
/// \def Catch_RC_Error
/// Place after a try block to catch RC errors and print the error text.
/** Note:  Exits on ErrorMsgFatal with return value -1. */
#define Catch_RC_Error() catch (RC::ErrorMsgFatal& err) { fprintf(stderr, "Fatal Error:  %s\n", err.what()); exit(-1); }  catch (RC::ErrorMsg& err) { fprintf(stderr, "Error:  %s\n", err.what()); }
/// \def Catch_RC_Error_Exit
/// Place after a try block to catch RC errors, print the error text, and exit.
#define Catch_RC_Error_Exit() catch (RC::ErrorMsg& err) { fprintf(stderr, "Error:  %s\n", err.what()); exit(-1); }


namespace RC {
  /// A static class for catching and throwing segfaults.
  /** Call Segfault::SetHandler() at the beginning of the program, and 
   *  segfaults will result in an ErrorMsgFatal with a descriptive stack
   *  trace.  In gcc -fnon-call-exceptions is required for the exception to
   *  propagate out of this handler.
   *  Note:  This is automatically called with the RC_MAIN { } macro.
   */
  class Segfault {
    public:

    /// The default handler for a segfault.  It throws ErrorMsgFatal.
    static void Handler(int /*sig*/) {
      Throw_RC_Type(Fatal, "Segmentation fault");
    }


    /// Call this to set the segfault handler to Segfault::Handler.
    static void SetHandler() {
#ifdef WIN32
      signal(SIGSEGV, Handler);
#else
      struct sigaction sa;
      memset (&sa, 0, sizeof(sa));
      sa.sa_handler = Handler;
      sigaction(SIGSEGV, &sa, NULL);
#endif
    }
  };
}

#endif // RC_ERRORS_H
