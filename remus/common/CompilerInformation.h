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

#ifdef _MSC_VER
#define REMUS_MSVC
#endif

#if defined(__clang__) && !defined(__INTEL_COMPILER)
//On OSX the intel compiler uses clang as the front end
#define REMUS_CLANG
#endif

#ifdef __INTEL_COMPILER
#define REMUS_ICC
#endif

#ifdef __PGI
#define REMUS_PGI
#endif

// Several compilers pretend to be GCC but have minor differences. Try to
// compensate for that.
#if defined(__GNUC__) && !defined(REMUS_CLANG) && !defined(REMUS_ICC)
#define REMUS_GCC
#endif

#if __cplusplus >= 201103L || ( defined(REMUS_MSVC) && _MSC_VER >= 1700  )
#define REMUS_HAVE_CXX_11
#endif

#endif
