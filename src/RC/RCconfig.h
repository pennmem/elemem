/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2016, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file RCconfig.h
/// The version information and configuration settings for RC Lib.
/////////////////////////////////////////////////////////////////////

#ifdef RC_VERSION
#if RC_VERSION != 202403231100ul
#error "Included two different RC library versions"
#endif
#endif
/// \def RC_VERSION
/// The date-time stamped version number for this release.
#define RC_VERSION 202403231100ul

#ifndef RC_CONFIG_H
/// @cond UNDOC
#define RC_CONFIG_H
/// @endcond

#ifndef WIN32
#if defined(_WIN32) || defined(__WIN32__)
#define WIN32
#endif
#endif

#ifndef MACOS
#if defined(__APPLE__) && defined(__MACH__)
#define MACOS
#endif
#endif

#ifndef unix
#if defined(__unix__) || defined(__unix) || defined(linux) || defined(MACOS)
#define unix
#endif
#endif

/// \def CPP11
/// Defined if C++11 features are available.
#if __cplusplus >= 201103L
#define CPP11
#endif

/// \def CPP14
/// Defined if C++14 features are available.
#if __cplusplus >= 201402L
#define CPP14
#endif

/// \def RC_HAVE_QT
/// Define this if Qt is available in this compile. (Default off)
/// \def RC_HAVE_BOOST
/// Define this if Boost is available. (Default off)
/// \def RC_NO_STACKTRACE
/// Define this to disable stacktracing. (Default off) Note that for
/// linux g++ stacktraces, one should use the compiler option -rdynamic.
// option.
#ifdef DOXYGEN_ONLY
#define RC_HAVE_QT
#define RC_HAVE_BOOST
#define RC_NO_STACKTRACE
#endif

//////////////////
// Begin Options
//////////////////

//#define RC_HAVE_QT
//#define RC_HAVE_BOOST
//#define RC_NO_STACKTRACE

#ifdef unix
/// \def RC_HAVE_URAND
/// If defined, a /dev/urandom file is available on the platform.
#define RC_HAVE_URAND
/// \def RC_DEV_URANDOM
/// Set to the full pathname of the /dev/urandom file to use.
#define RC_DEV_URANDOM "/dev/urandom"
#endif

//////////////////
// End Options
//////////////////


#ifndef RC_DEBOUT_STREAM
/// \def RC_DEBOUT_STREAM
/// The output stream to which RC_DEBOUT debugging messages should be sent.
#define RC_DEBOUT_STREAM std::cerr
#endif

#endif // RC_CONFIG_H

