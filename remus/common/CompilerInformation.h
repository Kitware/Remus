//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2012 Sandia Corporation.
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//=============================================================================
#ifndef remus_commmon_CompilerInformation_h
#define remus_commmon_CompilerInformation_h

#if defined(_MSC_VER)
#define REMUS_MSVC

#elif defined(__INTEL_COMPILER)
#define REMUS_ICC

#elif defined(__PGI)
#define REMUS_PGI

#elif defined(__clang__)
//On OSX the intel compiler uses clang as the front end
//On Windows you can specify the clang compiler as front end to msvc
#define REMUS_CLANG

#elif defined(__GNUC__)
//Now that we have gone through all other compilers that report as gcc/clang
//we can safely say we have the 'real' gcc compiler
#define REMUS_GCC
#endif

#if __cplusplus >= 201103L || ( defined(REMUS_MSVC) && _MSC_VER >= 1700  )
#define REMUS_HAVE_CXX_11
#endif

// Define a pair of macros, REMUS_THIRDPARTY_PRE_INCLUDE and REMUS_THIRDPARTY_POST_INCLUDE,
// that should be wrapped around any #include for a boost  header file. Mostly
// this is used to set pragmas that disable warnings that Remus checks for
// but boost does not.
#if (defined(REMUS_GCC) || defined(REMUS_CLANG))

#define REMUS_THIRDPARTY_GCC_WARNING_PRAGMAS \
  _Pragma("GCC diagnostic ignored \"-Wconversion\"") \
  _Pragma("GCC diagnostic ignored \"-Wshadow\"") \
  _Pragma("GCC diagnostic ignored \"-Wcast-align\"") \
  _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")

// Older versions of GCC don't support the push/pop pragmas. Right now we are
// not checking for GCC 3 or earlier. I'm not sure we have a use case for that.
#if defined(REMUS_GCC) && (__GNUC__ == 4 && __GNUC_MINOR__ < 6)
#define REMUS_THIRDPARTY_WARNINGS_PUSH
#define REMUS_THRIDPARTY_WARNINGS_POP
#else
#define REMUS_THIRDPARTY_WARNINGS_PUSH _Pragma("GCC diagnostic push")
#define REMUS_THRIDPARTY_WARNINGS_POP  _Pragma("GCC diagnostic pop")
#endif

#define REMUS_THIRDPARTY_PRE_INCLUDE \
  REMUS_THIRDPARTY_WARNINGS_PUSH \
  REMUS_THIRDPARTY_GCC_WARNING_PRAGMAS
#define REMUS_THIRDPARTY_POST_INCLUDE \
  REMUS_THRIDPARTY_WARNINGS_POP

#else
#define REMUS_THIRDPARTY_PRE_INCLUDE
#define REMUS_THIRDPARTY_POST_INCLUDE
#endif

#endif

//Now define some global windows suppressions
#if defined(REMUS_MSVC) // Visual studio
# pragma warning ( disable : 4251 ) //missing DLL-interface
# pragma warning ( disable : 4514 ) //unreferenced inline function
#endif
