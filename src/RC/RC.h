/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file RC.h
/// The main RC Library include file, which includes all other components.
/////////////////////////////////////////////////////////////////////

#include "RCconfig.h"

#ifndef RC_LIBRARY_H
#define RC_LIBRARY_H

#ifdef WIN32
#ifndef _WIN32_WINNT
/// @cond UNDOC
#define _WIN32_WINNT 0x0501  // XP and later only, for network routines.
/// @endcond
#endif
#include <winsock2.h>  // Supress warnings about include order.
#endif

#include "Macros.h"
#include "Types.h"
#include "Errors.h"
#include "RCBits.h"
#include "Iter.h"
#include "Data1D.h"
#include "Data2D.h"
#include "Data3D.h"
#include "Bitfield.h"
#include "Bitfield2D.h"
#include "Bitfield3D.h"
#include "Ptr.h"
#include "APtr.h"
#include "RevPtr.h"
#include "Tuple.h"
#include "Caller.h"
#include "RStr.h"
#include "RTime.h"
#include "File.h"
#include "RND.h"
#include "Net.h"


/// \def RC_USE
/// Place this shorthand macro after including RC.h to use namespaces RC
/// and std.
#define RC_USE using namespace RC; \
               using namespace std; \
               namespace std {namespace placeholders{}} \
               using namespace std::placeholders;
/// \def RC_MAIN
/// This macro wraps int main so that command line arguments are placed into
/// RC::Data1D<RC::RStr> args.
/** Usually args[0] is the executable name, and 1 through args.size()-1 will
 *  be the command line parameters.  Usage example:  RC_MAIN {
 *  if (args.size()<2) {Help(); return -1;} Code(args[1]); Here(); return 0; }
 */
#define RC_MAIN int RC_main(RC::Data1D<RC::RStr> &args); \
                int main (int argc, char *argv[]) { \
                  RC::Segfault::SetHandler();\
                  RC::Data1D<RC::RStr> args = RC::RStr::Args(argc, argv); \
                  return RC_main(args); \
                } \
                int RC_main(RC::Data1D<RC::RStr> &RC_UNUSED_PARAM(args))

#endif // RC_LIBRARY_H

